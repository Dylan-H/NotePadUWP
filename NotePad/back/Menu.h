#pragma once
#include "Menu.g.h"
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Xaml::Controls::Primitives;

namespace winrt::NotePad::implementation
{
    struct Menu : MenuT<Menu>
    {
		Menu();

		FlyoutPlacementMode CurrentFlyoutPlacement() { return _CurrentFlyoutPlacement; };
        bool UpdateMenuItemsFlyoutPlacement();
        void CalculateBounds();
		void IsInTransitionState(bool value) { _IsInTransitionState = value; };
		Controls::Orientation Orientation() { return winrt::unbox_value<Windows::UI::Xaml::Controls::Orientation>(GetValue(_OrientationProperty)); };
		void Orientation(Controls::Orientation const& value) { SetValue(_OrientationProperty, winrt::box_value(value)); };
		winrt::Windows::UI::Xaml::Style MenuFlyoutStyle() {return  winrt::unbox_value<winrt::Windows::UI::Xaml::Style>(GetValue(_MenuFlyoutStyleProperty)); };
		void MenuFlyoutStyle(winrt::Windows::UI::Xaml::Style const& value) { SetValue(_MenuFlyoutStyleProperty, winrt::box_value(value)); };
		winrt::Windows::UI::Xaml::Style TooltipStyle() { return winrt::unbox_value <winrt::Windows::UI::Xaml::Style>(GetValue(_TooltipStyleProperty)); };
		void TooltipStyle(winrt::Windows::UI::Xaml::Style const& value) { SetValue(_TooltipStyleProperty, winrt::box_value(value)); };
		PlacementMode TooltipPlacement() { return  winrt::unbox_value <PlacementMode>(GetValue(_TooltipPlacementProperty)); };
		void TooltipPlacement(PlacementMode const& value) { SetValue(_TooltipPlacementProperty, winrt::box_value(value)); };
		NotePad::MenuItem SelectedMenuItem() {return _SelectedMenuItem ;};
		void SelectedMenuItem(NotePad::MenuItem const& value) { _SelectedMenuItem = value; };

		static DependencyProperty OrientationProperty() {return  _OrientationProperty; };
        static DependencyProperty MenuFlyoutStyleProperty() { return  _MenuFlyoutStyleProperty; };
        static DependencyProperty TooltipStyleProperty() { return  _TooltipStyleProperty; };
        static DependencyProperty TooltipPlacementProperty() { return  _TooltipPlacementProperty; };
        static DependencyProperty InputGestureTextProperty() { return  _InputGestureTextProperty; };
		static DependencyProperty AllowTooltipProperty() { return _AllowTooltipProperty; };

		static hstring GetInputGestureText(Windows::UI::Xaml::FrameworkElement const& obj) { return winrt::unbox_value<hstring>(obj.GetValue(_InputGestureTextProperty)); };
		static void SetInputGestureText(Windows::UI::Xaml::FrameworkElement const& obj, hstring const& value) { obj.SetValue(_InputGestureTextProperty, winrt::box_value(value)); };
		static bool GetAllowTooltip(NotePad::Menu const& obj) { return  winrt::unbox_value<bool>(obj.GetValue(_AllowTooltipProperty)); }
		static void SetAllowTooltip(NotePad::Menu const& obj, bool value) { obj.SetValue(_AllowTooltipProperty, winrt::box_value(value)); };

		FlyoutPlacementMode GetMenuFlyoutPlacementMode();
		void OnApplyTemplate();
		DependencyObject GetContainerForItemOverride();
		bool IsItemItsOwnContainerOverride(winrt::Windows::Foundation::IInspectable item);
	private:
		bool AllowTooltip() { return winrt::unbox_value<bool>(GetValue(_AllowTooltipProperty)); };

		static DependencyProperty _OrientationProperty;
		static DependencyProperty _MenuFlyoutStyleProperty;
		static DependencyProperty _TooltipStyleProperty;
		static DependencyProperty _TooltipPlacementProperty;
		static DependencyProperty _InputGestureTextProperty;
		static DependencyProperty _AllowTooltipProperty;
		static Windows::Foundation::Collections::IMap<hstring, DependencyObject> MenuItemInputGestureCache;
		static void RemoveElementFromCache(FrameworkElement const& descendant);
		event_token  Menu_LoadedToken;
		event_token  Menu_LostFocusToken;
		event_token  Menu_UnloadedToken;
		event_token  CoreWindow_PointerMovedToken;
		event_token  CoreWindow_KeyDownToken;
		event_token  Dispatcher_AcceleratorKeyActivatedDownToken;
		event_token Dispatcher_AcceleratorKeyActivatedToken;

		static NotePad::MenuItem GetNextMenuItem(Menu &menu, int moveCount);
		static hstring MapInputToGestureKey(Windows::System::VirtualKey key, bool menuHasFocus = false);
		static bool NavigateUsingKeyboard(KeyEventArgs args, Menu &menu, Controls::Orientation orientation);
		static void InputGestureTextChanged(DependencyObject sender, DependencyPropertyChangedEventArgs const&e);
		static void OrientationPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs const&e);

		void Menu_Loaded(Windows::Foundation::IInspectable const & sender, RoutedEventArgs const& e);
		void Menu_Unloaded(Windows::Foundation::IInspectable const & sender, RoutedEventArgs const& e);
		void CoreWindow_PointerMoved(CoreWindow sender, PointerEventArgs args);

		void CoreWindow_KeyDown(CoreWindow sender, KeyEventArgs args);

		void Dispatcher_AcceleratorKeyActivated(CoreDispatcher sender,AcceleratorKeyEventArgs const&args);

		void Menu_LostFocus(Windows::Foundation::IInspectable const &  sender, RoutedEventArgs const&e);
		void RemoveUnderlineMenuItems();

		void HideMenuItemsTooltips();
		void UnderlineMenuItems();
		void ShowMenuItemsToolTips();
		void HideToolTip();
		FlyoutPlacementMode  _CurrentFlyoutPlacement;

		const unsigned int AltScanCode = 56;
		bool _onlyAltCharacterPressed = true;
		Control _lastFocusElement{ nullptr };
		bool _isLostFocus = true;
		Control _lastFocusElementBeforeMenu{ nullptr };
		Windows::Foundation::Rect _bounds;
		NotePad::MenuItem _SelectedMenuItem{ nullptr };
		bool _IsInTransitionState;
		bool _IsOpened;
		NotePad::WrapPanel _wrapPanel{ nullptr };
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct Menu : MenuT<Menu, implementation::Menu>
    {
    };
}
