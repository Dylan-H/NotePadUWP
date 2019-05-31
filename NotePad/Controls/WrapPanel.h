#pragma once

#include "WrapPanel.g.h"
using namespace winrt::Windows::UI::Xaml;
namespace winrt::NotePad::implementation
{
    struct WrapPanel : WrapPanelT<WrapPanel>
    {
        WrapPanel() = default;

		double HorizontalSpacing() { return winrt::unbox_value<double>(GetValue(_HorizontalSpacingProperty)); };
		void HorizontalSpacing(double value) { SetValue(_HorizontalSpacingProperty, winrt::box_value(value)); };
		double VerticalSpacing() { return winrt::unbox_value<double>(GetValue(_VerticalSpacingProperty)); };
		void VerticalSpacing(double value) { SetValue(_VerticalSpacingProperty, winrt::box_value(value)); };
		Windows::UI::Xaml::Controls::Orientation Orientation() { return winrt::unbox_value<Controls::Orientation>(GetValue(_OrientationProperty)); };
		void Orientation(Windows::UI::Xaml::Controls::Orientation const& value) { SetValue(_OrientationProperty, winrt::box_value(value)); };
		Windows::UI::Xaml::Thickness Padding() { return winrt::unbox_value <Thickness>(GetValue(_PaddingProperty)); };
		void Padding(Windows::UI::Xaml::Thickness const& value) { SetValue(_PaddingProperty, winrt::box_value(value)); };

		static DependencyProperty HorizontalSpacingProperty() { return _HorizontalSpacingProperty; };
		static DependencyProperty VerticalSpacingProperty() { return _VerticalSpacingProperty; };
		static DependencyProperty OrientationProperty() { return _OrientationProperty; };
		static DependencyProperty PaddingProperty() { return _PaddingProperty; };
		static void LayoutPropertyChanged(DependencyObject const &d, DependencyPropertyChangedEventArgs const&e);

		Windows::Foundation::Size MeasureOverride(Windows::Foundation::Size availableSize);
		Windows::Foundation::Size ArrangeOverride(Windows::Foundation::Size finalSize);
	private:
		static DependencyProperty _HorizontalSpacingProperty;
		static DependencyProperty _VerticalSpacingProperty;
		static DependencyProperty _OrientationProperty;
		static DependencyProperty _PaddingProperty;
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct WrapPanel : WrapPanelT<WrapPanel, implementation::WrapPanel>
    {
    };
}
