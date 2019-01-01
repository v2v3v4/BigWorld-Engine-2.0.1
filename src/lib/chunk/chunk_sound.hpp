/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_SOUND_HPP
#define CHUNK_SOUND_HPP

#include "chunk/chunk_item.hpp"

class Base3DSound;

/**
 *	This chunk item is a sound tied to a location.
 */
class ChunkSound : public ChunkItem
{
	DECLARE_CHUNK_ITEM( ChunkSound )
public:
	ChunkSound();
	~ChunkSound();

	bool load( DataSectionPtr pSection, Chunk * pChunk, std::string* errorString = NULL );

	virtual void tick( float );

protected:
	Base3DSound *	pSound_;
};



#endif // CHUNK_SOUND_HPP
