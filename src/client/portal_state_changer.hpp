/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PORTAL_STATE_CHANGER_HPP
#define PORTAL_STATE_CHANGER_HPP

#include "physics2/worldtri.hpp"

class Entity;

namespace PortalStateChanger
{
	void changePortalCollisionState(
		Entity * pEntity, bool isPermissive, WorldTriangle::Flags flags );
	void tick( float dTime );
}

#endif // PORTAL_STATE_CHANGER_HPP