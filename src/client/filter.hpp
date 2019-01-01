/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FILTER_HPP
#define FILTER_HPP


#include "math/vector3.hpp"
#include "network/basictypes.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"


class Entity;
class Filter;

typedef SmartPointer<Filter> FilterPtr;

/*~ class BigWorld.Filter
 *
 *	This is the abstract base class for all Filter objects.  Filters are used
 *	to interpolate volatile data of entities (the position and the orientation) 
 *	between server updates of these variables.
 *
 *	The only functionality defined in the base class is the reset function,
 *	which resets the filter to have its current time set to the specified time.
 *
 *	Every entity has a filter attribute.  Assigning a filter to that attribute
 *	sets that entity to be the owner of the specified filter.  This will make
 *	the filter process server updates for that entity.
 */
/**
 *	This class forms the bass of all filters in BigWorld.\n
 *	Filters are components attached to entities to process updates to the
 *	entity's 'volatile'  members. These include position, yaw, pitch and roll.
 *	Volatile members are different to other entity members in that their
 *	transmission to clients is 'best effort' as apposed to the 'reliable'
 *	fashion most members are updated. In addition to this the frequency of
 *	updates is affected with distance to the player. It is the responsibility
 *	of filters to take these inputs and produce a visually pleasing movement of
 *	the entity.

 *	@note:	The network layer will sometimes provided positions that are
 *	'on ground' and will need to have their \f$y\f$ coordinate sampled from the
 *	terrain at the given \f$(x,z)\f$. They will appear as positions with a
 *	\f$y\f$ component approximately equal to -13000.
 */
class Filter : public PyObjectPlus
{
	Py_Header( Filter, PyObjectPlus )

public:
	Filter( PyTypePlus * pType );
	~Filter();

	virtual void reset( double time );

	// Each class knows how much and what auxFiltered data there is.
	virtual void input( double time,
						SpaceID spaceID,
						EntityID vehicleID,
						const Position3D & pos,
						const Vector3 & posError,
						float * auxFiltered );

	virtual void output( double time ) = 0;

	virtual void owner( Entity * pEntity );

	virtual bool getLastInput(	double & time,
								SpaceID & spaceID,
								EntityID & vehicleID,
								Position3D & pos,
								Vector3 & posError,
								float * auxFiltered ) = 0;


	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );


	static bool isActive();
	static void isActive( bool value );


	PY_AUTO_METHOD_DECLARE( RETVOID, reset, OPTARG( double, getTimeNow(), END ) );

protected:
	/**
	 *	This member holds a week reference to the filter's owner entity.
	 */
	Entity *	entity_;

	static double getTimeNow();

private:
	static bool	isActive_;
};


PY_SCRIPT_CONVERTERS_DECLARE( Filter );




#endif // FILTER_HPP

// filter.hpp
