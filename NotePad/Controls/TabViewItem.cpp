#include "pch.h"
#include "TabViewItem.h"

namespace winrt::NotePad::implementation
{
	DependencyProperty TabViewItem::_HeaderProperty = DependencyProperty::Register(L"Header", winrt::xaml_typename<winrt::Windows::Foundation::IInspectable>(), winrt::xaml_typename<NotePad::TabViewItem>(), PropertyMetadata(nullptr));
	DependencyProperty TabViewItem::_IconProperty = DependencyProperty::Register(L"Icon", winrt::xaml_typename<IconElement>(), winrt::xaml_typename<NotePad::TabViewItem>(), PropertyMetadata(nullptr));
	DependencyProperty TabViewItem::_HeaderTemplateProperty=DependencyProperty::Register(L"HeaderTemplate", winrt::xaml_typename<DataTemplate>(), winrt::xaml_typename<NotePad::TabViewItem>(),  PropertyMetadata(nullptr));
	DependencyProperty TabViewItem::_IsClosableProperty=DependencyProperty::Register(L"IsClosable", winrt::xaml_typename<bool>(), winrt::xaml_typename<NotePad::TabViewItem>(),  PropertyMetadata(nullptr));


	void TabViewItem::OnApplyTemplate()
	{
		base_type::OnApplyTemplate();

		if (_tabCloseButton)
		{
			_tabCloseButton.Click(TabCloseButton_ClickToken);
		}

		_tabCloseButton = GetTemplateChild(TabCloseButtonName).try_as<ButtonBase>();

		if (_tabCloseButton)
		{
			TabCloseButton_ClickToken = _tabCloseButton.Click(RoutedEventHandler(this, &TabViewItem::TabCloseButton_Click) );
		}
	}
	void TabViewItem::OnPointerPressed(PointerRoutedEventArgs e)
	{
		base_type::OnPointerPressed(e);

		_isMiddleClick = false;

		if (e.Pointer().PointerDeviceType() == PointerDeviceType::Mouse)
		{
			PointerPoint pointerPoint = e.GetCurrentPoint(*this);

			// Record if middle button is pressed
			if (pointerPoint.Properties().IsMiddleButtonPressed())
			{
				_isMiddleClick = true;
			}
		}
	}
	void TabViewItem::OnPointerReleased(PointerRoutedEventArgs e)
	{
		base_type::OnPointerReleased(e);

		// Close on Middle-Click
		if (_isMiddleClick)
		{
			TabCloseButton_Click(*this, nullptr);
		}

		_isMiddleClick = false;
	}
	void TabViewItem::TabCloseButton_Click(Windows::Foundation::IInspectable sender, RoutedEventArgs e)
	{
		if (IsClosable())
		{
			if(_Closing)
				_Closing(*this, winrt::box_value(TabClosingEventArgs(Content(), *this)));
		}
	}
}
