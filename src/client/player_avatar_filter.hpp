/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef PLAYER_AVATAR_FILTER_HPP
#define PLAYER_AVATAR_FILTER_HPP


#include "filter.hpp"


/*~ class BigWorld.PlayerAvatarFilter
 *
 *	This class inherits from Filter.  It updates the position and
 *	yaw of the entity to the most recent update received from the
 *	server.
 *
 *	A new PlayerAvatarFilters can be created using the
 *	BigWorld.PlayerAvatarFilter() function.
 */
/**
 *	This is a simple filter intended for client controlled player entities.\n
 *	The PlayerAvatarFilter behaves almost exactly like the DumbFilter except
 *	it does not try and resolve 'on ground' input positions as it assumes it
 *	will never be given one.
 */
class PlayerAvatarFilter : public Filter
{
	Py_Header( PlayerAvatarFilter, Filter );

public:
	PlayerAvatarFilter( PyTypePlus * pType = &s_type_ );
	~PlayerAvatarFilter();

	void input(	double time,
				SpaceID spaceID,
				EntityID vehicleID,
				const Position3D & pos,
				const Vector3 & posError,
				float * auxFiltered );

	void output( double time );

	bool getLastInput(	double & time,
						SpaceID & spaceID,
						EntityID & vehicleID,
						Position3D & pos,
						Vector3 & posError,
						float * auxFiltered );

	PY_FACTORY_DECLARE();

private:

	double		time_;
	SpaceID		spaceID_;
	EntityID	vehicleID_;
	Position3D	pos_;
	float		headYaw_;
	float		headPitch_;
	float		aux2_;
};




#endif // PLAYER_AVATAR_FILTER_HPP

// player_avatar_filter.hpp
