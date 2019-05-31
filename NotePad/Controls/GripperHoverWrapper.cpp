#include "pch.h"
#include "GripperHoverWrapper.h"
#include "GridSplitter.h"
namespace winrt::NotePad::implementation
{

	/// <summary>
	/// Initializes a new instance of the <see cref="GripperHoverWrapper"/> class that add cursor change on hover functionality for GridSplitter.
	/// </summary>
	/// <param name="element">UI element to apply cursor change on hover</param>
	/// <param name="gridSplitterDirection">GridSplitter resize direction</param>
	/// <param name="gripperCursor">GridSplitter gripper on hover cursor type</param>
	/// <param name="gripperCustomCursorResource">GridSplitter gripper custom cursor resource number</param>

	 GripperHoverWrapper::GripperHoverWrapper(UIElement element, GridResizeDirection gridSplitterDirection, GripperCursorType gripperCursor, int gripperCustomCursorResource)
	{
		_gridSplitterDirection = gridSplitterDirection;
		_gripperCursor = gripperCursor;
		_gripperCustomCursorResource = gripperCustomCursorResource;
		_element = element;
		UnhookEvents();
		
		Element_PointerEnteredToken=  _element.PointerEntered(PointerEventHandler(this, &GripperHoverWrapper::Element_PointerEntered));
		Element_PointerExitedToken = _element.PointerExited(PointerEventHandler(this, &GripperHoverWrapper::Element_PointerExited));
	}

	GripperHoverWrapper::~GripperHoverWrapper()
	{
	}
	 void GripperHoverWrapper::UpdateHoverElement(UIElement element)
	{
		UnhookEvents();
		_element = element;
		Element_PointerEnteredToken = _element.PointerEntered(PointerEventHandler(this, &GripperHoverWrapper::Element_PointerEntered));
		Element_PointerExitedToken = _element.PointerExited(PointerEventHandler(this, &GripperHoverWrapper::Element_PointerExited));
	}
	  void GripperHoverWrapper::SplitterManipulationStarted(winrt::Windows::Foundation::IInspectable sender, ManipulationStartedRoutedEventArgs e)
	 {
		 auto splitter = winrt::unbox_value<NotePad::GridSplitter>(sender);
		 if (splitter)
		 {
			 _splitterPreviousPointer = splitter.PreviousCursor();
			 _isDragging = true;
		 }
	 }
	   void GripperHoverWrapper::SplitterManipulationCompleted(winrt::Windows::Foundation::IInspectable sender, ManipulationCompletedRoutedEventArgs e)
	  {
		  auto splitter = winrt::unbox_value<NotePad::GridSplitter>(sender);
		  if (splitter)
		  {
			  Window::Current().CoreWindow().PointerCursor( _splitterPreviousPointer);
			  splitter.PreviousCursor(_splitterPreviousPointer);
			  _isDragging = false;
		  }


	  }
	    void GripperHoverWrapper::UnhookEvents()
	   {
		   if (_element)
		   {
			   _element.PointerEntered(Element_PointerEnteredToken);
			   _element.PointerExited(Element_PointerExitedToken);
		   }

	   }
		 void GripperHoverWrapper::Element_PointerExited(winrt::Windows::Foundation::IInspectable sender, PointerRoutedEventArgs e)
		{
			if (_isDragging)
			{
				// if dragging don't update the curser just update the splitter cursor with the last window cursor,
				// because the splitter is still using the arrow cursor and will revert to original case when drag completes
				_splitterPreviousPointer = _previousCursor;
			}
			else
			{
				Window::Current().CoreWindow().PointerCursor(_previousCursor);
			}
		}
		  void GripperHoverWrapper::Element_PointerEntered(winrt::Windows::Foundation::IInspectable sender, PointerRoutedEventArgs e)
		 {
			 // if not dragging
			 if (!_isDragging)
			 {
				 _previousCursor = _splitterPreviousPointer = Window::Current().CoreWindow().PointerCursor();
				 UpdateDisplayCursor();
			 }

			 // if dragging
			 else
			 {
				 _previousCursor = _splitterPreviousPointer;
			 }
		 }
		   void GripperHoverWrapper::UpdateDisplayCursor()
		  {
			  if (_gripperCursor == NotePad::GripperCursorType::Default)
			  {
				  if (_gridSplitterDirection == NotePad::GridResizeDirection::Columns)
				  {
					  Window::Current().CoreWindow().PointerCursor(GridSplitter::ColumnsSplitterCursor);
				  }
				  else if (_gridSplitterDirection == NotePad::GridResizeDirection::Rows)
				  {
					  Window::Current().CoreWindow().PointerCursor(GridSplitter::RowSplitterCursor);
				  }
			  }
			  else
			  {
				  auto coreCursor = (CoreCursorType)((int)_gripperCursor);
				  if (_gripperCursor == NotePad::GripperCursorType::Custom)
				  {
					  if (_gripperCustomCursorResource > GridSplitter::GripperCustomCursorDefaultResource)
					  {
						  Window::Current().CoreWindow().PointerCursor(CoreCursor(coreCursor, _gripperCustomCursorResource));
					  }
				  }
				  else
				  {
					  Window::Current().CoreWindow().PointerCursor(CoreCursor(coreCursor, 1));
				  }
			  }
		  }
}