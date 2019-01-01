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
#include "pyfashion.hpp"


// -----------------------------------------------------------------------------
// Section: PyFashion
// -----------------------------------------------------------------------------


PY_TYPEOBJECT( PyFashion )

PY_BEGIN_METHODS( PyFashion )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyFashion )
PY_END_ATTRIBUTES()



/**
 *	Default makeCopy method
 */
PyFashion * PyFashion::makeCopy( PyModel * pModel, const char * attrName )
{
	BW_GUARD;
	Py_INCREF( this );
	return this;
}


/**
 *	Python get attribute method
 */
PyObject * PyFashion::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();
	return this->PyObjectPlus::pyGetAttribute( attr );
}

/**
 *	Python set attribute method
 */
int PyFashion::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();
	return this->PyObjectPlus::pySetAttribute( attr, value );
}

// pyfashion.cpp
