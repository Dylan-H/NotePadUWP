#include "pch.h"
#include "WrapPanel.h"

namespace winrt::NotePad::implementation
{
	struct UvMeasure
	{
		float U;
		float V;
		UvMeasure()
		{
			U = V = 0;
		}
		UvMeasure(Controls::Orientation orientation, float width, float height)
		{
			if (orientation == Controls::Orientation::Horizontal)
			{
				U = width;
				V = height;
			}
			else
			{
				U = height;
				V = width;
			}
		}
	};


	DependencyProperty WrapPanel::_HorizontalSpacingProperty = DependencyProperty::Register(L"HorizontalSpacing", { winrt::hstring(winrt::name_of<double>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename <NotePad::WrapPanel>(), PropertyMetadata(winrt::box_value(0.0), &WrapPanel::LayoutPropertyChanged));
	DependencyProperty WrapPanel::_VerticalSpacingProperty = DependencyProperty::Register(L"VerticalSpacing", { winrt::hstring(winrt::name_of<double>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename <NotePad::WrapPanel>(), PropertyMetadata(winrt::box_value(0.0), &WrapPanel::LayoutPropertyChanged));
	DependencyProperty WrapPanel::_OrientationProperty = DependencyProperty::Register(L"Orientation", { winrt::hstring(winrt::name_of<Controls::Orientation>()) ,Interop::TypeKind::Primitive }, winrt::xaml_typename<NotePad::WrapPanel>(), PropertyMetadata(winrt::box_value(Controls::Orientation::Horizontal), PropertyChangedCallback(&WrapPanel::LayoutPropertyChanged)));
	DependencyProperty WrapPanel::_PaddingProperty = DependencyProperty::Register(L"Padding", winrt::xaml_typename <Thickness>(), winrt::xaml_typename <NotePad::WrapPanel>(), PropertyMetadata(winrt::box_value(Thickness()), &WrapPanel::LayoutPropertyChanged));


	void WrapPanel::LayoutPropertyChanged(DependencyObject const & d, DependencyPropertyChangedEventArgs const & e)
	{
		NotePad::WrapPanel p = { d.try_as<NotePad::WrapPanel>() };
		if (p)
		{
			p.InvalidateMeasure();
			p.InvalidateArrange();
		}
	}
	Windows::Foundation::Size WrapPanel::MeasureOverride(Windows::Foundation::Size availableSize)
	{
		availableSize.Width = (float)(availableSize.Width - Padding().Left - Padding().Right);
		availableSize.Height = (float)(availableSize.Height - Padding().Top - Padding().Bottom);
		UvMeasure totalMeasure;
		UvMeasure parentMeasure(Orientation(), (float)availableSize.Width, (float)availableSize.Height);
		UvMeasure spacingMeasure(Orientation(), (float)HorizontalSpacing(), (float)VerticalSpacing());
		UvMeasure lineMeasure;

		for (unsigned int i = 0; i < Children().Size(); i++)
		{
			auto child = Children().GetAt(i);
			child.Measure(availableSize);

			UvMeasure currentMeasure(Orientation(), child.DesiredSize().Width, child.DesiredSize().Height);

			if (parentMeasure.U >= currentMeasure.U + lineMeasure.U + spacingMeasure.U)
			{
				lineMeasure.U += currentMeasure.U + spacingMeasure.U;
				lineMeasure.V = max(lineMeasure.V, currentMeasure.V);
			}
			else
			{
				// new line should be added
				// to get the max U to provide it correctly to ui width ex: ---| or -----|
				totalMeasure.U = max(lineMeasure.U, totalMeasure.U);
				totalMeasure.V += lineMeasure.V + spacingMeasure.V;

				// if the next new row still can handle more controls
				if (parentMeasure.U > currentMeasure.U)
				{
					// set lineMeasure initial values to the currentMeasure to be calculated later on the new loop
					lineMeasure = currentMeasure;
				}

				// the control will take one row alone
				else
				{
					// validate the new control measures
					totalMeasure.U = max(currentMeasure.U, totalMeasure.U);
					totalMeasure.V += currentMeasure.V;
				}
			}
		}

		// update value with the last line
		// if the the last loop is(parentMeasure.U > currentMeasure.U + lineMeasure.U) the total isn't calculated then calculate it
		// if the last loop is (parentMeasure.U > currentMeasure.U) the currentMeasure isn't added to the total so add it here
		// for the last condition it is zeros so adding it will make no difference
		// this way is faster than an if condition in every loop for checking the last item
		totalMeasure.U = max(lineMeasure.U, totalMeasure.U);
		totalMeasure.V += lineMeasure.V;

		totalMeasure.U = ceil(totalMeasure.U);

		return Orientation() == Controls::Orientation::Horizontal ? Windows::Foundation::Size(totalMeasure.U, totalMeasure.V) : Windows::Foundation::Size(totalMeasure.V, totalMeasure.U);
	}
	Windows::Foundation::Size WrapPanel::ArrangeOverride(Windows::Foundation::Size finalSize)
	{
		UvMeasure parentMeasure(Orientation(), finalSize.Width, finalSize.Height);
		UvMeasure spacingMeasure(Orientation(), HorizontalSpacing(), VerticalSpacing());
		UvMeasure paddingStart(Orientation(), Padding().Left, Padding().Top);
		UvMeasure paddingEnd(Orientation(), Padding().Right, Padding().Bottom);
		UvMeasure position(Orientation(), Padding().Left, Padding().Top);

		double currentV = 0;
		for (unsigned int i = 0; i < Children().Size(); i++)
		{
			auto child = Children().GetAt(i);
			UvMeasure desiredMeasure(Orientation(), child.DesiredSize().Width, child.DesiredSize().Height);
			if ((desiredMeasure.U + position.U + paddingEnd.U) > parentMeasure.U)
			{
				// next row!
				position.U = paddingStart.U;
				position.V += currentV + spacingMeasure.V;
				currentV = 0;
			}

			// Place the item
			if (Orientation() == Windows::UI::Xaml::Controls::Orientation::Horizontal)
			{
				child.Arrange(Windows::Foundation::Rect(position.U, position.V, child.DesiredSize().Width, child.DesiredSize().Height));
			}
			else
			{
				child.Arrange(Windows::Foundation::Rect(position.V, position.U, child.DesiredSize().Width, child.DesiredSize().Height));
			}

			// adjust the location for the next items
			position.U += desiredMeasure.U + spacingMeasure.U;
			currentV = max(desiredMeasure.V, currentV);
		}

		return finalSize;
	}
}
