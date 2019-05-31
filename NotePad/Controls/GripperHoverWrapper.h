#pragma once


namespace winrt::NotePad::implementation
{
	class GripperHoverWrapper
	{
	public:
		/// <summary>
	/// Initializes a new instance of the <see cref="GripperHoverWrapper"/> class that add cursor change on hover functionality for GridSplitter.
	/// </summary>
	/// <param name="element">UI element to apply cursor change on hover</param>
	/// <param name="gridSplitterDirection">GridSplitter resize direction</param>
	/// <param name="gripperCursor">GridSplitter gripper on hover cursor type</param>
	/// <param name="gripperCustomCursorResource">GridSplitter gripper custom cursor resource number</param>
		GripperHoverWrapper(UIElement element, GridResizeDirection gridSplitterDirection, GripperCursorType gripperCursor, int gripperCustomCursorResource);
		~GripperHoverWrapper();


		GripperCursorType GripperCursor()
		{
			return _gripperCursor;
		}

		void GripperCursor(GripperCursorType value)
		{
			_gripperCursor = value;
		}

		int GripperCustomCursorResource()
		{

			return _gripperCustomCursorResource;
		}

		void GripperCustomCursorResource(int value)
		{
			_gripperCustomCursorResource = value;
		}

		void UpdateHoverElement(UIElement element);

		void SplitterManipulationStarted(winrt::Windows::Foundation::IInspectable sender, ManipulationStartedRoutedEventArgs e);

		void SplitterManipulationCompleted(winrt::Windows::Foundation::IInspectable sender, ManipulationCompletedRoutedEventArgs e);

		void UnhookEvents();

	private:
		void Element_PointerExited(winrt::Windows::Foundation::IInspectable sender, PointerRoutedEventArgs e);

		void Element_PointerEntered(winrt::Windows::Foundation::IInspectable sender, PointerRoutedEventArgs e);

		void UpdateDisplayCursor();
	private:
		winrt::event_token Element_PointerEnteredToken;
		winrt::event_token Element_PointerExitedToken;
		GridResizeDirection _gridSplitterDirection;

		CoreCursor _splitterPreviousPointer{ nullptr };
		CoreCursor _previousCursor{ nullptr };
		GripperCursorType _gripperCursor;
		int _gripperCustomCursorResource;
		bool _isDragging = false;
		UIElement _element{ nullptr };
	};

}

