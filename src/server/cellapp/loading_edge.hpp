/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOADING_EDGE_HPP
#define LOADING_EDGE_HPP

#include "loading_column.hpp"
#include "math/rectt.hpp"

/**
 *	This class stores information about how much is loaded along one
 *	edge of the loading rectangle. It expands along the edge of the
 *	rectangle in both positive and negative directions.
 */
class LoadingEdge
{
public:
	void init( int edgeType, int edgePos, int chunkPos, int destPos )
	{
		edgeType_ = edgeType;
		currPos_ = edgePos;
		destPos_ = destPos;

		high_.pos_ = chunkPos;
		low_.pos_ = chunkPos - 1;
	}

	bool isFullyLoaded() const
	{
		return destPos_ == currPos_;
	}

	bool tick( ServerGeometryMapping * pMapping,
			const LoadingEdge & lineLo, const LoadingEdge & lineHi,
			const LoadingEdge & lineOth,
			bool & rAnyColumnsLoaded );

	// Is it the left or bottom edge?
	bool isLow() const	{ return edgeType_ < 2; }

	// Is it the left or right edge?
	bool isVertical() const	{ return !(edgeType_ & 1); }

	bool isLoading() const
	{
		return low_.isLoading() || high_.isLoading();
	}

	bool clipDestTo( int pos, int antiHyst );

	void prepareNewlyLoadedChunksForDelete( ServerGeometryMapping * pMapping )
	{
		low_.prepareNewlyLoadedChunksForDelete( pMapping );
		high_.prepareNewlyLoadedChunksForDelete( pMapping );
	}

	int currPos() const	{ return currPos_; }
	int destPos() const	{ return destPos_; }

private:
	int	currPos_;		// where does the done rectangle stop
	int destPos_;		// what is the limit of the destination rect

	LoadingColumn low_; // Loading strip in negative direction
	LoadingColumn high_; // Loading strip in positive direction

	int edgeType_;
};

#endif // LOADING_EDGE_HPP
