/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PORTAL_OBSTACLE_HPP
#define PORTAL_OBSTACLE_HPP

#include "chunk_boundary.hpp"
#include "chunk_item.hpp"

// -----------------------------------------------------------------------------
// Section: PortalObstacle
// -----------------------------------------------------------------------------

/**
 *	This class is the obstacle that a ChunkPortal puts in the collision scene
 */
class PortalChunkItem : public ChunkItem
{
public:
	PortalChunkItem( ChunkBoundary::Portal * pPortal );

	virtual void toss( Chunk * pChunk );
	virtual void draw();

	ChunkBoundary::Portal * pPortal() const		{ return pPortal_; }

private:
	ChunkBoundary::Portal * pPortal_;
};


#endif // PORTAL_OBSTACLE_HPP
