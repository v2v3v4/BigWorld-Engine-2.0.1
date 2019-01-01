/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "string_mapping.hpp"

#include "resmgr/datasection.hpp"


/**
 *	Constructor.
 */
StringMapping::StringMapping( const Namer & namer,
		const std::string & propName,
		bool isNameIndex,
		uint charLength,
		DataSectionPtr pDefaultValue ) :
	StringLikeMapping( namer, propName, isNameIndex, charLength )
{
	if (pDefaultValue)
	{
		defaultValue_ = pDefaultValue->as<std::string>();

		if (defaultValue_.size() > charLength_)
		{
			defaultValue_.resize( charLength_ );

			WARNING_MSG( "StringMapping::StringMapping: "
					"Default value (non-UTF8) for property %s has been "
					"truncated to '%s'\n",
				propName.c_str(), defaultValue_.c_str() );
		}
	}
}


/*
 *	Override from StringLikeMapping.
 */
enum_field_types StringMapping::getColumnType() const
{
	// __kyl__ (24/7/2006) Special handling of STRING < 255 characters
	// because this is how we magically pass the size of the name index
	// field. If type is not VARCHAR then index size is assumed to be
	// 255 (see createEntityTableIndex()).
	return (charLength_ < 256) ?
		MYSQL_TYPE_VAR_STRING :
		this->StringLikeMapping::getColumnType();
}

// string_mapping.cpp
