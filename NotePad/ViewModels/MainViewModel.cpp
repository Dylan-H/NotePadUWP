#include "pch.h"

#include "MainViewModel.h"
#include <functional>
#include "ScintillaControl.h"
using namespace winrt::NotePad::implementation;
using namespace winrt::Windows::Foundation;

MainViewModel::MainViewModel() {
	m_OnNewFileCommand = make<DelegateCommand>(
		//std::bind(&MainViewModel::AnotherClick, this)
		std::bind(&MainViewModel::OnNewFileClick, this, std::placeholders::_1)
		//,[=](IInspectable const& parameter) {
		//	hstring value = parameter.as<IReference<hstring>>().Value();
		//	Title(value + L"Johan Lindfors");
		//}
		//,[=](IInspectable const& /*parameter*/) {
		//	return title != hstring(L"Click meJohan Lindfors");
		//}
	);

	m_OnOpenFileCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnOpenFileClick, this, std::placeholders::_1));
	m_OnRevertCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnRevertClick, this, std::placeholders::_1));
	m_OnCloseCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnCloseClick, this, std::placeholders::_1));
	m_OnSaveCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnSaveClick, this, std::placeholders::_1));
	m_OnSaveAsCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnSaveAsClick, this, std::placeholders::_1));
	m_OnEncodingCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnEncodingClick, this, std::placeholders::_1));
	m_OnExportCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnExportClick, this, std::placeholders::_1));
	m_OnExitCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnExitClick, this, std::placeholders::_1));
	//Edit																  
	m_OnUndoCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnUndoClick, this, std::placeholders::_1));
	m_OnRedoCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnRedoClick, this, std::placeholders::_1));
	m_OnCutCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnCutClick, this, std::placeholders::_1));
	m_OnPasteCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnPasteClick, this, std::placeholders::_1));
	m_OnDuplicateCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnDuplicateClick, this, std::placeholders::_1));
	m_OnDeleteCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnDeleteClick, this, std::placeholders::_1));
	m_OnSelectAllCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnSelectAllClick, this, std::placeholders::_1));
	m_OnCopyAsRTFCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnCopyAsRTFClick, this, std::placeholders::_1));

	m_OnAboutCommand = make<DelegateCommand>(std::bind(&MainViewModel::OnAboutClick, this, std::placeholders::_1));


	textSize = 150;
	_FolderDataSource = winrt::single_threaded_observable_vector<winrt::Windows::Foundation::IInspectable>();
	_OpenFileDataSource = winrt::single_threaded_observable_vector<winrt::Windows::Foundation::IInspectable>();
	_OpenFileTabDataSource = winrt::single_threaded_observable_vector<winrt::Windows::Foundation::IInspectable>();
	_FolderFileDoubleTapped = Windows::UI::Xaml::Input::DoubleTappedEventHandler(this,&MainViewModel::FolderTree_DoubleTapped);




}

void winrt::NotePad::implementation::MainViewModel::DeleteOpenFileCommand_ExecuteRequested(XamlUICommand sender, ExecuteRequestedEventArgs args)
{
	if (args.Parameter() != nullptr)
	{
		for (int i = 0; i< _OpenFileDataSource.Size(); i++)
		{
		    auto item = _OpenFileDataSource.GetAt(i).try_as<NotePad::OpenFileItem>();
		    if (item.File().Name() == (winrt::unbox_value<hstring>(args.Parameter())))
		    {
				_OpenFileDataSource.RemoveAt(i);
			    break;
		    }
		}
		for (int i = 0; i < _OpenFileTabDataSource.Size(); i++)
		{
			auto item = _OpenFileTabDataSource.GetAt(i).try_as<NotePad::TabViewItem>();
			hstring name = winrt::unbox_value<winrt::hstring>( item.Header());
			if (name == (winrt::unbox_value<hstring>(args.Parameter())))
			{
				_OpenFileTabDataSource.RemoveAt(i);
				break;
			}
		}
	}
	//if (ListViewRight.SelectedIndex != -1)
	//{
	//	collection.RemoveAt(ListViewRight.SelectedIndex);
	//}
}

void winrt::NotePad::implementation::MainViewModel::SetWindowName() {
	if (filePath.IsUntitled()) {
		windowName = localiser.Text("Untitled");
		windowName.insert(0, GUI_TEXT("("));
		windowName += GUI_TEXT(")");
	}
	else if (props.GetInt("title.full.path") == 2) {
		windowName = FileNameExt().AsInternal();
		windowName += GUI_TEXT(" ");
		windowName += localiser.Text("in");
		windowName += GUI_TEXT(" ");
		windowName += filePath.AsInternal();
	}
	else if (props.GetInt("title.full.path") == 1) {
		windowName = filePath.AsInternal();
	}
	else {
		windowName = FileNameExt().AsInternal();
	}
	if (CurrentBufferConst()->isDirty)
		windowName += GUI_TEXT(" * ");
	else
		windowName += GUI_TEXT(" - ");
	windowName += appName;

	if (buffers.length > 1 && props.GetInt("title.show.buffers")) {
		windowName += GUI_TEXT(" [");
		windowName += GUI::StringFromInteger(buffers.Current() + 1);
		windowName += GUI_TEXT(" ");
		windowName += localiser.Text("of");
		windowName += GUI_TEXT(" ");
		windowName += GUI::StringFromInteger(buffers.length);
		windowName += GUI_TEXT("]");
	}

	Title(windowName.c_str());
}


void winrt::NotePad::implementation::MainViewModel::AddOpenFile(Windows::Storage::StorageFile const &file)
{
	auto deleteCommand = Input::XamlUICommand(); //  Input::StandardUICommand(Input::StandardUICommandKind::Delete);
	SymbolIconSource icon;
	icon.Symbol(Symbol::Cancel);
	deleteCommand.IconSource(icon);
	deleteCommand.ExecuteRequested(TypedEventHandler<Input::XamlUICommand, Input::ExecuteRequestedEventArgs>(this, &MainViewModel::DeleteOpenFileCommand_ExecuteRequested));
	_OpenFileDataSource.Append(NotePad::OpenFileItem(file, deleteCommand));
	OpenAsync(FilePath(file));
	NotePad::TabViewItem tabItem;
	tabItem.Header(winrt::box_value(file.Name()));

	tabItem.Icon(SymbolIcon(Symbol::Document));
	tabItem.Content(winrt::box_value(L"This is new tab #"));
	_OpenFileTabDataSource.Append(winrt::box_value(tabItem));

}



winrt::hstring MainViewModel::Title() {
	return title;
}

void MainViewModel::Title(winrt::hstring const& value)
{
	if (title != value) {
		title = value;
		RaisePropertyChanged(L"Title");
		m_OnNewFileCommand.RaiseCanExecuteChanged();
	}
}

 void winrt::NotePad::implementation::MainViewModel::FolderName(hstring const & value) {
	 if (_FolderName != value) {
		 _FolderName = value;
		 RaisePropertyChanged(L"FolderName");
	 }
 }

double MainViewModel::TextSize() {
	return textSize;
}

 void winrt::NotePad::implementation::MainViewModel::SciCtrl(NotePad::ScintillaControl const & value) {
	sciCtrl = value;
	winrt::get_self<winrt::NotePad::implementation::ScintillaControl>(sciCtrl)->notify = std::bind(&MainViewModel::Notify, this, std::placeholders::_1);
}

winrt::Windows::Foundation::IAsyncAction winrt::NotePad::implementation::MainViewModel::Init()
{
	co_await ReadGlobalPropFile();
	SetScaleFactor(0);
	//tbLarge = props.GetInt("toolbar.large");
	/// Need to copy properties to variables before setting up window
	SetPropertiesInitial();
	//ReadAbbrevPropFile();
	CreateBuffers();
	wEditor.SetScintilla(sciCtrl);
	if (!buffers.initialised) {
		InitialiseBuffers();
	}
	co_return;
}


void MainViewModel::AnotherClick() {

}

void winrt::NotePad::implementation::MainViewModel::FolderTree_DoubleTapped(winrt::Windows::Foundation::IInspectable const & sender, DoubleTappedRoutedEventArgs const & e)
{

}

void winrt::NotePad::implementation::MainViewModel::OnNewFileClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnNewFileClick");
	dialog.ShowAsync();
}

winrt::Windows::Foundation::IAsyncAction winrt::NotePad::implementation::MainViewModel::doFolder(winrt::Windows::Storage::StorageFolder folder, NotePad::ExplorerItem folderitem)
{
	if (folder != nullptr)
	{
		for (auto const& folder : co_await folder.GetFoldersAsync())
		{
			NotePad::ExplorerItem folderItemChild;
			folderItemChild.Folder(folder);
			folderItemChild.Type(NotePad::ExplorerItemType::Folder);
			folderitem.Children().Append(folderItemChild);
			doFolder(folder, folderItemChild);
		}
		for (auto const& file : co_await folder.GetFilesAsync())
		{
			NotePad::ExplorerItem fileItem;
			fileItem.File(file);
			fileItem.Type(NotePad::ExplorerItemType::File);
			folderitem.Children().Append(fileItem);
		}
	}
}

winrt::Windows::Foundation::IAsyncAction winrt::NotePad::implementation::MainViewModel::OnOpenFileClick(Windows::Foundation::IInspectable const & parameter)
{
	//auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnOpenFileClick");
	//dialog.ShowAsync();
	auto folderPicker = winrt::Windows::Storage::Pickers::FolderPicker();
	folderPicker.SuggestedStartLocation ( winrt::Windows::Storage::Pickers::PickerLocationId::Desktop);
	folderPicker.FileTypeFilter().Append(L"*");

	winrt::Windows::Storage::StorageFolder folder =co_await folderPicker.PickSingleFolderAsync();
	if (folder != nullptr)
	{
		//NotePad::ExplorerItem folderItem;
		//folderItem.Folder(folder);
		//folderItem.Type(ExplorerItemType::Folder);
		FolderName(folder.Name());
		_FolderDataSource.Clear();

		for (auto const& folder : co_await folder.GetFoldersAsync())
		{
			NotePad::ExplorerItem folderItemChild;
			folderItemChild.Folder(folder);
			folderItemChild.Type(NotePad::ExplorerItemType::Folder);
			_FolderDataSource.Append(folderItemChild);
			doFolder(folder, folderItemChild);
		}
		for (auto const& file : co_await folder.GetFilesAsync())
		{
			NotePad::ExplorerItem fileItem;
			fileItem.File(file);
			fileItem.Type(NotePad::ExplorerItemType::File);
			_FolderDataSource.Append(fileItem);
		}

		//doFolder(folder, folderItem);

		//_FolderDataSource.Append(folderItem);

	}
}
void winrt::NotePad::implementation::MainViewModel::OnRevertClick(Windows::Foundation::IInspectable const & parameter)
{
	//auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnRevertClick");
	//dialog.ShowAsync();
	lineNumbers = !lineNumbers;
	SetLineNumberWidth();
}

void winrt::NotePad::implementation::MainViewModel::OnCloseClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnCloseClick");
	dialog.ShowAsync();
}

void winrt::NotePad::implementation::MainViewModel::OnSaveClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnSaveClick");
	dialog.ShowAsync();
}

void winrt::NotePad::implementation::MainViewModel::OnSaveAsClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnSaveAsClick");
	dialog.ShowAsync();
}

void winrt::NotePad::implementation::MainViewModel::OnEncodingClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnEncodingClick");
	dialog.ShowAsync();
}

void winrt::NotePad::implementation::MainViewModel::OnExportClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnExportClick");
	dialog.ShowAsync();
}

void winrt::NotePad::implementation::MainViewModel::OnExitClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnExitClick");
	dialog.ShowAsync();
}

void winrt::NotePad::implementation::MainViewModel::OnUndoClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnUndoClick");
	dialog.ShowAsync();
}

void winrt::NotePad::implementation::MainViewModel::OnRedoClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnRedoClick");
	dialog.ShowAsync();
}

void winrt::NotePad::implementation::MainViewModel::OnCutClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnCutClick");
	dialog.ShowAsync();
}

void winrt::NotePad::implementation::MainViewModel::OnPasteClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnPasteClick");
	dialog.ShowAsync();
}

void winrt::NotePad::implementation::MainViewModel::OnDuplicateClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnDuplicateClick");
	dialog.ShowAsync();
}

void winrt::NotePad::implementation::MainViewModel::OnDeleteClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnDeleteClick");
	dialog.ShowAsync();
}

void winrt::NotePad::implementation::MainViewModel::OnSelectAllClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnSelectAllClick");
	dialog.ShowAsync();
}

void winrt::NotePad::implementation::MainViewModel::OnCopyAsRTFClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnCopyAsRTFClick");
	dialog.ShowAsync();
}


void winrt::NotePad::implementation::MainViewModel::OnAboutClick(Windows::Foundation::IInspectable const & parameter)
{
	auto dialog = winrt::Windows::UI::Popups::MessageDialog(L"OnAboutClick");
	dialog.ShowAsync();
}


void  winrt::NotePad::implementation::MainViewModel::FindMessageBox(const std::string & msg, const std::string * findItem)
{
}
void  winrt::NotePad::implementation::MainViewModel::AboutDialog()
{
}
void  winrt::NotePad::implementation::MainViewModel::FindIncrement()
{
}
void  winrt::NotePad::implementation::MainViewModel::Find()
{
}
void  winrt::NotePad::implementation::MainViewModel::FindInFiles()
{
}
void  winrt::NotePad::implementation::MainViewModel::Replace()
{
}
void  winrt::NotePad::implementation::MainViewModel::FindReplace(bool replace)
{
}
void  winrt::NotePad::implementation::MainViewModel::DestroyFindReplace()
{
}
void  winrt::NotePad::implementation::MainViewModel::SetMenuItem(int menuNumber, int position, int itemID, const GUI::gui_char * text, const GUI::gui_char * mnemonic)
{
}
void  winrt::NotePad::implementation::MainViewModel::RedrawMenu()
{
}
void  winrt::NotePad::implementation::MainViewModel::DestroyMenuItem(int menuNumber, int itemID)
{
}
void  winrt::NotePad::implementation::MainViewModel::CheckAMenuItem(int wIDCheckItem, bool val)
{
}
void  winrt::NotePad::implementation::MainViewModel::EnableAMenuItem(int wIDCheckItem, bool val)
{
}
void  winrt::NotePad::implementation::MainViewModel::CheckMenus()
{
	if (buffers.size() > 0) {
		
		TabSelect(buffers.Current());
	}
}
void  winrt::NotePad::implementation::MainViewModel::QuitProgram()
{
}
void  winrt::NotePad::implementation::MainViewModel::Execute()
{
}
void  winrt::NotePad::implementation::MainViewModel::StopExecute()
{
}
void  winrt::NotePad::implementation::MainViewModel::AddCommand(const std::string & cmd, const std::string & dir, JobSubsystem jobType, const std::string & input, int flags)
{
}
void  winrt::NotePad::implementation::MainViewModel::AddToPopUp(const char * label, int cmd, bool enabled)
{
}
std::string  winrt::NotePad::implementation::MainViewModel::EncodeString(const std::string & s)
{
	return std::string();
}
INT_PTR  winrt::NotePad::implementation::MainViewModel::GoLineDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return INT_PTR();
}
void  winrt::NotePad::implementation::MainViewModel::GoLineDialog()
{
}
BOOL  winrt::NotePad::implementation::MainViewModel::AbbrevMessage(HWND hDlg, UINT message, WPARAM wParam)
{
	return 0;
}
INT_PTR  winrt::NotePad::implementation::MainViewModel::AbbrevDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return INT_PTR();
}
bool  winrt::NotePad::implementation::MainViewModel::AbbrevDialog()
{
	return false;
}
BOOL  winrt::NotePad::implementation::MainViewModel::TabSizeMessage(HWND hDlg, UINT message, WPARAM wParam)
{
	return 0;
}
INT_PTR  winrt::NotePad::implementation::MainViewModel::TabSizeDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return INT_PTR();
}
void  winrt::NotePad::implementation::MainViewModel::TabSizeDialog()
{
}
bool  winrt::NotePad::implementation::MainViewModel::ParametersOpen()
{
	return false;
}
void  winrt::NotePad::implementation::MainViewModel::ParamGrab()
{
}
bool  winrt::NotePad::implementation::MainViewModel::ParametersDialog(bool modal)
{
	return false;
}
void  winrt::NotePad::implementation::MainViewModel::TabInsert(int index, const GUI::gui_char * title)
{
}
void  winrt::NotePad::implementation::MainViewModel::TabSelect(int index)
{
	TabIndex(index);
}
void  winrt::NotePad::implementation::MainViewModel::RemoveAllTabs()
{
}
void  winrt::NotePad::implementation::MainViewModel::WarnUser(int warnID)
{
}
void  winrt::NotePad::implementation::MainViewModel::Notify(SCNotification * notification)
{
	SciTEBase::Notify(notification);
}
void  winrt::NotePad::implementation::MainViewModel::ShowToolBar()
{
}
void  winrt::NotePad::implementation::MainViewModel::ShowTabBar()
{
}
void  winrt::NotePad::implementation::MainViewModel::ShowStatusBar()
{
}
void  winrt::NotePad::implementation::MainViewModel::ActivateWindow(const char * timestamp)
{
}
void  winrt::NotePad::implementation::MainViewModel::ExecuteHelp(const char * cmd)
{
}
void  winrt::NotePad::implementation::MainViewModel::ExecuteOtherHelp(const char * cmd)
{
}
void  winrt::NotePad::implementation::MainViewModel::CopyAsRTF()
{
}
void  winrt::NotePad::implementation::MainViewModel::CopyPath()
{
}
void winrt::NotePad::implementation::MainViewModel::SetScaleFactor(int scale)
{
	scale = 100;
	//if (scale == 0) {
	//	HDC hdcMeasure = ::CreateCompatibleDC(NULL);
	//	scale = ::GetDeviceCaps(hdcMeasure, LOGPIXELSX) * 100 / 96;
	//	::DeleteDC(hdcMeasure);
	//}
	std::string sScale = StdStringFromInteger(scale);
	propsPlatform.Set("ScaleFactor", sScale.c_str());
}
bool  winrt::NotePad::implementation::MainViewModel::OpenDialog(const FilePath & directory, const GUI::gui_char * filesFilter)
{
	return false;
}
bool  winrt::NotePad::implementation::MainViewModel::SaveAsDialog()
{
	return false;
}
void  winrt::NotePad::implementation::MainViewModel::SaveACopy()
{
}
void  winrt::NotePad::implementation::MainViewModel::SaveAsHTML()
{
}
void  winrt::NotePad::implementation::MainViewModel::SaveAsRTF()
{
}
void  winrt::NotePad::implementation::MainViewModel::SaveAsPDF()
{
}
void  winrt::NotePad::implementation::MainViewModel::SaveAsTEX()
{
}
void  winrt::NotePad::implementation::MainViewModel::SaveAsXML()
{
}
void  winrt::NotePad::implementation::MainViewModel::LoadSessionDialog()
{
}
void  winrt::NotePad::implementation::MainViewModel::SaveSessionDialog()
{
}
bool  winrt::NotePad::implementation::MainViewModel::PreOpenCheck(const GUI::gui_char * arg)
{
	return false;
}
bool  winrt::NotePad::implementation::MainViewModel::IsStdinBlocked()
{
	return false;
}
void  winrt::NotePad::implementation::MainViewModel::GetWindowPosition(int * left, int * top, int * width, int * height, int * maximize)
{
}
FilePath  winrt::NotePad::implementation::MainViewModel::GetDefaultDirectory()
{
	return FilePath();
}
FilePath  winrt::NotePad::implementation::MainViewModel::GetSciteDefaultHome()
{
	return FilePath();
}
FilePath  winrt::NotePad::implementation::MainViewModel::GetSciteUserHome()
{
	return FilePath();
}
winrt::Windows::Foundation::IAsyncAction   winrt::NotePad::implementation::MainViewModel::ReadEmbeddedProperties()
{
	propsEmbed.Clear();


	StorageFile file = co_await  StorageFile::GetFileFromApplicationUriAsync(Uri(L"ms-appx:///Assets/Embedded.properties"));


	winrt::Windows::Storage::Streams::IBuffer buf = co_await winrt::Windows::Storage::FileIO::ReadBufferAsync(file);
	if (buf != nullptr)
	{
		uint8_t* bytes= GetBufferData(buf);
		if (bytes!=nullptr) {
			propsEmbed.ReadFromMemory((const char *)(bytes), buf.Length(), FilePath(), filter, NULL, 0);
		}
	}
	co_return;
}
void  winrt::NotePad::implementation::MainViewModel::ReadPropertiesInitial()
{
	SciTEBase::ReadPropertiesInitial();
}
void  winrt::NotePad::implementation::MainViewModel::ReadProperties()
{
	SciTEBase::ReadProperties();
}
void  winrt::NotePad::implementation::MainViewModel::TimerStart(int mask)
{
}
void  winrt::NotePad::implementation::MainViewModel::TimerEnd(int mask)
{
}
void  winrt::NotePad::implementation::MainViewModel::ShowOutputOnMainThread()
{
}
void  winrt::NotePad::implementation::MainViewModel::SizeContentWindows()
{
}
void  winrt::NotePad::implementation::MainViewModel::SizeSubWindows()
{
}
void  winrt::NotePad::implementation::MainViewModel::SetFileProperties(PropSetFile & ps)
{
}
void  winrt::NotePad::implementation::MainViewModel::SetStatusBarText(const char * s)
{
}
