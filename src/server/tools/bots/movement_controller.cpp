/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "Python.h"		// See http://docs.python.org/api/includes.html

#include "movement_controller.hpp"

#include "main_app.hpp"

DECLARE_DEBUG_COMPONENT2( "Bots", 0 );

// -----------------------------------------------------------------------------
// Section: MovementFactory
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
MovementFactory::MovementFactory( const char * name )
{
	MainApp::addFactory( name, *this );
}

// movement_controller.cpp
