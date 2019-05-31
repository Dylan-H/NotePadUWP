// SciTE - Scintilla based Text Editor
/** @file ScintillaWindow.h
 ** Interface to a Scintilla instance.
 **/
// Copyright 1998-2018 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.
#include "pch.h"
#include <string>

#include "Scintilla.h"
#include "ScintillaControl.h"
#include "GUI.h"
#include "ScintillaWindow.h"

namespace GUI {

ScintillaWindow::ScintillaWindow() : status() {
}

ScintillaWindow::~ScintillaWindow() = default;

void ScintillaWindow::SetScintilla(winrt::NotePad::ScintillaControl& wid_) {
	scictrl = wid_;
}

bool ScintillaWindow::CanCall() const {
	return  scictrl!=nullptr;
}

int ScintillaWindow::Call(unsigned int msg, uptr_t wParam, sptr_t lParam) {
	if (!scictrl)
		return 0;
	switch (msg) {
	case SCI_CREATEDOCUMENT:
	case SCI_CREATELOADER:
	case SCI_PRIVATELEXERCALL:
	case SCI_GETDIRECTFUNCTION:
	case SCI_GETDIRECTPOINTER:
	case SCI_GETDOCPOINTER:
	case SCI_GETCHARACTERPOINTER:
		throw ScintillaFailure(SC_STATUS_FAILURE);
	}
	if (!scictrl)
		throw ScintillaFailure(SC_STATUS_FAILURE);
	const sptr_t retVal = winrt::get_self<winrt::NotePad::implementation::ScintillaControl>(scictrl)->SendMessage( msg, wParam, lParam);
	status = winrt::get_self<winrt::NotePad::implementation::ScintillaControl>(scictrl)->SendMessage( SCI_GETSTATUS, 0, 0);
	if (status > 0 && status < SC_STATUS_WARN_START)
		throw ScintillaFailure(status);
	if (msg == SCI_ADDTEXT)
		scictrl.setDraw(true);
	return static_cast<int>(retVal);
}

sptr_t ScintillaWindow::CallReturnPointer(unsigned int msg, uptr_t wParam, sptr_t lParam) {
	const sptr_t retVal = winrt::get_self<winrt::NotePad::implementation::ScintillaControl>(scictrl)->SendMessage( msg, wParam, lParam);
	status = winrt::get_self<winrt::NotePad::implementation::ScintillaControl>(scictrl)->SendMessage( SCI_GETSTATUS, 0, 0);
	if (status > 0 && status < SC_STATUS_WARN_START)
		throw ScintillaFailure(status);
	return retVal;
}

int ScintillaWindow::CallPointer(unsigned int msg, uptr_t wParam, void *s) {
	return Call(msg, wParam, reinterpret_cast<sptr_t>(s));
}

int ScintillaWindow::CallString(unsigned int msg, uptr_t wParam, const char *s) {
	return Call(msg, wParam, reinterpret_cast<sptr_t>(s));
}

// Common APIs made more accessible
int ScintillaWindow::LineStart(int line) {
	return Call(SCI_POSITIONFROMLINE, line);
}
int ScintillaWindow::LineFromPosition(int position) {
	return Call(SCI_LINEFROMPOSITION, position);
}

}
