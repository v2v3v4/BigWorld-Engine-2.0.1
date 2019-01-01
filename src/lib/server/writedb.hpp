/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVER_WRITEDB_HPP
#define SERVER_WRITEDB_HPP

// flags for a writeToDB operation
enum WriteDBFlags
{
	WRITE_LOG_OFF			= 1,
	WRITE_EXPLICIT			= 2,
	WRITE_BASE_DATA			= 4,
	WRITE_CELL_DATA			= 8,
	WRITE_BASE_CELL_DATA	= WRITE_BASE_DATA|WRITE_CELL_DATA,
	WRITE_DELETE_FROM_DB	= 16,
	WRITE_AUTO_LOAD_YES		= 32,
	WRITE_AUTO_LOAD_NO		= 64,
	WRITE_AUTO_LOAD_MASK	= WRITE_AUTO_LOAD_YES | WRITE_AUTO_LOAD_NO
};

#endif // SERVER_WRITEDB_HPP
