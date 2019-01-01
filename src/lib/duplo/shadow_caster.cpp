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
#include "shadow_caster.hpp"

#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_terrain.hpp"
#include "material_draw_override.hpp"
#include "terrain/terrain_settings.hpp"
#include "terrain/base_terrain_block.hpp"
#include "moo/effect_visual_context.hpp"
#include "moo/render_context.hpp"
#include "terrain/base_terrain_renderer.hpp"
#include "moo/visual_channels.hpp"
#include "moo/visual_compound.hpp"
#include "pymodel.hpp"
#include "resmgr/datasection.hpp"
#include "romp/time_of_day.hpp"
#include "shadow_caster_common.hpp"

DECLARE_DEBUG_COMPONENT2( "Duplo", 0 );

// -----------------------------------------------------------------------------
// Section: ShadowCaster
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
ShadowCaster::ShadowCaster()
: angleIntensity_( 1.f ),
  pCommon_( NULL ),
  shadowBufferSize_( 1024 )
{
	viewport_ = Matrix::identity;
	viewport_._11 = 0.5f;
	viewport_._22 = -0.5f;
	viewport_._41 = 0.5f;
	viewport_._42 = 0.5f;
}

/**
 *	This method inits the shadow caster.
 *	@param pCommon the common properties shared by all shadow casters
 *	@param halfRes whether this shadow target should be half the resolution or not.
 *	@param idx identifier to help name the render target.
 *	@param pDepthParent rendertarget to inherit the depth buffer from.
 *	@return true if the init was successful
 */
bool ShadowCaster::init( ShadowCasterCommon* pCommon, bool halfRes, uint32 idx, Moo::RenderTarget* pDepthParent  )
{
	BW_GUARD;
	bool success = false;
	if (Moo::rc().supportsTextureFormat( D3DFMT_G16R16F ))
	{
		// Set up the common properties
		pCommon_ = pCommon;
		shadowBufferSize_ = pCommon_->shadowBufferSize();
		if (halfRes)
			shadowBufferSize_ /= 2;

		// Set up our scissor rect, it's set to be one pixel inside
		// the render target so that we do not need to worry about
		// texture clamping when rendering the shadows.
		scissorRect_.left = 1;
		scissorRect_.top = 1;
		scissorRect_.bottom = shadowBufferSize_ - 1;
		scissorRect_.right = shadowBufferSize_ - 1;

		// Create the render target
		char cidx[3];
		cidx[0] = '1'+idx/10;
		cidx[1] = '1'+idx%10;
		cidx[2] = 0;
		pShadowTarget_ = new Moo::RenderTarget( std::string("ShadowTarget") + cidx );
		success = pShadowTarget_->create( shadowBufferSize_, shadowBufferSize_, false, D3DFMT_G16R16F, pDepthParent );
	}
	return success;
}

/**
 *	This method starts the rendering to the shadow buffer.
 *	@param worldSpaceBB the bounding box in world space of the object to cast shadows from.
 *	@param lightDirection the light direction shadowing should occur in.
 */
void ShadowCaster::begin( const BoundingBox& worldSpaceBB, const Vector3& lightDirection )
{
	BW_GUARD;
	// Set up the transforms used by the shadow casting
	this->setupMatrices( worldSpaceBB, lightDirection );
	
	// Set all textures to NULL, just in case the shadow render target was set on 
	// any of the stages before.
	uint32 nTextures = Moo::rc().deviceInfo( Moo::rc().deviceIndex() ).caps_.MaxSimultaneousTextures;
	for (uint32 i = 0; i < nTextures; i++)
	{
		Moo::rc().setTexture( i, NULL );
	}

	// Set and clear the render target
	if ( pShadowTarget_->push() )
	{
		Moo::rc().device()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x0, 1.f, 0 );

		// Set up scissoring
		Moo::rc().device()->SetScissorRect( &scissorRect_ );
		Moo::rc().setRenderState( D3DRS_SCISSORTESTENABLE, TRUE );

		// Set up the render override and the current transforms
		Moo::Visual::s_pDrawOverride = pCommon_->pCasterOverride();
		Moo::rc().view( view_ );
		Moo::rc().projection( projection_ );
		Moo::rc().updateViewTransforms();

		// Disable the visual channels (ie sorted, shimmer etc)
		Moo::VisualChannel::enabled(false);
	}
}

/**
 *	This methods sets up the various matrices used by the shadowing.
 *	@param worldSpaceBB the bounding box in world space of the object to cast shadows from.
 *	@param lightDirection the light direction shadowing should occur in.
 */
void ShadowCaster::setupMatrices( const BoundingBox& worldSpaceBB, const Vector3& lightDirection )
{
	BW_GUARD;
	// Get the view matrix along the lightdirection.
	view_.lookAt( Vector3( 0, 0, 0 ), lightDirection, Vector3( 0, 1, 0 ) );

	// transform boundingbox to view and get the dimensions in view space
	BoundingBox bb = worldSpaceBB;
	bb.transformBy( view_ );
	
	// Set up the bounding box for the shadow volume
	Vector3 offset = (bb.maxBounds() + bb.minBounds()) * 0.5f;

	Vector3 diff = (bb.maxBounds() - bb.minBounds());
	diff.x = max( diff.x, diff.y );
	diff.y = diff.x;
	diff.z = pCommon_->shadowDistance();

	offset.z -= diff.z * 0.5f;

	view_.postTranslateBy( Vector3(0, 0, 0 ) - offset );

	// Create the projection matrix of the shadow volume
	projection_.orthogonalProjection( diff.x, diff.y, 0, diff.z );

	viewProjection_.multiply( view_, projection_ );

	// Create the axis aligned bounding box of the
	// shadow volume in world space
	aaShadowVolume_.setBounds(	Vector3( -1.f, -1.f, 0.f ),
								Vector3( 1.f, 1.f, 1.f ) );

	Matrix invViewProj;
	invViewProj.invert( viewProjection_ );
	aaShadowVolume_.transformBy( invViewProj );

	shadowProjection_.multiply( viewProjection_, viewport_ );
}

/**
 *	This methods sets everything back to normal after rendering 
 *	to the shadow buffer.
 */
void ShadowCaster::end()
{
	BW_GUARD;
	if ( pShadowTarget_->valid() )
	{
		Moo::rc().setRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
		Moo::Visual::s_pDrawOverride = NULL;
		pShadowTarget_->pop();
		Moo::VisualChannel::enabled(true);
	}
}

/*
 *	This is a helper method to set up the constants for a shadow rendering effect.
 */
void ShadowCaster::setupConstants( Moo::EffectMaterial& effect )
{
	BW_GUARD;
	ID3DXEffect* pEffect = effect.pEffect()->pEffect();
	
	SAFE_SET( pEffect, Texture, "depthTexture", pShadowTarget_->pTexture() );
	SAFE_SET( pEffect, Float, "shadowTexelSize", 1.f / float(shadowBufferSize_) );
	SAFE_SET( pEffect, Float, "shadowIntensity", pCommon_->shadowIntensity() * angleIntensity_ );
	SAFE_SET( pEffect, Matrix, "shadowProj", &shadowProjection_ );
	SAFE_SET( pEffect, Float, "shadowDistance", pCommon_->shadowDistance() );
	SAFE_SET( pEffect, Float, "shadowFadeStart", pCommon_->shadowFadeStart() );
	SAFE_SET( pEffect, Int, "shadowQuality", pCommon_->shadowQuality() );
	SAFE_SET( pEffect, Vector, "shadowDir", &Vector4( view_[0][2], view_[1][2], view_[2][2], 0 ) );
}

/**
 *	This method sets everything up for rendering the shadow 
 *	pass using this render target. It also renders all the
 *	objects in the collision scene that intersect the
 *	shadow volume
 */
void ShadowCaster::beginReceive( bool useTerrain /* = true */ )
{
	BW_GUARD;
	// Set up shader constants used to render the shadow pass.
	MaterialDrawOverride* pReceiverOverride = pCommon_->pReceiverOverride();
	setupConstants( *pReceiverOverride->pSkinnedEffect_ );	
	setupConstants( *pReceiverOverride->pRigidEffect_ );
	setupConstants( *pCommon_->pTerrainReceiver() );
	setupConstants( *pCommon_->pTerrain2Receiver() );

	// Disable visual channels and set the draw override for the
	// visual to be the shadow receiver.
	Moo::VisualChannel::enabled( false );
	Moo::Visual::s_pDrawOverride = pReceiverOverride;

	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	Terrain::BaseTerrainRendererPtr pTerrainRenderer;
	if (pSpace && pSpace->terrainSettings().exists())
	{
		pTerrainRenderer = pSpace->terrainSettings()->pRenderer();
	}

	// Clear the terrain renderer.
	if (useTerrain && pTerrainRenderer.exists())
	{
        pTerrainRenderer->clearBlocks();
	}

	// Pick the objects in the scene that should receive shadows.
	this->pickReceivers();

	// Iterate over the shadow items and draw them using the shadow
	// receiver shaders.
	ShadowItems::iterator it = shadowItems_.begin();
	while (it != shadowItems_.end())
	{
		if (*it)
		{
			ChunkItemPtr pItem = *it;
			Chunk* pChunk = pItem->chunk();
			if (pItem->drawMark() == (Chunk::s_nextMark_ - 1) && pChunk)
			{
				Moo::rc().push();
				Moo::rc().world( pChunk->transform() );
				Moo::EffectVisualContext::instance().isOutside( pChunk->isOutsideChunk() );
				pItem->draw();
				Moo::rc().pop();
			}

			*it = NULL;
		}
		it++;
	}
	
	// Clear the shadow items so that we don't steal references to objects
	// receiving shadows
	shadowItems_.clear();

	// Draw the terrain using the terrain receiver.
	if (useTerrain && pTerrainRenderer.exists())
	{
		// TODO: should get the receiver shader automatically, using the
		// terrain version.
		if ( pTerrainRenderer->version() == 200 )
			pTerrainRenderer->drawAll( pCommon_->pTerrain2Receiver() );
		else
			pTerrainRenderer->drawAll( pCommon_->pTerrainReceiver() );
	}

	// This must currently be called even if useTerrain is false.  Because
	// the shadowItems_ may contain terrain blocks despite the outside not
	// being visible.  When the shadow calcs take into account whether or not
	// outside items should be used, then we can move this back into the
	// useTerrain code block just above.
	// Note : if we don't do this, then there will be left over terrain blocks
	// that will get drawn during the next pass (and that will signal app to
	// think it can see outside, causing the flora to be drawn too).
	if (pTerrainRenderer.exists())
	{
		pTerrainRenderer->clearBlocks();
	}
}

/**
 *	This method sets everything back to normal after rendering shadows.
 */
void ShadowCaster::endReceive()
{
	BW_GUARD;
	Moo::VisualCompound::drawAll( pCommon_->pReceiverOverride()->pRigidEffect_ );
	Moo::Visual::s_pDrawOverride = NULL;
	Moo::VisualChannel::enabled(true);
}

/**
 *	This method picks the objects in the collision scene that the shadow volume
 *	intersects with.
 */
void ShadowCaster::pickReceivers( )
{
	BW_GUARD;
	// make sure we have nothing in our shadow items.
	shadowItems_.clear();

	// Get the chunk space
	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (pSpace)
	{

		// increment the mark counter so that we only get each obstacle once.
		ChunkObstacle::nextMark();

		// Create a cylinder out of the shadow volume for obstacle traversal

		// Create the shadow space bb
		BoundingBox bb( Vector3( -1, -1, 0 ), Vector3( 1, 1, 1 ) );
		Matrix m;
		m.invert( projection_ );
		bb.transformBy( m );

		// Get the end points of the cylinder in world space
		Vector3 startPos = bb.centre();
		startPos.z = bb.minBounds().z;

		Vector3 endPos = bb.centre();
		endPos.z = bb.maxBounds().z;

		m.invert( view_ );

		startPos = m.applyPoint( startPos );
		endPos = m.applyPoint( endPos );

		// Calculate the radius of the cylinder
		Vector3 side = bb.maxBounds() - bb.minBounds();
		side.z = 0;

		float radius = side.length() / 2.f;

		// Get the grid outline covered by our shadow volume and iterate over it.
		float xMin = floorf(aaShadowVolume_.minBounds().x / GRID_RESOLUTION );
		float xMax = floorf(aaShadowVolume_.maxBounds().x / GRID_RESOLUTION ) + 1;
		float zMin = floorf(aaShadowVolume_.minBounds().z / GRID_RESOLUTION );
		float zMax = floorf(aaShadowVolume_.maxBounds().z / GRID_RESOLUTION ) + 1;
		for (float z = zMin; z < zMax; z++)
		{
			for (float x = xMin; x < xMax; x++)
			{
				// Get the column we are in.
				float zGrid = (z * GRID_RESOLUTION) + (GRID_RESOLUTION * 0.5f);
				float xGrid = (x * GRID_RESOLUTION) + (GRID_RESOLUTION * 0.5f);
				ChunkSpace::Column* pColumn = pSpace->column( Vector3( xGrid, 0, zGrid ), false );
				if (pColumn)
				{
					// Grab the obstacles we want to shadow.
					const ObstacleTree& obstacles = pColumn->obstacles();
					ObstacleTree::Traversal& traversal = obstacles.traverse( startPos, endPos, radius );
					const ChunkObstacle* pObstacle = NULL;
					while ((pObstacle = traversal.next()) != NULL)
					{
						if (pObstacle->mark()) continue;
                        
						shadowItems_.push_back( pObstacle->pItem() );
					}
				}
			}
		}
	}
}
