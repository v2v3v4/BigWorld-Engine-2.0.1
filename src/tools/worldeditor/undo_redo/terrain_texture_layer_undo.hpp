/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TEXTURE_LAYER_UNDO
#define TERRAIN_TEXTURE_LAYER_UNDO


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/undoredo.hpp"
#include "terrain/editor_base_terrain_block.hpp"
#include "terrain/terrain_texture_layer.hpp"
#include "chunk/chunk.hpp"


/**
 *  This class saves the state of the layers for a terrain block, and
 *	allows restoring the saved state.
 */
class TerrainTextureLayerState : public ReferenceCount
{
public:
    TerrainTextureLayerState
    (
        Terrain::EditorBaseTerrainBlockPtr  block, 
        ChunkPtr                        chunk  
    );

	void restore();

	bool operator==( const TerrainTextureLayerState &oth ) const;

	Terrain::EditorBaseTerrainBlockPtr block() const { return block_; }

	ChunkPtr chunk() const { return chunk_; }

private:
    struct LayerInfo
    {
        typedef Terrain::TerrainTextureLayer::ImageType ImageType;
        typedef SmartPointer<ImageType>             ImageTypePtr;

        std::string     textureName_;
        Vector4         uProjection_;
        Vector4         vProjection_;
        BinaryPtr	    compBlends_;
    };

    typedef std::vector<LayerInfo> LayerInfoVec;

    Terrain::EditorBaseTerrainBlockPtr          block_;
    ChunkPtr				                chunk_;
    LayerInfoVec                            textureInfo_;
};
typedef SmartPointer<TerrainTextureLayerState> TerrainTextureLayerStatePtr;


/**
 *  This class can be used to save and restore the a single texture layer of a 
 *  terrain block.
 */
class TerrainTextureLayerUndo : public UndoRedo::Operation
{
public:
    TerrainTextureLayerUndo
    (
        Terrain::EditorBaseTerrainBlockPtr  block, 
        ChunkPtr                        chunk  
    );

    explicit TerrainTextureLayerUndo
    (
		TerrainTextureLayerStatePtr layerState
    );

    virtual void undo();

    virtual bool iseq(const UndoRedo::Operation &oth) const;

private:
	TerrainTextureLayerStatePtr layerState_;
};


#endif // TERRAIN_TEXTURE_LAYER_UNDO
