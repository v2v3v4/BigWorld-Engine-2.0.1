/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CLEAN_CHUNK_LIST_HPP
#define CLEAN_CHUNK_LIST_HPP


#include <map>
#include "chunk/chunk_space.hpp"
#include "romp/progress.hpp"

/**
 *	This class manages the clean chunk list. In the list, it
 *	stores the time stamps for each chunk that is known to
 *	be clean. In case the chunk file gets modified outside WE,
 *	we will check the stored time stamp against its file time
 *	stamp to update the clean status.
 */
class CleanChunkList : public SafeReferenceCount
{
	CleanChunkList( CleanChunkList& );
	CleanChunkList& operator= ( CleanChunkList& );
public:
	CleanChunkList( GeometryMapping& dirMapping );

	void save() const;
	void sync( ProgressTask* task );
	void update( Chunk* pChunk );
	void markAsClean( const std::string& chunk );
	bool isClean( const std::string& chunk ) const;

private:
	void load();
	DataSectionPtr getDataSection() const;

	GeometryMapping& dirMapping_;

	struct ChunkInfo
	{
		uint64 cdataModificationTime_;
		uint64 cdataCreationTime_;
		uint64 chunkModificationTime_;
		uint64 chunkCreationTime_;
	};

	std::map<std::string, ChunkInfo> checkedChunks_;
	std::map<std::string, ChunkInfo> uncheckedChunks_;
};

typedef SmartPointer<CleanChunkList> CleanChunkListPtr;

#endif//CLEAN_CHUNK_LIST_HPP
