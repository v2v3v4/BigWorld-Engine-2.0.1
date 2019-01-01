/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_META_DATA_HPP
#define EDITOR_CHUNK_META_DATA_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/items/editor_chunk_substance.hpp"
	

/**
 *	This class is the editor version of a sound item in a chunk.
 */
class EditorChunkMetaData : public EditorChunkSubstance<ChunkItem>
{
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkMetaData )
public:
	EditorChunkMetaData();
	~EditorChunkMetaData();

	#if UMBRA_ENABLE
	virtual void tick( float /*dTime*/ );
	#endif // UMBRA_ENABLE

	virtual void syncInit();

	bool load( DataSectionPtr pSection, Chunk * pChunk );

	virtual bool edSave( DataSectionPtr pSection );

	virtual const Matrix & edTransform()	{ return transform_; }
	virtual bool edTransform( const Matrix & m, bool transient );

	virtual bool edEdit( class GeneralEditor & editor );

private:
	EditorChunkMetaData( const EditorChunkMetaData& );
	EditorChunkMetaData& operator=( const EditorChunkMetaData& );

	virtual const char * sectName() const	{ return "metaData"; }
	virtual const char * drawFlag() const	{ return "render/drawChunkMetaData"; }
	virtual ModelPtr reprModel() const;

	Matrix			transform_;

#if UMBRA_ENABLE
	// Pointer to whichever of the big small icons umbra should use.
	ModelPtr	currentUmbraModel_;
#endif

	ModelPtr model_;
};


#endif // EDITOR_CHUNK_META_DATA_HPP
