/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CAMERA_BLENDER_HPP
#define CAMERA_BLENDER_HPP

#include "base_camera.hpp"

/**
 *	This is a record of a camera and how much it affects the eventual
 *	world transform.
 */
struct CameraBlend
{
	explicit CameraBlend( BaseCameraPtr pCam ) :
		pCam_( pCam ), prop_( 0.f ) { }

	BaseCameraPtr	pCam_;
	float			prop_;
};

typedef std::vector<CameraBlend> CameraBlends;


/**
 *	This is a vector of selected cameras, with routines to move a
 *	camera to prime position and decay the influence of others.
 */
class CameraBlender : public BaseCamera
{
public:
	CameraBlender( float maxAge = 0.5f );

	void add( BaseCameraPtr pCam );
	void select( int index );

	virtual void set( const Matrix & viewMatrix );
	virtual void update( float dTime );

	CameraBlends & elts() { return elts_; }

private:
	CameraBlends	elts_;
	float			maxAge_;
};



#ifdef CODE_INLINE
#include "camera_blender.ipp"
#endif

#endif // CAMERA_BLENDER_HPP
