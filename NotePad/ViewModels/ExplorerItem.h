#pragma once

#include "ExplorerItem.g.h"
#include "BindableBase.h"
using namespace winrt::Windows::Foundation::Collections;
namespace winrt::NotePad::implementation
{
    struct ExplorerItem : ExplorerItemT<ExplorerItem, NotePad::implementation::BindableBase>
    {
		ExplorerItem() = default;

		Windows::Storage::StorageFile File() { return _File; };
		Windows::Storage::StorageFolder Folder() { return _Folder; };
		void File(Windows::Storage::StorageFile const& value) { _File = value; };
		void Folder(Windows::Storage::StorageFolder const& value) { _Folder = value; };

		NotePad::ExplorerItemType Type() { return _Type; };
		void Type(NotePad::ExplorerItemType const& value) { _Type = value; };
		IObservableVector<winrt::Windows::Foundation::IInspectable> Children() {
			if (_Children == nullptr)
			{
				_Children = winrt::single_threaded_observable_vector<winrt::Windows::Foundation::IInspectable>();
			}
			return _Children;
		};
		void Children(IObservableVector<winrt::Windows::Foundation::IInspectable> const& value) { _Children = value; };
		bool IsExpanded() {return _IsExpanded;};
		void IsExpanded(bool value) { 
			if (_IsExpanded!=value) {
				_IsExpanded = value;
				RaisePropertyChanged(L"IsExpanded");
			}
		};
        bool IsSelected() { return _IsSelected; };
        void IsSelected(bool value) {
			if (_IsSelected != value) {
				_IsSelected = value;
				RaisePropertyChanged(L"IsSelected");
			}
		};

	private:
		Windows::Storage::StorageFile _File{nullptr};
		Windows::Storage::StorageFolder _Folder{nullptr};
		ExplorerItemType _Type;
		IObservableVector<winrt::Windows::Foundation::IInspectable> _Children{nullptr};
		bool _IsExpanded;
		bool _IsSelected;
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct ExplorerItem : ExplorerItemT<ExplorerItem, implementation::ExplorerItem>
    {
    };
}
