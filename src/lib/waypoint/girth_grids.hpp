/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GIRTH_GRIDS_HPP
#define GIRTH_GRIDS_HPP

#include <vector>

class GirthGridList;

struct GirthGrid
{
	float				girth;
	GirthGridList * 	grid;
};

typedef std::vector< GirthGrid > GirthGrids;

#endif // GIRTH_GRIDS
