// SciTE - Scintilla based Text Editor
/** @file FilePath.h
 ** Definition of platform independent base class of editor.
 **/
// Copyright 1998-2005 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.
#pragma once
#include "GUI.h"
extern const GUI::gui_char pathSepString[];
extern const GUI::gui_char pathSepChar;
extern const GUI::gui_char listSepString[];
extern const GUI::gui_char configFileVisibilityString[];
extern const GUI::gui_char fileRead[];
extern const GUI::gui_char fileWrite[];

uint8_t* GetBufferData(winrt::Windows::Storage::Streams::IBuffer& buffer);

using namespace winrt::Windows::Storage;
class FilePath {
	StorageFile file{nullptr};
public:
	FilePath() = default;
	FilePath(const StorageFile &file_);
	FilePath(FilePath const &) = default;
	FilePath(FilePath &&) = default;
	FilePath &operator=(FilePath const &) = default;
	FilePath &operator=(FilePath &&) = default;
	virtual ~FilePath() = default;
	const StorageFile& GetFile() const { return file; };
	void Set(const StorageFile& file_);
	void Set(FilePath const &other);
	virtual void Init();
	bool SameNameAs(const FilePath &other) const;
	bool operator==(const FilePath &other) const;
	bool operator<(const FilePath &other) const;
	bool IsSet() const;
	bool IsUntitled() const;
	bool IsAbsolute() const;
	bool IsRoot() const;
	static int RootLength();
	const GUI::gui_char *AsInternal() const;
	std::string AsUTF8() const;
	GUI::gui_string Name() const;
	GUI::gui_string BaseName() const;
	const GUI::gui_char* Extension() const;
	//FilePath Directory() const;
	void FixName();

	void List(std::vector<FilePath> &directories, std::vector<FilePath> &files) const;

	winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> ReadAsync()const;
	std::string Read() const;
	void Remove() const;
	time_t ModifiedTime() const;
	winrt::Windows::Foundation::IAsyncOperation<long long> GetFileLengthAsync() const;
	long long GetFileLength() const;
	bool Exists() const;
	bool IsDirectory() const;
	bool Matches(const GUI::gui_char *pattern) const;
	static bool CaseSensitive();
};

std::string CommandExecute(const GUI::gui_char *command, const GUI::gui_char *directoryForRun);
