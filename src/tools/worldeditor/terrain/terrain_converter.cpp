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
#include "worldeditor/terrain/terrain_converter.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "chunk/chunk_format.hpp"
#include "chunk/chunk_space.hpp"
#include "cstdmf/timestamp.hpp"
#include "moo/png.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/bin_section.hpp"
#include "resmgr/data_section_census.hpp"
#include "romp/progress.hpp"
#include "terrain/terrain2/aliased_height_map.hpp"
#include "terrain/terrain2/editor_terrain_block2.hpp"
#include "terrain/terrain2/editor_vertex_lod_manager.hpp"
#include "terrain/terrain2/terrain_block2.hpp"
#include "terrain/terrain2/terrain_height_map2.hpp"
#include "terrain/terrain2/terrain_index_buffer.hpp"
#include "terrain/terrain2/terrain_normal_map2.hpp"
#include "terrain/terrain2/terrain_photographer.hpp"
#include "terrain/terrain2/terrain_vertex_buffer.hpp"
#include "terrain/terrain_data.hpp"
#include "terrain/terrain_height_map.hpp"
#include "terrain/terrain_hole_map.hpp"
#include "terrain/terrain_texture_layer.hpp"
#include "terrain/height_map_compress.hpp"


const uint32 SRC_BLOCK_SIZE = 25;
const uint32 SRC_BLOCK_RES = SRC_BLOCK_SIZE + 1;
const uint32 SRC_BLOCK_STRIDE = SRC_BLOCK_RES + 2;
const float BLEND_VALUE_THRESHOLD =
	Terrain::EditorBaseTerrainBlock::BLENDS_MIN_VALUE_THRESHOLD / 255.0f;


// -----------------------------------------------------------------------------
// Section: HeightMapConverter
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
TerrainConverter::TerrainConverter()	:
	minX_( 0 ),
	maxX_( 0 ),
	minZ_( 0 ),
	maxZ_( 0 ),
	singleDir_( false )
{
}


/**
 *	Destructor.
 */
TerrainConverter::~TerrainConverter()
{
}

/**
 *	This method inits the converter
 *	@param pSpaceSection the datasection of the folder the space is in
 */
void TerrainConverter::init( const std::string& spacePath, DataSectionPtr pSpaceSection )
{
	BW_GUARD;

	// Read settings
	if (readSettings(pSpaceSection))
	{
		spacePath_ = spacePath;
		pSpaceSection_ = pSpaceSection;
	}
}

/*
 *	This method reads the space settings from the space.settings file
 */
bool TerrainConverter::readSettings( DataSectionPtr pSpaceSection )
{
	BW_GUARD;

	bool ret = false;

	DataSectionPtr pSettings = pSpaceSection->openSection( SPACE_SETTING_FILE_NAME );

	if (pSettings)
	{
		singleDir_ = pSettings->readBool( "singleDir", false );
		minX_ = pSettings->readInt( "bounds/minX" );
		maxX_ = pSettings->readInt( "bounds/maxX" );
		minZ_ = pSettings->readInt( "bounds/minY" );
		maxZ_ = pSettings->readInt( "bounds/maxY" );
		pTerrainSettings_ = new Terrain::TerrainSettings();
		pTerrainSettings_->init( pSettings->openSection( "terrain" ) );
		ret = true;
	}

	return ret;
}

namespace 
{

/*
 *	This function calculates the coefficients for a point on a catmul rom spline
 *	at time t.
 */
void catmulRomCoefficients( float t, float& a, float& b, float& c, float& d )
{
	float t2 = powf( t, 2.f );
	float t3 = powf( t, 3.f );
	a = (-t3 + 2.f * t2 - t) / 2.f;
	b = (3.f * t3 - 5.f * t2 + 2) / 2.f;
	c = (-3.f * t3 + 4.f * t2 + t) / 2.f;
	d = (t3 - t2) / 2.f;
}

uint32 positiveBX2( uint32 value )
{
	return value > 128 ? ((value - 128) << 1) : 0;
}

Vector4 expandBlendValue( uint32 value )
{
	Vector4 out( float(positiveBX2((value & 0xff) ) ),
		float(positiveBX2((value >> 8) & 0xff) ),
		float(positiveBX2((value >> 16) & 0xff)),
		float((value >> 24) & 0xff) );
	out *= 1.f / (out.x + out.y + out.z + out.w);
	return out;
}

}

/**
 *	This method converts the entire heightmap to a new heightmap size
 *	@param progress      optional progress callback
 *  @param reconvert     reconvert a space already converted
 */
void TerrainConverter::convertAll( ProgressTask* progress, bool reconvert )
{
	BW_GUARD;

	if (pSpaceSection_)
	{
		if ( progress )
		{
			// There are two iterations, and one iteration is much slower than
			// the other, so using 2/3 of the progress bar length for the slow
			// iteration and 1/3 for the fast one.
			progress->length(
				( maxZ_ - minZ_ + 1.0f ) * ( maxX_ - minX_ + 1.0f ) * 3 );
		}

		// Iterate over all chunks and create the heightmap
		for (int32 z = minZ_; z <= maxZ_; z++)
		{
			for (int32 x = minX_; x <= maxX_; x++)
			{
				convertSingle( x, z, reconvert );
				if ( progress )
					progress->step(2); // step twice because this is slow
			}
		}
		// Change "resource" tag in the "terrain" section in the chunk file
		// to "terrain2", and remove old "terrain" section from the cdata file.
		for (int32 z = minZ_; z <= maxZ_; z++)
		{
			for (int32 x = minX_; x <= maxX_; x++)
			{
				std::string chunkId = ChunkFormat::outsideChunkIdentifier( x, z, singleDir_ );

				std::string cFileName = chunkId + ".chunk";
				std::string cDataName = chunkId + ".cdata";

				// Update the chunk file's terrain resource name
				DataSectionPtr chunkDS = pSpaceSection_->openSection(cFileName);
				if ( chunkDS )
				{
					DataSectionPtr chunkTerrainDS = chunkDS->openSection( "terrain" );
					if ( chunkTerrainDS )
					{
						// found the old terrain resource section, update to new
						chunkTerrainDS->deleteSection( "resource" );
						chunkTerrainDS->writeString( "resource", cDataName + "/terrain2" );
						chunkDS->save();
					}
				}
				// Remove old terrain file from the CData file
				DataSectionPtr pCDataSection = pSpaceSection_->openSection( cDataName );
				if (pCDataSection)
				{
					pCDataSection->deleteSection( "terrain" );
					pCDataSection->save();
				}

				if ( progress )
					progress->step();
			}
		}
	}
}

/**
 *	This method converts a single terrain block
 *	@param x the x grid position of the terrain block
 *	@param z the z grid position of the terrain block
 *	@param the size of the heightmap to output
 *	@return true if successful
 */
bool TerrainConverter::convertSingle( int32 x, int32 z, bool reconvert )
{
	BW_GUARD;

	bool res = false;
	if (pSpaceSection_)
	{
		// Get the identifier of the chunk
		std::string chunkId = ChunkFormat::outsideChunkIdentifier( x, z, singleDir_ );

		std::string cFileName = chunkId + ".chunk";
		std::string cDataName = chunkId + ".cdata";
		std::string terrainName = "terrain2";
		std::string terrainSectionPath = cDataName + "/" + terrainName;

		if (!reconvert && pSpaceSection_->openSection(terrainSectionPath))
		{
			INFO_MSG( "HeightMapConverter::convert - skipping %s\n", cDataName.c_str() );
			return true;
		}

		// First we need to grab all the blocks needed
		// we need the 8 surrounding blocks so that we can get enough information in one
		// height block to calculate the normal map as well. The heightmap stores 2 extra
		// heights on each border for this purpose.
		BlockConverterPtr pSouthWest = blockConverter( x - 1, z - 1 );
		BlockConverterPtr pWest = blockConverter( x - 1, z );
		BlockConverterPtr pNorthWest = blockConverter( x - 1, z + 1 );
		BlockConverterPtr pSouth = blockConverter( x, z - 1 );
		BlockConverterPtr pBlock = blockConverter( x, z );
		BlockConverterPtr pNorth = blockConverter( x, z + 1 );
		BlockConverterPtr pSouthEast = blockConverter( x + 1, z - 1 );
		BlockConverterPtr pEast = blockConverter( x + 1, z );
		BlockConverterPtr pNorthEast = blockConverter( x + 1, z + 1 );

		if (pBlock.hasObject())
		{
			// Purge everything from the data section cache
			BWResource::instance().purgeAll();

			//get the cdata section
			DataSectionPtr pCDataSection = pSpaceSection_->openSection( cDataName );
			if (pCDataSection)
			{
				pCDataSection=pCDataSection->convertToZip();

				// Open the terrain2 section if we have one, otherwise create a new one
				DataSectionPtr pTerrainSection = pCDataSection->openSection( terrainName );
				if (!pTerrainSection)
					pTerrainSection = pCDataSection->newSection( terrainName );

				pTerrainSection->setParent(pCDataSection);
				pTerrainSection = pTerrainSection->convertToZip();

				// Make sure we start with an empty terrain section
				pTerrainSection->delChildren();

				INFO_MSG( "HeightMapConverter::convert - saving %s\n", cDataName.c_str() );

				// Init the heights
				pBlock->initSrcHeights( pSouthWest.getObject(), pSouth.getObject(),
					pSouthEast.getObject(), pWest.getObject(), pEast.getObject(), 
					pNorthWest.getObject(), pNorth.getObject(), pNorthEast.getObject() ); 

				// Convert all the data
				pBlock->convertHeights( pTerrainSection );
				pBlock->convertBlends( pTerrainSection );
				pBlock->convertShadows( pTerrainSection );
				pBlock->convertHoles( pTerrainSection );
				pBlock->convertLODs( pTerrainSection );
				pBlock->convertNormals( pTerrainSection );

				// Save it
				pCDataSection->save();
				pTerrainSection->setParent( NULL );

				// Now load the block, take photo and resave it
	 			Terrain::EditorTerrainBlock2* pTerrainBlock = 
					new Terrain::EditorTerrainBlock2(pTerrainSettings_);
				Matrix chunkTransform;
				chunkTransform.setTranslate( x * GRID_RESOLUTION, 0.0f, z * GRID_RESOLUTION );
				if (pTerrainBlock->load( spacePath_ + '/' + terrainSectionPath, chunkTransform, chunkTransform.applyToOrigin() ))
				{
					pTerrainBlock->rebuildCombinedLayers();
					pTerrainBlock->rebuildLodTexture( chunkTransform );

					EditorChunkCache::UpdateFlags flags( pCDataSection );
					flags.terrainLOD_ = !pTerrainBlock->isLodTextureDirty();
					flags.save();

					pTerrainBlock->save( spacePath_ + '/' + cDataName );

					delete pTerrainBlock;
				}
				
				res = true;
			}

		}
		else
		{
			ERROR_MSG( "HeightMapConverter::convert - unable to save %s\n", 
				cDataName.c_str() );
		}
	}
	return res;
}

/**
 *	This method wipes the terrain2 data from a group of chunks
 *	@param xStart the start on the x axis of our wipe window
 *	@param xEnd the end on the x axis of our wipe window
 *	@param zStart the start on the z axis of our wipe window
 *	@param zEnd the end on the z axis of our wipe window
 */
void TerrainConverter::wipeRect( int32 xStart, int32 xEnd, int32 zStart, int32 zEnd )
{
	BW_GUARD;

	if (pSpaceSection_)
	{
		// Iterate over our chunks and wipe the terrain2 section
		for (int32 z = zStart; z <= zEnd; z++)
		{
			for (int32 x = xStart; x <= xEnd; x++)
			{
				wipeSingle( x, z );
			}
		}
	}
}

/**
 *	This method wipes the terrain2 data from all chunks in a space
 */
void TerrainConverter::wipeAll()
{
	BW_GUARD;

	if (pSpaceSection_)
	{
		// Iterate over all our chunks and wipe the terrain2 section
		for (int32 z = minZ_; z <= maxZ_; z++)
		{
			for (int32 x = minX_; x <= maxX_; x++)
			{
				wipeSingle( x, z );
			}
		}
	}
}

/**
 *	This method wipes the terrain2 data from a chunk
 */
void TerrainConverter::wipeSingle( int32 x, int32 z )
{
	BW_GUARD;

	if ( x >= minX_ && x <= maxX_ && z >= minZ_ && z <= maxZ_ 
		&& pSpaceSection_)
	{
		// Get the chunk id
		std::string chunkID = ChunkFormat::outsideChunkIdentifier( x, z, singleDir_ );

		// Purge everything from the data section cache
		BWResource::instance().purgeAll();

		// Open the cdata section
		DataSectionPtr pCDataSection = pSpaceSection_->openSection( chunkID + ".cdata" );
		if (pCDataSection)
		{
			// Wipe the terrain2 section if we have one
			DataSectionPtr pTerrainSection = pCDataSection->openSection( "terrain2" );
			if (pTerrainSection)
			{
				pTerrainSection = NULL;
				pCDataSection->delChild("terrain2");
				INFO_MSG( "HeightMapConverter::wipeSingle - saving %s\n", chunkID.c_str() );
				pCDataSection->save();
			}
		}
	}
}

const uint32 LRU_SIZE = 30;


/*
 *	This method handles the least recently used cache for block converters
 *	@param id the identifier of the block
 *	@param pBlock the block
 */
void TerrainConverter::blockLRU( std::string id, BlockConverterPtr pBlock )
{
	BW_GUARD;

	// If our block is in the lru list erase it
	LRUBlocks::iterator it = std::find( lruBlocks_.begin(), lruBlocks_.end(), id );
	if (it != lruBlocks_.end())
	{
		lruBlocks_.erase( it );
	}
	// Add the block to the lru list
	lruBlocks_.push_back( id );
	cachedBlocks_[id] = pBlock;

	// Remove blocks from the lru list if we have exceeded the lru size
	while (lruBlocks_.size() > LRU_SIZE)
	{
		BlockMap::iterator it = cachedBlocks_.find( lruBlocks_.front() );
		if (it != cachedBlocks_.end())
		{
			cachedBlocks_.erase( it );
		}
		lruBlocks_.pop_front();
	}
}

/*
 *	This method handles the creation and caching of block converters
 *	@param x the x grid position of the terrain block
 *	@param z the z grid position of the terrain block
 *	@return the BlockConverter for the specified grid position
 */
BlockConverterPtr TerrainConverter::blockConverter( int32 x, int32 z )
{
	BW_GUARD;

	BlockConverterPtr pBlock;
	if ( x >= minX_ && x <= maxX_ && z >= minZ_ && z <= maxZ_ )
	{
		// Get the chunk identifier
		std::string chunkID = ChunkFormat::outsideChunkIdentifier( x, z, singleDir_ );
		
		// See if the chunk is cached		
		BlockMap::iterator it = cachedBlocks_.find( chunkID );
		if (it == cachedBlocks_.end())
		{
			// Get the identifier for the terrain section
			std::string terrainID = pSpaceSection_->readString( chunkID + ".chunk/terrain/resource" );
			if (terrainID.length())
			{
				// Open the terrain section
				DataSectionPtr pTerrainSection = pSpaceSection_->openSection( terrainID );
				if (pTerrainSection)
				{
					// Create the BlockConverter init it and add it to the cache
					pBlock = new BlockConverter;
					pBlock->init( pTerrainSection, pTerrainSettings_ );
					blockLRU( chunkID, pBlock );
				}
			}
		}
		else
		{
			pBlock = it->second;
			blockLRU( chunkID, pBlock );
		}
	}
	return pBlock;
}

/**
 *	This method inits the BlockConverter
 */
void BlockConverter::init( DataSectionPtr pTerrainSection, Terrain::TerrainSettingsPtr pSettings )
{
	BW_GUARD;

	// Save the terrain settings
	pTerrainSettings_ = pSettings;

	// Open the binary resource of the terrain block.
	BinaryPtr pTerrainData =
		pTerrainSection->asBinary();

	if (!pTerrainData) return;

	// Read the header and calculate statistics for the terrain block.
	const Terrain::TerrainBlock1Header* pBlockHeader =
		reinterpret_cast<const Terrain::TerrainBlock1Header*>(pTerrainData->data());
	spacing_      = pBlockHeader->spacing_;
	width_        = pBlockHeader->heightMapWidth_;
	height_       = pBlockHeader->heightMapHeight_;
	detailWidth_  = pBlockHeader->detailWidth_;
	detailHeight_ = pBlockHeader->detailHeight_;

	blocksWidth_    = width_ - 3;
	blocksHeight_   = height_ - 3;
	verticesWidth_  = width_ - 2;
	verticesHeight_ = height_ - 2;

	// Load the texture names.
	const char* pTextureNames =
			reinterpret_cast< const char* >( pBlockHeader + 1 );
	uint32 nHeightValues =
			pBlockHeader->heightMapWidth_ * pBlockHeader->heightMapHeight_;

	for (uint32 i = 0; i < pBlockHeader->nTextures_; i++)
	{
		textures_.push_back(pTextureNames);
		pTextureNames += pBlockHeader->textureNameSize_;
	}


	// Load the height values .
	const float* pHeights = reinterpret_cast< const float* >( pTextureNames );
	heights_.assign( pHeights, pHeights + nHeightValues );

	// Load the blend values.
	const uint32* pBlends =
			reinterpret_cast< const uint32* >( pHeights + nHeightValues );
	blendValues_.resize( nHeightValues );
	std::vector< Vector4 >::iterator it = blendValues_.begin();
	std::vector< Vector4 >::iterator end = blendValues_.end();
	const uint32* pValues = pBlends;
	while (it != end)
	{
		*(it++) = expandBlendValue( *(pValues++) );
	}

	// Load the shadow values.
	const uint16* pShadows =
			reinterpret_cast< const uint16* >( pBlends + nHeightValues );
	shadowValues_.assign( pShadows, pShadows + nHeightValues );

	// load the terrain block holes.
	const bool* pHoles =
			reinterpret_cast< const bool* >( pShadows + nHeightValues );

	uint32 nTerrainSquares = blocksWidth_ * blocksHeight_;
	holes_.assign( pHoles, pHoles + nTerrainSquares );

}

/**
 *	This method prepares the heights for interpolation, as we want to save out
 *	the heights with a border of 2 extra heights, we need the neighbouring
 *	block heights as well.
 *	The 2 height border is necessary for normal calculation, as the normals 
 *	may be at a higher frequency than the vertices and for interpolation we
 *	need a 1 height border for catmul-rom interpolation.
 *	@param pSouthWest the terrain block at (-1,-1)
 *	@param pSouth the terrain block at ( 0,-1)
 *	@param pSouthEast the terrain block at ( 1,-1)
 *	@param pWest the terrain block at (-1, 0)
 *	@param pEast the terrain block at ( 1, 0)
 *	@param pNorthWest the terrain block at (-1, 1)
 *	@param pNorth the terrain block at ( 0, 1)
 *	@param pNorthEast the terrain block at ( 1, 1)
 */
void BlockConverter::initSrcHeights( BlockConverter* pSouthWest, BlockConverter* pSouth,
		 BlockConverter* pSouthEast, BlockConverter* pWest,
		 BlockConverter* pEast, BlockConverter* pNorthWest,
		 BlockConverter* pNorth, BlockConverter* pNorthEast )
{
	BW_GUARD;

	// The source heights includes heights from some neighbouring chunks
	srcStride_ = width_ + 8;
	srcHeight_ = height_ + 8;

	srcHeights_.resize( srcStride_ * srcHeight_, 99999999.f );

	// Do the middle of the block and extend to the edges so that we have a border
	// with semi-sensible values
	for (int32 z = -4; z < int32(height_ + 4); z++)
	{
		
		uint32 srcOffset = width_ * std::min( std::max(z, int32(0)), int32(height_) - 1);
		uint32 destOffset = srcStride_ * (z + 4);
		
		// Copy the row
		memcpy( &srcHeights_[destOffset + 4], &heights_[ srcOffset ], 
			width_ * sizeof(float) );
		
		// Extend row to east west edges
		srcHeights_[destOffset] = heights_[srcOffset];
		srcHeights_[destOffset+1] = heights_[srcOffset];
		srcHeights_[destOffset+2] = heights_[srcOffset];
		srcHeights_[destOffset+3] = heights_[srcOffset];
		srcHeights_[destOffset + srcStride_ - 4] = heights_[srcOffset + width_ - 1];
		srcHeights_[destOffset + srcStride_ - 3] = heights_[srcOffset + width_ - 1];
		srcHeights_[destOffset + srcStride_ - 2] = heights_[srcOffset + width_ - 1];
		srcHeights_[destOffset + srcStride_ - 1] = heights_[srcOffset + width_ - 1];
	}

	// copy the borders if we have the border blocks
	if (pSouthWest)
	{
		this->copyHeights( pSouthWest, width_ - 7, height_ - 7, 
			0, 0, 5, 5 );
	}

	if (pSouthEast)
	{
		this->copyHeights( pSouthEast, 1, height_ - 7, 
			srcStride_ - 6, 0, 6, 5 );
	}

	if (pNorthWest)
	{
		this->copyHeights( pNorthWest, width_ - 7, 1, 
			0, srcHeight_ - 6, 5, 6 );
	}

	if (pNorthEast)
	{
		this->copyHeights( pNorthEast, 1, 1, 
			srcStride_ - 6, srcHeight_ - 6, 6, 6 );
	}

	if (pSouth)
	{
		this->copyHeights( pSouth, 1, height_ - 7, 
			5, 0, width_ - 3, 5 );
	}
	
	if (pNorth)
	{
		this->copyHeights( pNorth, 1, 1, 
			5, srcHeight_ - 6, width_ - 3, 6 );
	}

	if (pWest)
	{
		this->copyHeights( pWest, width_ - 7, 1,
			0, 5, 5, height_ - 3 );
	}

	if (pEast)
	{
		this->copyHeights( pEast, 1, 1, 
			srcStride_ - 6, 5, 6, height_ - 3 );
	}
}

void BlockConverter::copyHeights(
		 BlockConverterPtr pSrcBlock, uint32 srcX, uint32 srcZ, uint32 x, uint32 z,
		 uint32 width, uint32 height )
{
	BW_GUARD;

	uint32 srcOffset = srcZ * pSrcBlock->width_ + srcX;
	uint32 offset = z * srcStride_ + x;
	for (uint32 zi = 0; zi < height; zi++)
	{
		for (uint32 xi = 0; xi < width; xi++)
		{
			srcHeights_[ offset + xi ] = pSrcBlock->heights_[ srcOffset + xi ];
		}
		srcOffset += pSrcBlock->width_;
		offset += srcStride_;
	}
}


/**
 *	This method outputs the heights for a terrain block. It outputs the heights
 *	for the vertices with a 2 height border on all sides so that higher res normals
 *	can be generated from the heights. Heights are up-scaled by catmul-rom 
 *	spline interpolation.
 *	@param pParentSection the parent datasection
 */
void BlockConverter::convertHeights( DataSectionPtr pParentSection )
{
	BW_GUARD;

	uint32 outWidth = pTerrainSettings_->heightMapSize() + 1;
	uint32 outHeight = pTerrainSettings_->heightMapSize() + 1;
	//
	// Upscale old heights into a float image
	//
	Moo::Image< float > upscaledHeights( outWidth + 4, outHeight + 4 );
	float* pUpscaledHeights = upscaledHeights.getRow(0);

	// The proportion of out values to in values
	double zi = double( verticesHeight_ - 1 ) / double( outHeight - 1 );
	double xi = double( verticesWidth_ - 1 ) / double( outWidth - 1 );

	float upscaledMinHeight = srcHeights_[2 * srcStride_ + 2];
	float upscaledMaxHeight = upscaledMinHeight;

	// Iterate over every out vertex
	for (int32 z = -2; z < int32(outHeight + 2); z++)
	{
		float zPos = float(double(z) * zi);
		uint32 zOffset = uint32(floorf( zPos ) + 5.f);

		MF_ASSERT( zOffset >= 1 );
		zOffset -= 1;

		float zFrac = zPos - floorf( zPos );

		// Calculate vertical coefficients, these are constant for the entire
		// row
		Vector4 vCoeff;
		catmulRomCoefficients( zFrac, vCoeff.x, vCoeff.y, vCoeff.z, vCoeff.w );

		for (int32 x = -2; x < int32(outWidth + 2); x++)
		{
			float xPos = float(double(x) * xi);
			uint32 xOffset = uint32(floorf( xPos ) + 5.f);

			MF_ASSERT( xOffset >= 1 );
			xOffset -= 1;

			float xFrac = xPos - floorf( xPos );

			// Calculate horizontal coefficients
			Vector4 hCoeff;
			catmulRomCoefficients( xFrac, hCoeff.x, hCoeff.y, hCoeff.z, hCoeff.w );
			
			uint32 offset = zOffset * srcStride_ + xOffset;
			Vector4 horizontal(0,0,0,0);

			// Do the spline interpolation for the vertical direction
			// by grabbing a row of four vertices and multiplying by the
			// vertical coefficients
			for (uint32 i = 0; i < 4; i++)
			{
				Vector4 hv
				(
					srcHeights_[offset    ], srcHeights_[offset + 1], 
					srcHeights_[offset + 2], srcHeights_[offset + 3]
				);
				// The following four lines were unrolled from the code:
				//    horizontal += vCoeff[i]*hv;
				// for which the compiler produced incorrect SEE code.
				horizontal.x = horizontal.x + vCoeff[i]*hv.x;
				horizontal.y = horizontal.y + vCoeff[i]*hv.y;
				horizontal.z = horizontal.z + vCoeff[i]*hv.z;
				horizontal.w = horizontal.w + vCoeff[i]*hv.w;
				offset += srcStride_;
			}

			float h = hCoeff.x * horizontal.x + 
				hCoeff.y * horizontal.y + 
				hCoeff.z * horizontal.z + 
				hCoeff.w * horizontal.w;

			// Adjust min and max
			upscaledMinHeight	= std::min( upscaledMinHeight, h );
			upscaledMaxHeight	= std::max( upscaledMaxHeight, h );

			*(pUpscaledHeights++) = h;
		}
	}

	// compress 
	BinaryPtr compressedHeights = Terrain::compressHeightMap(upscaledHeights);

	// Storage for the height map data.
	// The size of the data is the size of the header + size of heights
	std::vector<uint8> data;
	data.resize( sizeof(Terrain::HeightMapHeader) + compressedHeights->len(), 0 );

	Terrain::HeightMapHeader* hmh= (Terrain::HeightMapHeader*)(&data.front());
	hmh->magic_			= Terrain::HeightMapHeader::MAGIC;
	hmh->compression_	= Terrain::COMPRESS_RAW;
	hmh->width_			= outWidth + 4;
	hmh->height_		= outHeight + 4;
	hmh->minHeight_		= upscaledMinHeight;
	hmh->maxHeight_		= upscaledMaxHeight;
	hmh->version_		= Terrain::HeightMapHeader::VERSION_ABS_QFLOAT;

	// copy compressed into output buffer
	memcpy( hmh + 1, compressedHeights->data(), compressedHeights->len() );

	DataSectionPtr pSection = pParentSection->openSection( "heights", false );
	BinaryPtr binaryBlock = 
		new BinaryBlock(&data.front(), data.size(), "BinaryBlock/BlockConverter" );
	if ( !pSection )
		pSection = pParentSection->newSection("heights");

	pSection->setBinary( binaryBlock );
}


/**
 *	This method converts the texture blend values. It rescales the values
 *	by linearly interpolating between neighbouring source blend values
 */
void BlockConverter::convertBlends( DataSectionPtr pParentSection )
{
	BW_GUARD;

	uint32 outWidth = pTerrainSettings_->blendMapSize();
	uint32 outHeight = pTerrainSettings_->blendMapSize();

	std::vector<Vector4> convertedBlends;
	convertedBlends.resize( outWidth * outHeight );

	float zi = float( verticesHeight_ - 1 ) / float( outHeight - 1 );
	float xi = float( verticesWidth_ - 1 ) / float( outWidth - 1 );

	std::vector<Vector4>::iterator itOut = convertedBlends.begin();

	// Iterate over the output heights row by row
	for (uint32 z = 0; z < outHeight; z++)
	{
		float zPos = float(z) * zi;
		uint32 zOffset = uint32(floorf( zPos ) + 1.f);

		float zFrac = zPos - floorf( zPos );

		for (uint32 x = 0; x < outWidth; x++)
		{
			float xPos = float(x) * xi;
			uint32 xOffset = uint32(floorf( xPos ) + 1.f);

			float xFrac = xPos - floorf( xPos );

			uint32 offset = zOffset * width_ + xOffset;

			const Vector4& p00 = blendValues_[offset];
			const Vector4& p10 = blendValues_[offset + 1];
			const Vector4& p01 = blendValues_[offset + width_];
			const Vector4& p11 = blendValues_[offset + width_ + 1];

			// The following code was unrolled from the commented out
			// lines of code.  This was because the compiler was producing
			// incorrect SSE code.
			float t1 = (1.f - xFrac) * (1.f - zFrac);
			float t2 = xFrac * (1.f - zFrac);
			float t3 = (1.f - xFrac) * zFrac;
			float t4 = xFrac * zFrac;
 			itOut->x = t1*p00.x + t2*p10.x + t3*p01.x + t4*p11.x;
			itOut->y = t1*p00.y + t2*p10.y + t3*p01.y + t4*p11.y;
			itOut->z = t1*p00.z + t2*p10.z + t3*p01.z + t4*p11.z;
			itOut->w = t1*p00.w + t2*p10.w + t3*p01.w + t4*p11.w;
			++itOut;

			//float scale = 1.f;// = float(rand()) / float(RAND_MAX);
			//*(itOut++) = (p00 * (1.f - xFrac) * (1.f - zFrac) +
			//	p10 * xFrac * (1.f - zFrac) +
			//	p01 * (1.f - xFrac) * zFrac +
			//	p11 * xFrac * zFrac) * scale;
		}
	}

	// Remove layers that have a very small contribution
	size_t validTex = textures_.size();
	for (size_t i = 0; i < textures_.size() && validTex == textures_.size(); i++ )
	{
		// first, find a texture that contributes significantly
		for (uint32 p = 0; p < convertedBlends.size(); ++p)
		{
			if ( convertedBlends[p][i] >= BLEND_VALUE_THRESHOLD )
			{
				validTex = i;
				break;
			}
		}
	}
	if ( validTex < textures_.size() )
	{
		// now iterate over the textures, and if a texture doesn't provide
		// enough contribution, then merge it to validTex
		for (size_t i = 0; i < textures_.size(); i++ )
		{
			bool keep = false;
			for (uint32 p = 0; p < convertedBlends.size(); ++p)
			{
				if ( convertedBlends[p][i] >= BLEND_VALUE_THRESHOLD )
				{
					keep = true;
					break;
				}
			}

			if ( !keep )
			{	
				// copy the blends to the last valid, non-empty layer, and set i's
				// values to 0 so it's skipped when saving.
				for (uint32 p = 0; p < convertedBlends.size(); ++p)
				{
					convertedBlends[p][validTex] = 
						std::min( 1.0f, convertedBlends[p][validTex] + convertedBlends[p][i] );
					convertedBlends[p][i] = 0;
				}
			}
		}
	}

	// Merge blends of layers with the same texture into one texture
	for (size_t i = 0; i < textures_.size() - 1; i++ )
	{
		for (size_t j = i + 1; j < textures_.size(); j++ )
		{
			if ( textures_[j] == textures_[i] )
			{	
				// j is the same as i, so blend j's values into i and set j's
				// values to 0 so it's skipped in the next step while saving.
				for (uint32 p = 0; p < convertedBlends.size(); ++p)
				{
					convertedBlends[p][i] = 
						std::min( 1.0f, convertedBlends[p][i] + convertedBlends[p][j] );
					convertedBlends[p][j] = 0;
				}
			}
		}
	}

	// Iterate over the texture layers (max 4)
	for (uint32 i = 0; i < textures_.size(); i++)
	{
		// Convert the blends to an image:
		std::vector<uint8> imgBlends(outWidth*outHeight);
		uint8 *pBlendValue = &imgBlends.front();				

		bool save = false;
		for
		(
			std::vector<Vector4>::iterator it = convertedBlends.begin();
			it != convertedBlends.end();
			++it, ++pBlendValue
		)
		{
			*pBlendValue = uint8((*it)[i] * 255.9f);
			if (*pBlendValue != 0)
				save = true;
		}

		if (save)
		{
			// Compress the image to PNG format:
			Moo::PNGImageData pngData;
			pngData.data_   = &imgBlends.front();
			pngData.width_  = pngData.stride_ = outWidth;
			pngData.height_ = outHeight;
			pngData.bpp_    = 8;
			BinaryPtr compData = compressPNG(pngData);

			// Get the texture name and pad it to 4 bytes:	
			std::string texName = textures_[i];
			texName.append( (4 - texName.length()) & 3, '\0' );

			std::vector<uint8> data;
			// The size of the data is the size of the header, the texture
			// name (including a length) and the size of the compressed image:
			data.resize
			( 
				sizeof(Terrain::BlendHeader) + texName.length() + sizeof(uint32) 
					+ compData->len(), 
				0 
			);
			
			Terrain::BlendHeader* bh = (Terrain::BlendHeader*)&data.front();
			bh->magic_ = Terrain::BlendHeader::MAGIC;
			bh->width_ = outWidth;
			bh->height_ = outHeight;
			bh->version_ = Terrain::BlendHeader::VERSION_PNG_BLENDS;
			bh->bpp_ = 8;
			Terrain::TerrainTextureLayer::defaultUVProjections(bh->uProjection_, bh->vProjection_);

			uint32* pLength = (uint32*)(bh + 1);
			*pLength = texName.length();
			char* pName = (char*)(pLength + 1);
			memcpy( pName, texName.data(), texName.length() );

			uint8* compImgData = (uint8*)(pName + texName.length());
			::memcpy(compImgData, compData->data(), compData->len());

			DataSectionPtr pSection = pParentSection->newSection( "layer" );
			BinaryPtr binaryBlock = 
				new BinaryBlock(&data.front(), data.size(), "BinaryBlock/BlockConverter" );
			pSection->setBinary( binaryBlock );
		}
		else
		{
			INFO_MSG( "BlockConverter::convertBlends - not saving layer %d\n", i );
		}
	}
}


namespace // Anonymous
{
	Vector2 unpackShadowValue( uint16 sourceVal )
	{
		return Vector2( float(sourceVal &0xff) / 255.f,
			float( sourceVal >> 8 ) / 255.f );
	}
	uint32 repackShadowValue( const Vector2& val )
	{
		Vector2 v = val;
		v.x = std::max( 0.f, std::min( 1.f, v.x ) );
		v.y = std::max( 0.f, std::min( 1.f, v.y ) );
		return ( uint32( v.y * 65535.5f ) |
			(uint32( v.x * 65535.f ) << 16) );
	}
}

/**
 *	This method converts the horizon shadow values. It rescales the values
 *	by linearly interpolating between neighbouring source shadow values.
 */
void BlockConverter::convertShadows( DataSectionPtr pParentSection )
{
	BW_GUARD;

	uint32 outWidth = pTerrainSettings_->shadowMapSize();
	uint32 outHeight = pTerrainSettings_->shadowMapSize();

	std::vector<uint8> data;
	data.resize( sizeof( Terrain::ShadowHeader ) + outWidth * outHeight * sizeof(uint32) );

	Terrain::ShadowHeader* pSH = (Terrain::ShadowHeader*)&data.front();
	pSH->magic_ = Terrain::ShadowHeader::MAGIC;
	pSH->width_ = outWidth;
	pSH->height_ = outHeight;
	pSH->bpp_ = 32;
	pSH->version_ = 1;

	uint32* itOut = (uint32*)(pSH + 1);

	float zi = float( verticesHeight_ - 1 ) / float( outHeight - 1 );
	float xi = float( verticesWidth_ - 1 ) / float( outWidth - 1 );

	// Iterate over the output heights row by row
	for (uint32 z = 0; z < outHeight; z++)
	{
		float zPos = float(z) * zi;
		uint32 zOffset = uint32(floorf( zPos ) + 1.f);

		float zFrac = zPos - floorf( zPos );

		for (uint32 x = 0; x < outWidth; x++)
		{
			float xPos = float(x) * xi;
			uint32 xOffset = uint32(floorf( xPos ) + 1.f);

			float xFrac = xPos - floorf( xPos );

			uint32 offset = zOffset * width_ + xOffset;

			const Vector2 p00 = unpackShadowValue( shadowValues_[offset] );
			const Vector2 p10 = unpackShadowValue( shadowValues_[offset + 1] );
			const Vector2 p01 = unpackShadowValue( shadowValues_[offset + width_] );
			const Vector2 p11 = unpackShadowValue( shadowValues_[offset + width_ + 1] );

			*(itOut++) = repackShadowValue( p00 * (1.f - xFrac) * (1.f - zFrac) +
				p10 * xFrac * (1.f - zFrac) +
				p01 * (1.f - xFrac) * zFrac +
				p11 * xFrac * zFrac );
		}
	}

	DataSectionPtr pSection = pParentSection->openSection( "horizonShadows", false );
	if ( !pSection )
		pSection = pParentSection->newSection( "horizonShadows" );

	BinaryPtr binaryBlock = 
		new BinaryBlock(&data.front(), data.size(), "BinaryBlock/BlockConverter" );
	pSection->setBinary( binaryBlock->compress() );

}

void BlockConverter::convertHoles( DataSectionPtr pParentSection )
{
	BW_GUARD;

	uint32 outWidth = pTerrainSettings_->holeMapSize();
	uint32 outHeight = pTerrainSettings_->holeMapSize();

	uint32 compressedWidth = ( outWidth / 8 ) + 
		((outWidth & 3) ? 1 : 0);
	std::vector<uint8> data;
	data.resize( sizeof(Terrain::HolesHeader) + compressedWidth * outHeight, 0);
	data.resize( data.size() + ((4 - data.size()) & 3), 0 );

	Terrain::HolesHeader* pHeader = (Terrain::HolesHeader*)&data.front();
	pHeader->magic_ = Terrain::HolesHeader::MAGIC;
	pHeader->width_ = outWidth;
	pHeader->height_ = outHeight;
	pHeader->version_ = 1;

	uint8* pHolesBase = (uint8*)(pHeader + 1);

	// create an image with the old holes for convenient access
	Terrain::TerrainHoleMap::ImageType oldHoles( blocksWidth_, blocksHeight_ );
	std::vector<bool>::iterator it = holes_.begin();
	for ( uint32 y = 0; y < blocksHeight_; y++ )
	{
		for ( uint32 x = 0; x < blocksWidth_; x++ )
		{
			oldHoles.set( x, y, *it++ );
		}
	}

	// convert to the new holes map
	bool hasHoles = false;
	for (uint32 z = 0; z < outHeight; z++)
	{
		uint8 value = 0x1;
		uint8* pHole = pHolesBase + z * compressedWidth;
		for (uint32 x = 0; x < outWidth; x++)
		{
			if (value == 0)
			{
				value = 1;
				pHole++;
			}
			
			if ( oldHoles.get(
					x * blocksWidth_ / outWidth,
					z * blocksHeight_ / outHeight ) )
			{
				*pHole |= value;
				hasHoles = true;
			}

			value <<= 1;
		}
	}
	
	if (hasHoles)
	{
		DataSectionPtr pSection = pParentSection->openSection( "holes", false );
		if ( !pSection )
			pSection = pParentSection->newSection( "holes" );

		BinaryPtr binaryBlock = 
			new BinaryBlock(&data.front(), data.size(), "BinaryBlock/BlockConverter" );
		pSection->setBinary( binaryBlock->compress() );
		INFO_MSG( "BlockConverter::convertHoles - saving holes\n" );
	}
}

void BlockConverter::convertLODs( DataSectionPtr pTerrainSection )
{
	BW_GUARD;

	// Get heights section from terrain section
	DataSectionPtr pHeights = pTerrainSection->openSection("heights");
	MF_ASSERT( pHeights );

	Terrain::TerrainHeightMap2Ptr thm2 = new Terrain::TerrainHeightMap2();
    thm2->load(pHeights);
	
	Terrain::EditorVertexLodManager::save( pTerrainSection, thm2 );
}

void BlockConverter::convertNormals( DataSectionPtr pTerrainSection )
{
	BW_GUARD;

	// Get heights section from terrain section 
	DataSectionPtr pHeights = pTerrainSection->openSection("heights");
	MF_ASSERT( pHeights );

	Terrain::TerrainHeightMap2Ptr thm2 = new Terrain::TerrainHeightMap2();
	thm2->load(pHeights);

	// generate normals.
	Terrain::TerrainNormalMap2Ptr pNormals = new Terrain::TerrainNormalMap2;
	pNormals->generate( thm2, Terrain::NMQ_NICE, pTerrainSettings_->normalMapSize() );

	// save normals
	pNormals->save( pTerrainSection );
}
