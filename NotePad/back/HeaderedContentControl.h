#pragma once

#include "HeaderedContentControl.g.h"
using namespace winrt::Windows::UI::Xaml;
namespace winrt::NotePad::implementation
{
    struct HeaderedContentControl : HeaderedContentControlT<HeaderedContentControl>
    {
		HeaderedContentControl();;
		IInspectable Header() { return GetValue(_HeaderProperty); };
		void Header(IInspectable const& value) { SetValue(_HeaderProperty, value); };
		DataTemplate HeaderTemplate() { return winrt::unbox_value<DataTemplate>(GetValue(_HeaderTemplateProperty)); };
		void HeaderTemplate(Windows::UI::Xaml::DataTemplate const& value) { SetValue(_HeaderTemplateProperty, winrt::box_value(value)); };
		Controls::Orientation Orientation() { return winrt::unbox_value<Controls::Orientation >(GetValue(_OrientationProperty)); };
		void Orientation(Controls::Orientation const& value) { SetValue(_OrientationProperty, winrt::box_value(value)); };
		void OnApplyTemplate();
		void OnHeaderChanged(IInspectable const& oldValue,IInspectable const& newValue);


		void SetHeaderVisibility();
		static DependencyProperty HeaderProperty() { return _HeaderProperty; };
		static DependencyProperty HeaderTemplateProperty() { return _HeaderTemplateProperty; };
		static DependencyProperty OrientationProperty() { return _OrientationProperty; };
	private:
		static void OnOrientationChanged(DependencyObject d, DependencyPropertyChangedEventArgs e);

		static void OnHeaderChanged(DependencyObject d, DependencyPropertyChangedEventArgs e);
	private:
		static DependencyProperty _HeaderProperty;
		static DependencyProperty _HeaderTemplateProperty;
		static DependencyProperty _OrientationProperty;
		const hstring PartHeaderPresenter = L"HeaderPresenter";
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct HeaderedContentControl : HeaderedContentControlT<HeaderedContentControl, implementation::HeaderedContentControl>
    {
    };
}
