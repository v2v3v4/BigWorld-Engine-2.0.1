/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/



#ifndef DUMB_FILTER_HPP
#define DUMB_FILTER_HPP


#include "filter.hpp"


/*~ class BigWorld.DumbFilter
 *
 *	This subclass of the Filter class simply sets the position of the entity
 *	to the most recent position specified by the server, performing no 
 *	interpolation.  It ignores position updates which are older than the
 *	current position of the entity.
 *
 *	A new DumbFilter can be created using BigWorld.DumbFilter function.
 */
/**
 *	This is a dumb filter class. It uses only the first element of auxFiltered.
 *	It simply sets the position to the last input position.
 *	It does however ensure ordering on time, and doesn't call
 *	the Entity's 'pos' function unless necessary. This could
 *	almost be the Teleport filter.
 */
class DumbFilter : public Filter
{
	Py_Header( DumbFilter, Filter )

public:
	DumbFilter( PyTypePlus * pType = &s_type_ );

	void reset( double time );

	virtual void input(	double time,
						SpaceID spaceID,
						EntityID vehicleID,
						const Position3D & pos,
						const Vector3 & posError,
						float * auxFiltered );

	virtual void output( double time );

	virtual bool getLastInput(	double & time,
								SpaceID & spaceID,
								EntityID & vehicleID,
								Position3D & pos,
								Vector3 & posError,
								float * auxFiltered );

	PY_FACTORY_DECLARE()

private:
	// Doxygen comments for all members can be found in the .cpp
	double		time_;
	Position3D	pos_;

	float		yaw_;

	SpaceID		spaceID_;
	EntityID	vehicleID_;
};



#endif // DUMB_FILTER_HPP

// dumb_filter.hpp
