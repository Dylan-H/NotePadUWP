#include "pch.h"
#include "HeaderedContentControl.h"

namespace winrt::NotePad::implementation
{
	DependencyProperty HeaderedContentControl::_HeaderProperty =
		DependencyProperty::Register(L"Header", winrt::xaml_typename<IInspectable>(), winrt::xaml_typename<NotePad::HeaderedContentControl>(), PropertyMetadata(nullptr, PropertyChangedCallback(&HeaderedContentControl::OnHeaderChanged)));
	DependencyProperty HeaderedContentControl::_HeaderTemplateProperty =
		DependencyProperty::Register(L"HeaderTemplate", winrt::xaml_typename<DataTemplate>(), winrt::xaml_typename<NotePad::HeaderedContentControl>(), PropertyMetadata(nullptr));
	DependencyProperty HeaderedContentControl::_OrientationProperty =
		DependencyProperty::Register(L"Orientation", winrt::xaml_typename<Controls::Orientation>(), winrt::xaml_typename<NotePad::HeaderedContentControl>(), PropertyMetadata(winrt::box_value(Orientation::Vertical), &HeaderedContentControl::OnOrientationChanged));
	HeaderedContentControl::HeaderedContentControl() { 
		DefaultStyleKey(winrt::box_value(L"NotePad.HeaderedContentControl"));
	}
	void HeaderedContentControl::OnApplyTemplate()
	{
		base_type::OnApplyTemplate();

		SetHeaderVisibility();
	}
	void HeaderedContentControl::OnHeaderChanged(IInspectable const& oldValue, IInspectable const& newValue)
	{
	}
	void HeaderedContentControl::SetHeaderVisibility()
	{
		auto p = GetTemplateChild(PartHeaderPresenter);
		auto headerPresenter = GetTemplateChild(PartHeaderPresenter).try_as<FrameworkElement>();
		if (headerPresenter)
		{
			headerPresenter.Visibility(Header() != nullptr
				? Visibility::Visible
				: Visibility::Collapsed);
		}
	}
	void HeaderedContentControl::OnOrientationChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
	{

		auto control = d.try_as<NotePad::HeaderedContentControl>();

		auto orientation = control.Orientation() == Orientation::Vertical
			? L"Orientation.Vertical"
			: L"Orientation.Horizontal";

		VisualStateManager::GoToState(control, orientation, true);
	}
	void HeaderedContentControl::OnHeaderChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
	{
		auto control = d.try_as<NotePad::HeaderedContentControl>();
		if (control) {
			NotePad::implementation::HeaderedContentControl* ptr{ winrt::from_abi<NotePad::implementation::HeaderedContentControl>(control) };
			ptr->SetHeaderVisibility();
			ptr->OnHeaderChanged(e.OldValue(), e.NewValue());
		}
	}
}
