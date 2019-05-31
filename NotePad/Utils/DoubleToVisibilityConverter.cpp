#include "pch.h"
#include "DoubleToVisibilityConverter.h"
#include "ConverterTools.h"
namespace winrt::NotePad::implementation
{
	DependencyProperty DoubleToVisibilityConverter::_TrueValueProperty =
		DependencyProperty::Register(L"TrueValue", winrt::xaml_typename<IInspectable>(), winrt::xaml_typename <NotePad::DoubleToVisibilityConverter>(), PropertyMetadata(nullptr));
	DependencyProperty DoubleToVisibilityConverter::_FalseValueProperty =
		DependencyProperty::Register(L"FalseValue", winrt::xaml_typename<IInspectable>(), winrt::xaml_typename <NotePad::DoubleToVisibilityConverter>(), PropertyMetadata(nullptr));
	DependencyProperty DoubleToVisibilityConverter::_NullValueProperty =
		DependencyProperty::Register(L"NullValue", winrt::xaml_typename<IInspectable>(), winrt::xaml_typename <NotePad::DoubleToVisibilityConverter>(), PropertyMetadata(nullptr));

	DependencyProperty DoubleToVisibilityConverter::_GreaterThanProperty =
		DependencyProperty::Register(L"GreaterThan", { winrt::hstring(winrt::name_of<double>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename <NotePad::DoubleToVisibilityConverter>(), PropertyMetadata(winrt::box_value(std::numeric_limits<double>::quiet_NaN())));
	DependencyProperty DoubleToVisibilityConverter::_LessThanProperty =
		DependencyProperty::Register(L"LessThan", { winrt::hstring(winrt::name_of<double>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename <NotePad::DoubleToVisibilityConverter>(), PropertyMetadata(winrt::box_value(std::numeric_limits<double>::quiet_NaN())));
    IInspectable DoubleToVisibilityConverter::Convert(IInspectable const& value, Interop::TypeName const& targetType, IInspectable const& parameter, hstring const& language)
    {
		if (value == nullptr)
		{
			return NullValue();
		}

		double vd = winrt::unbox_value<double>(value);


		auto boolValue = false;

		double d_nan = std::numeric_limits<double>::quiet_NaN();
		if (GreaterThan() != d_nan && LessThan() != d_nan &&
			vd > GreaterThan() && vd < LessThan())
		{
			boolValue = true;
		}
		else if (GreaterThan() != d_nan && vd > GreaterThan())
		{
			boolValue = true;
		}
		else if (LessThan() != d_nan && vd < LessThan())
		{
			boolValue = true;
		}

		// Negate if needed
		if (ConverterTools::TryParseBool(parameter))
		{
			boolValue = !boolValue;
		}

		return ConverterTools::Convert(boolValue ? TrueValue() : FalseValue(), targetType);
    }

    IInspectable DoubleToVisibilityConverter::ConvertBack(IInspectable const& value, Interop::TypeName const& targetType, IInspectable const& parameter, hstring const& language)
    {
        throw hresult_not_implemented();
    }

}
