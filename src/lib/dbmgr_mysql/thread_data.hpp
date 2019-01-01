/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_THREAD_DATA_HPP
#define MYSQL_THREAD_DATA_HPP

#include "connection_info.hpp"
#include "wrapper.hpp"
#include "cstdmf/bgtask_manager.hpp"


/**
 *	This class is used to store data assoicated with background threads.
 */
class MySqlThreadData : public BackgroundThreadData
{
public:
	MySqlThreadData( const DBConfig::ConnectionInfo & connInfo );

	virtual void onStart( BackgroundTaskThread & thread );
	virtual void onEnd( BackgroundTaskThread & thread );

	MySql & connection()		{ return *pConnection_; }

	bool reconnect();

private:
	MySql * pConnection_;
	DBConfig::ConnectionInfo connectionInfo_;
};

#endif // MYSQL_THREAD_DATA_HPP
