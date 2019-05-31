#include "pch.h"
#include "MenuItem.h"

namespace winrt::NotePad::implementation
{
	DependencyProperty MenuItem::_HeaderProperty = DependencyProperty::Register(L"Header", winrt::xaml_typename<IInspectable>(), winrt::xaml_typename<NotePad::MenuItem>(), PropertyMetadata(nullptr, PropertyChangedCallback(&MenuItem::OnHeaderChanged)));
	DependencyProperty MenuItem::_HeaderTemplateProperty = DependencyProperty::Register(L"HeaderTemplate", winrt::xaml_typename<DataTemplate>(), winrt::xaml_typename<NotePad::MenuItem>(), PropertyMetadata(nullptr));
	DependencyProperty MenuItem::_OrientationProperty = DependencyProperty::Register(L"Orientation", winrt::xaml_typename<Controls::Orientation>(), winrt::xaml_typename<NotePad::MenuItem>(), PropertyMetadata(winrt::box_value(Orientation::Vertical), &MenuItem::OnOrientationChanged));


	void MenuItem::OnApplyTemplate()
	{
		_FlyoutButton = GetTemplateChild(FlyoutButtonName).try_as< Button>();
		_parentMenu = FindParent(*this);
		_isOpened = false;
		if (Items_VectorChangedToken) {
			Items().VectorChanged(Items_VectorChangedToken);
			Items_VectorChangedToken.value = 0;
		}
		if (MenuItem_IsEnabledChangedToken) {
			IsEnabledChanged(MenuItem_IsEnabledChangedToken);
			MenuItem_IsEnabledChangedToken.value = 0;
		}

		if (!_MenuFlyout)
		{
			_MenuFlyout = Windows::UI::Xaml::Controls::MenuFlyout();
		}
		else
		{
			_MenuFlyout.Opened(MenuFlyout_OpenedToken);

			_MenuFlyout.Closed(MenuFlyout_ClosedToken);
		}

		if (_FlyoutButton)
		{
			_FlyoutButton.PointerExited(FlyoutButton_PointerExitedToken);

			// Items().VectorChanged(Items_VectorChangedToken);

			MenuFlyout_OpenedToken = _MenuFlyout.Opened(Windows::Foundation::EventHandler<IInspectable>(this, &MenuItem::MenuFlyout_Opened));
			MenuFlyout_ClosedToken = _MenuFlyout.Closed(Windows::Foundation::EventHandler<IInspectable>(this, &MenuItem::MenuFlyout_Closed));
			FlyoutButton_PointerExitedToken = _FlyoutButton.PointerExited(Windows::UI::Xaml::Input::PointerEventHandler(this, &MenuItem::FlyoutButton_PointerExited));

			_MenuFlyout.MenuFlyoutPresenterStyle(_parentMenu.MenuFlyoutStyle());
			ReAddItemsToFlyout();

			MenuItem_IsEnabledChangedToken = IsEnabledChanged(DependencyPropertyChangedEventHandler(this, &MenuItem::MenuItem_IsEnabledChanged));


			if (_isAccessKeySupported)
			{
				_FlyoutButton.AccessKey(AccessKey());
				AccessKey(hstring(L""));
			}
		}

		if (_parentMenu)
		{
			_MenuFlyout.Placement(_parentMenu.CurrentFlyoutPlacement());
		}

		UpdateEnabledVisualState();

		base_type::OnApplyTemplate();
	}

	void MenuItem::UpdateEnabledVisualState()
	{
		if (IsEnabled())
		{
			VisualStateManager::GoToState(*this, L"Normal", true);
		}
		else
		{
			VisualStateManager::GoToState(*this, L"Disabled", true);
		}
	}
	void MenuItem::OnHeaderChanged(IInspectable oldValue, IInspectable newValue)
	{

		if (_isInternalHeaderUpdate)
		{
			return;
		}

		_originalHeader = L"";

		std::wstring  headerString = winrt::unbox_value<hstring>(newValue).c_str();

		if (headerString.empty())
		{
			return;
		}

		auto underlineCharacterIndex = headerString.find(UnderlineCharacter);

		if (underlineCharacterIndex == std::wstring::npos)
		{
			return;
		}
		headerString.erase(std::remove(headerString.begin(), headerString.end(), '^'), headerString.end());
		if (underlineCharacterIndex == headerString.length() - 1)
		{
			InternalHeader(winrt::box_value(winrt::hstring(headerString)));
			return;
		}

		_originalHeader = headerString;
		InternalHeader(winrt::box_value(winrt::hstring(headerString)));
	}
	IIterator<MenuFlyoutItemBase> MenuItem::GetMenuFlyoutItems()
	{
		IVector<MenuFlyoutItemBase> allItems{ winrt::single_threaded_vector<MenuFlyoutItemBase>() };
		if (_MenuFlyout)
		{
			GetMenuFlyoutItemItems(_MenuFlyout.Items(), allItems);
		}

		return allItems.First();
	}
	bool MenuItem::ContainsPoint(Windows::Foundation::Point point)
	{
		return RectHelper::Contains(_bounds, point);
	}

	void MenuItem::Underline()
	{
		if (_originalHeader.empty())
		{
			return;
		}

		auto underlineCharacterIndex = _originalHeader.find(UnderlineCharacter);

		auto underlinedCharacter = _originalHeader[underlineCharacterIndex + 1];
		auto text = TextBlock();

		auto runWithUnderlinedCharacter = Windows::UI::Xaml::Documents::Run();

		runWithUnderlinedCharacter.Text((wchar_t*)underlinedCharacter);

		if (_isTextTextDecorationsSupported)
		{
			runWithUnderlinedCharacter.TextDecorations(Windows::UI::Text::TextDecorations::Underline);
		}



		auto firstPart = winrt::hstring(_originalHeader.c_str(), underlineCharacterIndex);
		auto secondPart = winrt::hstring(_originalHeader.c_str() + underlineCharacterIndex + 2, _originalHeader.size() - underlineCharacterIndex - 2);

		if (!firstPart.empty())
		{
			Windows::UI::Xaml::Documents::Run temp;
			temp.Text(firstPart);
			text.Inlines().Append(temp);
		}

		text.Inlines().Append(runWithUnderlinedCharacter);

		if (!secondPart.empty())
		{
			Windows::UI::Xaml::Documents::Run temp;
			temp.Text(secondPart);
			text.Inlines().Append(temp);
		}

		InternalHeader(text);
	}

	void MenuItem::ShowTooltip()
	{
		auto inputGestureText = winrt::unbox_value<hstring>(GetValue(Menu::InputGestureTextProperty()));
		if (inputGestureText.empty())
		{
			return;
		}

		auto tooltip = ToolTipService::GetToolTip(_FlyoutButton).try_as<ToolTip>();
		if (tooltip)
		{
			ToolTip tooltip;
			tooltip.Style(_parentMenu.TooltipStyle());
			ToolTipService::SetToolTip(_FlyoutButton, winrt::box_value(tooltip));
		}

		tooltip.Placement(_parentMenu.TooltipPlacement());
		tooltip.Content(winrt::box_value(RemoveAlt(inputGestureText)));
		tooltip.IsOpen(true);
		tooltip.IsEnabled(true);
	}

	void MenuItem::HideTooltip()
	{
		auto tp = ToolTipService::GetToolTip(_FlyoutButton);
		if (tp) {
			auto tooltip = winrt::unbox_value<ToolTip>(tp);
			if (tooltip)
			{
				tooltip.IsOpen(false);
				tooltip.IsEnabled(false);
			}
		}
	}

	void MenuItem::RemoveUnderline()
	{
		if (!_originalHeader.empty())
		{
			_originalHeader.erase(std::remove(_originalHeader.begin(), _originalHeader.end(), '^'), _originalHeader.end());
			InternalHeader(winrt::box_value(winrt::hstring(_originalHeader.c_str())));
		}
	}

	void MenuItem::CalculateBounds()
	{
		auto ttv = TransformToVisual(Window::Current().Content());
		Windows::Foundation::Point screenCoords = ttv.TransformPoint(Windows::Foundation::Point(0, 0));
		_bounds.X = screenCoords.X;
		_bounds.Y = screenCoords.Y;
		_bounds.Width = ActualWidth();
		_bounds.Height = ActualHeight();
	}

	void MenuItem::InternalHeader(IInspectable value)
	{
		_isInternalHeaderUpdate = true;
		Header(value);
		_isInternalHeaderUpdate = false;
	}


	void MenuItem::ReAddItemsToFlyout()
	{
		if (!_MenuFlyout)
		{
			return;
		}
		_MenuFlyout.Items().Clear();
		for (unsigned int i = 0; i < Items().Size(); i++)
		{
			auto item = Items().GetAt(i);
			AddItemToFlyout(item);
		}
	}

	void MenuItem::AddItemToFlyout(IInspectable item)
	{
		auto menuItem = winrt::unbox_value<MenuFlyoutItemBase>(item);
		if (menuItem)
		{
			_MenuFlyout.Items().Append(menuItem);
		}
		else
		{
			MenuFlyoutItem newMenuItem;
			newMenuItem.DataContext(item);
			_MenuFlyout.Items().Append(newMenuItem);
		}
	}
	void MenuItem::InsertItemToFlyout(IInspectable item, int index)
	{
		auto menuItem = winrt::unbox_value<MenuFlyoutItemBase>(item);
		if (menuItem)
		{
			_MenuFlyout.Items().InsertAt(index, menuItem);
		}
		else
		{
			MenuFlyoutItem newMenuItem;
			newMenuItem.DataContext(item);
			_MenuFlyout.Items().InsertAt(index, newMenuItem);
		}
	}

	void MenuItem::ShowMenuRepositioned(double menuWidth, double menuHeight)
	{
		if (!IsEnabled() || _MenuFlyout.Items().Size() == 0)
		{
			return;
		}

		_menuFlyoutRepositioned = true;
		Windows::Foundation::Point location;
		if (_MenuFlyout.Placement() == Primitives::FlyoutPlacementMode::Bottom)
		{
			location = Windows::Foundation::Point(_FlyoutButton.ActualWidth() - menuWidth, _FlyoutButton.ActualHeight());
		}
		else if (_MenuFlyout.Placement() == Primitives::FlyoutPlacementMode::Right)
		{
			location = Windows::Foundation::Point(_FlyoutButton.ActualWidth(), _FlyoutButton.ActualHeight() - menuHeight);
		}
		else
		{
			// let the flyout decide where to show
			_MenuFlyout.ShowAt(_FlyoutButton);
			return;
		}

		MenuFlyout().ShowAt(_FlyoutButton, location);
	}
	NotePad::Menu MenuItem::FindParent(FrameworkElement const & element)
	{
		if (!element.Parent())
		{
			return NotePad::Menu{ nullptr };
		}
		NotePad::Menu menu = { element.Parent().try_as<NotePad::Menu>() };

		if (menu)
		{
			return menu;
		}

		return FindParent(element.Parent().try_as<FrameworkElement>());
	}

	void MenuItem::Items_VectorChanged(IObservableVector<IInspectable> sender, IVectorChangedEventArgs const & e)
	{
		auto index = (int)e.Index();
		switch (e.CollectionChange())
		{
		case CollectionChange::Reset:
			ReAddItemsToFlyout();
			break;
		case CollectionChange::ItemInserted:
			InsertItemToFlyout(sender.GetAt(index), index);
			break;
		case CollectionChange::ItemRemoved:
			_MenuFlyout.Items().RemoveAt(index);
			break;
		case CollectionChange::ItemChanged:
			_MenuFlyout.Items().RemoveAt(index);
			InsertItemToFlyout(sender.GetAt(index), index);
			break;
		}
	}
	void MenuItem::MenuFlyout_Opened(IInspectable const & sender, IInspectable const & e)
	{
		if (_parentMenu.UpdateMenuItemsFlyoutPlacement() && !_menuFlyoutPlacementChanged)
		{
			_menuFlyoutPlacementChanged = true;
			ShowMenu();
		}

		_parentMenu.CalculateBounds();
		_isOpened = true;
		VisualStateManager::GoToState(*this, L"Opened", true);
		_parentMenu.IsInTransitionState(false);

		if (!_menuFlyoutRepositioned)
		{
			Popup popup{ nullptr };
			auto popuplist = VisualTreeHelper::GetOpenPopups(Window::Current());
			for (unsigned int i = 0; i < popuplist.Size(); i++) {
				auto pop = popuplist.GetAt(i);
				MenuFlyoutPresenter p = pop.Child().try_as<MenuFlyoutPresenter>();
				if (p) {
					popup = pop;
					break;
				}
			}

			if (popup)
			{
				auto mfp = popup.Child().try_as<MenuFlyoutPresenter>();
				auto height = mfp.ActualHeight();
				auto width = mfp.ActualWidth();

				auto flytoutButtonPoint = _FlyoutButton.TransformToVisual(Window::Current().Content()).TransformPoint(Windows::Foundation::Point(0, 0));

				if ((width > Window::Current().Bounds().Width - flytoutButtonPoint.X &&
					(_MenuFlyout.Placement() == Primitives::FlyoutPlacementMode::Bottom)) ||
					(height > Window::Current().Bounds().Height - flytoutButtonPoint.Y &&
					(_MenuFlyout.Placement() == Primitives::FlyoutPlacementMode::Right)))
				{
					ShowMenuRepositioned(width, height);
				}
			}
		}
	}
	inline void MenuItem::MenuFlyout_Closed(IInspectable const & sender, IInspectable const & e)
	{
		_isOpened = false;
		_menuFlyoutRepositioned = false;
		_menuFlyoutPlacementChanged = false;
		VisualStateManager::GoToState(*this, L"Normal", true);
	}
	void MenuItem::FlyoutButton_PointerExited(IInspectable const & sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const & e)
	{
		if (_isOpened)
		{
			VisualStateManager::GoToState(*this, L"Opened", true);
		}

	}
	void MenuItem::MenuItem_IsEnabledChanged(IInspectable const & sender, DependencyPropertyChangedEventArgs const & e)
	{
		if (NotePad::MenuItem menuItemControl{ sender.try_as<NotePad::MenuItem>() })
		{
			NotePad::implementation::MenuItem* ptr{ winrt::from_abi<NotePad::implementation::MenuItem>(menuItemControl) };
			ptr->UpdateEnabledVisualState();
		}
	}
	void MenuItem::OnHeaderChanged(DependencyObject d, DependencyPropertyChangedEventArgs const & e)
	{
		if (NotePad::MenuItem menuItemControl{ d.try_as<NotePad::MenuItem>() })
		{
			NotePad::implementation::MenuItem* ptr{ winrt::from_abi<NotePad::implementation::MenuItem>(menuItemControl) };
			ptr->HeaderChanged(e.OldValue(), e.NewValue());
		}
	}
	void MenuItem::HeaderChanged(IInspectable oldValue, IInspectable newValue)
	{
		if (_isInternalHeaderUpdate)
		{
			return;
		}

		_originalHeader = L"";

		std::wstring  headerString = winrt::unbox_value<hstring>(newValue).c_str();

		if (headerString.empty())
		{
			return;
		}

		auto underlineCharacterIndex = headerString.find(UnderlineCharacter);

		if (underlineCharacterIndex == std::wstring::npos)
		{
			return;
		}
		headerString.erase(std::remove(headerString.begin(), headerString.end(), '^'), headerString.end());
		if (underlineCharacterIndex == headerString.length() - 1)
		{
			InternalHeader(winrt::box_value(winrt::hstring(headerString)));
			return;
		}

		_originalHeader = headerString;
		InternalHeader(winrt::box_value(winrt::hstring(headerString)));
	}
	void MenuItem::GetMenuFlyoutItemItems(IVector<MenuFlyoutItemBase> menuFlyoutItems, IVector<MenuFlyoutItemBase> allItems)
	{
		for (unsigned int i = 0; i < menuFlyoutItems.Size(); i++)
		{
			auto menuFlyoutItem = menuFlyoutItems.GetAt(i);
			allItems.Append(menuFlyoutItem);

			MenuFlyoutSubItem menuItem{ menuFlyoutItem.try_as<MenuFlyoutSubItem>() };
			if (menuItem)
			{
				GetMenuFlyoutItemItems(menuItem.Items(), allItems);
			}
		}
	}
	void MenuItem::OnOrientationChanged(DependencyObject d, DependencyPropertyChangedEventArgs const & e)
	{
		auto control = d.try_as<NotePad::MenuItem>();

		auto orientation = control.Orientation() == Orientation::Vertical
			? L"Orientation.Vertical"
			: L"Orientation.Horizontal";

		VisualStateManager::GoToState(control, orientation, true);
	}
	hstring MenuItem::RemoveAlt(hstring inputGesture)
	{
		if (inputGesture.empty())
		{
			return L"";
		}
		std::wstring ws(inputGesture.c_str());
		int index = ws.find_first_of(L"Alt+");
		if (index > 0)
			ws.erase(index, 4);
		inputGesture = winrt::hstring(ws.c_str());
		return inputGesture;
	}
	void MenuItem::ShowMenu()
	{
		if (!IsEnabled() || MenuFlyout().Items().Size() == 0)
		{
			return;
		}

		Windows::Foundation::Point location;
		if (MenuFlyout().Placement() == FlyoutPlacementMode::Bottom)
		{
			location = Windows::Foundation::Point(0, _FlyoutButton.ActualHeight());

		}
		else if (MenuFlyout().Placement() == FlyoutPlacementMode::Right)
		{
			location = Windows::Foundation::Point(_FlyoutButton.ActualWidth(), 0);
		}
		else
		{
			// let the flyout decide where to show
			MenuFlyout().ShowAt(_FlyoutButton);
			return;
		}

		MenuFlyout().ShowAt(_FlyoutButton, location);
	}
	void MenuItem::HideMenu()
	{
		if (_MenuFlyout)
			_MenuFlyout.Hide();
	}
	void MenuItem::OnTapped(Input::TappedRoutedEventArgs e)
	{
		_parentMenu.SelectedMenuItem((class_type)(*this));
		ShowMenu();
		base_type::OnTapped(e);
	}
	void MenuItem::OnGotFocus(RoutedEventArgs e)
	{
		_parentMenu.SelectedMenuItem((class_type)(*this));
		base_type::OnGotFocus(e);
	}
}
