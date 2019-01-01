/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOUSE_LOOK_CAMERA_HPP
#define MOUSE_LOOK_CAMERA_HPP

#include <iostream>
#include "base_camera.hpp"
#include "math/boundbox.hpp"

/**
 * This class implements a mouse look camera that
 * can be driven around using the keyboard
 */
class MouseLookCamera : public BaseCamera
{
public:
	MouseLookCamera();
	~MouseLookCamera();

	void update( float dTime, bool activeInputHandler = true );
	bool handleKeyEvent( const KeyEvent & event );
	bool handleMouseEvent( const MouseEvent & event );

	const Matrix & view() const { return BaseCamera::view(); }
	void view( const Matrix & );
	void view( const MouseLookCamera& other );

	void limit( const BoundingBox b ) { limit_ = b; }
	BoundingBox limit() { return limit_; }

private:
	void	handleInput( float dTime );
    void 	viewToPolar();
    void 	polarToView();

	MouseLookCamera(const MouseLookCamera&);
	MouseLookCamera& operator=(const MouseLookCamera&);

	float		pitch_;
	float		yaw_;

	//Stuff to handle key events correctly
	typedef std::map< KeyCode::Key, bool>	KeyDownMap;
	KeyDownMap	keyDown_;

	//For hiding and showing the mouse cursor
	bool isCursorHidden_;

	/**
	 * Where the cursor was when we started looking around,
	 * so we can restore it to here when done
	 */
	POINT lastCursorPosition_;

	/**
	 *  These last pos, yaw and pitch variables keep track of modifications to
	 *  the camera, and only modify the camera if any of these values change.
	 *  This prevents the camera from slowly moving without user interaction
	 *  due to fp imprecisions when doing a 2x inverts.
	 */
	Vector3 lastPos_;
	float lastYaw_;
	float lastPitch_;

	/** The extents the camera may move to, applied in update() */
	BoundingBox limit_;

	friend std::ostream& operator<<(std::ostream&, const MouseLookCamera&);
};

#ifdef CODE_INLINE
#include "mouse_look_camera.ipp"
#endif

typedef SmartPointer<MouseLookCamera> MouseLookCameraPtr;

#endif
/*mouse_look_camera.hpp*/
