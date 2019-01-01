/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOUSE_DRAG_HANDLER_HPP
#define MOUSE_DRAG_HANDLER_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"


/**
 *	This class handles mouse dragging painting, that is, holding down the mouse
 *	and detecting when the mouse buttons has been pressed or released outside
 *	the main view window.
 */
class MouseDragHandler
{
public:
	enum MouseKey
	{
		KEY_LEFTMOUSE	= 0,
		KEY_MIDDLEMOUSE = 1,
		KEY_RIGHTMOUSE	= 2
	};

	MouseDragHandler();

	void setDragging( MouseKey key, const bool val );
	bool isDragging( MouseKey key ) const;

protected:
	bool updateDragging( bool currentState, bool & stateVar ) const; 

private:
	mutable bool 	draggingLeftMouse_;
	mutable bool 	draggingMiddleMouse_;
	mutable bool 	draggingRightMouse_;
};


#endif // MOUSE_DRAG_HANDLER_HPP
