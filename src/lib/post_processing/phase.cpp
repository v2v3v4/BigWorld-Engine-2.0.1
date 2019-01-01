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
#include "phase.hpp"

#ifndef CODE_INLINE
#include "phase.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "PostProcessing", 0 )

namespace PostProcessing
{
// Python statics
PY_TYPEOBJECT( Phase )

PY_BEGIN_METHODS( Phase )	
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( Phase )	
PY_END_ATTRIBUTES()


Phase::Phase( PyTypePlus *pType ):
	PyObjectPlus( pType )
{
}


Phase::~Phase()
{
}


PyObject * Phase::pyGetAttribute( const char *attr )
{
	PY_GETATTR_STD();
	return PyObjectPlus::pyGetAttribute(attr);
}


int Phase::pySetAttribute( const char *attr, PyObject *value )
{
	PY_SETATTR_STD();
	return PyObjectPlus::pySetAttribute(attr,value);
}


} //namespace PostProcessing


PY_SCRIPT_CONVERTERS( PostProcessing::Phase )
