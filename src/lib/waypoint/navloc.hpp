/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NAVLOC_HPP
#define NAVLOC_HPP

#include "chunk_waypoint_set.hpp"

#include "cstdmf/smartpointer.hpp"
#include "cstdmf/stdmf.hpp"

#include "math/vector3.hpp"

class ChunkSpace;
class Chunk;
class ChunkWaypointSet;
typedef SmartPointer<ChunkWaypointSet> ChunkWaypointSetPtr;

/**
 *	This class is a location in the navigation mesh
 */
class NavLoc
{
public:
	NavLoc();

	NavLoc( ChunkSpace * pSpace, const Vector3 & point, float girth );

	NavLoc( Chunk * pChunk, const Vector3 & point, float girth );

	NavLoc( const NavLoc & guess, const Vector3 & point );

	~NavLoc();

	bool valid() const	
		{ return pSet_ && pSet_->chunk(); }

	ChunkWaypointSetPtr pSet() const			
		{ return pSet_; }

	int	waypoint() const	
		{ return waypoint_; }

	Vector3	point() const	
		{ return point_; }

	bool isWithinWP() const;

	void clip();

	void makeMaxHeight( Vector3 & point ) const;

	void clip( Vector3 & point ) const;

	std::string desc() const;

	bool waypointsEqual( const NavLoc & other ) const
		{ return pSet_ == other.pSet_ && waypoint_ == other.waypoint_; }

private:
	ChunkWaypointSetPtr	pSet_;
	int					waypoint_;
	Vector3				point_;

	friend class ChunkWaypointState;
	friend class Navigator;
};

#endif // NAVLOC_HPP
