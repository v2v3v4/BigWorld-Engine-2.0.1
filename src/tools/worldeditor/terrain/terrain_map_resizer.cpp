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
#include "worldeditor/terrain/terrain_map_resizer.hpp"
#include "chunk/chunk_format.hpp"
#include "terrain/terrain_settings.hpp"
#include "terrain/terrain2/editor_terrain_block2.hpp"
#include "terrain/terrain2/terrain_texture_layer2.hpp"
#include "romp/progress.hpp"
#include <set>



//-----------------------------------------------------------------------------
// Section: TerrainMapResizer::MapSizeInfo
//-----------------------------------------------------------------------------


namespace
{
	// Constant to make know whether a MapSizeInfo struct is not valid.
	const uint32 INVALID_SIZE = std::numeric_limits< uint32 >::max();
}


/**
 *	MapSizeInfo Constructor
 */
TerrainMapResizer::MapSizeInfo::MapSizeInfo() :
	heightMap_( INVALID_SIZE ),
	normalMap_( INVALID_SIZE ),
	holeMap_( INVALID_SIZE ),
	shadowMap_( INVALID_SIZE ),
	blendMap_( INVALID_SIZE )
{
}


/**
 *	Checks the MapSizeInfo structure to see if it has been initialised properly
 *	
 *	@return		true if the map sizes are valid, false otherwise.
 */
bool TerrainMapResizer::MapSizeInfo::valid() const
{
	return 
		heightMap_ != INVALID_SIZE &&
		normalMap_ != INVALID_SIZE &&
		holeMap_ != INVALID_SIZE &&
		shadowMap_ != INVALID_SIZE &&
		blendMap_ != INVALID_SIZE;
}


//-----------------------------------------------------------------------------
// Section: TerrainMapResizer
//-----------------------------------------------------------------------------

/**
 *	TerrainMapResizer Constructor
 */
TerrainMapResizer::TerrainMapResizer()
{
}


/**
 *	TerrainMapResizer Destructor
 */
TerrainMapResizer::~TerrainMapResizer()
{
}


/**
 *	Returns the map sizes from the space's 'space.settings'.
 *
 *	@param pSpaceSettings	DataSection of the space.settings file.
 *	@return					Struct containing the space's map sizes.
 */
TerrainMapResizer::MapSizeInfo TerrainMapResizer::spaceSizes(
									DataSectionPtr pSpaceSettings ) const
{
	BW_GUARD;

	// Check data sections
	MapSizeInfo sizes;

	if (!pSpaceSettings)
	{
		ERROR_MSG( "Cannot resize terrain maps, space.settings file not found.\n" );
		return sizes;
	}

	DataSectionPtr pTerrainDS = pSpaceSettings->openSection( "terrain" );
	if (!pTerrainDS)
	{
		ERROR_MSG( "Cannot resize terrain maps, 'terrain' section not found in space.settings file.\n" );
		return sizes;
	}

	Terrain::TerrainSettingsPtr pTempSettings = new Terrain::TerrainSettings;
	pTempSettings->init( pTerrainDS );

	if (pTempSettings->version() != 200)
	{
		ERROR_MSG( "Cannot resize terrain maps, terrain version not supported\n" );
		return sizes;
	}

	sizes.heightMap_ = pTempSettings->heightMapSize();
	sizes.normalMap_ = pTempSettings->normalMapSize();
	sizes.holeMap_ = pTempSettings->holeMapSize();
	sizes.shadowMap_ = pTempSettings->shadowMapSize();
	sizes.blendMap_ = pTempSettings->blendMapSize();

	return sizes;
}


/**
 *	Resizes a space's terrain maps.
 *
 *	@param space		Space folder of the space to resize.
 *	@param newSizes		Struct containing the new size, or 0 to keep the old
 *						size, for each terrain map.
 *	@param pProgress	Optional progress bar object, or NULL.
 *	@return				True if successful, false otherwise.
 */
bool TerrainMapResizer::resize( const std::string & space, const MapSizeInfo & newSizes,
							   ProgressTask * pProgress /*=NULL*/ )
{
	BW_GUARD;

	DataSectionPtr pSpaceSection = BWResource::openSection( space );
	if (!pSpaceSection)
	{
		ERROR_MSG( "Cannot resize terrain maps, couldn't find space '%s'.\n", space.c_str() );
		return false;
	}

	DataSectionPtr pSpaceSettings = pSpaceSection->openSection( SPACE_SETTING_FILE_NAME );
	if (!pSpaceSettings)
	{
		ERROR_MSG( "Cannot resize terrain maps, couldn't open '%s' for '%s'.\n",
			SPACE_SETTING_FILE_NAME.c_str(), space.c_str() );
		return false;
	}
	MapSizeInfo oldSizes = this->spaceSizes( pSpaceSettings );

	if (!oldSizes.valid())
	{
		ERROR_MSG( "Cannot resize terrain maps, couldn't read the space's terrain settings.\n" );
		return false;
	}

	DataSectionPtr pSpaceTerrain = pSpaceSettings->openSection( "terrain" );
	if (!pSpaceTerrain)
	{
		ERROR_MSG( "Cannot resize terrain maps, couldn't read the space's terrain settings.\n" );
		return false;
	}
	pTerrainSettings_ = new Terrain::TerrainSettings;
	pTerrainSettings_->init( pSpaceTerrain );

	// Validate trivial case: sizes are the same.
	if ((oldSizes.heightMap_ == newSizes.heightMap_ || newSizes.heightMap_ == DONT_RESIZE) &&
		(oldSizes.normalMap_ == newSizes.normalMap_ || newSizes.normalMap_ == DONT_RESIZE) && 
		(oldSizes.shadowMap_ == newSizes.shadowMap_ || newSizes.shadowMap_ == DONT_RESIZE) && 
		(oldSizes.holeMap_ == newSizes.holeMap_ || newSizes.holeMap_ == DONT_RESIZE) && 
		(oldSizes.blendMap_ == newSizes.blendMap_ || newSizes.blendMap_ == DONT_RESIZE))
	{
		ERROR_MSG( "No need to resize terrain maps, sizes given are identical to the current map sizes.\n" );
		return true;
	}

	// Something different, so must convert

	// TODO: support converting all maps, not just the blends.
	if ((newSizes.heightMap_ != DONT_RESIZE && oldSizes.heightMap_ != newSizes.heightMap_) ||
		(newSizes.normalMap_ != DONT_RESIZE && oldSizes.normalMap_ != newSizes.normalMap_) ||
		(newSizes.shadowMap_ != DONT_RESIZE && oldSizes.shadowMap_ != newSizes.shadowMap_) ||
		(newSizes.holeMap_ != DONT_RESIZE && oldSizes.holeMap_ != newSizes.holeMap_))
	{
		ERROR_MSG( "Cannot resize terrain maps, only blend resizing supported.\n" );
		return false;
	}

	if (!this->convert( space, pSpaceSettings, oldSizes, newSizes, pProgress ))
	{
		return false;
	}

	if (!this->changeTerrainSettings( pSpaceSettings, newSizes ))
	{
		return false;
	}

	return true;
}


/**
 *	Internal method to load a block
 *
 *	@param space		Space folder of the space where the block resides.
 *	@param postfix		Postfix to add to the cdata file name, or "" to load
 *						the block from the plain cdata file.
 *	@param x			Grid X coord of the block in the space.
 *	@param z			Grid Z coord of the block in the space.
 *	@param pError		Optional string pointer to return error messages.
 *	@return				Terrain block pointer if successful, or NULL otherwise.
 */
Terrain::EditorTerrainBlock2Ptr TerrainMapResizer::loadBlock(
	const std::string & space, const std::string & postfix,
	int x, int z, std::string * pError /* = NULL*/ )
{
	BW_GUARD;

	Matrix transform;
	transform.setTranslate( GRID_RESOLUTION * x, 0.0f, GRID_RESOLUTION * z );

	Terrain::EditorTerrainBlock2Ptr pBlock = new Terrain::EditorTerrainBlock2( pTerrainSettings_ );

	std::string chunkId = ChunkFormat::outsideChunkIdentifier( x, z );
	std::string cDataName = space + "/" + chunkId + ".cdata" + postfix;
	std::string terrainName = cDataName + "/" + pBlock->dataSectionName();
	if (!pBlock->load( terrainName, transform, transform.applyToOrigin(), pError ))
	{
		if (pError)
		{
			*pError = "Cannot load block " + cDataName + ": " + *pError;
		}
		return NULL;
	}

	return pBlock.getObject();
}


/**
 *	Internal method to resize a space's terrain.
 *
 *	@param space			Space folder of the space to resize.
 *	@param pSpaceSettings	DataSection of the space.settings file.
 *	@param oldSizes			Original map sizes of the terrain.
 *	@param newSizes			New, desired map sizes of the terrain.
 *	@param pProgress		Optional progress bar object, or NULL.
 *	@return					True if successful, or false otherwise.
 */
bool TerrainMapResizer::convert(
	const std::string & space, DataSectionPtr pSpaceSettings,
	const MapSizeInfo & oldSizes, const MapSizeInfo & newSizes,
	ProgressTask * pProgress /* = NULL */ )
{
	BW_GUARD;

	int minX = pSpaceSettings->readInt( "bounds/minX" );
	int maxX = pSpaceSettings->readInt( "bounds/maxX" );
	int minZ = pSpaceSettings->readInt( "bounds/minY" );
	int maxZ = pSpaceSettings->readInt( "bounds/maxY" );

	if (pProgress)
	{
		// There are two iterations, and one iteration is much slower than
		// the other, so using 2/3 of the progress bar length for the slow
		// iteration and 1/3 for the fast one.
		pProgress->length( ( maxZ - minZ + 1.0f ) * ( maxX - minX + 1.0f ) * 3 );
	}

	for (int32 z = minZ; z <= maxZ; z++)
	{
		for (int32 x = minX; x <= maxX; x++)
		{
			std::string error;
			Terrain::EditorTerrainBlock2Ptr pBlock = loadBlock( space, "", x, z, &error );
			if (!pBlock)
			{
				ERROR_MSG( "%s\n", error.c_str() );
				return false;
			}

			// resize height map
			// TODO!

			// resize normal map
			// TODO!

			// resize shadow map
			// TODO!

			// resize hole map
			// TODO!

			// resize blends
			if (newSizes.blendMap_ != DONT_RESIZE)
			{
				if (!resizeBlends( pBlock, space, x, z, newSizes.blendMap_ ))
				{
					ERROR_MSG( "Failed to resize blends for block %s\n", pBlock->resourceName().c_str() );
					return false;
				}
			}

			Matrix m( Matrix::identity );
			m.translation(
				Vector3( GRID_RESOLUTION * x, 0.0f, GRID_RESOLUTION * z ) );
			if (!pBlock->rebuildLodTexture( m ))
			{
				ERROR_MSG( "Cannot regenerate lod texture for block %s\n", pBlock->resourceName().c_str() );
				return false;
			}
			std::string chunkId = ChunkFormat::outsideChunkIdentifier( x, z );
			std::string cDataName = space + "/" + chunkId + ".cdata";
			std::string original = BWResource::resolveFilename( cDataName );
			std::string converted = original + ".temp";
			std::wstring woriginal, wconverted;
			bw_utf8tow( original, woriginal );
			bw_utf8tow( converted, wconverted );
			CopyFile( woriginal.c_str(), wconverted.c_str(), FALSE );
			DataSectionPtr pCDataDS = BWResource::openSection( cDataName + ".temp" );
			if (!pCDataDS)
			{
				ERROR_MSG( "Cannot open temp block %s for resizing\n", pBlock->resourceName().c_str() );
				return false;
			}
			pCDataDS = pCDataDS->convertToZip();
			DataSectionPtr pTerrainDS = pCDataDS->openSection( pBlock->dataSectionName() );
			if (!pTerrainDS)
			{
				ERROR_MSG( "Cannot open temp terrain data for block %s for resizing\n", pBlock->resourceName().c_str() );
				return false;
			}
			pTerrainDS->setParent( pCDataDS );
			pTerrainDS = pTerrainDS->convertToZip();
			// TODO: when we implement resizing height, etc, we must update the
			// shadow dirty flags as well.
			bool savedOK = (pBlock->save( pTerrainDS ) && pCDataDS->save());
			pTerrainDS->setParent( NULL );
			if (!savedOK)
			{
				ERROR_MSG( "Cannot save resized block %s\n", pBlock->resourceName().c_str() );
				return false;
			}
			if (pProgress)
			{
				pProgress->step( 2 ); // step twice because this step is slow.
			}
		}
	}

	// delete old cdata files and rename temp files
	for (int32 z = minZ; z <= maxZ; z++)
	{
		for (int32 x = minX; x <= maxX; x++)
		{
			std::string chunkId = ChunkFormat::outsideChunkIdentifier( x, z );
			std::string cDataName = space + "/" + chunkId + ".cdata";
			std::string original = BWResource::resolveFilename( cDataName );
			std::string converted = original + ".temp";
			std::remove( original.c_str() );
			std::wstring wconverted, woriginal;
			bw_utf8tow( converted, wconverted );
			bw_utf8tow( original, woriginal );
			MoveFile( wconverted.c_str(), woriginal.c_str() );
			if (pProgress)
			{
				pProgress->step();
			}
		}
	}

	return true;
}


/**
 *	Internal method to resize a terrain block's blends maps.
 *
 *	@param pBlock		Terrain block pointer to the block to resize.
 *	@param space		Space folder of the space to resize.
 *	@param x			Grid X coord of the block in the space.
 *	@param z			Grid Z coord of the block in the space.
 *	@param size			New size for the blends map.
 *	@return				True if successful, or false otherwise.
 */
bool TerrainMapResizer::resizeBlends(
	Terrain::EditorTerrainBlock2Ptr pBlock, const std::string & space, int x, int z,
	uint32 size )
{
	BW_GUARD;

	if (size == 0 || pBlock->numberTextureLayers() == 0)
	{
		return true;
	}

	Terrain::EditorTerrainBlock2Ptr pLeft = loadBlock( space, ".temp", x - 1, z );
	Terrain::EditorTerrainBlock2Ptr pTop = loadBlock( space, ".temp", x, z - 1 );

	// Merge blends of layers with the same texture into one texture
	for (size_t i = 0; i < pBlock->numberTextureLayers() - 1; i++)
	{
		Terrain::TerrainTextureLayer & layerI = pBlock->textureLayer( i );
		for (size_t j = i + 1; j < pBlock->numberTextureLayers(); j++)
		{
			Terrain::TerrainTextureLayer & layerJ = pBlock->textureLayer( j );
			if (layerI.sameTexture(
				layerJ.textureName(),
				layerJ.uProjection(),
				layerJ.vProjection(),
				0.01f ))
			{	
				// j is the same as i, so blend j's values into i and delete j
				{
					// Scoping the lock holders before the removeLayer
					Terrain::TerrainTextureLayerHolder lockerI( &layerI, false );
					Terrain::TerrainTextureLayerHolder lockerJ( &layerJ, true );
					size_t curSize = layerI.width();
					for (size_t z = 0; z < curSize; ++z)
					{
						Terrain::TerrainTextureLayer::PixelType * dst = layerI.image().getRow( z );
						Terrain::TerrainTextureLayer::PixelType * src = layerJ.image().getRow( z );
						for (size_t x = 0; x < curSize; ++x)
						{
							*dst = *src + *dst;
							++dst;
							++src;
						}
					}
				}
				pBlock->removeTextureLayer( j );
				j--;
			}
		}
	}

	// resize this layer
	for (size_t l = 0; l < pBlock->numberTextureLayers(); ++l)
	{
		Terrain::TerrainTextureLayer & layer = pBlock->textureLayer( l );
		this->resizeLayer( layer, size );
		// Copy edge pixels from left and top neighbour blocks
		// (already processed).
		Terrain::TerrainTextureLayerHolder locker( &layer, false );
		if (pLeft)
		{
			size_t layerIndex = pLeft->findLayer( layer, 0.01f );
			if (layerIndex < pLeft->numberTextureLayers())
			{
				Terrain::TerrainTextureLayerHolder locker2( &pLeft->textureLayer( layerIndex ), true );
				layer.image().blit(
					pLeft->textureLayer( layerIndex ).image(),
					layer.width() - 1, 0,
					1, layer.height(),
					0, 0 );
			}
		}
		if (pTop)
		{
			size_t layerIndex = pTop->findLayer( layer, 0.01f );
			if (layerIndex < pTop->numberTextureLayers())
			{
				Terrain::TerrainTextureLayerHolder locker2( &pTop->textureLayer( layerIndex ), true );
				layer.image().blit(
					pTop->textureLayer( layerIndex ).image(),
					0, layer.height() - 1,
					layer.width(), 1,
					0, 0 );
			}
		}
	}

	// Normalise, because re-sampling might have introduced errors in the sum,
	// and also borders might have changed.
	for (size_t l = 0; l < pBlock->numberTextureLayers(); ++l)
	{
		pBlock->textureLayer( l ).lock( false );
	}

	for (size_t z = 0; z < size; ++z)
	{
		for (size_t x = 0; x < size; ++x)
		{
			int sum = 0;
			for (size_t l = 0; l < pBlock->numberTextureLayers(); ++l)
			{
				sum += pBlock->textureLayer( l ).image().get( x, z );
			}

			if (sum == 0)
			{
				// This should never happen, but let's handle it anyway.
				sum = 255 / pBlock->numberTextureLayers();
				for (size_t l = 0; l < pBlock->numberTextureLayers(); ++l)
				{
					pBlock->textureLayer( l ).image().set( x, z, sum );
				}
			}
			else
			{
				for (size_t l = 0; l < pBlock->numberTextureLayers(); ++l)
				{
					pBlock->textureLayer( l ).image().set( x, z,
						pBlock->textureLayer( l ).image().get( x, z ) * 255 / sum );
				}
			}
		}
	}

	for (size_t l = 0; l < pBlock->numberTextureLayers(); ++l)
	{
		pBlock->textureLayer( l ).unlock();
	}

	// generate combined layers
	pBlock->rebuildCombinedLayers();

	return true;
}


/**
 *	Internal method set the space's terrain settings to match the new terrain
 *	map sizes.
 *
 *	@param pSpaceSettings	DataSection of the space.settings file.
 *	@param newSizes			New map sizes of the terrain.
 *	@return					True if successful, or false otherwise.
 */
bool TerrainMapResizer::changeTerrainSettings(
	DataSectionPtr pSpaceSettings, const MapSizeInfo & newSizes )
{
	BW_GUARD;

	DataSectionPtr pTerrainDS = pSpaceSettings->openSection( "terrain" );
	if (!pTerrainDS)
	{
		return false;
	}

	Terrain::TerrainSettingsPtr pTempSettings = new Terrain::TerrainSettings;
	pTempSettings->init( pTerrainDS );
		
	if (newSizes.heightMap_ != DONT_RESIZE) 
	{
		pTempSettings->heightMapSize( newSizes.heightMap_ );
	}

	if (newSizes.normalMap_ != DONT_RESIZE) 
	{
		pTempSettings->normalMapSize( newSizes.normalMap_ );
	}

	if (newSizes.shadowMap_ != DONT_RESIZE) 
	{
		pTempSettings->shadowMapSize( newSizes.shadowMap_ );
	}

	if (newSizes.holeMap_ != DONT_RESIZE) 
	{
		pTempSettings->holeMapSize( newSizes.holeMap_ );
	}

	if (newSizes.blendMap_ != DONT_RESIZE) 
	{
		pTempSettings->blendMapSize( newSizes.blendMap_ );
	}

	pTempSettings->save( pTerrainDS );
	pSpaceSettings->save();
	return true;
}


/**
 *	Internal method to resize a single texture layer's blends map.
 *
 *	@param layer		Layer to resize.
 *	@param newSize		New size of the layer's blends.
 */
void TerrainMapResizer::resizeLayer( Terrain::TerrainTextureLayer & layer, uint32 newSize )
{
	BW_GUARD;

	Terrain::TerrainTextureLayerHolder locker( &layer, false );
	
	Terrain::TerrainTextureLayer::ImageType newBlend( newSize, newSize );

	float xmult = layer.width() / float( newSize );
	float ymult = layer.height() / float( newSize );

	for (uint32 y = 0; y < newSize; ++y)
	{
		for (uint32 x = 0; x < newSize; ++x)
		{
			newBlend.set( x, y,
				layer.image().getBicubic( x * xmult, y * ymult ) );
		}
	}

	layer.image().resize( newSize, newSize );
	layer.image().blit( newBlend );
}