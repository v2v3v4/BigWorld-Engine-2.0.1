/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STRING_MAPPING_HPP
#define STRING_MAPPING_HPP

#include "string_like_mapping.hpp"

/**
 *	This class maps the STRING type to the database.
 */
class StringMapping : public StringLikeMapping
{
public:
	/**
	 *	Constructor.
	 */
	StringMapping( const Namer & namer, const std::string & propName,
			bool isNameIndex, uint charLength, DataSectionPtr pDefaultValue );

	virtual bool isBinary() const	{ return true; }
	virtual enum_field_types getColumnType() const;
};

#endif // STRING_MAPPING_HPP
