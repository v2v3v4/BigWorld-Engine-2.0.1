/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_VERTEX_LOD_HPP
#define TERRAIN_TERRAIN_VERTEX_LOD_HPP

#include "terrain_block2.hpp"

namespace Terrain
{
    /**
     *	This class is a helper class for calculating lod and lod values for the 
     *  terrain.
     */
	class TerrainVertexLod : public std::vector<float>
    {
    public:
		
		uint32	calculateLodLevel( float dist ) const;

		void	calculateMasks(	const TerrainBlock2::DistanceInfo& distanceInfo,
								TerrainBlock2::LodRenderInfo& lodRenderInfo ) 
								const;
		
		float	distance( uint32 lod ) const;

		void	distance( uint32 lod, float value );

		float	startBias() const { return startBias_; }
		void	startBias( float value ) { startBias_ = value; }
		float	endBias() const { return endBias_; }
		void	endBias( float value ) { endBias_ = value; }

		static float xzDistance(const Vector3&	chunkCorner,
								float			blockSize,
								const Vector3&	cameraPosition );
		static void minMaxXZDistance( const Vector3& relativeCameraPos,
			const float blockSize, float& minDistance, float& maxDistance );


	private:
		// Test sub-blocks against themselves for degenerates.
		void internalSubBlockTests(	NeighbourMasks&  neighbourMasks,
									uint8			subBlockMask ) const;
		
		void externalSubBlockTests( uint32			mainBlockLod,
									uint8			subBlockMask,
									const Vector3&  cameraPosition,
									NeighbourMasks& neighbourMasks ) const;


		Vector2 calcMorphRanges( uint32 lodLevel ) const;

		// Start end bias for vertex lod, these are the proportions
		// of the lod-deltas the geomorphing happens between.
		float	startBias_;
		float	endBias_;

	};
}

#endif // TERRAIN_TERRAIN_VERTEX_LOD_HPP
