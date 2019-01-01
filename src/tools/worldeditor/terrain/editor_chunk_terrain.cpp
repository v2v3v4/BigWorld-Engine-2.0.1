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
#include "worldeditor/terrain/editor_chunk_terrain.hpp"
#include "worldeditor/terrain/editor_chunk_terrain_cache.hpp"
#include "worldeditor/undo_redo/terrain_height_map_undo.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "worldeditor/gui/pages/panel_manager.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/project/project_module.hpp"
#include "worldeditor/project/space_helpers.hpp"
#include "worldeditor/editor/item_properties.hpp"
#include "worldeditor/misc/sync_mode.hpp"
#include "worldeditor/misc/options_helper.hpp"
#include "appmgr/module_manager.hpp"
#include "appmgr/options.hpp"
#include "chunk/base_chunk_space.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_light.hpp"
#include "terrain/base_terrain_renderer.hpp"
#include "terrain/terrain1/editor_terrain_block1.hpp"
#include "terrain/terrain2/editor_terrain_block2.hpp"
#include "terrain/horizon_shadow_map.hpp"
#include "terrain/terrain_data.hpp"
#include "terrain/terrain_height_map.hpp"
#include "terrain/terrain_hole_map.hpp"
#include "terrain/terrain_settings.hpp"
#include "terrain/terrain_texture_layer.hpp"
#include "romp/line_helper.hpp"

#if UMBRA_ENABLE
#include <umbraModel.hpp>
#include <umbraObject.hpp>
#include "chunk/chunk_umbra.hpp"
#endif

DECLARE_DEBUG_COMPONENT2( "EditorChunkTerrain", 2 );


//These are all helpers to debug / change the shadow calculation rays.
static bool                 s_debugShadowRays   = false;
static bool                 s_debugShadowPoles  = false;
static std::vector<Vector3> s_lines;
static std::vector<Vector3> s_lineEnds;
static int                  s_collType          = -1;
static float                s_raySubdivision    = 32.f;
static float                s_rayDisplacement   =  1.5f;


namespace
{

	const float SHADOW_RAY_ERROR_OFFSET	= 0.001f;
	const int	SHADOW_MAX_VALUE		= 65535;
	const int	SHADOW_MAX_CALC_RAYS	= 256;
	const int	SHADOW_MAX_CALC_RAY_VAL	= SHADOW_MAX_CALC_RAYS - 1;

	class ShadowObstacleCatcher : public CollisionCallback
	{
	public:
		ShadowObstacleCatcher() : hit_( false ) {}

		void direction( const Vector3& dir )	{ dir_ = dir; }
		bool hit() { return hit_; }
		void hit( bool h ) { hit_ = h; }
		float dist() const	{	return dist_;	}
	private:
		virtual int operator()( const ChunkObstacle & obstacle,
			const WorldTriangle & triangle, float dist )
		{
			BW_GUARD;

			dist_ = dist;
			if (!(triangle.flags() & TRIANGLE_TERRAIN))
			{
				// TODO: New terrain only requires self-intersection, and the
				// code below should be skipped for the new terrain, and should
				// be called for the old terrain.

				// Don't collide with models that are really close
				//if (dist < .5f)
				//	return COLLIDE_ALL;

				// protect vs. deleted items
				if (!obstacle.pItem()->chunk())
					return COLLIDE_ALL;				

				// Make sure it's a model, ie, we don't want entities casting
				// shadows
				/*DataSectionPtr ds = obstacle.pItem()->pOwnSect();
				if (!ds || ds->sectionName() != "model")
					return COLLIDE_ALL;*/

				if( !obstacle.pItem()->edAffectShadow() )
					return COLLIDE_ALL;

				// Check that we don't hit back facing triangles.				
				//Vector3 trin = obstacle.transform_.applyVector( triangle.normal() );
				//if ( trin.dotProduct( dir_ ) >= 0.f )
				//	return COLLIDE_ALL;
			}

			// Transparent BSP does not cast shadows
			if( triangle.flags() & TRIANGLE_TRANSPARENT )
				return COLLIDE_ALL;

			hit_ = true;

			// We only require the first collision for shadow.
			return COLLIDE_STOP;
		}

		bool hit_;
		float dist_;
		Vector3 dir_;
	};

	std::pair<int, Vector3> 
    getMinVisibleAngle0
    ( 
        Chunk          *pChunk, 
        Vector3        const &sPolePos, 
        Vector3        const &lastPos, 
        bool           reverse
    )
	{
		BW_GUARD;

		// Cast up to SHADOW_RAYS_IN_CALCULATION rays, from horiz over a 180
		// deg range, finding the 1st one that doesn't collide (do it twice
		// starting from diff directions each time)

		// We've gotta add a lil to the z, or when checking along chunk boundaries
		// we don't hit the terrain block
		Vector3 polePos = sPolePos + Vector3(0.0f, 0.1f, 0.0001f);

		// apply the chunk's xform to polePos to get it's world pos
		polePos = pChunk->transform().applyPoint( polePos );

		// adding an offset to avoid errors in the collisions at chunk seams
		polePos.z += SHADOW_RAY_ERROR_OFFSET;

		Vector3 lastHit = lastPos;
		float xdiff = reverse ? polePos.x - lastPos.x : lastPos.x - polePos.x;
		float ydiff = lastPos.y - polePos.y;
		bool valid = lastPos.y > polePos.y;
		float lastAngle = atan2f( ydiff, xdiff );
		if( lastAngle < 0 )
			lastAngle += MATH_PI / 2;

		for (int i = 0; i < SHADOW_MAX_CALC_RAYS; i++)
		{
			// sun travels along the x axis (thus around the z axis)

			// Make a ray that points the in correct direction
			float angle = MATH_PI * (i / float(SHADOW_MAX_CALC_RAY_VAL) );

			if( valid && angle < lastAngle )
				continue;

			Vector3 ray ( cosf( angle ), sinf( angle ), 0.f );

			if (reverse)
				ray.x *= -1;

			if (s_debugShadowRays)
			{
				s_lines.push_back( polePos );
				s_lineEnds.push_back( polePos + (ray * MAX_TERRAIN_SHADOW_RANGE) );
			}

			ShadowObstacleCatcher soc;
			soc.direction(ray);

			Terrain::TerrainHeightMap2::optimiseCollisionAlongZAxis( true );

			ChunkManager::instance().cameraSpace()->collide( 
				polePos,
				polePos + (ray * MAX_TERRAIN_SHADOW_RANGE),
				soc );

			Terrain::TerrainHeightMap2::optimiseCollisionAlongZAxis( false );

			if (!soc.hit())
				return std::make_pair( i, lastHit );
			else
				lastHit = Vector3( polePos + soc.dist() * ray );

			WorldManager::instance().fiberPause();
		}

		return std::make_pair( 0xff, lastHit );
	}

	std::pair<int, Vector3> 
    getMinVisibleAngle1
    ( 
        Chunk           *pChunk, 
        Vector3         const &sPolePos,  
        Vector3         const &lastPos, 
        bool            reverse 
    )
	{
		BW_GUARD;

		// cast up to SHADOW_RAYS_IN_CALCULATION rays, from horiz over a 90
		// deg range, finding the 1st one that doesn't collide (do it twice
		// starting from diff directions each time)

		// We've gotta add a lil to the z, or when checking along chunk boundaries
		// we don't hit the terrain block

		Vector3 polePos = sPolePos + Vector3(0.0f, 0.1f, 0.0001f);

		// apply the chunk's xform to polePos to get it's world pos
		polePos = pChunk->transform().applyPoint( polePos );

		// adding an offset to avoid errors in the collisions at chunk seams
		polePos.z += SHADOW_RAY_ERROR_OFFSET;

		Vector3 lastHit = lastPos;
		float xdiff = reverse ? polePos.x - lastPos.x : lastPos.x - polePos.x;
		float ydiff = lastPos.y - polePos.y;
		bool valid = lastPos.y > polePos.y;
		float lastAngle = atan2f( ydiff, xdiff );
		if( lastAngle < 0 )
			lastAngle += MATH_PI / 2;

		for (int i = 0; i < SHADOW_MAX_CALC_RAYS; i++)
		{			
			// sun travels along the x axis (thus around the z axis)

			// Make a ray that points the in correct direction
			float angle = MATH_PI * (i / float( SHADOW_MAX_CALC_RAY_VAL ) );

			//if( valid && angle < lastAngle )
			//	continue;

			float nRays = max(1.f, (SHADOW_MAX_CALC_RAY_VAL - i) / s_raySubdivision);
			float dAngle = ((MATH_PI/2.f) - angle) / nRays;
			float dDispl = (MAX_TERRAIN_SHADOW_RANGE / nRays) * s_rayDisplacement;

			Vector3 start( polePos );
			ShadowObstacleCatcher soc;			

			for (int r=0; r < nRays; r++)
			{
				angle = (MATH_PI * (i/float(SHADOW_MAX_CALC_RAY_VAL))) + (r * dAngle);
				Vector3 ray ( cosf(angle), sinf(angle), 0.f );

				if (reverse)
					ray.x *= -1;				

				if (s_debugShadowRays)
				{
					s_lines.push_back( start );
					s_lineEnds.push_back( start + (ray * dDispl) );
				}

				soc.direction(ray);

				Terrain::TerrainHeightMap2::optimiseCollisionAlongZAxis( true );

				ChunkManager::instance().cameraSpace()->collide( 
					start,
					start + (ray * dDispl),
					soc );

				Terrain::TerrainHeightMap2::optimiseCollisionAlongZAxis( false );

				if (soc.hit())
				{
					lastHit = start + soc.dist() * ray;
					break;
				}

				//always start slightly back along the line, so as
				//not to create a miniscule break in the line (i mean curve)
				start += (ray * dDispl * 0.999f);
			}			

			if (!soc.hit())
			{
				return std::make_pair( i, lastHit );
			}

			WorldManager::instance().fiberPause();
		}

		return std::make_pair( 0xff, lastHit );
	}

	void checkUIUpdate( DWORD & tick, ProgressTask * task )
	{
		BW_GUARD;

		DWORD currTick = GetTickCount();
		if ((DWORD)(currTick - tick) >= MAX_NO_RESPONDING_TIME)
		{
			tick = currTick;
			task->step( 0 );
		}

		WorldManager::processMessages();
	}

}


EditorChunkTerrain::EditorChunkTerrain():
    ChunkTerrain(),
    transform_(Matrix::identity),
    pOwnSect_(NULL),
	brushTextureExist_( false )
{
	BW_GUARD;

	// We want to pretend the origin is near (but not at) the centre so
	// everything rotates correctly when we have an even and an odd amount of 
	// terrain blocks selected
	transform_.setTranslate( 0.5f*GRID_RESOLUTION - 0.5f, 0.f, 0.5f*GRID_RESOLUTION - 0.5f );

	maxLayersWarning_ = new MaxLayersOverlay( this );

	if (s_collType < 0)
	{
		s_collType = 1;
		MF_WATCH
        ( 
            "Render/Terrain Shadows/debug shadow rays",
			s_debugShadowRays,
			Watcher::WT_READ_WRITE,
			"Enabled, this draws a visual representation of the rays used to "
			"calculate the shadows on the terrain." 
        );
		MF_WATCH
        ( 
            "Render/Terrain Shadows/coll type", 
            s_collType,
			Watcher::WT_READ_WRITE,
			"0 - 500m straight line rays, 1 - <500m curved rays" 
        );
		MF_WATCH
        ( 
            "Render/Terrain Shadows/ray subdv",
			s_raySubdivision,
			Watcher::WT_READ_WRITE,
			"If using curved rays, this amount changes the subdivision.  A "
			"lower value indicates more rays will be used."
        );
		MF_WATCH
        ( 
            "Render/Terrain Shadows/ray displ",
			s_rayDisplacement,
			Watcher::WT_READ_WRITE,
			"This value changes the lenght of the ray after the subdivisions. "
			"Use it to tweak how long the resultant rays are." 
        );
	}
}


EditorChunkTerrain::~EditorChunkTerrain()
{
	BW_GUARD;

	// removes the rendereable, just in case it's still hanging around.
	WorldManager::instance().removeRenderable( maxLayersWarning_ );
}


/**
 *  This function accesses the underlying EditorBaseTerrainBlock.
 *
 *  @returns            The underlying EditorBaseTerrainBlock.
 */
Terrain::EditorBaseTerrainBlock &EditorChunkTerrain::block()
{
	BW_GUARD;

    return *static_cast<Terrain::EditorBaseTerrainBlock *>(ChunkTerrain::block().getObject());
}


/**
 *  This function accesses the underlying EditorBaseTerrainBlock.
 *
 *  @returns            The underlying EditorBaseTerrainBlock.
 */
Terrain::EditorBaseTerrainBlock const &EditorChunkTerrain::block() const
{
	BW_GUARD;

    return *static_cast<Terrain::EditorBaseTerrainBlock const*>(ChunkTerrain::block().getObject());
}


/**
 *  This method should be called if you have changed the heights in a chunk
 *  terrain.  It makes the neighbouring changes borders match and it 
 *  recalculates the chunk's bounding box/collision scene.
 */
void EditorChunkTerrain::onEditHeights()
{
	BW_GUARD;

    // Lock the terrain height map for writing:
    Terrain::TerrainHeightMap &thm = block().heightMap();    
    { // scope to lock terrain height map
        Terrain::TerrainHeightMapHolder holder(&thm, false);
        // Coordinates of first height pole, first visible height pole, the 
        // last visible height pole, one past last height pole in x-direction.
		// It effectivelly copies one extra pole from the right chunk,
		// stitching both chunks to avoid seams
        /*uint32 x1  = 0;*/
        uint32 x2  = thm.xVisibleOffset();
        uint32 x3  = x2 + thm.verticesWidth() - 1;
        uint32 x4  = thm.polesWidth();

        // Coordinates of first height pole, first visible height pole, the 
        // last visible height pole, one past last height pole in z-direction.
		// It effectivelly copies one extra pole from the below chunk,
		// stitching both chunks to avoid seams
        /*uint32 z1  = 0;*/
        uint32 z2  = thm.zVisibleOffset();
        uint32 z3  = z2 + thm.verticesHeight() - 1;
        uint32 z4  = thm.polesHeight();

        // Copy heights from the left, right, above and below blocks:
        copyHeights(-1,  0, x3 - x2, 0      , x2 + 1 , z4     , 0  , 0 ); // left
        copyHeights(+1,  0, x2     , 0      , x4 - x3, z4     , x3 , 0 ); // right
        copyHeights( 0, -1, 0      , z3 - z2, x4     , z2 + 1 , 0  , 0 ); // above
        copyHeights( 0, +1, 0      , z2     , x4     , z4 - z3, 0  , z3); // below

        // Copy heights from the diagonals:
        copyHeights(-1, -1, x3 - x2, z3 - z2, x2 + 1 , z2 + 1 , 0 , 0 ); // left & above
        copyHeights(+1, -1, x2     , z3 - z2, x4 - x3, z2 + 1  , x3, 0 ); // right & above
        copyHeights(-1, +1, x3 - x2, z2     , x2 + 1 , z4 - z3, 0 , z3); // left & below
        copyHeights(+1, +1, x2     , z2     , x4 - x3, z4 - z3, x3, z3); // right & below
    }

	// Finally fix our bounding box, the collision scene etc.
    this->onTerrainChanged();
}


/**
 *	This method is exclusive to editorChunkTerrian.  It calculates an image
 *	of relative elevation values for each pole.
 *
 *	@param relHeights   	An image with the relative height information.
 */
void 
EditorChunkTerrain::relativeElevations
(
    Terrain::TerrainHeightMap::ImageType &relHeights
) const
{
	BW_GUARD;

	Terrain::EditorBaseTerrainBlock const &b    = block();
    Terrain::TerrainHeightMap       const &cthm = b.heightMap();
    // Cast away const - we are only reading the terrain height anyway
    Terrain::TerrainHeightMap       &thm        = const_cast<Terrain::TerrainHeightMap &>(cthm);

    Terrain::TerrainHeightMapHolder holder(&thm, true); // lock/unlock height map
    relHeights = thm.image();

    // Find the average height:
    float average = 0.0f;
    for (uint32 h = 0; h < relHeights.height(); ++h)
    {
        for (uint32 w = 0; w < relHeights.width(); ++w)
        {
            average += relHeights.get(w, h);
        }
    }
    uint32 samples = relHeights.width()*relHeights.height();
    MF_ASSERT(samples != 0);
    average /= samples;

    // Subtract the average:
    for (uint32 h = 0; h < relHeights.height(); ++h)
    {
        for (uint32 w = 0; w < relHeights.width(); ++w)
        {
            float v = relHeights.get(w, h);
            relHeights.set(w, h, v - average);
        }
    }
}


/**
 *	This method calculates the slope at a height pole, and returns the
 *	angle in degrees.
 *
 *  @param xIdx     The x coordinate of the sample to get the slope at.
 *  @param zIdx     The z coordinate of the sample to get the slope at.
 *  @returns        The slope at the given height pole in degrees.
 */
float EditorChunkTerrain::slope(int xIdx, int zIdx) const
{
	BW_GUARD;

    Terrain::EditorBaseTerrainBlock const &b   = block();
    Terrain::TerrainHeightMap       const &thm = b.heightMap();
    float x = (float)xIdx/(float)thm.verticesWidth ();
    float z = (float)zIdx/(float)thm.verticesHeight();
    Vector3 normal = b.normalAt(x, z);
	return RAD_TO_DEG(acosf(Math::clamp(-1.0f, normal.y, +1.0f)));
}


/** 
 *  This calculates the dominant blend value at the given x, z.
 *
 *	@param x		The x coordinate to get the blend at.
 *	@param z		The z coordinate to get the blend at.
 *	@param strength	If not NULL then this is set to the strength of the 
 *					dominant blend.
 */
size_t EditorChunkTerrain::dominantBlend(float x, float z, uint8 *strength) const
{
	BW_GUARD;

    size_t result  = NO_DOMINANT_BLEND;
    uint8 maxValue = 0;

    Terrain::EditorBaseTerrainBlock const &b = block();
    for (size_t i = 0; i < b.numberTextureLayers(); ++i)
    {
        Terrain::TerrainTextureLayer const &ttl = b.textureLayer(i);
        Terrain::TerrainTextureLayerHolder 
            holder(const_cast<Terrain::TerrainTextureLayer *>(&ttl), true);
        Terrain::TerrainTextureLayer::ImageType const &image = ttl.image();
        int sx = (int)(x*image.width ()/GRID_RESOLUTION + 0.5f);
        int sz = (int)(z*image.height()/GRID_RESOLUTION + 0.5f);
        uint8 v = image.get(sx, sz);
        if (v > maxValue)
        {
            result   = i;
            maxValue = v;
			if (strength != NULL) *strength = v;
        }
    }

    return result;
}

// This define enables the following function and changes some sections of code in the
// calculateShadows() function below.
//#define PROFILE_SHADOW_CALCULATION

#ifdef PROFILE_SHADOW_CALCULATION
/**
 * Utility function that forces shadow calculation on one chunk.
 */
void ProfileShadowsOnCameraChunk()
{
	Chunk * cc = ChunkManager::instance().cameraChunk();
	if (cc)
	{
		static EditorChunkTerrain* pEct = 
			static_cast<EditorChunkTerrain*>
				( ChunkTerrainCache::instance( *cc ).pTerrain());
		MF_ASSERT( pEct );
		pEct->calculateShadows();
	}
}
#endif

/**
 *  This is called to recalculate the self-shadowing of the terrain.
 *
 *	@param canExitEarly		If true then early exit due to a change in the 
 *							working chunk is allowed.  If false then the shadow 
 *							must be fully calculated.
 */
void EditorChunkTerrain::calculateShadows(bool canExitEarly /*=true*/, ProgressTask * task /*= NULL*/)
{
	BW_GUARD;

	DWORD startTick = GetTickCount();

	Chunk* pChunk = chunk();

	MF_ASSERT( pChunk->isBound() );

	TRACE_MSG( "Calculating shadows for chunk %s\n", pChunk->identifier().c_str() );

    Terrain::EditorBaseTerrainBlock &b   = block();
    Terrain::HorizonShadowMap       &hsm = b.shadowMap();

	Terrain::HorizonShadowMap::HorizonShadowImage tempMap;
	{
		// get a copy of the map to avoid accessing this DX object from
		// two different fibers.
		Terrain::HorizonShadowMapHolder holder(&hsm, false);
		tempMap = hsm.image();
	}

    for (int32 z = 0; z < (int32)hsm.height(); ++z)
    {
	    Vector3 lastPos( 0.f, MIN_CHUNK_HEIGHT, 0.f );
	    for (int32 x = hsm.width() - 1; x >= 0; --x)		
	    {
			float chunkX = Terrain::BLOCK_SIZE_METRES*(float)x/(float)(hsm.width()-1);
			float chunkZ = Terrain::BLOCK_SIZE_METRES*(float)z/(float)(hsm.height()-1);
			float chunkY = b.heightAt( chunkX, chunkZ );

			Vector3 polePos = Vector3( chunkX, chunkY, chunkZ );

			if ( s_debugShadowPoles )
			{
				Vector3 pos = polePos;
				s_lines.push_back(    pChunk->transform().applyPoint( pos - Vector3( 0, 0, -2 ) ) );
				s_lineEnds.push_back( pChunk->transform().applyPoint( pos - Vector3( 0, 0, +2 ) ) );
				s_lines.push_back(    pChunk->transform().applyPoint( pos - Vector3( -2, 0, 0 ) ) );
				s_lineEnds.push_back( pChunk->transform().applyPoint( pos - Vector3( +2, 0, 0 ) ) );
				s_lines.push_back(    pChunk->transform().applyPoint( pos - Vector3( 0, -2, 0 ) ) );
				s_lineEnds.push_back( pChunk->transform().applyPoint( pos - Vector3( 0, +2, 0 ) ) );
				bool gotIn = false;
				// hold shift to completely stop for a while
				while ( GetAsyncKeyState( VK_SHIFT ) < 0 )
				{
					gotIn = true;
					Sleep( 10 );
				}
				if ( gotIn )
				{
					Sleep( 500 );
				}
				else
				{
					// hold ctrl to go step by step
					if ( GetAsyncKeyState( VK_CONTROL ) < 0 )
						Sleep( 40 );
				}
			}

		    std::pair<int, Vector3> result = 
                s_collType 
                    ? getMinVisibleAngle1( pChunk, polePos, lastPos, false ) 
                    : getMinVisibleAngle0( pChunk, polePos, lastPos, false );

		    int posAngle = SHADOW_MAX_CALC_RAY_VAL - result.first;
		    lastPos = result.second;

		    MF_ASSERT(posAngle >= 0 && posAngle <= SHADOW_MAX_CALC_RAY_VAL);

		    Terrain::HorizonShadowPixel shadow = tempMap.get( int( x ), int( z ) );
            shadow.west = posAngle * SHADOW_MAX_VALUE / SHADOW_MAX_CALC_RAY_VAL;
	        tempMap.set( int( x ), int( z ), shadow );

		    if( canExitEarly && !WorldManager::instance().isWorkingChunk( pChunk ) )
			    return;	
			if (task)
			{
				checkUIUpdate( startTick, task );
			}
	    }
		WorldManager::instance().escapePressed();
	    lastPos = Vector3( 0.f, MIN_CHUNK_HEIGHT, 0.f );
	    for (uint32 x = 0; x < hsm.width(); ++x)		
	    {		
            Terrain::HorizonShadowMap::Iterator pole = hsm.iterator(x, z);
			float chunkX = Terrain::BLOCK_SIZE_METRES*(float)x/(float)(hsm.width()-1);
			float chunkZ = Terrain::BLOCK_SIZE_METRES*(float)z/(float)(hsm.height()-1);
			float chunkY = b.heightAt( chunkX, chunkZ );

			Vector3 polePos = Vector3( chunkX, chunkY, chunkZ );

		    std::pair<int, Vector3> result = 
                s_collType 
                    ? getMinVisibleAngle1( pChunk, polePos, lastPos, true ) 
                    : getMinVisibleAngle0( pChunk, polePos, lastPos, true );
		    int negAngle = result.first;
		    lastPos = result.second;
		    MF_ASSERT(negAngle >= 0 && negAngle <= SHADOW_MAX_CALC_RAY_VAL);

		    Terrain::HorizonShadowPixel shadow = tempMap.get( int( x ), int( z ) );
            shadow.east = negAngle * SHADOW_MAX_VALUE / SHADOW_MAX_CALC_RAY_VAL;
	        tempMap.set( int( x ), int( z ), shadow );

#ifndef PROFILE_SHADOW_CALCULATION
			// When profiling, turn off fiber stuff to avoid issues with profiler only
			// seeing one thread.
		    if( canExitEarly && !WorldManager::instance().isWorkingChunk( pChunk ) )
			    return;	
#endif
			if (task)
			{
				checkUIUpdate( startTick, task );
			}
	    }
		WorldManager::instance().escapePressed();
		{
			// Blit the tempMap per line, to get a nice feedback.
			Terrain::HorizonShadowMapHolder holder(&hsm, false);
			Terrain::HorizonShadowMap::PixelType* src = tempMap.getRow( z );
			Terrain::HorizonShadowMap::PixelType* dst = hsm.image().getRow( z );
			Terrain::HorizonShadowMap::PixelType* end = dst + hsm.image().width();

			while ( dst != end )
				*dst++ = *src++;
		}
		if (task)
		{
			checkUIUpdate( startTick, task );
		}

	}

#ifndef PROFILE_SHADOW_CALCULATION
	// When profiling, turn off "dirty" state reset.
	WorldManager::instance().changedTerrainBlock( pChunk, /*rebuildNavmesh=*/false );
	WorldManager::instance().chunkShadowUpdated( pChunk );
#endif
}


/**
 *  This function gets the transformation of the EditorChunkTerrain.
 *
 *  @returns        The EditorChunkTerrain's transformation.
 */
/*virtual*/ const Matrix & EditorChunkTerrain::edTransform()	
{ 
    return transform_; 
}


/**
 *  This function applies a transformation to the EditorChunkTerrain.
 *
 *  @param m            The transformation to apply.
 *  @param transient    Is the transformation only temporary?
 *  @returns            True if successful.
 */
bool EditorChunkTerrain::edTransform(const Matrix & m, bool transient)
{
	BW_GUARD;

	Matrix newWorldPos;
	newWorldPos.multiply( m, pChunk_->transform() );

	// If this is only a temporary change, do nothing:
	if (transient)
	{
		transform_ = m;
		this->syncInit();
		return true;
	}

	// It's permanent, so find out which chunk we belong to now:
	Chunk *pOldChunk = pChunk_;
	Chunk *pNewChunk = 
        EditorChunk::findOutsideChunk( newWorldPos.applyToOrigin() );
	if (pNewChunk == NULL)
		return false;

	EditorChunkTerrain *newChunkTerrain = 
        (EditorChunkTerrain*)ChunkTerrainCache::instance( *pNewChunk ).pTerrain();

	// Don't allow two terrain blocks in a single chunk
	// We allow an exception for ourself to support tossing into our current chunk
	// We allow an exception for a selected item as it implies we're moving a group of
	// terrain items in a direction, and the current one likely won't be there any longer.
	if 
    (
        newChunkTerrain 
        && 
        newChunkTerrain != this 
        &&
		!WorldManager::instance().isItemSelected( newChunkTerrain ) 
    )
    {
		return false;
    }


	// Make sure that the chunks aren't read only:
	if 
    (
        !EditorChunkCache::instance( *pOldChunk ).edIsWriteable() 
		|| 
        !EditorChunkCache::instance( *pNewChunk ).edIsWriteable()
    )
    {
		return false;
    }

	transform_ = Matrix::identity;
	transform_.setTranslate( 0.5f*GRID_RESOLUTION - 0.5f, 0.f, 0.5f*GRID_RESOLUTION - 0.5f );

	// Note that both affected chunks have seen changes
	WorldManager::instance().changedChunk( pOldChunk );
	WorldManager::instance().changedChunk( pNewChunk );

	WorldManager::instance().markTerrainShadowsDirty( pOldChunk );
	WorldManager::instance().markTerrainShadowsDirty( pNewChunk );

	edMove( pOldChunk, pNewChunk );

	// Ok, now apply any rotation

	// Calculate rotation angle
	Vector3 newPt = m.applyVector( Vector3( 1.f, 0.f, 0.f ) );
	newPt.y = 0.f;

	float angle = acosf( Math::clamp(-1.0f, newPt.dotProduct( Vector3( 1.f, 0.f, 0.f ) ), +1.0f) );

	if (!almostZero( angle, 0.0001f) )
	{
		// dp will only give an angle up to 180 degs, make it from 0..2PI
		if (newPt.crossProduct(Vector3( 1.f, 0.f, 0.f )).y > 0.f)
			angle = (2 * MATH_PI) - angle;
	}	

	// Turn the rotation angle into an amount of 90 degree rotations
	int rotateAmount = 0;

	if (angle > (DEG_TO_RAD(90.f) / 2.f))
		++rotateAmount;
	if (angle > (DEG_TO_RAD(90.f) + (DEG_TO_RAD(90.f) / 2.f)))
		++rotateAmount;
	if (angle > (DEG_TO_RAD(180.f) + (DEG_TO_RAD(90.f) / 2.f)))
		++rotateAmount;
	if (angle > (DEG_TO_RAD(270.f) + (DEG_TO_RAD(90.f) / 2.f)))
		rotateAmount = 0;

	if (rotateAmount > 0)
	{
		MF_ASSERT( rotateAmount < 4 );

	    UndoRedo::instance().add(new TerrainHeightMapUndo(&block(), pChunk_));

		// Rotate terrain 90 degress rotateAmount amount of times

        Terrain::TerrainHeightMap &thm = block().heightMap();        
        {
            Terrain::TerrainHeightMapHolder holder(&thm, false);
            Terrain::TerrainHeightMap::ImageType &heights = thm.image();
            MF_ASSERT(heights.width() == heights.height());
            for (int r = 0; r < rotateAmount; ++r)
            {                
                // A slow but sure way of rotating:
                Terrain::TerrainHeightMap::ImageType heightsCopy(heights);
                MF_ASSERT(0); // TODO: I'm not sure if the rotate direction is ok.
                heightsCopy.rotate(true);
                heights.blit(heightsCopy);
            }
        } // rebuild DX objects for the terrain
    	WorldManager::instance().changedTerrainBlock( pChunk_ );
	}
	this->syncInit();
	return true;
}


/**
 *  This method gets the bounds of the terrain.
 */
void EditorChunkTerrain::edBounds(BoundingBox &bbRet) const
{
	BW_GUARD;

    // I removed the hard-coded numbers, but I'm not sure why we add/subtract
    // 0.5 or why 0.25 was chosen as the expansion number in the y-directions.
    Vector3 minb = 
        bb_.minBounds() + 
            Vector3
            ( 
                -0.5f*GRID_RESOLUTION + 0.5f, 
                -0.25f,
                -0.5f*GRID_RESOLUTION + 0.5f
            );
    Vector3 maxb =
        bb_.maxBounds() + 
            Vector3
            ( 
                -0.5f*GRID_RESOLUTION - 0.5f,  
                0.25f, 
                -0.5f*GRID_RESOLUTION - 0.5f 
            );
	bbRet.setBounds(minb, maxb);
}


bool EditorChunkTerrain::edSave( DataSectionPtr pSection )
{
	BW_GUARD;

	edCommonSave( pSection );
	pSection->writeString( "resource", pChunk_->identifier() + ".cdata/" + block().dataSectionName() );
	return true;
}


DataSectionPtr EditorChunkTerrain::pOwnSect()
{ 
    return pOwnSect_; 
}


const DataSectionPtr EditorChunkTerrain::pOwnSect()	const 
{ 
    return pOwnSect_; 
}


void EditorChunkTerrain::edPreDelete()
{
	EditorChunkItem::edPreDelete();
	WorldManager::instance().markTerrainShadowsDirty( this->chunk() );
	WorldManager::instance().changedTerrainBlock( this->chunk() );
}


void EditorChunkTerrain::edPostCreate()
{
	WorldManager::instance().markTerrainShadowsDirty( this->chunk() );
	WorldManager::instance().changedTerrainBlock( this->chunk() );
	EditorChunkItem::edPostCreate();
}


void EditorChunkTerrain::edPostClone(EditorChunkItem* srcItem)
{
	BW_GUARD;

	EditorChunkItem::edPostClone( srcItem );
	WorldManager::instance().markTerrainShadowsDirty( this->chunk() );
	WorldManager::instance().changedTerrainBlock( this->chunk() );
}


BinaryPtr EditorChunkTerrain::edExportBinaryData()
{
	BW_GUARD;

	// Return a copy of the terrain block
	std::string terrainName = "export_temp." + this->block().dataSectionName();

	BWResource::instance().purge( terrainName, true );
	std::wstring fname;
	bw_utf8tow( BWResource::resolveFilename( terrainName ), fname );
	::DeleteFile( fname.c_str() );

	block().save( terrainName );

	BinaryPtr bp;
	{
		DataSectionPtr ds = BWResource::openSection( terrainName );
		bp = ds->asBinary();
	}

	::DeleteFile( fname.c_str() );

	return bp;
}


bool EditorChunkTerrain::edShouldDraw()
{
	BW_GUARD;

	if( !ChunkTerrain::edShouldDraw() )
		return false;

	return OptionsTerrain::visible();
}


/**
 *	The 20000 y value is pretty much just for prefabs, so that when placing it,
 *	it'll most likely end up at y = 0.f, if it's anything else items will not
 *	be sitting on the ground.
 */
Vector3 EditorChunkTerrain::edMovementDeltaSnaps()
{
	return Vector3( GRID_RESOLUTION, 20000.f, GRID_RESOLUTION );
}


float EditorChunkTerrain::edAngleSnaps()
{
	return 90.f;
}


void EditorChunkTerrain::draw()
{
	BW_GUARD;

	if( !edShouldDraw() )
		return;
	if (block().holeMap().allHoles())
		return;

	// Draw using our transform, to show when the user is dragging the block around
	Matrix m = transform_;
	//m.setTranslate( m[3] - Vector3( GRID_RESOLUTION, 0.f, GRID_RESOLUTION ) );
	m.setTranslate( m[3] - Vector3( 0.5f*GRID_RESOLUTION - 0.5f, 0.f, 0.5f*GRID_RESOLUTION - 0.5f ) );

	m.preMultiply( Moo::rc().world() );

	bool isWriteable = EditorChunkCache::instance( *chunk() ).edIsWriteable();

	bool shadeReadOnlyBlocks = OptionsMisc::readOnlyVisible();
	bool projectModule = ProjectModule::currentInstance() == ModuleManager::instance().currentModule();
	if( !isWriteable && WorldManager::instance().drawSelection() )
		return;

	if (WorldManager::instance().drawSelection())
	{
		WorldManager::instance().registerDrawSelectionItem( this );
	}

	if (!isWriteable && shadeReadOnlyBlocks && !projectModule )
	{
        WorldManager::instance().addReadOnlyBlock( m, ChunkTerrain::block() );
	}
	else
	{
		if (OptionsTerrain::numLayersWarningVisible() &&
			PanelManager::pInstance() &&
			PanelManager::instance().currentTool() == L"TerrainTexture")
		{
			// Show/hide too-many-textures warning overlay
			int maxNumLayers = OptionsTerrain::numLayersWarning();
			if ( maxNumLayers > 0 &&
				(int)block().numberTextureLayers() >= maxNumLayers )
			{
				// add the "maxLayersWarning_" rendereable, which will be rendered
				// next frame, and will then remove itself from WorldEditor's
				// rendereables list.
				WorldManager::instance().addRenderable( maxLayersWarning_ );
			}
		}

		// Add the terrain block to the terrain's drawlist.
		Terrain::BaseTerrainRenderer::instance()->addBlock( &block(), m );
	}

	// draw debug lines
	if (!s_lines.empty())
	{
		for (uint32 i=0; i<s_lines.size(); i++)
		{
			LineHelper::instance().drawLine(s_lines[i], s_lineEnds[i], i % 2 ? 0xffffffff : 0xffff8000 );
		}
		LineHelper::instance().purge();
		s_lines.clear();
		s_lineEnds.clear();
	}
}


/**
 *	Handle ourselves being added to or removed from a chunk
 */
void EditorChunkTerrain::toss( Chunk * pChunk )
{
	BW_GUARD;

	Chunk* oldChunk = pChunk_;


	if (pChunk_ != NULL)
	{
		if (pOwnSect_)
		{
			EditorChunkCache::instance( *pChunk_ ).
				pChunkSection()->delChild( pOwnSect_ );
			pOwnSect_ = NULL;
		}
	}


	ChunkTerrain::toss( pChunk );

	if (pChunk_ != NULL)
	{
		std::string newResourceID = pChunk_->mapping()->path() + 
			pChunk_->identifier() + ".cdata/" + block().dataSectionName();

		if (block().resourceName() != newResourceID)
		{
			block().resourceName(newResourceID);
			WorldManager::instance().changedTerrainBlock( pChunk_ );
			WorldManager::instance().changedChunk( pChunk_ );
		}

		if (!pOwnSect_)
		{
			pOwnSect_ = EditorChunkCache::instance( *pChunk_ ).
				pChunkSection()->newSection( "terrain" );
		}

		this->edSave( pOwnSect_ );
	}

	// If there are any other terrain blocks in our old chunk, let them be the
	// one the terrain item cache knows about
	if (oldChunk)
	{
		EditorChunkCache::instance( *oldChunk ).fixTerrainBlocks();
	}
}


/**
 *	This gets the neighbouring terrain.
 *
 *  @param n			The neighbour to query for.
 *  @param forceLoad	Load the neighbour if not in memory?
 *	@param writable 	Does the neighbour need to be writable?
 *	@param isValid		If non-null then this is set to a member of 
 *						NeighbourError signifying whether the neighbour exists,
 *						does not exist or is locked.
 *  @returns			The neighbouring block.  This can be NULL at the edge 
 *						of the space, or if not loaded and forceLoad is false.
 */
EditorChunkTerrain *
EditorChunkTerrain::neighbour
(
	Neighbour			n, 
	bool				forceLoad		/*= false*/,
	bool				writable		/*= true*/,
	NeighbourError		*isValid		/*= NULL*/
) const
{
	BW_GUARD;

	// Calculate deltas:
	int dx = 0;
	int dz = 0;
	switch (n)
	{
	case NORTH_WEST:
		dx = -1; dz = +1;
		break;
	case NORTH:
		dx =  0; dz = +1;
		break;
	case NORTH_EAST:
		dx = +1; dz = +1;
		break;
	case EAST:
		dx = +1; dz =  0;
		break;
	case SOUTH_EAST:
		dx = +1; dz = -1;
		break;
	case SOUTH:
		dx =  0; dz = -1;
		break;
	case SOUTH_WEST:
		dx = -1; dz = -1;
		break;
	case WEST:
		dx = -1; dz =  0;
		break;
	}

	// Calculate the center of the neighbour:
    Vector3 cen  = (bb_.maxBounds() + bb_.minBounds()) * 0.5f;
	Vector3 delt = bb_.maxBounds() - bb_.minBounds();
    Vector3 lpos = cen + Vector3(dx*delt.x, 0.0f, dz*delt.z);
    Vector3 wpos = pChunk_->transform().applyPoint(lpos);

	// Find the neighbouring chunk:
	GeometryMapping *mapping = WorldManager::instance().geometryMapping();
	std::string chunkName = mapping->outsideChunkIdentifier(wpos);
	if (chunkName.empty())
	{
		if (isValid != NULL) 
			*isValid = NEIGHBOUR_NONEXISTANT;
		return NULL;
	}
	Chunk *chunk = 
		ChunkManager::instance().findChunkByName(chunkName, mapping, false);
	if (chunk == NULL)
	{
		if (isValid != NULL) 
			*isValid = NEIGHBOUR_NONEXISTANT;
		return NULL;
	}
	if (writable && !chunkWritable(chunk, false))
	{
		if (isValid != NULL) 
			*isValid = NEIGHBOUR_LOCKED;
		return NULL;
	}
	// Handle the case where the chunk is not loaded:
	if (!chunk->loaded())
	{
		// Chunk not loaded and we are not forcing into memory?
		if (!forceLoad)
		{
			if (isValid != NULL) 
				*isValid = NEIGHBOUR_NOTLOADED;
			return NULL;
		}
		// Force the chunk into memory:
		SyncMode chunkStopper;
		ChunkManager::instance().loadChunkNow(chunkName, mapping);
		ChunkManager::instance().checkLoadingChunks();
	}

	// Finally, get the terrain:
	if (isValid != NULL) 
		*isValid = NEIGHBOUR_OK;	
	return 
		static_cast<EditorChunkTerrain*>
        (
            ChunkTerrainCache::instance(*chunk).pTerrain()
        );
}


/*static*/ Terrain::EditorBaseTerrainBlockPtr 
EditorChunkTerrain::loadBlock
(
    std::string         const &	resource,
    EditorChunkTerrain*			ect,
	const Vector3&				worldPosition			
)
{
	BW_GUARD;

	Terrain::EditorBaseTerrainBlockPtr result;

	std::string verResource = resource;
	uint32 ver = Terrain::BaseTerrainBlock::terrainVersion( verResource );

	Terrain::TerrainSettingsPtr pSettings = WorldManager::instance().pTerrainSettings();

	if (pSettings->pRenderer() == NULL)
	{
		if (pSettings->version() == 200)
		{
			ERROR_MSG( "Failed to initialise advanced terrain. Please make sure Shader Model 2.0 or higher is used.\n" );
		}
		else
		{
			ERROR_MSG( "Failed to initialise terrain.\n" );
		}
		return NULL;
	}

	if ( ver == 100 )
	{
		result = new Terrain::TERRAINBLOCK1();
	}
	else if ( ver == 200 )
	{
		result = new Terrain::TERRAINBLOCK2(pSettings);
	}
	else
	{
		// It's not a normal terrain block (i.e. from a .cdata file). Find
		// out if it's a prefab or a cloned block, and find out which version
		// is needed.
		verResource = resource;
		uint32 prefabVer = prefabVersion( verResource );

		uint32 currentTerrainVer = pSettings->version();
		if ( (prefabVer == 100 && currentTerrainVer != 100) ||
			 (prefabVer == 200 && currentTerrainVer != 200) )
		{
			ERROR_MSG( "EditorChunkTerrain::loadBlock: Failed to load %s, "
				"prefab is version %d while the current terrain is version %d.\n",
				resource.c_str(), prefabVer, currentTerrainVer );
			return NULL;
		}

		if ( prefabVer == 100 )
		{
			result = new Terrain::TERRAINBLOCK1();
		}
		else if ( prefabVer == 200 )
		{
			result = new Terrain::TERRAINBLOCK2(pSettings);
		}
		else
		{
			ERROR_MSG( "EditorChunkTerrain::loadBlock: Failed to load %s, unknown terrain version.\n", resource.c_str() );
			return NULL;
		}
	}

	result->resourceName( verResource );

	Matrix worldTransform;
	worldTransform.setTranslate( worldPosition );

	bool ok = result->load(
		verResource,
		worldTransform,
		ChunkManager::instance().cameraTrans().applyToOrigin(),
		NULL );

	return ok ? result : NULL;
}


/**
 *  Used when loading from a prefab, to get the terrain data.
 */ 
extern DataSectionPtr prefabBinarySection;


/**
 *  Validates that a prefab's terrain resource section is of the same version
 *  as the current terrain version, and returns the appropriate terrain section
 *  name in the 'resource' param
 *  
 *  @param resource       resource sub-section from the prefab's terrain
 *                        section, and contains the returned terrain section
 *                        name.
 *  @returns              true if the prefab's terrain is compatible with the
 *                        current terrain.
 */
bool EditorChunkTerrain::validatePrefabVersion( std::string& resource )
{
	BW_GUARD;

	size_t lastSlash = resource.find_last_of( '/' );
	if ( lastSlash != std::string::npos )
		resource = resource.substr( lastSlash + 1 );

	uint32 prefabVer;
	if ( resource.length() >= 7 && resource.substr( resource.length() - 7 ) == "terrain" )
		prefabVer = 100;
	else if ( resource.length() >= 8 && resource.substr( resource.length() - 8 ) == "terrain2" )
		prefabVer = 200;
	else
		return false;

	uint32 currentVer = WorldManager::instance().pTerrainSettings()->version();
	if ( prefabVer != currentVer )
		return false;

	return true;
}


/**
 *	This method replaces ChunkTerrain::load
 */
bool 
EditorChunkTerrain::load
( 
    DataSectionPtr      pSection, 
    Chunk               *pChunk, 
    std::string         *errorString 
)
{
	BW_GUARD;

	if (!Terrain::BaseTerrainRenderer::instance()->version())
	{// terrain setting initialisation failure
		return false;
	}

	if (!pChunk->isOutsideChunk())
	{
		std::string err = "Can't load terrain block into an indoor chunk";
		if ( errorString )
		{
			*errorString = err;
		}
		ERROR_MSG( "%s\n", err.c_str() );
		return false;
	}

	edCommonLoad( pSection );

	pOwnSect_ = pSection;

	std::string prefabBinKey = pOwnSect_->readString( "prefabBin" );
	if (!prefabBinKey.empty())
	{
		// Loading from a prefab, source our terrain block from there
		if (!prefabBinarySection)
		{
			std::string err = "Unable to load prefab. Section prefabBin found, but no current prefab binary section";
			if ( errorString )
			{
				*errorString = err;
			}
			ERROR_MSG( "%s\n", err.c_str() );
			return false;
		}

		BinaryPtr bp = prefabBinarySection->readBinary( prefabBinKey );

		if (!bp)
		{
			std::string err = "Unable to load " + prefabBinKey + "  from current prefab binary section";
			if ( errorString )
			{
				*errorString = err;
			}
			ERROR_MSG( "%s\n", err.c_str() );
			return false;
		}

		// Write the binary out to a temp file and read the block in from there
		std::string terrainRes = pSection->readString( "resource", "" );
		if ( !validatePrefabVersion( terrainRes ) )
		{
			std::string err =
				"Prefab's terrain version different than the current terrain version "
				"('" + terrainRes + "')";
			if ( errorString )
			{
				*errorString = err;
			}
			ERROR_MSG( "%s\n", err.c_str() );
			return false;
		}

		size_t lastSlash = terrainRes.find_last_of( '/' );
		if ( lastSlash != std::string::npos )
			terrainRes = terrainRes.substr( lastSlash + 1 );

		std::string terrainName = "prefab_temp." + terrainRes;

		BWResource::instance().purge( terrainName, true );

		std::wstring fname;
		bw_utf8tow( BWResource::resolveFilename( terrainName ), fname );

		FILE* pFile = _wfopen( fname.c_str(), L"wb" );

		if ( pFile )
		{
			fwrite( bp->data(), bp->len(), 1, pFile );	
			fclose( pFile );

			block_ = EditorChunkTerrain::loadBlock( terrainName, this, 
						pChunk->transform().applyToOrigin() );

			BWResource::instance().purge( terrainName, true );
			::DeleteFile( fname.c_str() );

			pOwnSect_->delChild( "prefabBin" );
		}

		if ( !block_ )
		{
			std::string err = "Could not load prefab's terrain block. "
				"The prefab terrain is corrupted, or has different map sizes "
				"than the current space.";
			if ( errorString )
			{
				*errorString = err;
			}
			ERROR_MSG( "%s\n", err.c_str() );
			return false;
		}
	}
	else
	{
		// Loading as normal or cloning, check

		EditorChunkTerrain* currentTerrain = (EditorChunkTerrain*)
			ChunkTerrainCache::instance( *pChunk ).pTerrain();

		// If there's already a terrain block there, that implies we're cloning
		// from it.
		if (currentTerrain)
		{
			// Poxy hack to copy to terrain block
			std::string terrainRes = pSection->readString( "resource", "" );
			if ( !validatePrefabVersion( terrainRes ) )
			{
				// this should never happen, but still need to do the validate
				// to get the correct resource name in terrainRes
				std::string err =
					"Cloned terrain's version different than the current terrain version "
					"('" + terrainRes + "')";
				if ( errorString )
				{
					*errorString = err;
				}
				ERROR_MSG( "%s\n", err.c_str() );
				return false;
			}

			std::string terrainName = "clone_temp." + terrainRes;

			BWResource::instance().purge( terrainName );

			currentTerrain->block().rebuildLodTexture( pChunk->transform() );
			currentTerrain->block().save( terrainName );
			block_ = EditorChunkTerrain::loadBlock( terrainName, this, Vector3(0,0,0)  );

			std::wstring fname;
			bw_utf8tow( BWResource::resolveFilename( terrainName ), fname );
			::DeleteFile( fname.c_str() );

			if (!block_)
			{
				std::string err = "Could not clone terrain block ";
				if ( errorString )
				{
					*errorString = err;
				}
				ERROR_MSG( "%s\n", err.c_str() );
				return false;
			}
		}
		else
		{
			std::string resName = pChunk->mapping()->path() + 
										pSection->readString( "resource" );
			// Allocate the terrainblock.
			block_ = EditorChunkTerrain::loadBlock( resName, this, 
										pChunk->transform().applyToOrigin() );

			if (!block_)
			{
				std::string err = "Could not load terrain block '" 
										+ resName + "'";
				if ( errorString )
				{
					*errorString = err;
				}
				ERROR_MSG( "%s\n", err.c_str() );
				return false;
			}
		}
	}

	static_cast<Terrain::EditorBaseTerrainBlock*>
		(block_.getObject())->setSelectionKey( (DWORD)this );
    calculateBB();
    #if UMBRA_ENABLE
	if (ChunkUmbra::softwareMode() )
		{										
			this->block_->createUMBRAMesh( umbraMesh_ );
			this->umbraHasHoles_ = !this->block_->holeMap().noHoles();
		}			
	#endif
	return true;
}


/**
 *  This function finds the chunk terrain (chunkDx, chunkDy) relative to this 
 *  one, and if it exists, copies the given source rectangle of its height data 
 *  to the destination location.
 *
 *  @param chunkDx              How many chunks across?
 *  @param chunkDy              How many chunks down/up?
 *  @param srcX                 The source rectangle's left coordinate.
 *  @param srcY                 The source rectangle's top coordinate.
 *  @param srcW                 The source rectangle's width.
 *  @param srcH                 The source rectangle's height.
 *  @param dstX                 The destination rectangle's left coordinate.
 *  @param dstY                 The destination rectangle's right coordinate.
 */
void EditorChunkTerrain::copyHeights
(
    int32                       chunkDx, 
    int32                       chunkDz, 
    uint32                      srcX,
    uint32                      srcZ,
    uint32                      srcW,
    uint32                      srcH,
    uint32                      dstX,
    uint32                      dstZ
)
{
	BW_GUARD;

	// Find the chunk/terrain (chunkDx, chunkDz) coordinates away.
	GeometryMapping *mapping = WorldManager::instance().geometryMapping();
	int16 cx, cz;
	if (!mapping->gridFromChunkName(pChunk_->identifier(), cx, cz))
	{
		return;
	}
	cx += (int16)chunkDx;
	cz += (int16)chunkDz;
	std::string nchunkId;
	::chunkID(nchunkId, cx, cz);
	Chunk *chunk = 
		ChunkManager::instance().findChunkByName
		(
			nchunkId,
			WorldManager::instance().geometryMapping()
		);
	if (chunk == NULL)
		return;
	ChunkTerrain *chunkTerrain = 
		ChunkTerrainCache::instance(*chunk).pTerrain();
	if (chunkTerrain == NULL)
		return;
	Terrain::BaseTerrainBlockPtr other = 
		static_cast<Terrain::EditorBaseTerrainBlock *>(chunkTerrain->block().getObject());

    // Lock this chunk's height map for writing:
    Terrain::TerrainHeightMap            &thm     = block().heightMap();
    Terrain::TerrainHeightMapHolder      dstHolder(&thm, false);
    Terrain::TerrainHeightMap::ImageType &heights = thm.image();

    // Lock the other chunk's height for reading only:
    Terrain::TerrainHeightMap            &otherTHM     = other->heightMap();
    Terrain::TerrainHeightMapHolder      srcHolder(&otherTHM, true);
    Terrain::TerrainHeightMap::ImageType &otherHeights = otherTHM.image();

    heights.blit(otherHeights, srcX, srcZ, srcW, srcH, dstX, dstZ);
}


class TerrainItemMatrix : public ChunkItemMatrix
{
public:
	TerrainItemMatrix( ChunkItemPtr pItem ) : ChunkItemMatrix( pItem )
	{
		// Remove snaps, we don't want them as we're pretending the origin is
		// at 50,0,50
		movementSnaps( Vector3( 0.f, 0.f, 0.f ) );
	}

	/**
	 *	Overriding base class to not check the bounding box is < 100m, we just
	 *	don't care for terrain items
	 */
	bool EDCALL setMatrix( const Matrix & m )
	{
		BW_GUARD;

		pItem_->edTransform( m, true );
	}
};


/*static*/ uint32 EditorChunkTerrain::prefabVersion( std::string& resource )
{
	BW_GUARD;

	if ( resource.substr( resource.length() - 8 ) == ".terrain" != NULL )
	{
		resource += "/terrain";
		return 100;
	}
	else if ( resource.substr( resource.length() - 9 ) == ".terrain2" != NULL )
	{
		resource += "/terrain2";
		return 200;
	}

	return 0;
}


void EditorChunkTerrain::syncInit()
{
	BW_GUARD;

	#if UMBRA_ENABLE
		if (ChunkUmbra::softwareMode() )
		{										
			this->block_->createUMBRAMesh( umbraMesh_ );
			this->umbraHasHoles_ = !this->block_->holeMap().noHoles();
		}				
	#endif
	ChunkTerrain::syncInit();
}


/* Regenerate the bounding box and umbra model */
void EditorChunkTerrain::onTerrainChanged()
{
	BW_GUARD;

	this->calculateBB();
	this->syncInit();
}


/**
 *	Disable hide/freeze operations on terrain
 */
void EditorChunkTerrain::edHidden( bool value )
{
	WARNING_MSG( "Failed to hide terrain. Terrain cannot be hidden.\n" );
}


/**
 *	Disable hide/freeze operations on terrain
 */
bool EditorChunkTerrain::edHidden() const
{
	return false;
}


/**
 *	Disable hide/freeze operations on terrain
 */
void EditorChunkTerrain::edFrozen( bool value )
{
	WARNING_MSG( "Failed to freeze terrain. Terrain cannot be frozen.\n" );
}


/**
 *	Disable hide/freeze operations on terrain
 */
bool EditorChunkTerrain::edFrozen() const
{
	return false;
}

/**
 *	Returns a light container used to visualise dynamic lights on this object.
 */
Moo::LightContainerPtr EditorChunkTerrain::edVisualiseLightContainer()
{
	if (!chunk() || !block_)
		return NULL;

	Moo::LightContainerPtr lc = new Moo::LightContainer;

	BoundingBox bb = this->block_->boundingBox();
	bb.transformBy( this->chunk()->transform() );
	lc->init( ChunkLightCache::instance( *chunk() ).pAllLights(), bb );

	return lc;
}


// Use macros to write EditorChunkTerrain's static create method
#undef IMPLEMENT_CHUNK_ITEM_ARGS
#define IMPLEMENT_CHUNK_ITEM_ARGS (pSection, pChunk, &errorString)
IMPLEMENT_CHUNK_ITEM( EditorChunkTerrain, terrain, 1 )
