/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_WAYPOINT_GENERATOR_HPP
#define CHUNK_WAYPOINT_GENERATOR_HPP

#include "chunk/chunk.hpp"
#include "chunk_flooder.hpp"
#include "girth.hpp"
#include "waypoint_generator/waypoint_generator.hpp"

#include <string>

/**
 *	Helper class to generate waypoints for a chunk
 */
class ChunkWaypointGenerator
{
public:
	ChunkWaypointGenerator( Chunk * pChunk, const std::string& floodResultPath );
	virtual ~ChunkWaypointGenerator();
	bool modified()	const	{ return modified_; }
	bool ready() const;

	int maxFloodPoints() const;
	void flood( bool (*progressCallback)( int npoints ), girth gSpec, bool writeTGAs );
	void generate( bool annotate, girth gSpec );
	void output( float girth, bool firstGirth );
	void outputDirtyFlag(bool dirty = false);
    Chunk *chunk() const { return pChunk_; }

    static bool canProcess(Chunk *chunk);

private:
	Chunk * pChunk_;

	ChunkFlooder		flooder_;
	WaypointGenerator	gener_;

	bool				modified_;
};

#endif
