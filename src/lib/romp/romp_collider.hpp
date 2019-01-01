/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ROMP_COLLIDER_HPP
#define ROMP_COLLIDER_HPP

#include "cstdmf/smartpointer.hpp"

/**
 *	This abstract class is the interface between the particle source's
 *	'grounded' feature and whatever collision scene is implemented.
 *
 *	The ground() method returns the height of the ground beneath the
 *	particle or NO_GROUND_COLLISION if none was found.
 */
class RompCollider : public ReferenceCount
{
public:
    virtual float ground( const Vector3 &pos, float dropDistance, bool oneSided = false ) = 0;
	virtual float ground( const Vector3 &pos ) = 0;
	virtual float collide( const Vector3 &start, const Vector3& end, class WorldTriangle& result ) = 0;

    static float NO_GROUND_COLLISION;
};

typedef SmartPointer<RompCollider> RompColliderPtr;

#endif // ROMP_COLLIDER_HPP
