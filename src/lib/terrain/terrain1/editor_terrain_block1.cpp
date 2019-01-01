/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#ifdef EDITOR_ENABLED

#include "editor_terrain_block1.hpp"
#include "../terrain_data.hpp"
#include "terrain_height_map1.hpp"

#include "../terrain_hole_map.hpp"
#include "../horizon_shadow_map.hpp"
#include "terrain_texture_layer1.hpp"
#include "terrain_height_map1.hpp"
#include "resmgr/data_section_census.hpp"
#include "resmgr/bin_section.hpp"
#include "resmgr/auto_config.hpp"
#include <limits>

DECLARE_DEBUG_COMPONENT2( "Moo", 0 );

using namespace Terrain;

static AutoConfigString s_blankTerrainTexture(
	"system/emptyTerrainTextureBmp", "helpers/maps/aid_builder.tga" );


EditorTerrainBlock1::EditorTerrainBlock1():
    TerrainBlock1()
{
}


bool EditorTerrainBlock1::postLoad(std::string const &filename, 
            Matrix			const &worldTransform,
            Vector3         const &cameraPosition,
            DataSectionPtr  pTerrain,
            TextureLayers   &textureLayers,
            std::string     *error /* = NULL*/)
{
	BW_GUARD;
	TerrainBlock1::postLoad(
		filename, worldTransform, cameraPosition, pTerrain,
		textureLayers, error );
	
	recalcLayerStates();
	return true;
}


bool EditorTerrainBlock1::load
(	
	std::string			const &filename, 
	Matrix				const &worldTransform,
	Vector3				const &cameraPosition,
	std::string			*error /*= NULL*/
)
{
	bool ok = TerrainBlock1::load( filename, worldTransform, cameraPosition, error );
	if (ok)
	{
		worldPos_ = worldTransform.applyToOrigin();
	}
	return ok;
}


/*virtual*/ bool EditorTerrainBlock1::save(DataSectionPtr dataSection) const
{
	BW_GUARD;
	bool ok = true;

	{
		// write the header
		TerrainBlockHeader tbh;
		ZeroMemory( &tbh, sizeof( TerrainBlockHeader ) );
		tbh.version_ = 0x101;
		tbh.heightMapWidth_ = width();
		tbh.heightMapHeight_ = height();
		tbh.spacing_ = spacing();
		tbh.nTextures_ = textureLayers().size();
		tbh.textureNameSize_ = textureNameSize();
		tbh.detailWidth_ = detailWidth();
		tbh.detailHeight_ = detailHeight();

		// discard old data
		dataSection->setBinary
		( 
			new BinaryBlock( NULL, size(), "BinaryBlock/EditorTerrainBlock1" ) 
		);

		BinaryPtr pTerrainData = dataSection->asBinary();
		if (!pTerrainData || pTerrainData->len() != size())
			return false;
			
		unsigned char* data = (unsigned char*)pTerrainData->data();
		memcpy( data, &tbh, sizeof(TerrainBlockHeader) );
	}

	ok &= this->heightMap().save( dataSection );

	ok &= this->shadowMap().save( dataSection );

	ok &= this->holeMap().save( dataSection );

	for (size_t i = 0; i < textureLayers().size(); ++i)
	{
	   ok &= this->textureLayers()[i]->save( dataSection );
	}

	return ok;
} 


/**
 *  Returns the size required to store a terrain1 block
 *
 *  @returns        size required to store a terrain1 block
 */
uint32 EditorTerrainBlock1::size() const
{
	return holesOffset() + blocksWidth() * blocksHeight() * sizeof(bool);
}


/*virtual*/ bool EditorTerrainBlock1::saveToCData(DataSectionPtr pCDataSection) const
{
	BW_GUARD;
	bool ret = false;
	if (pCDataSection)
	{
		DataSectionPtr dataSection = 
			pCDataSection->openSection( dataSectionName(), true );
		if ( dataSection )
		{
			// allocate space first, in case the bin section is new and empty.
			dataSection->setBinary
			(
				new BinaryBlock
				( 
					NULL, 
					size(),
					"BinaryBlock/EditorTerrainBlock1"
				)
			);
			// and then save
			ret = save(dataSection);
			ret &= pCDataSection->save();
		}
	}
    return ret;
}


/*virtual*/ bool EditorTerrainBlock1::save(std::string const &filename) const
{
	BW_GUARD;
	bool ret = false;
	DataSectionPtr pCDataSection = BWResource::openSection(filename);
	if (!pCDataSection)
	{
		// make a new empty bin section
		pCDataSection = 
			new BinSection
			( 
				filename, 
				new BinaryBlock( NULL, 0, "BinaryBlock/EditorTerrainBlock1" ) 
			);
		pCDataSection->setParent( BWResource::instance().rootSection() );
		pCDataSection = DataSectionCensus::add( filename, pCDataSection );
	}
    return saveToCData( pCDataSection );
}


/*virtual*/ size_t EditorTerrainBlock1::numberTextureLayers() const
{
    BW_GUARD;
	const TextureLayers &layers = textureLayers();
	size_t numLayers = 0;
	for( uint32 i = 0; i < layers.size(); ++i )
	{
		if ( !isLayerEmpty_[i] )
			numLayers++;
	}
    return numLayers;
}


/*virtual*/ size_t EditorTerrainBlock1::maxNumberTextureLayers() const
{
	// textureLayers().size() should be 4 for the old terrain!
    return textureLayers().size();
}


size_t EditorTerrainBlock1::realTextureLayerIndex( size_t idx ) const
{
    BW_GUARD;
	const TextureLayers &layers = textureLayers();
	size_t realIndex = 0;
	for( uint32 i = 0; i < layers.size(); ++i )
	{
		if ( !isLayerEmpty_[i] )
		{
			if ( idx-- == 0 )
				break;
		}
		realIndex++;
	}

	MF_ASSERT( realIndex < layers.size() );

	return realIndex;
}


bool EditorTerrainBlock1::textureLayerEmpty( size_t realIdx ) const
{
	BW_GUARD;
	TerrainTextureLayer1::ImageType& image = textureLayers()[ realIdx ]->image();
	uint8* p = image.getRow( 0 );
	uint8* end = p + image.rawDataSize(); // assuming no padding
	while( p != end )
		if ( *p++ )
			return false;

	return true;
}


void EditorTerrainBlock1::recalcLayerStates()
{
    BW_GUARD;
	TextureLayers &layers = textureLayers();
	isLayerEmpty_.resize( layers.size() );
	for( uint32 i = 0; i < layers.size(); ++i )
	{
		isLayerEmpty_[i] = textureLayerEmpty( i );
	}
}


/*virtual*/ size_t EditorTerrainBlock1::insertTextureLayer(
	uint32 blendWidth /*= 0*/, uint32 blendHeight /*= 0*/ )
{
	BW_GUARD;
	// Assert to make sure calls are consistent, even if we don't use these
	// values.
	MF_ASSERT(
		blendWidth != 0  || blendHeight != 0 ||
		blendWidth != 25 || blendHeight != 25 );
	
    TextureLayers &layers = textureLayers();
	for ( uint32 i = 0; i < layers.size(); ++i )
	{
		if ( isLayerEmpty_[i] )
		{
			isLayerEmpty_[i] = false;
			layers[i]->textureName( s_blankTerrainTexture.value() );
			layers[i]->image().fill(0);
			// the virtual index is the same as the real index, because this
			// was the first empty texture.
			return int32( i );
		}
	}
    return INVALID_LAYER;
}


/*virtual*/ bool EditorTerrainBlock1::removeTextureLayer(size_t idx)
{
    BW_GUARD;
	TextureLayers &layers = textureLayers();
	size_t i = realTextureLayerIndex( idx );
	isLayerEmpty_[i] = true;
	layers[i]->textureName( s_blankTerrainTexture.value() );
	layers[i]->image().fill(0);
    return true;
}


/*virtual*/ TerrainTextureLayer &EditorTerrainBlock1::textureLayer(size_t idx)
{
	idx = realTextureLayerIndex( idx );
    return *textureLayers()[idx];
}


/*virtual*/ TerrainTextureLayer const &EditorTerrainBlock1::textureLayer(size_t idx) const
{
	idx = realTextureLayerIndex( idx );
    return *textureLayers()[idx];
}


/*virtual*/ void EditorTerrainBlock1::draw(TerrainTextureSetter *tts)
{
    if( s_drawSelection_ )
		Moo::rc().setRenderState( D3DRS_TEXTUREFACTOR, selectionKey_ );
	TerrainBlock1::draw( tts );
}


/*virtual*/ bool EditorTerrainBlock1::draw( Moo::EffectMaterialPtr pMaterial )
{
    if( s_drawSelection_ )
		Moo::rc().setRenderState( D3DRS_TEXTUREFACTOR, selectionKey_ );
	return TerrainBlock1::draw( pMaterial );
}


/*virtual*/ void EditorTerrainBlock1::drawIgnoringHoles(bool setTextures /*= true*/)
{
    // TODO: is this still needed????
}


/*virtual*/ void EditorTerrainBlock1::setHeightMapDirty()
{
	if ( managedObjectsCreated() )
	{
		// recreate DX objects.
		deleteManagedObjects();
		createManagedObjects();
	}
}


/*virtual*/ void EditorTerrainBlock1::rebuildNormalMap( NormalMapQuality normalMapQuality )
{
	// no need to do this in the old terrain
}



#endif // EDITOR_ENABLED
