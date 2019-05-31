#pragma once

#include "TabViewItem.g.h"

namespace winrt::NotePad::implementation
{
	using namespace winrt::Windows::UI::Xaml;
	using namespace winrt::Windows::UI::Xaml::Input;
	using namespace winrt::Windows::UI::Xaml::Controls;
	using namespace winrt::Windows::UI::Xaml::Controls::Primitives;
	using namespace winrt::Windows::Devices::Input;
	using namespace winrt::Windows::UI::Input;
	struct TabViewItem : TabViewItemT<TabViewItem>
	{
		TabViewItem() { DefaultStyleKey(winrt::box_value(L"NotePad.TabViewItem")); };

		Windows::Foundation::IInspectable Header() { return GetValue(_HeaderProperty); };
		void Header(Windows::Foundation::IInspectable const& value) {
			SetValue(_HeaderProperty, value); 
		};
		IconElement Icon() { return winrt::unbox_value<IconElement>(GetValue(_IconProperty)); };
		void Icon(IconElement const& value) { SetValue(_IconProperty, winrt::box_value(value)); };
		DataTemplate HeaderTemplate() { 
			auto ret =GetValue(_HeaderTemplateProperty);
			if (ret == nullptr)
				return nullptr;
			return winrt::unbox_value<DataTemplate>(GetValue(_HeaderTemplateProperty));
		};
		void HeaderTemplate(DataTemplate const& value) {
			SetValue(_HeaderTemplateProperty, winrt::box_value(value));
		};
		bool IsClosable() { return winrt::unbox_value<bool>(GetValue(_IsClosableProperty)); };
		void IsClosable(bool value) { SetValue(_IsClosableProperty, winrt::box_value(value)); };

		static DependencyProperty HeaderProperty() { return _HeaderProperty; };
		static DependencyProperty IconProperty() { return _IconProperty; };
		static DependencyProperty HeaderTemplateProperty() { return _HeaderTemplateProperty; };
		static DependencyProperty IsClosableProperty() { return _IsClosableProperty; };

		event_token Closing(Windows::Foundation::EventHandler<Windows::Foundation::IInspectable> const& handler) { return _Closing.add(handler); };
		void Closing(event_token const& token) { _Closing.remove(token); };
		void OnApplyTemplate();
		void OnPointerPressed(PointerRoutedEventArgs e);
		void OnPointerReleased(PointerRoutedEventArgs e);
		
	private:
		void TabCloseButton_Click(Windows::Foundation::IInspectable  sender, RoutedEventArgs e);
	private:
		static DependencyProperty _HeaderProperty;
		static DependencyProperty _IconProperty;
		static DependencyProperty _HeaderTemplateProperty;
		static DependencyProperty _IsClosableProperty;
		winrt::event<Windows::Foundation::EventHandler<Windows::Foundation::IInspectable>> _Closing;
		winrt::event_token TabCloseButton_ClickToken;
		
		const hstring TabCloseButtonName = L"CloseButton";
		ButtonBase _tabCloseButton{ nullptr };
		bool _isMiddleClick;
    };


}

namespace winrt::NotePad::factory_implementation
{
    struct TabViewItem : TabViewItemT<TabViewItem, implementation::TabViewItem>
    {
    };
}