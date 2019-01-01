/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WORLD_HPP
#define WORLD_HPP

/**
 *	This class stores a scene, terrain and collision scene
 */
class World
{
public:
	World();
	~World();

	static void					drawDebugTriangles();

	// The instance method
	static World & instance();

private:
};

#ifdef CODE_INLINE
#include "world.ipp"
#endif




#endif
/*world.hpp*/