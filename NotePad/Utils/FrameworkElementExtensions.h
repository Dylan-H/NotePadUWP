#pragma once
#include<typeinfo>
#include<string>
#include "FrameworkElementExtensions.g.h"


namespace winrt::NotePad::implementation
{
	using namespace winrt::Windows::UI::Xaml;
    struct FrameworkElementExtensions : FrameworkElementExtensionsT<FrameworkElementExtensions>
    {
        FrameworkElementExtensions() = delete;


		static DependencyProperty AncestorTypeProperty() { return _AncestorTypeProperty; };
		static hstring GetAncestorType(DependencyObject const& obj) { return winrt::unbox_value<hstring>(obj.GetValue(_AncestorTypeProperty)); };
		static void SetAncestorType(DependencyObject const& obj, hstring const& value) {
			obj.SetValue(_AncestorTypeProperty, winrt::box_value(value));
		};
		static Windows::UI::Xaml::DependencyProperty AncestorProperty() { return _AncestorProperty; };
		static Windows::Foundation::IInspectable GetAncestor(Windows::UI::Xaml::DependencyObject const& obj) { return obj.GetValue(_AncestorProperty); };
		static void SetAncestor(Windows::UI::Xaml::DependencyObject const& obj, Windows::Foundation::IInspectable const& value){
			obj.SetValue(_AncestorProperty, value);
		};

		static void AncestorType_PropertyChanged(DependencyObject obj, DependencyPropertyChangedEventArgs args)
		{
			auto fe =obj.try_as<FrameworkElement>();
			if (fe)
			{
				fe.Loaded(FrameworkElement_LoadedToken);

				if (args.NewValue() != nullptr)
				{
					FrameworkElement_LoadedToken =fe.Loaded(RoutedEventHandler(FrameworkElementExtensions::FrameworkElement_Loaded));
					if (fe.Parent() != nullptr)
					{
						FrameworkElement_Loaded(fe, nullptr);
					}
				}
			}
		}
		static Windows::Foundation::IInspectable FindAscendant(DependencyObject element, hstring  type)
		{
			auto  parent = Windows::UI::Xaml::Media::VisualTreeHelper::GetParent(element);

			if (parent == nullptr)
			{
				return nullptr;
			}

			std::string t1=winrt::to_string(type);
			std::string t2 = typeid(parent).name();
			if ( t1==t2)
			{
				return parent;
			}

			if (element.try_as<NotePad::TabView>())
			{
				return parent;
			}

			return FindAscendant(parent,type);
		}
		static void FrameworkElement_Loaded(Windows::Foundation::IInspectable sender, RoutedEventArgs e)
		{
			auto fe = sender.try_as<FrameworkElement>();
			if (fe)
			{
				SetAncestor(fe, FindAscendant(fe,GetAncestorType(fe)));
			}
		}
	private:
		static DependencyProperty _AncestorTypeProperty;
		static DependencyProperty _AncestorProperty;
		static winrt::event_token FrameworkElement_LoadedToken;
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct FrameworkElementExtensions : FrameworkElementExtensionsT<FrameworkElementExtensions, implementation::FrameworkElementExtensions>
    {
    };
}
