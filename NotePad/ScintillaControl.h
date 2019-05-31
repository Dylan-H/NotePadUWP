//
// Declaration of the ScintillaControl class.
//

#pragma once

#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono>
#include <memory>
#include <functional>

#include "Scintilla.h"
#include "Platform.h"
#include "ILoader.h"
#include "ILexer.h"
#include "Position.h"
#include "UniqueString.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "ContractionState.h"
#include "CellBuffer.h"
#include "CallTip.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "LineMarker.h"
#include "Style.h"
#include "AutoComplete.h"
#include "ViewStyle.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "CaseFolder.h"
#include "Document.h"
#include "Selection.h"
#include "PositionCache.h"
#include "EditModel.h"
#include "MarginView.h"
#include "EditView.h"
#include "Editor.h"
#include "ScintillaBase.h"
#include "CaseConvert.h"

#include "winrt/Microsoft.Graphics.Canvas.UI.h"
#include "winrt/Microsoft.Graphics.Canvas.UI.Xaml.h"

#include "ScintillaControl.g.h"

namespace winrt::NotePad::implementation
{
	using namespace  Scintilla;
	using namespace  winrt::Windows::UI::Xaml::Controls;
	using namespace  winrt::Microsoft::Graphics::Canvas::UI::Xaml;
	using namespace  winrt::Windows::UI::Xaml::Controls;
	using namespace  winrt::Windows::UI::Xaml::Controls::Primitives;
	using namespace  winrt::Windows::UI::Text::Core;
    struct ScintillaControl :public ScintillaBase,ScintillaControlT<ScintillaControl>
    {
        ScintillaControl();


        void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);

	private:
		std::chrono::time_point<std::chrono::system_clock>  lasttime;
		bool lastKeyDownConsumed;
		bool capturedMouse;
		unsigned int linesPerScroll = 3;
		ScrollBar mvScrollBar{nullptr};
		ScrollBar mhScrollBar{ nullptr };
		int sysCaretWidth;
		int sysCaretHeight;
		__int64 elapsedTime();
	public:
		explicit ScintillaControl(CanvasControl&  ctrl, ScrollBar&  vSrcollBar, ScrollBar& hScrollBar);
		void EditSetNewText(char* lpstrText, int cbText);
		void Scroll(int wheelDelta, bool bctrl);
		bool CreateSystemCaret();
		bool DestroySystemCaret();

		Scintilla::Point PointMainCaret();

		void RealizeWindowPalette(bool inBackGround);
		void ImeStartComposition();
		void ImeEndComposition();

		void ChangeSize();
		void ChangeScrollPos(int barType, int pos);
		virtual void SetVerticalScrollPos() override;
		virtual void SetHorizontalScrollPos() override;
		virtual bool ModifyScrollBars(Sci::Line nMax, Sci::Line nPage) override;
		virtual void Copy() override;
		virtual void Paste() override;
		virtual void ClaimSelection() override;
		virtual void NotifyChange() override;
		virtual void ScrollText(int linesToMove);
		virtual void NotifyParent(SCNotification scn) override;
		virtual void CopyToClipboard(const SelectionText & selectedText) override;
		virtual void SetMouseCapture(bool on) override;
		virtual bool HaveMouseCapture() override;
		virtual sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) override;
		virtual void Initialise() override;
		virtual void CreateCallTipWindow(PRectangle rc) override;
		virtual void AddToPopUp(const char * label, int cmd = 0, bool enabled = true) override;
		void* timers[tickDwell + 1]{};
		bool FineTickerRunning(TickReason reason) override;
		void FineTickerStart(TickReason reason, int millis, int tolerance) override;
		void FineTickerCancel(TickReason reason) override;


		void setLButtonUp(Scintilla::Point pt, unsigned int curTime, bool ctrl);
		void setLButtonDown(Scintilla::Point pt, unsigned int curTime, bool shift, bool ctrl, bool alt);
		void setRButtonDown(Scintilla::Point pt);
		void setMouseMove(Scintilla::Point pt);
		void setMouseLeave();
		void setFocus();
		void setScrollMessage(ScrollEventType type);
		void setHorizontalScrollMessage(ScrollEventType type);
		void setKeyDown(int key, bool bctrl, bool shift, bool balt);
		void setTextUpdating(const wchar_t *wcs);
		void HandleComposition(const wchar_t *wcs, int len);

		void setCursor(int x, int y);
		void setPaintingAllText(bool b);
		void PaintWin(Surface *surfaceWindow, PRectangle rcArea);


	private:
		CoreTextEditContext _editContext{ nullptr };
		CoreTextRange _textRange;
		std::wstring _text = L"";
		bool bDraw;
		bool bComposition;
		std::map<int, Windows::Foundation::Point> contacts;
	public:
		sptr_t SendMessage(int Msg, uptr_t wParam, sptr_t lParam);
		void setDraw(bool b);
		std::function<void (SCNotification *)> notify;
	private:
		void EditContext_TextUpdating(CoreTextEditContext sender, CoreTextTextUpdatingEventArgs const& args);
		void EditContext_TextRequested(CoreTextEditContext  sender, CoreTextTextRequestedEventArgs const& args);
		void EditContext_SelectionRequested(CoreTextEditContext  sender, CoreTextSelectionRequestedEventArgs const& args);//支持输入
		void EditContext_CompositionStarted(CoreTextEditContext   sender, CoreTextCompositionStartedEventArgs const& args);//IME输入
		void EditContext_CompositionCompleted(CoreTextEditContext  sender, CoreTextCompositionCompletedEventArgs const& args);
		void EditContext_LayoutRequested(CoreTextEditContext sender, CoreTextLayoutRequestedEventArgs const& args);
	public:
		void canvas_Draw(Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl  sender, Microsoft::Graphics::Canvas::UI::Xaml::CanvasDrawEventArgs  args);
		void canvas_PointerWheelChanged(IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e);
		//void UserControl_SizeChanged(IInspectable const& sender, Windows::UI::Xaml::SizeChangedEventArgs const& e);
		void canvas_PointerMoved(IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e);
		void canvas_PointerPressed(IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e);
		void canvas_PointerReleased(IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e);
			  
		void canvas_PointerEntered(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e) ;
		void canvas_KeyDown(IInspectable const& sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e);
		void canvas_GotFocus(IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e);
		void canvas_LostFocus(IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e);
		void canvas_PointerExited(IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e);
		void VerticalScrollBar_Scroll(IInspectable const& sender, Windows::UI::Xaml::Controls::Primitives::ScrollEventArgs const& e);
		void VerticalScrollBar_ValueChanged(IInspectable const& sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
		void Grid_PointerEntered(IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e);
		void Grid_PointerExited(IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e);
		void HorizontalScrollBar_Scroll(IInspectable const& sender, Windows::UI::Xaml::Controls::Primitives::ScrollEventArgs const& e);

    };
}

namespace winrt::NotePad::factory_implementation
{
    struct ScintillaControl : ScintillaControlT<ScintillaControl, implementation::ScintillaControl>
    {
    };
}
