#include "pch.h"
#include "EmptyObjectToObjectConverter.h"
#include "ConverterTools.h"
namespace winrt::NotePad::implementation
{
	DependencyProperty EmptyObjectToObjectConverter::_NotEmptyValueProperty =
		DependencyProperty::Register(L"NotEmptyValue", winrt::xaml_typename<IInspectable>(), winrt::xaml_typename<NotePad::EmptyObjectToObjectConverter>(),  PropertyMetadata(nullptr));
	DependencyProperty EmptyObjectToObjectConverter::_EmptyValueProperty =
		DependencyProperty::Register(L"EmptyValue", winrt::xaml_typename<IInspectable>(), winrt::xaml_typename<NotePad::EmptyObjectToObjectConverter>(),  PropertyMetadata(nullptr));

    IInspectable EmptyObjectToObjectConverter::Convert(IInspectable const& value, Interop::TypeName const& targetType, IInspectable const& parameter, hstring const& language)
    {
		auto isEmpty = CheckValueIsEmpty(value);

		// Negate if needed
		if (winrt::NotePad::ConverterTools::TryParseBool(parameter))
		{
			isEmpty = !isEmpty;
		}

		return winrt::NotePad::ConverterTools::Convert(isEmpty ? EmptyValue() : NotEmptyValue(), targetType);
    }

    IInspectable EmptyObjectToObjectConverter::ConvertBack(IInspectable const& value, Interop::TypeName const& targetType, IInspectable const& parameter, hstring const& language)
    {
        throw hresult_not_implemented();
    }

}
