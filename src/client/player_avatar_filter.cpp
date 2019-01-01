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

#include "player_avatar_filter.hpp"

#include "entity.hpp"

#include "filter_utility_functions.hpp"



PY_TYPEOBJECT( PlayerAvatarFilter )

PY_BEGIN_METHODS( PlayerAvatarFilter )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PlayerAvatarFilter )
PY_END_ATTRIBUTES()




///////////////////////////////////////////////////////////////////////////////
///  Member Documentation for PlayerAvatarFilter
///////////////////////////////////////////////////////////////////////////////

/**
 *	@property	double PlayerAvatarFilter::time_
 *
 *	This member holds the time stamp of the most recent input to be received.
 */


/**
 *	@property	SpaceID PlayerAvatarFilter::spaceID_
 *
 *	This member holds the space ID of the most recent input to be
 *	received.
 */


/**
 *	@property	EntityID PlayerAvatarFilter::vehicleID_
 *
 *	This member holds the vehicle ID of the most recent input to be
 *	received. It may be null.
 */


/**
 *	@property	Position3D PlayerAvatarFilter::pos_
 *
 *	This member holds the position provided by the most recent input to be
 *	received in vehicle coordinates.
 */


/**
 *	@property	float PlayerAvatarFilter::headYaw_
 *
 *	This member holds the yaw of the head in vehicle coordinates.
 */


/**
 *	@property	float PlayerAvatarFilter::headPitch_
 *
 *	This member holds the pitch of the head in vehicle coordinates.
 */


/**
 *	@property	float PlayerAvatarFilter::aux2_
 *
 *	This is not actually used to roll the avatar head however the 3rd auxiliary
 *	parameter is preserved and passed on anyway.
 */

///////////////////////////////////////////////////////////////////////////////
///  End Member Documentation for PlayerAvatarFilter
///////////////////////////////////////////////////////////////////////////////




/*~ function BigWorld.PlayerAvatarFilter
 *
 *	This function creates a new PlayerAvatarFilter.  This updates the position
 *	and yaw of the entity to those specified by the most recent server update.
 *
 *	@return	a new PlayerAvatarFilter
 */
PY_FACTORY( PlayerAvatarFilter, BigWorld )


/**
 *	Constructor
 *
 *	@param	pType	The python object defining the type of the filter. 
 */
PlayerAvatarFilter::PlayerAvatarFilter( PyTypePlus * pType ) :
	Filter( pType ),
	time_( -1000.0 ),
	spaceID_( 0 ),
	vehicleID_( 0 ),
	pos_( 0, 0, 0 ),
	headYaw_( 0 ),
	headPitch_( 0 ),
	aux2_( 0 )
{
}


/**
 *	Destructor
 */
PlayerAvatarFilter::~PlayerAvatarFilter()
{
}


/**
 *	This method updates the player filter with a new set of inputs.
 *
 *	@param	time		The client time when the input was received.
 *	@param	spaceID		The server space that the position resides in.
 *	@param	vehicleID	The ID of the vehicle in who's coordinate system the
 *						position is defined. A null vehicle ID means that the
 *						position is in world coordinates. 
 *	@param	pos			The new position in either local vehicle or 
 *						world space/common coordinates.
 *	@param	posError	The amount of uncertainty in the position.
 *	@param	auxFiltered	If not NULL, a pointer to an array of three floats
 *						representing yaw pitch and roll.
 */
void PlayerAvatarFilter::input(	double time,
								SpaceID spaceID,
								EntityID vehicleID,
								const Position3D & pos,
								const Vector3 & posError,
								float * auxFiltered )
{
	BW_GUARD;
	time_ = time;

	spaceID_ = spaceID;
	vehicleID_ = vehicleID;
	pos_ = pos;
	if ( auxFiltered != NULL )
	{
		headYaw_ = auxFiltered[0];
		headPitch_ = auxFiltered[1];
		aux2_ = auxFiltered[2];
	}
}


/**
 *	This method updates the owner entity's position and direction with that
 *	stored in the filter; but only if time has progressed since the last
 *	time the filter was queried.
 *
 *	@param	time	The client game time in seconds to which the entity's volatile
 *					members should be updated.
 */
void PlayerAvatarFilter::output( double time )
{
	BW_GUARD;
	if (entity_->isPhysicsAllowed())
	{
		FilterUtilityFunctions::coordinateSystemCheck( entity_, spaceID_, vehicleID_ );

		float	aux[3];
		aux[0] = headYaw_;
		aux[1] = headPitch_;
		aux[2] = aux2_;
		entity_->pos( pos_, aux, 3 );
	}
}


/**
 *	This function gets the last input received by the filter.
 *	Its arguments will be untouched if no input has yet been received.
 *
 *	@param	time		Will be set to the time stamp of the last input.
 *	@param	spaceID		Will be set to the space ID of the last input.
 *	@param	vehicleID	Will be set to the vehicle ID provided in the last
 *						input.
 *	@param	pos			The position of received in the last input, with its
 *						height fully resolved.
 *	@param	posError	The amount of uncertainty in the position.
 *	@param	auxFiltered	An array of two floats into which the yaw and pitch of
 *						the last input will be placed.
 *
 *	@return				Returns true if an input existed and the values in the parameters
 *						were set.
 */
bool PlayerAvatarFilter::getLastInput(	double & time,
										SpaceID & spaceID,
										EntityID & vehicleID,
										Position3D & pos,
										Vector3 & posError,
										float * auxFiltered )
{
	BW_GUARD;
	if ( time_ != -1000.0 )
	{
		time = this->time_;
		spaceID = this->spaceID_;
		vehicleID = this->vehicleID_;
		pos = this->pos_;
		if ( auxFiltered != NULL )
		{
			auxFiltered[0] = this->headYaw_;
			auxFiltered[1] = this->headPitch_;
			auxFiltered[2] = this->aux2_;
		}
		return true;
	}
	return false;
}


/**
 *	Python factory method
 */
PyObject * PlayerAvatarFilter::pyNew( PyObject * args )
{
	BW_GUARD;
	if ( PyTuple_Size( args ) != 0 )
	{
		PyErr_SetString( PyExc_TypeError,
			"BigWorld.PlayerAvatarFilter() expects no arguments" );
		return NULL;
	}

	return new PlayerAvatarFilter();
}
