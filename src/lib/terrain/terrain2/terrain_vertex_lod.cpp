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
#include "terrain_vertex_lod.hpp"

#include "../terrain_settings.hpp"
#include "terrain_index_buffer.hpp"
#include "resmgr/auto_config.hpp"

using namespace Terrain;

DECLARE_DEBUG_COMPONENT2( "Terrain", 0 );

namespace
{
	// The last lod is set to be really far away, Not FLT_MAX as shaders may 
	// have different max limits to regular floats
	const float LAST_LOD_DIST = 1000000.f;

	// The minimum distance from the corner of a block to the first neighbour 
	// sub-block that can influence the degenerates
	const float minNeighbourDistance = 
		sqrtf(BLOCK_SIZE_METRES*BLOCK_SIZE_METRES + 
			SUB_BLOCK_SIZE_METRES*SUB_BLOCK_SIZE_METRES);

	// The distance from a corner of the block to the centre of the block
	const float blockCentreDistance = 
		sqrtf(SUB_BLOCK_SIZE_METRES*SUB_BLOCK_SIZE_METRES + 
			SUB_BLOCK_SIZE_METRES*SUB_BLOCK_SIZE_METRES);

	const Vector3 blockNegativeX( -BLOCK_SIZE_METRES, 0, 0 );
	const Vector3 blockPositiveX( BLOCK_SIZE_METRES, 0, 0 );
	const Vector3 blockNegativeZ( 0, 0, -BLOCK_SIZE_METRES );
	const Vector3 blockPositiveZ( 0, 0, BLOCK_SIZE_METRES );

	const Vector3 subBlockOffset0( 0, 0, 0 );
	const Vector3 subBlockOffset1( SUB_BLOCK_SIZE_METRES, 0, 0 );
	const Vector3 subBlockOffset2( 0, 0, SUB_BLOCK_SIZE_METRES );
	const Vector3 subBlockOffset3( SUB_BLOCK_SIZE_METRES, 0, SUB_BLOCK_SIZE_METRES );
}

/**
 *	This method calculates the lod values for a terrain block.
 *
 *	@param distanceInfo   This structure holds input lod and distance data used
 * 						to calculate ouput lod data.
 *	@param lodRenderInfo  This structure contains the output lod data including
 *						neighbour masks.
 *
 *	@return the main lod level of the block
 */
void TerrainVertexLod::calculateMasks(
							const TerrainBlock2::DistanceInfo& distanceInfo,
							TerrainBlock2::LodRenderInfo& lodRenderInfo ) const
{
	BW_GUARD;
	// Clear masks to default values
	lodRenderInfo.neighbourMasks_.assign( 0 );
	lodRenderInfo.subBlockMask_ = 0x0;

	// Calculate morph ranges
	lodRenderInfo.morphRanges_.main_
		= calcMorphRanges( distanceInfo.currentVertexLod_);

	// Early out for block that won't be split
	float lodLevelDistance = distance( distanceInfo.currentVertexLod_ );
	if ( TerrainSettings::constantLod() ||
		(lodLevelDistance > ( distanceInfo.minDistanceToCamera_ + minNeighbourDistance)))
	{
		// Draw entire block at single lod.
		lodRenderInfo.subBlockMask_ = 0xF;
		return;
	}

	// Calculate neighbours
	const float blockSize = BLOCK_SIZE_METRES;

	if ( xzDistance( blockNegativeX, blockSize, distanceInfo.relativeCameraPos_ ) > lodLevelDistance )
		lodRenderInfo.neighbourMasks_[0] |= TerrainIndexBuffer::DIRECTION_NEGATIVEX;
	if ( xzDistance( blockPositiveX, blockSize, distanceInfo.relativeCameraPos_) > lodLevelDistance )
		lodRenderInfo.neighbourMasks_[0] |= TerrainIndexBuffer::DIRECTION_POSITIVEX;
	if ( xzDistance( blockNegativeZ, blockSize, distanceInfo.relativeCameraPos_) > lodLevelDistance )
		lodRenderInfo.neighbourMasks_[0] |= TerrainIndexBuffer::DIRECTION_NEGATIVEZ;
	if ( xzDistance( blockPositiveZ, blockSize, distanceInfo.relativeCameraPos_) > lodLevelDistance )
		lodRenderInfo.neighbourMasks_[0] |= TerrainIndexBuffer::DIRECTION_POSITIVEZ;

	// Work out if we need to lod any sub-blocks, we only split if
	// 1. We're lod 1 or 0
	// 2. We have lower-lod neighbours
	// 3. We're close enough to a lod boundary
	// 4. Splitting is enabled ( tested last, as it is usually on )
	if (distanceInfo.currentVertexLod_	 <  2							&&
		lodRenderInfo.neighbourMasks_[0] != 0							&& 
		lodLevelDistance < 
			( distanceInfo.minDistanceToCamera_ + blockCentreDistance)	&&
		TerrainSettings::doBlockSplit()			)
	{	
		lodRenderInfo.morphRanges_.subblock_ = calcMorphRanges( 
			distanceInfo.currentVertexLod_ + 1 );
	
		const float subBlockSize = SUB_BLOCK_SIZE_METRES;

		// calculate distances to each sub-block from camera, note order is
		// is important - see below. Sub-blocks are arranged as follows:
		// *---+---+    * = origin
		// | 0 | 1 |
		// +---+---+
		// | 2 | 3 |
		// +---+---+ +x
		//        +z
		const float	subBlockDistances[4] =
		{
			xzDistance( subBlockOffset0, subBlockSize, distanceInfo.relativeCameraPos_ ),
			xzDistance( subBlockOffset1, subBlockSize, distanceInfo.relativeCameraPos_ ),
			xzDistance( subBlockOffset2, subBlockSize, distanceInfo.relativeCameraPos_ ),
			xzDistance( subBlockOffset3, subBlockSize, distanceInfo.relativeCameraPos_ )
		};

		// mask each sub-block according to its distance, note order must match
		// flags (as used by TerrainIndexBuffer::draw()
		for ( uint32 i = 0; i < ARRAY_SIZE( subBlockDistances ); i++ )
		{
			if ( subBlockDistances[i] < lodLevelDistance )
				lodRenderInfo.subBlockMask_ |= 1 << i;
		}

		// we may have tried to sub-divide a block that didn't need it, so don't
		// bother with sub-block tests in that case.
		if ( lodRenderInfo.subBlockMask_ != 0xF )
		{
			// work out which sub-blocks require degenerates for inside borders.
			internalSubBlockTests( lodRenderInfo.neighbourMasks_, 
									lodRenderInfo.subBlockMask_ );
		}
	}
	else
	{
		// Draw entire block at single lod.
		lodRenderInfo.subBlockMask_ = 0xF;	
	}

	// Work out borders of all sub-blocks.
	externalSubBlockTests( distanceInfo.currentVertexLod_, 
							lodRenderInfo.subBlockMask_, 
							distanceInfo.relativeCameraPos_, 
							lodRenderInfo.neighbourMasks_ );
}

void TerrainVertexLod::internalSubBlockTests(	NeighbourMasks&  neighbourMasks,
												uint8			subBlockMask ) const
{
	BW_GUARD;
	// for each sub block work out if we need degenerates, by making sure
	// non-diagonal neighbours have the same mask (ie the same lod). An entry
	// in the sub-block mask means we have the higher lod and must provide 
	// degenerates.
	
	// indexes
	uint8 myindex	= 0;	// my index		
	uint8 index1	= 1;	// first neighbour
	uint8 index2	= 2;	// second neighbour

	// lod values ( 0 == different lod, !0 == same lod)
	bool myValue= 0 != (( 1 << myindex ) & subBlockMask );
	bool value1	= 0 != (( 1 << index1 )  & subBlockMask );
	bool value2	= 0 != (( 1 << index2 )  & subBlockMask );

	// check neighbours and set, sub-block neighbor masks start at index 1.
	if ( myValue != value1 ) 
	{
		if ( myValue )
		{
			// set me
			neighbourMasks[ myindex + 1] |= TerrainIndexBuffer::DIRECTION_POSITIVEX;		
		}
		else		
		{
			// set neighbour
			neighbourMasks[ index1 + 1]|= TerrainIndexBuffer::DIRECTION_NEGATIVEX;
		}
	}
	if ( myValue != value2 )
	{
		if ( myValue )
		{
			// set me
			neighbourMasks[ myindex + 1] |= TerrainIndexBuffer::DIRECTION_POSITIVEZ;
		}
		else
		{
			// set neighbour
			neighbourMasks[ index2 + 1] |= TerrainIndexBuffer::DIRECTION_NEGATIVEZ;
		}
	}

	// indexes
	myindex	= 3;	// my index		

	// lod values ( 0 == different lod, !0 == same lod)
	myValue= 0 != (( 1 << myindex ) & subBlockMask );

	// check neighbours
	if ( myValue != value1 ) 
	{
		if ( myValue )
		{
			// set me
			neighbourMasks[ myindex + 1] |= TerrainIndexBuffer::DIRECTION_NEGATIVEZ;
		}
		else
		{
			// set neighbour
			neighbourMasks[ index1 + 1]|= TerrainIndexBuffer::DIRECTION_POSITIVEZ;
		}
	}
	if ( myValue != value2 )
	{
		if ( myValue )
		{
			// set me
			neighbourMasks[ myindex + 1] |= TerrainIndexBuffer::DIRECTION_NEGATIVEX;		
		}
		else
		{
			// set neighbour
			neighbourMasks[ index2 + 1]|= TerrainIndexBuffer::DIRECTION_POSITIVEX;
		}
	}
}

namespace
{
	// Positions and corresponding masks
	struct PositionsAndMasks
	{
		uint32	sbIndex;	// this sub-block we're testing.
		uint8	mask;		// direction from this to neighbour sub-block
		Vector3 sbOffset;	// world position of the neighbour sub-block.
	};

	const PositionsAndMasks testData[8] = 
	{
		{2, TerrainIndexBuffer::DIRECTION_NEGATIVEX, 
			blockNegativeX + subBlockOffset3},
		{0, TerrainIndexBuffer::DIRECTION_NEGATIVEX, 
			blockNegativeX + subBlockOffset1},
		{0, TerrainIndexBuffer::DIRECTION_NEGATIVEZ, 
			blockNegativeZ + subBlockOffset2},
		{1, TerrainIndexBuffer::DIRECTION_NEGATIVEZ, 
			blockNegativeZ + subBlockOffset3},
		{1, TerrainIndexBuffer::DIRECTION_POSITIVEX, 
			blockPositiveX + subBlockOffset0},
	    {3, TerrainIndexBuffer::DIRECTION_POSITIVEX, 
			blockPositiveX + subBlockOffset2},
		{3, TerrainIndexBuffer::DIRECTION_POSITIVEZ, 
			blockPositiveZ + subBlockOffset1},
 		{2, TerrainIndexBuffer::DIRECTION_POSITIVEZ, 
			blockPositiveZ + subBlockOffset0}
	};
}

void TerrainVertexLod::externalSubBlockTests(	uint32			mainBlockLod,
												uint8			subBlockMask,
												const Vector3&  cameraPosition,
												NeighbourMasks&	neighbourMasks ) const
{
	BW_GUARD;
	// work out the lod values of the surrounding sub-blocks, numbered 0-7 and
	// arranged as follows:
	//
	//		|nz |
	//		|2 3|
	//	 ---+-+-+---
	//	nx 1|0|1|4 px
	//	   0|2|3|5
	//	 ---+---+---
	//		|7 6|
	//		|pz |
	//

	static const float subBlockSize	= SUB_BLOCK_SIZE_METRES;

	// We use the actual lod distances.
	float mainDistance = distance(mainBlockLod);
	float subDistance = distance(mainBlockLod + 1);

	// Do tests and mask sub-block degenerates
	for ( uint32 i = 0; i < ARRAY_SIZE( testData ); i++ )
	{
		float ourDistance = subDistance;
		if ( (1 << testData[i].sbIndex) & subBlockMask  )
		{
			ourDistance = mainDistance;
		}

		// if we're a higher lod than our neighbour, then flag that direction for
		// degenerates
		float distance = xzDistance( testData[i].sbOffset, 
									subBlockSize, cameraPosition  );
		
		if (distance > ourDistance)
		{
			neighbourMasks[ 1 + testData[i].sbIndex ] |= testData[i].mask;
		}
	}
}

/**
 *	This method calculates the distance on the xz plane to a terrain block
 *
 *	@param blockCorner    the min xz corner of the terrain block
 *	@param blockSize      the size on the x and z axis of the block
 *	@param cameraPosition the camera position to calculate the distance from
 *
 *	@return the distance to the block
 */
float TerrainVertexLod::xzDistance( const Vector3&	blockCorner,
								   float			blockSize, 
								   const Vector3&	cameraPosition )
{
	// get xz delta for the camera to block position
	Vector2 v(cameraPosition.x - blockCorner.x, cameraPosition.z - blockCorner.z);

	// get the x distance to the block, 0 is inside the block
	if (v.x > blockSize)
		v.x -= blockSize;
	else if (v.x > 0.f)
		v.x = 0.f;

	// get the z distance to the block, 0 is inside the block
	if (v.y > blockSize)
		v.y -= blockSize;
	else if (v.y > 0.f)
		v.y = 0.f;

	// calculate the xz distance to the block
	return v.length();
}

/**
 *	This method calculates the distance to the nearest point on a block
 *	and the furthest point on the same block
 *
 *	@param relativeCameraPos the camera pos relative to the block
 *	@param blockSize the size on the x and z axis of the block
 *	@param minDistance output value for the closest point on the block
 *	@param maxDistance output value for the furthest point on the block
 */
void TerrainVertexLod::minMaxXZDistance( const Vector3& relativeCameraPos,
			const float blockSize, float& minDistance, float& maxDistance )
{
	Vector2 min(relativeCameraPos.x, relativeCameraPos.z);
	Vector2 max = min;

	float halfBlockSize = blockSize * 0.5f;

	if (relativeCameraPos.x > blockSize)
	{
		min.x -= blockSize;
	}
	else if (relativeCameraPos.x > 0.f)
	{
		min.x = 0.f;
		max.x = fabsf(max.x - halfBlockSize) + halfBlockSize;
	}
	else
	{
		max.x -= blockSize;
	}

	if (relativeCameraPos.z > blockSize)
	{
		min.y -= blockSize;
	}
	else if (relativeCameraPos.z > 0.f)
	{
		min.y = 0.f;
		max.y = fabsf(max.y - halfBlockSize) + halfBlockSize;
	}
	else
	{
		max.y -= blockSize;
	}

	minDistance = min.length();
	maxDistance = max.length();
}

/**
 *	This method calculates the lodLevel at a specific distance.
 *	
 */
uint32 TerrainVertexLod::calculateLodLevel( float blockDistance ) const
{
	const uint32 nLods = size();

	// Start with the current lod level
	uint32 lod = std::min(TerrainSettings::topVertexLod(), nLods);

	if ( !TerrainSettings::constantLod() )
	{
		for ( ; lod < nLods; lod++ )
		{
			if ( blockDistance < (*this)[lod] )
			{
				break;
			}
		}
	}
	return lod;
}

/**
 * This method calculates the morph ranges for a given lod level.
 */ 
Vector2 TerrainVertexLod::calcMorphRanges( uint32 lodLevel ) const
{
	float lodDelta = distance(lodLevel);
	float lodStart = 0;

	// If we are not at lod0 then the lodStart is the previous lod distance
	if (lodLevel != 0)
		lodStart = distance(lodLevel - 1);

	// Calculate lod delta
	lodDelta -= lodStart;

	if (TerrainSettings::constantLod())
	{
		lodStart = 0;
		lodDelta = LAST_LOD_DIST;
	}

	// Calculate the morph ranges from the this lod level and the previous
	Vector2 morphRange( lodStart + lodDelta * startBias_,
		lodStart + lodDelta * endBias_ );

	// return our LOD index
	return morphRange;
}

/**
 *	Get the distance at which a particular lod level will lod out
 *	@param lod the lod level
 *	@return the distance at which this lod level will lod out
 */
float TerrainVertexLod::distance( uint32 lod ) const
{
	float dist = LAST_LOD_DIST;
	if (lod < size())
	{
		dist = (*this)[lod];
	}
	return dist;
}

/**
 *	Set the distance at which a particular lod level will lod out
 *	@param lod the lod level
 *	@param value the distance at which the lod level lods out
 */
void TerrainVertexLod::distance( uint32 lod, float value )
{
	if (lod < size())
	{
		(*this)[lod] = value;
	}
}


#ifdef TESTS_ENABLED
void TerrainVertexLod::TestTerrainVertexLod()
{
	// Test 1 - sub-sub test with none set
	{
		NeighbourMasks nm( 5 );
		for ( int i = 0; i < 5; i++ )
		{
			nm[i] = 0;
		}
		uint8 sbm = 0;

		subBlockSubBlockTests( nm, sbm );

		for ( int i = 0; i < 5; i++ )
		{
			MF_ASSERT( "Test failed!" && nm[i] == 0 );
		}
	}

	// Test 2 - sub-sub test with all set
	{
		NeighbourMasks nm( 5 );
		for ( int i = 0; i < 5; i++ )
		{
			nm[i] = 0;
		}
		uint8 sbm = 0xf;

		subBlockSubBlockTests( nm, sbm );

		for ( int i = 0; i < 5; i++ )
		{
			MF_ASSERT( "Test failed!" && nm[i] == 0 );
		}
	}

	// Test 3 - sub-sub test with first one set only
	{
		NeighbourMasks nm( 5 );
		for ( int i = 0; i < 5; i++ )
		{
			nm[i] = 0;
		}
		uint8 sbm = 0x1;

		subBlockSubBlockTests( nm, sbm );

		MF_ASSERT( "Test failed!" && ( nm[0] == 0 ) );
		MF_ASSERT( "Test failed!" && ( nm[1] & TerrainIndexBuffer::DIRECTION_POSITIVEX ) );
		MF_ASSERT( "Test failed!" && ( nm[1] & TerrainIndexBuffer::DIRECTION_POSITIVEZ ) );
		MF_ASSERT( "Test failed!" && ( nm[2] == 0 ) );
		MF_ASSERT( "Test failed!" && ( nm[3] == 0 ) );
		MF_ASSERT( "Test failed!" && ( nm[4] == 0 ) );

	}

	// Test 4 - sub-sub test with first two set only
	{
		NeighbourMasks nm( 5 );
		for ( int i = 0; i < 5; i++ )
		{
			nm[i] = 0;
		}
		uint8 sbm = 0x3;

		subBlockSubBlockTests( nm, sbm );

		MF_ASSERT( "Test failed!" && ( nm[0] == 0 ) );
		MF_ASSERT( "Test failed!" && (( nm[1] ^ TerrainIndexBuffer::DIRECTION_POSITIVEZ ) == 0 ));
		MF_ASSERT( "Test failed!" && (( nm[2] ^ TerrainIndexBuffer::DIRECTION_POSITIVEZ ) == 0 ));
		MF_ASSERT( "Test failed!" && ( nm[3] == 0 ) );
		MF_ASSERT( "Test failed!" && ( nm[4] == 0 ) );		
	}

	// Test 5 - sub-sub test with first three set only
	{
		NeighbourMasks nm( 5 );
		for ( int i = 0; i < 5; i++ )
		{
			nm[i] = 0;
		}
		uint8 sbm = 0x7;

		subBlockSubBlockTests( nm, sbm );

		MF_ASSERT( "Test failed!" && ( nm[0] == 0 ) );
		MF_ASSERT( "Test failed!" && ( nm[1] == 0 ) );
		MF_ASSERT( "Test failed!" && (( nm[2] ^ TerrainIndexBuffer::DIRECTION_POSITIVEZ ) == 0 ));
		MF_ASSERT( "Test failed!" && (( nm[3] ^ TerrainIndexBuffer::DIRECTION_POSITIVEX ) == 0 ));
		MF_ASSERT( "Test failed!" && ( nm[4] == 0 ) );		
	}

	// Test 6 - sub-sub test with last one set only
	{
		NeighbourMasks nm( 5 );
		for ( int i = 0; i < 5; i++ )
		{
			nm[i] = 0;
		}
		uint8 sbm = 0x8;

		subBlockSubBlockTests( nm, sbm );

		MF_ASSERT( "Test failed!" && ( nm[0] == 0 ) );
		MF_ASSERT( "Test failed!" && ( nm[1] == 0 ) );
		MF_ASSERT( "Test failed!" && ( nm[2] == 0 ) );
		MF_ASSERT( "Test failed!" && ( nm[3] == 0 ) );
		MF_ASSERT( "Test failed!" && ( nm[4] & TerrainIndexBuffer::DIRECTION_NEGATIVEX ) );
		MF_ASSERT( "Test failed!" && ( nm[4] & TerrainIndexBuffer::DIRECTION_NEGATIVEZ ) );
	}
}
#endif
