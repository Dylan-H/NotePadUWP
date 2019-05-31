#pragma once

#include "MenuItem.g.h"
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::UI::Xaml::Controls::Primitives;
using namespace winrt::Windows::UI::Xaml::Media;
namespace winrt::NotePad::implementation
{
    struct MenuItem : MenuItemT<MenuItem>
    {
		MenuItem() {
			DefaultStyleKey(winrt::box_value(L"NotePad.MenuItem"));
			IsFocusEngagementEnabled(true);
		};

		Windows::Foundation::IInspectable Header() { return GetValue(_HeaderProperty); };
		void Header(Windows::Foundation::IInspectable const& value) { SetValue(_HeaderProperty, value); };
		Windows::UI::Xaml::DataTemplate HeaderTemplate() { return winrt::unbox_value<DataTemplate>(GetValue(_HeaderTemplateProperty)); };
		void HeaderTemplate(Windows::UI::Xaml::DataTemplate const& value) { SetValue(_HeaderTemplateProperty, winrt::box_value(value)); };
		Windows::UI::Xaml::Controls::Orientation Orientation() { return winrt::unbox_value<Controls::Orientation >(GetValue(_OrientationProperty)); };
		void Orientation(Windows::UI::Xaml::Controls::Orientation const& value) { SetValue(_OrientationProperty, winrt::box_value(value)); };

		static DependencyProperty HeaderProperty() { return _HeaderProperty; };
        static DependencyProperty HeaderTemplateProperty() { return _HeaderTemplateProperty; };
        static DependencyProperty OrientationProperty() { return _OrientationProperty; };

		void OnApplyTemplate();
		void UpdateEnabledVisualState();
		void OnHeaderChanged(IInspectable oldValue, IInspectable newValue);
		IIterator<MenuFlyoutItemBase> GetMenuFlyoutItems();
		bool ContainsPoint(Windows::Foundation::Point point);
		void Underline();
		void ShowTooltip();
		void HideTooltip();
		void RemoveUnderline();
		void CalculateBounds();
		NotePad::Menu FindParent(FrameworkElement const & element);
		Controls::MenuFlyout MenuFlyout(){ return _MenuFlyout; };
		void ShowMenu();
		void HideMenu();
		void OnTapped(Input::TappedRoutedEventArgs  e);
		void OnGotFocus(RoutedEventArgs  e);
	private:
		static DependencyProperty _HeaderProperty;
		static DependencyProperty _HeaderTemplateProperty;
		static DependencyProperty _OrientationProperty;

	private:
		winrt::event_token Items_VectorChangedToken;
		winrt::event_token FlyoutButton_PointerExitedToken;
		winrt::event_token MenuFlyout_ClosedToken;
		winrt::event_token MenuFlyout_OpenedToken;
		winrt::event_token MenuItem_IsEnabledChangedToken;

		void InternalHeader(IInspectable value);
		void ReAddItemsToFlyout();
		void AddItemToFlyout(IInspectable item);
		void InsertItemToFlyout(IInspectable item, int index);
		void ShowMenuRepositioned(double menuWidth, double menuHeight);
		void Items_VectorChanged(IObservableVector<IInspectable> sender, IVectorChangedEventArgs const& e);
		void MenuFlyout_Opened(IInspectable const &sender, IInspectable  const &e);
		void MenuFlyout_Closed(IInspectable const & sender, IInspectable  const & e);
		void FlyoutButton_PointerExited(IInspectable const & sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e);
		void MenuItem_IsEnabledChanged(IInspectable const & sender, DependencyPropertyChangedEventArgs const& e);
		static void OnHeaderChanged(DependencyObject d, DependencyPropertyChangedEventArgs const& e);
		void HeaderChanged(IInspectable oldValue, IInspectable newValue);
		void GetMenuFlyoutItemItems(IVector<MenuFlyoutItemBase> menuFlyoutItems, IVector<MenuFlyoutItemBase> allItems);
		static void OnOrientationChanged(DependencyObject d, DependencyPropertyChangedEventArgs const& e);
		hstring RemoveAlt(hstring inputGesture);



		bool _isOpened;
		NotePad::Menu _parentMenu{ nullptr };
		Controls::MenuFlyout _MenuFlyout{ nullptr };
		Button _FlyoutButton{ nullptr };

		const hstring FlyoutButtonName = L"FlyoutButton";
		const char UnderlineCharacter = '^';
		bool _isAccessKeySupported = Windows::Foundation::Metadata::ApiInformation::IsApiContractPresent(L"Windows.Foundation.UniversalApiContract", 3);
		bool _isTextTextDecorationsSupported = Windows::Foundation::Metadata::ApiInformation::IsApiContractPresent(L"Windows.Foundation.UniversalApiContract", 4);
		bool _menuFlyoutRepositioned = false;
		bool _menuFlyoutPlacementChanged = false;
		std::wstring _originalHeader;
		bool _isInternalHeaderUpdate=false;
		Windows::Foundation::Rect _bounds;
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct MenuItem : MenuItemT<MenuItem, implementation::MenuItem>
    {
    };
}
