#include "ScintillaWinRT.h"
#include "UniConversion.h"
#include<sstream>

#include<windows.foundation.h>
#include<Windows.ApplicationModel.h>
#include<winrt/Windows.System.Threading.h>
#include<winrt/Windows.ApplicationModel.DataTransfer.h>
using namespace winrt;
using namespace winrt::Windows::System::Threading;
using namespace Windows::Foundation;

namespace Scintilla {


	unsigned int CodePageFromCharSet(unsigned long characterSet, unsigned int documentCodePage) {
		if (documentCodePage == SC_CP_UTF8) {
			return SC_CP_UTF8;
		}
		switch (characterSet) {
		case SC_CHARSET_ANSI: return 1252;
		case SC_CHARSET_DEFAULT: return documentCodePage ? documentCodePage : 1252;
		case SC_CHARSET_BALTIC: return 1257;
		case SC_CHARSET_CHINESEBIG5: return 950;
		case SC_CHARSET_EASTEUROPE: return 1250;
		case SC_CHARSET_GB2312: return 936;
		case SC_CHARSET_GREEK: return 1253;
		case SC_CHARSET_HANGUL: return 949;
		case SC_CHARSET_MAC: return 10000;
		case SC_CHARSET_OEM: return 437;
		case SC_CHARSET_RUSSIAN: return 1251;
		case SC_CHARSET_SHIFTJIS: return 932;
		case SC_CHARSET_TURKISH: return 1254;
		case SC_CHARSET_JOHAB: return 1361;
		case SC_CHARSET_HEBREW: return 1255;
		case SC_CHARSET_ARABIC: return 1256;
		case SC_CHARSET_VIETNAMESE: return 1258;
		case SC_CHARSET_THAI: return 874;
		case SC_CHARSET_8859_15: return 28605;
			// Not supported
		case SC_CHARSET_CYRILLIC: return documentCodePage;
		case SC_CHARSET_SYMBOL: return documentCodePage;
		}
		return documentCodePage;
	}

	/** Map the key codes to their equivalent SCK_ form. */
	static int KeyTranslate(int keyIn) {
		//PLATFORM_ASSERT(!keyIn);
		switch (keyIn) {
		case VK_DOWN:		return SCK_DOWN;
		case VK_UP:		return SCK_UP;
		case VK_LEFT:		return SCK_LEFT;
		case VK_RIGHT:		return SCK_RIGHT;
		case VK_HOME:		return SCK_HOME;
		case VK_END:		return SCK_END;
		case VK_PRIOR:		return SCK_PRIOR;
		case VK_NEXT:		return SCK_NEXT;
		case VK_DELETE:	return SCK_DELETE;
		case VK_INSERT:		return SCK_INSERT;
		case VK_ESCAPE:	return SCK_ESCAPE;
		case VK_BACK:		return SCK_BACK;
		case VK_TAB:		return SCK_TAB;
		case VK_RETURN:	return SCK_RETURN;
		case VK_ADD:		return SCK_ADD;
		case VK_SUBTRACT:	return SCK_SUBTRACT;
		case VK_DIVIDE:		return SCK_DIVIDE;
		case VK_LWIN:		return SCK_WIN;
		case VK_RWIN:		return SCK_RWIN;
		case VK_APPS:		return SCK_MENU;
		case VK_OEM_2:		return '/';
		case VK_OEM_3:		return '`';
		case VK_OEM_4:		return '[';
		case VK_OEM_5:		return '\\';
		case VK_OEM_6:		return ']';
		default:			return keyIn;
		}
	}


	__int64 ScintillaWinRT::elapsedTime()
	{
		auto end = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - lasttime);
		lasttime = end;
		return duration.count();
	}

	ScintillaWinRT::ScintillaWinRT(CanvasControl& ctrl, ScrollBar&   vScrollBar, ScrollBar& hScrollBar) {
		wMain = winrt::get_abi(ctrl);
		mvScrollBar = vScrollBar;
		mhScrollBar = hScrollBar;
		lastKeyDownConsumed = false;
		lasttime = std::chrono::system_clock::now();
	}


	void ScintillaWinRT::EditSetNewText(char * lpstrText, int cbText)
	{

	}

	void ScintillaWinRT::ChangeSize() {
		Editor::ChangeSize();
	}

	void ScintillaWinRT::ChangeScrollPos(int barType, int pos) {

		if (barType == 1) {
			DwellEnd(true);
			std::wstringstream wss;
			mvScrollBar.Value( pos);

		}
		else if (barType == 2) {
			// scroll->ScrollToHorizontalOffset(pos);
		}

	}

	void ScintillaWinRT::SetVerticalScrollPos()
	{
		ChangeScrollPos(1, topLine*vs.lineHeight);
		//ChangeScrollPos(1, 3*vs.lineHeight);
	}

	void ScintillaWinRT::SetHorizontalScrollPos()
	{
		//ChangeScrollPos(2, xOffset);
	}

	bool ScintillaWinRT::ModifyScrollBars(int nMax, int nPage)
	{
		bool modified = false;
		mvScrollBar.ViewportSize( mvScrollBar.ActualHeight());
		int vps2 = mvScrollBar.ViewportSize();
		mvScrollBar.Maximum(nMax * vs.lineHeight - vps2);
		mvScrollBar.SmallChange (vs.lineHeight);
		mvScrollBar.LargeChange (LinesToScroll());

		mvScrollBar.IsEnabled (mvScrollBar.ViewportSize() >= 0);

		modified = true;

		PRectangle rcText = GetTextRectangle();
		int horizEndPreferred = scrollWidth;
		if (horizEndPreferred < 0)
			horizEndPreferred = 0;
		unsigned int pageWidth = static_cast<unsigned int>(rcText.Width());
		if (!horizontalScrollBarVisible || Wrapping())
			pageWidth = horizEndPreferred + 1;
		mhScrollBar.ViewportSize (pageWidth);
		mhScrollBar.Maximum(horizEndPreferred);
		return modified;
	}
	using namespace Windows::ApplicationModel;
	using namespace Windows::ApplicationModel::DataTransfer;
	void ScintillaWinRT::Copy()
	{
		if (!sel.Empty()) {
			SelectionText selectedText;
			CopySelectionRange(&selectedText);
			CopyToClipboard(selectedText);
		}

	}

	void ScintillaWinRT::Paste()
	{
		auto dataPackageView = Clipboard::GetContent();

		UndoGroup ug(pdoc);
		bool isLine = SelectionEmpty() && (dataPackageView.Contains(L"MSDEVLineSelect"));
		ClearSelection(multiPasteMode == SC_MULTIPASTE_EACH);
		SelectionPosition selStart = sel.IsRectangular() ?
			sel.Rectangular().Start() :
			sel.Range(sel.Main()).Start();
		bool isRectangular = dataPackageView.Contains(L"MSDEVColumnSelect");
		const PasteShape pasteShape = isRectangular ? pasteRectangular : (isLine ? pasteLine : pasteStream);
		if (dataPackageView.Contains(StandardDataFormats::Text())) {
			hstring text = dataPackageView.GetTextAsync().get();

			int textlen = text.size();
			const wchar_t *uptr = text.c_str();
			if (uptr) {
				unsigned int len;
				char *putf;
				// Default Scintilla behaviour in Unicode mode
				if (IsUnicodeMode()) {
					len = UTF8Length(std::wstring_view( uptr, textlen));
					putf = new char[len + 1];
					UTF8FromUTF16(std::wstring_view (uptr, textlen), putf, len);
				}
				else {
					// CF_UNICODETEXT available, but not in Unicode mode
					// Convert from Unicode to current Scintilla code page
					UINT cpDest = CodePageFromCharSet(vs.styles[STYLE_DEFAULT].characterSet, pdoc->dbcsCodePage);
					len = ::WideCharToMultiByte(cpDest, 0, uptr, -1,
						NULL, 0, NULL, NULL) - 1; // subtract 0 terminator
					putf = new char[len + 1];
					::WideCharToMultiByte(cpDest, 0, uptr, -1,
						putf, len + 1, NULL, NULL);
				}

				InsertPasteShape(&putf[0], len, pasteShape);
				delete[]putf;
			}
		}

		Redraw();
	}

	void ScintillaWinRT::ClaimSelection()
	{
	}

	void ScintillaWinRT::NotifyChange()
	{
	}

	void ScintillaWinRT::ScrollText(int linesToMove)
	{
		ScrollTo(linesToMove);
	}

	void ScintillaWinRT::Scroll(int wheelDelta, bool bctrl)
	{

		if (abs(wheelDelta) >= WHEEL_DELTA && linesPerScroll > 0) {
			int linesToScroll = linesPerScroll;
			if (linesPerScroll == WHEEL_PAGESCROLL)
				linesToScroll = LinesOnScreen() - 1;
			if (linesToScroll == 0) {
				linesToScroll = 1;
			}
			linesToScroll *= (wheelDelta / WHEEL_DELTA);
			if (wheelDelta >= 0)
				wheelDelta = wheelDelta % WHEEL_DELTA;
			else
				wheelDelta = -(-wheelDelta % WHEEL_DELTA);

			if (bctrl) {
				// Zoom! We play with the font sizes in the styles.
				// Number of steps/line is ignored, we just care if sizing up or down
				if (linesToScroll < 0) {
					KeyCommand(SCI_ZOOMIN);
				}
				else {
					KeyCommand(SCI_ZOOMOUT);
				}
			}
			else {
				Editor::ScrollTo(topLine + linesToScroll);
			}
		}
	}

	bool ScintillaWinRT::CreateSystemCaret()
	{

		return false;

	}

	bool ScintillaWinRT::DestroySystemCaret()
	{
		return false;
	}


	Point ScintillaWinRT::PointMainCaret()
	{
		return Editor::PointMainCaret();
	}

	void ScintillaWinRT::RealizeWindowPalette(bool inBackGround)
	{

	}

	void ScintillaWinRT::ImeStartComposition()
	{
		// Digital Mars compiler does not include Imm library
		if (caret.active) {
			// Set font of IME window to same as surrounded text.
			if (stylesValid) {
				// Since the style creation code has been made platform independent,
				// The logfont for the IME is recreated here.
				int styleHere = (pdoc->StyleAt(sel.MainCaret())) & 31;
				LOGFONTA lf = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "" };
				int sizeZoomed = vs.styles[styleHere].size + vs.zoomLevel;
				if (sizeZoomed <= 2)	// Hangs if sizeZoomed <= 1
					sizeZoomed = 2;
				AutoSurface surface(this);
				int deviceHeight = sizeZoomed;
				if (surface) {
					deviceHeight = (sizeZoomed * surface->LogPixelsY()) / 72;
				}
				// The negative is to allow for leading
				lf.lfHeight = -(abs(deviceHeight));
				// lf.lfWeight = vs.styles[styleHere] ? FW_BOLD : FW_NORMAL;
				lf.lfItalic = static_cast<BYTE>(vs.styles[styleHere].italic ? 1 : 0);
				lf.lfCharSet = DEFAULT_CHARSET;
				lf.lfFaceName[0] = '\0';
				if (vs.styles[styleHere].fontName)
					strcpy(lf.lfFaceName, vs.styles[styleHere].fontName);

			}
			// Caret is displayed in IME window. So, caret in Scintilla is useless.
			DropCaret();
		}
	}

	void ScintillaWinRT::ImeEndComposition()
	{
		ShowCaretAtCurrentPosition();

	}

	void ScintillaWinRT::NotifyParent(SCNotification scn)
	{
	}

	void ScintillaWinRT::CopyToClipboard(const SelectionText & selectedText)
	{
		DataPackage dataPackage;
		// 
		wchar_t *buf = nullptr;
		int len = 0;
		// Default Scintilla behaviour in Unicode mode
		if (IsUnicodeMode()) {
			len = UTF16Length(std::string_view(selectedText.Data(), selectedText.Length()));
			buf = new wchar_t[2 * len];
			if (buf) {
				UTF16FromUTF8(std::string_view(selectedText.Data(), selectedText.Length()), buf, len);
			}
		}
		else {
			// Not Unicode mode
			// Convert to Unicode using the current Scintilla code page
			UINT cpSrc = CodePageFromCharSet(
				selectedText.characterSet, selectedText.codePage);
			len = ::MultiByteToWideChar(cpSrc, 0, selectedText.Data(), selectedText.Length(), 0, 0);
			buf = new wchar_t[2 * len];
			::MultiByteToWideChar(cpSrc, 0, selectedText.Data(), selectedText.Length(),
				buf, len);
		}

		if (buf) {
			dataPackage.SetText(hstring(buf, len));

		}
		if (selectedText.rectangular) {
			//dataPackage->SetText(ref new Platform::String(buf, len));
			//::SetClipboardData(cfColumnSelect, 0);
		}

		if (selectedText.lineCopy) {
			//dataPackage->SetText(ref new Platform::String(buf, len));
		   // ::SetClipboardData(cfLineSelect, 0);
		}
		Clipboard::SetContent(dataPackage);
	}

	void ScintillaWinRT::SetMouseCapture(bool on)
	{
		capturedMouse = on;
	}

	bool ScintillaWinRT::HaveMouseCapture()
	{
		return capturedMouse;
	}

	sptr_t ScintillaWinRT::DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam)
	{
		return sptr_t();
	}

	void ScintillaWinRT::Initialise()
	{
	}

	void ScintillaWinRT::CreateCallTipWindow(PRectangle rc)
	{
	}

	void ScintillaWinRT::AddToPopUp(const char * label, int cmd, bool enabled)
	{
	}

	bool ScintillaWinRT::FineTickerRunning(TickReason reason)
	{
		return timers[reason] != 0;
	}

	void ScintillaWinRT::FineTickerStart(TickReason reason, int millis, int tolerance)
	{

		FineTickerCancel(reason);

		ThreadPoolTimer PeriodicTimer = ThreadPoolTimer::CreatePeriodicTimer(Windows::System::Threading::TimerElapsedHandler([this](ThreadPoolTimer const& timer) {
			for (TickReason tr = tickCaret; tr <= tickDwell; tr = static_cast<TickReason>(tr + 1)) {
				if (timers[tr] == winrt::get_abi(timer)) {
					TickFor(tr);
				}
			}
		}),std::chrono::milliseconds{ millis });
		timers[reason] == winrt::get_abi(PeriodicTimer);

		//auto dispatcherTimer = DispatcherTimer();

		//dispatcherTimer.Interval(std::chrono::milliseconds{ millis });;
		//timers[reason] == winrt::get_abi(dispatcherTimer);
		//dispatcherTimer.Tick(Windows::Foundation::EventHandler([](Windows::Foundation::IInspectable   const &sender, Windows::Foundation::IInspectable const&)
		//{
		//	//DispatcherTimer timer = winrt::unbox_value<DispatcherTimer>(sender);
		//	//for (TickReason tr = tickCaret; tr <= tickDwell; tr = static_cast<TickReason>(tr + 1)) {
		//	//	if (timers[tr] == winrt::get_abi(timer)) {
		//	//		TickFor(tr);
		//	//	}
		//	//}
		//}));
		//dispatcherTimer.Start();

	}

	void ScintillaWinRT::FineTickerCancel(TickReason reason)
	{
		if (timers[reason]) {
			ThreadPoolTimer dispatcherTimer{ nullptr };
			winrt::attach_abi(dispatcherTimer, timers[reason]);
			dispatcherTimer.Cancel();
			timers[reason] = 0;
		}
	}

	void ScintillaWinRT::setPaintingAllText(bool b)
	{
		paintingAllText = b;
	}

	void ScintillaWinRT::PaintWin(Surface * surfaceWindow, PRectangle rcArea)
	{
		Paint(surfaceWindow, rcArea);
	}
	void ScintillaWinRT::setLButtonUp(Point pt, unsigned int curTime, bool ctrl)
	{
		ButtonUpWithModifiers(pt, curTime, ctrl);
	}
	void ScintillaWinRT::setLButtonDown(Point pt, unsigned int curTime, bool shift, bool ctrl, bool alt)
	{
		CancelModes();
		Editor::ButtonDownWithModifiers(pt, curTime, ModifierFlags(shift, ctrl, alt));
	}
	void ScintillaWinRT::setRButtonDown(Point pt)
	{
		if (!PointInSelection(pt)) {
			CancelModes();
			SetEmptySelection(PositionFromLocation(pt));
		}
	}
	void ScintillaWinRT::setMouseMove(Point pt)
	{
		ButtonMoveWithModifiers(pt, elapsedTime(),false);
	}
	void ScintillaWinRT::setMouseLeave()
	{
		MouseLeave();
	}
	void ScintillaWinRT::setFocus()
	{
		SetFocusState(true);
		RealizeWindowPalette(false);
		DestroySystemCaret();
		CreateSystemCaret();
	}
	void ScintillaWinRT::setScrollMessage(ScrollEventType type)
	{
		int topLineNew = topLine;
		switch (type) {
		case ScrollEventType::SmallDecrement:
			topLineNew -= 1;
			break;
		case ScrollEventType::SmallIncrement:
			topLineNew += 1;
			break;
		case ScrollEventType::LargeDecrement:
			topLineNew -= LinesToScroll(); break;
		case ScrollEventType::LargeIncrement:
			topLineNew += LinesToScroll(); break;
		case ScrollEventType::First:
			topLineNew = 0; break;
		case ScrollEventType::Last:
			topLineNew = MaxScrollPos(); break;
		case ScrollEventType::ThumbPosition:
		case ScrollEventType::ThumbTrack:
			topLineNew = mvScrollBar.Value() / vs.lineHeight;
			break;
		}
		if (topLineNew < 0)
			topLineNew = 0;
		ScrollTo(topLineNew, false);
	}
	void ScintillaWinRT::setHorizontalScrollMessage(ScrollEventType type)
	{
		int xPos = xOffset;
		PRectangle rcText = GetTextRectangle();
		int pageWidth = rcText.Width() * 2 / 3;
		switch (type) {
		case ScrollEventType::SmallDecrement:
			xPos -= 20;
			break;
		case ScrollEventType::SmallIncrement:
			xPos += 20;
			break;
		case ScrollEventType::LargeDecrement:
			xPos -= pageWidth; break;
		case ScrollEventType::LargeIncrement:
			xPos += pageWidth;
			if (xPos > scrollWidth - rcText.Width()) {	// Hit the end exactly
				xPos = scrollWidth - rcText.Width();
			};
			break;
		case ScrollEventType::First:
			xPos = 0; break;
		case ScrollEventType::Last:
			xPos = scrollWidth; break;
		case ScrollEventType::ThumbPosition:
		case ScrollEventType::ThumbTrack:
			xPos = mhScrollBar.Value() / 20;
			break;
		}
		HorizontalScrollTo(xPos);
	}
	void ScintillaWinRT::setKeyDown(int key, bool bctrl, bool shift, bool balt)
	{
		lastKeyDownConsumed = false;
		int ret = Editor::KeyDownWithModifiers(KeyTranslate(key),ModifierFlags( shift, bctrl, balt), &lastKeyDownConsumed);
	}
	void ScintillaWinRT::setTextUpdating(const wchar_t * wcs)
	{
		if (lastKeyDownConsumed)
			return;
		char utfval[4];
		unsigned int len = UTF8Length(std::wstring_view(wcs, 1));
		UTF8FromUTF16(std::wstring_view(wcs, 1), utfval, len);
		AddCharUTF(utfval, len);
	}
	void ScintillaWinRT::HandleComposition(const wchar_t * wcs, int wlen)
	{
		if (IsUnicodeMode()) {
			char utfval[maxLenInputIME * 3];
			unsigned int len = UTF8Length(std::wstring_view(wcs, wlen));
			UTF8FromUTF16(std::wstring_view(wcs, wlen), utfval, len);
			utfval[len] = '\0';
			AddCharUTF(utfval, len);
		}
	}
	void ScintillaWinRT::setCursor(int x, int y)
	{
		if (PointInSelMargin(Point(x, y))) {
			DisplayCursor(GetMarginCursor(Point(x, y)));
		}
		else if (PointInSelection(Point(x, y)) && !SelectionEmpty()) {
			DisplayCursor(Window::cursorArrow);
		}
		else if (PointIsHotspot(Point(x, y))) {
			DisplayCursor(Window::cursorHand);
		}
		else {
			DisplayCursor(Window::cursorText);
		}
	}
	int ScintillaWinRT::SendMessage(int Msg, unsigned int wParam, unsigned int lParam)
	{
		return ScintillaBase::WndProc(Msg, wParam, lParam);
	}

}