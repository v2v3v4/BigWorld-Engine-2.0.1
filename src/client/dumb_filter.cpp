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


#include "dumb_filter.hpp"
#include "filter_utility_functions.hpp"

#include "entity.hpp"
#include "terrain/base_terrain_block.hpp"


DECLARE_DEBUG_COMPONENT2( "Entity", 0 )


PY_TYPEOBJECT( DumbFilter )

PY_BEGIN_METHODS( DumbFilter )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( DumbFilter )
PY_END_ATTRIBUTES()


///////////////////////////////////////////////////////////////////////////////
///  Member Documentation for DumbFilter
///////////////////////////////////////////////////////////////////////////////

/**
 *	@property	double DumbFilter::time_
 *
 *	This member holds the time stamp of the most recent input to be received.
 */


/**
 *	@property	Position3D DumbFilter::pos_
 *
 *	This member holds the position provided by the most recent input to be
 *	received in vehicle coordinates.
 */


/**
 *	@property	float DumbFilter::yaw_
 *
 *	This member holds the local space yaw of the most recent input to be
 *	received.
 */


/**
 *	@property	SpaceID DumbFilter::spaceID_
 *
 *	This member holds the space ID of the most recent input to be
 *	received.
 */


/**
 *	@property	EntityID DumbFilter::vehicleID_
 *
 *	This member holds the vehicle ID of the most recent input to be
 *	received. It may be null.
 */

///////////////////////////////////////////////////////////////////////////////
///  End Member Documentation for DumbFilter
///////////////////////////////////////////////////////////////////////////////


/*~ function BigWorld.DumbFilter
 *
 *	This function creates a new DumbFilter, which is the simplest filter,
 *	which just sets the position of the owning entity to the most recently received
 *	position from the server.
 *
 *	@return	A new DumbFilter object
 */
PY_FACTORY( DumbFilter, BigWorld )


/**
 *	This constructor sets up a DumbFilter for the input entity.
 */
DumbFilter::DumbFilter( PyTypePlus * pType ) :
	Filter( pType ),
	time_( -1000.0 ),
	spaceID_( 0 ),
	vehicleID_( 0 ),
	pos_( 0, 0, 0 ),
	yaw_( 0.f )
{
	BW_GUARD;	
}


/**
 *	This method invalidates all previously collected inputs. They will then
 *	be discarded when the next input is received.
 *
 *	@param	time	Ignored
 */
void DumbFilter::reset( double time )
{
	time_ = -1000.0;
}


/**
 *	This method updates the stored position and direction information with that
 *	provided. But only it the new data's time stamp if more recent.
 *
 *	@param	time		The estimated client time when the input was sent from
 *						the server.
 *	@param	spaceID		The server space that the position resides in.
 *	@param	vehicleID	The ID of the vehicle in who's coordinate system the
 *						position is defined. A null vehicle ID means that the
 *						position is in world coordinates. 
 *	@param	pos			The new position in either local vehicle or 
 *						world space/common coordinates. The player relative
 *						compression will have already been decoded at this
 *						point by the network layer.
 *	@param	posError	The amount of uncertainty in the position.
 *	@param	auxFiltered	If not NULL, a pointer to a float containing a new yaw.			
 */
void DumbFilter::input(	double time,
						SpaceID spaceID,
						EntityID vehicleID,
						const Position3D & pos,
						const Vector3 & posError,
						float * auxFiltered )
{
	BW_GUARD;
	if ( time > time_ )
	{
		time_ = time;

		spaceID_ = spaceID;
		vehicleID_ = vehicleID;
		pos_ = pos;

		if (auxFiltered != NULL)
		{
			yaw_ = auxFiltered[0];
		}
	}
}


/**
 *	This method updates the slave entity's position, velocity and yaw to match
 *	that stored from the last input.
 *
 *	@param	time	The client game time in seconds that the entity's volatile
 *					members should be updated for.
 */
void DumbFilter::output( double time )
{
	BW_GUARD;
	static DogWatch dwDumbFilterOutput("DumbFilter");
	dwDumbFilterOutput.start();

	FilterUtilityFunctions::coordinateSystemCheck( entity_, spaceID_, vehicleID_ );

	float yaw = yaw_;

	// make sure it's above the ground if it's a bot
	if (pos_[1] < -12000.f)
	{
		pos_[1] = (entity_->position()[1] < -12000.f) ?
			90.f :
			entity_->position()[1] + 1.f;

		Vector3 rpos_;
		if (FilterUtilityFunctions::filterDropPoint( &*entity_->pSpace(), Vector3( pos_ ), rpos_ ))
		{
			pos_[1] = rpos_[1];
		}
		else
		{
			float terrainHeight =
				Terrain::BaseTerrainBlock::getHeight( pos_[0], pos_[2] );

			if (terrainHeight != Terrain::BaseTerrainBlock::NO_TERRAIN)
			{
				pos_[1] = terrainHeight;
			}
			else
			{
				WARNING_MSG( "DumbFilter::output: Could not get terrain\n" );
			}
		}
	}

	entity_->pos( pos_, &yaw, 1 );

	dwDumbFilterOutput.stop();
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
 *	@param	auxFiltered	An pointer to a float into which the yaw of the last
 *						input will be placed.
 *
 *	@return				Returns true if an input existed and the values in the parameters
 *						were set.
 */
bool DumbFilter::getLastInput(	double & time,
								SpaceID & spaceID,
								EntityID & vehicleID,
								Position3D & pos,
								Vector3 & posError,
								float * auxFiltered )
{
	BW_GUARD;
	if (time_ != -1000.0)
	{
		time = time_;
		spaceID = spaceID_;
		vehicleID = vehicleID_;
		pos = pos_;
		if (auxFiltered != NULL)
		{
			auxFiltered[0] = yaw_;
			auxFiltered[1] = 0.f;
			auxFiltered[2] = 0.f;
		}
		return true;
	}
	return false;
}


/**
 *	Python factory method
 */
PyObject * DumbFilter::pyNew( PyObject * args )
{
	BW_GUARD;
	if (PyTuple_Size( args ) != 0)
	{
		PyErr_SetString( PyExc_TypeError,
			"BigWorld.DumbFilter() expects no arguments" );
		return NULL;
	}

	return new DumbFilter();
}


// dumb_filter.cpp
