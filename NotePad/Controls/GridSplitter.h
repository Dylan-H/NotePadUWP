#pragma once

#include "GridSplitter.g.h"
#include "GripperHoverWrapper.h"

namespace winrt::NotePad::implementation
{
	using namespace winrt::Windows::UI::Xaml;
	using namespace winrt::Windows::System;
	using namespace winrt::Windows::UI::Xaml::Media;
	struct GridSplitter : GridSplitterT<GridSplitter>
	{

		/// <summary>
		/// Initializes a new instance of the <see cref="GridSplitter"/> class.
		/// </summary>
		GridSplitter();
		static const int GripperCustomCursorDefaultResource = -1;
		static CoreCursor ColumnsSplitterCursor;
		static CoreCursor RowSplitterCursor;

		CoreCursor _PreviousCursor{ nullptr };
		CoreCursor PreviousCursor() { return _PreviousCursor; };
		void PreviousCursor(CoreCursor value) { _PreviousCursor = value; };
		UIElement Element() { return winrt::unbox_value<UIElement>(GetValue(_ElementProperty)); };
		void Element(Windows::UI::Xaml::UIElement const& value) { SetValue(_ElementProperty, winrt::box_value(value)); };

		NotePad::GridResizeDirection ResizeDirection() { return winrt::unbox_value< NotePad::GridResizeDirection>(GetValue(_ResizeDirectionProperty)); };
		void ResizeDirection(NotePad::GridResizeDirection const& value) { SetValue(_ResizeDirectionProperty, winrt::box_value(value)); };

		NotePad::GridResizeBehavior ResizeBehavior() { return winrt::unbox_value<NotePad::GridResizeBehavior>(GetValue(_ResizeBehaviorProperty)); };
		void ResizeBehavior(NotePad::GridResizeBehavior const& value) { SetValue(_ResizeBehaviorProperty, winrt::box_value(value)); };

		Windows::UI::Xaml::Media::Brush GripperForeground() { return winrt::unbox_value<Brush>(GetValue(_GripperForegroundProperty)); };
		void GripperForeground(Windows::UI::Xaml::Media::Brush const& value) { SetValue(_GripperForegroundProperty, winrt::box_value(value)); };

		int32_t ParentLevel() { return winrt::unbox_value<int32_t>(GetValue(_ParentLevelProperty)); };
		void ParentLevel(int32_t value) { SetValue(_ParentLevelProperty, winrt::box_value(value)); };

		NotePad::GripperCursorType GripperCursor() { return winrt::unbox_value<NotePad::GripperCursorType>(GetValue(_GripperCursorProperty)); };
		void GripperCursor(NotePad::GripperCursorType const& value) { SetValue(_GripperCursorProperty, winrt::box_value(value)); };

		int32_t GripperCustomCursorResource() { return winrt::unbox_value<int32_t>(GetValue(_GripperCustomCursorResourceProperty)); };
		void GripperCustomCursorResource(int32_t value) { SetValue(_GripperCustomCursorResourceProperty, winrt::box_value(value)); };
		NotePad::SplitterCursorBehavior CursorBehavior() { return winrt::unbox_value<NotePad::SplitterCursorBehavior>(GetValue(_CursorBehaviorProperty)); };
		void CursorBehavior(NotePad::SplitterCursorBehavior const& value) { SetValue(_CursorBehaviorProperty, winrt::box_value(value)); };

		static DependencyProperty ElementProperty() { return _ElementProperty; };
		static DependencyProperty ResizeDirectionProperty() { return _ResizeDirectionProperty; };
		static DependencyProperty ResizeBehaviorProperty() { return _ResizeBehaviorProperty; };
		static DependencyProperty GripperForegroundProperty() { return _GripperForegroundProperty; };
		static DependencyProperty ParentLevelProperty() { return _ParentLevelProperty; };
		static DependencyProperty GripperCursorProperty() { return _GripperCursorProperty; };
		static DependencyProperty GripperCustomCursorResourceProperty() { return _GripperCustomCursorResourceProperty; };
		static DependencyProperty CursorBehaviorProperty() { return _CursorBehaviorProperty; };


		/// <inheritdoc />
		void OnKeyDown(KeyRoutedEventArgs e);

		/// <inheritdoc />
		void OnManipulationStarted(ManipulationStartedRoutedEventArgs e);

		/// <inheritdoc />
		void OnManipulationCompleted(ManipulationCompletedRoutedEventArgs e);

		/// <inheritdoc />
		void OnManipulationDelta(ManipulationDeltaRoutedEventArgs e);



		/// <inheritdoc />
		void OnApplyTemplate();
	private:

		static void OnGripperForegroundPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e);
		static void OnGripperCursorPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e);
		static void GripperCustomCursorResourcePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e);
		static void CursorBehaviorPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e);
		static void OnElementPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e);

		static bool IsStarColumn(ColumnDefinition definition);

		static bool IsStarRow(RowDefinition definition);
	private:
		Grid Resizable() {
			auto ret = TargetControl();
			if (ret) {
				return  ret.Parent().try_as<Grid>();
			}
			return nullptr;
		}
		FrameworkElement TargetControl();


		/// <summary>
		/// Gets the current Column definition of the parent Grid
		/// </summary>
		ColumnDefinition CurrentColumn();

		/// <summary>
		/// Gets the Sibling Column definition of the parent Grid
		/// </summary>
		ColumnDefinition SiblingColumn();

		/// <summary>
		/// Gets the current Row definition of the parent Grid
		/// </summary>
		RowDefinition CurrentRow();

		/// <summary>
		/// Gets the Sibling Row definition of the parent Grid
		/// </summary>
		RowDefinition SiblingRow();

		void GridSplitter_PointerReleased(winrt::Windows::Foundation::IInspectable sender, PointerRoutedEventArgs e);

		void GridSplitter_PointerPressed(winrt::Windows::Foundation::IInspectable sender, PointerRoutedEventArgs e);

		void GridSplitter_PointerExited(winrt::Windows::Foundation::IInspectable sender, PointerRoutedEventArgs e);

		void GridSplitter_PointerEntered(winrt::Windows::Foundation::IInspectable sender, PointerRoutedEventArgs e);

		void GridSplitter_ManipulationCompleted(winrt::Windows::Foundation::IInspectable sender, ManipulationCompletedRoutedEventArgs e);

		void GridSplitter_ManipulationStarted(winrt::Windows::Foundation::IInspectable sender, ManipulationStartedRoutedEventArgs e);
		bool SetColumnWidth(ColumnDefinition columnDefinition, double horizontalChange, GridUnitType unitType);

		bool IsValidColumnWidth(ColumnDefinition columnDefinition, double horizontalChange);

		bool SetRowHeight(RowDefinition rowDefinition, double verticalChange, GridUnitType unitType);

		bool IsValidRowHeight(RowDefinition rowDefinition, double verticalChange);

		// Return the targeted Column based on the resize behavior
		int GetTargetedColumn();

		// Return the sibling Row based on the resize behavior
		int GetTargetedRow();

		// Return the sibling Column based on the resize behavior
		int GetSiblingColumn();

		// Return the sibling Row based on the resize behavior
		int GetSiblingRow();

		// Gets index based on resize behavior for first targeted row/column
		int GetTargetIndex(int currentIndex);

		// Gets index based on resize behavior for second targeted row/column
		int GetSiblingIndex(int currentIndex);

		// Checks the control alignment and Width/Height to detect the control resize direction columns/rows
		GridResizeDirection GetResizeDirection();

		// Get the resize behavior (Which columns/rows should be resized) based on alignment and Direction
		GridResizeBehavior GetResizeBehavior();

		void GridSplitter_Loaded(Windows::Foundation::IInspectable sender, RoutedEventArgs e);

		void CreateGripperDisplay();



		bool VerticalMove(double verticalChange);

		bool HorizontalMove(double horizontalChange);

	private:
		static DependencyProperty _ElementProperty;
		static DependencyProperty _ResizeDirectionProperty;
		static DependencyProperty _ResizeBehaviorProperty;
		static DependencyProperty _GripperForegroundProperty;
		static DependencyProperty _ParentLevelProperty;
		static DependencyProperty _GripperCursorProperty;
		static DependencyProperty _GripperCustomCursorResourceProperty;
		static DependencyProperty _CursorBehaviorProperty;

		winrt::event_token GridSplitter_LoadedToken;
		winrt::event_token GridSplitter_PointerEnteredToken;
		winrt::event_token GridSplitter_PointerExitedToken;
		winrt::event_token GridSplitter_PointerPressedToken;
		winrt::event_token GridSplitter_PointerReleasedToken;
		winrt::event_token GridSplitter_ManipulationStartedToken;
		winrt::event_token GridSplitter_ManipulationCompletedToken;


		const winrt::hstring GripperBarVertical = L"\xE784";
		const winrt::hstring GripperBarHorizontal = L"\xE76F";
		const winrt::hstring GripperDisplayFont = L"Segoe MDL2 Assets";
		GridResizeDirection _resizeDirection;
		GridResizeBehavior _resizeBehavior;
		GripperHoverWrapper *_hoverWrapper=nullptr;
		TextBlock _gripperDisplay{ nullptr };
		bool _pressed = false;
		bool _dragging = false;
		bool _pointerEntered = false;
	};
}

namespace winrt::NotePad::factory_implementation
{
	struct GridSplitter : GridSplitterT<GridSplitter, implementation::GridSplitter>
	{
	};
}
