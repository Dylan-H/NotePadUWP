// SciTE - Scintilla based Text Editor
/** @file ScintillaWindow.h
 ** Interface to a Scintilla instance.
 **/
// Copyright 1998-2018 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLAWINDOW_H
#define SCINTILLAWINDOW_H

namespace GUI {
	typedef void * WindowID;
struct ScintillaFailure {
	sptr_t status;
	explicit ScintillaFailure(sptr_t status_) : status(status_) {
	}
};

class ScintillaWindow : public ScintillaPrimitive {
	//SciFnDirect fn;
	//sptr_t ptr;
	winrt::NotePad::ScintillaControl scictrl{nullptr};
public:
	sptr_t status;
	ScintillaWindow();
	~ScintillaWindow();
	// Deleted so ScintillaWindow objects can not be copied.
	ScintillaWindow(const ScintillaWindow &source) = delete;
	ScintillaWindow(ScintillaWindow &&) = delete;
	ScintillaWindow &operator=(const ScintillaWindow &) = delete;
	ScintillaWindow &operator=(ScintillaWindow &&) = delete;

	void SetScintilla(winrt::NotePad::ScintillaControl& wid_);
	bool CanCall() const;
	int Call(unsigned int msg, uptr_t wParam=0, sptr_t lParam=0);
	sptr_t CallReturnPointer(unsigned int msg, uptr_t wParam=0, sptr_t lParam=0);
	int CallPointer(unsigned int msg, uptr_t wParam, void *s);
	int CallString(unsigned int msg, uptr_t wParam, const char *s);

	// Common APIs made more accessible
	int LineStart(int line);
	int LineFromPosition(int position);
	bool HasFocus() { return true; };
};

}

#endif
