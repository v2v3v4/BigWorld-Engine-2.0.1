/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ORTHOGRAPHIC_CAMERA_HPP
#define ORTHOGRAPHIC_CAMERA_HPP

#include <iostream>
#include "base_camera.hpp"
#include "math/boundbox.hpp"

/**
 * This class implements an orthographic camera that
 * can be driven around using the keyboard and mouse
 */
class OrthographicCamera : public BaseCamera
{
public:
	OrthographicCamera();
	~OrthographicCamera();

	void update( float dTime, bool activeInputHandler = true );
	bool handleKeyEvent( const KeyEvent & );
	bool handleMouseEvent( const MouseEvent & );

    void view( const Matrix & );
	void view( const OrthographicCamera& other );

	void limit( const BoundingBox b ) { limit_ = b; }
	BoundingBox limit() { return limit_; }

private:
	void	handleInput( float dTime );
    void 	viewToPolar();
    void 	polarToView();

	OrthographicCamera(const OrthographicCamera&);
	OrthographicCamera& operator=(const OrthographicCamera&);

	float		up_;
	float		right_;
	typedef std::map< KeyCode::Key, bool>	KeyDownMap;
	KeyDownMap	keyDown_;

	//For hiding and showing the mouse cursor
	bool isCursorHidden_;

	// Where the cursor was when we started looking around
	POINT lastCursorPosition_;

	/** The extents the camera may move to, applied in update() */
	BoundingBox limit_;

	friend std::ostream& operator<<(std::ostream&, const OrthographicCamera&);
};

#endif  // ORTHOGRAPHIC_CAMERA_HPP
