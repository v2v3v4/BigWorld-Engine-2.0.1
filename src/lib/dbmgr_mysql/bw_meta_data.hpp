/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_BW_META_DATA_HPP
#define MYSQL_BW_META_DATA_HPP

class MySql;

#include "network/basictypes.hpp"

/**
 *	This class is used to access tables that stores entity meta data.
 */
class BigWorldMetaData
{
public:
	BigWorldMetaData( MySql & connection );

	MySql & connection()		{ return connection_; }

	EntityTypeID getEntityTypeID( const std::string & entityName );
	void setEntityTypeID( const std::string & entityName, EntityTypeID typeID );
	void addEntityType( const std::string & entityName, EntityTypeID typeID );
	void removeEntityType( const std::string & entityName );

private:
	MySql &				connection_;
};

#endif // MYSQL_BW_META_DATA_HPP
