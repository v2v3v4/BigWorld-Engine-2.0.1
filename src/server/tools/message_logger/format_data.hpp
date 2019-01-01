/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FORMAT_DATA_HPP
#define FORMAT_DATA_HPP

#include "cstdmf/stdmf.hpp"

#pragma pack( push, 1 )

class FormatData
{
public:
	FormatData() {}
	FormatData( char type, int cflags, int base,
		int min, int max, int flags, int vflags ) :
		type_( type ), cflags_( cflags ), vflags_( vflags ), base_( base ),
		min_( min ), max_( max ), flags_( flags ) {}

	char type_;
	unsigned cflags_:4;
	unsigned vflags_:4;
	uint8 base_;
	int min_;
	int max_;
	int flags_;
};
#pragma pack( pop )

#endif // FORMAT_DATA_HPP
