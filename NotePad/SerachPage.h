//
// Declaration of the SerachPage class.
//

#pragma once

#include "SerachPage.g.h"

namespace winrt::NotePad::implementation
{
    struct SerachPage : SerachPageT<SerachPage>
    {
        SerachPage();
		
		void btnReplace_Checked(winrt::Windows::Foundation::IInspectable const& sender, RoutedEventArgs const& e);

		void btnReplace_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, RoutedEventArgs const& e);


    };
}

namespace winrt::NotePad::factory_implementation
{
    struct SerachPage : SerachPageT<SerachPage, implementation::SerachPage>
    {
    };
}
