/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FACE_ENTITY_CONTROLLER_HPP
#define FACE_ENTITY_CONTROLLER_HPP

#include "controller.hpp"
#include "network/basictypes.hpp"
#include "updatable.hpp"

typedef SmartPointer< Entity > EntityPtr;

/**
 *  This class is the controller for an entity that is turning at a specific
 *  rate of angle/second.
 */
class FaceEntityController : public Controller, public Updatable
{
	DECLARE_CONTROLLER_TYPE( FaceEntityController )

public:
	FaceEntityController( int entityId = 0,
		float velocity = 2*MATH_PI, // in radians per second
		int period = 10 );

	// Overrides from Controller
	virtual void		startReal( bool isInitialStart );
	virtual void		stopReal( bool isFinalStop );

	void				writeRealToStream( BinaryOStream & stream );
	bool 				readRealFromStream( BinaryIStream & stream );

	void				update();

protected:
	EntityPtr  pTargetEntity_;
	int		targetEntityId_;
	float	radiansPerTick_;
	int		ticksToNextUpdate_;
	int		period_;
};

#endif // FACE_ENTITY_CONTROLLER_HPP
