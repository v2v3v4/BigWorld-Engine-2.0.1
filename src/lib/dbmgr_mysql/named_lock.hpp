/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_NAMEDLOCK_HPP
#define MYSQL_NAMEDLOCK_HPP

#include <string>

class MySql;

namespace MySQL
{
/**
 * 	This class obtains and releases an named lock.
 */
class NamedLock
{
public:
	NamedLock( const std::string & lockName = "" );
	~NamedLock();

	bool lock( MySql & connection );
	bool unlock();

	const std::string& lockName() const 	{ return lockName_; }
	bool isLocked() const 					{ return pConnection_ != NULL; }

private:
	MySql *		pConnection_;
	std::string lockName_;
};

}	// namespace MySQL

#endif // MYSQL_NAMEDLOCK_HPP
