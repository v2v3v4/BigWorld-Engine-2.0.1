/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_TRANSACTION_HPP
#define MYSQL_TRANSACTION_HPP

class MySql;

/**
 *	This class handles a MySQL transaction. The transaction will be rolled back
 *	if commit() has not be called when an object of this type leaves scope.
 */
class MySqlTransaction
{
public:
	MySqlTransaction( MySql & sql );
	// MySqlTransaction( MySql & sql, int & errorNum );
	~MySqlTransaction();

	bool shouldRetry() const;

	void commit();

private:
	MySqlTransaction( const MySqlTransaction& );
	void operator=( const MySqlTransaction& );

	MySql & sql_;
	bool committed_;
};

#endif // MYSQL_TRANSACTION_HPP
