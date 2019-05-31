#pragma once

#include "OpenFileItem.g.h"
#include "BindableBase.h"

namespace winrt::NotePad::implementation
{
    struct OpenFileItem : OpenFileItemT<OpenFileItem, NotePad::implementation::BindableBase>
    {
		OpenFileItem(Windows::Storage::StorageFile file, Windows::UI::Xaml::Input::ICommand command) {
			_File = file;
			_Command = command;
		};

		Windows::Storage::StorageFile File() { return _File; };
        void File(Windows::Storage::StorageFile const& value) {  _File=value; };
		Windows::UI::Xaml::Input::ICommand Command() { return _Command; };
		void Command(Windows::UI::Xaml::Input::ICommand const& value) { _Command = value; };
	private:
		Windows::Storage::StorageFile _File{nullptr};
		Windows::UI::Xaml::Input::ICommand _Command{ nullptr };
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct OpenFileItem : OpenFileItemT<OpenFileItem, implementation::OpenFileItem>
    {
    };
}
