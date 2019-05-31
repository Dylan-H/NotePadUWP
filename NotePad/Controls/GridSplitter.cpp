#include "pch.h"
#include "GridSplitter.h"

namespace winrt::NotePad::implementation
{
	DependencyProperty GridSplitter::_ElementProperty =
		DependencyProperty::Register(L"Element", winrt::xaml_typename<UIElement>(), winrt::xaml_typename<NotePad::GridSplitter>(), PropertyMetadata(winrt::box_value(UIElement{ nullptr }), PropertyChangedCallback(&GridSplitter::OnElementPropertyChanged)));
	DependencyProperty GridSplitter::_ResizeDirectionProperty =
		DependencyProperty::Register(L"ResizeDirection", winrt::xaml_typename<GridResizeDirection>(), winrt::xaml_typename<NotePad::GridSplitter>(), PropertyMetadata(winrt::box_value(GridResizeDirection::Auto)));
	DependencyProperty GridSplitter::_ResizeBehaviorProperty =
		DependencyProperty::Register(L"ResizeBehavior", winrt::xaml_typename<GridResizeBehavior>(), winrt::xaml_typename<NotePad::GridSplitter>(), PropertyMetadata(winrt::box_value(GridResizeBehavior::BasedOnAlignment)));
	DependencyProperty GridSplitter::_GripperForegroundProperty =
		DependencyProperty::Register(L"GripperForeground", winrt::xaml_typename<Brush>(), winrt::xaml_typename<NotePad::GridSplitter>(), PropertyMetadata(winrt::box_value(Brush{ nullptr }), PropertyChangedCallback(&GridSplitter::OnGripperForegroundPropertyChanged)));
	DependencyProperty GridSplitter::_ParentLevelProperty =
		DependencyProperty::Register(L"ParentLevel", { winrt::hstring(winrt::name_of<int>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename<NotePad::GridSplitter>(), PropertyMetadata(winrt::box_value(0)));
	DependencyProperty GridSplitter::_GripperCursorProperty =
		DependencyProperty::RegisterAttached(L"GripperCursor", winrt::xaml_typename<CoreCursorType>(), winrt::xaml_typename<NotePad::GridSplitter>(), PropertyMetadata(winrt::box_value(GripperCursorType::Default), OnGripperCursorPropertyChanged));
	DependencyProperty GridSplitter::_GripperCustomCursorResourceProperty =
		DependencyProperty::RegisterAttached(L"GripperCustomCursorResource", { winrt::hstring(winrt::name_of<unsigned int>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename<NotePad::GridSplitter>(), PropertyMetadata(winrt::box_value(GripperCustomCursorDefaultResource), PropertyChangedCallback(&GridSplitter::GripperCustomCursorResourcePropertyChanged)));
	DependencyProperty GridSplitter::_CursorBehaviorProperty =
		DependencyProperty::RegisterAttached(L"CursorBehavior", winrt::xaml_typename<SplitterCursorBehavior>(), winrt::xaml_typename<NotePad::GridSplitter>(), PropertyMetadata(winrt::box_value(SplitterCursorBehavior::ChangeOnSplitterHover), PropertyChangedCallback(&GridSplitter::CursorBehaviorPropertyChanged)));

	CoreCursor  GridSplitter::ColumnsSplitterCursor = CoreCursor(CoreCursorType::SizeWestEast, 1);
	CoreCursor  GridSplitter::RowSplitterCursor = CoreCursor(CoreCursorType::SizeNorthSouth, 1);


	/// <summary>
	/// Initializes a new instance of the <see cref="GridSplitter"/> class.
	/// </summary>
	GridSplitter::GridSplitter()
	{
		DefaultStyleKey(winrt::box_value(L"NotePad.GridSplitter"));

		GridSplitter_LoadedToken = Loaded(RoutedEventHandler(this, &GridSplitter::GridSplitter_Loaded));
	}


	/// <inheritdoc />

	void GridSplitter::OnKeyDown(KeyRoutedEventArgs e)
	{
		auto step = 1;
		auto ctrlState = Window::Current().CoreWindow().GetKeyState(VirtualKey::Control);
		bool isCtrlDown = (ctrlState & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down;
		if ((ctrlState & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down)
		{
			step = 5;
		}

		if (_resizeDirection == GridResizeDirection::Columns)
		{
			if (e.Key() == VirtualKey::Left)
			{
				HorizontalMove(-step);
			}
			else if (e.Key() == VirtualKey::Right)
			{
				HorizontalMove(step);
			}
			else
			{
				return;
			}

			e.Handled(true);
			return;
		}

		if (_resizeDirection == GridResizeDirection::Rows)
		{
			if (e.Key() == VirtualKey::Up)
			{
				VerticalMove(-step);
			}
			else if (e.Key() == VirtualKey::Down)
			{
				VerticalMove(step);
			}
			else
			{
				return;
			}

			e.Handled(true);
		}

		base_type::OnKeyDown(e);
	}

	/// <inheritdoc />

	void GridSplitter::OnManipulationStarted(ManipulationStartedRoutedEventArgs e)
	{
		// saving the previous state
		PreviousCursor(Window::Current().CoreWindow().PointerCursor());
		_resizeDirection = GetResizeDirection();
		_resizeBehavior = GetResizeBehavior();

		if (_resizeDirection == GridResizeDirection::Columns)
		{
			Window::Current().CoreWindow().PointerCursor(ColumnsSplitterCursor);
		}
		else if (_resizeDirection == GridResizeDirection::Rows)
		{
			Window::Current().CoreWindow().PointerCursor(RowSplitterCursor);
		}

		base_type::OnManipulationStarted(e);
	}

	/// <inheritdoc />

	void GridSplitter::OnManipulationCompleted(ManipulationCompletedRoutedEventArgs e)
	{
		Window::Current().CoreWindow().PointerCursor(PreviousCursor());

		base_type::OnManipulationCompleted(e);
	}

	/// <inheritdoc />

	void GridSplitter::OnManipulationDelta(ManipulationDeltaRoutedEventArgs e)
	{
		auto horizontalChange = e.Delta().Translation.X;
		auto verticalChange = e.Delta().Translation.Y;

		if (_resizeDirection == GridResizeDirection::Columns)
		{
			if (HorizontalMove(horizontalChange))
			{
				return;
			}
		}
		else if (_resizeDirection == GridResizeDirection::Rows)
		{
			if (VerticalMove(verticalChange))
			{
				return;
			}
		}

		base_type::OnManipulationDelta(e);
	}


	/// <inheritdoc />

	void GridSplitter::OnApplyTemplate()
	{
		base_type::OnApplyTemplate();

		// Unhook registered events
		Loaded(GridSplitter_LoadedToken);
		PointerEntered(GridSplitter_PointerEnteredToken);
		PointerExited(GridSplitter_PointerExitedToken);
		PointerPressed(GridSplitter_PointerPressedToken);
		PointerReleased(GridSplitter_PointerReleasedToken);
		ManipulationStarted(GridSplitter_ManipulationStartedToken);
		ManipulationCompleted(GridSplitter_ManipulationCompletedToken);

		if (_hoverWrapper)
			_hoverWrapper->UnhookEvents();

		// Register Events
		GridSplitter_LoadedToken = Loaded(RoutedEventHandler(this, &GridSplitter::GridSplitter_Loaded));
		GridSplitter_PointerEnteredToken = PointerEntered(PointerEventHandler(this, &GridSplitter::GridSplitter_PointerEntered));
		GridSplitter_PointerExitedToken = PointerExited(PointerEventHandler(this, &GridSplitter::GridSplitter_PointerExited));
		GridSplitter_PointerPressedToken = PointerPressed(PointerEventHandler(this, &GridSplitter::GridSplitter_PointerPressed));
		GridSplitter_PointerReleasedToken = PointerReleased(PointerEventHandler(this, &GridSplitter::GridSplitter_PointerReleased));
		GridSplitter_ManipulationStartedToken = ManipulationStarted(ManipulationStartedEventHandler(this, &GridSplitter::GridSplitter_ManipulationStarted));
		GridSplitter_ManipulationCompletedToken = ManipulationCompleted(ManipulationCompletedEventHandler(this, &GridSplitter::GridSplitter_ManipulationCompleted));

		if (_hoverWrapper)
			_hoverWrapper->UpdateHoverElement(Element());

		ManipulationMode(ManipulationModes::TranslateX | ManipulationModes::TranslateY);
	}

	void GridSplitter::OnGripperForegroundPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
	{
		auto gridSplitter = d.try_as<NotePad::GridSplitter>();
		if (gridSplitter)
		{
			NotePad::implementation::GridSplitter* ptr{ winrt::from_abi<NotePad::implementation::GridSplitter>(gridSplitter) };
			if (ptr->_gripperDisplay)
			{
				ptr->_gripperDisplay.Foreground(ptr->GripperForeground());
			}
		}
	}
	void GridSplitter::OnGripperCursorPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
	{
		auto gridSplitter = d.try_as<NotePad::GridSplitter>();

		if (gridSplitter)
		{
			NotePad::implementation::GridSplitter* ptr{ winrt::from_abi<NotePad::implementation::GridSplitter>(gridSplitter) };
			if (ptr->_gripperDisplay)
			{
				ptr->_hoverWrapper->GripperCursor(ptr->GripperCursor());
			}
		}
	}
	void GridSplitter::GripperCustomCursorResourcePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
	{
		auto gridSplitter = d.try_as<NotePad::GridSplitter>();

		if (gridSplitter)
		{
			NotePad::implementation::GridSplitter* ptr{ winrt::from_abi<NotePad::implementation::GridSplitter>(gridSplitter) };
			if (ptr->_gripperDisplay)
			{
				ptr->_hoverWrapper->GripperCustomCursorResource(ptr->GripperCustomCursorResource());
			}
		}
	}
	void GridSplitter::CursorBehaviorPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
	{
		auto gridSplitter = d.try_as<NotePad::GridSplitter>();
		if (gridSplitter)
		{
			NotePad::implementation::GridSplitter* ptr{ winrt::from_abi<NotePad::implementation::GridSplitter>(gridSplitter) };
			if (ptr->_hoverWrapper != nullptr)
			{

				ptr->_hoverWrapper->UpdateHoverElement((ptr->CursorBehavior() ==
					SplitterCursorBehavior::ChangeOnSplitterHover
					? gridSplitter
					: gridSplitter.Element()));
			}
		}
	}
	void GridSplitter::OnElementPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
	{
		auto gridSplitter = d.try_as<NotePad::GridSplitter>();

		if (gridSplitter)
		{
			NotePad::implementation::GridSplitter* ptr{ winrt::from_abi<NotePad::implementation::GridSplitter>(gridSplitter) };
			if (ptr->_hoverWrapper)
			{
				ptr->_hoverWrapper->UpdateHoverElement((ptr->CursorBehavior() ==
					SplitterCursorBehavior::ChangeOnSplitterHover
					? gridSplitter
					: gridSplitter.Element()));
			}
		}
	}
	bool GridSplitter::IsStarColumn(ColumnDefinition definition)
	{
		auto gl = winrt::unbox_value<GridLength>(definition.GetValue(ColumnDefinition::WidthProperty()));
		return GridLengthHelper::GetIsStar(gl);
	}
	bool GridSplitter::IsStarRow(RowDefinition definition)
	{
		auto gl = winrt::unbox_value<GridLength>(definition.GetValue(RowDefinition::HeightProperty()));
		return GridLengthHelper::GetIsStar(gl);
	}
	FrameworkElement GridSplitter::TargetControl()
	{
		if (ParentLevel() == 0)
		{
			return *this;
		}

		auto parent = Parent();
		for (int i = 2; i < ParentLevel(); i++)
		{
			auto frameworkElement = parent.try_as<FrameworkElement>();
			if (frameworkElement)
			{
				parent = frameworkElement.Parent();
			}
		}

		return parent.try_as<FrameworkElement>();

	}

	/// <summary>
	/// Gets the current Column definition of the parent Grid
	/// </summary>

	ColumnDefinition GridSplitter::CurrentColumn()
	{
		if (Resizable() == nullptr)
		{
			return nullptr;
		}

		auto gridSplitterTargetedColumnIndex = GetTargetedColumn();

		if ((gridSplitterTargetedColumnIndex >= 0)
			&& (gridSplitterTargetedColumnIndex < Resizable().ColumnDefinitions().Size()))
		{
			return Resizable().ColumnDefinitions().GetAt(gridSplitterTargetedColumnIndex);
		}

		return nullptr;

	}

	/// <summary>
	/// Gets the Sibling Column definition of the parent Grid
	/// </summary>

	ColumnDefinition GridSplitter::SiblingColumn()
	{
		if (Resizable() == nullptr)
		{
			return nullptr;
		}

		auto gridSplitterSiblingColumnIndex = GetSiblingColumn();

		if ((gridSplitterSiblingColumnIndex >= 0)
			&& (gridSplitterSiblingColumnIndex < Resizable().ColumnDefinitions().Size()))
		{
			return Resizable().ColumnDefinitions().GetAt(gridSplitterSiblingColumnIndex);
		}

		return nullptr;

	}

	/// <summary>
	/// Gets the current Row definition of the parent Grid
	/// </summary>

	RowDefinition GridSplitter::CurrentRow()
	{
		if (Resizable() == nullptr)
		{
			return nullptr;
		}

		auto gridSplitterTargetedRowIndex = GetTargetedRow();

		if ((gridSplitterTargetedRowIndex >= 0)
			&& (gridSplitterTargetedRowIndex < Resizable().RowDefinitions().Size()))
		{
			return Resizable().RowDefinitions().GetAt(gridSplitterTargetedRowIndex);
		}

		return nullptr;

	}

	/// <summary>
	/// Gets the Sibling Row definition of the parent Grid
	/// </summary>

	RowDefinition GridSplitter::SiblingRow()
	{

		if (Resizable() == nullptr)
		{
			return nullptr;
		}

		auto gridSplitterSiblingRowIndex = GetSiblingRow();

		if ((gridSplitterSiblingRowIndex >= 0)
			&& (gridSplitterSiblingRowIndex < Resizable().RowDefinitions().Size()))
		{
			return Resizable().RowDefinitions().GetAt(gridSplitterSiblingRowIndex);
		}

		return nullptr;

	}
	void GridSplitter::GridSplitter_PointerReleased(winrt::Windows::Foundation::IInspectable sender, PointerRoutedEventArgs e)
	{
		_pressed = false;
		VisualStateManager::GoToState(*this, _pointerEntered ? L"PointerOver" : L"Normal", true);
	}
	void GridSplitter::GridSplitter_PointerPressed(winrt::Windows::Foundation::IInspectable sender, PointerRoutedEventArgs e)
	{
		_pressed = true;
		VisualStateManager::GoToState(*this, L"Pressed", true);
	}
	void GridSplitter::GridSplitter_PointerExited(winrt::Windows::Foundation::IInspectable sender, PointerRoutedEventArgs e)
	{
		_pointerEntered = false;

		if (!_pressed && !_dragging)
		{
			VisualStateManager::GoToState(*this, L"Normal", true);
		}
	}
	void GridSplitter::GridSplitter_PointerEntered(winrt::Windows::Foundation::IInspectable sender, PointerRoutedEventArgs e)
	{
		_pointerEntered = true;

		if (!_pressed && !_dragging)
		{
			VisualStateManager::GoToState(*this, L"PointerOver", true);
		}
	}
	void GridSplitter::GridSplitter_ManipulationCompleted(winrt::Windows::Foundation::IInspectable sender, ManipulationCompletedRoutedEventArgs e)
	{
		_dragging = false;
		_pressed = false;
		VisualStateManager::GoToState(*this, _pointerEntered ? L"PointerOver" : L"Normal", true);
	}
	void GridSplitter::GridSplitter_ManipulationStarted(winrt::Windows::Foundation::IInspectable sender, ManipulationStartedRoutedEventArgs e)
	{
		_dragging = true;
		VisualStateManager::GoToState(*this, L"Pressed", true);
	}
	bool GridSplitter::SetColumnWidth(ColumnDefinition columnDefinition, double horizontalChange, GridUnitType unitType)
	{
		auto newWidth = columnDefinition.ActualWidth() + horizontalChange;

		auto minWidth = columnDefinition.MinWidth();
		if (!std::isnan(minWidth) && newWidth < minWidth)
		{
			newWidth = minWidth;
		}

		auto maxWidth = columnDefinition.MaxWidth();
		if (!std::isnan(maxWidth) && newWidth > maxWidth)
		{
			newWidth = maxWidth;
		}

		if (newWidth > ActualWidth())
		{
			columnDefinition.Width(GridLengthHelper::FromValueAndType(newWidth, unitType));
			return true;
		}

		return false;
	}
	bool GridSplitter::IsValidColumnWidth(ColumnDefinition columnDefinition, double horizontalChange)
	{
		auto newWidth = columnDefinition.ActualWidth() + horizontalChange;

		auto minWidth = columnDefinition.MinWidth();
		if (!std::isnan(minWidth) && newWidth < minWidth)
		{
			return false;
		}

		auto maxWidth = columnDefinition.MaxWidth();
		if (!std::isnan(maxWidth) && newWidth > maxWidth)
		{
			return false;
		}

		if (newWidth <= ActualWidth())
		{
			return false;
		}

		return true;
	}
	bool GridSplitter::SetRowHeight(RowDefinition rowDefinition, double verticalChange, GridUnitType unitType)
	{
		auto newHeight = rowDefinition.ActualHeight() + verticalChange;

		auto minHeight = rowDefinition.MinHeight();
		if (!std::isnan(minHeight) && newHeight < minHeight)
		{
			newHeight = minHeight;
		}

		auto maxWidth = rowDefinition.MaxHeight();
		if (!std::isnan(maxWidth) && newHeight > maxWidth)
		{
			newHeight = maxWidth;
		}

		if (newHeight > ActualHeight())
		{
			rowDefinition.Height(GridLengthHelper::FromValueAndType(newHeight, unitType));
			return true;
		}

		return false;
	}
	bool GridSplitter::IsValidRowHeight(RowDefinition rowDefinition, double verticalChange)
	{
		auto newHeight = rowDefinition.ActualHeight() + verticalChange;

		auto minHeight = rowDefinition.MinHeight();
		if (!std::isnan(minHeight) && newHeight < minHeight)
		{
			return false;
		}

		auto maxHeight = rowDefinition.MaxHeight();
		if (!std::isnan(maxHeight) && newHeight > maxHeight)
		{
			return false;
		}

		if (newHeight <= ActualHeight())
		{
			return false;
		}

		return true;
	}

	// Return the targeted Column based on the resize behavior

	int GridSplitter::GetTargetedColumn()
	{
		auto currentIndex = Grid::GetColumn(TargetControl());
		return GetTargetIndex(currentIndex);
	}

	// Return the sibling Row based on the resize behavior

	int GridSplitter::GetTargetedRow()
	{
		auto currentIndex = Grid::GetRow(TargetControl());
		return GetTargetIndex(currentIndex);
	}

	// Return the sibling Column based on the resize behavior

	int GridSplitter::GetSiblingColumn()
	{
		auto currentIndex = Grid::GetColumn(TargetControl());
		return GetSiblingIndex(currentIndex);
	}

	// Return the sibling Row based on the resize behavior

	int GridSplitter::GetSiblingRow()
	{
		auto currentIndex = Grid::GetRow(TargetControl());
		return GetSiblingIndex(currentIndex);
	}

	// Gets index based on resize behavior for first targeted row/column

	int GridSplitter::GetTargetIndex(int currentIndex)
	{
		switch (_resizeBehavior)
		{
		case GridResizeBehavior::CurrentAndNext:
			return currentIndex;
		case GridResizeBehavior::PreviousAndNext:
			return currentIndex - 1;
		case GridResizeBehavior::PreviousAndCurrent:
			return currentIndex - 1;
		default:
			return -1;
		}
	}

	// Gets index based on resize behavior for second targeted row/column

	int GridSplitter::GetSiblingIndex(int currentIndex)
	{
		switch (_resizeBehavior)
		{
		case GridResizeBehavior::CurrentAndNext:
			return currentIndex + 1;
		case GridResizeBehavior::PreviousAndNext:
			return currentIndex + 1;
		case GridResizeBehavior::PreviousAndCurrent:
			return currentIndex;
		default:
			return -1;
		}
	}

	// Checks the control alignment and Width/Height to detect the control resize direction columns/rows

	GridResizeDirection GridSplitter::GetResizeDirection()
	{
		GridResizeDirection direction = ResizeDirection();

		if (direction == GridResizeDirection::Auto)
		{
			// When HorizontalAlignment is Left, Right or Center, resize Columns
			if (HorizontalAlignment() != HorizontalAlignment::Stretch)
			{
				direction = GridResizeDirection::Columns;
			}

			// When VerticalAlignment is Top, Bottom or Center, resize Rows
			else if (VerticalAlignment() != VerticalAlignment::Stretch)
			{
				direction = GridResizeDirection::Rows;
			}

			// Check Width vs Height
			else if (ActualWidth() <= ActualHeight())
			{
				direction = GridResizeDirection::Columns;
			}
			else
			{
				direction = GridResizeDirection::Rows;
			}
		}

		return direction;
	}

	// Get the resize behavior (Which columns/rows should be resized) based on alignment and Direction

	GridResizeBehavior GridSplitter::GetResizeBehavior()
	{
		GridResizeBehavior resizeBehavior = ResizeBehavior();

		if (resizeBehavior == GridResizeBehavior::BasedOnAlignment)
		{
			if (_resizeDirection == GridResizeDirection::Columns)
			{
				switch (HorizontalAlignment())
				{
				case HorizontalAlignment::Left:
					resizeBehavior = GridResizeBehavior::PreviousAndCurrent;
					break;
				case HorizontalAlignment::Right:
					resizeBehavior = GridResizeBehavior::CurrentAndNext;
					break;
				default:
					resizeBehavior = GridResizeBehavior::PreviousAndNext;
					break;
				}
			}

			// resize direction is vertical
			else
			{
				switch (VerticalAlignment())
				{
				case VerticalAlignment::Top:
					resizeBehavior = GridResizeBehavior::PreviousAndCurrent;
					break;
				case VerticalAlignment::Bottom:
					resizeBehavior = GridResizeBehavior::CurrentAndNext;
					break;
				default:
					resizeBehavior = GridResizeBehavior::PreviousAndNext;
					break;
				}
			}
		}

		return resizeBehavior;
	}
	void GridSplitter::GridSplitter_Loaded(Windows::Foundation::IInspectable sender, RoutedEventArgs e)
	{
		_resizeDirection = GetResizeDirection();
		_resizeBehavior = GetResizeBehavior();

		// Adding Grip to Grid Splitter
		if (Element() == UIElement{ nullptr })
		{
			CreateGripperDisplay();
			Element(_gripperDisplay);
		}

		if (_hoverWrapper == nullptr)
		{
			auto hoverWrapper = new GripperHoverWrapper(
				CursorBehavior() == SplitterCursorBehavior::ChangeOnSplitterHover
				? *this
				: Element(),
				_resizeDirection,
				GripperCursor(),
				GripperCustomCursorResource());
			ManipulationStarted(ManipulationStartedEventHandler(hoverWrapper, &GripperHoverWrapper::SplitterManipulationStarted));
			ManipulationCompleted(ManipulationCompletedEventHandler(hoverWrapper, &GripperHoverWrapper::SplitterManipulationCompleted));

			_hoverWrapper = hoverWrapper;
		}
	}
	void GridSplitter::CreateGripperDisplay()
	{
		if (_gripperDisplay == nullptr)
		{
			_gripperDisplay = TextBlock();
			auto f = winrt::Windows::UI::Xaml::Media::FontFamily(GripperDisplayFont);
			_gripperDisplay.FontFamily(f);
			_gripperDisplay.HorizontalAlignment(HorizontalAlignment::Center);
			_gripperDisplay.VerticalAlignment(VerticalAlignment::Center);
			_gripperDisplay.Foreground(GripperForeground());
			_gripperDisplay.Text(_resizeDirection == GridResizeDirection::Columns ? GripperBarVertical : GripperBarHorizontal);

		}
	}
	bool GridSplitter::VerticalMove(double verticalChange)
	{
		if (CurrentRow() == nullptr || SiblingRow() == nullptr)
		{
			return true;
		}

		// if current row has fixed height then resize it
		if (!IsStarRow(CurrentRow()))
		{
			// No need to check for the row Min height because it is automatically respected
			if (!SetRowHeight(CurrentRow(), verticalChange, GridUnitType::Pixel))
			{
				return true;
			}
		}

		// if sibling row has fixed width then resize it
		else if (!IsStarRow(SiblingRow()))
		{
			// Would adding to this column make the current column violate the MinWidth?
			if (IsValidRowHeight(CurrentRow(), verticalChange) == false)
			{
				return false;
			}

			if (!SetRowHeight(SiblingRow(), verticalChange * -1, GridUnitType::Pixel))
			{
				return true;
			}
		}

		// if both row haven't fixed height (auto *)
		else
		{
			// change current row height to the new height with respecting the auto
			// change sibling row height to the new height relative to current row
			// respect the other star row height by setting it's height to it's actual height with stars

			// We need to validate current and sibling height to not cause any un expected behavior
			if (!IsValidRowHeight(CurrentRow(), verticalChange) ||
				!IsValidRowHeight(SiblingRow(), verticalChange * -1))
			{
				return true;
			}

			for (auto rowDefinition : Resizable().RowDefinitions())
			{
				if (rowDefinition == CurrentRow())
				{
					SetRowHeight(CurrentRow(), verticalChange, GridUnitType::Star);
				}
				else if (rowDefinition == SiblingRow())
				{
					SetRowHeight(SiblingRow(), verticalChange * -1, GridUnitType::Star);
				}
				else if (IsStarRow(rowDefinition))
				{
					rowDefinition.Height(GridLengthHelper::FromValueAndType(rowDefinition.ActualHeight(), GridUnitType::Star));
				}
			}
		}

		return false;
	}
	bool GridSplitter::HorizontalMove(double horizontalChange)
	{
		if (CurrentColumn() == nullptr || SiblingColumn() == nullptr)
		{
			return true;
		}

		// if current column has fixed width then resize it
		if (!IsStarColumn(CurrentColumn()))
		{
			// No need to check for the Column Min width because it is automatically respected
			if (!SetColumnWidth(CurrentColumn(), horizontalChange, GridUnitType::Pixel))
			{
				return true;
			}
		}

		// if sibling column has fixed width then resize it
		else if (!IsStarColumn(SiblingColumn()))
		{
			// Would adding to this column make the current column violate the MinWidth?
			if (IsValidColumnWidth(CurrentColumn(), horizontalChange) == false)
			{
				return false;
			}

			if (!SetColumnWidth(SiblingColumn(), horizontalChange * -1, GridUnitType::Pixel))
			{
				return true;
			}
		}

		// if both column haven't fixed width (auto *)
		else
		{
			// change current column width to the new width with respecting the auto
			// change sibling column width to the new width relative to current column
			// respect the other star column width by setting it's width to it's actual width with stars

			// We need to validate current and sibling width to not cause any un expected behavior
			if (!IsValidColumnWidth(CurrentColumn(), horizontalChange) ||
				!IsValidColumnWidth(SiblingColumn(), horizontalChange * -1))
			{
				return true;
			}

			for (auto columnDefinition : Resizable().ColumnDefinitions())
			{
				if (columnDefinition == CurrentColumn())
				{
					SetColumnWidth(CurrentColumn(), horizontalChange, GridUnitType::Star);
				}
				else if (columnDefinition == SiblingColumn())
				{
					SetColumnWidth(SiblingColumn(), horizontalChange * -1, GridUnitType::Star);
				}
				else if (IsStarColumn(columnDefinition))
				{
					columnDefinition.Width(GridLengthHelper::FromValueAndType(columnDefinition.ActualWidth(), GridUnitType::Star));
				}
			}
		}

		return false;
	}
}
