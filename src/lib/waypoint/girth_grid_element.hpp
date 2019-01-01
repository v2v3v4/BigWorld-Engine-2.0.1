/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GIRTH_GRID_ELEMENT_HPP
#define GIRTH_GRID_ELEMENT_HPP

class ChunkWaypointSet;

class GirthGridElement
{
public:
	GirthGridElement( ChunkWaypointSet * pSet, int waypoint ) :
		pSet_( pSet ), 
		waypoint_( waypoint )
	{}

	ChunkWaypointSet * pSet() const
		{ return pSet_; }

	int waypoint() const 
		{ return waypoint_; }


private:
	ChunkWaypointSet	* pSet_;	// dumb pointer
	int					waypoint_;

};

#endif // GIRTH_GRID_ELEMENT_HPP
