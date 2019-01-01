/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/



#ifndef FILTER_UTILITY_FUNCTIONS_HPP
#define FILTER_UTILITY_FUNCTIONS_HPP


#include "network/basictypes.hpp"


class ChunkSpace;
class Entity;
class Vector3;

namespace FilterUtilityFunctions
{

	void coordinateSystemCheck(	Entity * pEntity,
								SpaceID spaceID,
								EntityID vehicleID );


	bool resolveOnGroundPosition(	Position3D & position,
									bool & onGround );

	bool filterDropPoint(	ChunkSpace * pSpace,
							const Position3D & fall,
							Position3D & land,
							float maxDrop = 100.f,
							Vector3 * groundNormal = NULL );


	void transformIntoCommon(	SpaceID s,
								EntityID v,
								Position3D & pos,
								Vector3 & dir );

	void transformFromCommon(	SpaceID s,
								EntityID v,
								Position3D & pos,
								Vector3 & dir );

};




#endif // FILTER_UTILITY_FUNCTIONS_HPP

// filter_utility_functions.hpp
