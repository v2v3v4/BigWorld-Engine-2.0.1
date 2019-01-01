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
#include "terrain/terrain2/editor_terrain_block2.hpp"
#include "terrain/terrain2/horizon_shadow_map2.hpp"
#include "terrain/terrain2/terrain_texture_layer2.hpp"
#include "terrain/terrain2/terrain_hole_map2.hpp"
#include "terrain/terrain2/terrain_height_map2.hpp"
#include "terrain/terrain2/terrain_normal_map2.hpp"
#include "terrain/terrain2/dominant_texture_map2.hpp"
#include "terrain/terrain2/terrain_photographer.hpp"
#include "terrain/terrain2/terrain_lod_map2.hpp"
#include "terrain/terrain2/terrain_renderer2.hpp"
#include "terrain/terrain_data.hpp"
#include "resmgr/data_section_census.hpp"
#include "resmgr/bin_section.hpp"
#include "resmgr/zip_section.hpp"
#include "resmgr/multi_file_system.hpp"
#include <limits>


using namespace Terrain;


DECLARE_DEBUG_COMPONENT2( "Moo", 0 );


#ifdef EDITOR_ENABLED


namespace
{
	// Frame mark used to prevent rebuildCombinedLayers to be called more than
	// once per frame.
	uint32 s_lastBlendBuildMark = 0;

	// Timestamp to keep track of the last time we rebuilt layers.
	uint64 s_lastBlendBuildTime = 0;

}


// Frame timestamp to prevent rebuildCombinedLayers to be called more than once
// per frame.
/*static*/ uint32 EditorTerrainBlock2::s_nextBlendBuildMark_ = 0;

// Interval between each layer rebuild, in milliseconds.
/*static*/ uint32 EditorTerrainBlock2::s_blendBuildInterval_ = 100;


EditorTerrainBlock2::EditorTerrainBlock2(TerrainSettingsPtr pSettings):
    TerrainBlock2( pSettings ),
	lodMapDirty_( false ),
	unlockCallback_( NULL )
{
}


EditorTerrainBlock2::~EditorTerrainBlock2()
{
	delete unlockCallback_; unlockCallback_ = NULL;
}


/**
 *  This function creates an EditorTerrainBlock2 using the settings in the
 *  DataSection.
 *
 *  @param settings         DataSection containing the terrain settings.
 *  @param error		    Optional output parameter that receives an error
 *							message if an error occurs.
 *  @returns                True if successful, false otherwise.
 */
bool EditorTerrainBlock2::create( DataSectionPtr settings, std::string* error /*= NULL*/ )
{
	BW_GUARD;
	unlockCallback_ = new ETB2UnlockCallback( *this );

	heightMap2( new TerrainHeightMap2( 0, 0, unlockCallback_ ) );
	if ( !heightMap2()->create( settings_->heightMapSize(), error ) )
		return false;

	holeMap2( new TerrainHoleMap2(*this) );
	if ( !holeMap2()->create( settings_->holeMapSize(), error ) )
		return false;

	normalMap2( new TerrainNormalMap2 );
	rebuildNormalMap( NMQ_NICE );

    horizonMap2( new HorizonShadowMap2(*this) );
	if ( !horizonMap2()->create( settings_->shadowMapSize(), error ) )
		return false;

	pDetailHeightMapResource_ = new HeightMapResource( fileName_, 0 );

	if ( !initVerticesResource( error ) )
		return false;
	
	// Force creation of blends:
	evaluate(Vector3::zero());
	stream();

	return true;
}



bool EditorTerrainBlock2::load
(	
	std::string			const &filename, 
	Matrix				const &worldTransform,
	Vector3				const &cameraPosition,
	std::string			*error /*= NULL*/
)
{
	BW_GUARD;
	bool ok = 
		TerrainBlock2::load(filename, worldTransform, cameraPosition, error);
	lodMapDirty_ = (pLodMap_ == NULL); // If the lodMap is missing, then mark it as dirty.
	if ( ok )
	{
		worldPos_ = worldTransform.applyToOrigin();

		unlockCallback_ = new ETB2UnlockCallback( *this );
		heightMap2()->unlockCallback( unlockCallback_ );

		// Validate block's map dimensions. This is specially useful for when
		// placing terrain prefabs.
		if ( heightMap().blocksWidth() != settings_->heightMapSize() )
		{
			return false;
		}
		if ( normalMapSize() != settings_->normalMapSize() )
		{
			return false;
		}
		// Must have at least one blend
		if (numberTextureLayers() == 0)
		{
			return false;
		}
		// Assuming all layers are the same size
		if ( numberTextureLayers() > 0 &&
			textureLayer( 0 ).width() != settings_->blendMapSize() )
		{
			return false;
		}
		if ( holesSize() != settings_->holeMapSize() )
		{
			return false;
		}
		if ( horizonMapSize() != settings_->shadowMapSize() )
		{
			return false;
		}
	}

	return ok;
}


bool EditorTerrainBlock2::saveLodTexture( DataSectionPtr dataSection ) const
{
	bool ok = false;

	dataSection->deleteSections( "lodTexture.dds" );

	if (lodMapDirty_)
	{
		ERROR_MSG("EditorTerrainBlock2 has dirty lod texture\n");
	}

	if (pLodMap_ != NULL && !lodMapDirty_)
	{
		ok = pLodMap_->save( dataSection, "lodTexture.dds" );
	}
	else
	{
		dataSection->deleteSection( "lodTexture.dds" );
	}

	return ok;
}


/*virtual*/ bool EditorTerrainBlock2::save(DataSectionPtr dataSection) const
{
	BW_GUARD;
	ensureBlendsLoaded();

	bool ok = true;

	ok = saveHeightmap( dataSection );

	ok &= pNormalMap_->save( dataSection );

	dataSection->deleteSections( "horizonShadows" );
	ok &= this->shadowMap().save( dataSection->newSection("horizonShadows",
												 BinSection::creator()) );

	dataSection->deleteSections( "holes" );
	if (!this->holeMap().noHoles())
	{
	   ok &= this->holeMap().save( dataSection->newSection("holes",
	   											 BinSection::creator()) );
	}

	dataSection->deleteSections( "layer" );
	for (size_t i = 0; i < numberTextureLayers(); ++i)
	{
	   ok &= this->textureLayer(i).save( dataSection->newSection("layer",
	   											BinSection::creator()) );
	}

	dataSection->deleteSections( "dominantTextures" );
	if ( this->dominantTextureMap() )
	{
		ok &= this->dominantTextureMap()->save( dataSection->newSection("dominantTextures",
												 BinSection::creator()) );
	}

	ok &= saveLodTexture( dataSection );

	// remove the seven (old 1.9) vertex lods if they exist.
	for ( uint32 i = 0; i < 7 ; i++ )
	{
		std::string sectionName;
		VertexLodEntry::getSectionName( i, sectionName );
		dataSection->deleteSections( sectionName.c_str() );
	}

	return ok;
} 


/*virtual*/ bool EditorTerrainBlock2::saveToCData(DataSectionPtr pCDataSection) const
{
	BW_GUARD;
	bool ret = false;
	if (pCDataSection)
	{
		DataSectionPtr dataSection = 
			pCDataSection->openSection( dataSectionName(), true,
										 ZipSection::creator());

		if ( dataSection )
		{
			dataSection->setParent(pCDataSection);
			dataSection = dataSection->convertToZip("",pCDataSection);
			ret = save(dataSection);
			ret &= pCDataSection->save();
			dataSection->setParent( NULL );
		}
	}
    return ret;
}


/*virtual*/ bool EditorTerrainBlock2::save(std::string const &filename) const
{
	BW_GUARD;
	DataSectionPtr pCDataSection = BWResource::openSection(filename, true,
													ZipSection::creator());
    return saveToCData( pCDataSection );
}


/*virtual*/ size_t EditorTerrainBlock2::numberTextureLayers() const
{
	BW_GUARD;
	ensureBlendsLoaded();

	if (pBlendsResource_->getObject())
	{
		return pBlendsResource_->getObject()->textureLayers_.size();
	}
	else
	{
		return 0;
	}
}


/*virtual*/ size_t EditorTerrainBlock2::maxNumberTextureLayers() const
{
    return std::numeric_limits<size_t>::max();
}


/*virtual*/ size_t EditorTerrainBlock2::insertTextureLayer(
	uint32 blendWidth /*= 0*/, uint32 blendHeight /*= 0*/ )
{
	BW_GUARD;
	ensureBlendsLoaded();

    TextureLayers *layers = textureLayers();
    TerrainTextureLayer2Ptr layer =
		new TerrainTextureLayer2(*this, blendWidth, blendHeight);
    layers->push_back( layer );
    return layers->size() - 1;
}


/*virtual*/ bool EditorTerrainBlock2::removeTextureLayer(size_t idx)
{
	BW_GUARD;
	ensureBlendsLoaded();

    TextureLayers* layers = textureLayers();
    layers->erase(layers->begin() + idx);
    return false;
}


/*virtual*/ TerrainTextureLayer &EditorTerrainBlock2::textureLayer(size_t idx)
{
	BW_GUARD;
	ensureBlendsLoaded();

    return *pBlendsResource_->getObject()->textureLayers_[idx];
}


/*virtual*/ TerrainTextureLayer const &EditorTerrainBlock2::textureLayer(size_t idx) const
{
	BW_GUARD;
	ensureBlendsLoaded();

    return *pBlendsResource_->getObject()->textureLayers_[idx];
}


/*virtual*/ bool EditorTerrainBlock2::draw( Moo::EffectMaterialPtr pMaterial )
{
    BW_GUARD;
	if( s_drawSelection_ )
	{
		HRESULT hr = Moo::rc().setRenderState( D3DRS_TEXTUREFACTOR, selectionKey_ );
		MF_ASSERT(hr == D3D_OK);
	}
	return TerrainBlock2::draw( pMaterial );
}


void EditorTerrainBlock2::setHeightMapDirty()
{
	BW_GUARD;
	pVerticesResource_->setDirty();
	rebuildNormalMap( NMQ_FAST );
}


void EditorTerrainBlock2::rebuildCombinedLayers(
	bool compressTextures /*= true*/, 
	bool generateDominantTextureMap /*= true */)
{
	BW_GUARD;
	ensureBlendsLoaded();

	DominantTextureMap2Ptr tempDominantMap = dominantTextureMap2();
	pBlendsResource_->rebuild
	(
		compressTextures, 
		generateDominantTextureMap ? &tempDominantMap : NULL
	);
	if (generateDominantTextureMap)
		dominantTextureMap2(tempDominantMap);

	lodMapDirty_ = true;
}


void EditorTerrainBlock2::rebuildNormalMap( NormalMapQuality normalMapQuality )
{
	BW_GUARD;
	pNormalMap_->generate( heightMap2(), normalMapQuality, 
		settings_->normalMapSize() );
}


bool EditorTerrainBlock2::rebuildLodTexture( const Matrix& transform )
{
	BW_GUARD;
	ensureBlendsLoaded();

	bool ok = false;

	if ( pLodMap_ == NULL )
	{
		pLodMap_ = new TerrainLodMap2();
	}

	// Save state of lodding
	uint32	savedStartLod		= settings_->topVertexLod();
	bool	savedConstantLod	= settings_->constantLod();
	bool	savedUseLodTexture	= settings_->useLodTexture();

	// Enable the correct renderer
	settings_->setActiveRenderer();

	// Enable constant min vertex lod, and disable texture lod ( because thats what
	// we are generating ).
	settings_->topVertexLod( settings_->numVertexLods() - 1 );
	settings_->constantLod( true );
	settings_->useLodTexture( false );

	// 'stream' must be called before rendering. The position passed are ignored
	// because we are setting constant LODing to the smallest LOD above.
	evaluate(Vector3::zero() );

	if (pBlendsResource_->getState() == RS_Unloaded)
	{
		// Combining the layers is done over a number of frames, so we need to
		// ensure the combined layers are loaded for the photographer to work,
		// so here we trick "stream" into thinking it should rebuild the blends
		s_lastBlendBuildTime = 0;
		s_lastBlendBuildMark = s_nextBlendBuildMark_ - 1;
	}
	stream();

	// now take photograph
	TerrainPhotographer& photographer = TerrainRenderer2::instance()->photographer();

	bool oldUseHoleMap = TerrainRenderer2::instance()->enableHoleMap( false );

	ComObjectWrap<DX::Texture> texture = pLodMap_->pTexture();

	// Record whether or not the texture existed beforehand.
	bool isNewTexture = !texture;
	
	Moo::rc().beginScene();
	if (photographer.init( 128 ) &&
		photographer.photographBlock( const_cast<EditorTerrainBlock2*>(this),
									transform ) &&
		photographer.output( texture, D3DFMT_DXT5 ))
	{
		// if its a new texture, add allocation marker and assign back to lod 
		// map, otherwise we edited the existing texture in-place.
		if ( isNewTexture ) 
		{
			pLodMap_->pTexture( texture );
		}

		if ( Moo::rc().device()->TestCooperativeLevel() == D3D_OK )
		{
			// If we didn't lose device during photo (eg from a CTRL+ALT+DEL)
			// then mark as ok.
			ok				= true;
			lodMapDirty_	= false;
		}
		else
		{
			// Remove possibly corrupt lod texture.
			pLodMap_ = NULL;
			ERROR_MSG( "EditorTerrainBlock2::rebuildLodTexture: "
				"Failed on terrain block %s, the DX device was lost.\n",
				this->resourceName().c_str() );
		}
	}
	else
	{
		ERROR_MSG( "EditorTerrainBlock2::rebuildLodTexture: "
			"Failed on terrain block %s, failed to photograph the terrain.\n",
			this->resourceName().c_str() );
	}
	Moo::rc().endScene();

	TerrainRenderer2::instance()->enableHoleMap( oldUseHoleMap );

	// reset lod states
	settings_->topVertexLod( savedStartLod );
	settings_->constantLod( savedConstantLod );
	settings_->useLodTexture( savedUseLodTexture );

	return ok;
}


void EditorTerrainBlock2::setLodTextureDirty( bool state )
{
	lodMapDirty_ = state;
}

void EditorTerrainBlock2::streamBlends()
{
	BW_GUARD;
	if 
	( 
		(lodRenderInfo_.renderTextureMask_ & TerrainRenderer2::RTM_DrawBlend) != 0
		||
		(lodRenderInfo_.renderTextureMask_ & TerrainRenderer2::RTM_PreLoadBlend) != 0			
	)	
	{
		ensureBlendsLoaded();
	}

	if (pBlendsResource_->getObject() != NULL)
	{
		// If we are drawing blends, or the lod map is dirty and so we are being
		// forced to draw them (lodMapDirty_ is true) then make sure that there 
		// are combined layers.
		// If the use of LOD textures is turned off then it's likely that the
		// combined layers don't exist for far away chunks.  If this happens
		// then we need to regenerate them.
		if 
		(
			(
				lodMapDirty_ 
				|| 
				(lodRenderInfo_.renderTextureMask_ & TerrainRenderer2::RTM_DrawBlend) != 0
				||
				!settings_->useLodTexture() 
			)
			&&
			pBlendsResource_->getState() == RS_Unloaded
		)
		{
			// Since rebuilding the combined layers is an expensive operation,
			// specially because it ends up calling ttl2Cache::onLock which
			// calls the expensive decompressImage, we only rebuild layers
			// every s_blendBuildInterval milliseconds.
			uint64 blendBuildTimeout = s_lastBlendBuildTime +
				s_blendBuildInterval_ * stampsPerSecond() / 1000;

			if (blendBuildTimeout < timestamp() &&
				s_lastBlendBuildMark != s_nextBlendBuildMark_)
			{
				// retain the value of lodMapDirty_
				bool wasLodDirty = lodMapDirty_; 
				rebuildCombinedLayers( true, false );
				lodMapDirty_ = wasLodDirty;

				// rebuild layers again in s_blendBuildInterval milliseconds
				// from now, but only if it's another frame.
				s_lastBlendBuildMark = s_nextBlendBuildMark_;
				s_lastBlendBuildTime = timestamp();
			}
		}

		// If we are drawing blends (or forced into drawing them) and not drawing
		// the lod texture then get rid of the blend texture to save precious
		// memory.  If we need it again then we regenerate it in the above if
		// statement.
		if 
		(
			!lodMapDirty_
			&&
			(lodRenderInfo_.renderTextureMask_ & TerrainRenderer2::RTM_DrawBlend) == 0
		)
		{
			pBlendsResource_->unload();
		}
	}	
}


void EditorTerrainBlock2::ensureBlendsLoaded() const
{
	BW_GUARD;
	if (pBlendsResource_->getObject() == NULL)
	{
		if (!pBlendsResource_->load())
		{
			ERROR_MSG( "Failed to load blends for terrain EditorTerrainBlock2 "
				" %s\n", fileName_.c_str() );
		}
	}
}


void EditorTerrainBlock2::stream()
{
	BW_GUARD;
	// Stream vertices
	pVerticesResource_->stream();

	// Stream blends if we are not in a reflection. We override it here rather 
	// than in visibility test to stop reflection scene from unloading blends
	// required by main scene render.
	if ( !Moo::rc().reflectionScene() )
	{
		streamBlends();
	}
}


bool EditorTerrainBlock2::canDrawLodTexture() const
{
	return TerrainBlock2::canDrawLodTexture() && !isLodTextureDirty();
}


/**
 *  This method increments the frame mark for rebuilding combined layers.
 */
/*static*/ void EditorTerrainBlock2::nextBlendBuildMark()
{
	s_nextBlendBuildMark_ = Moo::rc().frameTimestamp();
}


/**
 *  This method returns the interval to wait between calls to the
 *	rebuildCombinedLayers method, because it's quite expensive to update all
 *	layers for all blocks that need it at once.
 *
 *	@return Current interval in milliseconds between calls.
 */
/*static*/ uint32 EditorTerrainBlock2::blendBuildInterval()
{
	return s_blendBuildInterval_;
}


/**
 *  This method allows configuring the interval between calls to the
 *	rebuildCombinedLayers method, because it's quite expensive to update all
 *	layers for all blocks that need it at once.
 *
 *	@param interval Interval in milliseconds between calls.
 */
/*static*/ void EditorTerrainBlock2::blendBuildInterval( uint32 interval )
{
	s_blendBuildInterval_ = interval;
}


//#define DEBUG_HEIGHT_MAPS
#ifdef DEBUG_HEIGHT_MAPS

#include "moo/png.hpp"

void WriteHeightImagePNG( const TerrainHeightMap2::ImageType& dimg,
							float minHeight, float maxHeight, 
							uint32 fileCount, uint32 lodLevel )
{
	BW_GUARD;
	Moo::PNGImageData pngData;
	pngData.data_ = new uint8[dimg.width()*dimg.height()];
	pngData.width_ = dimg.width();
	pngData.height_ = dimg.height();
	pngData.bpp_ = 8;
	pngData.stride_ = dimg.width();
	pngData.upsideDown_ = false;
	uint8 *p = pngData.data_;

	for ( uint32 x = 0; x < dimg.width(); x++ )
	{
		for ( uint32 y = 0; y < dimg.height(); y++ )
		{
			float val  = dimg.get(x, y);
			uint8 grey = Math::lerp(val, minHeight, maxHeight, (uint8)0, (uint8)255);
			*p++ = grey; 
		}
	}
	BinaryPtr png = compressPNG(pngData);

	char filename[64]; 
	bw_snprintf( filename, 64,"e:/temp/%d_heights_%d.png",
		fileCount, lodLevel );
	FILE *file = BWResource::instance().fileSystem()->posixFileOpen( filename, "wb");
	fwrite(png->data(), png->len(), 1, file);
	fclose(file); file = NULL;
	delete[] pngData.data_; pngData.data_ = NULL;
}
#endif

bool EditorTerrainBlock2::saveHeightmap( DataSectionPtr dataSection ) const
{
	BW_GUARD;
	dataSection->deleteSections( "heights" );

	// save first lod of height map
	bool ok = heightMap().save( dataSection->newSection("heights", 
												BinSection::creator()) );

#ifdef DEBUG_HEIGHT_MAPS
	// debug
	static uint32 fileCount = 0;
#endif

	if ( ok )
	{
		// save each lod of height map
		for ( uint32 i = 1; i < settings_->numVertexLods(); i++ )
		{
			// Source size is visible part of main height map
			uint32				srcVisSize	= heightMap2()->verticesWidth();
			uint32				lodSize		= (srcVisSize >> i) + 1;

			// create a down sampled height map - we only sample the visible
			// part of the original height map.
			AliasedHeightMap	ahm( i, heightMap2() );
			TerrainHeightMap2	dhm( lodSize, i );

			dhm.lock( false );
			TerrainHeightMap2::ImageType& dimg = dhm.image();

			for ( uint32 x = 0; x < lodSize; x++ )
			{
				for ( uint32 y = 0; y < lodSize; y++ )
				{
					dimg.set( x, y, ahm.height(x,y) );		
				}
			}
			dhm.unlock();

			// save down sampled height map
			char sectionName[64];
			bw_snprintf( sectionName, ARRAY_SIZE(sectionName), "heights%d", i );
			ok = dhm.save( dataSection->openSection( sectionName, true,
												BinSection::creator() ) );
			
#ifdef DEBUG_HEIGHT_MAPS
			if ( i == 1 )
			{
				// write source map only once as lod 0
				WriteHeightImagePNG( simg, heightMap2()->minHeight(), 
					heightMap2()->maxHeight(), fileCount, 0 );
			}
			// write lod height map
			WriteHeightImagePNG(dimg, heightMap2()->minHeight(), 
				heightMap2()->maxHeight(), fileCount, i );
#endif
			if ( !ok ) break;
		}
	}

#ifdef DEBUG_HEIGHT_MAPS
	//debug
	fileCount++;
#endif

	return ok;
}


#endif // EDITOR_ENABLED
