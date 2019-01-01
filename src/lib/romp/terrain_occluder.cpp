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

#include "terrain_occluder.hpp"

#ifndef CODE_INLINE
#include "terrain_occluder.ipp"
#endif

TerrainOccluder::TerrainOccluder()
{
}

TerrainOccluder::~TerrainOccluder()
{
}


float TerrainOccluder::collides(
		const Vector3 & photonSourcePosition,
		const Vector3 & cameraPosition,
		const LensEffect& le )
{
	//Check against terrain
	//Vector3	extent = cameraPosition + photonSourcePosition.applyToUnitAxisVector(2) * 500.f;
/*	if (Moo::Terrain::instance().getContact( cameraPosition, photonSourcePosition ) != photonSourcePosition)
	{
		return 0.f;
	}*/

	return 1.f;
}

std::ostream& operator<<(std::ostream& o, const TerrainOccluder& t)
{
	o << "TerrainOccluder\n";
	return o;
}


/*terrain_occluder.cpp*/
