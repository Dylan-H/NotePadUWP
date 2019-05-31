#include "pch.h"
#include "CustomTitleBar.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::NotePad::implementation
{
    CustomTitleBar::CustomTitleBar()
    {
        InitializeComponent();
    }

	 double CustomTitleBar::CoreTitleBarHeight() { 
		 return coreTitleBar.Height(); 
	 }

	 Thickness CustomTitleBar::CoreTitleBarPadding() {
		if (FlowDirection() == FlowDirection::LeftToRight)
		{
			Thickness thickness;
			thickness.Top = 0;
			thickness.Bottom = 0;
			thickness.Left = coreTitleBar.SystemOverlayLeftInset();
			thickness.Right = coreTitleBar.SystemOverlayRightInset();
			return  thickness;
		}
		else
		{
			Thickness thickness;
			thickness.Top = 0;
			thickness.Bottom = 0;
			thickness.Left = coreTitleBar.SystemOverlayRightInset();
			thickness.Right = coreTitleBar.SystemOverlayLeftInset();
			return  thickness;
		}
	}

	winrt::event_token CustomTitleBar::PropertyChanged(PropertyChangedEventHandler const & handler)
	{
		return _PropertyChanged.add(handler);
	}

	void CustomTitleBar::PropertyChanged(winrt::event_token const & token) noexcept
	{
		_PropertyChanged.remove(token);
	}

	 void CustomTitleBar::SetPageContent(UIElement newContent)
	{
		if (newContent != nullptr)
		{
			RootGrid().Children().Append(newContent);
			Grid::SetRow(newContent.try_as<FrameworkElement>(), 1);
			Window::Current().SetTitleBar(BackgroundElement());
		}
	}

	  void CustomTitleBar::removePageContent(UIElement con)
	 {
		 if (con != nullptr)
		 {
			 //RootGrid().Children().RemoveAt(con);
			 //Window.Current.SetTitleBar(null);
		 }
	 }

	 void CustomTitleBar::UserControl_Loaded(IInspectable const & sender, RoutedEventArgs e)
	{
		LayoutMetricsChangedToken =coreTitleBar.LayoutMetricsChanged(TypedEventHandler<CoreApplicationViewTitleBar, IInspectable>(this,&CustomTitleBar::OnLayoutMetricsChanged));
		IsVisibleChangedToken =coreTitleBar.IsVisibleChanged(TypedEventHandler<CoreApplicationViewTitleBar, IInspectable>(this, &CustomTitleBar::OnIsVisibleChanged));

		// 窗口变化时调整内容
		OnWindowSizeChangedToken =Window::Current().SizeChanged(WindowSizeChangedEventHandler(this,&CustomTitleBar::OnWindowSizeChanged));

		UpdateLayoutMetrics();
		UpdatePositionAndVisibility();
	}

	void CustomTitleBar::UserControl_Unloaded(IInspectable const & sender, RoutedEventArgs e)
	{
		coreTitleBar.LayoutMetricsChanged(LayoutMetricsChangedToken);
		coreTitleBar.IsVisibleChanged(IsVisibleChangedToken);
		Window::Current().SizeChanged(OnWindowSizeChangedToken);
	}

	 void CustomTitleBar::OnLayoutMetricsChanged(CoreApplicationViewTitleBar sender, IInspectable const & e)
	{
		UpdateLayoutMetrics();
	}

	  void CustomTitleBar::UpdateLayoutMetrics()
	 {
		 if (_PropertyChanged)
		 {
			 _PropertyChanged(*this, PropertyChangedEventArgs(L"CoreTitleBarHeight"));
			 _PropertyChanged(*this, PropertyChangedEventArgs(L"CoreTitleBarPadding"));
		 }
	 }

	   void CustomTitleBar::OnIsVisibleChanged(CoreApplicationViewTitleBar sender, IInspectable const & e)
	  {
		  UpdatePositionAndVisibility();
	  }

	    void CustomTitleBar::OnWindowSizeChanged(IInspectable const & sender, WindowSizeChangedEventArgs e)
	   {
		   UpdatePositionAndVisibility();
	   }

		 void CustomTitleBar::UpdatePositionAndVisibility()
		{
			if (ApplicationView::GetForCurrentView().IsFullScreenMode())
			{
				//如果全屏模式 标题栏应该覆盖在内容之上
				TitleBar().Visibility(coreTitleBar.IsVisible() ? Visibility::Visible : Visibility::Collapsed);
				Grid::SetRow(TitleBar(), 1);
			}
			else
			{
				// 不是全屏模式，标题栏在内容之上
				TitleBar().Visibility(Visibility::Visible);
				Grid::SetRow(TitleBar(), 0);
			}
		}

}
