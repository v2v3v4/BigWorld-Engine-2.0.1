/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CELL_APP_INIT_DATA_HPP
#define CELL_APP_INIT_DATA_HPP

/**
 * Data to use when initialising a CellApp.
 */
struct CellAppInitData
{
	int32 id;			//!< ID of the new CellApp
	GameTime time;		//!< Current game time
	Mercury::Address baseAppAddr;	//!< Address of the BaseApp to talk to
	bool isReady;		//!< Flag indicating whether the server is ready
};

#endif // CELL_APP_INIT_DATA_HPP
