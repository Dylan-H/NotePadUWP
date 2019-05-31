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

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Microsoft.Graphics.Canvas.UI.h"
#include "winrt/Microsoft.Graphics.Canvas.UI.Xaml.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.ApplicationModel.Activation.h"
#include "winrt/Windows.UI.Xaml.Controls.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Navigation.h"
using namespace winrt;

using namespace Microsoft::Graphics::Canvas::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;


namespace Scintilla {

	 class ScintillaWinRT : public ScintillaBase {

	private:
		std::chrono::time_point<std::chrono::system_clock>  lasttime;
		bool lastKeyDownConsumed;
		bool capturedMouse;
		unsigned int linesPerScroll = 3;
		ScrollBar mvScrollBar;
		ScrollBar mhScrollBar;
		int sysCaretWidth;
		int sysCaretHeight;
		__int64 elapsedTime();
	public:
		explicit ScintillaWinRT(CanvasControl&  ctrl, ScrollBar&  vSrcollBar, ScrollBar& hScrollBar);
		void EditSetNewText(char* lpstrText, int cbText);
		void Scroll(int wheelDelta, bool bctrl);
		bool CreateSystemCaret();
		bool DestroySystemCaret();

		Point PointMainCaret();

		void RealizeWindowPalette(bool inBackGround);
		void ImeStartComposition();
		void ImeEndComposition();


		void ChangeSize();
		void ChangeScrollPos(int barType, int pos);
		virtual void SetVerticalScrollPos() override;
		virtual void SetHorizontalScrollPos() override;
		virtual bool ModifyScrollBars(int nMax, int nPage) override;
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


		void setLButtonUp(Point pt, unsigned int curTime, bool ctrl);
		void setLButtonDown(Point pt, unsigned int curTime, bool shift, bool ctrl, bool alt);
		void setRButtonDown(Point pt);
		void setMouseMove(Point pt);
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
		int SendMessage(int Msg, unsigned int wParam, unsigned int lParam);
	};
}