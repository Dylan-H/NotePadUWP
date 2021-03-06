﻿#pragma once
#include "BoolToVisibilityConverter.g.h"


namespace winrt::NotePad::implementation
{
	using namespace winrt::Windows::UI::Xaml;
	using namespace winrt::Windows::Foundation;
    struct BoolToVisibilityConverter : BoolToVisibilityConverterT<BoolToVisibilityConverter>
    {
		BoolToVisibilityConverter() {
			TrueValue(winrt::box_value(Windows::UI::Xaml::Visibility::Visible));
			FalseValue(winrt::box_value(Windows::UI::Xaml::Visibility::Collapsed));
		};

		IInspectable TrueValue() { return  GetValue(_TrueValueProperty); };
		void TrueValue(IInspectable const& value) { SetValue(_TrueValueProperty, value); };
		IInspectable FalseValue() { return  GetValue(_FalseValueProperty); };
		void FalseValue(IInspectable const& value) { SetValue(_FalseValueProperty, value); };
		IInspectable Convert(IInspectable const& value, Interop::TypeName const& targetType, IInspectable const& parameter, hstring const& language);
		IInspectable ConvertBack(IInspectable const& value, Interop::TypeName const& targetType, IInspectable const& parameter, hstring const& language);

		static DependencyProperty TrueValueProperty() { return _TrueValueProperty; };
		static DependencyProperty FalseValueProperty() { return _FalseValueProperty; };
	private:
		static DependencyProperty _TrueValueProperty;
		static DependencyProperty _FalseValueProperty;
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct BoolToVisibilityConverter : BoolToVisibilityConverterT<BoolToVisibilityConverter, implementation::BoolToVisibilityConverter>
    {
    };
}
