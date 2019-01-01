/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STRING_OFFSET_HPP
#define STRING_OFFSET_HPP

#include <vector>

class StringOffset;
typedef std::vector< StringOffset > StringOffsetList;

#include "cstdmf/stdmf.hpp"

#pragma pack( push, 1 )
class StringOffset
{
public:
	StringOffset() {}
	StringOffset( int start, int end ) : start_( start ), end_( end ) {}

	uint16 start_;
	uint16 end_;
};
#pragma pack( pop )

#endif // STRING_OFFSET_HPP
