/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "speed_controller.hpp"
#include "speed_extra.hpp"

#include "cellapp/cellapp.hpp"

DECLARE_DEBUG_COMPONENT( 0 )

IMPLEMENT_CONTROLLER_TYPE( SpeedController,
		ControllerDomain( DOMAIN_GHOST | DOMAIN_REAL ) );

// -----------------------------------------------------------------------------
// Section: SpeedController
// -----------------------------------------------------------------------------

/**
 *	This method is an override from Controller. It is called when this
 *	controller is started on a real or ghost entity.
 */
void SpeedController::startGhost()
{
	// Create the SpeedExtra.
	SpeedExtra::instance( this->entity() );
}


/**
 *	This method is called when this controller is started on a real entity.
 */
void SpeedController::startReal( bool isInitialStart )
{
	SpeedExtra::instance( this->entity() ).setController( this );
}


/**
 *	This method is called when this controller is stopped on a real entity.
 */
void SpeedController::stopReal( bool isFinalStop )
{
	SpeedExtra::instance( this->entity() ).setController( NULL );
}


/**
 *	This method is called when this controller is stopped on a real or ghost
 *	entity.
 */
void SpeedController::stopGhost()
{
	// Destroy the SpeedExtra
	SpeedExtra::instance.clear( this->entity() );
}


/**
 *	This method streams the necessary state to create a ghost controller.
 */
void SpeedController::writeGhostToStream( BinaryOStream & stream )
{
	this->Controller::writeGhostToStream( stream );

	SpeedExtra::instance( this->entity() ).writeToStream( stream );
}


/**
 *	This method reads the ghost controller data from the input stream.
 */
bool SpeedController::readGhostFromStream( BinaryIStream & stream )
{
	this->Controller::readGhostFromStream( stream );

	SpeedExtra::instance( this->entity() ).readFromStream( stream );

	return true;
}

// speed_controller.cpp
