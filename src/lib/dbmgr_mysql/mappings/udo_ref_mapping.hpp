/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_UDO_REF_MAPPING_HPP
#define MYSQL_UDO_REF_MAPPING_HPP

#include "unique_id_mapping.hpp"

/**
 *	This class maps a UDO_REF property into the database
 */
class UDORefMapping : public UniqueIDMapping
{
public:
	UDORefMapping( const Namer & namer, const std::string & propName,
			DataSectionPtr pDefaultValue );

private:
	static DataSectionPtr getGuidSection( DataSectionPtr pParentSection );
};

#endif // MYSQL_UDO_REF_MAPPING_HPP
