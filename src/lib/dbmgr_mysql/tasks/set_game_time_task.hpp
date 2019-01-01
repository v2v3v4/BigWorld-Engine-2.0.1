/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SET_GAME_TIME_TASK_HPP
#define SET_GAME_TIME_TASK_HPP

#include "background_task.hpp"
#include "network/basictypes.hpp"

class SetGameTimeTask : public MySqlBackgroundTask
{
public:
	SetGameTimeTask( GameTime gameTime );

	virtual void performBackgroundTask( MySql & conn );
	virtual void performMainThreadTask( bool succeeded );

private:
	GameTime gameTime_;
};

#endif // SET_GAME_TIME_TASK_HPP
