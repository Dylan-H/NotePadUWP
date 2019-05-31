#include "pch.h"
#include "TabView.h"

namespace winrt::NotePad::implementation
{
	DependencyProperty TabView::_TabStartHeaderProperty =
		DependencyProperty::Register(L"TabStartHeader", winrt::xaml_typename<Windows::Foundation::IInspectable>(), winrt::xaml_typename<NotePad::TabView>(), PropertyMetadata(nullptr, OnLayoutEffectingPropertyChanged));
	DependencyProperty TabView::_TabStartHeaderTemplateProperty =
		DependencyProperty::Register(L"TabStartHeaderTemplate", winrt::xaml_typename<DataTemplate>(), winrt::xaml_typename<NotePad::TabView>(), PropertyMetadata(nullptr, OnLayoutEffectingPropertyChanged));
	DependencyProperty TabView::_TabActionHeaderProperty =
		DependencyProperty::Register(L"TabActionHeader", winrt::xaml_typename<winrt::Windows::Foundation::IInspectable>(), winrt::xaml_typename<NotePad::TabView>(), PropertyMetadata(nullptr, OnLayoutEffectingPropertyChanged));
	DependencyProperty TabView::_TabActionHeaderTemplateProperty =
		DependencyProperty::Register(L"TabActionHeaderTemplate", winrt::xaml_typename<DataTemplate>(), winrt::xaml_typename<NotePad::TabView>(), PropertyMetadata(nullptr, OnLayoutEffectingPropertyChanged));
	DependencyProperty TabView::_TabEndHeaderProperty =
		DependencyProperty::Register(L"TabEndHeader", winrt::xaml_typename<Windows::Foundation::IInspectable>(), winrt::xaml_typename<NotePad::TabView>(), PropertyMetadata(nullptr, OnLayoutEffectingPropertyChanged));
	DependencyProperty TabView::_TabEndHeaderTemplateProperty =
		DependencyProperty::Register(L"TabEndHeaderTemplate", winrt::xaml_typename<DataTemplate>(), winrt::xaml_typename<NotePad::TabView>(), PropertyMetadata(nullptr, OnLayoutEffectingPropertyChanged));
	DependencyProperty TabView::_ItemHeaderTemplateProperty =
		DependencyProperty::Register(L"ItemHeaderTemplate", winrt::xaml_typename<DataTemplate>(), winrt::xaml_typename<NotePad::TabView>(), PropertyMetadata(nullptr, OnLayoutEffectingPropertyChanged));
	DependencyProperty TabView::_CanCloseTabsProperty =
		DependencyProperty::Register(L"CanCloseTabs", { winrt::hstring(winrt::name_of<bool>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename<NotePad::TabView>(), PropertyMetadata(winrt::box_value(false), OnLayoutEffectingPropertyChanged));
	DependencyProperty TabView::_IsCloseButtonOverlayProperty =
		DependencyProperty::Register(L"IsCloseButtonOverlay", { winrt::hstring(winrt::name_of<bool>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename<NotePad::TabView>(), PropertyMetadata(winrt::box_value(false), OnLayoutEffectingPropertyChanged));
	DependencyProperty TabView::_SelectedTabWidthProperty =
		DependencyProperty::Register(L"SelectedTabWidth", { winrt::hstring(winrt::name_of<double>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename<NotePad::TabView>(), PropertyMetadata(winrt::box_value(0.0), OnLayoutEffectingPropertyChanged));
	DependencyProperty TabView::_TabWidthBehaviorProperty =
		DependencyProperty::Register(L"TabWidthBehavior", winrt::xaml_typename <NotePad::TabWidthMode>(), winrt::xaml_typename<NotePad::TabView>(), PropertyMetadata(winrt::box_value(TabWidthMode::Actual), OnLayoutEffectingPropertyChanged));
	DependencyProperty TabView::_IgnoreColumnProperty =
		DependencyProperty::RegisterAttached(L"IgnoreColumn", { winrt::hstring(winrt::name_of<bool>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename<NotePad::TabView>(), PropertyMetadata(winrt::box_value(false)));
	DependencyProperty TabView::_ConstrainColumnProperty =
		DependencyProperty::RegisterAttached(L"ConstrainColumn", { winrt::hstring(winrt::name_of<bool>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename<NotePad::TabView>(), PropertyMetadata(winrt::box_value(false)));
	DependencyProperty TabView::_OriginalWidthProperty =
		DependencyProperty::RegisterAttached(L"OriginalWidth", { winrt::hstring(winrt::name_of<double>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename<NotePad::TabView>(), PropertyMetadata(nullptr));



	TabView::TabView()
	{
		DefaultStyleKey(winrt::box_value(L"NotePad.TabView"));
		// Container Generation Hooks
		RegisterPropertyChangedCallback(ItemsControl::ItemsSourceProperty(), DependencyPropertyChangedCallback(this,&TabView::ItemsSource_PropertyChanged));
		ItemContainerGenerator_ItemsChangedToken = ItemContainerGenerator().ItemsChanged(ItemsChangedEventHandler(this, &TabView::ItemContainerGenerator_ItemsChanged));

		// Drag and Layout Hooks
		TabView_DragItemsStartingToken = DragItemsStarting(DragItemsStartingEventHandler(this, &TabView::TabView_DragItemsStarting));
		TabView_DragItemsCompletedToken = DragItemsCompleted(Windows::Foundation::TypedEventHandler<ListViewBase, DragItemsCompletedEventArgs>(this, &TabView::TabView_DragItemsCompleted));
		TabView_SizeChangedToken = SizeChanged(SizeChangedEventHandler(this, &TabView::TabView_SizeChanged));

		// Selection Hook
		TabView_SelectionChangedToken = SelectionChanged(SelectionChangedEventHandler(this, &TabView::TabView_SelectionChanged));
	}
	bool TabView::IsItemItsOwnContainerOverride(Windows::Foundation::IInspectable item)
	{
		auto ret = item.try_as<NotePad::TabViewItem>();
		return ret ? true : false;
	}
	void TabView::OnApplyTemplate()
	{
		base_type::OnApplyTemplate();

		if (_tabItemsPresenter)
		{
			_tabItemsPresenter.SizeChanged(TabView_SizeChangedToken);
		}

		if (_tabScroller)
		{
			_tabScroller.Loaded(ScrollViewer_LoadedToken);
		}
		auto child = GetTemplateChild(TabContentPresenterName);
		_tabContentPresenter = child.try_as<ContentPresenter>();
		_tabViewContainer = GetTemplateChild(TabViewContainerName).try_as<Grid>();
		_tabItemsPresenter = GetTemplateChild(TabsItemsPresenterName).try_as <ItemsPresenter>();
		_tabScroller = GetTemplateChild(TabsScrollViewerName).try_as<ScrollViewer>();

		if (_tabItemsPresenter != nullptr)
		{
			TabView_SizeChangedToken = _tabItemsPresenter.SizeChanged(SizeChangedEventHandler(this, &TabView::TabView_SizeChanged));
		}

		if (_tabScroller != nullptr)
		{
			ScrollViewer_LoadedToken = _tabScroller.Loaded(RoutedEventHandler(this, &TabView::ScrollViewer_Loaded));
		}
	}
	void TabView::OnItemsChanged(Windows::Foundation::IInspectable e)
	{
		auto args = winrt::unbox_value<Windows::Foundation::Collections::IVectorChangedEventArgs>(e);

		base_type::OnItemsChanged(e);

		if (args.CollectionChange() == Windows::Foundation::Collections::CollectionChange::ItemRemoved && SelectedIndex() == -1)
		{
			// If we remove the selected item we should select the previous item
			int startIndex = (int)args.Index() + 1;
			if (startIndex > Items().Size())
			{
				startIndex = 0;
			}

			SelectedIndex(FindNextTabIndex(startIndex, -1));
		}

		// Update Sizing (in case there are less items now)
		TabView_SizeChanged(*this, nullptr);
	}
	void TabView::ScrollViewer_Loaded(Windows::Foundation::IInspectable sender, RoutedEventArgs e)
	{
		_tabScroller.Loaded(ScrollViewer_LoadedToken);

		if (_tabScrollBackButton)
		{
			_tabScrollBackButton.Click(ScrollTabBackButton_ClickToken);
		}

		if (_tabScrollForwardButton)
		{
			_tabScrollForwardButton.Click(ScrollTabForwardButton_ClickToken);
		}
		auto sdbutton = FindDescendantByName(_tabScroller, TabsScrollBackButtonName);
		if (sdbutton) {
			_tabScrollBackButton = sdbutton.try_as<ButtonBase>();
		}
		auto sfbutton = FindDescendantByName(_tabScroller, TabsScrollForwardButtonName);
		if (sfbutton) {
			_tabScrollForwardButton = sfbutton.try_as<ButtonBase>();
		}

		if (_tabScrollBackButton)
		{
			ScrollTabBackButton_ClickToken = _tabScrollBackButton.Click(RoutedEventHandler(this, &TabView::ScrollTabBackButton_Click));;
		}

		if (_tabScrollForwardButton)
		{
			ScrollTabForwardButton_ClickToken = _tabScrollForwardButton.Click(RoutedEventHandler(this, &TabView::ScrollTabForwardButton_Click));
		}
	}
	FrameworkElement TabView::FindDescendantByName(DependencyObject element, hstring name)
	{
		if (element == nullptr || name.empty())
		{
			return nullptr;
		}
		auto fe = element.try_as< FrameworkElement>();
		if (fe) {
			std::wstring elementname = fe.Name().c_str();
			std::wstring wname = name.c_str();
			std::transform(elementname.begin(), elementname.end(), elementname.begin(), ::toupper);
			std::transform(wname.begin(), wname.end(), wname.begin(), ::toupper);

			if (elementname == wname)
			{
				return element.try_as<FrameworkElement>();
			}
		}
		auto childCount = VisualTreeHelper::GetChildrenCount(element);
		for (int i = 0; i < childCount; i++)
		{
			auto child = VisualTreeHelper::GetChild(element, i);
			auto result = FindDescendantByName(child, name);
			if (result)
			{
				return result;
			}
		}

		return nullptr;
	}
	 void TabView::PrepareContainerForItemOverride(DependencyObject element, Windows::Foundation::IInspectable item)
	{
		base_type::PrepareContainerForItemOverride(element, item);

		auto tabitem = element.try_as<NotePad::TabViewItem>();

		TabViewItem_LoadedToken = tabitem.Loaded(RoutedEventHandler(this, &TabView::TabViewItem_Loaded));
		TabViewItem_ClosingToken = tabitem.Closing(Windows::Foundation::EventHandler<Windows::Foundation::IInspectable>(this, &TabView::TabViewItem_Closing));

		if (tabitem.Header()==nullptr)
		{
			tabitem.Header(item);
		}

		if (tabitem.HeaderTemplate() == nullptr)
		{
			auto headertemplatebinding = Binding();

			headertemplatebinding.Source(*this);
			headertemplatebinding.Path(PropertyPath(L"ItemHeaderTemplate"));
			headertemplatebinding.Mode(BindingMode::OneWay);

			tabitem.SetBinding(TabViewItem::HeaderTemplateProperty(), headertemplatebinding);
		}


		if (tabitem.IsClosable() != true && tabitem.ReadLocalValue(TabViewItem::IsClosableProperty()) == DependencyProperty::UnsetValue())
		{
			auto iscloseablebinding = Binding();

			iscloseablebinding.Source(*this),
			iscloseablebinding.Path(PropertyPath(L"CanCloseTabs")),
			iscloseablebinding.Mode(BindingMode::OneWay),

			tabitem.SetBinding(TabViewItem::IsClosableProperty(), iscloseablebinding);
		}
	}
	 NotePad::TabViewItem TabView::FindParent(FrameworkElement const & element)
	{
		if (!element.Parent())
		{
			return NotePad::TabViewItem{ nullptr };
		}
		NotePad::TabViewItem item = { element.Parent().try_as<NotePad::TabViewItem>() };

		if (item)
		{
			return item;
		}

		return FindParent(element.Parent().try_as<FrameworkElement>());
	}
	void TabView::ScrollTabBackButton_Click(Windows::Foundation::IInspectable sender, RoutedEventArgs e)
	{
		_tabScroller.ChangeView(std::max(0.0, _tabScroller.HorizontalOffset() - ScrollAmount), nullptr, nullptr);
	}
	void TabView::ScrollTabForwardButton_Click(Windows::Foundation::IInspectable sender, RoutedEventArgs e)
	{
		_tabScroller.ChangeView(std::min(_tabScroller.ScrollableWidth(), _tabScroller.HorizontalOffset() + ScrollAmount), nullptr, nullptr);
	}
	 void TabView::TabView_SelectionChanged(Windows::Foundation::IInspectable sender, SelectionChangedEventArgs e)
	{
		if (_isDragging)
		{
			// Skip if we're dragging, we'll reset when we're done.
			return;
		}

		if (_tabContentPresenter)
		{
			if (SelectedItem() == nullptr)
			{
				_tabContentPresenter.Content(nullptr);
				_tabContentPresenter.ContentTemplate(nullptr);
			}
			else
			{
				auto container = ContainerFromItem(SelectedItem()).try_as< NotePad::TabViewItem>();
				if (container)
				{
					_tabContentPresenter.Content(container.Content());
					_tabContentPresenter.ContentTemplate(container.ContentTemplate());
				}
			}
		}

		// If our width can be effected by the selection, need to run algorithm.
		if (SelectedTabWidth() > 0)
		{
			TabView_SizeChanged(sender, nullptr);
		}
	}
	 void TabView::TabViewItem_Loaded(Windows::Foundation::IInspectable sender, RoutedEventArgs e)
	{
		auto tabitem = sender.try_as< NotePad::TabViewItem>();
		if (tabitem)
			tabitem.Loaded(TabViewItem_LoadedToken);

		// Only need to do this once.
		if (!_hasLoaded)
		{
			_hasLoaded = true;

			// Need to set a tab's selection on load, otherwise ListView resets to null.
			SetInitialSelection();

			// Need to make sure ContentPresenter is set to content based on selection.
			TabView_SelectionChanged(*this, nullptr);

			// Need to make sure we've registered our removal method.
			ItemsSource_PropertyChanged(*this, nullptr);

			// Make sure we complete layout now.
			TabView_SizeChanged(*this, nullptr);
		}
	}
	 void TabView::TabViewItem_Closing(Windows::Foundation::IInspectable sender, Windows::Foundation::IInspectable  e)
	{
		 auto ret = winrt::unbox_value< TabClosingEventArgs>(e);
		auto item = ItemFromContainer(ret.Tab());

		auto args = TabClosingEventArgs(item, ret.Tab());
		if (TabClosing)
			TabClosing(*this, args);

		if (!args.Cancel())
		{
			if (ItemsSource() != nullptr)
			{
				if (_bremoveItemsSourceMethod) {
					auto ret = ItemsSource();
				}
			}
			else
			{
				uint32_t index = -1;
				if (Items().IndexOf(item, index)) {
					Items().RemoveAt(index);
				}
			}
		}
	}
	void TabView::TabView_DragItemsStarting(Windows::Foundation::IInspectable sender, DragItemsStartingEventArgs e)
	{
		// Keep track of drag so we don't modify content until done.
		_isDragging = true;
	}
	 void TabView::TabView_DragItemsCompleted(ListViewBase sender, DragItemsCompletedEventArgs args)
	{
		_isDragging = false;

		// args.DropResult == None when outside of area (e.g. create new window)
		if (args.DropResult() == DataPackageOperation::None)
		{
			auto item = args.Items().GetAt(0);
			auto tab = ContainerFromItem(item).try_as< NotePad::TabViewItem>();
			auto fe = item.try_as< FrameworkElement>();
			if (tab == nullptr && fe)
			{
				tab = FindParent(fe);
			}

			if (tab == nullptr)
			{
				// We still don't have a TabViewItem, most likely is a static TabViewItem in the template being dragged and not selected.
				// This is a fallback scenario for static tabs.
				// Note: This can be wrong if two TabViewItems share the exact same Content (i.e. a string), this should be unlikely in any practical scenario.
				for (int i = 0; i < Items().Size(); i++)
				{
					auto tabItem = ContainerFromIndex(i).try_as< NotePad::TabViewItem>();
					if (tabItem.Content() == item)
					{
						tab = tabItem;
						break;
					}
				}
			}
			if (TabDraggedOutside) {
				TabDraggedOutside(*this, winrt::box_value(TabDraggedOutsideEventArgs(item, tab)));
			}

		}
		else
		{
			// If dragging the active tab, there's an issue with the CP blanking.
			TabView_SelectionChanged(*this, nullptr);
		}
	}
	void TabView::OnLayoutEffectingPropertyChanged(DependencyObject sender, DependencyPropertyChangedEventArgs args)
	{
		auto tabview = sender.try_as<NotePad::TabView>();
		if (tabview != nullptr)
		{
			NotePad::implementation::TabView* ptr{ winrt::from_abi<NotePad::implementation::TabView>(tabview) };
			if (ptr->_hasLoaded)
				ptr->TabView_SizeChanged(tabview, nullptr);
		}
	}
	void TabView::TabView_SizeChanged(Windows::Foundation::IInspectable sender, SizeChangedEventArgs e)
	{
		// We need to do this calculation here in Size Changed as the
		// Columns don't have their Actual Size calculated in Measure or Arrange.
		if (_hasLoaded && _tabViewContainer != nullptr)
		{
			// Look for our special columns to calculate size of other 'stuff'
			auto columnDefinitions = _tabViewContainer.ColumnDefinitions();
			double taken = 0;
			ColumnDefinition tabc{ nullptr };
			bool bfirst = true;
			for (int i = 0; i < columnDefinitions.Size(); i++) {
				auto cd = columnDefinitions.GetAt(i);
				if (GetIgnoreColumn(cd)) {
					if (bfirst) {
						tabc = cd;
						bfirst = false;
					}
				}
				else
				{
					taken += cd.ActualWidth();
				}


			}

			if (tabc != nullptr)
			{
				auto available = ActualWidth() - taken;
				auto required = 0.0;
				auto mintabwidth = (std::numeric_limits<double>::max)();

				if (TabWidthBehavior() == TabWidthMode::Actual)
				{
					if (_tabScroller != nullptr)
					{
						// If we have a scroll container, get its size.
						required = _tabScroller.ExtentWidth();
					}

					// Restore original widths
					for (unsigned int i = 0; i < Items().Size(); i++)
					{
						auto item = Items().GetAt(i);
						auto tab = ContainerFromItem(item).try_as<NotePad::TabViewItem>();
						if (tab == nullptr)
						{
							continue; // container not generated yet
						}

						if (tab.ReadLocalValue(_OriginalWidthProperty) != DependencyProperty::UnsetValue())
						{
							tab.Width(GetOriginalWidth(tab));
						}
					}
				}
				else if (available > 0)
				{
					// Calculate the width for each tab from the provider and determine how much space they take.
					for (unsigned int i = 0; i < Items().Size(); i++)
					{
						auto item = Items().GetAt(i);
						auto tab = ContainerFromItem(item).try_as< NotePad::TabViewItem>();
						if (tab == nullptr)
						{
							continue; // container not generated yet
						}

						mintabwidth = std::min(mintabwidth, tab.MinWidth());

						double width = 0;

						switch (TabWidthBehavior())
						{
						case TabWidthMode::Equal:
							width = ProvideEqualWidth(tab, item, available);
							break;
						case TabWidthMode::Compact:
							width = ProvideCompactWidth(tab, item, available);
							break;
						}

						if (tab.ReadLocalValue(_OriginalWidthProperty) == DependencyProperty::UnsetValue())
						{
							SetOriginalWidth(tab, tab.Width());
						}

						if (width > std::numeric_limits<float>::epsilon())
						{
							tab.Width(width);
							required += std::max(std::min(width, tab.MaxWidth()), tab.MinWidth());
						}
						else
						{
							tab.Width(GetOriginalWidth(tab));
							required += tab.ActualWidth();
						}
					}
				}
				else
				{
					// Fix negative bounds.
					available = 0.0;

					// Still need to determine a 'minimum' width (if available)
					// TODO: Consolidate this logic with above better?
					for (unsigned int i = 0; i < Items().Size(); i++)
					{
						auto item = Items().GetAt(i);
						auto tab = ContainerFromItem(item).try_as< NotePad::TabViewItem>();
						if (tab == nullptr)
						{
							continue; // container not generated yet
						}

						mintabwidth = std::min(mintabwidth, tab.MinWidth());
					}
				}
				if (!(mintabwidth < (std::numeric_limits<double>::max)()))
				{
					mintabwidth = 0.0; // No Containers, no visual, 0 size.
				}

				if (available > mintabwidth)
				{
					// Constrain the column based on our required and available space
					tabc.MaxWidth(available);
				}

				//// TODO: If it's less, should we move the selected tab to only be the one shown by default?

				//glength.GridUnitType = GridUnitType::Pixel;
				if (available <= mintabwidth || abs(available - mintabwidth) < std::numeric_limits<float>::epsilon())
				{
					tabc.Width(GridLengthHelper::FromPixels(mintabwidth));
				}
				else if (required >= available)
				{
					// Fix size as we don't have enough space for all the tabs.
					tabc.Width(GridLengthHelper::FromPixels(available));
				}
				else
				{
					// We haven't filled up our space, so we want to expand to take as much as needed.
					tabc.Width(GridLengthHelper::Auto());
				}
			}
		}
	}
	double TabView::ProvideEqualWidth(NotePad::TabViewItem tab, Windows::Foundation::IInspectable item, double availableWidth)
	{
		if (SelectedTabWidth() < 0)
		{
			if (Items().Size() <= 1)
			{
				return availableWidth;
			}

			return std::max(tab.MinWidth(), availableWidth / Items().Size());
		}
		else if (Items().Size() <= 1)
		{
			// Default case of a single tab, make it full size.
			return std::min(SelectedTabWidth(), availableWidth);
		}
		else
		{
			auto width = (availableWidth - SelectedTabWidth()) / (Items().Size() - 1);

			// Constrain between Min and Selected (Max)
			if (width < tab.MinWidth())
			{
				width = tab.MinWidth();
			}
			else if (width > SelectedTabWidth())
			{
				width = SelectedTabWidth();
			}

			// If it's selected make it full size, otherwise whatever the size should be.
			return tab.IsSelected()
				? std::min(SelectedTabWidth(), availableWidth)
				: width;
		}
	}
	double TabView::ProvideCompactWidth(NotePad::TabViewItem tab, Windows::Foundation::IInspectable item, double availableWidth)
	{
		// If we're selected and have a value for that, then just return that.
		if (tab.IsSelected() && SelectedTabWidth() > 0)
		{
			return SelectedTabWidth();
		}

		// Otherwise use std::min size.
		return tab.MinWidth();
	}
	void TabView::ItemContainerGenerator_ItemsChanged(Windows::Foundation::IInspectable sender, ItemsChangedEventArgs e)
	{
		auto action = (Windows::Foundation::Collections::CollectionChange)e.Action();
		if (action == Windows::Foundation::Collections::CollectionChange::Reset)
		{
			// Reset collection to reload later.
			_hasLoaded = false;
		}
	}
	void TabView::SetInitialSelection()
	{
		if (SelectedItem() == nullptr)
		{
			// If we have an index, but didn't get the selection, make the selection
			if (SelectedIndex() >= 0 && SelectedIndex() < Items().Size())
			{
				SelectedItem(Items().GetAt(SelectedIndex()));
			}

			// Otherwise, select the first item by default
			else if (Items().Size() >= 1)
			{
				SelectedItem(Items().GetAt(0));
			}
		}
	}
	int TabView::FindNextTabIndex(int startIndex, int direction)
	{
		int index = startIndex;
		if (direction != 0)
		{
			for (int i = 0; i < Items().Size(); i++)
			{
				index += direction;

				if (index >= Items().Size())
				{
					index = 0;
				}
				else if (index < 0)
				{
					index = Items().Size() - 1;
				}

				auto tabItem = ContainerFromIndex(index).try_as<NotePad::TabViewItem>();
				if (tabItem != nullptr && tabItem.IsEnabled() && tabItem.Visibility() == Visibility::Visible)
				{
					break;
				}
			}
		}

		return index;
	}
	void TabView::ItemsSource_PropertyChanged(DependencyObject sender, DependencyProperty dp)
	{
		// Use reflection to store a 'Remove' method of any possible collection in ItemsSource
		// Cache for efficiency later.
		if (ItemsSource() != nullptr)
		{
			_bremoveItemsSourceMethod = true;
		}
		else
		{
			_bremoveItemsSourceMethod = false;
		}
	}
}
