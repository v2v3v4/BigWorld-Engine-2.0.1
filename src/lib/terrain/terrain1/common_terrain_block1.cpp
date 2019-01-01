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
#include "common_terrain_block1.hpp"

#include "terrain_height_map1.hpp"
#include "terrain_hole_map1.hpp"
#include "../dominant_texture_map.hpp"
#include "terrain_texture_layer1.hpp"

#include "cstdmf/debug.hpp"
#include "resmgr/bwresource.hpp"


using namespace Terrain;


/**
 *	Constructor.
 */
CommonTerrainBlock1::CommonTerrainBlock1() :
	CommonTerrainBlock1Base(),
	numMapElements_(0),
	numTextures_(0),
	textureNameSize_(0),
    pHeightMap_( NULL ),
	pHoleMap_( NULL ),
    pDominantTextureMap_( NULL ),
    bb_( BoundingBox::s_insideOut_ )
{
}

/**
 *	Destructor.
 */
CommonTerrainBlock1::~CommonTerrainBlock1()
{
}


/**
 *  This function loads an BaseTerrainBlock from a DataSection.
 *
 *  @param filename         A file to load the terrain block from.
 *  @param worldPosition    The position in the world of this terrain block.
 *  @param cameraPosition   The position in the world of the camera (for
 *							initial LoD calculations).
 *  @param error            If not null and there was an error loading
 *                          the terrain then this will be set to a
 *                          description of the problem.
 *
 *  @returns                True if the load was completely successful,
 *                          false if a problem occured.
 */
/*virtual*/ bool
CommonTerrainBlock1::load(std::string const &filename,
					Matrix  const &worldTransform,
					Vector3 const &cameraPosition,
					std::string *error/* = NULL*/)
{
	BW_GUARD;
	// Open the binary resource of the terrain block.
	DataSectionPtr pTerrainDS =
			BWResource::instance().openSection( filename );

	if (!pTerrainDS)
	{
		std::stringstream msg;
			msg << "CommonTerrainBlock1::load: "; 
			msg << "Couldn't open file " << filename;
		if ( error )
			*error = msg.str();
		else
			ERROR_MSG( "%s\n", msg.str().c_str() );
		return false;
	}

	BinaryPtr pTerrainData = pTerrainDS->asBinary();

	if (!pTerrainData)
	{
		std::stringstream msg;
			msg << "CommonTerrainBlock1::load: "; 
			msg << "Couldn't get file " << filename << " as binary";
		if ( error )
			*error = msg.str();
		else
			ERROR_MSG( "%s\n", msg.str().c_str() );
		return false;
	}

	// Read the header and calculate statistics for the terrain block.
	TerrainBlockHeader* pBlockHeader = (TerrainBlockHeader*)(pTerrainData->data());

	if ( pBlockHeader->nTextures_ != 4 )
	{
		std::stringstream msg;
			msg << "CommonTerrainBlock1::load: "; 
			msg << "Terrain block " << filename << " is legacy terrain, ";
			msg << "but doesn't have 4 textures (has ";
			msg << pBlockHeader->nTextures_ << " textures)";
		if ( error )
			*error = msg.str();
		else
			ERROR_MSG( "%s\n", msg.str().c_str() );
		return false;
	}

	spacing_      = pBlockHeader->spacing_;
	width_        = pBlockHeader->heightMapWidth_;
	height_       = pBlockHeader->heightMapHeight_;
	detailWidth_  = pBlockHeader->detailWidth_;
	detailHeight_ = pBlockHeader->detailHeight_;

	blocksWidth_    = width_ - 3;
	blocksHeight_   = height_ - 3;
	verticesWidth_  = width_ - 2;
	verticesHeight_ = height_ - 2;

	// set pointers to the different data
	numMapElements_ =
		pBlockHeader->heightMapWidth_ * pBlockHeader->heightMapHeight_;

	textureNameSize_ = pBlockHeader->textureNameSize_;
	numTextures_ = pBlockHeader->nTextures_;

	// Load the texture layers and blends
	TextureLayers textureLayers;
	for (uint32 i = 0; i < pBlockHeader->nTextures_; i++)
	{
		TerrainTextureLayerPtr pLayer = new TerrainTextureLayer1(*this, i);
		bool loaded = pLayer->load( pTerrainDS, error );
		if ( loaded )
		{
			textureLayers.push_back( pLayer );
		}
		else
		{
			std::stringstream msg;
				msg << "CommonTerrainBlock1::load: "; 
				msg << "Couldn't load texture #" << i;
				msg << " for " << filename;
			if ( error )
				*error = msg.str();
			else
				ERROR_MSG( "%s\n", msg.str().c_str() );
			return false;
		}
	}

	// Load the heights
	pHeightMap_ = new TerrainHeightMap1( *this );
	if ( !pHeightMap_->load( pTerrainDS, error ) )
	{
		std::stringstream msg;
			msg << "CommonTerrainBlock1::load: "; 
			msg << "Couldn't load height data for " << filename;
		if ( error )
			*error = msg.str();
		else
			ERROR_MSG( "%s\n", msg.str().c_str() );
		return false;
	}

	// Load the holes
	pHoleMap_ = new TerrainHoleMap1( *this );
	if ( !pHoleMap_->load( pTerrainDS, error ) )
	{
		std::stringstream msg;
			msg << "CommonTerrainBlock1::load: "; 
			msg << "Couldn't load holes data for " << filename;
		if ( error )
			*error = msg.str();
		else
			ERROR_MSG( "%s\n", msg.str().c_str() );
		return false;
	}

	return this->postLoad( filename,
			worldTransform, cameraPosition, pTerrainDS, textureLayers, error );
}


/**
 *  This function gets the height map for the terrain.
 *
 *  @returns                The height map for the terrain.
 */
/*virtual*/ TerrainHeightMap &CommonTerrainBlock1::heightMap()
{
    return *pHeightMap_;
}


/**
 *  This function gets the height map for the terrain.
 *
 *  @returns                The height map for the terrain.
 */
/*virtual*/ TerrainHeightMap const &CommonTerrainBlock1::heightMap() const
{
    return *pHeightMap_;
}


/**
 *  This function gets the hole map for the terain.
 *
 *  @returns                The hole map for the terrain.
 */
/*virtual*/ TerrainHoleMap &CommonTerrainBlock1::holeMap()
{
    return *pHoleMap_;
}


/**
 *  This function gets the hole map for the terain.
 *
 *  @returns                The hole map for the terrain.
 */
/*virtual*/ TerrainHoleMap const &CommonTerrainBlock1::holeMap() const
{
    return *pHoleMap_;
}


/**
 *  This function gets the dominant texture map for the terain.
 *
 *  @returns                The dominant texture map for the terrain.
 */
DominantTextureMapPtr CommonTerrainBlock1::dominantTextureMap()
{
	return pDominantTextureMap_.getObject();
}


/**
 *  This function gets the dominant texture map for the terain.
 *
 *  @returns                The dominant texture map for the terrain.
 */
DominantTextureMapPtr const CommonTerrainBlock1::dominantTextureMap() const
{
	return pDominantTextureMap_.getObject();
}


/**
 *  This function gets the containing BoundingBox for the terrain.
 *
 *  @param bb               The bounding box to get.
 */
/*virtual*/ BoundingBox const &CommonTerrainBlock1::boundingBox() const
{
	float maxh = this->pHeightMap_->maxHeight();
	float minh = this->pHeightMap_->minHeight();

	bb_.setBounds( Vector3( 0.f, minh, 0.f ), Vector3( BLOCK_SIZE_METRES, maxh, BLOCK_SIZE_METRES ) );

    return bb_;
}


/**
 *  This function determines whether the given line segment intersects
 *  the terrain, and if so where.
 *
 *  @param start            The start of the line segment.
 *  @param end              The end of the line segment.
 *  @param callback         The collision callback.
 *  @returns                True if there was a collision, false otherwise.
 */
/*virtual*/ bool
CommonTerrainBlock1::collide
(
    Vector3                 const &start,
    Vector3                 const &end,
    TerrainCollisionCallback *callback
) const
{
    return pHeightMap_->collide(start, end, callback);
}


/**
 *  This function determines whether the given prism intersects
 *  the terrain, and if so where.
 *
 *  @param start            The start face of the prism.
 *  @param end              The end of the line segment.
 *  @param callback         The collision callback.
 *  @returns                True if there was a collision, false otherwise.
 */
/*virtual*/ bool
CommonTerrainBlock1::collide
(
    WorldTriangle           const &start,
    Vector3                 const &end,
    TerrainCollisionCallback *callback
) const
{
    return pHeightMap_->collide(start, end, callback);
}


/**
 *  This function determines the height at the given point, taking into
 *  account holes.
 *
 *  @param x            The x coordinate to get the height at.
 *  @param z            The z coordinate to get the height at.
 *  @returns            The height at x, z.
 */
/*virtual*/ float CommonTerrainBlock1::heightAt(float x, float z) const
{
	float res = NO_TERRAIN;

	if (!this->holeMap().holeAt( x, z ))
	{
	    res = pHeightMap_->heightAt(x, z);
	}

	return res;
}


/**
 *  This function determines the normal at the given point, taking into
 *  account holes.
 *
 *  @param x            The x coordinate to get the normal at.
 *  @param z            The z coordinate to get the normal at.
 *  @returns            The normal at x, z.
 */
/*virtual*/ Vector3 CommonTerrainBlock1::normalAt(float x, float z) const
{
    return pHeightMap_->normalAt(x, z);
}


/////////////////////////////////////////////////////////
// terrain block header info micro methods
/////////////////////////////////////////////////////////

/**
 *  Returns the number of elements for the blends, heights, holes, etc.
 *
 *  @returns            number of elements
 */
uint32 CommonTerrainBlock1::numMapElements() const
{
	MF_ASSERT( numMapElements_ );
	return numMapElements_;
}


/**
 *  Returns the offset to the index'th texture layer name from the start of the
 *  block's data.
 *
 *  @param index        index of the texture layer
 *  @returns            offset to the texture layer's name (in bytes)
 */
uint32 CommonTerrainBlock1::textureNameOffset( uint32 index ) const
{
	return textureNamesOffset() + index * textureNameSize_;
}


/**
 *  Returns the offset to the texture names from the start of the block's data.
 *
 *  @returns            offset to the texture layer names (in bytes)
 */
uint32 CommonTerrainBlock1::textureNamesOffset() const
{
	MF_ASSERT( numMapElements_ );
	return sizeof(TerrainBlockHeader);
}


/**
 *  Returns the offset to the height data from the start of the block's data.
 *
 *  @returns            offset to the height data (in bytes)
 */
uint32 CommonTerrainBlock1::heightsOffset() const
{
	return textureNamesOffset() + numTextures_ * textureNameSize_;
}


/**
 *  Returns the offset to the blends data from the start of the block's data.
 *
 *  @returns            offset to the blends data (in bytes)
 */
uint32 CommonTerrainBlock1::blendsOffset() const
{
	return heightsOffset() + numMapElements_*sizeof(float);
}


/**
 *  Returns the offset to the shadow data from the start of the block's data.
 *
 *  @returns            offset to the shadow data (in bytes)
 */
uint32 CommonTerrainBlock1::shadowsOffset() const
{
	return blendsOffset() + numMapElements_*sizeof(uint32);
}


/**
 *  Returns the offset to the holes data from the start of the block's data.
 *
 *  @returns            offset to the holes data (in bytes)
 */
uint32 CommonTerrainBlock1::holesOffset() const
{
	return shadowsOffset() + numMapElements_*sizeof(uint16);
}
/////////////////////////////////////////////////////////


/**
 *  This function gets called when CommonTerrainBlock1::load is done.
 *  Derived classes can use to load further sections or do something with
 *  the texture layers.
 *
 *  @param filename         The file that was used to load the block.
 *  @param worldPosition    The position of the block in the world.
 *  @param cameraPosition   The position of the camera in the world.
 *  @param pTerrain         The DataSection used to load the block.
 *  @param textureLayers    The texture layers used to load the block.
 *  @param error            If an error occurs during loading then this will
 *                          be set to an error string.
 */
/*virtual*/ bool CommonTerrainBlock1::postLoad
(
    std::string     const &filename,
    Matrix			const &worldTransform,
    Vector3         const &cameraPosition,
    DataSectionPtr  pTerrain,
    TextureLayers   &textureLayers,
    std::string     *error
)
{
	BW_GUARD;
	pDominantTextureMap_ = new DominantTextureMap( textureLayers, 1.0f );
	return true;
}


/**
 *  This function allows the terrain dominant texture map to be set.
 *
 *  @param dominantTextureMap         The terrain dominant texture map to use.
 */
void CommonTerrainBlock1::dominantTextureMap( DominantTextureMapPtr dominantTextureMap )
{
	BW_GUARD;
	pDominantTextureMap_ = dominantTextureMap;
}
