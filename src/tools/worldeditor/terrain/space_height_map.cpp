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
#include "worldeditor/terrain/space_height_map.hpp"
#include "worldeditor/terrain/editor_chunk_terrain.hpp"
#include "chunk/chunk_terrain.hpp"
#include "chunk/chunk_format.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_manager.hpp"
#include "worldeditor/world/editor_chunk.hpp"


namespace
{
	// This epsilon is needed because of floating point errors in optimised code.
	const float GRID_COORD_EPSILON = 0.001f;
}


/**
 *  Constructor, just initialises member variables.
 */
SpaceHeightMap::SpaceHeightMap() :
	invGridResolution_( 1.0f / GRID_RESOLUTION ),	// used to avoid division
	invPoleSpacingX_( 1.0f ),	// used to avoid division
	invPoleSpacingY_( 1.0f ),	// used to avoid division
	numPolesX_( 0 ),
	numPolesY_( 0 ),
	visOffsX_( 0 ),
	visOffsY_( 0 ),
	minGridX_( 0 ),
	maxGridX_( 0 ),
	minGridY_( 0 ),
	maxGridY_( 0 ),
	prepared_( false )
{
}


/**
 *  Destructor, does nothing.
 */
SpaceHeightMap::~SpaceHeightMap()
{
}


/**
 *	Sets internal states before processing.
 *
 *  @param format	Terrain format info.
 */
void SpaceHeightMap::prepare(
	const TerrainUtils::TerrainFormat &format )
{
	BW_GUARD;

	invPoleSpacingX_ = 1.0f / format.poleSpacingX;
	invPoleSpacingY_ = 1.0f / format.poleSpacingY;
	numPolesX_ = format.blockWidth;
	numPolesY_ = format.blockHeight;
	visOffsX_ = format.visOffsX;
	visOffsY_ = format.visOffsY;
	ChunkSpacePtr space = ChunkManager::instance().cameraSpace();
	minGridX_ = space->minGridX();
	maxGridX_ = space->maxGridX();
	minGridY_ = space->minGridY();
	maxGridY_ = space->maxGridY();
	prepared_ = true;
}


/**
 *	Returns if the space height map has been prepared
 *
 *  @return			true if it has been prepared, false, otherwise.
 */
bool SpaceHeightMap::prepared() const
{
	return prepared_;
}


/**
 *	Free all resources and change state to 'unprepared'
 */
void SpaceHeightMap::clear()
{
	BW_GUARD;

	heights_.clear();
	prepared_ = false;
}


/**
 *	Adds a block's height map image to the map of blocks. This allows changing
 *  the block's height map afterwards without modifying the cached version in
 *  SpaceHeightMap.
 *
 *  @param x		X coord of a point inside the the block, in space coords
 *  @param y		Y coord of a point inside the the block, in space coords
 *  @param img		image that contains the block's height map.
 */
void SpaceHeightMap::add( float x, float y, Image& img )
{
	BW_GUARD;

	if ( !prepared_ )
		return;

	int gx = gridCoordX( x );
	int gy = gridCoordY( y );
	Key key( gx, gy );
	Heights::iterator it = heights_.find( key );
	if ( it == heights_.end() )
	{
		std::pair<Heights::iterator, bool> res =
			heights_.insert( std::pair<Key,Image>( key, img ) );
		it = res.first;
	}
	if ( it == heights_.end() )
		return;

	(*it).second = img;
}


/**
 *	Gets a pixel from the virtual space height map. This method is not very
 *  efficient, as it has to do a fair bit of processing to get a single pixel.
 *  The prefered method should be getRect. Also, this method does not
 *  interpolate heights, it returns the nearest height to (x,y).
 *
 *  @param x		X coord of the point, in space coords
 *  @param y		Y coord of the point, in space coords
 *  @return			height at that point in space.
 */
SpaceHeightMap::Pixel SpaceHeightMap::get( float x, float y ) const
{
	BW_GUARD;

	if ( !prepared_ )
		return 0.0f;

	int gx = gridCoordX( x );
	int gy = gridCoordY( y );
	Key key( gx, gy );

	const Image* img = getImage( key, x, y );

	if ( img == NULL )
		return 0.0f;

	float height = img->get(
		mapCoordX( x, gx ) + visOffsX_,
		mapCoordY( y, gy ) + visOffsY_ );

	return height;
}


/**
 *	Gets a rectangle from the virtual space height map. This allows much faster
 *  processing than accesing a single pixel at a time. This method does not
 *  interpolate heights, it snaps input coords to height map coords.
 *
 *  @param x		X coord of the one corner of the rect, in space coords
 *  @param y		Y coord of the one corner of the rect, in space coords
 *  @param xmax		X coord of the other corner of the rect, in space coords
 *  @param ymax		Y coord of the other corner of the rect, in space coords
 *  @param outImg	image where the height map rectangle will be returned
 *  @return			true if successful, false otherwise.
 */
bool SpaceHeightMap::getRect( float x, float y, float xmax, float ymax, Image& outImg ) const
{
	BW_GUARD;

	if ( !prepared_ || x == xmax || y == ymax )
		return false;

	int gminx = gridCoordX( x );
	int gminy = gridCoordY( y );
	int gmaxx = gridCoordX( xmax );
	int gmaxy = gridCoordY( ymax );

	if ( gminx > gmaxx )
		std::swap( gminx, gmaxx );
	if ( gminy > gmaxy )
		std::swap( gminy, gmaxy );

	outImg.resize(
		mapCoordX( xmax - x, 0 ) + 1,
		mapCoordY( ymax - y, 0 ) + 1,
		0.0f );
	int outOffsetX = mapCoordX( x, gminx );
	int outOffsetY = mapCoordY( y, gminy );

	for ( int gy = gminy; gy <= gmaxy; ++gy )
	{
		for ( int gx = gminx; gx <= gmaxx; ++gx )
		{
			// process one grid cell at a time
			Key key( gx, gy );

			int minx = mapCoordX( x, gx );
			int miny = mapCoordY( y, gy );
			int maxx = mapCoordX( xmax, gx );
			int maxy = mapCoordY( ymax, gy );
			if ( minx < 0 )
				minx = 0;
			if ( miny < 0 )
				miny = 0;
			if ( maxx >= (int)numPolesX_ )
				maxx = (int)numPolesX_ - 1;
			if ( maxy >= (int)numPolesY_ )
				maxy = (int)numPolesY_ - 1;

			const Image* img = getImage( key, gx * GRID_RESOLUTION, gy * GRID_RESOLUTION );
			if ( img == NULL )
			{
				outImg.clear();
				return false;
			}

			for ( int y = miny; y <= maxy; ++y )
			{
				Pixel* src = img->getRow( y + visOffsY_ ) + (minx + visOffsX_);
				Pixel* dst = outImg.getRow( y + (gy-gminy) * numPolesY_ - outOffsetY ) +
							(minx + (gx-gminx) * numPolesX_ - outOffsetX);
				Pixel* end = src + (maxx - minx + 1);
				while ( src != end )
					*dst++ = *src++;
			}
		}
	}

	return true;
}


/**
 *	Converts an X coordinate in space coords to grid coords
 *
 *  @param spaceCoord	X coord in space coords
 *  @return				converted grid coord
 */
int SpaceHeightMap::gridCoordX( float spaceCoord ) const
{
	int coord = int( floorf( spaceCoord * invGridResolution_ + GRID_COORD_EPSILON ) );
	if ( coord < minGridX_ )
		coord = minGridX_;
	else if ( coord > maxGridX_ )
		coord = maxGridX_;
	return coord;
}


/**
 *	Converts an Y coordinate in space coords to grid coords
 *
 *  @param spaceCoord	Y coord in space coords
 *  @return				converted grid coord
 */
int SpaceHeightMap::gridCoordY( float spaceCoord ) const
{
	int coord = int( floorf( spaceCoord * invGridResolution_ + GRID_COORD_EPSILON ) );
	if ( coord < minGridY_ )
		coord = minGridY_;
	else if ( coord > maxGridY_ )
		coord = maxGridY_;
	return coord;
}


/**
 *	Converts an X coordinate in chunk coords to height map coords
 *
 *  @param spaceCoord	X coord in space coords
 *  @param gridCoord	X grid coord so it doesn't have to calculate it
 *  @return				converted height map coord
 */
int SpaceHeightMap::mapCoordX( float spaceCoord, int gridCoord ) const
{
	return int(
		(spaceCoord - gridCoord * GRID_RESOLUTION) * invPoleSpacingX_ + 0.5f);
}


/**
 *	Converts an Y coordinate in space coords to height map coords
 *
 *  @param spaceCoord	Y coord in space coords
 *  @param gridCoord	Y grid coord so it doesn't have to calculate it
 *  @return				converted height map coord
 */
int SpaceHeightMap::mapCoordY( float spaceCoord, int gridCoord ) const
{
	return int(
		(spaceCoord - gridCoord * GRID_RESOLUTION) * invPoleSpacingX_ + 0.5f);
}


/**
 *	Tries to find the image corresponding to the key grid coords, and if it
 *  can't find it in the map, it'll find the chunk and the terrain block using
 *  the space coords.
 *
 *  @param key		map key for the wanted image (grid coords)
 *  @param x		X coord in space coords of a point inside the block, used
 *					if the key doesn't exist in the map
 *  @param y		Y coord in space coords of a point inside the block, used
 *					if the key doesn't exist in the map
 *  @return			pointer to the image containing the height map
 */
const SpaceHeightMap::Image* SpaceHeightMap::getImage( const Key& key, float x, float y ) const
{
	BW_GUARD;

	Image* img = NULL;

	Heights::iterator it = heights_.find( key );
	if ( it == heights_.end() )
	{
		// block not found, so add it
		Chunk *chunk = EditorChunk::findOutsideChunk( Vector3( x, 0.0f, y ) );
		Terrain::EditorBaseTerrainBlock* block = NULL;
		if ( chunk )
		{
			// it's already loaded, so get the terrain from the chunk
			EditorChunkTerrain* ect = 
				static_cast<EditorChunkTerrain*>( ChunkTerrainCache::instance(*chunk).pTerrain() );
			if ( ect )
				block = &ect->block();
		}
		else
		{
			// Commented out block-loading code, it seems a bit of an overkill
			//// not loaded yet, so load the block directly
			//std::string chunkName = ChunkFormat::outsideChunkIdentifier( gx, gy );
			//Moo::EditorBaseTerrainBlockPtr block =
			//	static_cast<Moo::EditorBaseTerrainBlock*>(
			//		Moo::EditorBaseTerrainBlock::loadBlock(
			//			chunkName + ".cdata/terrain",
			//			Vector3( 0, 0, 0 ), Vector3( 0, 0, 0 ),
			//			NULL ).getObject() );
		}
		if ( block != NULL )
		{
			// found the other block, so use it!
			Terrain::TerrainHeightMapHolder holder(&block->heightMap(), true);
			std::pair<Heights::iterator, bool> res =
				heights_.insert( std::pair<Key,Image>( key, block->heightMap().image() ) );
			img = &(*res.first).second;
		}
	}
	else
	{
		img = &(*it).second;
	}

	return img;
}
