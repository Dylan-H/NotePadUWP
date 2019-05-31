#pragma once

#include "DoubleToVisibilityConverter.g.h"


namespace winrt::NotePad::implementation
{
	using namespace winrt::Windows::UI::Xaml;
	using namespace winrt::Windows::Foundation;
    struct DoubleToVisibilityConverter : DoubleToVisibilityConverterT<DoubleToVisibilityConverter>
    {
		DoubleToVisibilityConverter() {
			TrueValue(winrt::box_value(Windows::UI::Xaml::Visibility::Visible));
			FalseValue(winrt::box_value(Windows::UI::Xaml::Visibility::Collapsed));
			NullValue(winrt::box_value(Windows::UI::Xaml::Visibility::Collapsed));
		};
		IInspectable TrueValue() { return  GetValue(_TrueValueProperty); };
		void TrueValue(IInspectable const& value) { SetValue(_TrueValueProperty, value); };
		IInspectable FalseValue() { return  GetValue(_FalseValueProperty); };
		void FalseValue(IInspectable const& value) { SetValue(_FalseValueProperty, value); };

		IInspectable NullValue() { return  GetValue(_NullValueProperty); };
		void NullValue(IInspectable const& value) { SetValue(_NullValueProperty, value); };
		double GreaterThan() { return winrt::unbox_value<double>(GetValue(_GreaterThanProperty)); };
		void GreaterThan(double value) { SetValue(_GreaterThanProperty, winrt::box_value(value)); };
		double LessThan() { return winrt::unbox_value<double>(GetValue(_LessThanProperty)); };
		void LessThan(double value) { SetValue(_LessThanProperty, winrt::box_value(value)); };
		IInspectable Convert(IInspectable const& value, Interop::TypeName const& targetType, IInspectable const& parameter, hstring const& language);
		IInspectable ConvertBack(IInspectable const& value, Interop::TypeName const& targetType, IInspectable const& parameter, hstring const& language);

		static DependencyProperty TrueValueProperty() { return _TrueValueProperty; };
		static DependencyProperty FalseValueProperty() { return _FalseValueProperty; };
		static DependencyProperty NullValueProperty() { return _NullValueProperty; };
		static DependencyProperty GreaterThanProperty() { return _GreaterThanProperty; };
		static DependencyProperty LessThanProperty() { return _LessThanProperty; };
	private:
		static DependencyProperty _TrueValueProperty;
		static DependencyProperty _FalseValueProperty;
		static DependencyProperty _NullValueProperty;
		static DependencyProperty _GreaterThanProperty;
		static DependencyProperty _LessThanProperty;
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct DoubleToVisibilityConverter : DoubleToVisibilityConverterT<DoubleToVisibilityConverter, implementation::DoubleToVisibilityConverter>
    {
    };
}
