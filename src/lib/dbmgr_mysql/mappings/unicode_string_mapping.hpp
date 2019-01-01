/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_UNICODE_STRING_MAPPING_HPP
#define MYSQL_UNICODE_STRING_MAPPING_HPP

#include "string_like_mapping.hpp"

/**
 *	This class maps the UNICODE_STRING type to the database.
 *
 *	Unicode strings are stored as UTF-8 in the database for which MySQL
 *	requires 3 bytes being set aside for every encoded character.
 */
class UnicodeStringMapping : public StringLikeMapping
{
private:
	// For calculating UTF8 character start offsets.
	typedef std::vector< uint > Offsets;

public:
	UnicodeStringMapping( const Namer & namer, const std::string & propName,
			bool isNameIndex, uint charLength,
			DataSectionPtr pDefaultValue );

	static void getUTF8CharOffsets( const std::string & s, Offsets & offsets );

	virtual bool isBinary() const	{ return false; }
	virtual enum_field_types getColumnType() const;
};

#endif // MYSQL_UNICODE_STRING_MAPPING_HPP
