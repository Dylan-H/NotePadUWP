#pragma once

#include "TabDraggedOutsideEventArgs.g.h"

namespace winrt::NotePad::implementation
{
    struct TabDraggedOutsideEventArgs : TabDraggedOutsideEventArgsT<TabDraggedOutsideEventArgs>
    {
        TabDraggedOutsideEventArgs() = delete;
		TabDraggedOutsideEventArgs(Windows::Foundation::IInspectable const& item, NotePad::TabViewItem const& tab) { _Item = item; _Tab = tab; };

		Windows::Foundation::IInspectable Item() { return _Item; };
		void Item(Windows::Foundation::IInspectable const& value) { _Item =value; };
		NotePad::TabViewItem Tab() { return _Tab; };
	private:
		Windows::Foundation::IInspectable  _Item;
		NotePad::TabViewItem _Tab;
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct TabDraggedOutsideEventArgs : TabDraggedOutsideEventArgsT<TabDraggedOutsideEventArgs, implementation::TabDraggedOutsideEventArgs>
    {
    };
}
