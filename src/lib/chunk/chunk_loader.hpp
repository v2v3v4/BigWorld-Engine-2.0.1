/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_LOADER_HPP
#define CHUNK_LOADER_HPP

class Chunk;
class ChunkSpace;
class Vector3;


/**
 *	This class loads chunks using a background thread.
 */
class ChunkLoader
{
public:
	static void load( Chunk * pChunk, int priority = 0 );
	static void loadNow( Chunk * pChunk );
	static void findSeed( ChunkSpace * pSpace, const Vector3 & where,
		Chunk *& rpChunk );
};



#endif // CHUNK_LOADER_HPP
