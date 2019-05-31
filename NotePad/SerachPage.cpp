#include "pch.h"
#include "SerachPage.h"



namespace winrt::NotePad::implementation
{
	using namespace winrt;
	using namespace Windows::UI::Xaml;

    SerachPage::SerachPage()
    {
        InitializeComponent();
    }

	void SerachPage::btnReplace_Checked(winrt::Windows::Foundation::IInspectable const & sender, RoutedEventArgs const & e)
	{
		bool ret= VisualStateManager::GoToState(btnReplace(), L"VisibleDown", true);

	}

	void SerachPage::btnReplace_Unchecked(winrt::Windows::Foundation::IInspectable const & sender, RoutedEventArgs const & e)
	{
		bool ret = VisualStateManager::GoToState(btnReplace(), L"VisibleNormal", true);
	}


}
