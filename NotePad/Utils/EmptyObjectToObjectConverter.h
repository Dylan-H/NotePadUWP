#pragma once

#include "EmptyObjectToObjectConverter.g.h"


namespace winrt::NotePad::implementation
{
	using namespace winrt::Windows::UI::Xaml;
	using namespace winrt::Windows::Foundation;
    struct EmptyObjectToObjectConverter : EmptyObjectToObjectConverterT<EmptyObjectToObjectConverter>
    {
        EmptyObjectToObjectConverter() = default;

		IInspectable NotEmptyValue() { { return GetValue(_NotEmptyValueProperty); } };
		void NotEmptyValue(IInspectable const& value) { SetValue(_NotEmptyValueProperty, value); };
        IInspectable EmptyValue() { return GetValue(_EmptyValueProperty); };
        void EmptyValue(IInspectable const& value) { SetValue(_EmptyValueProperty, value); };
        IInspectable Convert(IInspectable const& value, Interop::TypeName const& targetType, IInspectable const& parameter, hstring const& language);
        IInspectable ConvertBack(IInspectable const& value, Interop::TypeName const& targetType, IInspectable const& parameter, hstring const& language);
		virtual bool CheckValueIsEmpty(IInspectable value)
		{
			return value == nullptr;
		}
		static DependencyProperty NotEmptyValueProperty() { return _NotEmptyValueProperty; };
        static DependencyProperty EmptyValueProperty() { return _EmptyValueProperty; };
	private:
		static DependencyProperty _NotEmptyValueProperty;
		static DependencyProperty _EmptyValueProperty;
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct EmptyObjectToObjectConverter : EmptyObjectToObjectConverterT<EmptyObjectToObjectConverter, implementation::EmptyObjectToObjectConverter>
    {
    };
}
