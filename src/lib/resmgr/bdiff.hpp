/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BDIFF_HPP
#define BDIFF_HPP

#include <vector>

bool performDiff( const std::vector<unsigned char>& first, 
				  const std::vector<unsigned char>& second, 
				  FILE * diff );

#endif
