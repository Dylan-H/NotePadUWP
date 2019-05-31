#include "pch.h"
#include "BoolToVisibilityConverter.h"
#include "ConverterTools.h"
namespace winrt::NotePad::implementation
{
	DependencyProperty BoolToVisibilityConverter::_TrueValueProperty =
		DependencyProperty::Register(L"TrueValue", winrt::xaml_typename<IInspectable>(), winrt::xaml_typename<NotePad::BoolToVisibilityConverter>(), PropertyMetadata(nullptr));
	DependencyProperty BoolToVisibilityConverter::_FalseValueProperty =
		DependencyProperty::Register(L"FalseValue", winrt::xaml_typename<IInspectable>(), winrt::xaml_typename<NotePad::BoolToVisibilityConverter>(), PropertyMetadata(nullptr));
    

    Windows::Foundation::IInspectable BoolToVisibilityConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
		bool boolValue = winrt::unbox_value<bool>(value);

		// Negate if needed
		if (winrt::NotePad::ConverterTools::TryParseBool(parameter))
		{
			boolValue = !boolValue;
		}

		return winrt::NotePad::ConverterTools::Convert(boolValue ? TrueValue() : FalseValue(), targetType);
    }

    Windows::Foundation::IInspectable BoolToVisibilityConverter::ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
		bool result = value == ConverterTools::Convert(TrueValue(), targetType);

		if (ConverterTools::TryParseBool(parameter))
		{
			result = !result;
		}

		return winrt::box_value(result);
    }

   
}
