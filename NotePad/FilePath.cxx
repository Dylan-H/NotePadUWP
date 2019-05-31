// SciTE - Scintilla based Text Editor
/** @file FilePath.cxx
 ** Encapsulate a file path.
 **/
// Copyright 1998-2005 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.
#include "pch.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cerrno>

#include <string>
#include <string_view>
#include <vector>
#include <algorithm>

#include <wrl.h>
#include <robuffer.h>

#include <fcntl.h>

#include <sys/stat.h>

#if !defined(__unix__)

#include <io.h>

#undef _WIN32_WINNT
#define _WIN32_WINNT  0x0602
#include <windows.h>
#include <commctrl.h>

// For chdir
#include <direct.h>

#endif

#include "Scintilla.h"

#include "GUI.h"
#include "ScintillaWindow.h"

#include "FilePath.h"


// Windows
const GUI::gui_char pathSepString[] = GUI_TEXT("\\");
const GUI::gui_char pathSepChar = '\\';
const GUI::gui_char listSepString[] = GUI_TEXT(";");
const GUI::gui_char configFileVisibilityString[] = GUI_TEXT("");


const GUI::gui_char fileRead[] = GUI_TEXT("rb");
const GUI::gui_char fileWrite[] = GUI_TEXT("wb");

namespace {
const GUI::gui_char currentDirectory[] = GUI_TEXT(".");
const GUI::gui_char parentDirectory[] = GUI_TEXT("..");
}



FilePath::FilePath(const StorageFile &file_) : file(file_) {}



void FilePath::Set(const StorageFile& file_) {
	file = file_;
}

void FilePath::Set(FilePath const &other) {
		file = other.file;
}



void FilePath::Init() {
}

bool FilePath::operator==(const FilePath &other) const {
	return SameNameAs(other);
}

bool FilePath::operator<(const FilePath &other) const {
	return file.Name() < other.file.Name();
}


bool FilePath::SameNameAs(const FilePath &other) const {
	return file== other.file;
}

bool FilePath::IsSet() const {
	return file.Name().size() > 0;
}

bool FilePath::IsUntitled() const {
	if (!file)
		return true;
	std::wstring filename = file.Path().c_str();
	const size_t dirEnd = filename.rfind(pathSepChar);
	return (dirEnd == GUI::gui_string::npos) || (!filename[dirEnd+1]);
}

bool FilePath::IsAbsolute() const {
	if (file.Name().size() == 0)
		return false;

	if (file.Name()[0] == pathSepChar || file.Name()[1] == ':')	// UNC path or drive separator
		return true;


	return false;
}

bool FilePath::IsRoot() const {
	if ((file.Name()[0] == pathSepChar) && (file.Name()[1] == pathSepChar) && (std::wstring(file.Name().c_str()).find(pathSepString, 2) == GUI::gui_string::npos))
        return true; // UNC path like \\server
	return (file.Name().size() == 3) && (file.Name()[1] == ':') && (file.Name()[2] == pathSepChar);

}

int FilePath::RootLength() {
	return 3;
}

const GUI::gui_char *FilePath::AsInternal() const {
	return file.Name().c_str();
}

std::string FilePath::AsUTF8() const {
	return GUI::UTF8FromString(file.Name().c_str());
}

GUI::gui_string FilePath::Name() const {
	return  file.Name().c_str();

}

GUI::gui_string FilePath::BaseName() const {
	return file.DisplayName().c_str();
}

const GUI::gui_char* FilePath::Extension() const {
	return file.FileType().c_str();
}


// Substitute functions that take wchar_t arguments but have the same name
// as char functions so that the compiler will choose the right form.

static size_t strlen(const wchar_t *str) {
	return wcslen(str);
}

static wchar_t *getcwd(wchar_t *buffer, int maxlen) {
	return _wgetcwd(buffer, maxlen);
}

static int chdir(const wchar_t *dirname) {
	return _wchdir(dirname);
}

static FILE *fopen(const wchar_t *filename, const wchar_t *mode) {
	return _wfopen(filename, mode);
}

static int unlink(const wchar_t *filename) {
	return _wunlink(filename);
}

static int access(const wchar_t *path, int mode) {
	return _waccess(path, mode);
}


static int stat(const wchar_t *path, struct _stat64i32 *buffer) {
	return _wstat(path, buffer);
}



void FilePath::List(std::vector<FilePath> &directories, std::vector<FilePath> &files) const {

	//FilePath wildCard(*this, GUI_TEXT("*.*"));
	//WIN32_FIND_DATAW findFileData;
	//HANDLE hFind = ::FindFirstFileW(wildCard.AsInternal(), &findFileData);
	//if (hFind != INVALID_HANDLE_VALUE) {
	//	bool complete = false;
	//	while (!complete) {
	//		const std::wstring_view entryName = findFileData.cFileName;
	//		if ((entryName != currentDirectory) && (entryName != parentDirectory)) {
	//			FilePath pathFull(AsInternal(), findFileData.cFileName);
	//			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
	//				directories.push_back(pathFull);
	//			} else {
	//				files.push_back(pathFull);
	//			}
	//		}
	//		if (!::FindNextFileW(hFind, &findFileData)) {
	//			complete = true;
	//		}
	//	}
	//	::FindClose(hFind);
	//}

	std::sort(files.begin(), files.end());
	std::sort(directories.begin(), directories.end());
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
winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> FilePath::ReadAsync()const {
	std::string propsData;

	winrt::Windows::Storage::Streams::IBuffer buf = co_await winrt::Windows::Storage::FileIO::ReadBufferAsync(file);
	if (buf != nullptr)
	{
		uint8_t* data = GetBufferData(buf);
		propsData.append((char*)data);
	}
	return winrt::to_hstring(propsData);
}
std::string FilePath::Read() const {
	std::string propsData;

	winrt::Windows::Storage::Streams::IBuffer buf = winrt::Windows::Storage::FileIO::ReadBufferAsync(file).get();
	if (buf != nullptr)
	{
		uint8_t* data = GetBufferData(buf);
		propsData.append((char*)data);
	}
	return propsData;
}

void FilePath::Remove() const {
	unlink(AsInternal());
}

#ifndef R_OK
// Microsoft does not define the constants used to call access
#define R_OK 4
#endif

time_t FilePath::ModifiedTime() const {
	if (IsUntitled())
		return 0;
	if (access(AsInternal(), R_OK) == -1)
		return 0;
#ifdef _WIN32
#if defined(_MSC_VER) && (_MSC_VER > 1310)
	struct _stat64i32 statusFile;
#else
	struct _stat statusFile;
#endif
#else
	struct stat statusFile;
#endif
	if (stat(AsInternal(), &statusFile) != -1)
		return statusFile.st_mtime;
	else
		return 0;
}
winrt::Windows::Foundation::IAsyncOperation<long long> FilePath::GetFileLengthAsync() const {
	winrt::Windows::Storage::FileProperties::BasicProperties pro= co_await file.GetBasicPropertiesAsync();
	return pro.Size();
}
long long FilePath::GetFileLength() const {

#ifdef WIN32
	// Using Win32 API as stat variants are complex and there were problems with stat
	// working on XP when compiling with XP compatibility flag.
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if (!GetFileAttributesEx(AsInternal(), GetFileExInfoStandard, &fad))
		return 0;
	LARGE_INTEGER liSze;
	liSze.HighPart = fad.nFileSizeHigh;
	liSze.LowPart = fad.nFileSizeLow;
	return liSze.QuadPart;
#else
	struct stat statusFile;
	if (stat(AsInternal(), &statusFile) != -1)
		return statusFile.st_size;
	return 0;
#endif
}

bool FilePath::Exists() const {
	bool ret = false;
	if (file) {
		ret =file.IsAvailable();
	}
	return ret;
}

bool FilePath::IsDirectory() const {
#ifdef _WIN32
#if defined(_MSC_VER) && (_MSC_VER > 1310)
	struct _stat64i32 statusFile;
#else
	struct _stat statusFile;
#endif
#else
	struct stat statusFile;
#endif
	if (stat(AsInternal(), &statusFile) != -1)
#ifdef WIN32
		return (statusFile.st_mode & _S_IFDIR) != 0;
#else
		return (statusFile.st_mode & S_IFDIR) != 0;
#endif
	else
		return false;
}

namespace {

#ifdef _WIN32
void Lowercase(GUI::gui_string &gs) {

	const int chars = ::LCMapStringEx(LOCALE_NAME_SYSTEM_DEFAULT, LCMAP_LOWERCASE, gs.c_str(), static_cast<int>(gs.size()), NULL, NULL, NULL, NULL, 0);
	std::vector<wchar_t> vc(chars);
	::LCMapStringEx(LOCALE_NAME_SYSTEM_DEFAULT, LCMAP_LOWERCASE, gs.c_str(), static_cast<int>(gs.size()), &vc[0], chars, NULL, 0, 0);

	gs = &vc[0];
}
#endif

bool PatternMatch(GUI::gui_string_view pattern, GUI::gui_string_view text) {
	if (pattern == text) {
		return true;
	} else if (pattern.empty()) {
		return false;
	} else if (pattern.front() == '*') {
		pattern.remove_prefix(1);
		if (pattern.empty()) {
			return true;
		}
		while (!text.empty()) {
			if (PatternMatch(pattern, text)) {
				return true;
			}
			text.remove_prefix(1);
		}
	} else if (text.empty()) {
		return false;
	} else if (pattern.front() == '?') {
		pattern.remove_prefix(1);
		text.remove_prefix(1);
		return PatternMatch(pattern, text);
	} else if (pattern.front() == text.front()) {
		pattern.remove_prefix(1);
		text.remove_prefix(1);
		return PatternMatch(pattern, text);
	}
	return false;
}

}

bool FilePath::Matches(const GUI::gui_char *pattern) const {
	GUI::gui_string pat(pattern);
	GUI::gui_string nameCopy(Name());
#ifdef _WIN32
	Lowercase(pat);
	Lowercase(nameCopy);
#endif
	std::replace(pat.begin(), pat.end(), ' ', '\0');
	size_t start = 0;
	while (start < pat.length()) {
		const GUI::gui_char *patElement = pat.c_str() + start;
		if (PatternMatch(patElement, nameCopy)) {
			return true;
		}
		start += strlen(patElement) + 1;
	}
	return false;
}


void FilePath::FixName() {

}

bool FilePath::CaseSensitive() {

	return false;

}

std::string CommandExecute(const GUI::gui_char *command, const GUI::gui_char *directoryForRun) {
	std::string output;


	return output;
}
