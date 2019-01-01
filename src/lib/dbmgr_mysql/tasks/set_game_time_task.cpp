/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "set_game_time_task.hpp"

#include "../query.hpp"

// -----------------------------------------------------------------------------
// Section: SetGameTimeTask
// -----------------------------------------------------------------------------

SetGameTimeTask::SetGameTimeTask( GameTime gameTime ) :
	MySqlBackgroundTask( "SetGameTimeTask" ),
	gameTime_( gameTime )
{
}


/**
 *	This method updates the game time in the database.
 */
void SetGameTimeTask::performBackgroundTask( MySql & conn )
{
	static const Query query( "UPDATE bigworldGameTime SET time=?" );

	query.execute( conn, gameTime_, NULL );
}


void SetGameTimeTask::performMainThreadTask( bool succeeded )
{
}

// set_game_time_task.cpp
