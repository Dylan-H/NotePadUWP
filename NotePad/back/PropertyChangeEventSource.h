#pragma once

#include "PropertyChangeEventSource.g.h"
using namespace winrt::Windows::UI::Xaml;
namespace winrt::NotePad::implementation
{
    struct PropertyChangeEventSource : PropertyChangeEventSourceT<PropertyChangeEventSource>
    {
        PropertyChangeEventSource() = default;
		PropertyChangeEventSource(
			DependencyObject source,
			hstring propertyName,
			BindingMode bindingMode = BindingMode::TwoWay)
		{
			_source = source;

			// Bind to the property to be able to get its changes relayed as events throug the ValueChanged event.
			Binding binding;

			binding.Source( source);
			binding.Path ( PropertyPath(propertyName));
			binding.Mode( bindingMode);
			SetBinding(_ValueProperty, binding);
		}
		double Value() { return winrt::unbox_value<double>(GetValue(_ValueProperty)); };
        void Value(double value) { SetValue(_ValueProperty, winrt::box_value(value)); };
		winrt::event_token ValueChanged(Windows::Foundation::EventHandler<double> const& handler) { return _ValueChanged.add(handler); };
		void ValueChanged(winrt::event_token const& token) noexcept { _ValueChanged.remove(token); };
		static DependencyProperty ValueProperty() { return _ValueProperty; };

		static void OnValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
		{
			auto target =  d.try_as< NotePad::PropertyChangeEventSource>();
			if (target) {
				double oldValue = winrt::unbox_value<double>(e.OldValue());
				double  newValue = target.Value();

				NotePad::implementation::PropertyChangeEventSource* ptr{ winrt::from_abi<NotePad::implementation::PropertyChangeEventSource>(target) };
				ptr->OnValueChanged(oldValue, newValue);
			}
		}
	private:
		 void OnValueChanged(double oldValue, double newValue)
		{
			 if(_ValueChanged)
				 _ValueChanged(_source, newValue);
		}

	private:
		static DependencyProperty _ValueProperty;
		winrt::event<Windows::Foundation::EventHandler<double>> _ValueChanged;
		DependencyObject _source{nullptr};
    };
}

namespace winrt::NotePad::factory_implementation
{
    struct PropertyChangeEventSource : PropertyChangeEventSourceT<PropertyChangeEventSource, implementation::PropertyChangeEventSource>
    {
    };
}
