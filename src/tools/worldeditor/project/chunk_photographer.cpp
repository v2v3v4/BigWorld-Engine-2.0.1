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
#include "worldeditor/project/chunk_photographer.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/world/items/editor_chunk_link.hpp"
#include "appmgr/options.hpp"
#include "chunk/chunk.hpp"
#include "chunk/chunk_water.hpp"
#include "chunk/chunk_flare.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "moo/mrt_support.hpp"
#include "moo/render_context.hpp"
#include "moo/texture_compressor.hpp"
#include "moo/visual_channels.hpp"
#include "duplo/py_splodge.hpp"
#include "romp/rain.hpp"
#include "terrain/terrain2/terrain_lod_controller.hpp"
#include "terrain/base_terrain_renderer.hpp"

#include "speedtree/billboard_optimiser.hpp"
#include "cstdmf/debug.hpp"


DECLARE_DEBUG_COMPONENT2( "WorldEditor", 0 )


BW_SINGLETON_STORAGE( ChunkPhotographer );


bool ChunkPhotographer::photograph( Chunk& chunk )
{
	BW_GUARD;

	return ChunkPhotographer::instance().takePhoto( chunk );
}


extern bool g_disableSkyLightMap;


ChunkPhotographer::ChunkPhotographer():
width_( 128 ),
height_( 128 ),
pRT_( NULL ),
savedLighting_( NULL ),
pOldChunk_( NULL )
{
}


/**
 *	This method photographs a chunk.
 */
bool ChunkPhotographer::takePhoto( Chunk& chunk )
{
	BW_GUARD;

	// make the render target if necessary
	if (!pRT_)
	{
		pRT_ = new Moo::RenderTarget( "TextureRenderer" );
		pRT_->create( width_, height_ );
	}

	if (!pRT_)
	{
		ERROR_MSG( "ChunkPhotographer::takePhoto: Failed because render target was not created\n" );
		return false;
	}

	bool ok = false;

	if (this->beginScene())
	{
		this->setStates(chunk);
		this->renderScene(chunk);
		this->resetStates();
		this->endScene();

		TextureCompressor tc( static_cast<DX::Texture*>(pRT_->pTexture()), D3DFMT_DXT1, 1 );
		if (Moo::rc().device()->TestCooperativeLevel() == D3D_OK)
		{
			DataSectionPtr thumbSection =
				EditorChunkCache::instance(chunk).pThumbSection();
			tc.stow(thumbSection);
			WorldManager::instance().chunkThumbnailUpdated( &chunk );
			ok = true;
		}
		else
		{
			ERROR_MSG( "ChunkPhotographer::takePhoto: "
				"Failed enabling render target while processing %s.\n",
				chunk.identifier().c_str() );
		}
	}
	return ok;
}


bool ChunkPhotographer::beginScene()
{
	BW_GUARD;

	// set and clear the render target
	bool ok = false;
	if (pRT_->valid() && pRT_->push())
	{
		Moo::rc().beginScene();
		if (Moo::rc().mixedVertexProcessing())
			Moo::rc().device()->SetSoftwareVertexProcessing( TRUE );
		Moo::rc().device()->Clear(
			0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff808080, 1.f, 0 );
		ok = true;
	}
	return ok;
}


void ChunkPhotographer::setStates( Chunk& chunk )
{
	BW_GUARD;

	//first, setup some rendering options.
	//we need to draw as cheaply as possible, and
	//not clobber the main pipeline.

	Waters::simulationEnabled(false);
	Waters::drawReflection(false);

	ChunkFlare::ignore( true );
    EditorChunkLink::enableDraw( false );
    EditorChunkStationNode::enableDraw( false );
	PySplodge::s_ignoreSplodge_ = true;
	Rain::disable( true );
	g_disableSkyLightMap = true;    

	//save some states we are about to change
	oldFOV_ = Moo::rc().camera().fov();
	savedLighting_ = Moo::rc().lightContainer();
	savedChunkLighting_ = &ChunkManager::instance().cameraSpace()->enviro().timeOfDay()->lighting();
	oldInvView_ = Moo::rc().invView();
	pOldChunk_ = ChunkManager::instance().cameraChunk();

	//create some lighting
	if ( !lighting_ )
	{
		Vector4 ambientColour( 0.08f,0.02f,0.1f,1.f );

		//outside lighting for chunks
		chunkLighting_.sunTransform.setRotateX( DEG_TO_RAD(90.f) );
		chunkLighting_.sunTransform.postRotateZ( DEG_TO_RAD(20.f) );
		chunkLighting_.sunColour.set( 1.f, 1.f, 1.f, 1.f );		
		chunkLighting_.ambientColour = ambientColour;

		//light container for terrain
		Moo::DirectionalLightPtr spDirectional = 
			new Moo::DirectionalLight( Moo::Colour( 1, 1, 1, 1 ), Vector3( 0, 0, -1.f ) );
		Vector3 lightPos = chunkLighting_.mainLightTransform().applyPoint( Vector3( 0, 0, -1 ) );
		Vector3 dir = Vector3( 0, 0, 0 ) - lightPos;
		dir.normalise();
		spDirectional->direction( dir ); 
		spDirectional->worldTransform( Matrix::identity );

		lighting_ = new Moo::LightContainer;
		lighting_->ambientColour( Moo::Colour( ambientColour ) );
		lighting_->addDirectional( spDirectional );
	}

	Moo::rc().lightContainer( lighting_ );

	//setup the correct transform for the given chunk.
	//adds of .25 is for the near clipping plane.
	std::vector<ChunkItemPtr> items;
	EditorChunkCache::instance(chunk).allItems( items );
	BoundingBox bb( Vector3::zero(), Vector3::zero() );
	for( std::vector<ChunkItemPtr>::iterator iter = items.begin(); iter != items.end(); ++iter )
		(*iter)->addYBounds( bb );
	Matrix view;
	Vector3 lookFrom( chunk.transform().applyToOrigin() );	
	lookFrom.x += GRID_RESOLUTION / 2.f;
	lookFrom.z += GRID_RESOLUTION / 2.f;
	lookFrom.y = bb.maxBounds().y + 0.25f + 300.f;

	float chunkHeight = bb.maxBounds().y - bb.minBounds().y + 320.f;
	view.lookAt( lookFrom, Vector3(0,-1,0), Vector3(0,0,1) );
	Moo::rc().push();
	Moo::rc().world( Matrix::identity );
	Matrix proj;
	proj.orthogonalProjection( GRID_RESOLUTION, GRID_RESOLUTION, 0.25f, chunkHeight + 0.25f );
	Moo::rc().view( view );
	Moo::rc().projection( proj );
	Moo::rc().updateViewTransforms();
	Terrain::BaseTerrainRenderer::instance()->enableSpecular( false );

	//make sure there are no states set into the main part of bigbang
	//that could upset the rendering
	Moo::rc().setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	Moo::rc().fogEnabled( false );
	FogController::instance().enable( false );	
}


void ChunkPhotographer::renderScene( Chunk & chunk )
{
	BW_GUARD;

	Moo::rc().lightContainer( lighting_ );
	ChunkManager::instance().camera( Moo::rc().invView(),
		ChunkManager::instance().cameraSpace(), &chunk );
	ChunkManager::instance().cameraSpace()->heavenlyLightSource( &chunkLighting_ );
	ChunkManager::instance().cameraSpace()->updateHeavenlyLighting();	

#if SPEEDTREE_SUPPORT
	speedtree::BillboardOptimiser::tick();
#endif//SPEEDTREE_SUPPORT

	ChunkManager::instance().draw();

	//turn off alpha writes, because we are saving in DXT1 format ( one-bit alpha )
	//and the terrain normally uses alpha channel encoding, which upsets the bitmap.
	//not sure why the terrain has to output its alpha, but hell this is a workaround.
	Moo::rc().setRenderState(D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN);
	Moo::rc().lightContainer( lighting_ );
	Terrain::BasicTerrainLodController::instance().setCameraPosition( Moo::rc().invView().applyToOrigin() );
	Terrain::BaseTerrainRenderer::instance()->drawAll();
	Moo::rc().setRenderState(D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_ALPHA);

	// Draw water
	Moo::MRTSupport::instance().bind();
	Waters::instance().projectView(true);	
	Waters::instance().drawDrawList( 0.f );
	Waters::instance().projectView(false);
	Moo::MRTSupport::instance().unbind();

	Moo::SortedChannel::draw();
}


void ChunkPhotographer::resetStates()
{
	BW_GUARD;

	//set the stuff back
	Moo::rc().lightContainer( savedLighting_ );
	ChunkManager::instance().camera( oldInvView_,
					ChunkManager::instance().cameraSpace(),
					(pOldChunk_ && pOldChunk_->isBound()) ? pOldChunk_ : NULL );
	ChunkManager::instance().cameraSpace()->heavenlyLightSource( savedChunkLighting_ );
	Moo::rc().camera().fov( oldFOV_ );
	Moo::rc().updateProjectionMatrix();
	Moo::rc().pop();
	Terrain::BaseTerrainRenderer::instance()->enableSpecular( true );

	//restore drawing states!
	ChunkFlare::ignore( false );
    EditorChunkLink::enableDraw( true );
    EditorChunkStationNode::enableDraw( true );
	PySplodge::s_ignoreSplodge_ = false;
	Rain::disable( false );
	g_disableSkyLightMap = false;
}


void ChunkPhotographer::endScene()
{
	BW_GUARD;

	// and reset everything
	Moo::rc().endScene();
	pRT_->pop();
}
