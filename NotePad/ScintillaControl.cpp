#include "pch.h"
#include "ScintillaControl.h"
#include "UniConversion.h"
#include<sstream>


using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::System::Threading;
using namespace winrt::Windows::UI::Xaml::Media;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::System;

#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter

namespace winrt::NotePad::implementation
{
	ScintillaControl::ScintillaControl()
	{
		InitializeComponent();
	
		//winrt::copy_to_abi(canvas(), wMain.GetID());
		//wMain = winrt::get_abi(canvas());
		wMain = canvas();

		mvScrollBar = VerticalScrollBar();
		mhScrollBar = HorizontalScrollBar();
		lastKeyDownConsumed = false;
		CoreTextServicesManager manager = CoreTextServicesManager::GetForCurrentView();
		_editContext = manager.CreateEditContext();
		_editContext.InputPaneDisplayPolicy (CoreTextInputPaneDisplayPolicy::Manual);
		_editContext.InputScope ( CoreTextInputScope::Text);
		_editContext.TextUpdating({ this, &ScintillaControl::EditContext_TextUpdating });
		_editContext.TextRequested({ this, &ScintillaControl::EditContext_TextRequested });
		_editContext.SelectionRequested({ this, &ScintillaControl::EditContext_SelectionRequested });
		_editContext.CompositionStarted({ this, &ScintillaControl::EditContext_CompositionStarted });
		_editContext.CompositionCompleted({ this, &ScintillaControl::EditContext_CompositionCompleted });
		_editContext.LayoutRequested ({this, &ScintillaControl::EditContext_LayoutRequested});
		bDraw = false;
	}



	void ScintillaControl::ClickHandler(IInspectable const&, RoutedEventArgs const&)
	{
		Button().Content(box_value(L"Clicked"));
	}

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


	__int64 ScintillaControl::elapsedTime()
	{
		auto end = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - lasttime);
		lasttime = end;
		return duration.count();
	}

	ScintillaControl::ScintillaControl(CanvasControl& ctrl, ScrollBar&   vScrollBar, ScrollBar& hScrollBar) {
		wMain = ctrl; //winrt::get_abi(ctrl);
		mvScrollBar = vScrollBar;
		mhScrollBar = hScrollBar;
		lastKeyDownConsumed = false;
		lasttime = std::chrono::system_clock::now();
	}


	void ScintillaControl::EditSetNewText(char * lpstrText, int cbText)
	{

	}

	void ScintillaControl::ChangeSize() {
		Editor::ChangeSize();
	}

	void ScintillaControl::ChangeScrollPos(int barType, int pos) {

		if (barType == 1) {
			DwellEnd(true);
			std::wstringstream wss;
			mvScrollBar.Value(pos);

		}
		else if (barType == 2) {
			// scroll.ScrollToHorizontalOffset(pos);
		}

	}

	void ScintillaControl::SetVerticalScrollPos()
	{
		ChangeScrollPos(1, topLine*vs.lineHeight);
		//ChangeScrollPos(1, 3*vs.lineHeight);
	}

	void ScintillaControl::SetHorizontalScrollPos()
	{
		//ChangeScrollPos(2, xOffset);
	}

	bool ScintillaControl::ModifyScrollBars(Sci::Line nMax, Sci::Line nPage)
	{
		bool modified = false;
		mvScrollBar.ViewportSize(mvScrollBar.ActualHeight());
		int vps2 = mvScrollBar.ViewportSize();
		mvScrollBar.Maximum(nMax * vs.lineHeight - vps2);
		mvScrollBar.SmallChange(vs.lineHeight);
		mvScrollBar.LargeChange(LinesToScroll());

		mvScrollBar.IsEnabled(mvScrollBar.ViewportSize() >= 0);

		modified = true;

		PRectangle rcText = GetTextRectangle();
		int horizEndPreferred = scrollWidth;
		if (horizEndPreferred < 0)
			horizEndPreferred = 0;
		unsigned int pageWidth = static_cast<unsigned int>(rcText.Width());
		if (!horizontalScrollBarVisible || Wrapping())
			pageWidth = horizEndPreferred + 1;
		mhScrollBar.ViewportSize(pageWidth);
		mhScrollBar.Maximum(horizEndPreferred);
		return modified;
	}
	using namespace Windows::ApplicationModel;
	using namespace Windows::ApplicationModel::DataTransfer;
	void ScintillaControl::Copy()
	{
		if (!sel.Empty()) {
			SelectionText selectedText;
			CopySelectionRange(&selectedText);
			CopyToClipboard(selectedText);
		}

	}

	void ScintillaControl::Paste()
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
					len = UTF8Length(std::wstring_view(uptr, textlen));
					putf = new char[len + 1];
					UTF8FromUTF16(std::wstring_view(uptr, textlen), putf, len);
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

	void ScintillaControl::ClaimSelection()
	{
	}

	void ScintillaControl::NotifyChange()
	{
	}

	void ScintillaControl::ScrollText(int linesToMove)
	{
		//ScrollTo(linesToMove);
		Redraw();
	}

	void ScintillaControl::Scroll(int wheelDelta, bool bctrl)
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

	bool ScintillaControl::CreateSystemCaret()
	{

		return false;

	}

	bool ScintillaControl::DestroySystemCaret()
	{
		return false;
	}


	Scintilla::Point ScintillaControl::PointMainCaret()
	{
		return Editor::PointMainCaret();
	}

	void ScintillaControl::RealizeWindowPalette(bool inBackGround)
	{

	}

	void ScintillaControl::ImeStartComposition()
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

	void ScintillaControl::ImeEndComposition()
	{
		ShowCaretAtCurrentPosition();

	}

	void ScintillaControl::NotifyParent(SCNotification scn)
	{
		if (notify)
			notify(&scn);
	}

	void ScintillaControl::CopyToClipboard(const SelectionText & selectedText)
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

	void ScintillaControl::SetMouseCapture(bool on)
	{
		capturedMouse = on;
	}

	bool ScintillaControl::HaveMouseCapture()
	{
		return capturedMouse;
	}

	sptr_t ScintillaControl::DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam)
	{
		return sptr_t();
	}

	void ScintillaControl::Initialise()
	{
	}

	void ScintillaControl::CreateCallTipWindow(PRectangle rc)
	{
	}

	void ScintillaControl::AddToPopUp(const char * label, int cmd, bool enabled)
	{
	}

	bool ScintillaControl::FineTickerRunning(TickReason reason)
	{
		return timers[reason] != 0;
	}

	void ScintillaControl::FineTickerStart(TickReason reason, int millis, int tolerance)
	{

		FineTickerCancel(reason);

		//ThreadPoolTimer PeriodicTimer = ThreadPoolTimer::CreatePeriodicTimer(Windows::System::Threading::TimerElapsedHandler([this](ThreadPoolTimer const& timer) {
		//	for (TickReason tr = tickCaret; tr <= tickDwell; tr = static_cast<TickReason>(tr + 1)) {
		//		if (timers[tr] == winrt::get_abi(timer)) {
		//			TickFor(tr);
		//		}
		//	}
		//}), std::chrono::milliseconds{ millis });
		//timers[reason] = winrt::get_abi(PeriodicTimer);

		auto dispatcherTimer = DispatcherTimer();

		dispatcherTimer.Interval(std::chrono::milliseconds{ millis });;
		dispatcherTimer.Tick([this](Windows::Foundation::IInspectable const&  sender, Windows::Foundation::IInspectable const& /* e */)
		{
			DispatcherTimer timer = winrt::unbox_value<DispatcherTimer>(sender);
			for (TickReason tr = tickCaret; tr <= tickDwell; tr = static_cast<TickReason>(tr + 1)) {
				if (timers[tr] == winrt::get_abi(timer)) {
					TickFor(tr);
				}
			}
		});

		dispatcherTimer.Start();
		timers[reason] = winrt::detach_abi(dispatcherTimer);
	}

	void ScintillaControl::FineTickerCancel(TickReason reason)
	{
		if (timers[reason]) {
			DispatcherTimer dispatcherTimer{ nullptr };
			winrt::attach_abi(dispatcherTimer, timers[reason]);
			dispatcherTimer.Stop();
			timers[reason] = 0;
		}
	}

	void ScintillaControl::setPaintingAllText(bool b)
	{
		paintingAllText = b;
	}

	void ScintillaControl::PaintWin(Surface * surfaceWindow, PRectangle rcArea)
	{
		Paint(surfaceWindow, rcArea);
	}
	void ScintillaControl::setLButtonUp(Scintilla::Point pt, unsigned int curTime, bool ctrl)
	{
		ButtonUpWithModifiers(pt, curTime, ctrl);
	}
	void ScintillaControl::setLButtonDown(Scintilla::Point pt, unsigned int curTime, bool shift, bool ctrl, bool alt)
	{
		CancelModes();
		Editor::ButtonDownWithModifiers(pt, curTime, ModifierFlags(shift, ctrl, alt));
	}
	void ScintillaControl::setRButtonDown(Scintilla::Point pt)
	{
		if (!PointInSelection(pt)) {
			CancelModes();
			SetEmptySelection(PositionFromLocation(pt));
		}
	}
	void ScintillaControl::setMouseMove(Scintilla::Point pt)
	{
		ButtonMoveWithModifiers(pt, elapsedTime(), false);
	}
	void ScintillaControl::setMouseLeave()
	{
		MouseLeave();
		DisplayCursor(Scintilla::Window::cursorArrow);
	}
	void ScintillaControl::setFocus()
	{
		SetFocusState(true);
		RealizeWindowPalette(false);
		DestroySystemCaret();
		CreateSystemCaret();
	}
	void ScintillaControl::setScrollMessage(ScrollEventType type)
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
	void ScintillaControl::setHorizontalScrollMessage(ScrollEventType type)
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
	void ScintillaControl::setKeyDown(int key, bool bctrl, bool shift, bool balt)
	{
		lastKeyDownConsumed = false;
		int ret = Editor::KeyDownWithModifiers(KeyTranslate(key), ModifierFlags(shift, bctrl, balt), &lastKeyDownConsumed);
	}
	void ScintillaControl::setTextUpdating(const wchar_t * wcs)
	{
		if (lastKeyDownConsumed)
			return;
		char utfval[4];
		unsigned int len = UTF8Length(std::wstring_view(wcs, 1));
		UTF8FromUTF16(std::wstring_view(wcs, 1), utfval, len);
		AddCharUTF(utfval, len);
	}
	void ScintillaControl::HandleComposition(const wchar_t * wcs, int wlen)
	{
		if (IsUnicodeMode()) {
			char utfval[maxLenInputIME * 3];
			unsigned int len = UTF8Length(std::wstring_view(wcs, wlen));
			UTF8FromUTF16(std::wstring_view(wcs, wlen), utfval, len);
			utfval[len] = '\0';
			AddCharUTF(utfval, len);
		}
	}
	void ScintillaControl::setCursor(int x, int y)
	{
		if (PointInSelMargin(Scintilla::Point(x, y))) {
			DisplayCursor(GetMarginCursor(Scintilla::Point(x, y)));
		}
		else if (PointInSelection(Scintilla::Point(x, y)) && !SelectionEmpty()) {
			DisplayCursor(Scintilla::Window::cursorArrow);
		}
		else if (PointIsHotspot(Scintilla::Point(x, y))) {
			DisplayCursor(Scintilla::Window::cursorHand);
		}
		else {
			DisplayCursor(Scintilla::Window::cursorText);
		}
	}
	sptr_t ScintillaControl::SendMessage(int Msg, uptr_t wParam, sptr_t lParam)
	{
		return ScintillaBase::WndProc(Msg, wParam, lParam);
	}


	//////////////////////////////////////////UI///////////////////////////////////////////////////////////////////

	void ScintillaControl::setDraw(bool b)
	{
		bDraw = b;
	}


	void ScintillaControl::EditContext_TextUpdating(CoreTextEditContext  sender, CoreTextTextUpdatingEventArgs const& args)
	{
		OutputDebugString(args.Text().c_str());
		using namespace Windows::System;
		using namespace Windows::UI::Core;
		auto ctrlState = Windows::UI::Core::CoreWindow::GetForCurrentThread().GetKeyState(VirtualKey::Control);
		bool isControlPressed = (ctrlState & Windows::UI::Core::CoreVirtualKeyStates::Down) == Windows::UI::Core::CoreVirtualKeyStates::Down;
		if (!isControlPressed) {
			if (!bComposition) {
				setTextUpdating(args.Text().c_str());
				auto newsel = args.NewSelection();
				newsel.StartCaretPosition = newsel.EndCaretPosition = 0;
			}
			else
			{
				_text =  std::wstring(_text.c_str(), args.Range().StartCaretPosition) +
					args.Text() +  std::wstring(_text.c_str() + std::min((int)_text.length(), args.Range().EndCaretPosition));
				auto newsel = args.NewSelection();
				//newsel.EndCaretPosition = newsel.StartCaretPosition;
			}
		}
	}

	void ScintillaControl::EditContext_TextRequested(CoreTextEditContext  sender, CoreTextTextRequestedEventArgs const& args)
	{

		CoreTextTextRequest  request = args.Request();
		//	auto range = request.Range;
				//args.Request.Text = "";
			//	auto ss = std::wstring(_text.Data() +request.Range.StartCaretPosition, min(request.Range.EndCaretPosition, _text.Length()) - request.Range.StartCaretPosition);
			//	request.Text = ref new Platform::String(ss.c_str());
		request.Text(_text);
		_textRange = request.Range();
		//std::wstringstream wss;
		//wss << request.Text.Data() << ",Range:" << _textRange.StartCaretPosition << "-" << _textRange.EndCaretPosition << std::endl;
		//auto s = wss.str();
		//OutputDebugString(s.c_str());
		//_text = L"";
	}

	void ScintillaControl::EditContext_SelectionRequested(CoreTextEditContext  sender, CoreTextSelectionRequestedEventArgs const& args)
	{

	}

	void ScintillaControl::EditContext_CompositionStarted(CoreTextEditContext  sender, CoreTextCompositionStartedEventArgs const& args)
	{
		bComposition = true;
	}

	void ScintillaControl::EditContext_CompositionCompleted(CoreTextEditContext , CoreTextCompositionCompletedEventArgs const& args)
	{
		bComposition = false;
		auto ss = std::wstring(_text.c_str() + _textRange.StartCaretPosition, std::min(_textRange.EndCaretPosition, (int)_text.length()) - _textRange.StartCaretPosition);
		_textRange.StartCaretPosition = _textRange.EndCaretPosition = 0;
		_editContext.NotifySelectionChanged(_textRange);
		_editContext.NotifyTextChanged(_textRange, 0, _textRange);
		OutputDebugString(ss.c_str());
		HandleComposition(ss.c_str(), ss.length());
		_text = L"";
	}

	void ScintillaControl::EditContext_LayoutRequested(CoreTextEditContext , CoreTextLayoutRequestedEventArgs const& args)
	{
		CoreTextLayoutRequest request = args.Request();

		// Get the screen coordinates of the entire control and the selected text.
		// This information is used to position the IME candidate window.

		// First, get the coordinates of the edit control and the selection
		// relative to the Window.
		GeneralTransform transform = canvas().TransformToVisual(nullptr);
		Windows::Foundation::Point point = transform.TransformPoint(Windows::Foundation::Point());
		Windows::Foundation::Rect contentRect(point, Windows::Foundation::Size(canvas().ActualWidth(), canvas().ActualHeight()));
		Scintilla::Point caretpt = PointMainCaret();
		Windows::Foundation::Rect selectionRect(point.X + caretpt.x, point.Y + caretpt.y, 100, 20);

		// Next, convert to screen coordinates in view pixels.
		Windows::Foundation::Rect windowBounds = Windows::UI::Xaml::Window::Current().CoreWindow().Bounds();
		contentRect.X += windowBounds.X;
		contentRect.Y += windowBounds.Y;
		selectionRect.X += windowBounds.X;
		selectionRect.Y += windowBounds.Y;

		// Finally, scale up to raw pixels.
		//double scaleFactor = DisplayInformation.GetForCurrentView().RawPixelsPerViewPixel;

		//contentRect = ScaleRect(contentRect, scaleFactor);
		//selectionRect = ScaleRect(selectionRect, scaleFactor);

		// This is the bounds of the selection.
		// Note: If you return bounds with 0 width and 0 height, candidates will not appear while typing.

		request.LayoutBounds().TextBounds (selectionRect);

		//This is the bounds of the whole control
		//contentRect.Width = 0;
		//contentRect.Height = 0; 
		request.LayoutBounds().ControlBounds(contentRect);
	}


	void ScintillaControl::canvas_Draw(Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl  sender, Microsoft::Graphics::Canvas::UI::Xaml::CanvasDrawEventArgs  args)
	{
		if (!bDraw)
			return;
		setPaintingAllText(true);
		PRectangle rcPaint = PRectangle::FromInts(0, 0, sender.ActualWidth(), sender.ActualHeight());
		//AutoSurface surfaceWindow(winrt::detach_abi(args.DrawingSession()), this);
		//AutoSurface  surfaceWindow(args.DrawingSession(), this);
		AutoSurface  surfaceWindow(args.DrawingSession(), this);
		if (surfaceWindow) {
			PaintWin(surfaceWindow, rcPaint);
		}
	}


	void ScintillaControl::canvas_PointerWheelChanged(IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e) 
	{
		auto s = std::wstring(L"canvas_PointerWheelChanged\r\n");
		OutputDebugString(s.c_str());

		auto ctrlState = CoreWindow::GetForCurrentThread().GetKeyState(VirtualKey::Control);
		bool isControlPressed = (ctrlState & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down;
		auto value = winrt::unbox_value<Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl>(sender);

		int wheelDelta = e.GetCurrentPoint(value).Properties().MouseWheelDelta();
		Scroll(-wheelDelta, isControlPressed);

		e.Handled(true);
	}


	//void ScintillaControl::UserControl_SizeChanged(IInspectable const& sender, Windows::UI::Xaml::SizeChangedEventArgs const& e)
	//{
	//	if (!bDraw)
	//		return;
	//	ChangeSize();
	//}


	void ScintillaControl::canvas_PointerMoved(IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e)
	{
		auto value = winrt::unbox_value<Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl>(sender);
		auto  pt = e.GetCurrentPoint(value).Position();
		setMouseMove(Scintilla::Point(pt.X, pt.Y));
		e.Handled(true);
	}


	void ScintillaControl::canvas_PointerPressed(IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e)
	{
		auto value = winrt::unbox_value<Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl>(sender);
		auto  ppt = e.GetCurrentPoint(value);
		auto  pt = ppt.Position();
		canvas().Focus(Windows::UI::Xaml::FocusState::Pointer);

		if (ppt.Properties(). IsLeftButtonPressed()) {
			auto ctrlState = CoreWindow::GetForCurrentThread().GetKeyState(VirtualKey::Control);
			bool isControlPressed = (ctrlState & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down;
			auto shiftState = CoreWindow::GetForCurrentThread().GetKeyState(VirtualKey::Shift);
			bool isShiftPressed = (shiftState & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down;
			auto altState = CoreWindow::GetForCurrentThread().GetKeyState(VirtualKey::Menu);//(VirtualKey)18);
			bool isaltPressed = (altState & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down;
			time_t nowtime = time(NULL); //获取当前时间 
			setLButtonDown(Scintilla::Point(pt.X, pt.Y), nowtime, isShiftPressed, isControlPressed, isaltPressed);
			e.Handled (true);
		}
		if (ppt.Properties().IsRightButtonPressed()) { //鼠标右键按下
			setRButtonDown(Scintilla::Point(pt.X, pt.Y));
			e.Handled(true);
		}

	}

	void ScintillaControl::canvas_PointerReleased(IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e)
	{
		//int ptrId = e.GetCurrentPoint((Microsoft::Graphics::Canvas::UI::Xaml::CanvasControlconst&)sender).PointerId;
		//contacts.erase(contacts.find(ptrId));
		auto value = winrt::unbox_value<Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl>(sender);
		auto p = e.GetCurrentPoint(value);
		using namespace Windows::System;
		if (p.Properties().PointerUpdateKind() == Windows::UI::Input::PointerUpdateKind::LeftButtonReleased) {
			auto ctrlState = Windows::UI::Core::CoreWindow::GetForCurrentThread().GetKeyState(VirtualKey::Control);
			bool isControlPressed = (ctrlState & Windows::UI::Core::CoreVirtualKeyStates::Down) == Windows::UI::Core::CoreVirtualKeyStates::Down;
			time_t nowtime = time(NULL); //获取当前时间 
			setLButtonUp(Scintilla::Point(p.Position().X, p.Position().Y), nowtime, isControlPressed);
			e.Handled(true);
		}


	}


	void ScintillaControl::canvas_PointerEntered(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e) 
	{
		auto value = winrt::unbox_value<Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl>(sender);
		Windows::Foundation::Point pt = e.GetCurrentPoint(value).Position();
		setCursor(pt.X, pt.Y);
		//auto s = std::wstring(L"canvas_PointerEntered"); 
		//OutputDebugString(s.c_str());

	}

	void ScintillaControl::canvas_PointerExited(IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e)
	{

		setMouseLeave();
		//	auto s = std::wstring(L"canvas_PointerExited");
			//OutputDebugString(s.c_str());

	}



	void ScintillaControl::canvas_KeyDown(IInspectable const& sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
	{
		using namespace Windows::System;
		using namespace Windows::UI::Core;
		int k = (int)e.Key();
		auto ctrlState = CoreWindow::GetForCurrentThread().GetKeyState(VirtualKey::Control);
		bool isControlPressed = (ctrlState & Windows::UI::Core::CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down;
		auto shiftState = CoreWindow::GetForCurrentThread().GetKeyState(VirtualKey::Shift);
		bool isShiftPressed = (shiftState & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down;
		setKeyDown(k, isControlPressed, isShiftPressed, e.KeyStatus().IsMenuKeyDown);

		switch (e.Key())
		{
		case VirtualKey::Enter:
			e.Handled(true);
			break;
		}

	}

	void ScintillaControl::canvas_GotFocus(IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		//auto s = std::wstring(L"canvas_GotFocus");
		//OutputDebugString(s.c_str());
		setFocus();
		_editContext.NotifyFocusEnter();
	}


	void ScintillaControl::canvas_LostFocus(IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		//auto s = std::wstring(L"canvas_LostFocus");
		//OutputDebugString(s.c_str());
		_editContext.NotifyFocusLeave();

	}


	void ScintillaControl::VerticalScrollBar_Scroll(IInspectable const& sender, Windows::UI::Xaml::Controls::Primitives::ScrollEventArgs const& e)
	{

		//std::wstringstream wss;
		//wss << L"VerticalScrollBar_Scroll:" << (int)e.ScrollEventType << L":" << VerticalScrollBar.Value << ",:" << e.NewValue << std::endl;
		//auto s = wss.str();
		//OutputDebugString(s.c_str());
		setScrollMessage(e.ScrollEventType());

	}


	void ScintillaControl::VerticalScrollBar_ValueChanged(IInspectable const& sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e)
	{
		//std::wstringstream wss;
		//wss << L"VerticalScrollBar_ValueChanged:" << VerticalScrollBar.ViewportSize << L":" << VerticalScrollBar.Value << ",:" << e.NewValue << std::endl;
		//auto s = wss.str();
		//OutputDebugString(s.c_str());
	}


	void ScintillaControl::Grid_PointerEntered(IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e)
	{
		VisualStateManager::GoToState(*this, winrt::hstring(L"MouseIndicator"), true);
	}


	void ScintillaControl::Grid_PointerExited(IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e)
	{
		VisualStateManager::GoToState(*this, winrt::hstring(L"NoIndicator"), true);
	}


	void ScintillaControl::HorizontalScrollBar_Scroll(IInspectable const& sender, Windows::UI::Xaml::Controls::Primitives::ScrollEventArgs const& e)
	{
		setHorizontalScrollMessage(e.ScrollEventType());
	}
}