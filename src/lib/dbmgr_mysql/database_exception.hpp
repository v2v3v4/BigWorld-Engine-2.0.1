/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DATABASE_EXCEPTION_HPP
#define DATABASE_EXCEPTION_HPP

#include <string>
#include <mysql/mysql.h>

/**
 *	This class is an exception that can be thrown when a database query fails.
 */
class DatabaseException : public std::exception
{
public:
	DatabaseException( MYSQL * pConnection );
	~DatabaseException() throw();

	virtual const char * what() const throw() { return errStr_.c_str(); }

	bool shouldRetry() const;
	bool isLostConnection() const;

private:
	std::string errStr_;
	unsigned int errNum_;
};

#endif // DATABASE_EXCEPTION_HPP
