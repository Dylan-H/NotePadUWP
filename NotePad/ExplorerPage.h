//
// Declaration of the ExplorerPage class.
//

#pragma once

#include "ExplorerPage.g.h"

namespace winrt::NotePad::implementation
{
	using namespace winrt::Windows::UI::Xaml;
	using namespace winrt::Windows::UI::Xaml::Navigation;
	using namespace winrt::Windows::Foundation::Collections;
    struct ExplorerPage : ExplorerPageT<ExplorerPage>
    {
        ExplorerPage();
		IObservableVector<winrt::Windows::Foundation::IInspectable> FolderDataSource() { return _FolderDataSource; };
		IObservableVector<Windows::Foundation::IInspectable> OpenFileDataSource() { return _OpenFileDataSource; };
		NotePad::MainViewModel MainViewModel(){ return _MainViewModel; };
		void FolderTree_DoubleTapped(winrt::Windows::Foundation::IInspectable const& sender, DoubleTappedRoutedEventArgs  const&e);
		void ListViewSwipeContainer_PointerEntered(winrt::Windows::Foundation::IInspectable const& sender, PointerRoutedEventArgs const&e);
		void ListViewSwipeContainer_PointerExited(winrt::Windows::Foundation::IInspectable const&  sender, PointerRoutedEventArgs const& e);
		void OnNavigatedTo(NavigationEventArgs  e) ;
	private:
		IObservableVector<winrt::Windows::Foundation::IInspectable>  _FolderDataSource;
		IObservableVector<winrt::Windows::Foundation::IInspectable> _OpenFileDataSource;
		NotePad::MainViewModel _MainViewModel{nullptr};
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct ExplorerPage : ExplorerPageT<ExplorerPage, implementation::ExplorerPage>
    {
    };
}
