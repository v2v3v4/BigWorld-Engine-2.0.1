/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PHOTON_OCCLUDER_HPP
#define PHOTON_OCCLUDER_HPP

#include "math/vector3.hpp"
class LensEffect;

/**
 *	This class defines the interface to all photon occluders, meaning
 *	those classes that can return yes or no : 
 *	I am in the way of the light source.
 */
class PhotonOccluder
{
public:
	PhotonOccluder()			{};
	virtual ~PhotonOccluder()	{};

	virtual float collides(
			const Vector3 & photonSourcePosition,
			const Vector3 & cameraPosition,
			const LensEffect& le ) = 0;
	virtual void beginOcclusionTests()	{};
	virtual void endOcclusionTests()	{};

private:
	PhotonOccluder(const PhotonOccluder&);
	PhotonOccluder& operator=(const PhotonOccluder&);
};

#ifdef CODE_INLINE
#include "photon_occluder.ipp"
#endif




#endif
/*photon_occluder.hpp*/