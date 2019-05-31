#pragma once

#include "ExpanderEx.g.h"

namespace winrt::NotePad::implementation
{
    struct ExpanderEx : ExpanderExT<ExpanderEx>
    {
		ExpanderEx();;
		IInspectable Header() { return GetValue(_HeaderProperty); };
		void Header(IInspectable const& value) { SetValue(_HeaderProperty, value); };
		bool IsExpanded() { return winrt::unbox_value<bool>(GetValue(_IsExpandedProperty)); };
		void IsExpanded(bool value) {
			SetValue(_IsExpandedProperty, winrt::box_value(value)); };

		UIElement ContentEnd() { return winrt::unbox_value<UIElement>(GetValue(_ContentEndProperty)); };
		void ContentEnd(UIElement const& value) { SetValue(_ContentEndProperty, winrt::box_value(value)); };
		Windows::UI::Xaml::Style HeaderStyle() { return winrt::unbox_value<Windows::UI::Xaml::Style>(GetValue(_HeaderStyleProperty)); };
		void HeaderStyle(Windows::UI::Xaml::Style const& value) { SetValue(_HeaderStyleProperty, winrt::box_value(value)); };

		winrt::event_token Expanded(Windows::Foundation::EventHandler<Windows::Foundation::IInspectable> const& handler) { return _Expanded.add(handler); };
		void Expanded(winrt::event_token const& token) { _Expanded.remove(token); };
		winrt::event_token Collapsed(Windows::Foundation::EventHandler<Windows::Foundation::IInspectable> const& handler) { return _Collapsed.add(handler); };
		static DependencyProperty HeaderProperty() { return _HeaderProperty; };
		void Collapsed(winrt::event_token const& token) { _Collapsed.remove(token); };
		static DependencyProperty IsExpandedProperty() { return _IsExpandedProperty; };
		static DependencyProperty ContentEndProperty() { return _ContentEndProperty; };
		static DependencyProperty HeaderStyleProperty() { return _HeaderStyleProperty; };

		void OnApplyTemplate();
		void ExpandControl();
		void CollapseControl();

	private:
		static void OnHeaderChanged(DependencyObject d, DependencyPropertyChangedEventArgs e);
		static void OnIsExpandedPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e);
		void ExpanderContentGridPart_PointerPressed(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e);
		void ExpanderContentGridPart_PointerReleased(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e);
		void ExpanderContentGridPart_LostFocus(Windows::Foundation::IInspectable const& sender, RoutedEventArgs const& handler);
		
		void ExpanderToggleButtonPart_KeyDown(IInspectable  const& sender, KeyRoutedEventArgs const& e);
	private:
		static DependencyProperty _HeaderProperty;
		static DependencyProperty _IsExpandedProperty;
		static DependencyProperty _ContentEndProperty;
		static DependencyProperty _HeaderStyleProperty;

		winrt::event_token ExpanderToggleButtonPart_KeyDownToken;
		winrt::event<Windows::Foundation::EventHandler<Windows::Foundation::IInspectable>> _Expanded;
		winrt::event<Windows::Foundation::EventHandler<Windows::Foundation::IInspectable>> _Collapsed;

		const hstring PART_ContentGrid = L"PART_ContentGrid";
		const hstring ExpanderToggleButtonPart = L"PART_ExpanderToggleButton";
		const hstring StateContentVisibleDown = L"VisibleDown";
		const hstring StateContentCollapsedDown = L"CollapsedDown";
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct ExpanderEx : ExpanderExT<ExpanderEx, implementation::ExpanderEx>
    {
    };
}
