/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TEXTURE_FUNCTOR_HPP
#define TERRAIN_TEXTURE_FUNCTOR_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"

#include "worldeditor/terrain/terrain_functor.hpp"
#include "worldeditor/terrain/texture_mask_cache.hpp"
#include "worldeditor/misc/editor_renderable.hpp"

#include "terrain/terrain_texture_layer.hpp"

#include "math/linear_lut.hpp"


/**
 *  This class applies the texture to the blends (alphas) the texture layers.
 */
class TerrainTextureFunctor : public TerrainFunctor
{
	Py_Header( TerrainTextureFunctor, TerrainFunctor )

public:
	explicit TerrainTextureFunctor( PyTypePlus * pType = &s_type_ );

	virtual bool handleKeyEvent(const KeyEvent & keyEvent, Tool & tool);

	void	update( float dTime, Tool & t );

	bool	handleContextMenu( Tool & tool );

	bool	applying() const;

	void    onEndUsing( Tool & tool );

    void getBlockFormat( 
		const EditorChunkTerrain &	chunkTerrain,
		TerrainUtils::TerrainFormat	& format 
	) const;

	void onFirstApply( EditorChunkTerrain & chunkTerrain );

	void applyToSubBlock( 
		EditorChunkTerrain &		chunkTerrain,
		const Vector3 & 			toolOffset, 
		const Vector3 &				chunkOffset,
		const TerrainUtils::TerrainFormat & format,
		int32						minx, 
		int32						minz, 
		int32						maxx, 
		int32						maxz );

	void onApplied( Tool & t );

	void onLastApply( EditorChunkTerrain & chunkTerrain );

	bool showWaitCursorOnLastApply() const;

	void mode(int m);
	int mode() const;

    TerrainPaintBrushPtr paintBrush() const;
    void paintBrush( TerrainPaintBrushPtr m );

	int displayOverlay() const;
	void displayOverlay( int display );

	int lastPaintedLayer() const;
	void lastPaintedLayer( int layer );

	const Vector3 & lastPaintedPos() const;
	void lastPaintedPos( const Vector3 & pos );

	int hadEscapeKey() const;
	void hadEscapeKey( int hadEscKey );

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( int				  , mode             , mode			     )
    PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( TerrainPaintBrushPtr, paintBrush       , paintBrush        )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( int	        	  , displayOverlay   , displayOverlay    )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( int		    	  , lastPaintedLayer , lastPaintedLayer  )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Vector3	    	  , lastPaintedPos   , lastPaintedPos    )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( int		    	  , hadEscapeKey     , hadEscapeKey      )
    

	PY_FACTORY_DECLARE()

    FUNCTOR_FACTORY_DECLARE( TerrainTextureFunctor() )

protected:

	size_t blendLevel( const Terrain::EditorBaseTerrainBlock & block ) const;

	size_t maskLevel( const Terrain::EditorBaseTerrainBlock & block ) const;

	void updateMaskProjection( Tool & tool );

	void createOverlayImage(
		EditorChunkTerrainPtr	terrain, 
		Moo::Image<uint8> &		img	) const;

	void invalidateOverlays();

	void applyToSubBlockNoTexMask( 
		EditorChunkTerrain &		chunkTerrain,
		const Vector3 &				toolOffset, 
		const Vector3 &				chunkOffset,
		const TerrainUtils::TerrainFormat & format,
		int32						minx, 
		int32						minz, 
		int32						maxx, 
		int32						maxz );

	void applyToSubBlockTexMask( 
		EditorChunkTerrain &		chunkTerrain,
		const Vector3 & 			toolOffset, 
		const Vector3 &				chunkOffset,
		const TerrainUtils::TerrainFormat & format,
		int32						minx, 
		int32						minz, 
		int32						maxx, 
		int32						maxz );

	void endApplyInternal();

private:
    typedef Terrain::TerrainTextureLayer::ImageType ImageType;
	typedef std::map< EditorChunkTerrain *, EditorRenderablePtr > ChunkRenderMap;
	typedef std::set< Terrain::EditorBaseTerrainBlock * > FullBlocks;

	int							mode_;
	float                       falloff_;
	float						randomness_;
	float						randomStrength_;
    EditorChunkTerrain *		pCurrentTerrain_;
    std::vector< ImageType * >	images_;
    TerrainPaintBrushPtr		pPaintBrush_;
	int							displayOverlay_;
	ChunkRenderMap				chunkRenderMap_;
	bool						applying_;
	FullBlocks					fullBlocks_;
	int							lastPaintedLayer_;
	Vector3						lastPaintedPos_;
	int							hadEscKey_;
	LinearLUT					strengthLUT_;
	float						currentStrength_;
};

#endif // TERRAIN_TEXTURE_FUNCTOR_HPP
