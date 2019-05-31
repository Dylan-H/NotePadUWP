#include "pch.h"
#include "BoolToObjectConverter.h"
#include "ConverterTools.h"
namespace winrt::NotePad::implementation
{
	DependencyProperty BoolToObjectConverter::_TrueValueProperty =
		DependencyProperty::Register(L"TrueValue", winrt::xaml_typename<IInspectable>(), winrt::xaml_typename<NotePad::BoolToObjectConverter>(), PropertyMetadata(nullptr));
	DependencyProperty BoolToObjectConverter::_FalseValueProperty =
		DependencyProperty::Register(L"FalseValue", winrt::xaml_typename<IInspectable>(), winrt::xaml_typename<NotePad::BoolToObjectConverter>(), PropertyMetadata(nullptr));



    IInspectable BoolToObjectConverter::Convert(IInspectable const& value, Interop::TypeName const& targetType, IInspectable const& parameter, hstring const& language)
    {
		bool boolValue = winrt::unbox_value<bool>(value);

		// Negate if needed
		if (winrt::NotePad::ConverterTools::TryParseBool(parameter))
		{
			boolValue = !boolValue;
		}

		return winrt::NotePad::ConverterTools::Convert(boolValue ? TrueValue() : FalseValue(), targetType);
    }

    IInspectable BoolToObjectConverter::ConvertBack(IInspectable const& value, Interop::TypeName const& targetType, IInspectable const& parameter, hstring const& language)
    {
		bool result = value == ConverterTools::Convert(TrueValue(), targetType);

		if (ConverterTools::TryParseBool(parameter))
		{
			result = !result;
		}

		return winrt::box_value(result);
    }


}
