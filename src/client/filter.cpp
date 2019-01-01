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

#include "filter.hpp"
#include "filter_utility_functions.hpp"

#include "app.hpp"


DECLARE_DEBUG_COMPONENT2( "Entity", 0 )


/**
 *	This static member stores the active state of the filter system. When the
 *	system if inactive (false) all filters behave like the DumbFilter,
 *	returning there most recently received input.
 */
bool Filter::isActive_ = true;


PY_TYPEOBJECT( Filter )

PY_BEGIN_METHODS( Filter )
	PY_METHOD( reset )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( Filter )
PY_END_ATTRIBUTES()

PY_SCRIPT_CONVERTERS( Filter )


/**
 *	Constructor
 *
 *	@param	pType	The python object defining the type of the filter.
 */
Filter::Filter( PyTypePlus * pType ) :
	PyObjectPlus( pType ),
	entity_( NULL )
{
}


/**
 *	Destructor
 */
Filter::~Filter()
{
}


void Filter::input(	double time,
					SpaceID spaceID,
					EntityID vehicleID,
					const Position3D & pos,
					const Vector3 & posError,
					float * auxFiltered )
{
}


/*~ function Filter.reset
 *
 *	This function resets the current time for the filter to the specified
 *	time.
 *
 *	Filters use the current time to interpolate forwards the last update of
 *	the volatile data from the server.  Setting this time to a different
 *	value will move all entities to the place the filter thinks they should
 *	be at that specified time.
 *
 *	As a general rule this shouldn't need to be called.
 *
 *	@param	time	a Float the time to set the filter to.
 */
/**
 *	This method resets this filter to the input time.
 */
void Filter::reset( double time )
{
}


/**
 *	This method connects this filter to the given entity.
 *
 *	Note that no references are taken or else a circular reference
 *	would result - the filter is 'owned' by the given entity.
 */
void Filter::owner( Entity * pEntity )
{
	entity_ = pEntity;
}


/**
 *	Standard get attribute method.
 */
PyObject * Filter::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	Standard set attribute method.
 */
int Filter::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}


/**
 *	Static helper method to get the current game time.
 *
 *	@return	The time in seconds since the client was started.
 */
double Filter::getTimeNow()
{
	BW_GUARD;
	return App::instance().getTime();
}


/**
 *	Queries if the filter system is 'active'. In the case that it is not, all
 *	filters should behave like the DumbFilter and simply return their most
 *	recently received input.
 *	This is part of the filter demonstration accessed using DEBUG-H in
 *	fantasydemo.
 *
 *	@return	Returns true if the filter system is currently active.
 */
bool Filter::isActive()
{
	return isActive_;
}


/**
 *	This function sets the filter system to active or inactive. When filters
 *	are 'inactive' all filters behave like the DumbFilter and simply return
 *	their most recently received input.
 *	This is part of a demonstration of the filters filter accessed using
 *	DEBUG-H in fantasydemo.
 *
 *	@param	value	The new state of the filter system. 'true' == active
 */
void Filter::isActive( bool value )
{
	isActive_ = value;
}


// filter.cpp
