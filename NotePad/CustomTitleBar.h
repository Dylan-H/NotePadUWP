//
// Declaration of the CustomTitleBar class.
//
#pragma once
#include "CustomTitleBar.g.h"


namespace winrt::NotePad::implementation
{
	using namespace winrt::Windows::UI::Xaml;
	using namespace winrt::Windows::UI::Xaml::Data;
	using namespace winrt::Windows::ApplicationModel::Core;
	using namespace winrt::Windows::UI::Core;
	using namespace winrt::Windows::UI::ViewManagement;
	using namespace winrt::Windows::Foundation;
    struct CustomTitleBar : CustomTitleBarT<CustomTitleBar>
    {
        CustomTitleBar();
		double CoreTitleBarHeight();
		Thickness CoreTitleBarPadding();;
		winrt::event_token PropertyChanged(PropertyChangedEventHandler const& handler);
		void PropertyChanged(winrt::event_token const& token) noexcept;
		void SetPageContent(UIElement newContent);

		void removePageContent(UIElement con);
		void UserControl_Loaded(IInspectable const& sender, RoutedEventArgs e);
		void UserControl_Unloaded(IInspectable const& sender, RoutedEventArgs e);
		void ViewModel(NotePad::MainViewModel const& value) {
			m_ViewModel = value;
			DataContext(value);
		};
		NotePad::MainViewModel ViewModel() { return m_ViewModel; };
	private:

		void OnLayoutMetricsChanged(CoreApplicationViewTitleBar sender, IInspectable const&  e);

		void UpdateLayoutMetrics();

		void OnIsVisibleChanged(CoreApplicationViewTitleBar sender, IInspectable const& e);

		void OnWindowSizeChanged(IInspectable const& sender, WindowSizeChangedEventArgs e);

		void UpdatePositionAndVisibility();

	private:
		winrt::event_token OnWindowSizeChangedToken;
		winrt::event_token LayoutMetricsChangedToken;
		winrt::event_token IsVisibleChangedToken;
		CoreApplicationViewTitleBar coreTitleBar = CoreApplication::GetCurrentView().TitleBar();
		winrt::event<PropertyChangedEventHandler> _PropertyChanged;
		NotePad::MainViewModel m_ViewModel{ nullptr };
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct CustomTitleBar : CustomTitleBarT<CustomTitleBar, implementation::CustomTitleBar>
    {
    };
}
