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
#include "terrain_block2.hpp"

#include "../terrain_data.hpp"
#include "terrain_height_map2.hpp"
#include "terrain_texture_layer2.hpp"
#include "terrain_hole_map2.hpp"
#include "dominant_texture_map2.hpp"
#include "aliased_height_map.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/timestamp.hpp"
#include "resmgr/bwresource.hpp"

#ifndef MF_SERVER
#include "d3dx9mesh.h"
#include "cstdmf/bgtask_manager.hpp"
#include "cstdmf/profiler.hpp"
#include "../manager.hpp"
#include "terrain_index_buffer.hpp"
#include "terrain_renderer2.hpp"
#include "horizon_shadow_map2.hpp"
#include "terrain_normal_map2.hpp"
#include "terrain_vertex_buffer.hpp"
#include "moo/effect_material.hpp"
#include "moo/png.hpp"
#include "terrain_lod_map2.hpp"
#include "terrain_lod_controller.hpp"
#ifdef EDITOR_ENABLED
	#include "editor_vertex_lod_manager.hpp"
#else
	#include "vertex_lod_manager.hpp"
#endif

#endif

using namespace Terrain;

DECLARE_DEBUG_COMPONENT2("Moo", 0)

#ifndef MF_SERVER

PROFILER_DECLARE( TerrainBlock2_evaluate, "TerrainBlock2 Evaluate" );
PROFILER_DECLARE( TerrainBlock2_evaluate_calculate, "TerrainBlock2 Evaluate-Calculate" );
PROFILER_DECLARE( TerrainBlock2_evaluate_evaluate, "TerrainBlock2 Evaluate-Evaluate" );
PROFILER_DECLARE( TerrainBlock2_draw, "TerrainBlock2 Draw" );
PROFILER_DECLARE( TerrainBlock2_predraw, "TerrainBlock2 preDraw" );

namespace
{

/*
 *	This class is a helper class to calculate the offsets for generating
 *	inclusive and exclusive meshes for the umbra mesh of the terrain.
 */
class TerrainHullHelper
{
public:
	/*
	 * Constructor for the hull helper
	 * We init the helper with the heights from four corners in a quad from our
	 * lower resolution mesh and the sense value of the quad, which is one if
	 * the diagonal goes from the top left to the bottom right corner, and 0
	 * otherwise.
	 */
	TerrainHullHelper( float bottomLeft, float bottomRight, 
		float topLeft, float topRight, uint32 sense ) :
	bottomLeft_( bottomLeft ),
	bottomRight_( bottomRight ),
	topLeft_( topLeft ),
	topRight_( topRight ),
	sense_( sense ),
	maxA_( 0 ),
	maxB_( 0 ),
	minA_( 0 ),
	minB_( 0 )
	{
		BW_GUARD;
	}

	/*
	 *	This method updates the min and max values for the two triangles in the
	 *	mesh. x and z describe the normalised location in the current quad.
	 *	height is the height in the source height map at that location.
	 */
	void updateMinMax( float x, float z, float height )
	{
		BW_GUARD;
		// get the difference between the height in the quad 
		// and the passed in height
		float diff = height - heightAt( x, z );

		// Choose which triangle to update depending on the sense value
		// and the location in the quad, then update the min and max
		// values for that triangle
		if ((sense_ && (1.f - x) > z ) ||
			(!sense_ && x > z) )
		{
			maxA_ = std::max( maxA_, diff);
			minA_ = std::min( minA_, diff);
		}
		else
		{
			maxB_ = std::max( maxB_, diff);
			minB_ = std::min( minB_, diff);
		}
	}

	/*
	 *	This method outputs the occlusion heights of the quad
	 */
	void occlusionHeights( float& bottomLeft, float& bottomRight, 
		float& topLeft, float& topRight )
	{
		BW_GUARD;
		// get the combined value
		float combined = std::min( minA_, minB_ );

		// Depending on the sense, we generate the corner positions
		// of the quad
		if (sense_)
		{
			bottomLeft = bottomLeft_ + minA_;
			topRight = topRight_ + minB_;
			topLeft = topLeft_ + combined;
			bottomRight = bottomRight_ + combined;
		}
		else
		{
			bottomRight = bottomRight_ + minA_;
			topLeft = topLeft_ + minB_;
			topRight = topRight_ + combined;
			bottomLeft = bottomLeft_ + combined;
		}
	}

	/*
	 *	This method outputs the occlusion heights of the quad
	 */
	void visibilityHeights( float& bottomLeft, float& bottomRight, 
		float& topLeft, float& topRight )
	{
		BW_GUARD;
		// get the combined value
		float combined = std::max( maxA_, maxB_ );

		// Depending on the sense, we generate the corner positions
		// of the quad
		if (sense_)
		{
			bottomLeft = bottomLeft_ + maxA_;
			topRight = topRight_ + maxB_;
			topLeft = topLeft_ + combined;
			bottomRight = bottomRight_ + combined;
		}
		else
		{
			bottomRight = bottomRight_ + maxA_;
			topLeft = topLeft_ + maxB_;
			topRight = topRight_ + combined;
			bottomLeft = bottomLeft_ + combined;
		}
	}

private:

	float heightAt( float x, float z )
	{
		BW_GUARD;
		float res;
		// Work out the diagonals for the height map cell, neighbouring cells
		// have opposing diagonals.
		if (sense_)
		{
			// Work out which triangle we are in and calculate the interpolated
			// height.
			if ((1.f - x) > z)
			{
				res = bottomLeft_ + 
					(bottomRight_ - bottomLeft_) * x + 
					(topLeft_ - bottomLeft_) * z;
			}
			else
			{
				res = topRight_ + 
					(topLeft_ - topRight_) * (1.f - x) + 
					(bottomRight_ - topRight_) * (1.f - z);
			}
		}
		else
		{
			// Work out which triangle we are in and calculate the interpolated
			// height.
			if (x > z)
			{
				res = bottomRight_ + 
					(bottomLeft_ - bottomRight_) * (1.f - x) + 
					(topRight_ - bottomRight_) * z;
			}
			else
			{
				res = topLeft_ + 
					(topRight_ - topLeft_) * x + 
					(bottomLeft_ - topLeft_) * (1.f - z);
			}
		}
		return res;

	}

	uint32 sense_;

	float maxA_;
	float maxB_;
	float minA_;
	float minB_;

	float bottomLeft_;
	float bottomRight_;
	float topLeft_;
	float topRight_;
};

	
} // anonymous namespace

/**
 *  This is the TerrainBlock2 constructor.
 */
TerrainBlock2::TerrainBlock2(TerrainSettingsPtr pSettings):
    CommonTerrainBlock2(pSettings),
	pDetailHeightMapResource_(NULL),
	pVerticesResource_( NULL ),
	pNormalMap_( NULL ),
	pHorizonMap_( NULL ),
	pLodMap_( NULL ),
	pDiffLights_( new Moo::LightContainer ),
	pSpecLights_( new Moo::LightContainer ),
	depthPassMark_( 0 ),
	preDrawMark_( 0 )
{
	BW_GUARD;
	pBlendsResource_ = new TerrainBlendsResource( *this );
}

/**
 *  This is the TerrainBlock2 destructor.
 */
/*virtual*/ TerrainBlock2::~TerrainBlock2()
{
	BW_GUARD;
	// remove ourselves from controller
	if (Manager::pInstance() != NULL)
		BasicTerrainLodController::instance().delBlock( this );
}

/**
 *  This function loads an BaseTerrainBlock from a DataSection.
 *
 *  @param filename         A file to load the terrain block from.
 *  @param worldPosition    The position that the terrain will placed.
 *	@param cameraPosition	The position of the camera (to work out LoD level).
 *  @param error            If not null and there was an error loading
 *                          the terrain then this will be set to a
 *                          description of the problem.
 *  @returns                True if the load was completely successful,
 *                          false if a problem occurred.
 */
/*virtual*/ bool
TerrainBlock2::load(std::string const &filename,
					Matrix  const &worldTransform,
					Vector3 const &cameraPosition,
					std::string *error/* = NULL*/)
{
	BW_GUARD;
	// initialise our filename record
	fileName_ = filename;

	// try to open our section
	DataSectionPtr pTerrain = BWResource::openSection( filename );
	if ( !pTerrain )
	{
		if ( error ) *error = "Can't open terrain section.";
		return false;
	}

	// set up detail height map resource (needs filename)
	pDetailHeightMapResource_ = new HeightMapResource( fileName_, settings_->topVertexLod() );

	// In editor we load the most detailed height lod available.
#if !defined(EDITOR_ENABLED)
	const uint32 defaultHMLod = settings_->defaultHeightMapLod();
#else
	const uint32 defaultHMLod = 0; // Lod 0 is most detailed
#endif

	// call loading in the base class first
	if ( !CommonTerrainBlock2::internalLoad( filename, pTerrain, worldTransform, 
									cameraPosition, defaultHMLod, error ) )
	{
		return false;
	}

	// Initialise LOD texture
	DataSectionPtr pLodMapData = pTerrain->openSection( "lodTexture.dds" );
	if (pLodMapData)
	{
		pLodMap_ = new TerrainLodMap2;
		if(!pLodMap_->load(pLodMapData))
		{
			pLodMap_ = NULL;
		}
	}
	else
	{
		WARNING_MSG( "TerrainBlock2::load: Missing lodMap datasection for %s.\n", fileName_.c_str() );
	}

	// Initialise vertex LOD cache
	if ( !initVerticesResource( error ) )
		return false;

	// Initialise normals
	pNormalMap_ = new TerrainNormalMap2;

	bool normalsLoaded = false;
	normalsLoaded = pNormalMap_->init( fileName_ );

	Matrix invWorld;
	invWorld.invert( worldTransform );

	// Evaluate this block's visibility settings
	evaluate( invWorld.applyPoint(cameraPosition) );
	stream();

	if (!normalsLoaded)
	{
#ifndef EDITOR_ENABLED
		if (error) *error = "No valid normals found.";
		return false;
#else
		// Editor may rebuild normals.
		rebuildNormalMap( NMQ_NICE );
#endif
	}

    pHorizonMap_ = new HorizonShadowMap2(*this);
	DataSectionPtr pHorizonShadows = pTerrain->openSection( "horizonShadows" );
	if (!pHorizonShadows)
	{
		if ( error ) *error = "No horizon shadows found in terrain chunk.";
		return false;
	}

	if ( !pHorizonMap_->load(pHorizonShadows) )
	{
		if ( error ) *error = "Couldn't load supplied horizon shadow data.";
		return false;
	}

	// add ourselves to controller
	BasicTerrainLodController::instance().addBlock( this, worldTransform );

	return true;
}


/**
 *	Initialises the vertex buffer holder for this terrain block.
 *
 *  @param error		    Optional output parameter that receives an error
 *							message if an error occurs.
 *  @returns                True if successful, false otherwise.
 */
bool TerrainBlock2::initVerticesResource( std::string*	error/* = NULL*/ )
{
	BW_GUARD;
	pVerticesResource_ = new VerticesResource( 
		*this, settings_->numVertexLods() );

	if ( !pVerticesResource_ )
	{
		if ( error ) *error = "Couldn't allocate a VerticesResource.";
		return false;
	}
	
	return true;
}

/**
 *  This function gets the shadow map for the terrain.
 *
 *  @returns                The shadow map for the terrain.
 */
/*virtual*/ HorizonShadowMap &TerrainBlock2::shadowMap()
{
    return *pHorizonMap_;
}


/**
 *  This function gets the shadow map for the terrain.
 *
 *  @returns                The shadow map for the terrain.
 */
/*virtual*/ HorizonShadowMap const &TerrainBlock2::shadowMap() const
{
    return *pHorizonMap_;
}

/**
 *	This function prepares the terrain block for drawing.
 *	
 *	@param	usedCachedLighting	Use stored light state if true, or renderer
 *								current light state otherwise.
 *	@param	doSubstitution		Substitute a missing vertex lod for best 
 *								available.
 *
 *	@returns true, if the block was successfully set up.
 */
bool TerrainBlock2::preDraw( bool useCachedLighting, bool doSubstitution )
{
	BW_GUARD_PROFILER( TerrainBlock2_predraw );

	// set up masks and morph range
	settings_->vertexLod().calculateMasks( distanceInfo_, lodRenderInfo_ );

#ifndef EDITOR_ENABLED
	// Set up lod texture mask - we have a partial result from evaluate()
	// step. Editor already has full result.
	lodRenderInfo_.renderTextureMask_  = TerrainRenderer2::getDrawFlag( 
											lodRenderInfo_.renderTextureMask_,
											distanceInfo_.minDistanceToCamera_, 
											distanceInfo_.maxDistanceToCamera_, 
											settings_ );
#endif

	// set lights
	if ( useCachedLighting )
	{
		Moo::rc().lightContainer( pDiffLights_ );
		Moo::rc().specularLightContainer( pSpecLights_ );
	}

	// Return true if the vertices are available
	VertexLodManager* vlm = pVerticesResource_.getObject();
	MF_ASSERT( vlm && "VertexLodManager should be loaded!" );

	// Mark the block as drawn in this render target
	if ( preDrawMark_ != Moo::rc().renderTargetCount() )
	{
		currentDrawState_.currentVertexLodPtr_= 
			vlm->getLod( distanceInfo_.currentVertexLod_, doSubstitution );
		currentDrawState_.nextVertexLodPtr_	= 
			vlm->getLod( distanceInfo_.nextVertexLod_, doSubstitution );
		currentDrawState_.blendsPtr_			= pBlendsResource_->getObject();
		currentDrawState_.blendsRendered_		= false;
		currentDrawState_.lodsRendered_		= false;

		preDrawMark_ = Moo::rc().renderTargetCount();
	}

	return currentDrawState_.currentVertexLodPtr_.exists() 
		&& currentDrawState_.nextVertexLodPtr_.exists();
}

/*virtual*/ bool TerrainBlock2::draw( Moo::EffectMaterialPtr pMaterial )
{
	BW_GUARD_PROFILER( TerrainBlock2_draw );

	bool ret = false;

	// Draw each block with in two stages. On the first pass we draw
	// appropriate sub-blocks at current lod, and degenerate triangles.
	// on second pass, draw the *other* sub-blocks at next lod.
	VertexLodEntryPtr currentLod	= currentDrawState_.currentVertexLodPtr_;
	VertexLodEntryPtr nextLod		= currentDrawState_.nextVertexLodPtr_;

	// If the current lod exists ...
	if ( currentLod.exists() )
	{
		// If we don't have the next lod, then just draw the whole thing.
		if ( !nextLod.exists() )
		{
			// Warn developer, player is either moving too fast or streaming is
			// too slow.
			//WARNING_MSG("Didn't load vertex lod in time,"
			//			" terrain cracks may appear.\n");
			lodRenderInfo_.subBlockMask_ = 0xF;
		}

		ret = currentLod->draw( pMaterial, lodRenderInfo_.morphRanges_.main_,
											lodRenderInfo_.neighbourMasks_, 
											lodRenderInfo_.subBlockMask_ );

		// If we didn't draw everything above then draw the other parts.
		if ( lodRenderInfo_.subBlockMask_ != 0xF )
		{
			MF_ASSERT_DEBUG( nextLod.exists() );
			ret = nextLod->draw( pMaterial, lodRenderInfo_.morphRanges_.subblock_,
											lodRenderInfo_.neighbourMasks_, 
											lodRenderInfo_.subBlockMask_ ^ 0xF );
		}
	}
	else
	{
		WARNING_MSG("TerrainBlock2::draw(), currentLodEntry doesn't exist, not "
					"drawing anything.\n" );
	}

	return ret;
}

/**
 *  This function creates the UMBRA mesh and returns it in the provided
 *  structure.
 *
 *  @param umbraMesh				umbra mesh info.
 */
void TerrainBlock2::createUMBRAMesh( UMBRAMesh& umbraMesh ) const
{
	BW_GUARD;
	// Grab the height map
	const TerrainHeightMap& map = heightMap();

	// We set our mesh size to be power of two, as we assume that
	// the height map is power of 2 + 1
	const uint32 MESH_SIZE = std::min(uint32(16), map.blocksWidth() );

	const uint32 VERTICES_SIZE = MESH_SIZE + 1;

	// This is the distance between heights in the umbra mesh
	float step = BLOCK_SIZE_METRES / (MESH_SIZE);

	// reserve enough data for our vertices
	umbraMesh.testVertices_.reserve(VERTICES_SIZE * VERTICES_SIZE);

	// Iterate over our heights and initialise our vertices with an aliased
	// sample
	float zPos = 0.f;
	for (uint32 z = 0; z < map.verticesHeight(); 
			z += (map.blocksHeight() / MESH_SIZE) )
	{
		float xPos = 0.f;
		for (uint32 x = 0; x < map.verticesWidth(); 
				x += (map.blocksWidth() / MESH_SIZE))
		{
			float h = map.heightAt( int(x), int(z) );
			umbraMesh.testVertices_.push_back( Vector3( xPos, h, zPos));
			xPos += step;
		}
		zPos += step;
	}

	// Copy the initial vertices to the write vertices as well
	umbraMesh.writeVertices_ = umbraMesh.testVertices_;

	// Iterate over all the quads in the test and write models and
	// update their values, the write mesh wants to fit snugly under
	// the terrain and the test mesh wants to fit around the terrain
	for (uint32 z = 0; z < MESH_SIZE; z++)
	{
		for (uint32 x = 0; x < MESH_SIZE; x++)
		{
			uint32 sense = (x ^ z) & 1;
			uint32 offset = z * VERTICES_SIZE + x;

			// Create occlusion hull from the write vertices
			TerrainHullHelper occlusionHull( umbraMesh.writeVertices_[offset].y,
				umbraMesh.writeVertices_[offset + 1].y,
				umbraMesh.writeVertices_[offset + VERTICES_SIZE].y,
				umbraMesh.writeVertices_[offset + VERTICES_SIZE + 1].y, sense );

			// Create visibility hull from the write vertices
			TerrainHullHelper visibilityHull( umbraMesh.testVertices_[offset].y,
				umbraMesh.testVertices_[offset + 1].y,
				umbraMesh.testVertices_[offset + VERTICES_SIZE].y,
				umbraMesh.testVertices_[offset + VERTICES_SIZE + 1].y, sense );

			const uint32 BLOCK_SIZE = (map.blocksWidth() / MESH_SIZE);
			const uint32 BLOCK_STRIDE = BLOCK_SIZE + 1;

			float zf = 0;
			float zi = 1.f / float(BLOCK_SIZE);

			uint32 zs = z * BLOCK_SIZE;
			uint32 ze = zs + BLOCK_STRIDE;
			uint32 xs = x * BLOCK_SIZE;

			// Iterate over all source heights covered by the quad in the
			// umbra meshes
			for ( uint32 zoff = zs; zoff < ze; zoff++ )
			{
				float xf = 0;
				float xi = 1.f / float(BLOCK_SIZE);

				for (uint32 xoff = 0; xoff < BLOCK_STRIDE; xoff++)
				{
					float height = map.heightAt( int(xs + xoff), int(zoff) );
					occlusionHull.updateMinMax( xf, zf, height );
					visibilityHull.updateMinMax( xf, zf, height );
					xf += xi;
				}
				zf += zi;
			}

			// Output our new heights
			occlusionHull.occlusionHeights( umbraMesh.writeVertices_[offset].y,
				umbraMesh.writeVertices_[offset + 1].y,
				umbraMesh.writeVertices_[offset + VERTICES_SIZE].y,
				umbraMesh.writeVertices_[offset + VERTICES_SIZE + 1].y);
			occlusionHull.visibilityHeights( umbraMesh.testVertices_[offset].y,
				umbraMesh.testVertices_[offset + 1].y,
				umbraMesh.testVertices_[offset + VERTICES_SIZE].y,
				umbraMesh.testVertices_[offset + VERTICES_SIZE + 1].y );
		}
	}

	umbraMesh.testIndices_.reserve(MESH_SIZE * MESH_SIZE * 6);
	umbraMesh.writeIndices_.reserve(MESH_SIZE * MESH_SIZE * 6);

	// Iterate over the quads in the umbra mesh and create indices, the
	// occlusion mesh only creates for quads that do not intersect with any 
	// holes.
	uint32 corner = 0;
	for (uint32 z = 0; z < MESH_SIZE; z++)
	{
		for (uint32 x = 0; x < MESH_SIZE; x++)
		{
			if ((x ^z) & 1)
			{
				umbraMesh.testIndices_.push_back(corner);
				umbraMesh.testIndices_.push_back(corner + VERTICES_SIZE);
				umbraMesh.testIndices_.push_back(corner + 1);
				umbraMesh.testIndices_.push_back(corner + VERTICES_SIZE);
				umbraMesh.testIndices_.push_back(corner + VERTICES_SIZE + 1);
				umbraMesh.testIndices_.push_back(corner + 1);
				const Vector3& bl = umbraMesh.writeVertices_[corner];
				const Vector3& tr = 
					umbraMesh.writeVertices_[corner + VERTICES_SIZE + 1];
				if (!holeMap().holeAt(bl.x, bl.z, tr.x, tr.z))
				{
					umbraMesh.writeIndices_.push_back(corner);
					umbraMesh.writeIndices_.push_back(corner + VERTICES_SIZE);
					umbraMesh.writeIndices_.push_back(corner + 1);
					umbraMesh.writeIndices_.push_back(corner + VERTICES_SIZE);
					umbraMesh.writeIndices_.push_back(corner + VERTICES_SIZE + 1);
					umbraMesh.writeIndices_.push_back(corner + 1);
				}
			}
			else
			{
				umbraMesh.testIndices_.push_back(corner);
				umbraMesh.testIndices_.push_back(corner + VERTICES_SIZE);
				umbraMesh.testIndices_.push_back(corner + VERTICES_SIZE + 1);
				umbraMesh.testIndices_.push_back(corner);
				umbraMesh.testIndices_.push_back(corner + VERTICES_SIZE + 1);
				umbraMesh.testIndices_.push_back(corner + 1);
				const Vector3& bl = umbraMesh.writeVertices_[corner];
				const Vector3& tr = 
					umbraMesh.writeVertices_[corner + VERTICES_SIZE + 1];
				if (!holeMap().holeAt(bl.x, bl.z, tr.x, tr.z))
				{
					umbraMesh.writeIndices_.push_back(corner);
					umbraMesh.writeIndices_.push_back(corner + VERTICES_SIZE);
					umbraMesh.writeIndices_.push_back(corner + VERTICES_SIZE + 1);
					umbraMesh.writeIndices_.push_back(corner);
					umbraMesh.writeIndices_.push_back(corner + VERTICES_SIZE + 1);
					umbraMesh.writeIndices_.push_back(corner + 1);
				}
			}
			++corner;
		}
		++corner;
	}
}

void TerrainBlock2::cacheCurrentLighting( bool addSpecular /* = true */ )
{
	BW_GUARD;
	BoundingBox bb = boundingBox();
	bb.transformBy( Moo::rc().world() );
	pDiffLights_->init( Moo::rc().lightContainer(), bb );
	if ( addSpecular )
	{
		pSpecLights_->init( Moo::rc().specularLightContainer(), bb );
	}
	else
	{
		pSpecLights_->init( NULL, bb );
	}
}

/**
 *	This function manages resources dependencies for the block. It calculates 
 *	visibility settings, which are later used to stream or discard resources as 
 *	appropriate. The actual streaming occurs in the stream() function.
 *
 *	@param relativeCameraPos	The camera position relative to the block
 */
void TerrainBlock2::evaluate(	const Vector3& relativeCameraPos )
{
	BW_GUARD;
	//
	// Calculate distance info for this block
	//
	{
		BW_GUARD_PROFILER( TerrainBlock2_evaluate_calculate );

		// Position of this block relative to camera 
		distanceInfo_.relativeCameraPos_ = relativeCameraPos;
		
		// Maximum and minimum distances (relative to camera) of the block's
		// corners.
		TerrainVertexLod::minMaxXZDistance( distanceInfo_.relativeCameraPos_ , 
											BLOCK_SIZE_METRES, 
											distanceInfo_.minDistanceToCamera_, 
											distanceInfo_.maxDistanceToCamera_ );

		// The lod level of this block.
		distanceInfo_.currentVertexLod_	= 
			settings_->vertexLod().calculateLodLevel( 
			distanceInfo_.minDistanceToCamera_ );
		distanceInfo_.nextVertexLod_		= 
			std::min( uint32( settings_->numVertexLods() - 1), 
				distanceInfo_.currentVertexLod_ + 1 );
	}

	//
	// Set up streaming - given dependencies work out what to stream.
	//
	{
		BW_GUARD_PROFILER( TerrainBlock2_evaluate_evaluate );
			
		// Evaluate whether or not we need the detail height map to generate 
		// the vertices, or for collisions.
		uint32 requiredVertexGridSize = 
			VertexLodManager::getLodSize(	distanceInfo_.currentVertexLod_, 
											settings_->numVertexLods() );

		// If we're in the editor, we never need detail height maps due to 
		// distance requirements (grid size requirement should also pass 
		// because default height map should be maximum size in load() above).
#if !defined(EDITOR_ENABLED)
		float detailDistance = settings_->detailHeightMapDistance();
#else
		float detailDistance = 0;
#endif
		pDetailHeightMapResource_->evaluate(
									requiredVertexGridSize, 
									heightMap().blocksWidth(),
									detailDistance, 
									distanceInfo_.minDistanceToCamera_,
									settings_->topVertexLod());

		// Vertex evaluation
		pVerticesResource_->evaluate( distanceInfo_.currentVertexLod_, settings_->topVertexLod() );

		// Blends evaluation

		// Get a partial result of the render texture mask, just enough to tell
		// whether we need to load blends or not. This partial result will be
		// expanded further in preDraw().
		lodRenderInfo_.renderTextureMask_ = TerrainRenderer2::getLoadFlag( 
									distanceInfo_.minDistanceToCamera_,
									settings_->absoluteBlendPreloadDistance(),
									settings_->absoluteNormalPreloadDistance());
#ifdef EDITOR_ENABLED
		// Editor needs full result here for blends
		lodRenderInfo_.renderTextureMask_ = TerrainRenderer2::getDrawFlag( 
									lodRenderInfo_.renderTextureMask_ ,
									distanceInfo_.minDistanceToCamera_,
									distanceInfo_.maxDistanceToCamera_,
									settings_ );
#endif
		pBlendsResource_->evaluate( lodRenderInfo_.renderTextureMask_ );
		pNormalMap_->evaluate( lodRenderInfo_.renderTextureMask_ );
	}

	currentDrawState_.currentVertexLodPtr_	= NULL;
	currentDrawState_.nextVertexLodPtr_		= NULL;
	currentDrawState_.blendsPtr_				= NULL;
}

void TerrainBlock2::stream()
{
	BW_GUARD;
	//
	// Do streaming
	//
	pDetailHeightMapResource_->stream();
	pVerticesResource_->stream();
	
	// Stream blends if we are not in a reflection. We override it here rather 
	// than in visibility test to stop reflection scene from unloading blends
	// required by main scene render.
	if ( !Moo::rc().reflectionScene() )
	{
		pBlendsResource_->stream();
	}

	pNormalMap_->stream(); 
}

/**
 * This function returns true if we are able to draw the lod texture.
 */
bool TerrainBlock2::canDrawLodTexture() const
{
	BW_GUARD;
	return settings_->useLodTexture() && ( pLodMap_ != NULL );
}

/**
 * This function returns true if we are doing background tasks
 */
bool TerrainBlock2::doingBackgroundTask() const
{
	BW_GUARD;
	return pVerticesResource_->getState() == RS_Loading ||
		pBlendsResource_->getState() == RS_Loading ||
		pDetailHeightMapResource_->getState() == RS_Loading ||
		pNormalMap_->isLoading();
}

/**
 * This functions returns true if at least one LOD has been loaded.
 */
bool TerrainBlock2::readyToDraw() const
{
	VertexLodManager* vlm = pVerticesResource_.getObject();	
	return vlm && vlm->getLod(0, true);
}

/**
 * This function returns the highest lod height map available for this block,
 * and its lod level (if requested).
 */
const TerrainHeightMap2Ptr 
	TerrainBlock2::getHighestLodHeightMap() const
{
	BW_GUARD;

#if !defined(EDITOR_ENABLED)
	TerrainHeightMap2Ptr returnValue = pDetailHeightMapResource_->getObject();
	if ( returnValue.exists() )
	{
		// detail lod is loaded, return it
		return returnValue;
	}
	
#endif
	
	return heightMap2();
}

/**
 * Override to use best heightmap loaded. 
 */
BoundingBox const & TerrainBlock2::boundingBox() const
{
	BW_GUARD;
	TerrainHeightMap2Ptr hm = getHighestLodHeightMap();

	float maxh = hm->maxHeight();
	float minh = hm->minHeight();

	bb_.setBounds( Vector3( 0.f, minh, 0.f ), 
		Vector3( BLOCK_SIZE_METRES, maxh, BLOCK_SIZE_METRES ) );

	return bb_;
}

/**
 * Override to collide with best height map loaded.
 */
bool TerrainBlock2::collide
(
	 Vector3                 const &start, 
	 Vector3                 const &end,
	 TerrainCollisionCallback *callback
) const
{
	BW_GUARD;
	return getHighestLodHeightMap()->collide( start, end, callback );
}

/**
* Override to collide with best height map loaded.
*/
bool TerrainBlock2::collide
(
	 WorldTriangle           const &start, 
	 Vector3                 const &end,
	 TerrainCollisionCallback *callback
 ) const
{
	BW_GUARD;
	return getHighestLodHeightMap()->collide( start, end, callback );
}

/**
 * Override to get height with best height map loaded
 */
float TerrainBlock2::heightAt(float x, float z) const
{
	BW_GUARD;
	float res = NO_TERRAIN;

	if (!holeMap().holeAt( x, z ))
	{
		res = getHighestLodHeightMap()->heightAt(x, z);
	}

	return res;
}

/**
* Override to get normal with best height map loaded
*/
Vector3 TerrainBlock2::normalAt(float x, float z) const
{
	return getHighestLodHeightMap()->normalAt(x, z);
}

/**
 *	This function allows access to the normal map.  This can be used by
 *	derived classes.
 *
 *  @param normalMap2			The TerrainNormalMap2.
 */
void TerrainBlock2::normalMap2( TerrainNormalMap2Ptr normalMap2 )
{
	pNormalMap_ = normalMap2;
}


/**
 *	This function allows access to the normal map.  This can be used by
 *	derived classes.
 *
 *  @returns			The TerrainNormalMap2.
 */
TerrainNormalMap2Ptr TerrainBlock2::normalMap2() const
{
	return pNormalMap_;
}


/**
 *	This function allows access to the shadow map.  This can be used by
 *	derived classes.
 *
 *  @param horizonMap2			The HorizonShadowMap2.
 */
void TerrainBlock2::horizonMap2( HorizonShadowMap2Ptr horizonMap2 )
{
	pHorizonMap_ = horizonMap2;
}


/**
 *	This function allows access to the shadow map.  This can be used by
 *	derived classes.
 *
 *  @returns			The HorizonShadowMap2.
 */
HorizonShadowMap2Ptr TerrainBlock2::horizonMap2() const
{
	return pHorizonMap_;
}


uint32 TerrainBlock2::normalMapSize() const
{
    return pNormalMap_->size();
}


DX::Texture* TerrainBlock2::pNormalMap() const
{
    return pNormalMap_->pMap().pComObject();
}


uint32 TerrainBlock2::horizonMapSize() const
{
    return (pHorizonMap_ != NULL) ? pHorizonMap_->width() : 0;
}


DX::Texture* TerrainBlock2::pHorizonMap() const
{
    return (pHorizonMap_ != NULL) ? pHorizonMap_->texture() : NULL;
}


DX::Texture* TerrainBlock2::pHolesMap() const
{
    return holeMap2() != NULL ? holeMap2()->texture() : NULL;
}


uint32 TerrainBlock2::nLayers() const
{
	return pBlendsResource_->getObject() ? 
		pBlendsResource_->getObject()->combinedLayers_.size() : 0;
}


const CombinedLayer* TerrainBlock2::layer( uint32 index ) const
{
	return pBlendsResource_->getObject() ? 
		&(pBlendsResource_->getObject()->combinedLayers_[index]) : NULL;
}


/**
 *  This method accesses the texture layers.
 */
TextureLayers* TerrainBlock2::textureLayers()
{
	return pBlendsResource_->getObject() ? 
		&(pBlendsResource_->getObject()->textureLayers_) : NULL;
}

/**
 *  This method accesses the texture layers.
 */
TextureLayers const * TerrainBlock2::textureLayers() const
{
	return pBlendsResource_->getObject() ? 
		&(pBlendsResource_->getObject()->textureLayers_) : NULL;
}

TerrainLodMap2Ptr TerrainBlock2::lodMap() const
{
	return pLodMap_;
}

#endif

// terrain_block2.cpp
