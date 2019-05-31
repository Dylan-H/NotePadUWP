#pragma once

#include "ExplorerItemTemplateSelector.g.h"

namespace winrt::NotePad::implementation
{
	using namespace winrt::Windows::UI::Xaml;
    struct ExplorerItemTemplateSelector : ExplorerItemTemplateSelectorT<ExplorerItemTemplateSelector>
    {
        ExplorerItemTemplateSelector() = default;

		DataTemplate FolderTemplate() { return _FolderTemplate; };
		void FolderTemplate(DataTemplate const& value) {
			_FolderTemplate = value; 
		};
		DataTemplate FileTemplate() { return _FileTemplate; };
        void FileTemplate(DataTemplate const& value) { 
			_FileTemplate = value; 
		};
		DataTemplate SelectTemplateCore(winrt::Windows::Foundation::IInspectable  item)
		{
			//OutputDebugStringW(L"------------SelectTemplateCore\r\n");
			auto explorerItem = winrt::unbox_value<NotePad::ExplorerItem>(item);
			return explorerItem.Type() == ExplorerItemType::Folder ? FolderTemplate() : FileTemplate();
		}
		DataTemplate SelectTemplateCore(winrt::Windows::Foundation::IInspectable  item, DependencyObject container)
		{

			auto explorerItem = winrt::unbox_value<NotePad::ExplorerItem>(item);
			return explorerItem.Type() == ExplorerItemType::Folder ? FolderTemplate() : FileTemplate();
		}

	private:
		DataTemplate _FolderTemplate{nullptr};
		DataTemplate _FileTemplate{ nullptr };
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct ExplorerItemTemplateSelector : ExplorerItemTemplateSelectorT<ExplorerItemTemplateSelector, implementation::ExplorerItemTemplateSelector>
    {
    };
}
