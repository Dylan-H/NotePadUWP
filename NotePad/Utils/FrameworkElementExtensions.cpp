#include "pch.h"
#include "FrameworkElementExtensions.h"

namespace winrt::NotePad::implementation
{
	 winrt::event_token FrameworkElementExtensions::FrameworkElement_LoadedToken;

	 DependencyProperty FrameworkElementExtensions::_AncestorTypeProperty =
		 DependencyProperty::RegisterAttached(L"AncestorType", winrt::xaml_typename<Windows::Foundation::IInspectable>(), winrt::xaml_typename<NotePad::FrameworkElementExtensions>(),  PropertyMetadata(nullptr, AncestorType_PropertyChanged));
	 DependencyProperty FrameworkElementExtensions::_AncestorProperty =
		 DependencyProperty::RegisterAttached(L"Ancestor", winrt::xaml_typename<Windows::Foundation::IInspectable>(), winrt::xaml_typename<NotePad::FrameworkElementExtensions>(),  PropertyMetadata(nullptr));
}
