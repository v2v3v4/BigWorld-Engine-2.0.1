/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_FLOODER_HPP
#define CHUNK_FLOODER_HPP

#include "math/vector3.hpp"
#include "girth.hpp"

class Chunk;
class AdjacentChunkSet;
class WaypointFlood;
union AdjGridElt;
class PhysicsHandler;

/**
 *	This class flood fills a chunk with physics tests to make
 *	passable path pixmap, which is used by a waypoint generator.
 */
class ChunkFlooder
{
public:
	ChunkFlooder( Chunk * pChunk, const std::string& floodResultPath );
	~ChunkFlooder();

	bool flood(
		girth gSpec,
		bool (*progressCallback)( int npoints ) = NULL,
		int nshrink = 0,
        bool writeTGAs = true );

	Vector3	minBounds() const;
	Vector3	maxBounds() const;
	float	resolution() const;
	int		width() const;
	int		height() const;
	AdjGridElt **	adjGrids() const;
	float **		hgtGrids() const;

	const std::vector<Vector3> & entityPts() const	{ return entityPts_; }

private:
	ChunkFlooder( const ChunkFlooder& );
	ChunkFlooder& operator=( const ChunkFlooder& );

	void reset();

	void getSeedPoints( std::vector<Vector3> & pts, PhysicsHandler& ph );
	bool flashFlood( const Vector3 & seedPt );

	Chunk *					pChunk_;
	WaypointFlood *			pWF_;
	std::vector<Vector3>	entityPts_;

	std::string				floodResultPath_;
};


#endif // CHUNK_FLOODER_HPP
