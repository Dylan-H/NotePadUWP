// SciTE - Scintilla based Text Editor
/** @file SciTEIO.cxx
 ** Manage input and output with the system.
 **/
// Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.
#include "pch.h"
#include <cstddef>
#include <cstdlib>
// Older versions of GNU stdint.h require this definition to be able to see INT32_MAX
#define __STDC_LIMIT_MACROS
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <ctime>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <memory>

#include <wrl.h>
#include <robuffer.h>
#include <fcntl.h>

#include "ILoader.h"
#include "Scintilla.h"

#include "GUI.h"
#include "ScintillaWindow.h"

#include "StringList.h"
#include "StringHelpers.h"
#include "FilePath.h"
#include "StyleDefinition.h"
#include "PropSetFile.h"
#include "StyleWriter.h"
#include "Extender.h"

#include "JobQueue.h"
#include "Cookie.h"

#include "MatchMarker.h"
#include "SciTEBase.h"
#include "Utf8_16.h"


// Windows
const GUI::gui_char propUserFileName[] = GUI_TEXT("SciTEUser.properties");

const GUI::gui_char propGlobalFileName[] = GUI_TEXT("SciTEGlobal.properties");
const GUI::gui_char propAbbrevFileName[] = GUI_TEXT("abbrev.properties");

void SciTEBase::SetFileName(const FilePath &openName, bool fixCase) {
	if (!openName.Exists())
			return;
	filePath.Set(openName);

	if (fixCase) {
		filePath.FixName();
	}

	ReadLocalPropFile();

	//props.Set("FilePath", filePath.AsUTF8().c_str());

	//props.Set("FileName",  filePath.BaseName().AsUTF8().c_str());

	//props.Set("FileNameExt", FileNameExt().AsUTF8().c_str());

	SetWindowName();
	if (buffers.buffers.size() > 0)
		CurrentBuffer()->file.Set(filePath);
}

// See if path exists.
// If path is not absolute, it is combined with dir.
// If resultPath is not NULL, it receives the absolute path if it exists.
bool SciTEBase::Exists(const GUI::gui_char *dir, const GUI::gui_char *path, FilePath *resultPath) {
	//FilePath copy(path);
	//if (!copy.IsAbsolute() && dir) {
	//	copy.SetDirectory(dir);
	//}
	//if (!copy.Exists())
	//	return false;
	//if (resultPath) {
	//	resultPath->Set(copy.AbsolutePath());
	//}
	return true;
}

void SciTEBase::CountLineEnds(int &linesCR, int &linesLF, int &linesCRLF) {
	linesCR = 0;
	linesLF = 0;
	linesCRLF = 0;
	const int lengthDoc = LengthDocument();
	char chPrev = ' ';
	TextReader acc(wEditor);
	char chNext = acc.SafeGetCharAt(0);
	for (int i = 0; i < lengthDoc; i++) {
		const char ch = chNext;
		chNext = acc.SafeGetCharAt(i + 1);
		if (ch == '\r') {
			if (chNext == '\n')
				linesCRLF++;
			else
				linesCR++;
		} else if (ch == '\n') {
			if (chPrev != '\r') {
				linesLF++;
			}
		} else if (i > 1000000) {
			return;
		}
		chPrev = ch;
	}
}

void SciTEBase::DiscoverEOLSetting() {
	SetEol();
	if (props.GetInt("eol.auto")) {
		int linesCR;
		int linesLF;
		int linesCRLF;
		CountLineEnds(linesCR, linesLF, linesCRLF);
		if (((linesLF >= linesCR) && (linesLF > linesCRLF)) || ((linesLF > linesCR) && (linesLF >= linesCRLF)))
			wEditor.Call(SCI_SETEOLMODE, SC_EOL_LF);
		else if (((linesCR >= linesLF) && (linesCR > linesCRLF)) || ((linesCR > linesLF) && (linesCR >= linesCRLF)))
			wEditor.Call(SCI_SETEOLMODE, SC_EOL_CR);
		else if (((linesCRLF >= linesLF) && (linesCRLF > linesCR)) || ((linesCRLF > linesLF) && (linesCRLF >= linesCR)))
			wEditor.Call(SCI_SETEOLMODE, SC_EOL_CRLF);
	}
}

// Look inside the first line for a #! clue regarding the language
std::string SciTEBase::DiscoverLanguage() {
	const int length = Minimum2(LengthDocument(), 64 * 1024);
	std::string buf = GetRangeString(wEditor, 0, length);
	std::string languageOverride = "";
	std::string l1 = ExtractLine(buf.c_str(), length);
	if (StartsWith(l1, "<?xml")) {
		languageOverride = "xml";
	} else if (StartsWith(l1, "#!")) {
		l1 = l1.substr(2);
		std::replace(l1.begin(), l1.end(), '\\', ' ');
		std::replace(l1.begin(), l1.end(), '/', ' ');
		std::replace(l1.begin(), l1.end(), '\t', ' ');
		Substitute(l1, "  ", " ");
		Substitute(l1, "  ", " ");
		Substitute(l1, "  ", " ");
		::Remove(l1, std::string("\r"));
		::Remove(l1, std::string("\n"));
		if (StartsWith(l1, " ")) {
			l1 = l1.substr(1);
		}
		std::replace(l1.begin(), l1.end(), ' ', '\0');
		l1.append(1, '\0');
		const char *word = l1.c_str();
		while (*word) {
			std::string propShBang("shbang.");
			propShBang.append(word);
			std::string langShBang = props.GetExpandedString(propShBang.c_str());
			if (langShBang.length()) {
				languageOverride = langShBang;
			}
			word += strlen(word) + 1;
		}
	}
	if (languageOverride.length()) {
		languageOverride.insert(0, "x.");
	}
	return languageOverride;
}

void SciTEBase::DiscoverIndentSetting() {
	const int lengthDoc = std::min(LengthDocument(), 1000000);
	TextReader acc(wEditor);
	bool newline = true;
	int indent = 0; // current line indentation
	int tabSizes[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // number of lines with corresponding indentation (index 0 - tab)
	int prevIndent = 0; // previous line indentation
	int prevTabSize = -1; // previous line tab size
	for (int i = 0; i < lengthDoc; i++) {
		const char ch = acc[i];
		if (ch == '\r' || ch == '\n') {
			indent = 0;
			newline = true;
		} else if (newline && ch == ' ') {
			indent++;
		} else if (newline) {
			if (indent) {
				if (indent == prevIndent && prevTabSize != -1) {
					tabSizes[prevTabSize]++;
				} else if (indent > prevIndent && prevIndent != -1) {
					if (indent - prevIndent <= 8) {
						prevTabSize = indent - prevIndent;
						tabSizes[prevTabSize]++;
					} else {
						prevTabSize = -1;
					}
				}
				prevIndent = indent;
			} else if (ch == '\t') {
				tabSizes[0]++;
				prevIndent = -1;
			} else {
				prevIndent = 0;
			}
			newline = false;
		}
	}
	// maximum non-zero indent
	int topTabSize = -1;
	for (int j = 0; j <= 8; j++) {
		if (tabSizes[j] && (topTabSize == -1 || tabSizes[j] > tabSizes[topTabSize])) {
			topTabSize = j;
		}
	}
	// set indentation
	if (topTabSize == 0) {
		wEditor.Call(SCI_SETUSETABS, 1);
	} else if (topTabSize != -1) {
		wEditor.Call(SCI_SETUSETABS, 0);
		wEditor.Call(SCI_SETINDENT, topTabSize);
	}
}
inline uint8_t* GetBufferData(winrt::Windows::Storage::Streams::IBuffer& buffer)
{
	::IUnknown* unknown = winrt::get_unknown(buffer);
	::Microsoft::WRL::ComPtr<::Windows::Storage::Streams::IBufferByteAccess> bufferByteAccess;
	HRESULT hr = unknown->QueryInterface(_uuidof(::Windows::Storage::Streams::IBufferByteAccess), &bufferByteAccess);
	if (FAILED(hr))
		return nullptr;
	byte* bytes = nullptr;
	bufferByteAccess->Buffer(&bytes);
	return bytes;
}

winrt::Windows::Foundation::IAsyncAction SciTEBase::OpenCurrentFileAsync(long long fileSize, bool suppressMessage) {
	CurrentBuffer()->SetTimeFromFile();

	wEditor.Call(SCI_BEGINUNDOACTION);	// Group together clear and insert
	wEditor.Call(SCI_CLEARALL);

	CurrentBuffer()->lifeState = Buffer::reading;

	wEditor.Call(SCI_ALLOCATE, static_cast<uptr_t>(fileSize) + 1000);

	Utf8_16_Read convert;
	//std::vector<char> data;
	size_t lenFile = 0;
	std::string propsData;

	winrt::Windows::Storage::Streams::IBuffer buf =co_await winrt::Windows::Storage::FileIO::ReadBufferAsync(filePath.GetFile());
	if (buf != nullptr)
	{
		lenFile = buf.Length();
		uint8_t* data = GetBufferData(buf);

		const UniMode umCodingCookie = CodingCookieValue((char*)data, lenFile);

		lenFile = convert.convert((char*)data, lenFile);
		const char *dataBlock = convert.getNewBuf();
		wEditor.CallString(SCI_ADDTEXT, lenFile, dataBlock);


		wEditor.Call(SCI_ENDUNDOACTION);

		CurrentBuffer()->unicodeMode = static_cast<UniMode>(
			static_cast<int>(convert.getEncoding()));
		// Check the first two lines for coding cookies
		if (CurrentBuffer()->unicodeMode == uni8Bit) {
			CurrentBuffer()->unicodeMode = umCodingCookie;
		}
		
		CompleteOpen(ocSynchronous);

	}
}
void SciTEBase::OpenCurrentFile(long long fileSize, bool suppressMessage, bool asynchronous) {

	CurrentBuffer()->SetTimeFromFile();

	wEditor.Call(SCI_BEGINUNDOACTION);	// Group together clear and insert
	wEditor.Call(SCI_CLEARALL);

	CurrentBuffer()->lifeState = Buffer::reading;
	if (asynchronous) {
		// Turn grey while loading
		wEditor.Call(SCI_STYLESETBACK, STYLE_DEFAULT, 0xEEEEEE);
		wEditor.Call(SCI_SETREADONLY, 1);
		//assert(CurrentBufferConst()->pFileWorker == nullptr);
		ILoader *pdocLoad;
		try {
			sptr_t docOptions = SC_DOCUMENTOPTION_DEFAULT;

			const long long sizeLarge = props.GetLongLong("file.size.large");
			if (sizeLarge && (fileSize > sizeLarge))
				docOptions |= SC_DOCUMENTOPTION_TEXT_LARGE;

			const long long sizeNoStyles = props.GetLongLong("file.size.no.styles");
			if (sizeNoStyles && (fileSize > sizeNoStyles))
				docOptions |= SC_DOCUMENTOPTION_STYLES_NONE;

			pdocLoad = reinterpret_cast<ILoader *>(
				wEditor.CallReturnPointer(SCI_CREATELOADER, static_cast<uptr_t>(fileSize) + 1000,
					docOptions));
		} catch (...) {
			wEditor.Call(SCI_SETSTATUS, 0);
			return;
		}
		//CurrentBuffer()->pFileWorker = new FileLoader(this, pdocLoad, filePath, static_cast<size_t>(fileSize), fp);
		//CurrentBuffer()->pFileWorker->sleepTime = props.GetInt("asynchronous.sleep");
		//PerformOnNewThread(CurrentBuffer()->pFileWorker);
	} else {
		wEditor.Call(SCI_ALLOCATE, static_cast<uptr_t>(fileSize) + 1000);

		Utf8_16_Read convert;
		//std::vector<char> data;
		size_t lenFile=0;
		std::string propsData;

		winrt::Windows::Storage::Streams::IBuffer buf = winrt::Windows::Storage::FileIO::ReadBufferAsync(filePath.GetFile()).get();
		if (buf != nullptr)
		{
			lenFile = buf.Length();
			uint8_t* data = GetBufferData(buf);

			const UniMode umCodingCookie = CodingCookieValue((char*)data, lenFile);

			lenFile = convert.convert((char*)data, lenFile);
			const char *dataBlock = convert.getNewBuf();
			wEditor.CallString(SCI_ADDTEXT, lenFile, dataBlock);


			wEditor.Call(SCI_ENDUNDOACTION);

			CurrentBuffer()->unicodeMode = static_cast<UniMode>(
				static_cast<int>(convert.getEncoding()));
			// Check the first two lines for coding cookies
			if (CurrentBuffer()->unicodeMode == uni8Bit) {
				CurrentBuffer()->unicodeMode = umCodingCookie;
			}
		}
		CompleteOpen(ocSynchronous);

	}
}



void SciTEBase::PerformDeferredTasks() {
	if (buffers.buffers[buffers.Current()].futureDo & Buffer::fdFinishSave) {
		wEditor.Call(SCI_SETSAVEPOINT);
		wEditor.Call(SCI_SETREADONLY, CurrentBuffer()->isReadOnly);
		buffers.FinishedFuture(buffers.Current(), Buffer::fdFinishSave);
	}
}

void SciTEBase::CompleteOpen(OpenCompletion oc) {
	wEditor.Call(SCI_SETREADONLY, CurrentBuffer()->isReadOnly);

	if (oc != ocSynchronous) {
		ReadProperties();
	}

	if (language == "") {
		std::string languageOverride = DiscoverLanguage();
		if (languageOverride.length()) {
			CurrentBuffer()->overrideExtension = languageOverride;
			CurrentBuffer()->lifeState = Buffer::open;
			ReadProperties();
			SetIndentSettings();
		}
	}

	if (oc != ocSynchronous) {
		SetIndentSettings();
		SetEol();
		UpdateBuffersCurrent();
		SizeSubWindows();
	}

	if (CurrentBuffer()->unicodeMode != uni8Bit) {
		// Override the code page if Unicode
		codePage = SC_CP_UTF8;
	} else {
		codePage = props.GetInt("code.page");
	}
	wEditor.Call(SCI_SETCODEPAGE, codePage);



	if (props.GetInt("indent.auto")) {
		DiscoverIndentSetting();
	}

	if (!wEditor.Call(SCI_GETUNDOCOLLECTION)) {
		wEditor.Call(SCI_SETUNDOCOLLECTION, 1);
	}
	wEditor.Call(SCI_SETSAVEPOINT);
	if (props.GetInt("fold.on.open") > 0) {
		FoldAll();
	}
	wEditor.Call(SCI_GOTOPOS, 0);

	CurrentBuffer()->CompleteLoading();

	Redraw();
}



bool SciTEBase::PreOpenCheck(const GUI::gui_char *) {
	return false;
}
winrt::Windows::Foundation::IAsyncOperation<bool> SciTEBase::OpenAsync(const FilePath &file, OpenFlags of)
{
	InitialiseBuffers();

	FilePath absPath = file;
	const int index = buffers.GetDocumentByName(file);
	if (index >= 0) {
		buffers.SetVisible(index, true);
		SetDocumentAt(index);
		RemoveFileFromStack(file);
		DeleteFileStackMenu();
		SetFileStackMenu();
		// If not forcing reload or currently busy with load or save, just rotate into view
		if ((!(of & ofForceLoad)))
			co_return true;
	}
	// See if we can have a buffer for the file to open
	if (!CanMakeRoom(!(of & ofNoSaveIfDirty))) {
		co_return false;
	}

	const long long fileSize = file.IsUntitled() ? 0 : co_await file.GetFileLengthAsync();
	if (fileSize > INTPTR_MAX) {
		const GUI::gui_string sSize = GUI::StringFromLongLong(fileSize);
		const GUI::gui_string msg = LocaliseMessage("File '^0' is ^1 bytes long, "
			"larger than 2GB which is the largest SciTE can open.",
			file.AsInternal(), sSize.c_str());
		//WindowMessageBox(wSciTE, msg, mbsIconWarning);
		return false;
	}
	if (fileSize > 0) {
		// Real file, not empty buffer
		const long long maxSize = props.GetLongLong("max.file.size", 2000000000LL);
		if (maxSize > 0 && fileSize > maxSize) {
			const GUI::gui_string sSize = GUI::StringFromLongLong(fileSize);
			const GUI::gui_string sMaxSize = GUI::StringFromLongLong(maxSize);
			const GUI::gui_string msg = LocaliseMessage("File '^0' is ^1 bytes long,\n"
				"larger than the ^2 bytes limit set in the properties.\n"
				"Do you still want to open it?",
				absPath.AsInternal(), sSize.c_str(), sMaxSize.c_str());
			//const MessageBoxChoice answer = WindowMessageBox(wSciTE, msg, mbsYesNo | mbsIconWarning);
			//if (answer != mbYes) {
			//	return false;
			//}
		}
	}

	if (buffers.size() == buffers.length) {
		AddFileToStack(RecentFile(filePath, GetSelectedRange(), GetCurrentScrollPosition()));
		ClearDocument();
		CurrentBuffer()->lifeState = Buffer::open;
		if (extender)
			extender->InitBuffer(buffers.Current());
	}
	else {
		if (index < 0 || !(of & ofForceLoad)) { // No new buffer, already opened
			New();
		}
	}

	//assert(CurrentBufferConst()->pFileWorker == nullptr);
	SetFileName(absPath);

	propsDiscovered.Clear();
	std::string discoveryScript = props.GetExpandedString("command.discover.properties");
	if (discoveryScript.length()) {
		std::string propertiesText = CommandExecute(GUI::StringFromUTF8(discoveryScript).c_str(),
			absPath.AsInternal());
		if (propertiesText.size()) {
			propsDiscovered.ReadFromMemory(propertiesText.c_str(), propertiesText.size(), absPath, filter, nullptr, 0);
		}
	}
	CurrentBuffer()->props = propsDiscovered;
	CurrentBuffer()->overrideExtension = "";
	ReadProperties();
	SetIndentSettings();
	SetEol();
	UpdateBuffersCurrent();
	SizeSubWindows();
	SetBuffersMenu();

	bool asynchronous = false;
	if (!filePath.IsUntitled()) {
		wEditor.Call(SCI_SETREADONLY, 0);
		wEditor.Call(SCI_CANCEL);
		if (of & ofPreserveUndo) {
			wEditor.Call(SCI_BEGINUNDOACTION);
		}
		else {
			wEditor.Call(SCI_SETUNDOCOLLECTION, 0);
		}

		asynchronous = false; //(fileSize > props.GetInt("background.open.size", -1)) &&
			//!(of & (ofPreserveUndo|ofSynchronous));
		co_await  OpenCurrentFileAsync(fileSize, of & ofQuiet);

		if (of & ofPreserveUndo) {
			wEditor.Call(SCI_ENDUNDOACTION);
		}
		else {
			wEditor.Call(SCI_EMPTYUNDOBUFFER);
		}
		CurrentBuffer()->isReadOnly = props.GetInt("read.only");
		wEditor.Call(SCI_SETREADONLY, CurrentBuffer()->isReadOnly);
	}
	RemoveFileFromStack(filePath);
	DeleteFileStackMenu();
	SetFileStackMenu();
	SetWindowName();
	if (lineNumbers && lineNumbersExpand)
		SetLineNumberWidth();
	UpdateStatusBar(true);
	if (extender && !asynchronous)
		extender->OnOpen(filePath.AsUTF8().c_str());
	co_return true;
}
bool SciTEBase::Open(const FilePath &file, OpenFlags of) {
	InitialiseBuffers();


	FilePath absPath = file;
	const int index = buffers.GetDocumentByName(file);
	if (index >= 0) {
		buffers.SetVisible(index, true);
		SetDocumentAt(index);
		RemoveFileFromStack(file);
		DeleteFileStackMenu();
		SetFileStackMenu();
		// If not forcing reload or currently busy with load or save, just rotate into view
		if ((!(of & ofForceLoad)))
			return true;
	}
	// See if we can have a buffer for the file to open
	if (!CanMakeRoom(!(of & ofNoSaveIfDirty))) {
		return false;
	}

	const long long fileSize = file.IsUntitled() ? 0 : file.GetFileLength();
	if (fileSize > INTPTR_MAX) {
		const GUI::gui_string sSize = GUI::StringFromLongLong(fileSize);
		const GUI::gui_string msg = LocaliseMessage("File '^0' is ^1 bytes long, "
			"larger than 2GB which is the largest SciTE can open.",
			file.AsInternal(), sSize.c_str());
		//WindowMessageBox(wSciTE, msg, mbsIconWarning);
		return false;
	}
	if (fileSize > 0) {
		// Real file, not empty buffer
		const long long maxSize = props.GetLongLong("max.file.size", 2000000000LL);
		if (maxSize > 0 && fileSize > maxSize) {
			const GUI::gui_string sSize = GUI::StringFromLongLong(fileSize);
			const GUI::gui_string sMaxSize = GUI::StringFromLongLong(maxSize);
			const GUI::gui_string msg = LocaliseMessage("File '^0' is ^1 bytes long,\n"
			        "larger than the ^2 bytes limit set in the properties.\n"
			        "Do you still want to open it?",
			        absPath.AsInternal(), sSize.c_str(), sMaxSize.c_str());
			//const MessageBoxChoice answer = WindowMessageBox(wSciTE, msg, mbsYesNo | mbsIconWarning);
			//if (answer != mbYes) {
			//	return false;
			//}
		}
	}

	if (buffers.size() == buffers.length) {
		AddFileToStack(RecentFile(filePath, GetSelectedRange(), GetCurrentScrollPosition()));
		ClearDocument();
		CurrentBuffer()->lifeState = Buffer::open;
		if (extender)
			extender->InitBuffer(buffers.Current());
	} else {
		if (index < 0 || !(of & ofForceLoad)) { // No new buffer, already opened
			New();
		}
	}

	//assert(CurrentBufferConst()->pFileWorker == nullptr);
	SetFileName(absPath);

	propsDiscovered.Clear();
	std::string discoveryScript = props.GetExpandedString("command.discover.properties");
	if (discoveryScript.length()) {
		std::string propertiesText = CommandExecute(GUI::StringFromUTF8(discoveryScript).c_str(),
			absPath.AsInternal());
		if (propertiesText.size()) {
			propsDiscovered.ReadFromMemory(propertiesText.c_str(), propertiesText.size(), absPath, filter, nullptr, 0);
		}
	}
	CurrentBuffer()->props = propsDiscovered;
	CurrentBuffer()->overrideExtension = "";
	ReadProperties();
	SetIndentSettings();
	SetEol();
	UpdateBuffersCurrent();
	SizeSubWindows();
	SetBuffersMenu();

	bool asynchronous = false;
	if (!filePath.IsUntitled()) {
		wEditor.Call(SCI_SETREADONLY, 0);
		wEditor.Call(SCI_CANCEL);
		if (of & ofPreserveUndo) {
			wEditor.Call(SCI_BEGINUNDOACTION);
		} else {
			wEditor.Call(SCI_SETUNDOCOLLECTION, 0);
		}

		asynchronous = false; //(fileSize > props.GetInt("background.open.size", -1)) &&
			//!(of & (ofPreserveUndo|ofSynchronous));
		OpenCurrentFile(fileSize, of & ofQuiet, asynchronous);

		if (of & ofPreserveUndo) {
			wEditor.Call(SCI_ENDUNDOACTION);
		} else {
			wEditor.Call(SCI_EMPTYUNDOBUFFER);
		}
		CurrentBuffer()->isReadOnly = props.GetInt("read.only");
		wEditor.Call(SCI_SETREADONLY, CurrentBuffer()->isReadOnly);
	}
	RemoveFileFromStack(filePath);
	DeleteFileStackMenu();
	SetFileStackMenu();
	SetWindowName();
	if (lineNumbers && lineNumbersExpand)
		SetLineNumberWidth();
	UpdateStatusBar(true);
	if (extender && !asynchronous)
		extender->OnOpen(filePath.AsUTF8().c_str());
	return true;
}

// Returns true if editor should get the focus
bool SciTEBase::OpenSelected() {
	std::string selName = SelectionFilename();
	if (selName.length() == 0) {
		WarnUser(warnWrongFile);
		return false;	// No selection
	}

#if !defined(GTK)
	if (StartsWith(selName, "http:") ||
		StartsWith(selName, "https:") ||
		StartsWith(selName, "ftp:") ||
		StartsWith(selName, "ftps:") ||
		StartsWith(selName, "news:") ||
		StartsWith(selName, "mailto:")) {
		std::string cmd = selName;
		AddCommand(cmd, "", jobShell);
		return false;	// Job is done
	}
#endif

	if (StartsWith(selName, "file://")) {
		selName.erase(0, 7);
		if (selName[0] == '/' && selName[2] == ':') { // file:///C:/filename.ext
			selName.erase(0, 1);
		}
	}

	std::string fileNameForExtension = ExtensionFileName();
	std::string openSuffix = props.GetNewExpandString("open.suffix.", fileNameForExtension.c_str());
	selName += openSuffix;

	if (EqualCaseInsensitive(selName.c_str(), FileNameExt().AsUTF8().c_str()) || EqualCaseInsensitive(selName.c_str(), filePath.AsUTF8().c_str())) {
		WarnUser(warnWrongFile);
		return true;	// Do not open if it is the current file!
	}

	std::string cTag;
	unsigned long lineNumber = 0;
	if (IsPropertiesFile(filePath) &&
	        (selName.find('.') == std::string::npos)) {
		// We are in a properties file and try to open a file without extension,
		// we suppose we want to open an imported .properties file
		// So we append the correct extension to open the included file.
		// Maybe we should check if the filename is preceded by "import"...
		selName += PROPERTIES_EXTENSION;
	} else {
		// Check if we have a line number (error message or grep result)
		// A bit of duplicate work with DecodeMessage, but we don't know
		// here the format of the line, so we do guess work.
		// Can't do much for space separated line numbers anyway...
		size_t endPath = selName.find('(');
		if (endPath != std::string::npos) {	// Visual Studio error message: F:\scite\src\SciTEBase.h(312):	bool Exists(
			lineNumber = atol(selName.c_str() + endPath + 1);
		} else {
			endPath = selName.find(':', 2);	// Skip Windows' drive separator
			if (endPath != std::string::npos) {	// grep -n line, perhaps gcc too: F:\scite\src\SciTEBase.h:312:	bool Exists(
				lineNumber = atol(selName.c_str() + endPath + 1);
			}
		}
		if (lineNumber > 0) {
			selName.erase(endPath);
		}

		// Support the ctags format

		if (lineNumber == 0) {
			cTag = GetCTag();
		}
	}

	FilePath path;
	// Don't load the path of the current file if the selected
	// filename is an absolute pathname
	//GUI::gui_string selFN = GUI::StringFromUTF8(selName);
	//if (!FilePath(selFN).IsAbsolute()) {
	//	path = filePath;
	//	// If not there, look in openpath
	//	if (!Exists(path.AsInternal(), selFN.c_str(), nullptr)) {
	//		GUI::gui_string openPath = GUI::StringFromUTF8(props.GetNewExpandString(
	//		            "openpath.", fileNameForExtension.c_str()));
	//		while (openPath.length()) {
	//			GUI::gui_string tryPath(openPath);
	//			const size_t sepIndex = tryPath.find(listSepString);
	//			if ((sepIndex != GUI::gui_string::npos) && (sepIndex != 0)) {
	//				tryPath.erase(sepIndex);
	//				openPath.erase(0, sepIndex + 1);
	//			} else {
	//				openPath.erase();
	//			}
	//			if (Exists(tryPath.c_str(), selFN.c_str(), nullptr)) {
	//				path.Set(tryPath.c_str());
	//				break;
	//			}
	//		}
	//	}
	//}
	//FilePath pathReturned;
	//if (Exists(path.AsInternal(), selFN.c_str(), &pathReturned)) {
	//	// Open synchronously if want to seek line number or search tag
	//	const OpenFlags of = ((lineNumber > 0) || (cTag.length() != 0)) ? ofSynchronous : ofNone;
	//	if (Open(pathReturned, of)) {
	//		if (lineNumber > 0) {
	//			wEditor.Call(SCI_GOTOLINE, lineNumber - 1);
	//		} else if (cTag.length() != 0) {
	//			if (atoi(cTag.c_str()) > 0) {
	//				wEditor.Call(SCI_GOTOLINE, atoi(cTag.c_str()) - 1);
	//			} else {
	//				findWhat = cTag;
	//				FindNext(false);
	//			}
	//		}
	//		return true;
	//	}
	//} else {
	//	WarnUser(warnWrongFile);
	//}
	return false;
}

void SciTEBase::Revert() {
	if (filePath.IsUntitled()) {
		wEditor.Call(SCI_CLEARALL);
	} else {
		RecentFile rf = GetFilePosition();
		OpenCurrentFile(filePath.GetFileLength(), false, false);
		DisplayAround(rf);
	}
}

void SciTEBase::CheckReload() {
	if (props.GetInt("load.on.activate")) {
		// Make a copy of fullPath as otherwise it gets aliased in Open
		const time_t newModTime = filePath.ModifiedTime();
		if ((newModTime != 0) && (newModTime != CurrentBuffer()->fileModTime)) {
			RecentFile rf = GetFilePosition();
			const OpenFlags of = props.GetInt("reload.preserves.undo") ? ofPreserveUndo : ofNone;
			if (CurrentBuffer()->isDirty || props.GetInt("are.you.sure.on.reload") != 0) {
				if ((0 == dialogsOnScreen) && (newModTime != CurrentBuffer()->fileModLastAsk)) {
					GUI::gui_string msg;
					if (CurrentBuffer()->isDirty) {
						msg = LocaliseMessage(
						          "The file '^0' has been modified. Should it be reloaded?",
						          filePath.AsInternal());
					} else {
						msg = LocaliseMessage(
						          "The file '^0' has been modified outside SciTE. Should it be reloaded?",
						          FileNameExt().AsInternal());
					}
					//const MessageBoxChoice decision = WindowMessageBox(wSciTE, msg, mbsYesNo | mbsIconQuestion);
					//if (decision == mbYes) {
					//	Open(filePath, static_cast<OpenFlags>(of | ofForceLoad));
					//	DisplayAround(rf);
					//}
					CurrentBuffer()->fileModLastAsk = newModTime;
				}
			} else {
				Open(filePath, static_cast<OpenFlags>(of | ofForceLoad));
				DisplayAround(rf);
			}
		}  else if (newModTime == 0 && CurrentBuffer()->fileModTime != 0)  {
			// Check if the file is deleted
			CurrentBuffer()->fileModTime = 0;
			CurrentBuffer()->fileModLastAsk = 0;
			CurrentBuffer()->isDirty = true;
			CheckMenus();
			SetWindowName();
			SetBuffersMenu();
			GUI::gui_string msg = LocaliseMessage(
						      "The file '^0' has been deleted.",
						      filePath.AsInternal());
			//WindowMessageBox(wSciTE, msg, mbsOK);
		}
	}
}

void SciTEBase::Activate(bool activeApp) {
	if (activeApp) {
		CheckReload();
	} else {
		if (props.GetInt("save.on.deactivate")) {
			SaveTitledBuffers();
		}
	}
}

FilePath SciTEBase::SaveName(const char *ext) const {
	GUI::gui_string savePath = filePath.AsInternal();
	if (ext) {
		int dot = static_cast<int>(savePath.length() - 1);
		while ((dot >= 0) && (savePath[dot] != '.')) {
			dot--;
		}
		if (dot >= 0) {
			const int keepExt = props.GetInt("export.keep.ext");
			if (keepExt == 0) {
				savePath.erase(dot);
			} else if (keepExt == 2) {
				savePath[dot] = '_';
			}
		}
		savePath += GUI::StringFromUTF8(ext);
	}
	//~ fprintf(stderr, "SaveName <%s> <%s> <%s>\n", filePath.AsInternal(), savePath.c_str(), ext);
	return FilePath(/*savePath.c_str()*/);
}

SciTEBase::SaveResult SciTEBase::SaveIfUnsure(bool forceQuestion, SaveFlags sf) {
	CurrentBuffer()->failedSave = false;
	//if (CurrentBuffer()->pFileWorker) {
	//	if (CurrentBuffer()->pFileWorker->IsLoading())
	//		// In semi-loaded state so refuse to save
	//		return saveCancelled;
	//	else
	//		return saveCompleted;
	//}
	if ((CurrentBuffer()->isDirty) && (LengthDocument() || !filePath.IsUntitled() || forceQuestion)) {
		if (props.GetInt("are.you.sure", 1) ||
		        filePath.IsUntitled() ||
		        forceQuestion) {
					GUI::gui_string msg;
			if (!filePath.IsUntitled()) {
				msg = LocaliseMessage("Save changes to '^0'?", filePath.AsInternal());
			} else {
				msg = LocaliseMessage("Save changes to (Untitled)?");
			}
			//const MessageBoxChoice decision = WindowMessageBox(wSciTE, msg, mbsYesNoCancel | mbsIconQuestion);
			//if (decision == mbYes) {
			//	if (!Save(sf))
			//		return saveCancelled;
			//}
			//return (decision == mbCancel) ? saveCancelled : saveCompleted;
		} else {
			if (!Save(sf))
				return saveCancelled;
		}
	}
	return saveCompleted;
}

SciTEBase::SaveResult SciTEBase::SaveIfUnsureAll() {
	if (SaveAllBuffers(false) == saveCancelled) {
		return saveCancelled;
	}
	if (props.GetInt("save.recent")) {
		for (int i = 0; i < buffers.lengthVisible; ++i) {
			Buffer buff = buffers.buffers[i];
			AddFileToStack(buff.file);
		}
	}
	if (props.GetInt("save.session") || props.GetInt("save.position") || props.GetInt("save.recent")) {
		SaveSessionFile(GUI_TEXT(""));
	}

	if (extender && extender->NeedsOnClose()) {
		// Ensure extender is told about each buffer closing
		for (int k = 0; k < buffers.lengthVisible; k++) {
			SetDocumentAt(k);
			extender->OnClose(filePath.AsUTF8().c_str());
		}
	}

	// Definitely going to exit now, so delete all documents
	// Set editor back to initial document
	if (buffers.lengthVisible > 0) {
		wEditor.Call(SCI_SETDOCPOINTER, 0, buffers.buffers[0].doc);
	}
	// Release all the extra documents
	for (int j = 0; j < buffers.size(); j++) {
		if (buffers.buffers[j].doc) {
			wEditor.Call(SCI_RELEASEDOCUMENT, 0, buffers.buffers[j].doc);
			buffers.buffers[j].doc = 0;
		}
	}
	// Initial document will be deleted when editor deleted
	return saveCompleted;
}

SciTEBase::SaveResult SciTEBase::SaveIfUnsureForBuilt() {
	if (props.GetInt("save.all.for.build")) {
		return SaveAllBuffers(!props.GetInt("are.you.sure.for.build"));
	}
	if (CurrentBuffer()->isDirty) {
		if (props.GetInt("are.you.sure.for.build"))
			return SaveIfUnsure(true);

		Save();
	}
	return saveCompleted;
}
/**
	Selection saver and restorer.

	If virtual space is disabled, the class does nothing.

	If virtual space is enabled, constructor saves all selections using (line, column) coordinates,
	destructor restores all the saved selections.
**/
class SelectionKeeper {
public:
	explicit SelectionKeeper(GUI::ScintillaWindow &editor) : wEditor(editor) {
		const int mask = SCVS_RECTANGULARSELECTION | SCVS_USERACCESSIBLE;
		if (wEditor.Call(SCI_GETVIRTUALSPACEOPTIONS, 0, 0) & mask) {
			const int n = wEditor.Call(SCI_GETSELECTIONS, 0, 0);
			for (int i = 0; i < n; ++i) {
				selections.push_back(LocFromPos(GetSelection(i)));
			}
		}
	}

	~SelectionKeeper() {
		int i = 0;
		for (auto const &sel : selections) {
			SetSelection(i, PosFromLoc(sel));
			++i;
		}
	}

private:
	struct Position {
		Position(int pos_, int virt_ = 0) : pos(pos_), virt(virt_) {};
		int pos;
		int virt;
	};

	struct Location {
		Location(int line_, int col_) : line(line_), col(col_) {};
		int line;
		int col;
	};

	Position GetAnchor(int i) {
		const int pos  = wEditor.Call(SCI_GETSELECTIONNANCHOR, i, 0);
		const int virt = wEditor.Call(SCI_GETSELECTIONNANCHORVIRTUALSPACE, i, 0);
		return Position(pos, virt);
	}

	Position GetCaret(int i) {
		const int pos  = wEditor.Call(SCI_GETSELECTIONNCARET, i, 0);
		const int virt = wEditor.Call(SCI_GETSELECTIONNCARETVIRTUALSPACE, i, 0);
		return Position(pos, virt);
	}

	std::pair<Position, Position> GetSelection(int i) {
		return {GetAnchor(i), GetCaret(i)};
	};

	Location LocFromPos(Position const &pos) {
		const int line = wEditor.Call(SCI_LINEFROMPOSITION, pos.pos, 0);
		const int col  = wEditor.Call(SCI_GETCOLUMN, pos.pos, 0) + pos.virt;
		return Location(line, col);
	}

	std::pair<Location, Location> LocFromPos(std::pair<Position, Position> const &pos) {
		return {LocFromPos(pos.first), LocFromPos(pos.second)};
	}

	Position PosFromLoc(Location const &loc) {
		const int pos = wEditor.Call(SCI_FINDCOLUMN, loc.line, loc.col);
		const int col = wEditor.Call(SCI_GETCOLUMN, pos, 0);
		return Position(pos, loc.col - col);
	}

	std::pair<Position, Position> PosFromLoc(std::pair<Location, Location> const &loc) {
		return {PosFromLoc(loc.first), PosFromLoc(loc.second)};
	}

	void SetAnchor(int i, Position const &pos) {
		wEditor.Call(SCI_SETSELECTIONNANCHOR, i, pos.pos);
		wEditor.Call(SCI_SETSELECTIONNANCHORVIRTUALSPACE, i, pos.virt);
	};

	void SetCaret(int i, Position const &pos) {
		wEditor.Call(SCI_SETSELECTIONNCARET, i, pos.pos);
		wEditor.Call(SCI_SETSELECTIONNCARETVIRTUALSPACE, i, pos.virt);
	}

	void SetSelection(int i, std::pair<Position, Position> const &pos) {
		SetAnchor(i, pos.first);
		SetCaret(i, pos.second);
	}

	GUI::ScintillaWindow &wEditor;
	std::vector<std::pair<Location, Location>> selections;
};

void SciTEBase::StripTrailingSpaces() {
	const int maxLines = wEditor.Call(SCI_GETLINECOUNT);
	SelectionKeeper keeper(wEditor);
	for (int line = 0; line < maxLines; line++) {
		const int lineStart = wEditor.Call(SCI_POSITIONFROMLINE, line);
		const int lineEnd = wEditor.Call(SCI_GETLINEENDPOSITION, line);
		int i = lineEnd - 1;
		char ch = static_cast<char>(wEditor.Call(SCI_GETCHARAT, i));
		while ((i >= lineStart) && ((ch == ' ') || (ch == '\t'))) {
			i--;
			ch = static_cast<char>(wEditor.Call(SCI_GETCHARAT, i));
		}
		if (i < (lineEnd - 1)) {
			wEditor.Call(SCI_SETTARGETSTART, i + 1);
			wEditor.Call(SCI_SETTARGETEND, lineEnd);
			wEditor.CallString(SCI_REPLACETARGET, 0, "");
		}
	}
}

void SciTEBase::EnsureFinalNewLine() {
	const int maxLines = wEditor.Call(SCI_GETLINECOUNT);
	bool appendNewLine = maxLines == 1;
	const int endDocument = wEditor.Call(SCI_POSITIONFROMLINE, maxLines);
	if (maxLines > 1) {
		appendNewLine = endDocument > wEditor.Call(SCI_POSITIONFROMLINE, maxLines - 1);
	}
	if (appendNewLine) {
		const char *eol = "\n";
		switch (wEditor.Call(SCI_GETEOLMODE)) {
		case SC_EOL_CRLF:
			eol = "\r\n";
			break;
		case SC_EOL_CR:
			eol = "\r";
			break;
		}
		wEditor.CallString(SCI_INSERTTEXT, endDocument, eol);
	}
}

// Perform any changes needed before saving such as normalizing spaces and line ends.
bool SciTEBase::PrepareBufferForSave(const FilePath &saveName) {
	bool retVal = false;
	// Perform clean ups on text before saving
	wEditor.Call(SCI_BEGINUNDOACTION);
	if (stripTrailingSpaces)
		StripTrailingSpaces();
	if (ensureFinalLineEnd)
		EnsureFinalNewLine();
	if (ensureConsistentLineEnds)
		wEditor.Call(SCI_CONVERTEOLS, wEditor.Call(SCI_GETEOLMODE));

	if (extender)
		retVal = extender->OnBeforeSave(saveName.AsUTF8().c_str());

	wEditor.Call(SCI_ENDUNDOACTION);

	return retVal;
}

/**
 * Writes the buffer to the given filename.
 */
bool SciTEBase::SaveBuffer(const FilePath &saveName, SaveFlags sf) {
	bool retVal = PrepareBufferForSave(saveName);

	//if (!retVal) {

	//	FILE *fp = saveName.Open(fileWrite);
	//	if (fp) {
	//		const size_t lengthDoc = LengthDocument();
	//		if (!(sf & sfSynchronous)) {
	//			wEditor.Call(SCI_SETREADONLY, 1);
	//			const char *documentBytes = reinterpret_cast<const char *>(wEditor.CallReturnPointer(SCI_GETCHARACTERPOINTER));
	//			CurrentBuffer()->pFileWorker = new FileStorer(this, documentBytes, saveName, lengthDoc, fp, CurrentBuffer()->unicodeMode, (sf & sfProgressVisible));
	//			CurrentBuffer()->pFileWorker->sleepTime = props.GetInt("asynchronous.sleep");
	//			if (PerformOnNewThread(CurrentBuffer()->pFileWorker)) {
	//				retVal = true;
	//			} else {
	//				GUI::gui_string msg = LocaliseMessage("Failed to save file '^0' as thread could not be started.", saveName.AsInternal());
	//				WindowMessageBox(wSciTE, msg);
	//			}
	//		} else {
	//			Utf8_16_Write convert;
	//			if (CurrentBuffer()->unicodeMode != uniCookie) {	// Save file with cookie without BOM.
	//				convert.setEncoding(static_cast<Utf8_16::encodingType>(
	//						static_cast<int>(CurrentBuffer()->unicodeMode)));
	//			}
	//			convert.setfile(fp);
	//			std::vector<char> data(blockSize + 1);
	//			retVal = true;
	//			size_t grabSize;
	//			for (size_t i = 0; i < lengthDoc; i += grabSize) {
	//				grabSize = lengthDoc - i;
	//				if (grabSize > blockSize)
	//					grabSize = blockSize;
	//				// Round down so only whole characters retrieved.
	//				grabSize = wEditor.Call(SCI_POSITIONBEFORE, i + grabSize + 1) - i;
	//				GetRange(wEditor, static_cast<int>(i), static_cast<int>(i + grabSize), &data[0]);
	//				const size_t written = convert.fwrite(&data[0], grabSize);
	//				if (written == 0) {
	//					retVal = false;
	//					break;
	//				}
	//			}
	//			if (convert.fclose() != 0) {
	//				retVal = false;
	//			}
	//		}
	//	}
	//}

	//if (retVal && extender && (sf & sfSynchronous)) {
	//	extender->OnSave(saveName.AsUTF8().c_str());
	//}
	//UpdateStatusBar(true);
	return retVal;
}

void SciTEBase::ReloadProperties() {
	ReadGlobalPropFile();
	SetImportMenu();
	ReadLocalPropFile();
	ReadAbbrevPropFile();
	ReadProperties();
	SetWindowName();
	BuffersMenu();
	Redraw();
}

// Returns false if cancelled or failed to save
bool SciTEBase::Save(SaveFlags sf) {
	if (!filePath.IsUntitled()) {
		GUI::gui_string msg;
		if (CurrentBuffer()->ShouldNotSave()) {
			msg = LocaliseMessage(
				"The file '^0' has not yet been loaded entirely, so it can not be saved right now. Please retry in a while.",
				filePath.AsInternal());
			//WindowMessageBox(wSciTE, msg);
			// It is OK to not save this file
			return true;
		}

		//if (CurrentBuffer()->pFileWorker) {
		//	msg = LocaliseMessage(
		//		"The file '^0' is already being saved.",
		//		filePath.AsInternal());
		//	WindowMessageBox(wSciTE, msg);
		//	// It is OK to not save this file
		//	return true;
		//}

		if (props.GetInt("save.deletes.first")) {
			filePath.Remove();
		} else if (props.GetInt("save.check.modified.time")) {
			const time_t newModTime = filePath.ModifiedTime();
			if ((newModTime != 0) && (CurrentBuffer()->fileModTime != 0) &&
				(newModTime != CurrentBuffer()->fileModTime)) {
				msg = LocaliseMessage("The file '^0' has been modified outside SciTE. Should it be saved?",
					filePath.AsInternal());
				//const MessageBoxChoice decision = WindowMessageBox(wSciTE, msg, mbsYesNo | mbsIconQuestion);
				//if (decision == mbNo) {
				//	return false;
				//}
			}
		}

		if ((LengthDocument() <= props.GetInt("background.save.size", -1)) ||
			(buffers.SingleBuffer()))
			sf = static_cast<SaveFlags>(sf | sfSynchronous);
		if (SaveBuffer(filePath, sf)) {
			CurrentBuffer()->SetTimeFromFile();
			if (sf & sfSynchronous) {
				wEditor.Call(SCI_SETSAVEPOINT);
				if (IsPropertiesFile(filePath)) {
					ReloadProperties();
				}
			}
		} else {
			if (!CurrentBuffer()->failedSave) {
				CurrentBuffer()->failedSave = true;
				msg = LocaliseMessage(
					"Could not save file '^0'. Save under a different name?", filePath.AsInternal());
				//const MessageBoxChoice decision = WindowMessageBox(wSciTE, msg, mbsYesNo | mbsIconWarning);
				//if (decision == mbYes) {
				//	return SaveAsDialog();
				//}
			}
			return false;
		}
		return true;
	} 
	//	else {
	//	if (props.GetString("save.path.suggestion").length()) {
	//		const time_t t = time(nullptr);
	//		char timeBuff[15];
	//		strftime(timeBuff, sizeof(timeBuff), "%Y%m%d%H%M%S",  localtime(&t));
	//		PropSetFile propsSuggestion;
	//		propsSuggestion.superPS = &props;  // Allow access to other settings
	//		propsSuggestion.Set("TimeStamp", timeBuff);
	//		propsSuggestion.Set("SciteUserHome", GetSciteUserHome().AsUTF8().c_str());
	//		std::string savePathSuggestion = propsSuggestion.GetExpandedString("save.path.suggestion");
	//		std::replace(savePathSuggestion.begin(), savePathSuggestion.end(), '\\', '/');  // To accept "\" on Unix
	//		if (savePathSuggestion.size() > 0) {
	//			filePath = FilePath(GUI::StringFromUTF8(savePathSuggestion)).NormalizePath();
	//		}
	//	}
	//	const bool ret = SaveAsDialog();
	//	if (!ret)
	//		filePath.Set(GUI_TEXT(""));
	//	return ret;
	//}
}

void SciTEBase::SaveAs(const GUI::gui_char *file, bool fixCase) {
	//SetFileName(file, fixCase);
	Save();
	ReadProperties();
	wEditor.Call(SCI_CLEARDOCUMENTSTYLE);
	wEditor.Call(SCI_COLOURISE, 0, wEditor.Call(SCI_POSITIONFROMLINE, 1));
	Redraw();
	SetWindowName();
	BuffersMenu();
	if (extender)
		extender->OnSave(filePath.AsUTF8().c_str());
}

bool SciTEBase::SaveIfNotOpen(const FilePath &destFile, bool fixCase) {
	FilePath absPath = destFile;
	const int index = buffers.GetDocumentByName(absPath, true /* excludeCurrent */);
	if (index >= 0) {
		GUI::gui_string msg = LocaliseMessage(
			    "File '^0' is already open in another buffer.", destFile.AsInternal());
		//WindowMessageBox(wSciTE, msg);
		return false;
	} else {
		SaveAs(absPath.AsInternal(), fixCase);
		return true;
	}
}

void SciTEBase::AbandonAutomaticSave() {
	CurrentBuffer()->AbandonAutomaticSave();
}

bool SciTEBase::IsStdinBlocked() {
	return false; /* always default to blocked */
}

void SciTEBase::OpenFromStdin(bool UseOutputPane) {
	//Utf8_16_Read convert;
	//std::vector<char> data(blockSize);

	///* if stdin is blocked, do not execute this method */
	//if (IsStdinBlocked())
	//	return;

	//Open(GUI_TEXT(""));
	//if (UseOutputPane) {
	//	wOutput.Call(SCI_CLEARALL);
	//} else {
	//	wEditor.Call(SCI_BEGINUNDOACTION);	// Group together clear and insert
	//	wEditor.Call(SCI_CLEARALL);
	//}
	//size_t lenFile = fread(&data[0], 1, data.size(), stdin);
	//const UniMode umCodingCookie = CodingCookieValue(&data[0], lenFile);
	//while (lenFile > 0) {
	//	lenFile = convert.convert(&data[0], lenFile);
	//	if (UseOutputPane) {
	//		wOutput.CallString(SCI_ADDTEXT, lenFile, convert.getNewBuf());
	//	} else {
	//		wEditor.CallString(SCI_ADDTEXT, lenFile, convert.getNewBuf());
	//	}
	//	lenFile = fread(&data[0], 1, data.size(), stdin);
	//}
	//if (UseOutputPane) {
	//	if (props.GetInt("split.vertical") == 0) {
	//		heightOutput = 2000;
	//	} else {
	//		heightOutput = 500;
	//	}
	//	SizeSubWindows();
	//} else {
	//	wEditor.Call(SCI_ENDUNDOACTION);
	//}
	//CurrentBuffer()->unicodeMode = static_cast<UniMode>(
	//            static_cast<int>(convert.getEncoding()));
	//// Check the first two lines for coding cookies
	//if (CurrentBuffer()->unicodeMode == uni8Bit) {
	//	CurrentBuffer()->unicodeMode = umCodingCookie;
	//}
	//if (CurrentBuffer()->unicodeMode != uni8Bit) {
	//	// Override the code page if Unicode
	//	codePage = SC_CP_UTF8;
	//} else {
	//	codePage = props.GetInt("code.page");
	//}
	//if (UseOutputPane) {
	//	wOutput.Call(SCI_SETSEL, 0, 0);
	//} else {
	//	wEditor.Call(SCI_SETCODEPAGE, codePage);

	//	// Zero all the style bytes
	//	wEditor.Call(SCI_CLEARDOCUMENTSTYLE);

	//	CurrentBuffer()->overrideExtension = "x.txt";
	//	ReadProperties();
	//	SetIndentSettings();
	//	wEditor.Call(SCI_COLOURISE, 0, -1);
	//	Redraw();

	//	wEditor.Call(SCI_SETSEL, 0, 0);
	//}
}

void SciTEBase::OpenFilesFromStdin() {
	char data[8 * 1024];

	/* if stdin is blocked, do not execute this method */
	//if (IsStdinBlocked())
	//	return;

	//while (fgets(data, sizeof(data) - 1, stdin)) {
	//	char *pNL;
	//	if ((pNL = strchr(data, '\n')) != nullptr)
	//		* pNL = '\0';
	//	Open(GUI::StringFromUTF8(data).c_str(), ofQuiet);
	//}
	//if (buffers.lengthVisible == 0)
	//	Open(GUI_TEXT(""));
}

class BufferedFile {
	winrt::Windows::Storage::Streams::DataReader dr{ nullptr };
	bool readAll;
	bool exhausted;
	enum {bufLen = 64 * 1024};
	char buffer[bufLen];
	size_t pos;
	size_t valid;
	void EnsureData() {
		if (pos >= valid) {
			if (readAll || !dr) {
				exhausted = true;
			} else {
				auto len = dr.UnconsumedBufferLength();
				size_t valid = len > bufLen?bufLen:len;
				winrt::array_view<uint8_t> buf((uint8_t*)buffer, (uint8_t*)(buffer + valid));
				dr.ReadBytes(buf);
				if (valid < bufLen) {
					readAll = true;
				}
				pos = 0;
			}
		}
	}
public:
	explicit BufferedFile(const FilePath &fPath) {
		auto fp = fPath.GetFile();
		winrt::Windows::Storage::Streams::IBuffer ibuffer = winrt::Windows::Storage::FileIO::ReadBufferAsync(fp).get();
		dr = winrt::Windows::Storage::Streams::DataReader::FromBuffer(ibuffer);
		readAll = false;
		exhausted = fp == nullptr;
		buffer[0] = 0;
		pos = 0;
		valid = 0;
	}
	~BufferedFile() {
		if(dr)
			dr.Close();
	}
	bool Exhausted() const noexcept {
		return exhausted;
	}
	int NextByte() {
		EnsureData();
		if (pos >= valid) {
			return 0;
		}
		return buffer[pos++];
	}
	bool BufferContainsNull() {
		EnsureData();
		for (size_t i = 0;i < valid;i++) {
			if (buffer[i] == '\0')
				return true;
		}
		return false;
	}
};

class FileReader {
	std::unique_ptr<BufferedFile> bf;
	int lineNum;
	bool lastWasCR;
	std::string lineToCompare;
	std::string lineToShow;
	bool caseSensitive;
public:
	FileReader(const FilePath &fPath, bool caseSensitive_) : bf(std::make_unique<BufferedFile>(fPath)) {
		lineNum = 0;
		lastWasCR = false;
		caseSensitive = caseSensitive_;
	}
	// Deleted so FileReader objects can not be copied.
	FileReader(const FileReader &) = delete;
	~FileReader() {
	}
	const char *Next() {
		if (bf->Exhausted()) {
			return nullptr;
		}
		lineToShow.clear();
		while (!bf->Exhausted()) {
			const int ch = bf->NextByte();
			if (lastWasCR && ch == '\n' && lineToShow.empty()) {
				lastWasCR = false;
			} else if (ch == '\r' || ch == '\n') {
				lastWasCR = ch == '\r';
				break;
			} else {
				lineToShow.push_back(static_cast<char>(ch));
			}
		}
		lineNum++;
		lineToCompare = lineToShow;
		if (!caseSensitive) {
			LowerCaseAZ(lineToCompare);
		}
		return lineToCompare.c_str();
	}
	int LineNumber() const noexcept {
		return lineNum;
	}
	const char *Original() const noexcept {
		return lineToShow.c_str();
	}
	bool BufferContainsNull() {
		return bf->BufferContainsNull();
	}
};

static bool IsWordCharacter(int ch) noexcept {
	return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')  || (ch >= '0' && ch <= '9')  || (ch == '_');
}

bool SciTEBase::GrepIntoDirectory(const FilePath &directory) {
    const GUI::gui_char *sDirectory = directory.AsInternal();
#ifdef __APPLE__
    if (strcmp(sDirectory, "build") == 0)
        return false;
#endif
    return sDirectory[0] != '.';
}

void SciTEBase::GrepRecursive(GrepFlags gf, const FilePath &baseDir, const char *searchString, const GUI::gui_char *fileTypes) {
	const int checkAfterLines = 10'000;
	FilePathSet directories;
	FilePathSet files;
	baseDir.List(directories, files);
	const size_t searchLength = strlen(searchString);
	std::string os;
	for (const FilePath &fPath : files) {
		if (jobQueue.Cancelled())
			return;
		if (*fileTypes == '\0' || fPath.Matches(fileTypes)) {
			//OutputAppendStringSynchronised(i->AsInternal());
			//OutputAppendStringSynchronised("\n");
			FileReader fr(fPath, gf & grepMatchCase);
			if ((gf & grepBinary) || !fr.BufferContainsNull()) {
				while (const char *line = fr.Next()) {
					if (((fr.LineNumber() % checkAfterLines) == 0) && jobQueue.Cancelled())
						return;
					const char *match = strstr(line, searchString);
					if (match) {
						if (gf & grepWholeWord) {
							const char *lineEnd = line + strlen(line);
							while (match) {
								if (((match == line) || !IsWordCharacter(match[-1])) &&
								        ((match + searchLength == (lineEnd)) || !IsWordCharacter(match[searchLength]))) {
									break;
								}
								match = strstr(match + 1, searchString);
							}
						}
						if (match) {
							os.append(fPath.AsUTF8().c_str());
							os.append(":");
							std::string lNumber = StdStringFromInteger(fr.LineNumber());
							os.append(lNumber.c_str());
							os.append(":");
							os.append(fr.Original());
							os.append("\n");
						}
					}
				}
			}
		}
	}
	if (os.length()) {
		if (gf & grepStdOut) {
			fwrite(os.c_str(), os.length(), 1, stdout);
		} else {
			OutputAppendStringSynchronised(os.c_str());
		}
	}
	//for (const FilePath &fPath : directories) {
	//	if ((gf & grepDot) || GrepIntoDirectory(fPath.Name())) {
	//		GrepRecursive(gf, fPath, searchString, fileTypes);
	//	}
	//}
}

void SciTEBase::InternalGrep(GrepFlags gf, const GUI::gui_char *directory, const GUI::gui_char *fileTypes, const char *search, sptr_t &originalEnd) {
	GUI::ElapsedTime commandTime;
	if (!(gf & grepStdOut)) {
		std::string os;
		os.append(">Internal search for \"");
		os.append(search);
		os.append("\" in \"");
		os.append(GUI::UTF8FromString(fileTypes).c_str());
		os.append("\"\n");
		OutputAppendStringSynchronised(os.c_str());
		ShowOutputOnMainThread();
		originalEnd += os.length();
	}
	std::string searchString(search);
	if (!(gf & grepMatchCase)) {
		LowerCaseAZ(searchString);
	}
	//GrepRecursive(gf, FilePath(directory), searchString.c_str(), fileTypes);
	//if (!(gf & grepStdOut)) {
	//	std::string sExitMessage(">");
	//	if (jobQueue.TimeCommands()) {
	//		sExitMessage += "    Time: ";
	//		sExitMessage += StdStringFromDouble(commandTime.Duration(), 3);
	//	}
	//	sExitMessage += "\n";
	//	OutputAppendStringSynchronised(sExitMessage.c_str());
	//}
}