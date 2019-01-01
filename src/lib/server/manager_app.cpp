/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "manager_app.hpp"

// -----------------------------------------------------------------------------
// Section: ManagerApp
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
ManagerApp::ManagerApp( Mercury::EventDispatcher & mainDispatcher,
			Mercury::NetworkInterface & interface ) :
	ServerApp( mainDispatcher, interface )
{
}

/**
 *	This method adds the watchers associated with this class.
 */
void ManagerApp::addWatchers( Watcher & watcher )
{
	ServerApp::addWatchers( watcher );
}

// manager_app.hpp
