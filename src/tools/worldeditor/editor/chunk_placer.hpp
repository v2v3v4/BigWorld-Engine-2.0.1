/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_PLACER_HPP
#define CHUNK_PLACER_HPP

#include "chunk/chunk.hpp"
#include "cstdmf/unique_id.hpp"
#include "pyscript/py_data_section.hpp"


/**
 *	The creation and deletion of chunks is handled with a pair of functions
 *	in the WorldEditor module, rather than with a functor.
 *	There are also utility methods to back these functions up.
 *
 *	@see ChunkItemPlacer
 */
class ChunkPlacer
{
	typedef ChunkPlacer This;

public:
	PY_MODULE_STATIC_METHOD_DECLARE( py_createChunk )
	PY_MODULE_STATIC_METHOD_DECLARE( py_recreateChunks )
	PY_MODULE_STATIC_METHOD_DECLARE( py_cloneAndAutoSnap )
//	PY_MODULE_STATIC_METHOD_DECLARE( py_deleteChunk )
	PY_MODULE_STATIC_METHOD_DECLARE( py_createInsideChunkName )
	PY_MODULE_STATIC_METHOD_DECLARE( py_chunkDataSection )
	PY_MODULE_STATIC_METHOD_DECLARE( py_createInsideChunkDataSection )
	PY_MODULE_STATIC_METHOD_DECLARE( py_cloneChunkDataSection )
//	PY_MODULE_STATIC_METHOD_DECLARE( py_cloneChunks )
	PY_MODULE_STATIC_METHOD_DECLARE( py_saveChunkTemplate )
	PY_MODULE_STATIC_METHOD_DECLARE( py_saveChunkPrefab )
	PY_MODULE_STATIC_METHOD_DECLARE( py_loadChunkPrefab )

	static DataSectionPtr utilCreateInsideChunkDataSection( Chunk * pNearbyChunk, std::string& retNewChunkName );
	static std::string utilCreateInsideChunkName( Chunk * pNearbyChunk );
	static void utilCloneChunkDataSection( Chunk* chunk, DataSectionPtr ds, const std::string& newChunkName );
	static void utilCloneChunkDataSection( DataSectionPtr sourceDataSection, DataSectionPtr ds, const std::string& newChunkName );
	static Chunk* utilCreateChunk( DataSectionPtr pDS, const std::string& newChunkName, Matrix* m = NULL );

	/**
	 *	Delete the given chunk, and add an undo operation to recreate it
	 */
	static bool deleteChunk( Chunk * pChunk );

	/**
	 *	Hide/Unhide the given chunk and its contained items.
	 */
	static bool chunkHidden( Chunk* pChunk, bool value );

	/**
	 *	Freeze/Unfreeze the given chunk and its contained items.
	 */
	static bool chunkFrozen( Chunk* pChunk, bool value );

	/**
	 *	Clone the given chunk, and add an undo operation to delete it
	 */
	static ChunkItem * cloneChunk(
		Chunk * pChunk, const Matrix & newTransform,
		EditorChunkItemLinkableManager::GuidToGuidMap & linkerCloneGuidMapping );
};


#endif