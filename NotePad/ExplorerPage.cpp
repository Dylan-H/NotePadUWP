#include "pch.h"
#include "ExplorerPage.h"
#include <sstream>
using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::Foundation::Metadata;
namespace winrt::NotePad::implementation
{
    ExplorerPage::ExplorerPage()
    {	
		//CoreApplication::GetCurrentView().TitleBar().ExtendViewIntoTitleBar(false);
		DataContext(*this);
        InitializeComponent();
    }

	void ExplorerPage::FolderTree_DoubleTapped(winrt::Windows::Foundation::IInspectable const & sender, DoubleTappedRoutedEventArgs const & e)
	{
		auto sp =sender.try_as<StackPanel>();
		auto file = winrt::unbox_value<Windows::Storage::StorageFile>(sp.Tag());
		_MainViewModel.AddOpenFile(file);

	}

	void ExplorerPage::ListViewSwipeContainer_PointerEntered(winrt::Windows::Foundation::IInspectable const & sender, PointerRoutedEventArgs const & e)
	{
		if (e.Pointer().PointerDeviceType() == winrt::Windows::Devices::Input::PointerDeviceType::Mouse ||
			e.Pointer().PointerDeviceType() == winrt::Windows::Devices::Input::PointerDeviceType::Pen)
		{
			VisualStateManager::GoToState(sender.try_as<Control>(), L"HoverButtonsShown", true);
		}
	}

	  void ExplorerPage::ListViewSwipeContainer_PointerExited(winrt::Windows::Foundation::IInspectable const & sender, PointerRoutedEventArgs const & e)
	 {
		 VisualStateManager::GoToState(sender.try_as<Control>(), L"HoverButtonsHidden", true);
	 }

	   void ExplorerPage::OnNavigatedTo(NavigationEventArgs  e)
	   {
		   _MainViewModel = e.Parameter().try_as<NotePad::MainViewModel>() ;
		   if(_FolderDataSource==nullptr)
			_FolderDataSource = _MainViewModel.FolderDataSource();
		   if(_OpenFileDataSource==nullptr)
			_OpenFileDataSource = _MainViewModel.OpenFileDataSource();

	   }


}
