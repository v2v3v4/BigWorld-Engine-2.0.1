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

#include "cstdmf/guard.hpp"

#include "chunk_umbra.hpp"
#include "chunk_manager.hpp"
#include "chunk_space.hpp"
#include "umbra_draw_item.hpp"
#include "speedtree/speedtree_renderer.hpp"
#include "moo/dynamic_vertex_buffer.hpp"
#include "moo/dynamic_index_buffer.hpp"


#if UMBRA_ENABLE

// The library to use, to link with the debug version of umbra change it to umbrad.lib,
// that will also need the debug dll of umbra.
#pragma comment(lib, "umbra.lib")

#include "romp/line_helper.hpp"

#include "moo/material.hpp"
#include "moo/visual.hpp"
#include "moo/visual_compound.hpp"
#include "terrain/base_terrain_renderer.hpp"

#ifdef EDITOR_ENABLED
	#include "appmgr/options.hpp"
#endif

#include <umbraCommander.hpp>
#include <umbraObject.hpp>
#include <umbraLibrary.hpp>

#include "umbra_proxies.hpp"


DECLARE_DEBUG_COMPONENT2( "Chunk", 0 );

PROFILER_DECLARE( ChunkCommander_command, "ChunkCommander Command" );
MEMTRACKER_DECLARE( ChunkUmbra, "ChunkUmbra", 0 );
PROFILER_DECLARE( ChunkCommander_occlusionStall, "ChunkCommander Occlusion Stall" );
PROFILER_DECLARE( ChunkCommander_occlusionQuery, "ChunkCommander Occlusion Query" );
PROFILER_DECLARE( UMBRA_tick, "UMBRA Tick" );

static bool clipEnabled = false;
static Moo::IndexBuffer s_queryTestIndexBuffer;

static const uint s_queryTestBoxes = 10;

// -----------------------------------------------------------------------------
// Section: ChunkCommander definition
// -----------------------------------------------------------------------------

/*
 *	This class implements the Umbra commander interface.
 *	The commander is the callback framework from the umbra scene
 *	traversal.
 */
class ChunkCommander : public Umbra::Commander
{
public:

	ChunkCommander(ChunkUmbraServices*);
	void terrainOverride( SmartPointer< BWBaseFunctor0 > pTerrainOverride );
	virtual void command	(Command c);
	void repeat();
	void clearCachedItems();
private:
	void flush();
	void enableDepthTestState();
	void disableDepthTestState();

	void drawStencilModel(UmbraPortal* portal, const Matrix& worldMtx);

	Chunk*						pLastChunk_;
	SmartPointer< BWBaseFunctor0 > pTerrainOverride_;
	std::vector< UmbraDrawItem* >	cachedItems_;
	bool						depthStateEnabled_;
	ChunkUmbraServices*			pServices_;
	Matrix						storedView_;
	bool						viewParametersChanged_;
};

// -----------------------------------------------------------------------------
// Section: ChunkUmbraServices definition
// -----------------------------------------------------------------------------

/*
 *	This class overrides some of the umbra services
 */
class ChunkUmbraServices : public Umbra::LibraryDefs::Services
{
public:
		virtual void error(const char* message);
		virtual void enterMutex();
		virtual void leaveMutex();
		virtual bool allocateQueryObject(int index);
		virtual void releaseQueryObject(int index);

		virtual void*	allocateMemory		(size_t bytes)
		{
			MEMTRACKER_SCOPED( ChunkUmbra );
			void* mem = malloc(bytes);
			if ( !mem )
			{
				CRITICAL_MSG( "Out of memory. Exiting.\n" );
			}
			return mem;
		}

        virtual void	releaseMemory		(void* mem)
		{
			MEMTRACKER_SCOPED( ChunkUmbra );
			free(mem);
		}
		
		Moo::OcclusionQuery* getQuery(int index) { return queries_[index]; }
private:
		SimpleMutex mutex_;
		std::vector< Moo::OcclusionQuery* >	queries_;
};

// -----------------------------------------------------------------------------
// Section: ChunkUmbra
// -----------------------------------------------------------------------------

/**
 *	This method inits the chunkumbra instance
 */
void ChunkUmbra::init( DataSectionPtr configSection )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( s_pInstance_ == NULL )
	{
		return;
	}
	s_pInstance_ = new ChunkUmbra( configSection );
	UmbraHelper::instance().init();

	bool enableUmbra = 
#ifdef EDITOR_ENABLED
		(Options::getOptionInt( "render/useUmbra", 1 ) != 0);
#else
		configSection && configSection->readBool( "renderer/enableUmbra", true );
#endif

	if ( DebugMsgHelper::automatedTest() )
	{
		char log[80];
		bw_snprintf( log, sizeof( log ), "Umbra: %s", enableUmbra ? "On" : "Off" );
		DebugMsgHelper::logToFile( log );
	}

	UmbraHelper::instance().umbraEnabled( enableUmbra );

	// Create a static index buffer for query test shapes
	HRESULT res = s_queryTestIndexBuffer.create( s_queryTestBoxes * 3 * 12, D3DFMT_INDEX16,
		D3DUSAGE_WRITEONLY, D3DPOOL_MANAGED );

	if ( res != D3D_OK )
	{
		ERROR_MSG( "Could not create index buffer for occlusion test shapes" );
		return;
	}

	Moo::IndicesReference ref = s_queryTestIndexBuffer.lock();

	static const uint16 indices[36] =
	{
		0, 3, 1,
		0, 2, 3, 
		6, 5, 7,
		6, 4, 5,
		2, 4, 6,
		2, 0, 4,
		1, 7, 5,
		1, 3, 7,
		6, 7, 2,
		2, 7, 3,
		0, 1, 4,
		4, 1, 5
	};

	uint offset = 0;
	uint icount = 0;

	for ( uint j = 0; j < s_queryTestBoxes; j++ )
	{
		for ( uint i = 0; i < 36; i++ )
		{
			ref.set( icount++, indices[i] + offset );
		}

		offset += 8;
	}

	s_queryTestIndexBuffer.unlock();
}

/**
 *	This method destroys the chunkumbra instance
 */
void ChunkUmbra::fini()
{
	BW_GUARD;
	UmbraHelper::instance().fini();
	UmbraModelProxy::invalidateAll();
	UmbraObjectProxy::invalidateAll();
	if (s_pInstance_)
	{
		s_queryTestIndexBuffer.release();
		delete s_pInstance_;
		s_pInstance_ = NULL;
	}
}

/**
 *	Return the commander instance.
 */
Umbra::Commander* ChunkUmbra::pCommander()
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( s_pInstance_ )
	{
		return NULL;
	}

	return s_pInstance_->pCommander_;
}


/*
 *	The constructor inits umbra and creates our commander
 */
ChunkUmbra::ChunkUmbra( DataSectionPtr configSection ) :
	softwareMode_(false),
	clipPlaneSupport_(true)
{
	BW_GUARD;
	pServices_ = new ChunkUmbraServices();

	// Try to create an occlusion query object to see if the hardware supports them
	Moo::OcclusionQuery* testQuery = Moo::rc().createOcclusionQuery();

	// We don't support hardware occlusion queries on fixed function hardware.
	// Although D3D says it supports it, D3D crashes when a query is used.
	bool forceSoftware = configSection && configSection->readBool( "renderer/forceSoftwareOcclusion", false );
	if ( !forceSoftware && Moo::rc().psVersion() > 0 && testQuery )
	{
		Umbra::Library::init(Umbra::LibraryDefs::COLUMN_MAJOR, Umbra::LibraryDefs::HARDWARE_OCCLUSION, pServices_);
		Moo::rc().destroyOcclusionQuery(testQuery);
		testQuery = NULL;
	}
	else
	{
		Umbra::Library::init(Umbra::LibraryDefs::COLUMN_MAJOR, Umbra::LibraryDefs::SOFTWARE_OCCLUSION, pServices_);
		softwareMode_ = true;
	}

	clipPlaneSupport_ = (Moo::rc().deviceInfo( Moo::rc().deviceIndex() ).caps_.MaxUserClipPlanes > 0);

	pCommander_ = new ChunkCommander(pServices_);
}


/*static*/
void ChunkUmbra::terrainOverride( SmartPointer< BWBaseFunctor0 > pTerrainOverride )
{
	BW_GUARD;
	if (s_pInstance_->pCommander_)
	{
		s_pInstance_->pCommander_->terrainOverride( pTerrainOverride );
	}
}


/*
 *	Repeat the drawing calls from the last query
 */
void ChunkUmbra::repeat()
{
	BW_GUARD;
	s_pInstance_->pCommander_->repeat();
}


bool ChunkUmbra::softwareMode()
{
	BW_GUARD;
	return s_pInstance_->softwareMode_;
}


bool ChunkUmbra::clipPlaneSupported()
{
	BW_GUARD;
	return s_pInstance_->clipPlaneSupport_;
}


/*
 *	Tick method, needs to be called once per frame
 */
void ChunkUmbra::tick()
{
	BW_GUARD_PROFILER( UMBRA_tick );
	Umbra::Library::resetStatistics();
	s_pInstance_->pCommander_->clearCachedItems();
}


void ChunkUmbra::minimiseMemUsage()
{
	Umbra::Library::minimizeMemoryUsage();
}


/*
 *	The destructor uninits umbra and destroys our umbra related objects.
 */
ChunkUmbra::~ChunkUmbra()
{
	BW_GUARD;
	delete pCommander_;

	Umbra::Library::exit();
	delete pServices_;
}

ChunkUmbra* ChunkUmbra::s_pInstance_ = NULL;


// -----------------------------------------------------------------------------
// Section: ChunkCommander
// -----------------------------------------------------------------------------

ChunkCommander::ChunkCommander(ChunkUmbraServices* pServices) :
	pLastChunk_(NULL),
	depthStateEnabled_(false),
	viewParametersChanged_(false),
	pServices_(pServices)

{
}


void ChunkCommander::terrainOverride( SmartPointer< BWBaseFunctor0 > pTerrainOverride )
{
	pTerrainOverride_ = pTerrainOverride;
}


/**
 * Redraws the scene, used for wireframe mode
 */
void ChunkCommander::repeat()
{
	BW_GUARD;
	Moo::rc().push();
	Chunk* pLastChunk = NULL;
	int count = cachedItems_.size();
	for (int i = 0; i < count; i++)
	{
		UmbraDrawItem* pItem = cachedItems_[i];
		pLastChunk = pItem->draw(pLastChunk);
	}
	Moo::rc().pop();
	flush();
	Chunk::nextMark();
	cachedItems_.clear();
}

/**
 * Clear the cached items, as they are only valid for one frame
 */
void ChunkCommander::clearCachedItems()
{
	cachedItems_.clear();
}


/**
 * Flush delayed rendering commands for occluders
 */
void ChunkCommander::flush()
{
	BW_GUARD;
	disableDepthTestState();

#if SPEEDTREE_SUPPORT
	if (UmbraHelper::instance().flushTrees())
	{
		speedtree::SpeedTreeRenderer::flush();
	}
#endif

	// Make sure we are using the correct view matrix
	if (viewParametersChanged_)
	{
		Moo::rc().view(storedView_);
		viewParametersChanged_ = false;
	}

	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (ChunkManager::instance().cameraSpace())
	{
		Moo::LightContainerPtr pRCLC = Moo::rc().lightContainer();
		Moo::LightContainerPtr pRCSLC = Moo::rc().specularLightContainer();
		static Moo::LightContainerPtr pLC = new Moo::LightContainer;
		static Moo::LightContainerPtr pSLC = new Moo::LightContainer;

		Moo::rc().lightContainer( pSpace->lights() );

		static DogWatch s_drawTerrain("Terrain draw");

		Moo::rc().pushRenderState( D3DRS_FILLMODE );

		Moo::rc().setRenderState( D3DRS_FILLMODE,
			( UmbraHelper::instance().wireFrameTerrain() ) ?
				D3DFILL_WIREFRAME : D3DFILL_SOLID );
	
		s_drawTerrain.start();
		if (pTerrainOverride_)
		{
			(*pTerrainOverride_)();
		}
		else
		{
			Terrain::BaseTerrainRenderer::instance()->drawAll();
		}
		s_drawTerrain.stop();

		Moo::rc().popRenderState();

		pLC->directionals().clear();
		pLC->ambientColour( ChunkManager::instance().cameraSpace()->ambientLight() );
		if (ChunkManager::instance().cameraSpace()->sunLight())
			pLC->addDirectional( ChunkManager::instance().cameraSpace()->sunLight() );
		pSLC->directionals().clear();
		pSLC->ambientColour( ChunkManager::instance().cameraSpace()->ambientLight() );
		if (ChunkManager::instance().cameraSpace()->sunLight())
			pSLC->addDirectional( ChunkManager::instance().cameraSpace()->sunLight() );

		Moo::rc().lightContainer( pLC );
		Moo::rc().specularLightContainer( pSLC );

		Moo::VisualCompound::drawAll();
		Moo::Visual::drawBatches();

		Moo::rc().lightContainer( pRCLC );
		Moo::rc().specularLightContainer( pRCSLC );

		pLastChunk_ = NULL;
	}
}


/**
 * 
 */
void ChunkCommander::enableDepthTestState()
{
	BW_GUARD;
	if (depthStateEnabled_)
		return;

	DX::Device* dev = Moo::rc().device();

	Moo::rc().push();

	Moo::rc().pushRenderState( D3DRS_CULLMODE );
	Moo::rc().pushRenderState( D3DRS_ALPHATESTENABLE );
	Moo::rc().pushRenderState( D3DRS_COLORWRITEENABLE1 );

	// Set up state and draw.

	Moo::rc().setFVF( Moo::VertexXYZ::fvf() );
	Moo::rc().setVertexShader( NULL );
	Moo::rc().setPixelShader( NULL );

	Moo::rc().setRenderState( D3DRS_ALPHATESTENABLE, D3DZB_FALSE );
	Moo::rc().setRenderState( D3DRS_ZWRITEENABLE, D3DZB_FALSE );
	Moo::rc().setRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
	Moo::rc().setRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
	Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE, 0 );
	Moo::rc().setWriteMask( 1, 0 );

	Moo::rc().setRenderState( D3DRS_CULLMODE, D3DCULL_NONE  );

	// we need to disable reflection surface clipping plane on the query boxes
	if (clipEnabled)
		Moo::rc().setRenderState(D3DRS_CLIPPLANEENABLE, 0);

	dev->SetTransform( D3DTS_WORLD, &Matrix::identity );
	dev->SetTransform( D3DTS_PROJECTION, &Moo::rc().projection() );

	depthStateEnabled_ = true;
}

/**
 *
 */
void ChunkCommander::disableDepthTestState()
{
	BW_GUARD;
	if (!depthStateEnabled_)
		return;

	DX::Device* dev = Moo::rc().device();

	// re-enable clipping plane 
	if (ChunkUmbra::clipPlaneSupported() && clipEnabled)
		Moo::rc().setRenderState(D3DRS_CLIPPLANEENABLE, 1);

	// restore state
	Moo::rc().setRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
	Moo::rc().setRenderState(D3DRS_ZWRITEENABLE, D3DZB_TRUE);
	Moo::rc().setRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
	Moo::rc().popRenderState();
	Moo::rc().popRenderState();
	Moo::rc().popRenderState();

	Moo::rc().pop();

	depthStateEnabled_ = false;
}

/**
 * This method is for debugging purposes only..
 * It flushes the rendering queue.
 */

static void sync()
{
	BW_GUARD;
	IDirect3DQuery9* pEventQuery;
	Moo::rc().device()->CreateQuery(D3DQUERYTYPE_EVENT, &pEventQuery);

	// Add an end marker to the command buffer queue.
	pEventQuery->Issue(D3DISSUE_END);

	// Force the driver to execute the commands from the command buffer.
	// Empty the command buffer and wait until the GPU is idle.
	while(S_FALSE == pEventQuery->GetData( NULL, 0, D3DGETDATA_FLUSH ))
		;

	pEventQuery->Release();
}

/**
 *
 */

void ChunkCommander::drawStencilModel(UmbraPortal* portal, const Matrix& worldMtx)
{
	BW_GUARD;
	// Set up state and draw.


	Moo::rc().setFVF( Moo::VertexXYZ::fvf() );

	Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE, 0 );
	Moo::rc().setRenderState( D3DRS_CULLMODE, D3DCULL_NONE  );

	Moo::rc().push();
	Moo::rc().world(Matrix::identity);

	Moo::rc().device()->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 
										0, 
										portal->vertexCount(), 
										portal->triangleCount(), 
										portal->triangles(), 
										D3DFMT_INDEX16, 
										portal->vertices(), 
										sizeof(Vector3));

	Moo::rc().pop();
}

void setDepthRange(float n, float f)
{
	BW_GUARD;
	DX::Viewport vp;
	Moo::rc().getViewport(&vp);
	vp.MinZ = n;
	vp.MaxZ = f;
	Moo::rc().setViewport(&vp);
}


static bool colorPass = false;

/**
 *	This method implements our callback from the umbra scene traversal
 */
void ChunkCommander::command(Command c)
{
	BW_GUARD_PROFILER( ChunkCommander_command );
	static DogWatch s_commander( "Commander callback" );
	s_commander.start();

	switch (c)
	{
		// Begin traversal
		case QUERY_BEGIN:
		{
			storedView_ = Moo::rc().view();
			viewParametersChanged_ = false;

			disableDepthTestState();
			colorPass = false;
			Moo::rc().setRenderState(D3DRS_STENCILENABLE, FALSE);

			cachedItems_.clear();

			pLastChunk_ = NULL;

			ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
			if (pSpace)
				Moo::rc().lightContainer( pSpace->lights() );

			Moo::rc().push();

			break;
		}

		// We have finished traversal
		case QUERY_END:
		{
			Moo::rc().view(storedView_);
			Moo::rc().setRenderState(D3DRS_STENCILENABLE, FALSE); 

			LineHelper::instance().purge();

			flush();

			Moo::rc().pop();

			break;
		}
	
		case FLUSH_DEPTH:
		{
			flush();
			break;
		}

		case OCCLUSION_QUERY_BEGIN:
		{
			OcclusionQuery* query = getOcclusionQuery();
			Moo::OcclusionQuery* pOcclusionQuery = pServices_->getQuery( query->getIndex() );
			Moo::rc().beginQuery( pOcclusionQuery );

			break;
		};

		case OCCLUSION_QUERY_END:
		{
			OcclusionQuery* query = getOcclusionQuery();
			Moo::OcclusionQuery* pOcclusionQuery = pServices_->getQuery( query->getIndex() );
			Moo::rc().endQuery( pOcclusionQuery );

			Moo::rc().depthOnly( false );

			break;
		};

		case OCCLUSION_QUERY_GET_RESULT:
		{
			BW_GUARD_PROFILER( ChunkCommander_occlusionStall );
			static DogWatch s_boxQuery( "GetOcclusionResult" );
			s_boxQuery.start();
		
			OcclusionQuery* query = getOcclusionQuery();
			Moo::OcclusionQuery* pOcclusionQuery = pServices_->getQuery( query->getIndex() );

			int visiblePixels = 0;

			bool available = Moo::rc().getQueryResult(visiblePixels, pOcclusionQuery, query->getWaitForResult());
			
			query->setResult(available, visiblePixels);

			s_boxQuery.stop();

			break;
		}

		// An object is visible
		case OCCLUSION_QUERY_DRAW_TEST_DEPTH:
		{
			BW_GUARD_PROFILER( ChunkCommander_occlusionQuery );
			static DogWatch s_renderTestDepth( "IssueOcclusionQuery" );
			s_renderTestDepth.start();

			enableDepthTestState();

			OcclusionQuery* query = getOcclusionQuery();

			Umbra::Matrix4x4 obbToCamera;
			query->getToCameraMatrix((Umbra::Matrix4x4&)obbToCamera);

			DX::Device* dev = Moo::rc().device();

			dev->SetTransform( D3DTS_VIEW, (const D3DMATRIX*)&obbToCamera );

			uint32 vertexBase=0;
			bool locked = Moo::DynamicVertexBuffer<Moo::VertexXYZ>::instance().lockAndLoad( 
				(const Moo::VertexXYZ*)query->getVertices(), query->getVertexCount(), vertexBase );

			MF_ASSERT( query->getTriangleCount() <= ( s_queryTestBoxes * 12 ) );

			s_queryTestIndexBuffer.set();

			Moo::DynamicVertexBuffer<Moo::VertexXYZ>::instance().set(); 
            Moo::rc().drawIndexedPrimitive( 
                D3DPT_TRIANGLELIST,  
                vertexBase,  
                0,  
                query->getVertexCount(),   
                0, 
                query->getTriangleCount() ); 

			s_renderTestDepth.stop();
			break;
		}

		case INSTANCE_DRAW_DEPTH:
		{
			// Get the umbra object that is visible
			const Instance* instance = getInstance();
			Umbra::Object*	object = instance->getObject();
			UmbraDrawItem* pItem = (UmbraDrawItem*)object->getUserPointer();

			if (pItem == NULL)
				break;

			// TODO: make sure only actual occluders have the occlusion flag set

			// Make sure we are using the correct view matrix
			if (viewParametersChanged_)
			{
				Moo::rc().view(storedView_);
				viewParametersChanged_ = false;
			}

			if ( UmbraHelper::instance().depthOnlyPass() )
				Moo::rc().depthOnly( true );

			disableDepthTestState();
			pLastChunk_ = pItem->drawDepth( pLastChunk_ );

			break;
		}

		// An object is visible
		case INSTANCE_VISIBLE:
		{
			if ( !colorPass )
				Moo::rc().setRenderState(D3DRS_STENCILENABLE, FALSE);

			// Get the umbra object that is visible
			const Instance* instance = getInstance();
			Umbra::Object*	object = instance->getObject();

			UmbraDrawItem* pItem = (UmbraDrawItem*)object->getUserPointer();

			if (pItem == NULL)
				break;

			colorPass = true;

			// This is only needed for wireframe mode
			cachedItems_.push_back(pItem);

			// when occlusion culling objects are rendered in OCCLUSION_QUERY_RENDER_INSTANCE_DEPTH
			// TODO objects that are not occluders should still be rendered here
			// TODO cache these until the end of resolvevisibility to improve parallelism
			//if (!UmbraHelper::instance().occlusionCulling() || !object->test(Umbra::Object::OCCLUDER))
			Moo::rc().depthOnly( false );

			// Make sure we are using the correct view matrix
			if (viewParametersChanged_)
			{
				Moo::rc().view(storedView_);
				viewParametersChanged_ = false;
			}


			disableDepthTestState();
			pLastChunk_ = pItem->draw( pLastChunk_ );

			break;
		}

		case VIEW_PARAMETERS_CHANGED:
		{
			disableDepthTestState();
			flush();
			
			LineHelper::instance().purge();

			const Viewer* viewer = getViewer();

			Matrix view;
		
			viewer->getCameraToWorldMatrix((Umbra::Matrix4x4&)view);
			view.invert();
			Moo::rc().view(view);
			viewParametersChanged_ = true;

			if (viewer->isMirrored())
			{
				Moo::rc().setRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
				Moo::rc().mirroredTransform(true);
			}
			else
			{
				Moo::rc().setRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
				Moo::rc().mirroredTransform(false);
			}

			if (ChunkUmbra::clipPlaneSupported() && viewer->getFrustumPlaneCount() == 7)
			{
				// additional clipping plane for virtual portals
				Matrix proj = Moo::rc().projection();

				proj.invert();
				proj.transpose();
				Vector4 plane;
				viewer->getFrustumPlane(6, (Umbra::Vector4&)plane);
				D3DXPlaneTransform((D3DXPLANE*)&plane.x, (D3DXPLANE*)&plane.x, &proj);

				Moo::rc().setRenderState(D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0);
				clipEnabled = true;

				Moo::rc().device()->SetClipPlane(0, &plane.x);
			}
			else
			{
				Moo::rc().setRenderState(D3DRS_CLIPPLANEENABLE, 0);
				clipEnabled = false;
			}
			break;
		}

		case STENCIL_MASK:
		{	
			if (colorPass)
				break;

			disableDepthTestState();

			Umbra::Matrix4x4 objectToCamera;
			const Umbra::Commander::Instance* instance = getInstance();

			instance->getObjectToCameraMatrix(objectToCamera);
			const Umbra::Object* portalObject = instance->getObject();

			UmbraPortal* portal = (UmbraPortal*)portalObject->getUserPointer();

			Moo::Material::setVertexColour();

			//---------------------------------------------------
			// Increment/Decrement stencil buffer values
			//---------------------------------------------------

			int test,write;
			getStencilValues(test, write);
 			bool increment = (write>test);

			Moo::rc().setRenderState(D3DRS_STENCILENABLE, TRUE);
			Moo::rc().setRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
			Moo::rc().setRenderState(D3DRS_STENCILREF, test);
			Moo::rc().setRenderState(D3DRS_STENCILMASK, 0x3f);

			if(increment)
			{
				Moo::rc().setRenderState(D3DRS_STENCILFAIL , D3DSTENCILOP_KEEP);
				Moo::rc().setRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
				Moo::rc().setRenderState(D3DRS_STENCILPASS , D3DSTENCILOP_INCR);
			}
			else
			{
				Moo::rc().setRenderState(D3DRS_STENCILFAIL , D3DSTENCILOP_KEEP);
				Moo::rc().setRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_DECR);
				Moo::rc().setRenderState(D3DRS_STENCILPASS , D3DSTENCILOP_DECR);
			}

			Moo::rc().setRenderState(D3DRS_ZWRITEENABLE, D3DZB_FALSE);

			Moo::rc().pushRenderState( D3DRS_COLORWRITEENABLE1 );
			Moo::rc().setWriteMask( 1, 0 );
			drawStencilModel(portal, (Matrix&)objectToCamera);

			//---------------------------------------------------
			// Restore state
			//---------------------------------------------------

			setDepthRange	(0.0, 1.0);

			Moo::rc().setRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

			Moo::rc().setRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
			Moo::rc().popRenderState();
			Moo::rc().setRenderState(D3DRS_ZWRITEENABLE, D3DZB_TRUE);

			//---------------------------------------------------
			// Set stencil variables to correct state for subsequent normal objects
			//---------------------------------------------------

			Moo::rc().setRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
			Moo::rc().setRenderState(D3DRS_STENCILREF, write);
			Moo::rc().setRenderState(D3DRS_STENCILMASK, 0x3f);

			Moo::rc().setRenderState(D3DRS_STENCILFAIL , D3DSTENCILOP_KEEP);
			Moo::rc().setRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
			Moo::rc().setRenderState(D3DRS_STENCILPASS , D3DSTENCILOP_KEEP);

			break;
		};

		// Umbra wants us to draw a 2d debug line
		case DRAW_LINE_2D:
		{
			disableDepthTestState();

			static Vector4 begin(0,0,0,1);
			static Vector4 end(0,0,0,1);
			static Moo::Colour colour;

			// Get a linehelper from umbra
			getLine2D((Umbra::Vector2&)begin, (Umbra::Vector2&)end, (Umbra::Vector4&)colour);

			// Add line to the linehelper
			LineHelper::instance().drawLineScreenSpace( begin, end, colour );
			break;
		}

		// Umbra wants us to draw a 3d debug line
		case DRAW_LINE_3D:
		{
			disableDepthTestState();

			Vector3 begin;
			Vector3 end;
			Moo::Colour colour;

			// Get a linehelper from umbra
			getLine3D((Umbra::Vector3&)begin, (Umbra::Vector3&)end, (Umbra::Vector4&)colour);

			// Add line to the linehelper
			LineHelper::instance().drawLine( begin, end, colour );
			break;
		}
	};

	s_commander.stop();

}

// -----------------------------------------------------------------------------
// Section: ChunkUmbraServices
// -----------------------------------------------------------------------------

/*
 *	This method outputs a umbra error
 */
void ChunkUmbraServices::error(const char* message)
{
	BW_GUARD;
	CRITICAL_MSG( message );
}

namespace Moo { extern THREADLOCAL( bool ) g_renderThread; }

/*
 * This is the entermutex method for umbra.
 * As we only allow access to umbra from the render thread in BigWorld, 
 * this is not implemented as a mutex, but triggers an assert if it is accessed
 * from any other thread.
 * We only allow umbra access from the render thread as umbra can hold on to the
 * mutex for an extended period of time, and as such we do not want
 * the loading thread to block for this amount of time.
 */
void ChunkUmbraServices::enterMutex()
{
	BW_GUARD;
	MF_ASSERT(Moo::g_renderThread);
}

/*
 *	This is the leavemutex method for umbra.
 *	See the comment for ChunkUmbraServices::enterMutex above for the reasoning 
 *  behind this being an assert only.
 */
void ChunkUmbraServices::leaveMutex()
{
	BW_GUARD;
	MF_ASSERT( Moo::g_renderThread );
}

bool ChunkUmbraServices::allocateQueryObject(int index)
{
	BW_GUARD;
	queries_.resize(index+1);
	queries_[index] = Moo::rc().createOcclusionQuery();
	return queries_[index] != NULL;
}

void ChunkUmbraServices::releaseQueryObject(int index)
{
	BW_GUARD;
	Moo::rc().destroyOcclusionQuery( queries_[index] );
	queries_[index] = NULL;
}

// -----------------------------------------------------------------------------
// Section: UmbraHelper
// -----------------------------------------------------------------------------

/*
 *	Helper class to contain one umbra statistic
 */
class UmbraHelper::Statistic
{
public:
	void set( Umbra::LibraryDefs::Statistic statistic )
	{
		statistic_ = statistic;
	}
	float get() const
	{
		return Umbra::Library::getStatistic( statistic_ );
	}
private:
	Umbra::LibraryDefs::Statistic statistic_;
};

/*
 *	This method inits the umbra helper
 */
void UmbraHelper::init()
{
	BW_GUARD;
	static Statistic stats[Umbra::LibraryDefs::STAT_MAX];

	// Register our debug watchers
	MF_WATCH( "Render/Umbra/occlusionCulling",	*this, MF_ACCESSORS( bool, UmbraHelper, occlusionCulling ),
		"Enable/disable umbra occlusion culling, this still uses umbra for frustum culling" );
	MF_WATCH( "Render/Umbra/enabled",		   *this, MF_ACCESSORS( bool, UmbraHelper, umbraEnabled ),
		"Enable/disable umbra, this causes the rendering to bypass umbra and use the BigWorld scene traversal" );
	MF_WATCH( "Render/Umbra/drawTestModels",	*this, MF_ACCESSORS( bool, UmbraHelper, drawTestModels ),
		"Draw the umbra test models" );
	MF_WATCH( "Render/Umbra/drawWriteModels",	*this, MF_ACCESSORS( bool, UmbraHelper, drawWriteModels ),
		"Draw the umbra writemodels" );
	MF_WATCH( "Render/Umbra/drawObjectBounds",	*this, MF_ACCESSORS( bool, UmbraHelper, drawObjectBounds ),
		"Draw the umbra object bounds" );
	MF_WATCH( "Render/Umbra/drawVoxels",		*this, MF_ACCESSORS( bool, UmbraHelper, drawVoxels ),
		"Draw the umbra voxels" );
	MF_WATCH( "Render/Umbra/drawSilhouettes",	*this, MF_ACCESSORS( bool, UmbraHelper, drawSilhouettes ),
		"Draw the umbra object silhouettes (software mode only)" );
	MF_WATCH( "Render/Umbra/drawQueries",		*this, MF_ACCESSORS( bool, UmbraHelper, drawQueries ),
		"Draw the umbra occlusion queries (hardware mode only)" );
	MF_WATCH( "Render/Umbra/flushTrees",		*this, MF_ACCESSORS( bool, UmbraHelper, flushTrees ),
		"Flush speedtrees as part of umbra, if this is set to false, all speedtrees are flushed after "
		"the occlusion queries have been issued. "
		"Which means that speedtrees do not act as occluders" );
	MF_WATCH( "Render/Umbra/depthOnlyPass",		*this, MF_ACCESSORS( bool, UmbraHelper, depthOnlyPass ),
		"Do seperate depth and colour passes as requested by Umbra" );
	
	// Set up the watchers for the statistics
	for (uint32 i = 0; i < (uint32)Umbra::LibraryDefs::STAT_MAX; i++)
	{
		std::string statName(Umbra::Library::getStatisticName( (Umbra::LibraryDefs::Statistic)i ));
		std::string statNameBegin = statName;

		stats[i].set( (Umbra::LibraryDefs::Statistic)i );

		MF_WATCH( (std::string( "Render/Umbra/Statistics/" ) + statNameBegin + "/" + statName).c_str(), 
			stats[i], &UmbraHelper::Statistic::get );
	}
}

void UmbraHelper::fini()
{
}

/**
 *	This method returns the umbra helper instance
 */
UmbraHelper& UmbraHelper::instance()
{
	static UmbraHelper s_instance;
	return s_instance;
}

/*
 *	macro to help implement the umbra flag switches
 */
#define IMPLEMENT_Umbra_HELPER_FLAG( method, flag )\
bool UmbraHelper::method() const\
{\
	return (Umbra::Library::getFlags(Umbra::LibraryDefs::LINEDRAW) & Umbra::LibraryDefs::flag) != 0;\
}\
void UmbraHelper::method(bool b) \
{ \
	if (b)\
		Umbra::Library::setFlags(Umbra::LibraryDefs::LINEDRAW, Umbra::LibraryDefs::flag); \
	else\
		Umbra::Library::clearFlags(Umbra::LibraryDefs::LINEDRAW, Umbra::LibraryDefs::flag); \
}

IMPLEMENT_Umbra_HELPER_FLAG( drawTestModels, LINE_OBJECT_TEST_MODEL )
IMPLEMENT_Umbra_HELPER_FLAG( drawWriteModels, LINE_OBJECT_WRITE_MODEL )
IMPLEMENT_Umbra_HELPER_FLAG( drawObjectBounds, LINE_OBJECT_BOUNDS )
IMPLEMENT_Umbra_HELPER_FLAG( drawVoxels, LINE_VOXELS )
IMPLEMENT_Umbra_HELPER_FLAG( drawSilhouettes, LINE_SILHOUETTES )
IMPLEMENT_Umbra_HELPER_FLAG( drawQueries, LINE_OCCLUSION_QUERIES )

UmbraHelper::UmbraHelper() :
	occlusionCulling_(true),
	umbraEnabled_(true),
	flushTrees_(true),
	depthOnlyPass_(true),
	wireFrameTerrain_( false )
{
}

UmbraHelper::~UmbraHelper()
{
}

#endif
