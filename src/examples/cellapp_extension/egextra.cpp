/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "egextra.hpp"
#include "egcontroller.hpp"

DECLARE_DEBUG_COMPONENT( 0 )

PY_TYPEOBJECT( EgExtra )

PY_BEGIN_METHODS( EgExtra )
	PY_METHOD( helloWorld )
	PY_METHOD( addEgController )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( EgExtra )
PY_END_ATTRIBUTES()

const EgExtra::Instance< EgExtra >
		EgExtra::instance( &EgExtra::s_attributes_.di_ );

EgExtra::EgExtra( Entity& e ) : EntityExtra( e )
{
}

EgExtra::~EgExtra()
{
}

PyObject * EgExtra::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();
	return this->EntityExtra::pyGetAttribute( attr );
}

int EgExtra::pySetAttribute( const char * attr, PyObject * value )
{
	PY_SETATTR_STD();
	return this->EntityExtra::pySetAttribute( attr, value );
}

void EgExtra::helloWorld()
{
	DEBUG_MSG( "egextra: hello world\n" );
}

PyObject * EgExtra::addEgController( int userArg )
{
	ControllerPtr pController = new EgController();
	ControllerID controllerID = entity_.addController( pController, userArg );
	return Script::getData( controllerID );
}

// egextra.cpp
