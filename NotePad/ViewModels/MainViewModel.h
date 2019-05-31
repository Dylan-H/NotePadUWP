#pragma once

#include "MainViewModel.g.h"
#include "BindableBase.h"
#include "DelegateCommand.h"
#include "SciTEBase.h"

namespace winrt::NotePad::implementation
{
	struct MainViewModel : MainViewModelT<MainViewModel, NotePad::implementation::BindableBase>,  public SciTEBase
	{
		MainViewModel();
		Windows::Foundation::Collections::IObservableVector<Windows::Foundation::IInspectable> FolderDataSource() {
			return _FolderDataSource;
		};
		Windows::Foundation::Collections::IObservableVector<Windows::Foundation::IInspectable> OpenFileDataSource() {
			return _OpenFileDataSource;
		};
		Windows::Foundation::Collections::IObservableVector<Windows::Foundation::IInspectable> OpenFileTabDataSource() {
			return _OpenFileTabDataSource;
		};
		void DeleteOpenFileCommand_ExecuteRequested(XamlUICommand sender, ExecuteRequestedEventArgs args);
		void AddOpenFile(Windows::Storage::StorageFile const&file);
		Windows::Foundation::IAsyncAction doFolder(winrt::Windows::Storage::StorageFolder folder, NotePad::ExplorerItem folderitem);
		hstring Title();
		void Title(hstring const& value);
		hstring FolderName() {
			return _FolderName;
		};
		void FolderName(hstring const& value);;
		double TextSize();
		int32_t TabIndex() { return _tabIndex; };
		void TabIndex(int32_t value) { 
		if (_tabIndex != value) {
			_tabIndex = value;
			RaisePropertyChanged(L"TabIndex");
		}
		};
		NotePad::ScintillaControl SciCtrl() { return sciCtrl; };
		void SciCtrl(NotePad::ScintillaControl const& value);;
		Windows::Foundation::IAsyncAction Init();

		//File
		NotePad::DelegateCommand OnNewFileCommand(){ return m_OnNewFileCommand; };
		NotePad::DelegateCommand OnOpenFileCommand() { return m_OnOpenFileCommand; };
		NotePad::DelegateCommand OnRevertCommand(){ return m_OnRevertCommand; };
		NotePad::DelegateCommand OnCloseCommand(){ return m_OnCloseCommand; };
		NotePad::DelegateCommand OnSaveCommand(){ return m_OnSaveCommand; };
		NotePad::DelegateCommand OnSaveAsCommand(){ return m_OnSaveAsCommand; };
		NotePad::DelegateCommand OnEncodingCommand(){ return m_OnEncodingCommand; };
		NotePad::DelegateCommand OnExportCommand(){ return m_OnExportCommand; };
		NotePad::DelegateCommand OnExitCommand(){ return m_OnExitCommand; };
		//Edit
		NotePad::DelegateCommand OnUndoCommand(){ return m_OnUndoCommand; };
		NotePad::DelegateCommand OnRedoCommand(){ return m_OnRedoCommand; };
		NotePad::DelegateCommand OnCutCommand(){ return m_OnCutCommand; };
		NotePad::DelegateCommand OnPasteCommand(){ return m_OnPasteCommand; };
		NotePad::DelegateCommand OnDuplicateCommand(){ return m_OnDuplicateCommand; };
		NotePad::DelegateCommand OnDeleteCommand(){ return m_OnDeleteCommand; };
		NotePad::DelegateCommand OnSelectAllCommand(){ return m_OnSelectAllCommand; };
		NotePad::DelegateCommand OnCopyAsRTFCommand(){ return m_OnCopyAsRTFCommand; };

		//Help
		NotePad::DelegateCommand OnAboutCommand(){ return m_OnAboutCommand; };


		void FolderTree_DoubleTapped(winrt::Windows::Foundation::IInspectable const& sender, DoubleTappedRoutedEventArgs  const&e);
		void TabViewSelectionChanged(int index) {
			SetDocumentAt(index);
			CheckReload();
		}
	private:
		//File
		void OnNewFileClick(Windows::Foundation::IInspectable const& parameter);
		Windows::Foundation::IAsyncAction OnOpenFileClick(Windows::Foundation::IInspectable const& parameter);
		void OnRevertClick(Windows::Foundation::IInspectable const& parameter);
		void OnCloseClick(Windows::Foundation::IInspectable const& parameter);
		void OnSaveClick(Windows::Foundation::IInspectable const& parameter);
		void OnSaveAsClick(Windows::Foundation::IInspectable const& parameter);
		void OnEncodingClick(Windows::Foundation::IInspectable const& parameter);
		void OnExportClick(Windows::Foundation::IInspectable const& parameter);
		void OnExitClick(Windows::Foundation::IInspectable const& parameter);
		//Edit
		void OnUndoClick(Windows::Foundation::IInspectable const& parameter);
		void OnRedoClick(Windows::Foundation::IInspectable const& parameter);
		void OnCutClick(Windows::Foundation::IInspectable const& parameter);
		void OnPasteClick(Windows::Foundation::IInspectable const& parameter);
		void OnDuplicateClick(Windows::Foundation::IInspectable const& parameter);
		void OnDeleteClick(Windows::Foundation::IInspectable const& parameter);
		void OnSelectAllClick(Windows::Foundation::IInspectable const& parameter);
		void OnCopyAsRTFClick(Windows::Foundation::IInspectable const& parameter);

		//help
		void OnAboutClick(Windows::Foundation::IInspectable const& parameter);
		void AnotherClick();

		Windows::UI::Xaml::Input::DoubleTappedEventHandler  _FolderFileDoubleTapped;
		hstring title;
		hstring _FolderName=L"FOLDER NAME";
		NotePad::ScintillaControl sciCtrl{nullptr};
		double textSize;
		int _tabIndex = 0;
		Windows::Foundation::Collections::IObservableVector<winrt::Windows::Foundation::IInspectable>  _FolderDataSource;

		Windows::Foundation::Collections::IObservableVector<winrt::Windows::Foundation::IInspectable> _OpenFileDataSource;
		Windows::Foundation::Collections::IObservableVector<winrt::Windows::Foundation::IInspectable> _OpenFileTabDataSource;
		//File
		NotePad::DelegateCommand m_OnNewFileCommand{ nullptr };
		NotePad::DelegateCommand m_OnOpenFileCommand{ nullptr };
		NotePad::DelegateCommand m_OnRevertCommand{ nullptr };
		NotePad::DelegateCommand m_OnCloseCommand{ nullptr };
		NotePad::DelegateCommand m_OnSaveCommand{ nullptr };
		NotePad::DelegateCommand m_OnSaveAsCommand{ nullptr };
		NotePad::DelegateCommand m_OnEncodingCommand{ nullptr };
		NotePad::DelegateCommand m_OnExportCommand{ nullptr };
		NotePad::DelegateCommand m_OnExitCommand{ nullptr };

		//Edit
		NotePad::DelegateCommand m_OnUndoCommand{ nullptr };
		NotePad::DelegateCommand m_OnRedoCommand{ nullptr };
		NotePad::DelegateCommand m_OnCutCommand{ nullptr };
		NotePad::DelegateCommand m_OnPasteCommand{ nullptr };
		NotePad::DelegateCommand m_OnDuplicateCommand{ nullptr };
		NotePad::DelegateCommand m_OnDeleteCommand{ nullptr };
		NotePad::DelegateCommand m_OnSelectAllCommand{ nullptr };
		NotePad::DelegateCommand m_OnCopyAsRTFCommand{ nullptr };
		//help
		NotePad::DelegateCommand m_OnAboutCommand{ nullptr };

	protected:
		void FindMessageBox(const std::string &msg, const std::string *findItem = 0) override;
		void AboutDialog() override;
		void FindIncrement() override;
		void Find() override;
		void FindInFiles() override;
		void Replace() override;
		void FindReplace(bool replace) override;
		void DestroyFindReplace() override;

		void SetMenuItem(int menuNumber, int position, int itemID,
			const GUI::gui_char *text, const GUI::gui_char *mnemonic = 0) override;
		void RedrawMenu() override;
		void DestroyMenuItem(int menuNumber, int itemID) override;
		void CheckAMenuItem(int wIDCheckItem, bool val) override;
		void EnableAMenuItem(int wIDCheckItem, bool val) override;
		void CheckMenus() override;

		void QuitProgram() override;
		void SetWindowName()override;


		void Execute() override;
		void StopExecute() override;
		void AddCommand(const std::string &cmd, const std::string &dir, JobSubsystem jobType, const std::string &input = "", int flags = 0) override;

		void AddToPopUp(const char *label, int cmd = 0, bool enabled = true) override;
		std::string EncodeString(const std::string &s) override;

		static INT_PTR CALLBACK GoLineDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		void GoLineDialog() override;

		BOOL AbbrevMessage(HWND hDlg, UINT message, WPARAM wParam);
		static INT_PTR CALLBACK AbbrevDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		bool AbbrevDialog() override;

		BOOL TabSizeMessage(HWND hDlg, UINT message, WPARAM wParam);
		static INT_PTR CALLBACK TabSizeDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		void TabSizeDialog() override;

		bool ParametersOpen() override;
		void ParamGrab() override;
		bool ParametersDialog(bool modal) override;

		bool OpenDialog(const FilePath &directory, const GUI::gui_char *filesFilter) override;
		bool SaveAsDialog() override;
		void SaveACopy() override;
		void SaveAsHTML() override;
		void SaveAsRTF() override;
		void SaveAsPDF() override;
		void SaveAsTEX() override;
		void SaveAsXML() override;
		void LoadSessionDialog() override;
		void SaveSessionDialog() override;
		bool PreOpenCheck(const GUI::gui_char *arg) override;
		bool IsStdinBlocked() override;
		void GetWindowPosition(int *left, int *top, int *width, int *height, int *maximize) override;
		FilePath GetDefaultDirectory() override;
		FilePath GetSciteDefaultHome() override;
		FilePath GetSciteUserHome() override;

		winrt::Windows::Foundation::IAsyncAction  ReadEmbeddedProperties() override;
		void ReadPropertiesInitial() override;
		void ReadProperties() override;

		void TimerStart(int mask) override;
		void TimerEnd(int mask) override;

		void ShowOutputOnMainThread() override;
		void SizeContentWindows() override;
		void SizeSubWindows() override;


		void SetFileProperties(PropSetFile &ps) override;
		void SetStatusBarText(const char *s) override;

		void TabInsert(int index, const GUI::gui_char *title) override;
		void TabSelect(int index) override;
		void RemoveAllTabs() override;

		/// Warn the user, by means defined in its properties.	
		void WarnUser(int warnID) override;

		void Notify(SCNotification *notification) override;
		void ShowToolBar() override;
		void ShowTabBar() override;
		void ShowStatusBar() override;
		void ActivateWindow(const char *timestamp) override;
		void ExecuteHelp(const char *cmd);
		void ExecuteOtherHelp(const char *cmd);
		void CopyAsRTF() override;
		void CopyPath() override;

		protected:
			void SetScaleFactor(int scale);


	};
}

namespace winrt::NotePad::factory_implementation
{
	struct MainViewModel : MainViewModelT<MainViewModel, implementation::MainViewModel>
	{
	};
}
