/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	This class checks line-of-sight of the sun flare only,
 *	through the sky domes using occlusion queries.
 */

#ifndef SKY_DOME_OCCLUDER_HPP
#define SKY_DOME_OCCLUDER_HPP

#include "photon_occluder.hpp"
#include "occlusion_query_helper.hpp"
#include "moo/device_callback.hpp"

class LensEffect;

/**
 * This class implements the PhotonOccluder interface, and provides
 * occlusion information for lens flares, especially the sun flare.
 *
 * It works by setting the "OcclusionTestEnable" flag as an effect
 * constant so that the existing sky domes, whatever they may be, can
 * be drawn again using special alpha-test occlusion pass.
 *
 * In order to capture the occlusion information, a view matrix is
 * constructed that makes the camera look directly at the sun; this
 * has the effect of drawing all possible sun occluders directly in
 * the middle of the screen.  A scissors rectangle is used to clip these
 * results to the size of the sun, and directX occlusion queries are
 * used to record how many pixels of sun are visible.
 */
class SkyDomeOccluder : public PhotonOccluder, public Moo::DeviceCallback
{
public:
	SkyDomeOccluder( class EnviroMinder& enviroMinder );
	~SkyDomeOccluder();

	static bool isAvailable();

	virtual	float collides(
			const Vector3 & photonSourcePosition,
			const Vector3 & cameraPosition,
			const LensEffect& le );
	virtual void beginOcclusionTests();
	virtual void endOcclusionTests();

	virtual void deleteUnmanagedObjects();
private:
	void getLookAtSunViewMatrix( Matrix& out );

	uint32 drawOccluders( float size );

	UINT	lastResult_;
	uint32	possiblePixels_;

	OcclusionQueryHelper	helper_;
	class EnviroMinder& enviroMinder_;
};

#endif
