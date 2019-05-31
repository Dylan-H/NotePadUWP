#pragma once
#include "pch.h"
namespace winrt::NotePad {
	 class ConverterTools
	{
		/// <summary>
		/// Helper method to safely cast an object to a boolean
		/// </summary>
		/// <param name="parameter">Parameter to cast to a boolean</param>
		/// <returns>Bool value or false if cast failed</returns>
	public:
		static bool TryParseBool(winrt::Windows::Foundation::IInspectable parameter)
		{
			auto parsed = false;
			if (parameter != nullptr)
			{
				parsed = winrt::unbox_value<bool>(parameter);
			}

			return parsed;
		}

		/// <summary>
		/// Helper method to convert a value from a source type to a target type.
		/// </summary>
		/// <param name="value">The value to convert</param>
		/// <param name="targetType">The target type</param>
		/// <returns>The converted value</returns>
		 static winrt::Windows::Foundation::IInspectable Convert(winrt::Windows::Foundation::IInspectable value, winrt::Windows::UI::Xaml::Interop::TypeName  targetType)
		{
			return winrt::Windows::UI::Xaml::Markup::XamlBindingHelper::ConvertValue(targetType, value);
		}
	};
}