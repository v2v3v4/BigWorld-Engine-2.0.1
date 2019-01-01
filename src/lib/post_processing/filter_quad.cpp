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
#include "filter_quad.hpp"

#ifndef CODE_INLINE
#include "filter_quad.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "PostProcessing", 0 )

namespace PostProcessing
{

// Python statics
PY_TYPEOBJECT( FilterQuad )

PY_BEGIN_METHODS( FilterQuad )	
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( FilterQuad )	
PY_END_ATTRIBUTES()


FilterQuad::FilterQuad( PyTypePlus *pType ):
	PyObjectPlus( pType )
{
}


FilterQuad::~FilterQuad()
{
}


void FilterQuad::draw()
{
}


PyObject * FilterQuad::pyGetAttribute( const char *attr )
{
	PY_GETATTR_STD();
	return PyObjectPlus::pyGetAttribute(attr);
}


int FilterQuad::pySetAttribute( const char *attr, PyObject *value )
{
	PY_SETATTR_STD();
	return PyObjectPlus::pySetAttribute(attr,value);
}

}	//namespace PostProcessing

PY_SCRIPT_CONVERTERS( PostProcessing::FilterQuad )
