#pragma once
#include<map>
#include <limits>
#include<vector>
#include "LayoutTransformControl.g.h"
#include "MatrixHelperEx.h"
using namespace winrt::Windows::UI::Xaml;

namespace winrt::NotePad::implementation
{
    struct LayoutTransformControl : LayoutTransformControlT<LayoutTransformControl>
    {
		LayoutTransformControl();;
		void OnApplyTemplate();
		void TransformUpdated()
		{
			ProcessTransform();
		}
		Size MeasureOverride(Size availableSize);

		/// <summary>
		/// Provides the behavior for the "Arrange" pass of layout.
		/// </summary>
		/// <param name="finalSize">The final area within the parent that this
		/// element should use to arrange itself and its children.</param>
		/// <returns>The actual size used.</returns>
		Size ArrangeOverride(Size finalSize);
		static Rect RectTransform(Rect rectangle, Matrix matrix);
        FrameworkElement Child() { return winrt::unbox_value<FrameworkElement>(GetValue(_ChildProperty)); };
        void Child(FrameworkElement const& value) { SetValue(_ChildProperty, winrt::box_value(value)); };
        Media::Transform Transform() { return winrt::unbox_value<Media::Transform>(GetValue(_TransformProperty)); };
        void Transform(Media::Transform const& value) { SetValue(_TransformProperty, winrt::box_value(value)); };

		static DependencyProperty ChildProperty() {return _ChildProperty;};
        static DependencyProperty TransformProperty() { return _TransformProperty; };
	private:
		

		Size ComputeLargestTransformedSize(Size arrangeBounds);
		static bool IsSizeSmaller(Size a, Size b);
		static void ChildChanged(DependencyObject o, DependencyPropertyChangedEventArgs e);
		void ProcessTransform();
		Matrix GetTransformMatrix(Media::Transform transform);
		void OnChildChanged(FrameworkElement newContent);

		static void TransformChanged(DependencyObject o, DependencyPropertyChangedEventArgs e);
		void OnTransformChanged(Media::Transform oldValue, Media::Transform newValue);

		void UnsubscribeFromTransformPropertyChanges(Media::Transform transform);

		void SubscribeToTransformPropertyChanges(Media::Transform transform);

		/// <summary>
		/// Called when a property of a Transform changes.
		/// </summary>
		void OnTransformPropertyChanged(IInspectable sender, double e);

	private:
		static DependencyProperty _ChildProperty;
		static DependencyProperty _TransformProperty;

		/// <summary>
		/// Value used to work around double arithmetic rounding issues.
		/// </summary>
		static  double AcceptableDelta;

		/// <summary>
		/// Value used to work around double arithmetic rounding issues.
		/// </summary>
		 const int DecimalsAfterRound = 4;

		/// <summary>
		/// List of property change event sources for events when properties of the Transform tree change
		/// </summary>
		 std::map<Media::Transform, std::vector<PropertyChangeEventSource>>
			 _transformPropertyChangeEventSources;

		 std::map<Media::Transform, std::vector<winrt::event_token>>
			 _transformPropertyChangeEventSourcesToken;
		/// <summary>
		/// Host panel for Child element.
		/// </summary>
		 Panel _layoutRoot{nullptr};

		/// <summary>
		/// RenderTransform/MatrixTransform applied to layout root.
		/// </summary>
		 MatrixTransform _matrixTransform{ nullptr };

		/// <summary>
		/// Transformation matrix corresponding to matrix transform.
		/// </summary>
		 Matrix _transformation;

		/// <summary>
		/// Actual DesiredSize of Child element.
		/// </summary>
		 Size _childActualSize;

    };
}

namespace winrt::NotePad::factory_implementation
{
    struct LayoutTransformControl : LayoutTransformControlT<LayoutTransformControl, implementation::LayoutTransformControl>
    {
    };
}
