#include "pch.h"
#include "ExpanderEx.h"

namespace winrt::NotePad::implementation
{
	DependencyProperty ExpanderEx::_HeaderProperty =
		DependencyProperty::Register(L"Header", winrt::xaml_typename<IInspectable>(), winrt::xaml_typename<NotePad::ExpanderEx>(), PropertyMetadata(nullptr, PropertyChangedCallback(&ExpanderEx::OnHeaderChanged)));
	DependencyProperty ExpanderEx::_IsExpandedProperty =
		DependencyProperty::Register(L"IsExpanded", winrt::xaml_typename<bool>(), winrt::xaml_typename<NotePad::ExpanderEx>(), PropertyMetadata(winrt::box_value(false), &ExpanderEx::OnIsExpandedPropertyChanged));
	DependencyProperty ExpanderEx::_ContentEndProperty =
		DependencyProperty::Register(L"ContentEnd", winrt::xaml_typename < UIElement>(), winrt::xaml_typename<NotePad::ExpanderEx>(), PropertyMetadata(winrt::box_value(UIElement(nullptr))));
	DependencyProperty ExpanderEx::_HeaderStyleProperty =
		DependencyProperty::Register(L"HeaderStyle", winrt::xaml_typename < winrt::Windows::UI::Xaml::Style>(), winrt::xaml_typename<NotePad::ExpanderEx>(), PropertyMetadata(winrt::box_value(winrt::Windows::UI::Xaml::Style(nullptr))));
	 ExpanderEx::ExpanderEx() { { DefaultStyleKey(winrt::box_value(L"NotePad.ExpanderEx")); } }
	void ExpanderEx::OnApplyTemplate()
	{
		base_type::OnApplyTemplate();
		LostFocus(RoutedEventHandler(this, &ExpanderEx::ExpanderContentGridPart_LostFocus));
		auto button = GetTemplateChild(PART_ContentGrid).try_as<Grid>();

		if (button)
		{
			button.KeyDown(ExpanderToggleButtonPart_KeyDownToken);
			button.KeyDown(Input::KeyEventHandler(this, &ExpanderEx::ExpanderToggleButtonPart_KeyDown));
			button.PointerPressed(Input::PointerEventHandler(this, &ExpanderEx::ExpanderContentGridPart_PointerPressed));
			button.PointerReleased(Input::PointerEventHandler(this, &ExpanderEx::ExpanderContentGridPart_PointerReleased));
		}
	}
	void ExpanderEx::ExpandControl()
	{
		VisualStateManager::GoToState(*this, StateContentVisibleDown, true);
		
		if (_Expanded)
			_Expanded(*this, nullptr);
	}
	void ExpanderEx::CollapseControl()
	{

		VisualStateManager::GoToState(*this, StateContentCollapsedDown, true);
		
		if (_Collapsed)
			_Collapsed(*this, nullptr);
	}
	void ExpanderEx::OnHeaderChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
	{

	}
	void ExpanderEx::OnIsExpandedPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
	{
		auto expander = d.try_as<NotePad::ExpanderEx>();

		bool isExpanded = winrt::unbox_value<bool>(e.NewValue());
		if (expander) {
			NotePad::implementation::ExpanderEx* ptr{ winrt::from_abi<NotePad::implementation::ExpanderEx>(expander) };
			if (isExpanded)
			{
				ptr->ExpandControl();
			}
			else
			{
				ptr->CollapseControl();
			}
		}
	}
	void ExpanderEx::ExpanderContentGridPart_PointerPressed(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const & e)
	{
		Focus(FocusState::Pointer);
		OutputDebugStringW(L"PointerPressed\r\n");
	}
	void ExpanderEx::ExpanderContentGridPart_PointerReleased(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const & e)
	{
		OutputDebugStringW(L"PointerReleased\r\n");
		auto border = GetTemplateChild(L"PART_ContentBorder").try_as<Border>();
		border.BorderThickness(Thickness{ 1,1,1,1 });
		auto ppt = e.GetCurrentPoint(sender.try_as<UIElement>());
		auto p = ppt.Properties();
		if(ppt.Properties().PointerUpdateKind()== winrt::Windows::UI::Input::PointerUpdateKind::LeftButtonReleased)
			IsExpanded(!IsExpanded());
	}
	void ExpanderEx::ExpanderContentGridPart_LostFocus(Windows::Foundation::IInspectable const& sender, RoutedEventArgs const& handler)
	{
		OutputDebugStringW(L"LostFocus\r\n");
		auto border = GetTemplateChild(L"PART_ContentBorder").try_as<Border>();
		border.BorderThickness(Thickness{0});
	}
	void ExpanderEx::ExpanderToggleButtonPart_KeyDown(IInspectable const & sender, KeyRoutedEventArgs const & e)
	{
		if (e.Key() != VirtualKey::Enter)
		{
			return;
		}

		auto button = sender.try_as<ToggleButton>();

		if (button == nullptr)
		{
			return;
		}

		IsExpanded(!IsExpanded());

		e.Handled(true);
	}
}
