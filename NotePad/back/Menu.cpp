#include "pch.h"
#include "Menu.h"
#include "MenuItem.h"
#include <sstream>

namespace winrt::NotePad::implementation
{

	DependencyProperty Menu::_OrientationProperty = DependencyProperty::Register(L"Orientation", { winrt::hstring(winrt::name_of<Controls::Orientation>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename<NotePad::Menu>(), PropertyMetadata(winrt::box_value(Orientation::Horizontal), PropertyChangedCallback(&Menu::OrientationPropertyChanged)));
	DependencyProperty Menu::_MenuFlyoutStyleProperty{nullptr};//=DependencyProperty::Register(L"MenuFlyoutStyle", winrt::xaml_typename<Windows::UI::Xaml::Style>(), winrt::xaml_typename <NotePad::Menu>(), PropertyMetadata(winrt::box_value(Menu::_Style)));
	DependencyProperty Menu::_TooltipStyleProperty{ nullptr };//=DependencyProperty::Register(L"TooltipStyle", winrt::xaml_typename<Windows::UI::Xaml::Style>(), winrt::xaml_typename <NotePad::Menu>(), PropertyMetadata(winrt::box_value(Menu::_Style)));
	DependencyProperty Menu::_TooltipPlacementProperty = DependencyProperty::Register(L"TooltipPlacement", winrt::xaml_typename<PlacementMode>(), winrt::xaml_typename<NotePad::Menu>(), PropertyMetadata(winrt::box_value(PlacementMode::Bottom)));
	DependencyProperty Menu::_InputGestureTextProperty = DependencyProperty::RegisterAttached(L"InputGestureText", winrt::xaml_typename<winrt::hstring>(), winrt::xaml_typename <FrameworkElement>(), PropertyMetadata(nullptr, PropertyChangedCallback(&Menu::InputGestureTextChanged)));
	DependencyProperty Menu::_AllowTooltipProperty = DependencyProperty::RegisterAttached(L"AllowTooltip", { winrt::hstring(winrt::name_of<bool>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename<NotePad::Menu>(), PropertyMetadata(winrt::box_value(false)));
	Windows::Foundation::Collections::IMap<hstring, DependencyObject> Menu::MenuItemInputGestureCache{ winrt::single_threaded_map<hstring, DependencyObject>() };
	 Menu::Menu() {
		Menu::_MenuFlyoutStyleProperty = DependencyProperty::Register(L"MenuFlyoutStyle", winrt::xaml_typename<Windows::UI::Xaml::Style>(), winrt::xaml_typename <NotePad::Menu>(), PropertyMetadata(winrt::box_value(winrt::Windows::UI::Xaml::Style())));
		Menu::_TooltipStyleProperty = DependencyProperty::Register(L"TooltipStyle", winrt::xaml_typename<Windows::UI::Xaml::Style>(), winrt::xaml_typename <NotePad::Menu>(), PropertyMetadata(winrt::box_value(winrt::Windows::UI::Xaml::Style())));
		DefaultStyleKey(winrt::box_value(L"NotePad.Menu")); }
	bool Menu::UpdateMenuItemsFlyoutPlacement()
	{
		auto placementMode = GetMenuFlyoutPlacementMode();

		if (placementMode == _CurrentFlyoutPlacement)
		{
			return false;
		}

		_CurrentFlyoutPlacement = placementMode;

		for (unsigned int i = 0; i < Items().Size(); i++)
		{
			NotePad::MenuItem menuItem = Items().GetAt(i).try_as<NotePad::MenuItem >();
			if (menuItem)
			{
				NotePad::implementation::MenuItem* ptr{ winrt::from_abi<NotePad::implementation::MenuItem>(menuItem) };
				ptr->MenuFlyout().Placement(_CurrentFlyoutPlacement);
			}
		}

		return true;
	}
	void Menu::CalculateBounds()
	{
		auto ttv = TransformToVisual(Window::Current().Content());
		Windows::Foundation::Point screenCoords = ttv.TransformPoint(Windows::Foundation::Point(0, 0));
		_bounds.X = screenCoords.X;
		_bounds.Y = screenCoords.Y;
		_bounds.Width = (float)ActualWidth();
		_bounds.Height = (float)ActualHeight();

		for (unsigned int i = 0; i < Items().Size(); i++)
		{
			NotePad::MenuItem menuItem = Items().GetAt(i).try_as<NotePad::MenuItem >();
			if (menuItem)
			{
				NotePad::implementation::MenuItem* ptr{ winrt::from_abi<NotePad::implementation::MenuItem>(menuItem) };
				ptr->CalculateBounds();
			}
		}
	}

	FlyoutPlacementMode Menu::GetMenuFlyoutPlacementMode()
	{
		auto ttv = TransformToVisual(Window::Current().Content());
		auto menuCoords = ttv.TransformPoint(Windows::Foundation::Point(0, 0));

		if (Orientation() == Controls::Orientation::Horizontal)
		{
			auto menuCenter = menuCoords.Y + (ActualHeight() / 2);

			if (menuCenter <= Window::Current().Bounds().Height / 2)
			{
				return FlyoutPlacementMode::Bottom;
			}
			else
			{
				return FlyoutPlacementMode::Top;
			}
		}
		else
		{
			auto menuCenter = menuCoords.X + (ActualWidth() / 2);

			if (menuCenter <= Window::Current().Bounds().Width / 2)
			{
				return FlyoutPlacementMode::Right;
			}
			else
			{
				return FlyoutPlacementMode::Left;
			}
		}
	}

	void Menu::OnApplyTemplate()
	{
		Loaded(Menu_LoadedToken);
		Unloaded(Menu_UnloadedToken);
		Menu_LoadedToken = Loaded(RoutedEventHandler(this, &Menu::Menu_Loaded));
		Menu_UnloadedToken = Unloaded(RoutedEventHandler(this, &Menu::Menu_Unloaded));

		base_type::OnApplyTemplate();
	}

	DependencyObject Menu::GetContainerForItemOverride()
	{
		return NotePad::MenuItem();
	}

	bool Menu::IsItemItsOwnContainerOverride(winrt::Windows::Foundation::IInspectable item)
	{
		auto ret=  winrt::unbox_value<NotePad::MenuItem>(item);
		return ret ? true : false;

	}

	void Menu::RemoveElementFromCache(FrameworkElement const & descendant)
	{
		auto value = descendant.GetValue(InputGestureTextProperty());
		if (!value)
		{
			return;
		}

		auto inputGestureText = winrt::unbox_value<hstring>(value);
		std::wstring upws = inputGestureText.c_str();
		std::transform(upws.begin(), upws.end(), upws.begin(), ::toupper);
		if (!MenuItemInputGestureCache.HasKey(inputGestureText))
		{
			return;
		}

		auto cachedMenuItem = MenuItemInputGestureCache.Lookup(inputGestureText);
		if (cachedMenuItem == descendant)
		{
			MenuItemInputGestureCache.Remove(inputGestureText);
		}
	}

	NotePad::MenuItem Menu::GetNextMenuItem(Menu & menu, int moveCount)
	{
		unsigned int currentMenuItemIndex = 0;
		bool ret = menu.Items().IndexOf(winrt::box_value(menu.SelectedMenuItem()), currentMenuItemIndex);

		auto nextIndex = (currentMenuItemIndex + moveCount + menu.Items().Size()) % menu.Items().Size();
		NotePad::MenuItem nextItem{ menu.Items().GetAt(nextIndex).try_as<NotePad::MenuItem>() };
		if (nextItem)
			nextItem.Focus(FocusState::Keyboard);
		return nextItem;
	}

	hstring Menu::MapInputToGestureKey(Windows::System::VirtualKey key, bool menuHasFocus)
	{
		using namespace Windows::System;
		auto ctrlState = CoreWindow::GetForCurrentThread().GetKeyState(VirtualKey::Control);
		bool isCtrlDown = (ctrlState & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down;
		auto shiftState = CoreWindow::GetForCurrentThread().GetKeyState(VirtualKey::Shift);
		bool isShiftDown = (shiftState & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down;
		auto altState = CoreWindow::GetForCurrentThread().GetKeyState(VirtualKey::Menu);//(VirtualKey)18);
		bool isAltDown = (altState & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down;

		if (!isCtrlDown && !isShiftDown && !isAltDown)
		{
			return L"";;
		}

		std::wstringstream wss;
		winrt::hstring CtrlValue = L"CTRL";
		winrt::hstring ShiftValue = L"SHIFT";
		winrt::hstring AltValue = L"ALT";
		if (isCtrlDown)
		{
			wss << L"CTRL+";
		}

		if (isShiftDown)
		{
			wss << L"SHIFT+";
		}

		if (isAltDown)
		{
			wss << L"ALT+";
		}

		if (key == VirtualKey::None)
		{
			auto s = wss.str();
			return winrt::hstring(s.c_str(), s.length() - 1);
		}
		else
		{
			wss <<(int)key;
			auto s = wss.str();
			return winrt::hstring(s.c_str(), s.length());
		}
	}

	bool Menu::NavigateUsingKeyboard(KeyEventArgs args, Menu & menu, Controls::Orientation orientation)
	{
		using namespace Windows::System;
		auto element = Input::FocusManager::GetFocusedElement();
		MenuFlyoutPresenter mfp = element.try_as< MenuFlyoutPresenter>();
		if (mfp &&
			((args.VirtualKey() == VirtualKey::Down) ||
			(args.VirtualKey() == VirtualKey::Up) ||
				(args.VirtualKey() == VirtualKey::Left) ||
				(args.VirtualKey() == VirtualKey::Right)))
		{
			// Hack to delay and let next element get focus
			Input::FocusManager::FindNextFocusableElement(Input::FocusNavigationDirection::Right);
			return true;
		}

		NotePad::MenuItem menuItem{ element.try_as<NotePad::MenuItem>() };
		if (!menu._IsOpened && menuItem)
		{
			menu.UpdateMenuItemsFlyoutPlacement();

			if (args.VirtualKey() == VirtualKey::Enter ||
				((args.VirtualKey() == VirtualKey::Down) && menu.CurrentFlyoutPlacement() == FlyoutPlacementMode::Bottom) ||
				((args.VirtualKey() == VirtualKey::Up) && menu.CurrentFlyoutPlacement() == FlyoutPlacementMode::Top) ||
				((args.VirtualKey() == VirtualKey::Left) && menu.CurrentFlyoutPlacement() == FlyoutPlacementMode::Left) ||
				((args.VirtualKey() == VirtualKey::Right) && menu.CurrentFlyoutPlacement() == FlyoutPlacementMode::Right))
			{
				menu.SelectedMenuItem().ShowMenu();
				return true;
			}

			if ((args.VirtualKey() == VirtualKey::Left && orientation == Controls::Orientation::Horizontal) ||
				(args.VirtualKey() == VirtualKey::Up && orientation == Controls::Orientation::Vertical))
			{
				GetNextMenuItem(menu, -1);
				return true;
			}

			if ((args.VirtualKey() == VirtualKey::Right && orientation == Controls::Orientation::Horizontal) ||
				(args.VirtualKey() == VirtualKey::Down && orientation == Controls::Orientation::Vertical))
			{
				GetNextMenuItem(menu, +1);
				return true;
			}
		}

		if ((menu.CurrentFlyoutPlacement() == FlyoutPlacementMode::Left &&
			args.VirtualKey() == VirtualKey::Right) ||
			(args.VirtualKey() == VirtualKey::Left &&
				menu.CurrentFlyoutPlacement() != FlyoutPlacementMode::Left))
		{
			if (element.try_as<MenuFlyoutItem>())
			{
				menu.IsInTransitionState(true);
				menu.SelectedMenuItem().HideMenu();
				GetNextMenuItem(menu, -1).ShowMenu();
				return true;
			}
			MenuFlyoutSubItem menuFlyoutSubItem = element.try_as< MenuFlyoutSubItem>();
			if (menuFlyoutSubItem)
			{
				if (menuFlyoutSubItem.Parent().try_as< MenuItem>() && element == menu._lastFocusElement)
				{
					menu.IsInTransitionState(true);
					menu.SelectedMenuItem().HideMenu();
					GetNextMenuItem(menu, -1).ShowMenu();
					return true;
				}
			}
		}

		if ((args.VirtualKey() == VirtualKey::Right &&
			menu.CurrentFlyoutPlacement() != FlyoutPlacementMode::Left) ||
			(args.VirtualKey() == VirtualKey::Left &&
				menu.CurrentFlyoutPlacement() == FlyoutPlacementMode::Left))
		{
			if (element.try_as<MenuFlyoutItem>())
			{
				menu.IsInTransitionState(true);
				menu.SelectedMenuItem().HideMenu();
				GetNextMenuItem(menu, +1).ShowMenu();
				return true;
			}
		}

		return false;
	}

	void Menu::InputGestureTextChanged(DependencyObject sender, DependencyPropertyChangedEventArgs const & e)
	{
		auto element = sender.try_as<FrameworkElement>();
		hstring inputGestureValue;
		if (element)
			inputGestureValue = winrt::unbox_value<hstring>(element.GetValue(InputGestureTextProperty()));
		if (inputGestureValue.empty())
		{
			return;
		}
		std::wstring upws = inputGestureValue.c_str();
		std::transform(upws.begin(), upws.end(), upws.begin(), ::toupper);
		if (MenuItemInputGestureCache.HasKey(inputGestureValue))
		{
			MenuItemInputGestureCache.Insert(inputGestureValue, element);
			return;
		}

		MenuItemInputGestureCache.Insert(inputGestureValue, element);
	}

	void Menu::OrientationPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs const & e)
	{
		if (NotePad::Menu menu{ d.try_as<NotePad::Menu>() })
		{
			NotePad::implementation::Menu* menuptr{ winrt::from_abi<NotePad::implementation::Menu>(menu) };
			if (menuptr->_wrapPanel)
			{
				menuptr->_wrapPanel.Orientation(menuptr->Orientation());
			}
			menuptr->UpdateMenuItemsFlyoutPlacement();
		}

	}

	void Menu::Menu_Loaded(Windows::Foundation::IInspectable const & sender, RoutedEventArgs const & e)
	{
		auto rootpanel = ItemsPanelRoot();
		_wrapPanel = rootpanel.try_as<NotePad::WrapPanel>();
		if (_wrapPanel)
		{
			_wrapPanel.Orientation(Orientation());
		}

		UpdateMenuItemsFlyoutPlacement();

		Window::Current().CoreWindow().PointerMoved(CoreWindow_PointerMovedToken);
		LostFocus(Menu_LostFocusToken);
		Menu_LostFocusToken = LostFocus(RoutedEventHandler(this, &Menu::Menu_LostFocus));
		Dispatcher().AcceleratorKeyActivated(Dispatcher_AcceleratorKeyActivatedToken);
		Window::Current().CoreWindow().KeyDown(CoreWindow_KeyDownToken);
		Dispatcher_AcceleratorKeyActivatedToken = Dispatcher().AcceleratorKeyActivated(Windows::Foundation::TypedEventHandler<CoreDispatcher, AcceleratorKeyEventArgs >(this, &Menu::Dispatcher_AcceleratorKeyActivated));
		CoreWindow_KeyDownToken = Window::Current().CoreWindow().KeyDown(Windows::Foundation::TypedEventHandler<CoreWindow, KeyEventArgs>(this, &Menu::CoreWindow_KeyDown));
		CoreWindow_PointerMovedToken = Window::Current().CoreWindow().PointerMoved(Windows::Foundation::TypedEventHandler<CoreWindow, PointerEventArgs>(this, &Menu::CoreWindow_PointerMoved));
	}

	void Menu::Menu_Unloaded(Windows::Foundation::IInspectable const & sender, RoutedEventArgs const & e)
	{
		Window::Current().CoreWindow().PointerMoved(CoreWindow_PointerMovedToken);
		Dispatcher().AcceleratorKeyActivated(Dispatcher_AcceleratorKeyActivatedToken);
		Window::Current().CoreWindow().KeyDown(CoreWindow_KeyDownToken);

		// Clear Cache
		for (unsigned int i = 0; i < Items().Size(); i++)
		{
			NotePad::MenuItem menuItem = Items().GetAt(i).try_as<NotePad::MenuItem>();
			if (menuItem) {
				NotePad::implementation::MenuItem* ptr{ winrt::from_abi<NotePad::implementation::MenuItem>(menuItem) };
				auto menuFlyoutItems = ptr->GetMenuFlyoutItems();

				winrt::array_view<MenuFlyoutItemBase> itmess;
				menuFlyoutItems.GetMany(itmess);
				while (menuFlyoutItems.MoveNext())
				{
					RemoveElementFromCache(menuFlyoutItems.Current());
				}

				RemoveElementFromCache(menuItem);
			}
		}
	}

	void Menu::CoreWindow_PointerMoved(CoreWindow sender, PointerEventArgs args)
	{
		if (_IsOpened && RectHelper::Contains(_bounds, args.CurrentPoint().Position()))
		{
			// if hover over current opened item
			NotePad::implementation::MenuItem* ptr{ winrt::from_abi<NotePad::implementation::MenuItem>(_SelectedMenuItem) };
			if (ptr->ContainsPoint(args.CurrentPoint().Position()))
			{
				return;
			}

			for (unsigned int i = 0; i < Items().Size(); i++)
			{
				NotePad::MenuItem menuItem = Items().GetAt(i).try_as<NotePad::MenuItem >();
				if (menuItem)
				{
					NotePad::implementation::MenuItem* ptr{ winrt::from_abi<NotePad::implementation::MenuItem>(menuItem) };
					if (ptr->ContainsPoint(args.CurrentPoint().Position())) {
						_SelectedMenuItem.HideMenu();
						menuItem.Focus(FocusState::Keyboard);
						menuItem.ShowMenu();
					}
				}
			}
		}
	}

	void Menu::CoreWindow_KeyDown(CoreWindow sender, KeyEventArgs args)
	{
		if (_IsInTransitionState)
		{
			return;
		}

		if (NavigateUsingKeyboard(args, *this, Orientation()))
		{
			return;
		}

		hstring gestureKey = MapInputToGestureKey(args.VirtualKey());

		if (gestureKey.empty())
		{
			return;
		}
		if (MenuItemInputGestureCache.HasKey(gestureKey))
		{
			auto cachedMenuItem = MenuItemInputGestureCache.Lookup(gestureKey);
			auto menuFlyoutItem = cachedMenuItem.try_as< MenuFlyoutItem>();
			if (menuFlyoutItem&&menuFlyoutItem.Command())
			{
				menuFlyoutItem.Command().Execute(menuFlyoutItem.CommandParameter());
			}
		}
	}

	void Menu::Dispatcher_AcceleratorKeyActivated(CoreDispatcher sender, AcceleratorKeyEventArgs const & args)
	{
		if (Items().Size() == 0)
		{
			return;
		}

		_lastFocusElement = Input::FocusManager::GetFocusedElement().try_as<Control>();

		if (args.KeyStatus().ScanCode != AltScanCode)
		{
			_onlyAltCharacterPressed = false;
		}

		if (args.VirtualKey() == Windows::System::VirtualKey::Menu)
		{
			if (!_IsOpened)
			{
				if (_isLostFocus)
				{
					if (_onlyAltCharacterPressed && args.KeyStatus().IsKeyReleased)
					{
						Items().GetAt(0).try_as< NotePad::MenuItem>().Focus(FocusState::Programmatic);

						if (!(_lastFocusElement.try_as< MenuItem>()))
						{
							_lastFocusElementBeforeMenu = _lastFocusElement;
						}
					}

					if (AllowTooltip())
					{
						ShowMenuItemsToolTips();
					}
					else
					{
						UnderlineMenuItems();
					}

					if (args.KeyStatus().IsKeyReleased)
					{
						_isLostFocus = false;
					}
				}
				else if (args.KeyStatus().IsKeyReleased)
				{
					HideToolTip();
					if (_lastFocusElementBeforeMenu)
						_lastFocusElementBeforeMenu.Focus(FocusState::Keyboard);
				}
			}
		}
		else if ((args.KeyStatus().IsMenuKeyDown || !_isLostFocus) && args.KeyStatus().IsKeyReleased)
		{
			auto gestureKey = MapInputToGestureKey(args.VirtualKey(), !_isLostFocus);
			if (gestureKey.empty())
			{
				return;
			}
			if (MenuItemInputGestureCache.HasKey(gestureKey))
			{
				auto cachedMenuItem = MenuItemInputGestureCache.Lookup(gestureKey);
				NotePad::MenuItem menuItem = { cachedMenuItem.try_as<NotePad::MenuItem>() };
				if (menuItem)
				{
					menuItem.ShowMenu();
					menuItem.Focus(FocusState::Keyboard);
				}
			}
		}

		if (args.KeyStatus().IsKeyReleased && args.EventType() == CoreAcceleratorKeyEventType::KeyUp)
		{
			_onlyAltCharacterPressed = true;
			_isLostFocus = true;
			HideToolTip();
		}
	}

	void Menu::Menu_LostFocus(Windows::Foundation::IInspectable const & sender, RoutedEventArgs const & e)
	{
		auto menuItem = Input::FocusManager::GetFocusedElement().try_as<MenuItem>();

		if (AllowTooltip())
		{
			HideMenuItemsTooltips();
		}

		if (menuItem || _IsOpened)
		{
			return;
		}

		_isLostFocus = true;
		if (!AllowTooltip())
		{
			RemoveUnderlineMenuItems();
		}
	}

	void Menu::RemoveUnderlineMenuItems()
	{
		for (unsigned int i = 0; i < Items().Size(); i++)
		{
			NotePad::MenuItem item = Items().GetAt(i).try_as<NotePad::MenuItem>();
			if (item) {
				NotePad::implementation::MenuItem* ptr{ winrt::from_abi<NotePad::implementation::MenuItem>(item) };
				ptr->RemoveUnderline();
			}
		}
	}

	void Menu::HideMenuItemsTooltips()
	{
		for (unsigned int i = 0; i < Items().Size(); i++)
		{
			NotePad::MenuItem item = Items().GetAt(i).try_as<NotePad::MenuItem>();
			if (item) {
				NotePad::implementation::MenuItem* ptr{ winrt::from_abi<NotePad::implementation::MenuItem>(item) };
				ptr->HideTooltip();
			}
		}
	}

	void Menu::UnderlineMenuItems()
	{
		for (unsigned int i = 0; i < Items().Size(); i++)
		{
			NotePad::MenuItem item = Items().GetAt(i).try_as<NotePad::MenuItem>();
			if (item)
			{
				NotePad::implementation::MenuItem* ptr{ winrt::from_abi<NotePad::implementation::MenuItem>(item) };
				ptr->Underline();
			}
		}
	}

	void Menu::ShowMenuItemsToolTips()
	{
		for (unsigned int i = 0; i < Items().Size(); i++)
		{
			NotePad::MenuItem item = Items().GetAt(i).try_as<NotePad::MenuItem>();
			if (item)
			{
				NotePad::implementation::MenuItem* ptr{ winrt::from_abi<NotePad::implementation::MenuItem>(item) };
				ptr->ShowTooltip();
			}
		}
	}

	void Menu::HideToolTip()
	{
		if (AllowTooltip())
		{
			HideMenuItemsTooltips();
		}
		else
		{
			RemoveUnderlineMenuItems();
		}
	}

}
