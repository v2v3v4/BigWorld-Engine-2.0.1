/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NAVIGATOR_FIND_RESULT_HPP
#define NAVIGATOR_FIND_RESULT_HPP

#include "cstdmf/smartpointer.hpp"

class ChunkWaypointSet;
typedef SmartPointer< ChunkWaypointSet > ChunkWaypointSetPtr;

/**
 *	This class is used to return the results of a ChunkNavigator::find operation.
 */
class NavigatorFindResult
{
public:
	NavigatorFindResult() :
		pSet_( NULL ),
		waypoint_( -1 ),
		exactMatch_( false )
	{ }

	ChunkWaypointSetPtr pSet()
		{ return pSet_; }

	void pSet( ChunkWaypointSetPtr pSet )
		{ pSet_ = pSet; }

	int waypoint() const
		{ return waypoint_; }

	void waypoint( int waypoint )
		{ waypoint_ = waypoint; }

	bool exactMatch() const
		{ return exactMatch_; }

	void exactMatch( bool exactMatch )
		{ exactMatch_ = exactMatch; }

private:
	ChunkWaypointSetPtr	pSet_;
	int					waypoint_;
	bool				exactMatch_;
};

#endif // NAVIGATOR_FIND_RESULT_HPP
