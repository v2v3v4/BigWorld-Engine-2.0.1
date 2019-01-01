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

#include "weather.hpp"

#include "cstdmf/stdmf.hpp"
#include "cstdmf/watcher.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "math/vector2.hpp"

#include <string>

// ----------------------------------------------------------------------------
// Section: Weather
// ----------------------------------------------------------------------------

PY_TYPEOBJECT( Weather );

PY_BEGIN_METHODS( Weather )	
	/*~ function Weather.windAverage
	 *	@components{ client, tools }
	 *
	 *	This function sets the average velocity of the wind, specified as
	 *	x and z coordinates.  The wind on average blows at this velocity
	 *	but deviates around it by a random amount which is capped at 
	 *	windGustiness.
	 *
	 *	The wind determines the direction and speed that clouds move across
	 *	the sky.  This is important, because when weather systems are changed
	 *	the changes appear 1000m upwind of the user, and are blown across the
	 *	sky at a rate specified by the wind velocity.
	 *
	 *	If the wind is changed, then existing clouds continue with their existing
	 *	velocities, and new clouds are produced with the new velocity, so that the
	 *	weather gradually switches to blow from a different direction.
	 *
	 *	The wind velocity is also used to perturb the detail objects, allowing
	 *	for swaying grass and the like.
	 *
	 *	@param	xWind	a float.  The x component of the winds average velocity.
	 *	@param	zWind	a float.  The z component of the winds average velocity.
	 *	
	 */
	PY_METHOD( windAverage )
	/*~ function Weather.windGustiness
	 *	@components{ client, tools }
	 *
	 *	This function specifies the gustiness of the wind.  This is the
	 *	largest magnitude by which the actual wind speed can vary from that
	 *	specified by windAverage, as it varies randomly.
	 *
	 *	The wind determines the direction and speed that clouds move across
	 *	the sky.  This is important, because when weather systems are changed
	 *	the changes appear 1000m upwind of the user, and are blown across the
	 *	sky at a rate specified by the wind velocity.
	 *
	 *	The wind velocity is also used to perturb the detail objects, allowing
	 *	for swaying grass and the like.
	 *
	 *	@param gust	a float.  The cap on the random speed of the wind.
	 */
	PY_METHOD( windGustiness )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( Weather )
PY_END_ATTRIBUTES()

/// Constructor
Weather::Weather() :
	PyObjectPlus( &Weather::s_type_ ),
	wind_(0.f, 0.f)
{	
	windVelX_ = 0.f;
	windVelY_ = 0.f;
	windGustiness_ = 0.f;
}


void Weather::activate()
{
	MF_WATCH( "Client Settings/Weather/windVelX",
		windVelX_,
		Watcher::WT_READ_WRITE,
		"Wind velocity on the X axis" );

	MF_WATCH( "Client Settings/Weather/windVelY",
		windVelY_,
		Watcher::WT_READ_WRITE,
		"Wind velocity on the Z axis" );

	MF_WATCH( "Client Settings/Weather/windGustiness",
		windGustiness_,
		Watcher::WT_READ_WRITE,
		"Wind gustiness" );

	MF_WATCH( "Client Settings/Weather/out: wind", wind_,
		Watcher::WT_READ_ONLY, "Current wind velocity." );	

}


void Weather::deactivate()
{
#if ENABLE_WATCHERS
	Watcher::rootWatcher().removeChild( "Client Settings/Weather/windVelX" );
	Watcher::rootWatcher().removeChild( "Client Settings/Weather/windVelY" );
	Watcher::rootWatcher().removeChild( "Client Settings/Weather/windGustiness" );
	Watcher::rootWatcher().removeChild( "Client Settings/Weather/out: wind" );
#endif
}


/// Destructor
Weather::~Weather()
{
}


/**
 *	This method calculates the new weather for this frame.
 */
void Weather::tick( float dTime )
{
	Vector2 windLast( wind_ );

	// overwrite the wind with our calculation
	Vector2 windWant = Vector2( windVelX_, windVelY_ ) + Vector2(
		2.f*rand()*windGustiness_/RAND_MAX - windGustiness_,
		2.f*rand()*windGustiness_/RAND_MAX - windGustiness_ );
	wind_ = windLast + (windWant - windLast) * (1.f * dTime);
	
}


/**
 *	This method allows scripts to get various properties.
 */
PyObject * Weather::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();
	return PyObjectPlus::pyGetAttribute( attr );
}

/**
 *	This method allows scripts to set various properties.
 */
int Weather::pySetAttribute( const char * attr, PyObject * value )
{
	PY_SETATTR_STD();
	return PyObjectPlus::pySetAttribute( attr, value );
}


/**
 *	Sets new wind velocity.
 */	
PyObject * Weather::py_windAverage( PyObject* args )
{
	float xv, yv;

	if(!PyArg_ParseTuple( args, "ff", &xv, &yv ))
	{
		// PyArg_ParseTuple sets the exception.
		return NULL;
	}

	this->windAverage(xv, yv);
	Py_Return;
}

/**
 *	Sets new wind gustiness.
 */	
PyObject * Weather::py_windGustiness( PyObject* args )
{
	float amount;

	if(!PyArg_ParseTuple( args, "f", &amount ))
	{
		// PyArg_ParseTuple sets the exception.
		return NULL;
	}

	this->windGustiness(amount);
	Py_Return;
}


/*~ function BigWorld.weather
 *	@components{ client, tools }
 *
 *	@param spaceID [optional] The id of the space to retrieve the weather object from,
 *	or the current camera space if no argument is passed in.
 *	@return	the unique Weather object.
 *
 *	This function returns the unique Weather object which is used to control
 *	the weather on the client.
 */
/**
 *	Returns the weather object.
 */
static PyObject * weather( SpaceID spaceID = NULL_SPACE )
{
	BW_GUARD;

	ChunkSpacePtr pSpace;
	if ( spaceID == NULL_SPACE )
	{
		pSpace = ChunkManager::instance().cameraSpace();
		if ( !pSpace )
		{
			PyErr_Format( PyExc_ValueError, "BigWorld.weather(): "
				"No current camera space" );
			return NULL;
		}
	}
	else
	{
		pSpace = ChunkManager::instance().space( spaceID, false );
		if ( !pSpace )
		{
			PyErr_Format( PyExc_ValueError, "BigWorld.weather(): "
				"No space ID %d", spaceID );
			return NULL;
		}
	}

	Weather * pWeather = pSpace->enviro().weather();
	if (pWeather != NULL)
	{
		Py_INCREF( pWeather );
		return pWeather;
	}
	else
	{
		PyErr_Format( PyExc_EnvironmentError, "BigWorld.weather(): "
			"No weather object in space ID %d", spaceID );
		return NULL;
	}
}
PY_AUTO_MODULE_FUNCTION( RETOWN, weather, OPTARG( SpaceID, NULL_SPACE, END ), BigWorld )

// weather.cpp
