#pragma once

#include "Expander.g.h"
#include "HeaderedContentControl.h"
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Xaml;
namespace winrt::NotePad::implementation
{
    struct Expander : ExpanderT<Expander, NotePad::implementation::HeaderedContentControl>
    {
		Expander();
        bool IsExpanded() { return winrt::unbox_value<bool>(GetValue(_IsExpandedProperty)); };
        void IsExpanded(bool value) { SetValue(_IsExpandedProperty, winrt::box_value(value)); };
        NotePad::ExpandDirection ExpandDirection() { return winrt::unbox_value< NotePad::ExpandDirection>(GetValue(_ExpandDirectionProperty)); };
        void ExpandDirection(NotePad::ExpandDirection const& value) { SetValue(_ExpandDirectionProperty, winrt::box_value(value)); };
        UIElement ContentOverlay() { return winrt::unbox_value<UIElement>(GetValue(_ContentOverlayProperty)); };
        void ContentOverlay(UIElement const& value) { SetValue(_ContentOverlayProperty, winrt::box_value(value)); };
        Windows::UI::Xaml::Style HeaderStyle() { return winrt::unbox_value<Windows::UI::Xaml::Style>(GetValue(_HeaderStyleProperty)); };
        void HeaderStyle(Windows::UI::Xaml::Style const& value) { SetValue(_HeaderStyleProperty, winrt::box_value(value)); };

		winrt::event_token Expanded(Windows::Foundation::EventHandler<Windows::Foundation::IInspectable> const& handler) { return _Expanded.add(handler); };
		void Expanded(winrt::event_token const& token) { _Expanded.remove(token); } ;
		winrt::event_token Collapsed(Windows::Foundation::EventHandler<Windows::Foundation::IInspectable> const& handler) { return _Collapsed.add(handler); };
		void Collapsed(winrt::event_token const& token) { _Collapsed.remove(token); };


        static DependencyProperty IsExpandedProperty() { return _IsExpandedProperty; };
        static DependencyProperty ExpandDirectionProperty() { return _ExpandDirectionProperty; };
        static DependencyProperty ContentOverlayProperty() { return _ContentOverlayProperty; };
        static DependencyProperty HeaderStyleProperty() { return _HeaderStyleProperty; };

		void OnApplyTemplate();
		virtual void OnExpanded(IInspectable args);
		virtual void OnCollapsed(IInspectable args);

	private:
		static void OnIsExpandedPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e);

		static void OnExpandDirectionChanged(DependencyObject d, DependencyPropertyChangedEventArgs e);
		void ExpanderToggleButtonPart_KeyDown(IInspectable sender, KeyRoutedEventArgs e);
		void ExpandControl();

		void CollapseControl();
		void OnExpandDirectionChanged(bool useTransitions = true);

		void OnDisplayModeOrIsExpandedChanged(bool useTransitions = true);

		void UpdateDisplayModeOrExpanderDirection(bool useTransitions = true);
		void OnHeaderChanged(IInspectable oldValue, IInspectable newValue);
		hstring GetDisplayModeVisualState(hstring collapsedState, hstring visibleState);
	private:
		static DependencyProperty _IsExpandedProperty;
		static DependencyProperty _ExpandDirectionProperty;
		static DependencyProperty _ContentOverlayProperty;
		static DependencyProperty _HeaderStyleProperty;

		winrt::event_token ExpanderToggleButtonPart_KeyDownToken;
		winrt::event<Windows::Foundation::EventHandler<Windows::Foundation::IInspectable>> _Expanded;
		winrt::event<Windows::Foundation::EventHandler<Windows::Foundation::IInspectable>> _Collapsed;
	private:
		 const hstring DisplayModeAndDirectionStatesGroupStateContent = L"DisplayModeAndDirectionStates";

		/// <summary>
		/// Key of the VisualState when expander is visible and expander direction is set to Left
		/// </summary>
		 const hstring StateContentVisibleLeft = L"VisibleLeft";

		/// <summary>
		/// Key of the VisualState when expander is visible and expander direction is set to Down
		/// </summary>
		 const hstring StateContentVisibleDown = L"VisibleDown";

		/// <summary>
		/// Key of the VisualState when expander is visible and expander direction is set to Right
		/// </summary>
		 const hstring StateContentVisibleRight = L"VisibleRight";

		/// <summary>
		/// Key of the VisualState when expander is visible and expander direction is set to Up
		/// </summary>
		 const hstring StateContentVisibleUp = L"VisibleUp";

		/// <summary>
		/// Key of the VisualState when expander is collapsed and expander direction is set to Left
		/// </summary>
		 const hstring StateContentCollapsedLeft = L"CollapsedLeft";

		/// <summary>
		/// Key of the VisualState when expander is collapsed and expander direction is set to Down
		/// </summary>
		 const hstring StateContentCollapsedDown = L"CollapsedDown";

		/// <summary>
		/// Key of the VisualState when expander is collapsed and expander direction is set to Right
		/// </summary>
		 const hstring StateContentCollapsedRight = L"CollapsedRight";

		/// <summary>
		/// Key of the VisualState when expander is collapsed and expander direction is set to Up
		/// </summary>
		 const hstring StateContentCollapsedUp = L"CollapsedUp";

		/// <summary>
		/// Key of the UI Element that toggle IsExpanded property
		/// </summary>
		 const hstring ExpanderToggleButtonPart = L"PART_ExpanderToggleButton";

		/// <summary>
		/// Key of the UI Element that contains the content of the control that is expanded/collapsed
		/// </summary>
		 const hstring MainContentPart = L"PART_MainContent";

		/// <summary>
		/// Key of the VisualStateGroup that set expander direction of the control
		/// </summary>
		 const hstring ExpandDirectionGroupStateContent = L"ExpandDirectionStates";

		/// <summary>
		/// Key of the VisualState when expander direction is set to Left
		/// </summary>
		 const hstring StateContentLeftDirection = L"LeftDirection";

		/// <summary>
		/// Key of the VisualState when expander direction is set to Down
		/// </summary>
		 const hstring StateContentDownDirection = L"DownDirection";

		/// <summary>
		/// Key of the VisualState when expander direction is set to Right
		/// </summary>
		 const hstring StateContentRightDirection = L"RightDirection";

		/// <summary>
		/// Key of the VisualState when expander direction is set to Up
		/// </summary>
		 const hstring StateContentUpDirection = L"UpDirection";

		/// <summary>
		/// Key of the UI Element that contains the content of the entire control
		/// </summary>
		 const hstring RootGridPart = L"PART_RootGrid";

		/// <summary>
		/// Key of the UI Element that contains the content of the LayoutTransformer (of the expander button)
		/// </summary>
		 const hstring LayoutTransformerPart = L"PART_LayoutTransformer";

    };
}

namespace winrt::NotePad::factory_implementation
{
    struct Expander : ExpanderT<Expander, implementation::Expander>
    {
    };
}
