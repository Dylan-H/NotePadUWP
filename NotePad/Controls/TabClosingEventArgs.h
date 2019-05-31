#pragma once

#include "TabClosingEventArgs.g.h"

namespace winrt::NotePad::implementation
{
    struct TabClosingEventArgs : TabClosingEventArgsT<TabClosingEventArgs>
    {
        TabClosingEventArgs() = delete;
		TabClosingEventArgs(Windows::Foundation::IInspectable const& obj, NotePad::TabViewItem const& item) { _Item = obj; _Tab = item; };

		Windows::Foundation::IInspectable Item() { return _Item; };
		NotePad::TabViewItem Tab() { return _Tab; };
		bool Cancel() { return _Cancel; };
		void Cancel(bool value) { _Cancel = value; };
	private:
		Windows::Foundation::IInspectable _Item;
		NotePad::TabViewItem _Tab;
		bool _Cancel;
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct TabClosingEventArgs : TabClosingEventArgsT<TabClosingEventArgs, implementation::TabClosingEventArgs>
    {
    };
}
