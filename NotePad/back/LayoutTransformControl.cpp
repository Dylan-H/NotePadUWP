#include "pch.h"
#include "LayoutTransformControl.h"

namespace winrt::NotePad::implementation
{
	double LayoutTransformControl::AcceptableDelta = 0.0001;
	DependencyProperty LayoutTransformControl::_ChildProperty =
		DependencyProperty::Register(L"Child", winrt::xaml_typename<FrameworkElement>(), winrt::xaml_typename <NotePad::LayoutTransformControl>(), PropertyMetadata(nullptr, &LayoutTransformControl::ChildChanged));
	DependencyProperty LayoutTransformControl::_TransformProperty =
		DependencyProperty::Register(L"Transform", winrt::xaml_typename <Media::Transform>(), winrt::xaml_typename < NotePad::LayoutTransformControl>(), PropertyMetadata(nullptr, &LayoutTransformControl::TransformChanged));
	LayoutTransformControl::LayoutTransformControl() {
		DefaultStyleKey(winrt::box_value(L"NotePad.LayoutTransformControl"));
		// Can't tab to LayoutTransformControl
		IsTabStop(false);

		// Disable layout rounding because its rounding of values confuses things.
		UseLayoutRounding(false);
	}
	void LayoutTransformControl::OnApplyTemplate()
	{
		// Save existing content and remove it from the visual tree
		FrameworkElement savedContent = Child();
		Child(nullptr);

		// Apply new template
		base_type::OnApplyTemplate();

		// Find template parts
		_layoutRoot = GetTemplateChild(L"LayoutRoot").try_as<Panel>();
		_matrixTransform = GetTemplateChild(L"MatrixTransform").try_as <MatrixTransform>();

		// RestoreAsync saved content
		Child(savedContent);

		// Apply the current transform
		TransformUpdated();
	}
	Size LayoutTransformControl::MeasureOverride(Size availableSize)
	{
		FrameworkElement child = Child();
		if (_layoutRoot == nullptr || child == nullptr)
		{
			// No content, no size
			return Size();
		}

		Size measureSize;
		if (_childActualSize == Size())
		{
			// Determine the largest size after the transformation
			measureSize = ComputeLargestTransformedSize(availableSize);
		}
		else
		{
			// Previous measure/arrange pass determined that Child.DesiredSize was larger than believed.
			measureSize = _childActualSize;
		}

		// Perform a mesaure on the _layoutRoot (containing Child)
		_layoutRoot.Measure(measureSize);

		// Transform DesiredSize to find its width/height
		Rect transformedDesiredRect = RectTransform(Rect(0, 0, _layoutRoot.DesiredSize().Width, _layoutRoot.DesiredSize().Height), _transformation);
		Size transformedDesiredSize(transformedDesiredRect.Width, transformedDesiredRect.Height);

		// Return result to allocate enough space for the transformation
		return transformedDesiredSize;
	}

	/// <summary>
	/// Provides the behavior for the "Arrange" pass of layout.
	/// </summary>
	/// <param name="finalSize">The final area within the parent that this
	/// element should use to arrange itself and its children.</param>
	/// <returns>The actual size used.</returns>

	Size LayoutTransformControl::ArrangeOverride(Size finalSize)
	{
		FrameworkElement child = Child();
		if (_layoutRoot == nullptr || child == nullptr)
		{
			// No child, use whatever was given
			return finalSize;
		}

		// Determine the largest available size after the transformation
		Size finalSizeTransformed = ComputeLargestTransformedSize(finalSize);
		if (IsSizeSmaller(finalSizeTransformed, _layoutRoot.DesiredSize()))
		{
			// Some elements do not like being given less space than they asked for (ex: TextBlock)
			// Bump the working size up to do the right thing by them
			finalSizeTransformed = _layoutRoot.DesiredSize();
		}

		// Transform the working size to find its width/height
		Rect transformedRect = RectTransform(Rect(0, 0, finalSizeTransformed.Width, finalSizeTransformed.Height), _transformation);

		// Create the Arrange rect to center the transformed content
		Rect finalRect(
			-RectHelper::GetLeft(transformedRect) + ((finalSize.Width - transformedRect.Width) / 2),
			-RectHelper::GetTop(transformedRect) + ((finalSize.Height - transformedRect.Height) / 2),
			finalSizeTransformed.Width,
			finalSizeTransformed.Height);

		// Perform an Arrange on _layoutRoot (containing Child)
		_layoutRoot.Arrange(finalRect);

		// This is the first opportunity to find out the Child's true DesiredSize
		if (IsSizeSmaller(finalSizeTransformed, child.RenderSize()) && (Size() == _childActualSize))
		{
			// Unfortunately, all the work so far is invalid because the wrong DesiredSize was used
			// Make a note of the actual DesiredSize
			_childActualSize = Size(child.ActualWidth(), child.ActualHeight());

			// Force a new measure/arrange pass
			InvalidateMeasure();
		}
		else
		{
			// Clear the "need to measure/arrange again" flag
			_childActualSize = Size();
		}

		// Return result to perform the transformation
		return finalSize;
	}
	Rect LayoutTransformControl::RectTransform(Rect rectangle, Matrix matrix)
	{
		// WPF equivalent of following code:
		// var rectTransformed = Rect.Transform(rect, matrix);
		Point leftTop = MatrixHelper::Transform(matrix, Point(RectHelper::GetLeft(rectangle), RectHelper::GetTop(rectangle)));
		Point rightTop = MatrixHelper::Transform(matrix, Point(RectHelper::GetRight(rectangle), RectHelper::GetTop(rectangle)));
		Point leftBottom = MatrixHelper::Transform(matrix, Point(RectHelper::GetLeft(rectangle), RectHelper::GetBottom(rectangle)));
		Point rightBottom = MatrixHelper::Transform(matrix, Point(RectHelper::GetRight(rectangle), RectHelper::GetBottom(rectangle)));
		double left = min(min(leftTop.X, rightTop.X), min(leftBottom.X, rightBottom.X));
		double top = min(min(leftTop.Y, rightTop.Y), min(leftBottom.Y, rightBottom.Y));
		double right = max(max(leftTop.X, rightTop.X), max(leftBottom.X, rightBottom.X));
		double bottom = max(max(leftTop.Y, rightTop.Y), max(leftBottom.Y, rightBottom.Y));
		Rect rectTransformed = Rect(left, top, right - left, bottom - top);
		return rectTransformed;
	}
	Size LayoutTransformControl::ComputeLargestTransformedSize(Size arrangeBounds)
	{
		// Computed largest transformed size
		Size computedSize = Size();

		// Detect infinite bounds and constrain the scenario
		bool infiniteWidth = std::isinf<double>(arrangeBounds.Width);

		if (infiniteWidth)
		{
			arrangeBounds.Width = arrangeBounds.Height;
		}

		bool infiniteHeight = std::isinf<double>(arrangeBounds.Height);

		if (infiniteHeight)
		{
			arrangeBounds.Height = arrangeBounds.Width;
		}

		// Capture the matrix parameters
		double a = _transformation.M11;
		double b = _transformation.M12;
		double c = _transformation.M21;
		double d = _transformation.M22;

		// Compute maximum possible transformed width/height based on starting width/height
		// These constraints define two lines in the positive x/y quadrant
		double maxWidthFromWidth = abs(arrangeBounds.Width / a);
		double maxHeightFromWidth = abs(arrangeBounds.Width / c);
		double maxWidthFromHeight = abs(arrangeBounds.Height / b);
		double maxHeightFromHeight = abs(arrangeBounds.Height / d);

		// The transformed width/height that maximize the area under each segment is its midpoint
		// At most one of the two midpoints will satisfy both constraints
		double idealWidthFromWidth = maxWidthFromWidth / 2;
		double idealHeightFromWidth = maxHeightFromWidth / 2;
		double idealWidthFromHeight = maxWidthFromHeight / 2;
		double idealHeightFromHeight = maxHeightFromHeight / 2;

		// Compute slope of both constraint lines
		double slopeFromWidth = -(maxHeightFromWidth / maxWidthFromWidth);
		double slopeFromHeight = -(maxHeightFromHeight / maxWidthFromHeight);
		double doubleInfinity = std::numeric_limits<double>::infinity();

		if (arrangeBounds.Width == 0 || arrangeBounds.Height == 0)
		{
			// Check for empty bounds
			computedSize = Size(0, 0);
		}
		else if (infiniteWidth && infiniteHeight)
		{
			// Check for completely unbound scenario
			computedSize = Size(doubleInfinity, doubleInfinity);
		}
		else if (!HasInverse(_transformation))
		{
			// Check for singular matrix
			computedSize = Size(0, 0);
		}
		else if (b == 0 || c == 0)
		{
			// Check for 0/180 degree special cases
			double maxHeight = infiniteHeight ? doubleInfinity : maxHeightFromHeight;
			double maxWidth = infiniteWidth ? doubleInfinity : maxWidthFromWidth;

			if (b == 0 && c == 0)
			{
				// No constraints
				computedSize = Size(maxWidth, maxHeight);
			}
			else if (b == 0)
			{
				// Constrained by width
				double computedHeight = min(idealHeightFromWidth, maxHeight);
				computedSize = Size(
					maxWidth - abs((c * computedHeight) / a),
					computedHeight);
			}
			else if (c == 0)
			{
				// Constrained by height
				double computedWidth = min(idealWidthFromHeight, maxWidth);
				computedSize = Size(
					computedWidth,
					maxHeight - abs((b * computedWidth) / d));
			}
		}
		else if (a == 0 || d == 0)
		{
			// Check for 90/270 degree special cases
			double maxWidth = infiniteHeight ? doubleInfinity : maxWidthFromHeight;
			double maxHeight = infiniteWidth ? doubleInfinity : maxHeightFromWidth;

			if (a == 0 && d == 0)
			{
				// No constraints
				computedSize = Size(maxWidth, maxHeight);
			}
			else if (a == 0)
			{
				// Constrained by width
				double computedHeight = min(idealHeightFromHeight, maxHeight);
				computedSize = Size(
					maxWidth - abs((d * computedHeight) / b),
					computedHeight);
			}
			else if (d == 0)
			{
				// Constrained by height.
				double computedWidth = min(idealWidthFromWidth, maxWidth);
				computedSize = Size(
					computedWidth,
					maxHeight - abs((a * computedWidth) / c));
			}
		}
		else if (idealHeightFromWidth <= ((slopeFromHeight * idealWidthFromWidth) + maxHeightFromHeight))
		{
			// Check the width midpoint for viability (by being below the height constraint line).
			computedSize = Size(idealWidthFromWidth, idealHeightFromWidth);
		}
		else if (idealHeightFromHeight <= ((slopeFromWidth * idealWidthFromHeight) + maxHeightFromWidth))
		{
			// Check the height midpoint for viability (by being below the width constraint line).
			computedSize = Size(idealWidthFromHeight, idealHeightFromHeight);
		}
		else
		{
			// Neither midpoint is viable; use the intersection of the two constraint lines instead.

			// Compute width by setting heights equal (m1*x+c1=m2*x+c2).
			double computedWidth = (maxHeightFromHeight - maxHeightFromWidth) / (slopeFromWidth - slopeFromHeight);

			// Compute height from width constraint line (y=m*x+c; using height would give same result).
			computedSize = Size(
				computedWidth,
				(slopeFromWidth * computedWidth) + maxHeightFromWidth);
		}

		return computedSize;
	}
	bool LayoutTransformControl::IsSizeSmaller(Size a, Size b)
	{
		// WPF equivalent of following code:
		// return ((a.Width < b.Width) || (a.Height < b.Height));
		return (a.Width + AcceptableDelta < b.Width) || (a.Height + AcceptableDelta < b.Height);
	}
	void LayoutTransformControl::ChildChanged(DependencyObject o, DependencyPropertyChangedEventArgs e)
	{
		auto lt = o.try_as<NotePad::LayoutTransformControl>();

		if (lt) {
			NotePad::implementation::LayoutTransformControl* ptr{ winrt::from_abi<NotePad::implementation::LayoutTransformControl>(lt) };
			ptr->OnChildChanged(winrt::unbox_value<FrameworkElement>(e.NewValue()));
		}

	}
	void LayoutTransformControl::ProcessTransform()
	{
		// Get the transform matrix and apply it
		_transformation = Round(GetTransformMatrix(Transform()));

		if (_matrixTransform != nullptr)
		{
			_matrixTransform.Matrix(_transformation);
		}

		// New transform means re-layout is necessary
		InvalidateMeasure();
	}
	Matrix LayoutTransformControl::GetTransformMatrix(Media::Transform transform)
	{
		if (transform != nullptr)
		{
			// WPF equivalent of this entire method (why oh why only WPF...):
			// return transform.Value;

			// Process the TransformGroup
			auto transformGroup = transform.try_as<TransformGroup>();

			if (transformGroup != nullptr)
			{
				auto groupMatrix = MatrixHelper::Identity();

				for (auto child : transformGroup.Children())
				{
					groupMatrix = Multiply(groupMatrix, GetTransformMatrix(child));
				}

				return groupMatrix;
			}

			// Process the RotateTransform
			auto rotateTransform = transform.try_as< RotateTransform>();

			if (rotateTransform != nullptr)
			{
				return GetMatrix(rotateTransform);
			}

			// Process the ScaleTransform
			auto scaleTransform = transform.try_as< ScaleTransform>();

			if (scaleTransform != nullptr)
			{
				return GetMatrix(scaleTransform);
			}

			// Process the SkewTransform
			auto skewTransform = transform.try_as< SkewTransform>();

			if (skewTransform != nullptr)
			{
				return GetMatrix(skewTransform);
			}

			// Process the MatrixTransform
			auto matrixTransform = transform.try_as<MatrixTransform>();
			if (matrixTransform != nullptr)
			{
				return matrixTransform.Matrix();
			}
			// TranslateTransform has no effect in LayoutTransform
		}

		// Fall back to no-op transformation
		return MatrixHelper::Identity();
	}
	void LayoutTransformControl::OnChildChanged(FrameworkElement newContent)
	{
		if (_layoutRoot)
		{
			// Clear current child
			_layoutRoot.Children().Clear();
			if (newContent)
			{
				// Add the new child to the tree
				_layoutRoot.Children().Append(newContent);
			}

			// New child means re-layout is necessary
			InvalidateMeasure();
		}
	}
	void LayoutTransformControl::TransformChanged(DependencyObject o, DependencyPropertyChangedEventArgs e)
	{
		auto ltc = o.try_as<NotePad::LayoutTransformControl>();
		if (ltc) {

			NotePad::implementation::LayoutTransformControl* ptr{ winrt::from_abi<NotePad::implementation::LayoutTransformControl>(ltc) };
			ptr->OnTransformChanged(
				e.OldValue().try_as<Media::Transform>(),
				e.NewValue().try_as<Media::Transform>());

		}
	}
	void LayoutTransformControl::OnTransformChanged(Media::Transform oldValue, Media::Transform newValue)
	{
		if (oldValue != nullptr)
		{
			UnsubscribeFromTransformPropertyChanges(oldValue);
		}

		if (newValue != nullptr)
		{
			SubscribeToTransformPropertyChanges(newValue);
		}

		ProcessTransform();
	}
	void LayoutTransformControl::UnsubscribeFromTransformPropertyChanges(Media::Transform transform)
	{
		auto propertyChangeEventSources =
			_transformPropertyChangeEventSources[transform];
		auto propertyChangeEventSourcesToken = _transformPropertyChangeEventSourcesToken[transform];
		for (int i=0;i< propertyChangeEventSources.size();i++)
		{
			auto pces = propertyChangeEventSources[i];
			pces.ValueChanged(propertyChangeEventSourcesToken[i]);
		}

		_transformPropertyChangeEventSources.erase(transform);
	}
	void LayoutTransformControl::SubscribeToTransformPropertyChanges(Media::Transform transform)
	{
		auto transformGroup = transform.try_as< TransformGroup>();

		if (transformGroup != nullptr)
		{
			for (auto childTransform : transformGroup.Children())
			{
				SubscribeToTransformPropertyChanges(childTransform);
			}

			return;
		}

		std::vector<PropertyChangeEventSource> propertyChangeEventSources;
		std::vector<winrt::event_token> propertyChangeEventSourcesToken;
		_transformPropertyChangeEventSources[transform] = propertyChangeEventSources;
		_transformPropertyChangeEventSourcesToken[transform] = propertyChangeEventSourcesToken;
		auto rotateTransform = transform.try_as< RotateTransform>();

		if (rotateTransform != nullptr)
		{
			auto anglePropertyChangeEventSource =
				PropertyChangeEventSource(
					rotateTransform,
					L"Angle", BindingMode::TwoWay);
			auto token = anglePropertyChangeEventSource.ValueChanged(EventHandler<double>(this, &LayoutTransformControl::OnTransformPropertyChanged));
			propertyChangeEventSourcesToken.push_back(token);
			propertyChangeEventSources.push_back(anglePropertyChangeEventSource);

			return;
		}

		auto scaleTransform = transform.try_as< ScaleTransform>();

		if (scaleTransform != nullptr)
		{
			auto scaleXPropertyChangeEventSource =
				PropertyChangeEventSource(
					scaleTransform,
					L"ScaleX", BindingMode::TwoWay);

			auto tokenx = scaleXPropertyChangeEventSource.ValueChanged(EventHandler<double>(this, &LayoutTransformControl::OnTransformPropertyChanged));
			propertyChangeEventSourcesToken.push_back(tokenx);
			propertyChangeEventSources.push_back(scaleXPropertyChangeEventSource);
			auto scaleYPropertyChangeEventSource =
				PropertyChangeEventSource(
					scaleTransform,
					L"ScaleY", BindingMode::TwoWay);

			auto tokeny = scaleYPropertyChangeEventSource.ValueChanged(EventHandler<double>(this, &LayoutTransformControl::OnTransformPropertyChanged));
			propertyChangeEventSourcesToken.push_back(tokeny);
			propertyChangeEventSources.push_back(scaleYPropertyChangeEventSource);
			return;
		}

		auto skewTransform = transform.try_as<SkewTransform>();

		if (skewTransform != nullptr)
		{
			auto angleXPropertyChangeEventSource =
				PropertyChangeEventSource(
					skewTransform,
					L"AngleX", BindingMode::TwoWay);

			auto tokenx = angleXPropertyChangeEventSource.ValueChanged(EventHandler<double>(this, &LayoutTransformControl::OnTransformPropertyChanged));
			propertyChangeEventSourcesToken.push_back(tokenx);
			propertyChangeEventSources.push_back(angleXPropertyChangeEventSource);
			auto angleYPropertyChangeEventSource =
				PropertyChangeEventSource(
					skewTransform,
					L"AngleY", BindingMode::TwoWay);
	
			auto tokeny = angleYPropertyChangeEventSource.ValueChanged(EventHandler<double>(this, &LayoutTransformControl::OnTransformPropertyChanged));
			propertyChangeEventSourcesToken.push_back(tokeny);
			propertyChangeEventSources.push_back(angleYPropertyChangeEventSource);
			return;
		}

		auto matrixTransform = transform.try_as< MatrixTransform>();

		if (matrixTransform != nullptr)
		{
			auto matrixPropertyChangeEventSource =
				PropertyChangeEventSource(
					matrixTransform,
					L"Matrix", BindingMode::TwoWay);
			auto token = matrixPropertyChangeEventSource.ValueChanged(EventHandler<double>(this, &LayoutTransformControl::OnTransformPropertyChanged));
			propertyChangeEventSourcesToken.push_back(token);
			propertyChangeEventSources.push_back(matrixPropertyChangeEventSource);
		}
	}

	/// <summary>
	/// Called when a property of a Transform changes.
	/// </summary>

	void LayoutTransformControl::OnTransformPropertyChanged(IInspectable sender, double e)
	{
		TransformUpdated();
	}
}
