#include "pch.h"
#include "PropertyChangeEventSource.h"

namespace winrt::NotePad::implementation
{
	DependencyProperty PropertyChangeEventSource::_ValueProperty =
		DependencyProperty::Register(L"Value",winrt::xaml_typename<double>(),winrt::xaml_typename<NotePad::PropertyChangeEventSource>(), PropertyMetadata(winrt::box_value(0.0), OnValueChanged));
}
