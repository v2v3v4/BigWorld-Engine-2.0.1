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
#include "terrain_vertex_buffer.hpp"
#include "aliased_height_map.hpp"
#include "grid_vertex_buffer.hpp"

namespace Terrain
{

// -----------------------------------------------------------------------------
// Section: TerrainVertexBuffer
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
TerrainVertexBuffer::TerrainVertexBuffer()
{
}


/**
 *	Destructor.
 */
TerrainVertexBuffer::~TerrainVertexBuffer()
{
}

/**
 *	This method generates the terrain vertices.
 *	
 *	@param hm            the heightmap for this lod.
 *	@param lodHM         the heightmap for the previous lod level.
 *	@param resolutionX   the number of x-axis vertices to generate.
 *	@param resolutionZ   the number of z-axis vertices to generate.
 *	@param vertices      is filled with two heights per vertex, the height of
 *                       the vertex in the current lod and the height of the
 *                       vertex in the next lower lod.
 */
void TerrainVertexBuffer::generate( const AliasedHeightMap* hm, 
									const AliasedHeightMap* lodHM, 
									uint32 resolutionX, 
									uint32 resolutionZ,
									std::vector< Vector2 > & vertices )
{
	BW_GUARD;
	uint32 nVertices =  resolutionX * resolutionZ;
	vertices.resize(nVertices);
	
	// Create the heights for the vertex buffer, the heights are calculated so
	// that the vertex contains the current height and the corresponding
	// height at the same x/z position in the next lower lod.
	for (uint32 z = 0; z < resolutionZ; z++)
	{
		// Odd and even rows in the heightmap are treated differently as 
		// they have different bases for calculating the position for the next
		// lod level down.
		if (z & 1)
		{
			std::vector< Vector2 >::iterator it = 
				vertices.begin() + resolutionX * z;

			// Calculate the odd row, even column.
			uint32 lodZ = z /2;
			uint32 lodX = 0;
			for (uint32 x = 0; x < resolutionX; x += 2)
			{
				Vector2& v = *it;
				v.x = hm->height( x,z );
				v.y = (lodHM->height( lodX, lodZ ) + 
						lodHM->height(lodX, lodZ + 1)) * 0.5f;

				++lodX;
				it += 2;
			}

			it = vertices.begin() + resolutionX * z + 1;

			// Calculate the odd row, odd column.
			lodZ = z /2;
			lodX = 0;

			// These indices are for the odd and even rows and columns for creating the 
			// approximate height value of the next lod down, since we are using crossed
			// indices we want to alternate the angle of the diagonals the height is
			// calculated from.

			uint32 x1 = 0;
			uint32 x2 = 1;

			if (lodZ & 1)
			{
				x1 = 1;
				x2 = 0;
			}

			for (uint32 x = 1; x < resolutionX; x += 2)
			{
				Vector2& v = *it;
				v.x = hm->height( x,z );
				v.y = (lodHM->height( lodX + x1, lodZ ) + 
						lodHM->height(lodX + x2, lodZ + 1)) * 0.5f;

				x1 ^= 1;
				x2 ^= 1;

				++lodX;
				it += 2;
			}
		}
		else
		{
			uint32 i = resolutionX * z;

			// Calculate the even row, even column.
			uint32 lodZ = z /2;
			uint32 lodX = 0;
			for (uint32 x = 0; x < resolutionX; x += 2)
			{
				Vector2& v = vertices[i];

				v.x = hm->height( x,z );
				v.y = lodHM->height( lodX, lodZ );

				++lodX;
				i += 2;
			}

			i = resolutionX * z + 1;

			// Calculate the even row, odd column.
			lodZ = z /2;
			lodX = 0;
			for (uint32 x = 1; x < resolutionX; x += 2)
			{
				Vector2& v = vertices[i];

				v.x = hm->height( x,z );
				v.y = (lodHM->height( lodX, lodZ ) + 
						lodHM->height( lodX + 1, lodZ )) * 0.5f;

				++lodX;
				i += 2;
			}
		}
	}
}

/**
 *	This method generates the terrain vertex buffer using given vertices.
 *
 *	@param vertices      the height of vertex this lod and previous lod.
 *	@param resolutionX   the number of x-axis vertices to use.
 *	@param resolutionZ   the number of z-axis vertices to use.
 *	@param usage         D3D usage options that identify how resources are to be used.

 */
bool TerrainVertexBuffer::init( const Vector2 * vertices,
								uint32 resolutionX, uint32 resolutionZ,
								uint32 usage )
{
	BW_GUARD;
	bool res = false;
	const uint32 bufferSize = sizeof( Vector2 ) * resolutionX * resolutionZ;

	if ( Moo::rc().device() )
	{
		// Create the actual vertex buffer.
		pVertexBuffer_.release();
		if (SUCCEEDED( pVertexBuffer_.create( 
					bufferSize,
					usage, 0, D3DPOOL_MANAGED,
					"Terrain/VertexBuffer" ) ) )
		{
			Moo::VertexLock<Vector2> vl( pVertexBuffer_ );
			if (vl)
			{
				memcpy( vl, vertices, bufferSize );
				res = true;
				pGridBuffer_ = GridVertexBuffer::get( uint16(resolutionX), uint16(resolutionZ) );

				// Add the buffer to the preload list so that it can get uploaded
				// to video memory
				pVertexBuffer_.addToPreloadList();
			}
			else
			{
				pVertexBuffer_.release();
			}
		}
	}

	return res;
}

/**
 *	This method sets the terrain vertex buffer on the device
 *	@return true if successful
 */
bool TerrainVertexBuffer::set()
{
	BW_GUARD;
	bool res = false;
	if (pVertexBuffer_.valid())
	{
		pVertexBuffer_.set( 0, 0, sizeof( Vector2 ) );
		pGridBuffer_->pBuffer().set( 1, 0, sizeof( Vector2 ) );
		res = true;
	}

	return res;
}

}
