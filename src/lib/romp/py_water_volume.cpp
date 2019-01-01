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
#include "py_water_volume.hpp"

// -----------------------------------------------------------------------------
// Section: Python Interface to the PyWaterVolume.
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyWaterVolume )

/*~ function BigWorld.PyWaterVolume
 *	@components{ client, tools }
 *
 *	Factory function to create and return a new PyWaterVolume object.
 *	@return A new PyWaterVolume object.
 */
PY_FACTORY_NAMED( PyWaterVolume, "WaterVolume", BigWorld )

PY_BEGIN_METHODS( PyWaterVolume )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyWaterVolume )
	/*~ attribute PyWaterVolume.surfaceHeight
	 *	@components{ client, tools }
	 *	The surfaceHeight attribute returns the world height of the level
	 *	surface of water.
	 *	@type Float.
	 */
	PY_ATTRIBUTE( surfaceHeight )
PY_END_ATTRIBUTES()


/**
 *	Constructor.
 */
PyWaterVolume::PyWaterVolume( Water* pWater, PyTypePlus *pType ):
	PyObjectPlus( pType ),
	pWater_( pWater )
{
}


/**
 *	This method returns the world y-value of the water surface.
 *
 *	@return float	The world y-value of the water surface.
 */
float PyWaterVolume::surfaceHeight() const
{
	return pWater_->position()[1];
}


/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the readable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 */
PyObject *PyWaterVolume::pyGetAttribute( const char *attr )
{
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the writable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 *	@param value	The value to assign to the attribute.
 */
int PyWaterVolume::pySetAttribute( const char *attr, PyObject *value )
{
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}


/**
 *	This is a static Python factory method. This is declared through the
 *	factory declaration in the class definition.
 *
 *	@param args		The list of parameters passed from Python. This should
 *					be a vector3 in python.
 */
PyObject *PyWaterVolume::pyNew( PyObject *args )
{
	PyErr_SetString( PyExc_TypeError, "BigWorld.PyWaterVolume: "
			"This constructor is private.  Instances of these objects are "
			"passed into the water volume listener callback functions" );
	return NULL;
}


PY_SCRIPT_CONVERTERS( PyWaterVolume )