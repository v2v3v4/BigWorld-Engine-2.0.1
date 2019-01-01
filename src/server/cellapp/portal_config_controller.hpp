/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PORTAL_CONFIG_CONTROLLER_HPP
#define PORTAL_CONFIG_CONTROLLER_HPP

#include "controller.hpp"

#include "chunk/chunk_space.hpp"
#include "cstdmf/time_queue.hpp"

#include "physics2/worldtri.hpp"


/**
 *	This class controls the configuration of the portal over which its
 *	entity sits.
 */
class PortalConfigController : public Controller, public TimerHandler
{
	DECLARE_CONTROLLER_TYPE( PortalConfigController )

public:
	PortalConfigController( bool permissive = true,
			WorldTriangle::Flags collisionFlags = 0 );

	virtual void writeGhostToStream( BinaryOStream & stream );
	virtual bool readGhostFromStream( BinaryIStream & stream );

	virtual void startGhost();
	virtual void stopGhost();

private:
	void apply();
	bool attemptToApply( bool permissive, WorldTriangle::Flags collisionFlags );
	void startTimer();

	virtual void handleTimeout( TimerHandle handle, void * pUser );

	ChunkSpacePtr	pSpace_;
	Vector3			point_;

	bool						permissive_;
	WorldTriangle::Flags		collisionFlags_;

	bool			started_;

	TimerHandle		timerHandle_;
};

#endif // PORTAL_CONFIG_CONTROLLER_HPP
