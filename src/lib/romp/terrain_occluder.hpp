/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_OCCLUDER_HPP
#define TERRAIN_OCCLUDER_HPP

#include <iostream>

#include "photon_occluder.hpp"

/**
 * TODO: to be documented.
 */
class TerrainOccluder : public PhotonOccluder
{
public:
	TerrainOccluder();
	~TerrainOccluder();

	virtual float collides(
			const Vector3 & photonSourcePosition,
			const Vector3 & cameraPosition,
			const LensEffect& le );

private:
	TerrainOccluder(const TerrainOccluder&);
	TerrainOccluder& operator=(const TerrainOccluder&);

	friend std::ostream& operator<<(std::ostream&, const TerrainOccluder&);
};

#ifdef CODE_INLINE
#include "terrain_occluder.ipp"
#endif




#endif
/*terrain_occluder.hpp*/
