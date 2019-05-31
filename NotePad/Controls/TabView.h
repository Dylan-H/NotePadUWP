#pragma once

#include "TabView.g.h"
#include <string>
#include <algorithm>
#include <limits>

namespace winrt::NotePad::implementation
{
	using namespace winrt::Windows::UI::Xaml;
	using namespace winrt::Windows::UI::Xaml::Input;
	using namespace winrt::Windows::UI::Xaml::Controls::Primitives;
	using namespace winrt::Windows::Devices::Input;
	using namespace winrt::Windows::UI::Input;
	using namespace winrt::Windows::UI::Xaml::Controls;
	using namespace winrt::Windows::UI::Xaml::Media;
	using namespace winrt::Windows::UI::Xaml::Data;
	using namespace winrt::Windows::ApplicationModel::DataTransfer;
    struct TabView : TabViewT<TabView>
    {
		TabView();
		Windows::Foundation::IInspectable TabStartHeader() {return  GetValue(_TabStartHeaderProperty); };
		void TabStartHeader(Windows::Foundation::IInspectable const& value) {SetValue(_TabStartHeaderProperty, value);};
		DataTemplate TabStartHeaderTemplate(){return winrt::unbox_value<DataTemplate> (GetValue(_TabStartHeaderTemplateProperty));};
		void TabStartHeaderTemplate(DataTemplate const& value) { SetValue(_TabStartHeaderTemplateProperty, winrt::box_value(value)); };
		Windows::Foundation::IInspectable TabActionHeader() { return GetValue(_TabActionHeaderProperty); };
		void TabActionHeader(Windows::Foundation::IInspectable const& value) { SetValue(_TabActionHeaderProperty, value); };
		DataTemplate TabActionHeaderTemplate() { return winrt::unbox_value<DataTemplate>(GetValue(_TabActionHeaderTemplateProperty)); };
		void TabActionHeaderTemplate(DataTemplate const& value) { SetValue(_TabActionHeaderTemplateProperty, winrt::box_value(value)); };
		Windows::Foundation::IInspectable TabEndHeader() { return GetValue(_TabEndHeaderProperty); };
		void TabEndHeader(Windows::Foundation::IInspectable const& value) { SetValue(_TabEndHeaderProperty, value); };
		DataTemplate TabEndHeaderTemplate() { return winrt::unbox_value<DataTemplate>(GetValue(_TabEndHeaderTemplateProperty)); };
		void TabEndHeaderTemplate(DataTemplate const& value) { SetValue(_TabEndHeaderTemplateProperty, winrt::box_value(value)); };
		DataTemplate ItemHeaderTemplate() { return winrt::unbox_value<DataTemplate>(GetValue(_ItemHeaderTemplateProperty)); };
		void ItemHeaderTemplate(DataTemplate const& value) { SetValue(_ItemHeaderTemplateProperty, winrt::box_value(value)); };
		bool CanCloseTabs() { return winrt::unbox_value<bool>(GetValue(_CanCloseTabsProperty)); };
		void CanCloseTabs(bool value) { SetValue(_CanCloseTabsProperty, winrt::box_value(value)); };
		bool IsCloseButtonOverlay() { return winrt::unbox_value<bool>(GetValue(_IsCloseButtonOverlayProperty)); };
		void IsCloseButtonOverlay(bool value){ SetValue(_IsCloseButtonOverlayProperty, winrt::box_value(value));};
		double SelectedTabWidth() { return winrt::unbox_value<double>(GetValue(_SelectedTabWidthProperty)); }
		void SelectedTabWidth(double value) { SetValue(_SelectedTabWidthProperty, winrt::box_value(value)); };
		NotePad::TabWidthMode TabWidthBehavior() { return winrt::unbox_value<NotePad::TabWidthMode>(GetValue(_TabWidthBehaviorProperty)); }
		void TabWidthBehavior(NotePad::TabWidthMode const& value) { SetValue(_TabWidthBehaviorProperty, winrt::box_value(value)); };

		static DependencyProperty TabStartHeaderProperty() { return _TabStartHeaderProperty; };
		static DependencyProperty TabStartHeaderTemplateProperty() { return _TabStartHeaderTemplateProperty; };
		static DependencyProperty TabActionHeaderProperty() { return _TabActionHeaderProperty; };
		static DependencyProperty TabActionHeaderTemplateProperty(){ return _TabActionHeaderTemplateProperty; };
		static DependencyProperty TabEndHeaderProperty(){ return _TabEndHeaderProperty; };
		static DependencyProperty TabEndHeaderTemplateProperty(){ return _TabEndHeaderTemplateProperty; };
		static DependencyProperty ItemHeaderTemplateProperty(){ return _ItemHeaderTemplateProperty; };
		static DependencyProperty CanCloseTabsProperty(){ return _CanCloseTabsProperty; };
		static DependencyProperty IsCloseButtonOverlayProperty(){ return _IsCloseButtonOverlayProperty; };
		static DependencyProperty SelectedTabWidthProperty(){ return _SelectedTabWidthProperty; };
		static DependencyProperty TabWidthBehaviorProperty(){ return _TabWidthBehaviorProperty;};
		static bool GetIgnoreColumn(Controls::ColumnDefinition const& obj) { return winrt::unbox_value<bool>(obj.GetValue(_IgnoreColumnProperty)); };
		static void SetIgnoreColumn(Controls::ColumnDefinition const& obj, bool value) { obj.SetValue(_IgnoreColumnProperty, winrt::box_value(value)); };
		static DependencyProperty IgnoreColumnProperty() { return _IgnoreColumnProperty; };
		static bool GetConstrainColumn(Controls::ColumnDefinition const& obj) { return winrt::unbox_value<bool>(obj.GetValue(_ConstrainColumnProperty)); };
		static void SetConstrainColumn(Controls::ColumnDefinition const& obj, bool value) { obj.SetValue(_ConstrainColumnProperty, winrt::box_value(value)); };
		static DependencyProperty ConstrainColumnProperty() { return _ConstrainColumnProperty; };
		static DependencyProperty OriginalWidthProperty() { return _OriginalWidthProperty; };
		static double GetOriginalWidth(NotePad::TabViewItem const& obj) { return winrt::unbox_value<double>(obj.GetValue(_OriginalWidthProperty)); }
		static void SetOriginalWidth(NotePad::TabViewItem const& obj, double value) { obj.SetValue(_OriginalWidthProperty, winrt::box_value(value)); };




		winrt::event<Windows::Foundation::EventHandler<Windows::Foundation::IInspectable>> TabDraggedOutside;
		winrt::event<Windows::Foundation::EventHandler<Windows::Foundation::IInspectable>> TabClosing;
		DependencyObject GetContainerForItemOverride() { return  TabViewItem(); };
		bool IsItemItsOwnContainerOverride(Windows::Foundation::IInspectable  item);
		void OnApplyTemplate();
		void OnItemsChanged(Windows::Foundation::IInspectable  e);
		void ScrollViewer_Loaded(Windows::Foundation::IInspectable sender, RoutedEventArgs e);

		FrameworkElement FindDescendantByName(DependencyObject element, hstring name);

		void PrepareContainerForItemOverride(DependencyObject element, Windows::Foundation::IInspectable item);
	private:
		NotePad::TabViewItem FindParent(FrameworkElement const & element);

		void ScrollTabBackButton_Click(Windows::Foundation::IInspectable  sender, RoutedEventArgs e);
		void ScrollTabForwardButton_Click(Windows::Foundation::IInspectable sender, RoutedEventArgs e);
		void TabView_SelectionChanged(Windows::Foundation::IInspectable sender, SelectionChangedEventArgs e);
		void TabViewItem_Loaded(Windows::Foundation::IInspectable sender, RoutedEventArgs e);
		void TabViewItem_Closing(Windows::Foundation::IInspectable sender, Windows::Foundation::IInspectable e);
		void TabView_DragItemsStarting(Windows::Foundation::IInspectable sender, DragItemsStartingEventArgs e);
		void TabView_DragItemsCompleted(ListViewBase sender, DragItemsCompletedEventArgs args);

		static void OnLayoutEffectingPropertyChanged(DependencyObject sender, DependencyPropertyChangedEventArgs args);

		void TabView_SizeChanged(Windows::Foundation::IInspectable sender, SizeChangedEventArgs e);

		double ProvideEqualWidth(NotePad::TabViewItem tab, Windows::Foundation::IInspectable item, double availableWidth);

		double ProvideCompactWidth(NotePad::TabViewItem tab, Windows::Foundation::IInspectable item, double availableWidth);

		void ItemContainerGenerator_ItemsChanged(Windows::Foundation::IInspectable sender, ItemsChangedEventArgs e);
		void SetInitialSelection();
		int FindNextTabIndex(int startIndex, int direction);
		void ItemsSource_PropertyChanged(DependencyObject sender, DependencyProperty dp);
	private:
		static DependencyProperty _TabStartHeaderProperty;
		static DependencyProperty _TabStartHeaderTemplateProperty;
		static DependencyProperty _TabActionHeaderProperty;
		static DependencyProperty _TabActionHeaderTemplateProperty;
		static DependencyProperty _TabEndHeaderProperty;
		static DependencyProperty _TabEndHeaderTemplateProperty;
		static DependencyProperty _ItemHeaderTemplateProperty;
		static DependencyProperty _CanCloseTabsProperty;
		static DependencyProperty _IsCloseButtonOverlayProperty;
		static DependencyProperty _SelectedTabWidthProperty;
		static DependencyProperty _TabWidthBehaviorProperty;
		static DependencyProperty _IgnoreColumnProperty;
		static DependencyProperty _ConstrainColumnProperty;
		static DependencyProperty _OriginalWidthProperty;


		winrt::event_token TabViewItem_ClosingToken;
		winrt::event_token TabViewItem_LoadedToken;
		winrt::event_token TabView_SizeChangedToken;
		winrt::event_token ScrollViewer_LoadedToken;
		winrt::event_token ScrollTabBackButton_ClickToken;
		winrt::event_token ScrollTabForwardButton_ClickToken;
		winrt::event_token ItemContainerGenerator_ItemsChangedToken;
		winrt::event_token TabView_DragItemsStartingToken;
		winrt::event_token TabView_DragItemsCompletedToken;
		winrt::event_token TabView_SelectionChangedToken;

		const int ScrollAmount = 50; // TODO: Should this be based on TabWidthMode
		const hstring TabContentPresenterName = L"TabContentPresenter";
		const hstring TabViewContainerName = L"TabViewContainer";
		const hstring TabsItemsPresenterName = L"TabsItemsPresenter";
		const hstring TabsScrollViewerName = L"ScrollViewer";
		const hstring TabsScrollBackButtonName = L"ScrollBackButton";
		const hstring TabsScrollForwardButtonName = L"ScrollForwardButton";
		ContentPresenter _tabContentPresenter{nullptr};
		Grid _tabViewContainer{ nullptr };
		ItemsPresenter _tabItemsPresenter{ nullptr };
		ScrollViewer _tabScroller{ nullptr };
		ButtonBase _tabScrollBackButton{ nullptr };
		ButtonBase _tabScrollForwardButton{ nullptr };
		bool _hasLoaded=false;
		bool _isDragging=false;
		bool _bremoveItemsSourceMethod=false;
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct TabView : TabViewT<TabView, implementation::TabView>
    {

    };
}
