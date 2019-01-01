/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_TYPE_TRAITS_HPP
#define MYSQL_TYPE_TRAITS_HPP

#include "cstdmf/stdmf.hpp"

#include <mysql/mysql.h>

#include <string>

// This traits class is used for mapping C types to MySQL types.
template < class CTYPE >
struct MySqlTypeTraits
{
	// static const enum enum_field_types	colType = MYSQL_TYPE_LONG;
};

#define MYSQL_TYPE_TRAITS( FROM_TYPE, TO_TYPE ) 					\
template <>															\
struct MySqlTypeTraits< FROM_TYPE >									\
{																	\
	static const enum enum_field_types	colType = TO_TYPE;			\
};

MYSQL_TYPE_TRAITS( int8, MYSQL_TYPE_TINY )
MYSQL_TYPE_TRAITS( uint8, MYSQL_TYPE_TINY )
MYSQL_TYPE_TRAITS( int16, MYSQL_TYPE_SHORT )
MYSQL_TYPE_TRAITS( uint16, MYSQL_TYPE_SHORT )
MYSQL_TYPE_TRAITS( int32, MYSQL_TYPE_LONG )
MYSQL_TYPE_TRAITS( uint32, MYSQL_TYPE_LONG )
MYSQL_TYPE_TRAITS( int64, MYSQL_TYPE_LONGLONG )
MYSQL_TYPE_TRAITS( uint64, MYSQL_TYPE_LONGLONG )
MYSQL_TYPE_TRAITS( float, MYSQL_TYPE_FLOAT )
MYSQL_TYPE_TRAITS( double, MYSQL_TYPE_DOUBLE )

// Slightly dodgy specialisation for std::string. Basically it maps to
// different sized BLOBs. colType and colTypeStr are actually functions
// instead of constants.
template <>
struct MySqlTypeTraits< std::string >
{
	typedef MySqlTypeTraits< std::string > THIS;

	static const enum enum_field_types colType( uint maxColWidthBytes )
	{
		if (maxColWidthBytes < 1<<8)
		{
			return MYSQL_TYPE_TINY_BLOB;
		}
		else if (maxColWidthBytes < 1<<16)
		{
			return MYSQL_TYPE_BLOB;
		}
		else if (maxColWidthBytes < 1<<24)
		{
			return MYSQL_TYPE_MEDIUM_BLOB;
		}
		else
		{
			return MYSQL_TYPE_LONG_BLOB;
		}
	}

	static const std::string TINYBLOB;
	static const std::string BLOB;
	static const std::string MEDIUMBLOB;
	static const std::string LONGBLOB;

	static const std::string colTypeStr( uint maxColWidthBytes )
	{
		switch (THIS::colType( maxColWidthBytes ))
		{
			case MYSQL_TYPE_TINY_BLOB:
				return TINYBLOB;
			case MYSQL_TYPE_BLOB:
				return BLOB;
			case MYSQL_TYPE_MEDIUM_BLOB:
				return MEDIUMBLOB;
			case MYSQL_TYPE_LONG_BLOB:
				return LONGBLOB;
			default:
				break;
		}
		return NULL;
	}
};

#endif // MYSQL_TYPE_TRAITS_HPP
