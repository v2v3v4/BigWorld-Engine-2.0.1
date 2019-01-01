/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPEED_CONTROLLER_HPP
#define SPEED_CONTROLLER_HPP

#include "cellapp/controller.hpp"

/**
 *	This class is an example controller. It works with a SpeedExtra to calculate
 *	the speed of an entity. The controller's main responsibilities are creating
 *	and destroying the SpeedExtra and streaming the SpeedExtra with the real
 *	entity.
 */
class SpeedController : public Controller
{
	DECLARE_CONTROLLER_TYPE( SpeedController );

public:
	void startReal( bool isInitialStart );
	void stopReal( bool isFinalStop );

	void startGhost();
	void stopGhost();

	void writeGhostToStream( BinaryOStream & stream );
	bool readGhostFromStream( BinaryIStream & stream );

	// Controller::cancel is protected. It really shouldn't be.
	void stop()		{ this->cancel(); }

private:
};

#endif // SPEED_CONTROLLER_HPP
