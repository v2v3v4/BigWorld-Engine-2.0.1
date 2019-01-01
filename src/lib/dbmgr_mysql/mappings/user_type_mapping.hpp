/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_USER_TYPE_MAPPING_HPP
#define MYSQL_USER_TYPE_MAPPING_HPP

#include "composite_property_mapping.hpp"

class UserDataType;

/**
 *	This class maps USER_TYPE in MySQL. It's a CompositePropertyMapping with
 *	special handling for serialisation.
 */
class UserTypeMapping : public CompositePropertyMapping
{
public:
	UserTypeMapping( const std::string & propName );

	// Overrides from PropertyMappingPtr
	virtual void fromStreamToDatabase( StreamToQueryHelper & helper,
			BinaryIStream & strm,
			QueryRunner & queryRunner ) const;

	virtual void fromDatabaseToStream( ResultToStreamHelper & helper,
				ResultStream & results,
				BinaryOStream & strm ) const;

	static PropertyMappingPtr create( const Namer & namer,
			const std::string & propName, const UserDataType & type,
			DataSectionPtr pDefaultValue );
};

#endif // MYSQL_USER_TYPE_MAPPING_HPP
