#include "pch.h"
#include "Expander.h"

namespace winrt::NotePad::implementation
{
	DependencyProperty Expander::_IsExpandedProperty =
		DependencyProperty::Register(L"IsExpanded", winrt::xaml_typename<bool>(), winrt::xaml_typename<NotePad::Expander>(), PropertyMetadata(winrt::box_value(false), &Expander::OnIsExpandedPropertyChanged));
	DependencyProperty Expander::_ExpandDirectionProperty =
		DependencyProperty::Register(L"ExpandDirection", winrt::xaml_typename <NotePad::ExpandDirection>(), winrt::xaml_typename<NotePad::Expander>(), PropertyMetadata(winrt::box_value(ExpandDirection::Down), &Expander::OnExpandDirectionChanged));
	DependencyProperty Expander::_ContentOverlayProperty =
		DependencyProperty::Register(L"ContentOverlay", winrt::xaml_typename < UIElement>(), winrt::xaml_typename<NotePad::Expander>(), PropertyMetadata(winrt::box_value(UIElement(nullptr))));
	DependencyProperty Expander::_HeaderStyleProperty =
		DependencyProperty::Register(L"HeaderStyle", winrt::xaml_typename < winrt::Windows::UI::Xaml::Style>(), winrt::xaml_typename<NotePad::Expander>(), PropertyMetadata(winrt::box_value(winrt::Windows::UI::Xaml::Style(nullptr))));
	Expander::Expander() { DefaultStyleKey(winrt::box_value(L"NotePad.Expander")); }
	void Expander::OnApplyTemplate()
	{
		base_type::OnApplyTemplate();
		auto button = GetTemplateChild(ExpanderToggleButtonPart).try_as<ToggleButton>();

		if (button)
		{
			button.KeyDown(ExpanderToggleButtonPart_KeyDownToken);
			button.KeyDown(Input::KeyEventHandler(this, &Expander::ExpanderToggleButtonPart_KeyDown));
		}

		OnExpandDirectionChanged(false);
		OnDisplayModeOrIsExpandedChanged(false);
	}
	void Expander::OnExpanded(IInspectable args)
	{
		if (_Expanded)
			_Expanded(*this, args);
	}
	void Expander::OnCollapsed(IInspectable args)
	{
		if (_Collapsed)
			_Collapsed(*this, args);
	}


	void Expander::OnIsExpandedPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
	{
		auto expander = d.try_as<NotePad::Expander>();

		bool isExpanded = winrt::unbox_value<bool>(e.NewValue());
		if (expander) {
			NotePad::implementation::Expander* ptr{ winrt::from_abi<NotePad::implementation::Expander>(expander) };
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
	void Expander::OnExpandDirectionChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
	{
		auto expander = d.try_as<NotePad::Expander>();
		if (expander) {
			NotePad::implementation::Expander* ptr{ winrt::from_abi<NotePad::implementation::Expander>(expander) };
			auto previousExpandDirection = winrt::unbox_value<NotePad::ExpandDirection>(e.OldValue());
			auto newExpandDirection = winrt::unbox_value < NotePad::ExpandDirection>(e.NewValue());

			if (previousExpandDirection != newExpandDirection)
			{
				ptr->OnExpandDirectionChanged();
			}
		}

	}

	void Expander::ExpanderToggleButtonPart_KeyDown(IInspectable sender, KeyRoutedEventArgs e)
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
	void Expander::ExpandControl()
	{
		OnDisplayModeOrIsExpandedChanged();
		OnExpanded(nullptr);
	}
	void Expander::CollapseControl()
	{
		OnDisplayModeOrIsExpandedChanged();
		OnCollapsed(nullptr);
	}
	void Expander::OnExpandDirectionChanged(bool useTransitions)
	{
		auto button = GetTemplateChild(ExpanderToggleButtonPart).try_as<ToggleButton>();

		if (button == nullptr)
		{
			return;
		}

		UpdateDisplayModeOrExpanderDirection(useTransitions);

		switch (ExpandDirection())
		{
		case ExpandDirection::Left:
			VisualStateManager::GoToState(button, StateContentLeftDirection, useTransitions);
			break;
		case ExpandDirection::Down:
			VisualStateManager::GoToState(button, StateContentDownDirection, useTransitions);
			break;
		case ExpandDirection::Right:
			VisualStateManager::GoToState(button, StateContentRightDirection, useTransitions);
			break;
		case ExpandDirection::Up:
			VisualStateManager::GoToState(button, StateContentUpDirection, useTransitions);
			break;
		}

		// Re-execute animation on expander toggle button (to set correct arrow rotation)
		VisualStateManager::GoToState(button, L"Normal", true);
		if (button.IsChecked().Value())
		{
			VisualStateManager::GoToState(button, L"Checked", true);
		}
	}
	void Expander::OnDisplayModeOrIsExpandedChanged(bool useTransitions)
	{
		UpdateDisplayModeOrExpanderDirection(useTransitions);
	}
	void Expander::UpdateDisplayModeOrExpanderDirection(bool useTransitions)
	{
		hstring visualState = L"";

		switch (ExpandDirection())
		{
		case ExpandDirection::Left:
			visualState = GetDisplayModeVisualState(StateContentCollapsedLeft, StateContentVisibleLeft);
			break;
		case ExpandDirection::Down:
			visualState = GetDisplayModeVisualState(StateContentCollapsedDown, StateContentVisibleDown);
			break;
		case ExpandDirection::Right:
			visualState = GetDisplayModeVisualState(StateContentCollapsedRight, StateContentVisibleRight);
			break;
		case ExpandDirection::Up:
			visualState = GetDisplayModeVisualState(StateContentCollapsedUp, StateContentVisibleUp);
			break;
		}

		if (!visualState.empty())
		{
			VisualStateManager::GoToState(*this, visualState, useTransitions);
		}
	}
	void Expander::OnHeaderChanged(IInspectable oldValue, IInspectable newValue)
	{
	}
	hstring Expander::GetDisplayModeVisualState(hstring collapsedState, hstring visibleState)
	{
		return IsExpanded() ? visibleState : collapsedState;
	}
}
