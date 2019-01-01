/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONSOLIDATE_DBS__DBMGR_STATUS_REPORTER_HPP
#define CONSOLIDATE_DBS__DBMGR_STATUS_REPORTER_HPP

#include <string>

class DBMgrStatusReporter
{
public:
	virtual void onStatus( const std::string & status ) = 0;
};

#endif // CONSOLIDATE_DBS__DBMGR_STATUS_REPORTER_HPP
