#include "pch.h"
#include "MainPage.h"

using namespace winrt;
using namespace Windows::UI::Xaml;



namespace winrt::NotePad::implementation
{
    MainPage::MainPage()
    {
		m_ViewModel = make<NotePad::implementation::MainViewModel>();
		DataContext(ViewModel());
		customTitleBar.ViewModel(m_ViewModel);
        InitializeComponent();
		CoreApplication::GetCurrentView().TitleBar().ExtendViewIntoTitleBar(true);
		//Window::Current().SetTitleBar(BackgroundElement());
		auto titleBar = CoreApplication::GetCurrentView().TitleBar();
		UIElement mainContent = Content(); //原来窗口上的元素
		Content(nullptr);
		customTitleBar.SetPageContent(mainContent);//把原来窗口上的元素放到自定义控件grid 的第二行 里面

		Content(customTitleBar);  //让窗口显示内容变成了自定义控件
		
    }
	void MainPage::MainPage_Loaded(IInspectable const & sender, RoutedEventArgs e)
	{
		m_ViewModel.SciCtrl(sciCtrl());
		
		Windows::UI::Xaml::Interop::TypeName pageTypeName;
		pageTypeName.Name = L"NotePad.ExplorerPage";
		pageTypeName.Kind = Windows::UI::Xaml::Interop::TypeKind::Primitive;
		ContentFrame().Navigate(pageTypeName, m_ViewModel);
		m_ViewModel.Init();
	}
	 void MainPage::OnContentFrameNavigated(IInspectable const & sender, NavigationEventArgs e)
	{
	
	}
	 void MainPage::OnNavigationViewItemInvoked(NavigationView const & sender, NavigationViewItemInvokedEventArgs args)
	 {
		 if (args.IsSettingsInvoked())
		 {
			 // Navigate to Settings.
		 }
		 else if (args.InvokedItemContainer())
		 {
			 Windows::UI::Xaml::Interop::TypeName pageTypeName;
			 pageTypeName.Name = unbox_value<hstring>(args.InvokedItemContainer().Tag());
			 pageTypeName.Kind = Windows::UI::Xaml::Interop::TypeKind::Primitive;
			 ContentFrame().Navigate(pageTypeName, m_ViewModel);
		 }
	 }

	 void MainPage::OnTabItemClick(winrt::Windows::Foundation::IInspectable const & sender, ItemClickEventArgs const & e)
	 {
		 auto tb= sender.try_as<winrt::NotePad::TabView>();
		 int index= tb.SelectedIndex();
		// m_ViewModel.SetDocumentAt(0);
	 }

	 void MainPage::TabView_SelectionChanged(Windows::Foundation::IInspectable sender, SelectionChangedEventArgs e)
	 {
		 auto tb = sender.try_as<winrt::NotePad::TabView>();
		 int index = tb.SelectedIndex();
		 winrt::get_self<MainViewModel>(m_ViewModel)->TabViewSelectionChanged(index);
	 }
	
}
