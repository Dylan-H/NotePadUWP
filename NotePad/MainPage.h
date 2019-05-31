//
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "CustomTitleBar.h"


namespace winrt::NotePad::implementation
{
	using namespace winrt::Windows::UI::Xaml::Navigation;
	using namespace winrt::Windows::UI::Xaml::Controls;

    struct MainPage : MainPageT<MainPage>
    {
        MainPage();
		void MainPage_Loaded(IInspectable const& sender, RoutedEventArgs e);
		void OnContentFrameNavigated(IInspectable const& sender, NavigationEventArgs e);
		void OnNavigationViewItemInvoked(NavigationView const& sender, NavigationViewItemInvokedEventArgs args);
		NotePad::MainViewModel ViewModel() { return m_ViewModel; };
		void OnTabItemClick(winrt::Windows::Foundation::IInspectable const& sender, ItemClickEventArgs const&e);
		void TabView_SelectionChanged(Windows::Foundation::IInspectable sender, SelectionChangedEventArgs e);
	private:
		NotePad::MainViewModel m_ViewModel{ nullptr };
		NotePad::CustomTitleBar customTitleBar;
	
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
