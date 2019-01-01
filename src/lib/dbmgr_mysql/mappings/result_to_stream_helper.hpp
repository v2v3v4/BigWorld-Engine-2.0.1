/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RESULT_TO_STREAM_HELPER_HPP
#define RESULT_TO_STREAM_HELPER_HPP

#include "network/basictypes.hpp"

class MySql;

/**
 *	This class is used as a helper when running
 *	PropertyMapping::fromStreamToDatabase.
 */
class ResultToStreamHelper
{
public:
	ResultToStreamHelper( MySql & connection, DatabaseID parentID = 0 ) :
		connection_( connection ),
		parentID_( parentID )
	{
	}

	MySql & connection()				{ return connection_; }

	DatabaseID parentID() const			{ return parentID_; }
	void parentID( DatabaseID id )		{ parentID_ = id; }

private:
	MySql & connection_;
	DatabaseID parentID_;
};

#endif // RESULT_TO_STREAM_HELPER_HPP
