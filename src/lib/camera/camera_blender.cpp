/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "camera_blender.hpp"

#ifndef CODE_INLINE
#include "camera_blender.ipp"
#endif

#include "math/blend_transform.hpp"


// -----------------------------------------------------------------------------
// Section: CameraBlender
// -----------------------------------------------------------------------------

/**
 *	Constructor
 */
CameraBlender::CameraBlender( float maxAge ) :
	BaseCamera( &BaseCamera::s_type_ ),	// should have own type object
	maxAge_( maxAge )
{
}


/**
 *	Add this camera to the pack, selecting it if there were
 *	none already there.
 */
void CameraBlender::add( BaseCameraPtr pCam )
{
	BW_GUARD;
	elts_.push_back( CameraBlend( pCam ) );
	if (elts_.size() == 1) elts_.back().prop_ = 2.f;
}


/**
 *	Make this camera the intended one
 */
void CameraBlender::select( int index )
{
	BW_GUARD;
	int sz = elts_.size();
	for (int i = 0; i < sz; i++)
	{
		float & prop = elts_[i].prop_;
		if (i == index)
			prop = 2.f;
		else if (prop > 1.f)
			prop = 1.f;
	}
}


/**
 *	Update all the cameras, and blend them as appropriate.
 */
void CameraBlender::update( float dTime )
{
	BW_GUARD;
	BlendTransform bt( Matrix::identity );
	float sumProp = 0.f;

	// go through all our cameras;
	int sz = elts_.size();
	for (int i = 0; i < sz; i++)
	{
		float & prop = elts_[i].prop_;
		if (prop <= 0.f) continue;

		// decay old cameras
		if (prop <= 1.f)
		{
			prop -= dTime / maxAge_;
			if (prop <= 0.f)
			{
				prop = 0.f;
				continue;
			}
		}

		// update it if we're still interested in it
		elts_[i].pCam_->update( dTime );

		// and add its contribution to the blend transfrom
		sumProp += prop;
		bt.blend( prop / sumProp,
			BlendTransform( elts_[i].pCam_->invView() ) );
	}

	// now put the output into our matrices
	bt.output( view_ );
	invView_.invert( view_ );
}

// camera_blender.cpp
