/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "egcontroller.hpp"

#include "cellapp/cellapp.hpp"

DECLARE_DEBUG_COMPONENT( 0 )

IMPLEMENT_CONTROLLER_TYPE( EgController, DOMAIN_REAL );

void EgController::startReal( bool isInitialStart )
{
	CellApp::instance().registerForUpdate( this );
}

void EgController::stopReal( bool isFinalStop )
{
	MF_VERIFY( CellApp::instance().deregisterForUpdate( this ) );
}

void EgController::update()
{
	TRACE_MSG( "EgController::update\n" );
}

// egcontroller.cpp
