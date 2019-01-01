/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPACE_INFORMATION_HPP
#define SPACE_INFORMATION_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/project/grid_coord.hpp"


class SpaceInformation
{
public:
	SpaceInformation():
		spaceName_( "" ),
		localToWorld_( 0,0 ),
		gridWidth_( 0 ),
		gridHeight_( 0 )
	{
	}

	SpaceInformation( const std::string& spaceName, const GridCoord& localToWorld, uint16 width, uint16 height ):
		spaceName_( spaceName ),
		localToWorld_( localToWorld ),
		gridWidth_( width ),
		gridHeight_( height )
	{
	}

	bool operator != ( const SpaceInformation& other ) const
	{
		return (other.spaceName_ != this->spaceName_);
	}

	std::string                 spaceName_;
	GridCoord                   localToWorld_; // transform from origin at (0,0) to origin at middle of map
	uint16	                    gridWidth_;
	uint16	                    gridHeight_;
};


#endif // SPACE_INFORMATION_HPP
