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

#include "water.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/diary.hpp"
#include "cstdmf/dogwatch.hpp"

#include "math/colour.hpp"

#include "moo/dynamic_vertex_buffer.hpp"
#include "moo/dynamic_index_buffer.hpp"
#include "moo/vertex_formats.hpp"
#include "moo/effect_material.hpp"
#include "moo/effect_visual_context.hpp"
#include "moo/graphics_settings.hpp"
#include "moo/mrt_support.hpp"
#include "moo/resource_load_context.hpp"
#if UMBRA_ENABLE
#include "terrain/base_terrain_block.hpp"
#endif //UMBRA_ENABLE

#include "effect_parameter_cache.hpp"
#include "enviro_minder.hpp"
#include "environment_cube_map.hpp"
#ifdef EDITOR_ENABLED
#include "appmgr/commentary.hpp"
#include "romp/geometrics.hpp"
#endif

#include "fog_controller.hpp"

#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "chunk/chunk.hpp"
#if UMBRA_ENABLE
#include "chunk/chunk_terrain.hpp"
#endif //UMBRA_ENABLE

#include "speedtree/speedtree_renderer.hpp"

#include "cstdmf/watcher.hpp"
#include "cstdmf/bgtask_manager.hpp"
#include "cstdmf/concurrency.hpp"

#include "resmgr/auto_config.hpp"
#include "resmgr/bin_section.hpp"
#include "resmgr/string_provider.hpp"
#include "resmgr/data_section_census.hpp"

#include "romp/line_helper.hpp"
#include "py_water_volume.hpp"

#ifndef CODE_INLINE
#include "water.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Romp", 0 )

PROFILER_DECLARE( Water_setupRenderState, "Water SetupRenderState" );
PROFILER_DECLARE( Water_stillValid, "Water stillValid" );
PROFILER_DECLARE( Water_deleteWater, "Water deleteWater" );
PROFILER_DECLARE( Water_addTerrainItem, "Water addTerrainItem" );
PROFILER_DECLARE( Water_eraseTerrainItem, "Water eraseTerrainItem" );

// ----------------------------------------------------------------------------
// Statics
// ----------------------------------------------------------------------------

uint32 Waters::s_nextMark_ = 1;
uint32 Waters::s_lastDrawMark_ = -1;
static AutoConfigString s_waterEffect( "environment/waterEffect" );
static AutoConfigString s_simulationEffect( "environment/waterSimEffect" );
static AutoConfigString s_screenFadeMap( "environment/waterScreenFadeMap" );
static bool s_enableDrawPrim = true;

// Water statics
Water::VertexBufferPtr			Water::s_quadVertexBuffer_;
Water::WaterRenderTargetMap		Water::s_renderTargetMap_;
bool							Water::s_cullCells_ = true;
float							Water::s_cullDistance_ = 1.2f;
								//a bit further than the far plane to avoid popping

float							Water::s_maxSimDistance_ = 200.0f;
float							Water::s_sceneCullDistance_ = 300.f;
float							Water::s_sceneCullFadeDistance_ = 30.f;

// Water statics
#ifndef EDITOR_ENABLED
bool                            Water::s_backgroundLoad_	= true;
#else
bool                            Water::s_backgroundLoad_	= false;
#endif

// Waters statics
Moo::EffectMaterialPtr			Waters::s_effect_			= NULL;
Moo::EffectMaterialPtr			Waters::s_simulationEffect_ = NULL;
bool							Waters::s_drawWaters_		= true;
bool							Waters::s_drawReflection_	= true;
bool							Waters::s_simulationEnabled_ = true;
bool							Waters::s_highQuality_		= true;
int								Waters::s_simulationLevel_	= 2;
float							Waters::s_autoImpactInterval_ = 0.4f;

#ifdef EDITOR_ENABLED
bool							Waters::s_projectView_		= false;
#endif


#ifdef DEBUG_WATER

bool							Water::s_debugSim_ = true;
int								Water::s_debugCell_ = -1;
int								Water::s_debugCell2_ = -1;

#define WATER_STAT(exp) exp

static uint 					s_waterCount=0;
static uint 					s_waterVisCount=0;
static uint 					s_activeCellCount=0;
static uint 					s_activeEdgeCellCount=0;
static uint 					s_movementCount=0;

#else
#define WATER_STAT(exp)
#endif //DEBUG_WATER

// ----------------------------------------------------------------------------
// Defines
// ----------------------------------------------------------------------------
#define MAX_SIM_TEXTURE_SIZE 256
// defines the distance from the edge of a cell that a movement will activate
// its neighbour
#define ACTIVITY_THRESHOLD 0.3f
#define RLE_KEY_VALUE 53
// ----------------------------------------------------------------------------
// Section: WaterCell
// ----------------------------------------------------------------------------

/**
 *	WaterCell contructor
 */
WaterCell::WaterCell() : 
		nIndices_( 0 ),
		water_( 0 ),
		xMax_( 0 ),
		zMax_( 0 ),
		xMin_( 0 ),
		zMin_( 0 ),
		min_(0,0),
		max_(0,0),		
		size_(0,0)
{
	for (int i=0; i<4; i++)
	{
		edgeCells_[i]=NULL;
	}
}

char* neighbours[4] = {	"neighbourHeightMap0",
						"neighbourHeightMap1",
						"neighbourHeightMap2",
						"neighbourHeightMap3" };

/**
 *	Binds the specified neighbour cell.
 */
void WaterCell::bindNeighbour( Moo::EffectMaterialPtr effect, int edge )
{
	BW_GUARD;
	MF_ASSERT(edge<4 && edge>=0);
	if ( edgeCells_[edge] && edgeCells_[edge]->isActive() )
	{
		edgeCells_[edge]->bindAsNeighbour( effect, neighbours[edge] );
	}
	else if (SimulationManager::instance().nullSim())
	{
 		effect->pEffect()->pEffect()->SetTexture( neighbours[edge], 
					SimulationManager::instance().nullSim()->pTexture() );
	}
	else
	{
		effect->pEffect()->pEffect()->SetTexture( neighbours[edge], NULL );
	}
}

void WaterCell::activateEdgeCell( uint index )
{
	BW_GUARD;
	this->addEdge((index+2) % 4);
	this->resetIdleTimer();
	this->edgeActivation_ = true;
	this->perturbed( true );
}

/**
 *	Checks for movement activity within a cell and activates/deactivates 
 *	accordingly
 */
void WaterCell::checkActivity(	SimulationCellPtrList& activeList,
								WaterCellPtrList& edgeList )
{
	BW_GUARD;
	if (hasMovements())
		perturbed(true);
	else if ( !edgeActivation_ ) //edge-activated cells shouldnt be disabled here
		perturbed(false);

	if (shouldActivate())
	{
		activate();
		activeList.push_back( this );
	}
	else 
	{
		bool outOfRange = false;
		if (isActive())
		{
			outOfRange = (cellDistance_ > Water::s_maxSimDistance_);			

			if (outOfRange)
			{
				clearMovements();
				perturbed(false);
			}
		}

		if ( outOfRange || shouldDeactivate() )
		{
			deactivate();
			activeList.remove( this );
			edgeActivation_=false;
		}
	}

	//inter-cell simulation....
	if (Waters::instance().simulationLevel() > 1) // only do for high detail settings
	{
		if (isActive())
		{
			if (edgeActivation_ == false)
				edge(0);

			int edges = getActiveEdge(ACTIVITY_THRESHOLD);

			if (movements().size())
			{
				this->edge(edges);
				if (edges > 0)
				{
					WaterCellPtrList cornerCells;

					for (int i=0; i<4; i++)
					{
						WaterCell* edgeCell = edgeCells_[i];
						if (edgeCell && (edges & (1<<i)))
						{
							edgeCell->activateEdgeCell( i );
							edgeList.push_back( edgeCell );

							edgeActivation_ = true;

							// check for a corner.
							uint cornerIdx = (i+1) % 4;
							if ( (edges & (1<<(cornerIdx))) )
							{
								WaterCell* cornerCell1 = edgeCell->edgeCell( cornerIdx );
								WaterCell* cornerCell2 = edgeCells_[ cornerIdx ];
								if ( cornerCell1 )
								{
									// Activate the edges around the corner.
									edgeCell->addEdge( cornerIdx );
									if (cornerCell2)
									{
										cornerCell1->addEdge( (cornerIdx+1) % 4 );
										cornerCell2->addEdge( i );
									}
									cornerCell1->activateEdgeCell( cornerIdx );
									cornerCells.push_back( cornerCell1 );
								}
							}
						}
					}

					// Cells at the front of this list have a higher activation priority, 
					// cells on an edge have priority over cells on a corner.
					edgeList.insert( edgeList.end(), cornerCells.begin(), cornerCells.end() );
				}
			}

			if (edgeActivation_)
			{
				edgeList.push_back( this );

				bool deactivate=!hasMovements();
				for (int i=0; i<4 && deactivate; i++)
				{
					if ( edgeCells_[i] && 
							edgeCells_[i]->isActive() )
					{
						if ( edgeCells_[i]->hasMovements() )
							deactivate = false;
					}
				}
				if (deactivate)
					perturbed( false );
			}
		}
		else if (edgeActivation_)
		{
			perturbed( false );
			edgeList.push_back( this );
		}
	}
}


/**
 *	Activity check when activated by a neighbour
 */
void WaterCell::checkEdgeActivity( SimulationCellPtrList& activeList )
{
	BW_GUARD;
	if (edgeActivation_)
	{
		if (hasMovements())
			perturbed(true);			
		if (shouldActivate())
		{
			activate();
			activeList.push_back( this );
		}
		else if (shouldDeactivate())
		{
			edgeActivation_ = false;
			deactivate();
			activeList.remove( this );
		}
	}
}


/**
 * Update the cell distance.
 */
void WaterCell::updateDistance(const Vector3& camPos)
{
	BW_GUARD;
	BoundingBox box = water_->bb();
	Vector3 cellMin(min().x, 0, min().y);
	Vector3 cellMax(max().x, 0, max().y);

	//TODO: cells may need to store their BBs...
	box.setBounds(	box.minBounds() + cellMin,
					box.minBounds() + cellMax );
	cellDistance_ = box.distance(camPos);
}


/**
 *	Initialise a water cell
 */
bool WaterCell::init( Water* water, Vector2 start, Vector2 size )
{ 
	BW_GUARD;
	if (!water)
		return false;

	water_ = water;

	min_ = start;
	max_ = (start+size);
	size_ = size;

	xMin_ = int( ceilf( min().x / water_->config_.tessellation_)  );
	zMin_ = int( ceilf( min().y / water_->config_.tessellation_)  );

	xMax_ = int( ceilf( (max().x) / water_->config_.tessellation_)  ) + 1;
	zMax_ = int( ceilf( (max().y) / water_->config_.tessellation_)  ) + 1;

	if (xMax_ > int( water_->gridSizeX_ ))
		xMax_ = int( water_->gridSizeX_ );

	if (zMax_ > int( water_->gridSizeZ_ ))
		zMax_ = int( water_->gridSizeZ_ );

	return true;
}


/**
 *	Release all managed data
 */
void WaterCell::deleteManaged()
{
	BW_GUARD;
	indexBuffer_.release();
}

/**
 *	Take the supplied vertex index and map it to a spot in a new
 *	vertex buffer.
 */
uint32 Water::remapVertex( uint32 index )
{
	BW_GUARD;
	uint32 newIndex = index;
	if (remappedVerts_.size())
	{
		std::map<uint32, uint32>& currentMap = remappedVerts_.back();
		std::map<uint32,uint32>::iterator it = currentMap.find(newIndex);
		// check if it's already mapped
		if (it == currentMap.end())
		{
			// not found, remap
			newIndex = (uint32)currentMap.size();
			currentMap[index] = newIndex;
		}
		else
		{
			newIndex = it->second;
		}
	}
	return newIndex;
}

/**
 *	Remaps a list of vertex indices to ensure they are contained within a
 *	single vertex buffer.
 */
template< class T >
uint32 Water::remap( std::vector<T>& dstIndices,
					const std::vector<uint32>& srcIndices )
{
	BW_GUARD;
	uint32 maxVerts = Moo::rc().maxVertexIndex();
	// Make a new buffer
	if (remappedVerts_.size() == 0)
	{
		remappedVerts_.resize(1);
	}
	// Allocate the destination index buffer.
	dstIndices.resize(srcIndices.size());

	// Transfer all the indices, remapping as necesary.
	for (uint i=0; i<srcIndices.size() ; i++)
	{
		dstIndices[i] = (T)this->remapVertex( srcIndices[i] );
	}

	// check if the current buffer has overflowed
	if ( remappedVerts_.back().size() > maxVerts)
	{
		// overflow, create new buffer + remap again
		remappedVerts_.push_back(std::map<uint32, uint32>());	
		for (uint i=0; i<srcIndices.size() ; i++)
		{
			dstIndices[i] = (T)this->remapVertex( srcIndices[i] );
		}

		// If it's full again then it's an error (too many verts)
		if ( remappedVerts_.back().size() > maxVerts )
		{
			ERROR_MSG("Water::remap( ): Maximum vertex count "
				"exceeded, please INCREASE the \"Mesh Size\" parameter.\n" );
			remappedVerts_.pop_back();
			return 0;
		}
	}
	return remappedVerts_.size();
}

/**
 *	Build the indices to match the templated value
 *  NOTE: only valid for uint32 and uint16
 */
template< class T >
void WaterCell::buildIndices( )
{
	BW_GUARD;
	#define WIDX_CHECKMAX  maxIndex = indices_32.back() > maxIndex ? \
									indices_32.back() : maxIndex;

	std::vector< T >		indices;
	std::vector< uint32 >	indices_32;

	int lastIndex = -1;
	bool instrip = false;
	uint32 gridIndex = 0;
	uint32 maxIndex=0;
	uint32 maxVerts = Moo::rc().maxVertexIndex();
	D3DFORMAT format;
	uint32 size = sizeof(T);

	if (size == 2)
		format = D3DFMT_INDEX16;
	else
		format = D3DFMT_INDEX32;

	// Build the master index list first.
	// then if any of the verts exceed the max... remap them all into a 
	// new vertex buffer.
	gridIndex = xMin_ + (water_->gridSizeX_*zMin_);
	for (uint z = uint(zMin_); z < uint(zMax_) - 1; z++)
	{
		for (uint x = uint(xMin_); x < uint(xMax_); x++)
		{
			bool lastX = (x == xMax_ - 1);

			if (!instrip && !lastX &&
				water_->wIndex_[ gridIndex ] >= 0 &&
				water_->wIndex_[ gridIndex + 1 ] >= 0 &&
				water_->wIndex_[ gridIndex + water_->gridSizeX_ ] >= 0 &&
				water_->wIndex_[ gridIndex + water_->gridSizeX_ + 1 ] >= 0
				)
			{
				if (lastIndex == -1) //first
					lastIndex = water_->wIndex_[ gridIndex ];

				indices_32.push_back( uint32( lastIndex ) );

				indices_32.push_back( uint32( water_->wIndex_[ gridIndex ] ));
				WIDX_CHECKMAX

				indices_32.push_back( uint32( water_->wIndex_[ gridIndex ] ));
				WIDX_CHECKMAX

				indices_32.push_back( uint32( water_->wIndex_[ gridIndex + water_->gridSizeX_] ));
				WIDX_CHECKMAX

				instrip = true;
			}
			else
			{
				if (water_->wIndex_[ gridIndex ] >= 0 &&
					water_->wIndex_[ gridIndex + water_->gridSizeX_] >= 0  &&
					instrip)
				{
					indices_32.push_back( uint32(water_->wIndex_[ gridIndex ] ) );
					WIDX_CHECKMAX

					indices_32.push_back( uint32(water_->wIndex_[ gridIndex + water_->gridSizeX_]));
					WIDX_CHECKMAX

					lastIndex = indices_32.back();
				}
				else
					instrip = false;
			}

			if (lastX)
				instrip = false;

			++gridIndex;
		}
		gridIndex += (water_->gridSizeX_ - xMax_) + xMin_;
	}

	// Process the indices
	bool remap = (maxIndex > maxVerts);
	if (remap)
	{
		vBufferIndex_ = water_->remap<T>( indices, indices_32 );
		if (vBufferIndex_ == 0) //error has occured.
		{
			nIndices_ = 0;		
			return;
		}
	}
	else
	{
		vBufferIndex_ = 0;
		indices.resize( indices_32.size() );
		for (uint i=0; i<indices_32.size() ; i++)
		{
			indices[i] = (T)indices_32[i];		
		}
	}

	// Build the D3D index buffer.
	if (indices.size() > 2)
	{
		indices.erase( indices.begin(), indices.begin() + 2 );
		nIndices_ = indices.size();

		// Create the index buffer
		// The index buffer consists of one big strip covering the whole water
		// cell surface, Does not include sections of water that is under the
		// terrain. made up of individual rows of strips connected by degenerate
		// triangles.
		if( SUCCEEDED( indexBuffer_.create( nIndices_, format, D3DUSAGE_WRITEONLY, D3DPOOL_MANAGED ) ) )
		{
			Moo::IndicesReference ir = indexBuffer_.lock( 0, nIndices_, 0 );
			if(ir.valid())
			{
				ir.fill( &indices.front(), nIndices_ );
				indexBuffer_.unlock();
			}
		}
	}
#undef WIDX_CHECKMAX
}

/**
 *	Create the water cells managed index buffer
 */
void WaterCell::createManagedIndices()
{
	BW_GUARD;
	if (!water_)
		return;
	if (Moo::rc().maxVertexIndex() <= 0xffff)
		buildIndices<uint16>();
	else
		buildIndices<uint32>();
}

// ----------------------------------------------------------------------------
// Section: Water
// ----------------------------------------------------------------------------

namespace { //anonymous

static SimpleMutex * deletionLock_ = NULL;

};


/**
 * Constructor for water.
 *
 * @param config a structure containing the configuration data for the water surface.
 * @param pCollider the collider to use to intersect the water with the scene
 */
Water::Water( const WaterState& config, RompColliderPtr pCollider )
:	config_( config ),
	gridSizeX_( int( ceilf( config.size_.x / config.tessellation_ ) + 1 ) ),
	gridSizeZ_( int( ceilf( config.size_.y / config.tessellation_ ) + 1 ) ),
	time_( 0 ),
	lastDTime_( 1.f ),
	normalTexture_( NULL ),
	screenFadeTexture_( NULL ),
	foamTexture_( NULL ),
	reflectionTexture_( NULL ),
	pCollider_( pCollider ),
	waterScene_( NULL),
	drawSelf_( true ),
	simulationEffect_( NULL ),
	reflectionCleared_( false ),
	paramCache_( new EffectParameterCache() ),
#ifdef EDITOR_ENABLED
	drawRed_( false ),
	highlight_( false ),
#endif
	visible_( true ),
	needSceneUpdate_( false ),
	createdCells_( false ),
	initialised_( false ),
	enableSim_( true ),
	drawMark_( 0 ),
	simpleReflection_( 0.0f ),
	outsideVisible_( false )
{
	BW_GUARD;

	if (!deletionLock_)
		deletionLock_ = new SimpleMutex;

	Waters::instance().push_back(this);

	// Resize the water buffers.
	wRigid_.resize( gridSizeX_ * gridSizeZ_, false );
	wAlpha_.resize( gridSizeX_ * gridSizeZ_, 0 );

	// Create the water transforms.
	transform_.setRotateY( config_.orientation_ );
	transform_.postTranslateBy( config_.position_ );
	invTransform_.invert( transform_ );

	if (Water::backgroundLoad())
    {
	    // do heavy setup stuff in the background
	    BgTaskManager::instance().addBackgroundTask(
			new CStyleBackgroundTask( 
			    &Water::doCreateTables, this,
			    &Water::onCreateTablesComplete, this ) );
    }
    else
	{
	    Water::doCreateTables( this );
	    Water::onCreateTablesComplete( this );
    }

	static bool first = true;
	if (first)
	{
		first = false;
#ifdef DEBUG_WATER
		MF_WATCH( "Client Settings/Water/debug cell", s_debugCell_, 
				Watcher::WT_READ_WRITE,
				"simulation debugging?" );
		MF_WATCH( "Client Settings/Water/debug cell2", s_debugCell2_, 
				Watcher::WT_READ_WRITE,
				"simulation debugging?" );
		MF_WATCH( "Client Settings/Water/debug sim", s_debugSim_, 
				Watcher::WT_READ_WRITE,
				"simulation debugging?" );
#endif
		MF_WATCH( "Client Settings/Water/cell cull enable", s_cullCells_, 
				Watcher::WT_READ_WRITE,
				"enable cell culling?" );
		MF_WATCH( "Client Settings/Water/cell cull dist", s_cullDistance_, 
				Watcher::WT_READ_WRITE,
				"cell culling distance (far plane)" );

		MF_WATCH( "Render/Performance/DrawPrim WaterCell", s_enableDrawPrim,
			Watcher::WT_READ_WRITE,
			"Allow WaterCell to call drawIndexedPrimitive()." );
	}

	//Create python accessor object
	pPyVolume_ = new PyWaterVolume( this );
}


/**
 * Destructor.
 */
Water::~Water()
{
	BW_GUARD;
	simulationEffect_ = NULL;
	delete paramCache_;
	paramCache_ = NULL;

	releaseWaterScene();

#if UMBRA_ENABLE
	terrainItems_.clear();
#endif //UMBRA_ENABLE
}

/**
 *	Check the water pointer is valid for usage.
 *	Used by the background tasks to avoid bad pointers.
 *
 *	@param	water	Pointer to check.
 *
 *	@return			Validity of the pointer.
 */
bool Water::stillValid(Water* water)
{
	BW_GUARD_PROFILER( Water_stillValid );
	if (!water)
	{
		return false;
	}

	if (Water::backgroundLoad())
	{
		if (deletionLock_)
		{
			deletionLock_->grab();

			std::vector< Water* >::iterator it =
				std::find(	Waters::instance().begin(),
							Waters::instance().end(), water );	

			if (it == Waters::instance().end()) // deleted
			{
				deletionLock_->give();
				return false;
			}
		}
		else
			return false;
	}
	return true;
}

/**
 *	Delete a water object. Controlled destruction is required by the 
 *	background tasks.
 *
 *	The water destructor is private so all destruction should come
 *	through here.
 *
 *	@param	water	Water object to destroy.
 *
 */
void Water::deleteWater(Water* water)
{	
	BW_GUARD_PROFILER( Water_deleteWater );
	if (deletionLock_)
	{
		deletionLock_->grab();
	
		std::vector< Water* >::iterator it = std::find( Waters::instance().begin( ), Waters::instance().end( ), water );
		Waters::instance().erase(it);

		delete water;

		bool deleteMutex = (Waters::instance().size() == 0);
		deletionLock_->give();
		if (deleteMutex)
		{
			delete deletionLock_;
			deletionLock_ = NULL;
		}
	}
	else
	{
		std::vector< Water* >::iterator it = std::find( Waters::instance().begin( ), Waters::instance().end( ), water );
		Waters::instance().erase(it);

		delete water;
	}
}


/**
 * Remove the references to the water scene
 */
void Water::releaseWaterScene()
{
	BW_GUARD;
	if (waterScene_)
	{
		waterScene_->removeOwner(this);	
		if ( s_renderTargetMap_.size() && waterScene_->refCount() == 1 )
		{
			float key = config_.position_.y;
			key = floorf(key+0.5f);
			WaterRenderTargetMap::iterator it = s_renderTargetMap_.find(key);
			WaterScene *(&a) = (it->second);
			a = NULL; //a ref
		}
		waterScene_->decRef();
		waterScene_ = NULL;
	}
}


/**
 * Recreate the water surface render data.
 */
void Water::rebuild( const WaterState& config )
{
	BW_GUARD;
	initialised_ = false;
	vertexBuffers_.clear();

	deleteManagedObjects();

	releaseWaterScene();

	config_ = config;
	gridSizeX_ = int( ceilf( config.size_.x / config.tessellation_ ) + 1 );
	gridSizeZ_ = int( ceilf( config.size_.y / config.tessellation_ ) + 1 );

	// Create the water transforms.
	transform_.setRotateY( config_.orientation_ );
	transform_.postTranslateBy( config_.position_ );
	invTransform_.invert( transform_ );

	if( wRigid_.size() != gridSizeX_ * gridSizeZ_ )
	{// don't clear it
		wRigid_.clear();
		wAlpha_.clear();

		wRigid_.resize( gridSizeX_ * gridSizeZ_, false );
		wAlpha_.resize( gridSizeX_ * gridSizeZ_, 0 );
	}

	if (config_.useEdgeAlpha_)
		buildTransparencyTable();
	else
	{
		wRigid_.assign( wRigid_.size(), false );
		wAlpha_.assign( wAlpha_.size(), 0 );
	}

	wIndex_.clear();
	cells_.clear();
	activeSimulations_.clear();
	nVertices_ = createIndices();
	
#ifdef EDITOR_ENABLED
	enableSim_ = true; // Since this can be toggled by the tools dynamically
#else
	enableSim_ = Waters::simulationEnabled();
#endif

	enableSim_ = enableSim_ && Moo::rc().supportsTextureFormat( D3DFMT_A16B16G16R16F ) && config_.useSimulation_;

	createCells();

	setup2ndPhase();

	startResLoader();
}


/**
 * Builds the index table for a water surface.
 * Creates multiple vertex buffers when the vertices go over the 
 * max vertex index
 */
uint32 Water::createIndices( )
{
	BW_GUARD;
	// Create the vertex index table for the water cell.
	uint32 index = 0;
	int32 waterIndex = 0;
	for (int zIndex = 0; zIndex < int( gridSizeZ_ ); zIndex++ )
	{
		for (int xIndex = 0; xIndex < int( gridSizeX_ ); xIndex++ )
		{
			int realNeighbours = 0;
			for (int cz = zIndex - 1; cz <= zIndex + 1; cz++ )
			{
				for (int cx = xIndex - 1; cx <= xIndex + 1; cx++ )
				{
					if (( cx >= 0 && cx < int( gridSizeX_ ) ) &&
						( cz >= 0 && cz < int( gridSizeZ_ ) ))
					{
						if (!wRigid_[ cx + ( cz * gridSizeX_ ) ])
						{
							realNeighbours++;
						}
					}
				}
			}

			if (realNeighbours > 0)
				wIndex_.push_back(waterIndex++);
			else
				wIndex_.push_back(-1);
		}
	}

#ifdef EDITOR_ENABLED
	if (waterIndex > 0xffff)
	{
		Commentary::instance().addMsg( "WARNING! Water surface contains excess"
				" vertices, please INCREASE the \"Mesh Size\" parameter", 1 );
	}
#endif //EDITOR

	return waterIndex;
}



#ifdef EDITOR_ENABLED

/**
 *	Delete the transparency/rigidity file.
 */
void Water::deleteData( )
{
	std::wstring fileName;
	bw_utf8tow( BWResource::resolveFilename( config_.transparencyTable_ ), fileName );
	::DeleteFile( fileName.c_str() );
}


/**
 *	Little helper function to write out some data to a string..
 */
template<class C> void writeVector( std::string& data, std::vector<C>& v )
{
	int clen = sizeof( C );
	int vlen = clen * v.size();
	data.append( (char*) &v.front(), vlen );
}


/**
 *	Writes a 32 bit uint to a string (in 4 chars)
 */
void writeValue(std::string& data, uint32 value )
{
	data.push_back( char( (value & (0xff << 0 ))    ) );
	data.push_back( char( (value & (0xff << 8 ))>>8 ) );
	data.push_back( char( (value & (0xff << 16))>>16) );
	data.push_back( char( (value & (0xff << 24))>>24) );
}


/**
 *	Writes a bool to a string
 */
void writeValue(std::string& data, bool value )
{
	data.push_back( char( value ) );
}


/**
 *	Compresses a vector using very simple RLE
 */
template <class C>
void compressVector( std::string& data, std::vector<C>& v )
{
	MF_ASSERT(v.size());
	uint32 imax = v.size();	
	uint32 prev_i = 0;
	bool first = true;

	C cur_v = v[0];
	C prev_v = !cur_v;
	for (uint32 i=0; i<imax; i++)
	{
		cur_v = v[i];
		uint32 cur_i = i;    
		uint32 c = cur_i - prev_i;

		// test for end of run cases
		if ( (cur_v != prev_v) || (c > 254) || (i==(imax-1)) )
		{
			if (prev_v == static_cast<C>(RLE_KEY_VALUE)) // exception for keyvalue
			{
				data.push_back(static_cast<char>(RLE_KEY_VALUE));
				writeValue(data, prev_v);
				data.push_back( static_cast<char>(c) );
			}
			else
			{
				if (c > 2)
				{
					data.push_back(static_cast<char>(RLE_KEY_VALUE));					
					writeValue(data, prev_v);
					data.push_back( static_cast<char>(c) );
				}
				else
				{
					if (first==false)
					{
						if ( c > 1 )
							writeValue(data, prev_v);
						writeValue(data, prev_v);
					}
				}
			}
			prev_i = cur_i;
		}
		prev_v = cur_v;
		first=false;
	}
	writeValue(data, cur_v);
}


/**
 *	Save rigidity and alpha tables to a file.
 */
void Water::saveTransparencyTable( )
{
	std::string data1;
	uint maxSize = data1.max_size();

	// Build the required data...
	compressVector<uint32>(data1, wAlpha_);
	std::string data2;
	compressVector<bool>(data2, wRigid_);

	BinaryPtr binaryBlockAlpha = new BinaryBlock( data1.data(), data1.size(), "BinaryBlock/Water" );
	BinaryPtr binaryBlockRigid = new BinaryBlock( data2.data(), data2.size(), "BinaryBlock/Water" );

	//Now copy it into the data file
	std::string dataName = config_.transparencyTable_;
	DataSectionPtr pSection = BWResource::openSection( dataName, false );
	if ( !pSection )
	{
		// create a section
		size_t lastSep = dataName.find_last_of('/');
		std::string parentName = dataName.substr(0, lastSep);
		DataSectionPtr parentSection = BWResource::openSection( parentName );
		MF_ASSERT(parentSection);

		std::string tagName = dataName.substr(lastSep + 1);

		// make it
		pSection = new BinSection( tagName, new BinaryBlock( NULL, 0, "BinaryBlock/Water" ) );
		pSection->setParent( parentSection );
		pSection = DataSectionCensus::add( dataName, pSection );
	}


	MF_ASSERT( pSection );
	pSection->delChild( "alpha" );
	DataSectionPtr alphaSection = pSection->openSection( "alpha", true );
	alphaSection->setParent(pSection);
	if ( !pSection->writeBinary( "alpha", binaryBlockAlpha ) )
	{
		CRITICAL_MSG( "Water::saveTransparencyTable - error while writing BinSection in %s/alpha\n", dataName.c_str() );
		return;
	}

	pSection->delChild( "rigid" );
	DataSectionPtr rigidSection = pSection->openSection( "rigid", true );
	rigidSection->setParent(pSection);
	if ( !pSection->writeBinary( "rigid", binaryBlockRigid ) )
	{
		CRITICAL_MSG( "Water::saveTransparencyTable - error while writing BinSection in %s/rigid\n", dataName.c_str() );
		return;
	}

	pSection->delChild( "version" );
	DataSectionPtr versionSection = pSection->openSection( "version", true );
	int version = 2;
	BinaryPtr binaryBlockVer = new BinaryBlock( &version, sizeof(int), "BinaryBlock/Water" );
	versionSection->setParent(pSection);
	if ( !pSection->writeBinary( "version", binaryBlockVer ) )
	{
		CRITICAL_MSG( "Water::saveTransparencyTable - error while writing BinSection in %s/version\n", dataName.c_str() );
		return;
	}

	// Now actually save...
	versionSection->save();
	alphaSection->save();
	rigidSection->save();
	pSection->save();

	// Make sure we break any circular references
	alphaSection->setParent( NULL );
	rigidSection->setParent( NULL );
	versionSection->setParent( NULL );
}

#endif //EDITOR_ENABLED


/**
 *	Read a uint32 from a 4 chars in a block
 *	- updates a referenced index.
 */
void readValue( const char* pCompressedData, uint32& index, uint32& value )
{
	char p1 = pCompressedData[index]; index++;	
	char p2 = pCompressedData[index]; index++;
	char p3 = pCompressedData[index]; index++;
	char p4 = pCompressedData[index]; index++;

	value = p1 & 0xff;
	value |= ((p2 << 8  ) & 0xff00);
	value |= ((p3 << 16 ) & 0xff0000);
	value |= ((p4 << 24 ) & 0xff000000);
}

#pragma warning( push )
#pragma warning( disable : 4800 ) //ignore the perf warning..

/**
 *	Read a bool from a char in a block
 *	- updates a referenced index.
 */
void readValue( const char* pCompressedData, uint32& index, bool& value )
{
	value = static_cast<bool>(pCompressedData[index]); index++;	
}

#pragma warning( pop )


/**
 *	Decompress a char block to vector using the RLE above.
 */
template< class C >
void decompressVector( const char* pCompressedData, uint32 length, std::vector<C>& v )
{
	uint32 i = 0;
	while (i < length)
	{
		// identify the RLE key
		if (pCompressedData[i] == char(RLE_KEY_VALUE))
		{
			i++;
			C val=0;
			readValue(pCompressedData, i, val);
			uint c = uint( pCompressedData[i] & 0xff ); 
			i++;

			// unfold the run..
			for (uint j=0; j < c; j++)
			{
				v.push_back(val);
			}
		}
		else
		{
			C val=0;
			readValue(pCompressedData, i, val);
			v.push_back( val );
		}
	}
}

/**
 *	Load the previously saved transparency/rigidity data.
 */
bool Water::loadTransparencyTable( )
{
	BW_GUARD;
	std::string sectionName = config_.transparencyTable_;
	DataSectionPtr pSection =
			BWResource::instance().rootSection()->openSection( sectionName );

	if (!pSection)
		return false;

	BinaryPtr pVersionData = pSection->readBinary( "version" );
	int fileVersion=-1;
	if (pVersionData)
	{
		const int* pVersion = reinterpret_cast<const int*>(pVersionData->data());
		fileVersion = pVersion[0];
	}

	BinaryPtr pAlphaData = pSection->readBinary( "alpha" );
	if (fileVersion < 2)
	{
		const uint32* pAlphaValues = reinterpret_cast<const uint32*>(pAlphaData->data());
		int nAlphaValues = pAlphaData->len() / sizeof(uint32);
		wAlpha_.assign( pAlphaValues, pAlphaValues + nAlphaValues );
	}
	else
	{
		wAlpha_.clear();
		const char* pCompressedValues = reinterpret_cast<const char*>(pAlphaData->data());
		decompressVector<uint32>(pCompressedValues, pAlphaData->len(), wAlpha_);
	}

	BinaryPtr pRigidData = pSection->readBinary( "rigid" );
	if (fileVersion < 2)
	{
		const bool* pRigidValues = reinterpret_cast<const bool*>(pRigidData->data());
		int nRigidValues = pRigidData->len() / sizeof(bool); //not needed.....remove..
		wRigid_.assign( pRigidValues, pRigidValues + nRigidValues );
	}
	else
	{
		wRigid_.clear();
		const char* pCompressedValues = reinterpret_cast<const char*>(pRigidData->data());
		decompressVector<bool>(pCompressedValues, pRigidData->len(), wRigid_);
	}

	return true;
}

#define MAX_DEPTH 100.f

uint32 encodeDepth( float vertexHeight, float terrainHeight )
{
//	float delta = Math::clamp<float>(0.f, vertexHeight - terrainHeight, MAX_DEPTH);
//	delta = delta / MAX_DEPTH;
//
//	return uint32( delta*255.f ) << 24;

	float h = 155.f + 100.f * min( 1.f, max( 0.f, (1.f - ( vertexHeight - terrainHeight ))));
	h = h/255.f;					
	h = Math::clamp<float>( 0.f, (h-0.5f)*2.f, 1.f );
	h = 1.f-h;
	h = powf(h,2.f)*2.f;
	h = min( h, 1.f);
	return uint32( h*255.f ) << 24;
}

/**
 *	Create rigidity and alpha tables.
 */
void Water::buildTransparencyTable( )
{
	BW_GUARD;
	float z = -config_.size_.y * 0.5f;
	uint32 index = 0;
	bool solidEdges =	(config_.size_.x <= (config_.tessellation_*2.f)) ||
						(config_.size_.y <= (config_.tessellation_*2.f));


	float depth = 10.f;

	for (uint32 zIndex = 0; zIndex < gridSizeZ_; zIndex++ )
	{
		if ((zIndex+1) == gridSizeZ_)
			z = config_.size_.y * 0.5f;

		float x = -config_.size_.x * 0.5f;
		for (uint32 xIndex = 0; xIndex < gridSizeX_; xIndex++ )
		{
			if ((xIndex+1) == gridSizeX_)
				x = config_.size_.x * 0.5f;

			Vector3 v = transform_.applyPoint( Vector3( x, 0.1f, z ) );

			//TODO: determine the depth automatically
			//float height = pCollider_ ? pCollider_->ground( v ) : (v.y - 100);
			//if (height > waterDepth_

			if (xIndex == 0 ||
				zIndex == 0 ||
				xIndex == ( gridSizeX_ - 1 ) ||
				zIndex == ( gridSizeZ_ - 1 ))
			{
				// Set all edge cases to be completely transparent and rigid.				
				if (solidEdges)
				{
					wRigid_[ index ] = false;
					wAlpha_[ index ] = ( 255UL ) << 24;
				}
				else
				{
					wRigid_[ index ] = true;				
					wAlpha_[ index ] = 0x00000000;
				}
			}
			else
			{
				// Get the terrain height, and calculate the transparency of this water grid point
				// to be the height above the terrain.
				float height = pCollider_ ? pCollider_->ground( v ) : (v.y - MAX_DEPTH);
				
				if (height == RompCollider::NO_GROUND_COLLISION)
				{
					wRigid_[ index ] = false;
					wAlpha_[ index ] = ( 255UL ) << 24;
				}
				else if ( height > v.y )
				{
					wRigid_[ index ] = true;
					wAlpha_[ index ] = 0x00000000;
				}
				else
				{
					float delta = v.y - height;
					if (delta > depth)
						depth = (v.y-height);

					wRigid_[ index ] = false;
					wAlpha_[ index ] = encodeDepth(v.y, height);
				}
			}

			++index;
			x += config_.tessellation_;
		}
		z += config_.tessellation_;
	}

	config_.depth_ = depth;
}

/**
 *	Generate the required simulation cell divisions.
 */
void Water::createCells()
{
	BW_GUARD;
	if ( enableSim_ )
	{
		float simGridSize =  ( ceilf( config_.simCellSize_ / config_.tessellation_ ) * config_.tessellation_);

		for (float y=0.f;y<config_.size_.y; y+=simGridSize)
		{
			Vector2 actualSize(simGridSize,simGridSize);
			if ( (y+simGridSize) > config_.size_.y )
				actualSize.y = (config_.size_.y - y);

			//TODO: if the extra bit is really small... just enlarge the current cell??? hmmmm
			for (float x=0.f;x<config_.size_.x; x+=simGridSize)			
			{
				WaterCell newCell;
				if ( (x+simGridSize) > config_.size_.x )
					actualSize.x = (config_.size_.x - x);

				newCell.init( this, Vector2(x,y), actualSize );
				cells_.push_back( newCell );
			}
		}
	}
	else
	{
		// If the simulation is disabled, dont divide the surface.. just use one cell..
		Vector2 actualSize(config_.size_.x, config_.size_.y);
		WaterCell newCell;
		newCell.init( this, Vector2(0.f,0.f), actualSize );
		cells_.push_back( newCell );
	}
}


/**
 *	Create the data tables
 */
void Water::doCreateTables( void* self )
{
	BW_GUARD;
	//DiaryEntryPtr de = Diary::instance().add( "water doCreateTables" );

	Water * selfWater = static_cast< Water * >( self );
	if (Water::stillValid(selfWater))
	{
		// if in the editor: create the transparency table.. else load it...
#ifdef EDITOR_ENABLED

		// Build the transparency information
		if (!selfWater->loadTransparencyTable())
		{
			selfWater->buildTransparencyTable();
		}

		selfWater->enableSim_ = true; // Since this can be toggled by the tools dynamically

#else

		if (!selfWater->loadTransparencyTable())
		{
			selfWater->config_.useEdgeAlpha_ = false;
		}

		selfWater->enableSim_ = Waters::simulationEnabled();

#endif

		// Build the water cells
		selfWater->nVertices_ = selfWater->createIndices();
		selfWater->enableSim_ = selfWater->enableSim_ && Moo::rc().supportsTextureFormat( D3DFMT_A16B16G16R16F ) && selfWater->config_.useSimulation_;

		// Create the FX shaders in the background too..
		bool res=false;
		static bool first=true;

		if (Waters::s_simulationEffect_ == NULL)
			selfWater->enableSim_ = false;
		else
			selfWater->simulationEffect_ = Waters::s_simulationEffect_;

		if (Waters::s_effect_ == NULL)
			selfWater->drawSelf_ = false;
		selfWater->createCells();

		if (Water::backgroundLoad())
			deletionLock_->give();
	}

	//de->stop();
}


/**
 *	This is called on the main thread, after the rigidity and alpha tables
 *	have been computed. Starts the second phase of the water setup.
 */
void Water::onCreateTablesComplete( void * self )
{
	BW_GUARD;
	// second step setup
	//DiaryEntryPtr de = Diary::instance().add( "water setup2ndPhase" );

	Water * selfWater = static_cast< Water * >( self );
	if (Water::stillValid(selfWater))
	{
		selfWater->setup2ndPhase();

		if (Water::backgroundLoad())
			deletionLock_->give();

		selfWater->startResLoader();
	}
	//de->stop();
}


/**
 *	Start the resource loader BG task.
 */
void Water::startResLoader( )
{
	BW_GUARD;
    if (Water::backgroundLoad())
    {
	    // load resources in the background
	    BgTaskManager::instance().addBackgroundTask(
			new CStyleBackgroundTask( 
			    &Water::doLoadResources, this,
			    &Water::onLoadResourcesComplete, this ) );
    }
    else
    {
	    Water::doLoadResources( this );
	    Water::onLoadResourcesComplete( this );
    }
}


/**
 *	Second phase of the water setup.
 */
void Water::setup2ndPhase( )
{
	BW_GUARD;
	static DogWatch w3( "Material" );
	w3.start();

	DEBUG_MSG( "Water::Water: using %d vertices out of %d\n",
		nVertices_, gridSizeX_ * gridSizeZ_ );

	float simGridSize =   ceilf( config_.simCellSize_ / config_.tessellation_ ) * config_.tessellation_;


	int cellX = int(ceilf(config_.size_.x / simGridSize));
	int cellY = int(ceilf(config_.size_.y / simGridSize)); //TODO: need to use the new cell size! :D
	uint cellCount = cells_.size();

	for (uint i=0; i<cellCount; i++)
	{
		cells_[i].initSimulation( MAX_SIM_TEXTURE_SIZE, config_.simCellSize_ );

		//TODO: fix this initialisation...
		if (enableSim_)
		{
			int col = (i/cellX)*cellX;
			int negX = i - 1;
			int posX = i + 1;
			int negY = i - cellX;
			int posY = i + cellX;

			cells_[i].initEdge( 1, (negX < col)					?	NULL	: &cells_[negX] );
			cells_[i].initEdge( 0, (negY < 0)					?	NULL	: &cells_[negY] );
			cells_[i].initEdge( 3, (posX >= (col+cellX))		?	NULL	: &cells_[posX] );
			cells_[i].initEdge( 2, (uint(posY) >= cellCount)	?	NULL	: &cells_[posY] );
		}
	}

	// Create the render targets
	// The WaterSceneRenderer are now grouped based on the height of the 
	//	water plane (maybe extend to using the whole plane?)
	float key = config_.position_.y;
	key = floorf(key+0.5f);
	if ( s_renderTargetMap_.find(key) == s_renderTargetMap_.end())
	{
		s_renderTargetMap_[key] = NULL;
	}

	WaterRenderTargetMap::iterator it = s_renderTargetMap_.find(key);

	WaterScene *(&a) = (it->second);
	if (a == NULL)
		a = new WaterScene( config_.position_[1]);

	waterScene_ = (it->second);
	waterScene_->incRef();
	waterScene_->addOwner(this);

	//	// Create the bounding box.
	bb_.setBounds(	
		Vector3(	-config_.size_.x * 0.5f,
					-1.f,
					-config_.size_.y * 0.5f ),
		Vector3(	config_.size_.x * 0.5f,
					1.f,
					config_.size_.y * 0.5f ) );

	// And the volumetric bounding box ... the water is 10m deep
	//TODO: modify this to use the actual depth...... 

	//TODO: determine the depth of the water and use that for the BB
	//go through all the points and find the lowest terrain point.... use that for the BB depth

	bbDeep_ = bb_;
	bbDeep_.setBounds( bbDeep_.minBounds() - Vector3(0.f,2.f,0.f), bbDeep_.maxBounds() );

	bbActual_.setBounds(	
		Vector3(	-config_.size_.x * 0.5f,
					-config_.depth_,
					-config_.size_.y * 0.5f ),
		Vector3(	config_.size_.x * 0.5f,
					0.f,
					config_.size_.y * 0.5f ) );


	w3.stop();
	static DogWatch w4( "Finalisation" );
	w4.start();

	createUnmanagedObjects();

	// Create the managed objects.
	createManagedObjects();

	w4.stop();
}




/**
 *	Loads all resources needed by the water. To avoid stalling 
 *	the main thread, this should be done in a background task.
 */
void Water::doLoadResources( void * self )
{
	BW_GUARD;
	Moo::ScopedResourceLoadContext resLoadCtx( "water" );

	Water * selfWater = static_cast< Water * >( self );
	if (Water::stillValid(selfWater))
	{
		selfWater->normalTexture_ = Moo::TextureManager::instance()->get(selfWater->config_.waveTexture_);
		selfWater->screenFadeTexture_ = Moo::TextureManager::instance()->get(s_screenFadeMap);
		selfWater->foamTexture_ = Moo::TextureManager::instance()->get(selfWater->config_.foamTexture_);
		selfWater->reflectionTexture_ = Moo::TextureManager::instance()->get(selfWater->config_.reflectionTexture_);
		SimulationManager::instance().loadResources();

		if (Water::backgroundLoad())
			deletionLock_->give();
	}
}


/**
 *	This is called on the main thread, after the resources 
 *	have been loaded. Sets up the texture stages.
 */
void Water::onLoadResourcesComplete( void * self )
{
	BW_GUARD;
	Water * selfWater = static_cast< Water * >( self );
	if (Water::stillValid(selfWater))
	{
		if (selfWater && Waters::s_effect_)
		{
			selfWater->paramCache_->effect(Waters::s_effect_->pEffect()->pEffect());
			if (selfWater->normalTexture_)
				selfWater->paramCache_->setTexture("normalMap", selfWater->normalTexture_->pTexture());
				
			if (selfWater->screenFadeTexture_)
				selfWater->paramCache_->setTexture("screenFadeMap", selfWater->screenFadeTexture_->pTexture());
			if (selfWater->foamTexture_)
				selfWater->paramCache_->setTexture("foamMap", selfWater->foamTexture_->pTexture());
			if (selfWater->reflectionTexture_)
				selfWater->paramCache_->setTexture("reflectionCubeMap", selfWater->reflectionTexture_->pTexture());
		}

		if (selfWater && Waters::s_simulationEffect_)
		{
			ComObjectWrap<ID3DXEffect> pEffect = Waters::s_simulationEffect_->pEffect()->pEffect();

			float borderSize = Waters::s_highQuality_ ? (float)SIM_BORDER_SIZE : 0.0f;
			float pixSize = 1.0f / 256.0f;
			float pix2 = 2.0f * borderSize * pixSize;

			pEffect->SetVector("g_cellInfo", 
								&D3DXVECTOR4( borderSize * pixSize, 1 - (borderSize*pixSize), pix2, 0.0f));
		}

		if (Water::backgroundLoad())
			deletionLock_->give();
	}
}


/**
 *	
 */
void Water::deleteUnmanagedObjects( )
{
	BW_GUARD;
	if (waterScene_)
	{
		waterScene_->deleteUnmanagedObjects();
	}
}


/**
 *	Create all unmanaged resources
 */
void Water::createUnmanagedObjects( )
{
	BW_GUARD;
	if (Waters::s_effect_)
	{
		SimulationCell::createUnmanaged();

		WaterCell::SimulationCellPtrList::iterator cellIt = activeSimulations_.begin();
		for (; cellIt != activeSimulations_.end(); cellIt++)
		{
			//activeList.remove( this );
			(*cellIt)->deactivate();
			(*cellIt)->edgeActivation(false);
		}
		activeSimulations_.clear();
	}
}


/**
 *	Delete managed objects
 */
void Water::deleteManagedObjects( )
{
	BW_GUARD;
	for (uint i=0; i<cells_.size(); i++)
	{
		cells_[i].deleteManaged();
	}
	s_quadVertexBuffer_ = NULL;		
}


/**
 *	Create managed objects
 */
void Water::createManagedObjects( )
{
	BW_GUARD;
	if (Moo::rc().device())
	{
		for (uint i=0; i<cells_.size(); i++)
		{
			cells_[i].createManagedIndices();
		}
		createdCells_ = true;

		//Create a quad for simulation renders
		typedef VertexBufferWrapper< VERTEX_TYPE > VBufferType;
		s_quadVertexBuffer_ = new VBufferType;

		if (s_quadVertexBuffer_->reset( 4 ) && 
			s_quadVertexBuffer_->lock())
		{
			VBufferType::iterator vbIt  = s_quadVertexBuffer_->begin();
			VBufferType::iterator vbEnd = s_quadVertexBuffer_->end();

			// Load the vertex data
			vbIt->pos_.set(-1.f, 1.f, 0.f);
			vbIt->colour_ = 0xffffffff;
			vbIt->uv_.set(0,0);
			++vbIt;

			vbIt->pos_.set(1.f, 1.f, 0.f);
			vbIt->colour_ = 0xffffffff;
			vbIt->uv_.set(1,0);
			++vbIt;

			vbIt->pos_.set(1,-1, 0);
			vbIt->colour_ = 0xffffffff;
			vbIt->uv_.set(1,1);
			++vbIt;

			vbIt->pos_.set(-1,-1,0);
			vbIt->colour_ = 0xffffffff;
			vbIt->uv_.set(0,1);
			++vbIt;

			s_quadVertexBuffer_->unlock();
		}
		else
		{
			ERROR_MSG(
				"Water::createManagedObjects: "
				"Could not create/lock vertex buffer\n");
		}

		MF_ASSERT( s_quadVertexBuffer_.getObject() != 0 );
	}
	initialised_=false;
	vertexBuffers_.clear();
}


/**
 *	Render the simulations for all the active cells in this water surface.
 */
void Water::renderSimulation(float dTime)
{
	BW_GUARD;
	if (!simulationEffect_)
		return;

	static DogWatch simulationWatch( "Simulation" );
	simulationWatch.start();

	static bool first = true;
	if (first || SimulationCell::s_hitTime==-2.0)
	{
		resetSimulation();
		first = false;
	}
	simulationEffect_->hTechnique( "water_simulation" );

	//TODO: move the rain simulation stuff out of the individual water simulation and
	// expose the resulting texture for use elsewhere....
	if (raining_)
	{
		static uint32 rainMark = 0;
		if (rainMark != Waters::s_nextMark_)
		{
			rainMark = Waters::s_nextMark_;
			SimulationManager::instance().simulateRain( dTime, Waters::instance().rainAmount(), simulationEffect_ );
		}
	}

	//TODO:
	// -pertubations should not be passed to a cell if its too far away from the camera
	// -cell with the closest perturbation to the camera should take priority (along with its neighbours)

	//TODO: only check activity when in view...however, keep simulating existing active cells
	// when the water is not visible (will automatically time out)
	//if (visible_)
	//{
		WaterCell::WaterCellVector::iterator cellIt = cells_.begin();
		WaterCell::WaterCellPtrList::iterator wCellIt;
		for (; cellIt != cells_.end(); cellIt++)
		{
			(*cellIt).checkActivity( activeSimulations_, edgeList_ );

			WATER_STAT(s_movementCount += (*cellIt).movements().size());
		}

		// this list is used to activate neighbouring cells
		for (wCellIt = edgeList_.begin(); wCellIt != edgeList_.end(); wCellIt++)
		{
			(*wCellIt)->checkEdgeActivity( activeSimulations_ );
		}
		edgeList_.clear();
	//}

	ComObjectWrap<ID3DXEffect> pEffect = simulationEffect_->pEffect()->pEffect();
	pEffect->SetVector("psSimulationPositionWeighting", 
						&D3DXVECTOR4((config_.consistency_+1.f), config_.consistency_, 0.0f, 0.0f));

	

	if (activeSimulations_.size())
	{
		WaterCell::SimulationCellPtrList::iterator it = activeSimulations_.begin(); 	
		for (; it != activeSimulations_.end(); it++)
		{
			if ((*it))
			{
				//if ((*it)->shouldDeactivate())
				//{
				//	(*it)->deactivate();
				//	(*it)->edgeActivation(false);
				//	it = activeSimulations_.erase(it);				
				//}

				WATER_STAT(s_activeEdgeCellCount = (*it)->edgeActivation() ? s_activeEdgeCellCount + 1 : s_activeEdgeCellCount);
				WATER_STAT(s_activeCellCount++);

				(*it)->simulate( simulationEffect_, dTime, Waters::instance() );
				(*it)->tick();
				(*it)->mark( Waters::s_nextMark_ );
			}
		}

		if (Waters::s_highQuality_ &&
			Waters::instance().simulationLevel() > 1) // only do for high detail settings
		{
			simulationEffect_->hTechnique( "simulation_edging" );
			for (it = activeSimulations_.begin(); it != activeSimulations_.end(); it++)
			{
				if ((*it) && (*it)->mark() == Waters::s_nextMark_)
				{
					(*it)->stitchEdges( simulationEffect_, dTime, Waters::instance() );
				}
			}
		}

	}
	simulationWatch.stop();
}


/**
 *	This method resets all the simulation cells.
 */
void Water::resetSimulation()
{
	BW_GUARD;
	for (uint i=0; i<cells_.size(); i++)
		cells_[i].clear();
}

/**
 * This clear the reflection render target.
 */
void Water::clearRT()
{
	BW_GUARD;
	Moo::RenderTargetPtr rt = waterScene_->reflectionTexture();

	if (rt && rt->push())
	{
		Moo::rc().beginScene();

		Moo::rc().device()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
			RGB( 255, 255, 255 ), 1, 0 );

		Moo::rc().endScene();

		rt->pop();
	}
}


void Waters::createSplashSystem( )
{
	BW_GUARD;
#ifdef SPLASH_ENABLED
	splashManager_.init();
#endif
}

/**
 * This method initialises the mesh data for the water surface.
 */
bool Water::init()
{
	BW_GUARD;
	if (initialised_)
		return true;

	if ( waterScene_ )
	{
		if ( !waterScene_->recreate() )
		{
		//	ERROR_MSG(	"Water::init()"
		//					" couldn't setup render targets" );	

			return false;
		}
	}
	else
		return false;



	initialised_ = true;
	vertexBuffers_.clear();
	vertexBuffers_.resize(remappedVerts_.size() + 1);
	// build multiple vertex buffers based on the remapping data.
	// if nVertices_ is greater than the max allowed, break up the buffer.


	typedef VertexBufferWrapper< VERTEX_TYPE > VBufferType;

	for (uint i=0; i<vertexBuffers_.size(); i++)
	{
		vertexBuffers_[i] = new VBufferType;
	}
	VertexBufferPtr pVerts0 = vertexBuffers_[0];
	uint32 nVert0=0;
	if (remappedVerts_.size())
		nVert0 = 0xffff + 1;
	else
		nVert0 = nVertices_;

	bool success = pVerts0->reset( nVert0 ) && pVerts0->lock();
	for ( uint i=1; i<vertexBuffers_.size(); i++ )
	{
		success = success &&
			vertexBuffers_[i]->reset(remappedVerts_[i-1].size()) &&
			vertexBuffers_[i]->lock();
	}

	if (success)
	{
		VBufferType::iterator vb0It  = pVerts0->begin();
		VBufferType::iterator vb01End = pVerts0->end();

		uint32 index = 0;
		WaterAlphas::iterator waterAlpha = wAlpha_.begin();

		float z = -config_.size_.y * 0.5f;
		float zT = 0.f;

		for (uint32 zIndex = 0; zIndex < gridSizeZ_; zIndex++)
		{
			if ((zIndex+1) == gridSizeZ_)
			{
				z = config_.size_.y * 0.5f;
				zT = config_.size_.y;
			}

			float x = -config_.size_.x * 0.5f;
			float xT = 0.f;
			for (uint32 xIndex = 0; xIndex < gridSizeX_; xIndex++)
			{
				if ((xIndex+1) == gridSizeX_)
				{
					x = config_.size_.x * 0.5f;
					xT = config_.size_.x;
				}

				if (wIndex_[ index ] >= 0)
				{
					//check if it's been re-mapped....
					// and put it in the second buffer it it has
					// if it's less than the current max.. add it here too.
					for ( uint vIdx=0; vIdx<remappedVerts_.size(); vIdx++ )
					{
						std::map<uint32, uint32>& mapping =
												remappedVerts_[vIdx];
						VertexBufferPtr pVertsX = vertexBuffers_[vIdx+1];

						std::map<uint32, uint32>::iterator it = 
									mapping.find( wIndex_[ index ] );
						if ( it != mapping.end() )
						{					
							//copy to buffer
							uint32 idx = it->second;
							VERTEX_TYPE&  pVert = (*pVertsX)[idx];
							
							// Set the position of the water point.
							pVert.pos_.set( x, 0, z );
							if (config_.useEdgeAlpha_)
								pVert.colour_ = *(waterAlpha);
							else
								pVert.colour_ = uint32(0xffffffff);

							pVert.uv_.set( xT / config_.textureTessellation_, zT / config_.textureTessellation_);
						}
					}
					if ( wIndex_[ index ] <= (int32)Moo::rc().maxVertexIndex() )
					{
						//copy to buffer one.
						// Set the position of the water point.
						vb0It->pos_.set( x, 0, z );

						if (config_.useEdgeAlpha_)
							vb0It->colour_ = *(waterAlpha);
						else
							vb0It->colour_ = uint32(0xffffffff);

						vb0It->uv_.set( xT / config_.textureTessellation_, zT / config_.textureTessellation_);
						++vb0It;
					}
				}

				++waterAlpha;
				++index;
				x += config_.tessellation_;
				xT += config_.tessellation_;
			}
			z += config_.tessellation_;
			zT += config_.tessellation_;
		}
		for ( uint i=0; i<vertexBuffers_.size(); i++ )
		{
			vertexBuffers_[i]->unlock();
		}	
	}
	else
	{
		ERROR_MSG(
			"Water::createManagedObjects: "
			"Could not create/lock vertex buffer\n");
		return false;
	}
	MF_ASSERT( vertexBuffers_[0].getObject() != 0 );

	remappedVerts_.clear();

	return true;
}


void Waters::selectTechnique()
{
	BW_GUARD;
#if EDITOR_ENABLED
	if (Waters::projectView())
	{
		Waters::s_effect_->hTechnique( "water_proj" );
//		raining_ = false;
	} else
#endif
// TODO: uncomment+implement these fallbacks
	if (shaderCap_ > 1)
		Waters::s_effect_->hTechnique( "water_rt" );
	else if (shaderCap_==1)
		Waters::s_effect_->hTechnique("water_SM1" );
	else
		Waters::s_effect_->hTechnique("water_SM0" );
}

/**
 * This method sets all the required render states.
 */
void Water::setupRenderState( float dTime )
{
	BW_GUARD_PROFILER( Water_setupRenderState );

	Moo::EffectVisualContext::instance().initConstants();

	// Set our renderstates and material.
	if (!paramCache_->hasEffect())
		paramCache_->effect(Waters::s_effect_->pEffect()->pEffect());

#if EDITOR_ENABLED
	if (Waters::projectView())
		raining_ = false;
#endif

	Waters::instance().selectTechnique();
	
	Matrix wvp;
	wvp.multiply( Moo::rc().world(), 
				  Moo::rc().viewProjection() );
	paramCache_->setMatrix( "worldViewProj", &wvp );
	paramCache_->setMatrix( "world", &Moo::rc().world() );

	static bool firstTime = true;
	static float texScale = 0.5f;
	static float freqX = 1.f;
	static float freqZ = 1.f;
	if (firstTime)
	{
		MF_WATCH( "Client Settings/Water/texScale", texScale, 
			Watcher::WT_READ_WRITE,
			"test scaling" );
		MF_WATCH( "Client Settings/Water/freqX", freqX, 
			Watcher::WT_READ_WRITE,
			"test scaling" );
		MF_WATCH( "Client Settings/Water/freqZ", freqZ, 
			Watcher::WT_READ_WRITE,
			"test scaling" );
		firstTime=false;
	}
	paramCache_->setFloat("texScale",texScale);
	paramCache_->setFloat("freqX",freqX);
	paramCache_->setFloat("freqZ",freqZ);

	paramCache_->setFloat( "maxDepth", config_.depth_ );

	if (waterScene_->eyeUnderWater())
		paramCache_->setFloat( "fadeDepth", (config_.depth_ - 0.001f) );
	else
		paramCache_->setFloat( "fadeDepth", config_.fadeDepth_ );
	paramCache_->setVector( "deepColour", &config_.deepColour_ );

	float w = Moo::rc().screenWidth();
	float h = Moo::rc().screenHeight();
	float pixelX = 1.f / w;
	float pixelY = 1.f / h;
	float offsetX = pixelX*0.25f;
	float offsetY = pixelY*0.25f + 1.f;
	Vector4 screenOffset( offsetX, offsetY, offsetY, offsetX );
	paramCache_->setVector( "screenOffset", &screenOffset );

	paramCache_->setFloat( "foamIntersectionFactor", config_.foamIntersection_ );
	paramCache_->setFloat( "foamMultiplier", config_.foamMultiplier_ );	
	paramCache_->setFloat( "foamTiling", config_.foamTiling_ );
	paramCache_->setBool( "bypassDepth", config_.bypassDepth_ );

	Vector4 x1(config_.waveScale_.x,0,0,0);
	Vector4 y1(0,config_.waveScale_.x,0,0);
	Vector4 x2(config_.waveScale_.y,0,0,0);
	Vector4 y2(0,config_.waveScale_.y,0,0);

	float texAnim = Waters::instance().time() * config_.windVelocity_;

	x1.w = texAnim*config_.scrollSpeed1_.x;
	y1.w = texAnim*config_.scrollSpeed1_.y;

	x2.w = texAnim*config_.scrollSpeed2_.x;
	y2.w = texAnim*config_.scrollSpeed2_.y;

	//TODO: use the parameter cache system.

	paramCache_->setVector( "bumpTexCoordTransformX", &x1 );
	paramCache_->setVector( "bumpTexCoordTransformY", &y1 );
	paramCache_->setVector( "bumpTexCoordTransformX2", &x2 );
	paramCache_->setVector( "bumpTexCoordTransformY2", &y2 );

	//TODO: branch off and setup the surface for simple env. map. reflection if the quality is set to low...

	Vector4 distortionScale( config_.reflectionScale_, config_.reflectionScale_,
							 config_.refractionScale_, config_.refractionScale_ );
	paramCache_->setVector( "scale", &distortionScale );
	paramCache_->setFloat( "fresnelExp", config_.fresnelExponent_ );
	paramCache_->setFloat( "fresnelConstant", config_.fresnelConstant_ );

	paramCache_->setFloat( "fresnelExp", config_.fresnelExponent_ );
	paramCache_->setFloat( "fresnelConstant", config_.fresnelConstant_ );

	paramCache_->setVector( "reflectionTint", &config_.reflectionTint_ );
	paramCache_->setVector( "refractionTint", &config_.refractionTint_ );

	paramCache_->setFloat( "sunPower", config_.sunPower_ );	
	paramCache_->setFloat( "sunScale", Waters::instance().insideVolume() ? 0.f : config_.sunScale_ );
	paramCache_->setFloat( "smoothness",
		Math::lerp(Waters::instance().rainAmount(), config_.smoothness_, 1.f) );

	if (waterScene_ && waterScene_->reflectionTexture())
	{
		paramCache_->setTexture("reflectionMap", waterScene_->reflectionTexture()->pTexture() );
	}
	paramCache_->setBool("highQuality", Waters::s_highQuality_ );	

	if (normalTexture_)
		paramCache_->setTexture("normalMap", normalTexture_->pTexture());

	if (foamTexture_)
		paramCache_->setTexture("foamMap", foamTexture_->pTexture());

	if (reflectionTexture_ && 
		(	config_.useCubeMap_ || 
			(config_.visibility_ == Water::INSIDE_ONLY &&
				WaterSceneRenderer::s_simpleScene_) ) )
	{		
		paramCache_->setFloat( "simpleReflection", 0.0f );
		paramCache_->setFloat( "useCubeMap", true );
		paramCache_->setTexture("reflectionCubeMap", reflectionTexture_->pTexture());
	}
	else
	{
		Vector4 camPos = Moo::rc().invView().row(3);
		const Vector3& waterPos = this->position();

		float camDist = (Vector3(camPos.x,camPos.y,camPos.z) - waterPos).length();
		float dist = camDist - this->size().length()*0.5f;

		dist = Math::clamp( s_sceneCullDistance_-s_sceneCullFadeDistance_, dist, s_sceneCullDistance_ );
		simpleReflection_ = Math::lerp( dist, s_sceneCullDistance_-s_sceneCullFadeDistance_, 
			s_sceneCullDistance_, 0.0f, 1.0f );

		paramCache_->setFloat( "simpleReflection", simpleReflection_ );
		paramCache_->setFloat( "useCubeMap", simpleReflection_ != 0.0f );

		EnviroMinder & enviro = ChunkManager::instance().cameraSpace()->enviro();
		paramCache_->setTexture("reflectionCubeMap", 
			enviro.environmentCubeMap()->cubeRenderTarget()->pTexture());
	}


	Moo::rc().setFVF( VERTEX_TYPE::fvf() );
	vertexBuffers_[0]->activate();


}


/**
 * Update the waters visibility flag
 *
 * TODO: base activity on water visibility... (perhaps just restrict adding new movements?)
 */
void Water::updateVisibility()
{	
	BW_GUARD;
	Matrix m (Moo::rc().viewProjection());
	m.preMultiply( transform_ );

	bb_.calculateOutcode( m );

	//Test visibility
	visible_ = (!bb_.combinedOutcode());

	WATER_STAT(s_waterVisCount++);
}


bool Water::canReflect( float* retDist ) const
{
	BW_GUARD;
	if (this->shouldDraw() && 
		this->drawMark() == (Waters::nextDrawMark()) &&
		!this->config_.useCubeMap_&&
		!(config_.visibility_ == Water::INSIDE_ONLY && WaterSceneRenderer::s_simpleScene_) &&
		Waters::s_drawReflection_)
	{
		Vector4 camPos = Moo::rc().invView().row(3);
		const Vector3& waterPos = this->position();

		float camDist = (Vector3(camPos.x,camPos.y,camPos.z) - waterPos).length();
		float dist = camDist - this->size().length()*0.5f;
		if (retDist)
		{
			*retDist = dist;
		}

		// If we are not using high quality water,
		// we disable the reflection at the fade out distance
		float cullDistance = s_sceneCullDistance_;
		if (!Waters::s_highQuality_)
		{
			cullDistance -= s_sceneCullFadeDistance_;
		}

		if (dist <= cullDistance)
			return true;
	}

	return false;
}

/**
 * Update water simulation cells for this water surface.
 * @param dTime delta time.
 */
void Water::updateSimulation( float dTime )
{
	BW_GUARD;
	if (!cells_.size() || !createdCells_ || (nVertices_==0)) return;

	static DogWatch waterCalc( "Calculations" );
	waterCalc.start();

	// Update the water at 30 fps.
	lastDTime_ = max( 0.001f, dTime );

	static bool changed = false;
	raining_ = false;

	//TODO: clean up the raining flags...

	if (enableSim_)
	{
		raining_ = (Waters::instance().rainAmount() || SimulationManager::instance().rainActive());
		time_ += dTime * 30.f;
		while (time_ >= 1 )
		{
			time_ -= floorf( time_ );

			renderSimulation(1.f/30.f);
		}
		changed = true;
	}
	else if (changed)
	{
		SimulationManager::instance().resetSimulations();
		changed = false;
	}

	waterCalc.stop();
}


/**
 * This method draws the water.
 * @param group the group of waters to draw.
 * @param dTime the time in seconds since the water was last drawn.
 */
void Water::draw( Waters & group, float dTime )
{
	BW_GUARD;

#ifdef EDITOR_ENABLED

	if (Waters::drawReflection())
	{
		reflectionCleared_ = false;
	}
	else
	{
		if (!reflectionCleared_)
		{
			clearRT();
			reflectionCleared_ = true;
		}
	}

#endif // EDITOR_ENABLED

	if (!cells_.size() || !createdCells_ || (nVertices_==0))
		return;	

	static DogWatch waterWatch( "Water" );
	static DogWatch waterDraw( "Draw" );

	waterWatch.start();

#ifdef EDITOR_ENABLED
	DWORD colourise = 0;
	if (drawRed())
	{
		// Set the fog to a constant red colour
		colourise = 0x00AA0000;
	}
	if ( colourise != 0 )
	{
		// colourise using fog
		float fnear = -10000.f;
		float ffar = 10000.f;

		Moo::rc().fogColour( colourise );
		Moo::rc().fogEnabled( true );

		Moo::rc().fogNear( fnear );
		Moo::rc().fogFar( ffar );
	}
#endif //EDITOR_ENABLED
	//{
	//	if (Waters::instance().insideVolume())
	//	{
	//		FogController::instance().setOverride(false);
	//		FogController::instance().commitFogToDevice();
	//	}
	//}

	// Set up the transforms.
	Moo::rc().push();
	Moo::rc().world( transform_ );

	bool inited = init();

	//Test visibility
	if (visible_ && inited)
	{
		waterDraw.start();

		setupRenderState(dTime);
		uint32 currentVBuffer = 0;
		Moo::rc().pushRenderState( D3DRS_FILLMODE );
		Moo::rc().setRenderState( D3DRS_FILLMODE,
			group.drawWireframe_ ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

		if (!paramCache_->hasEffect())
			paramCache_->effect(Waters::s_effect_->pEffect()->pEffect());

		bool bound=false;
		for (uint i=0; i<cells_.size(); i++)
		{
#ifdef DEBUG_WATER			
			if (s_debugCell_>=0 && (i != s_debugCell_ && i != s_debugCell2_))
				continue;
#endif
			//TODO: re-evaluate this culling....improve..
			//		should really be combining multiple cells into a single draw call..
			//TODO: better culling under terrain (linked to the vertex mesh generation)
#ifdef EDITOR_ENABLED
			if (s_cullCells_ && !Waters::projectView())
#else
			if (s_cullCells_)
#endif
			{
				if ( cells_[i].cellDistance() > s_cullDistance_*Moo::rc().camera().farPlane())
					continue;
			}
			if (currentVBuffer != cells_[i].bufferIndex())
			{
				currentVBuffer = cells_[i].bufferIndex();
				vertexBuffers_[ currentVBuffer ]->activate();
			}
			bool usingSim = cells_[i].simulationActive();
			if (raining_)
			{
				if (!bound) //first time
				{
					paramCache_->setBool("useSimulation", true);
					paramCache_->setFloat("simulationTiling", 10.f);
				}
				if (usingSim)
				{
					// Mix in with the regular sim.
					paramCache_->setBool("combineSimulation", true);
					paramCache_->setTexture("simulationMap", cells_[i].simTexture()->pTexture());					
					paramCache_->setTexture("rainSimulationMap", SimulationManager::instance().rainTexture()->pTexture());
				}
				else
				{
					usingSim=true;
					paramCache_->setBool("combineSimulation", false);
					paramCache_->setTexture("simulationMap", SimulationManager::instance().rainTexture()->pTexture());
					paramCache_->setTexture("rainSimulationMap", SimulationManager::instance().rainTexture()->pTexture());
				}
			}
			else if (usingSim)
			{
				if (!bound) //first time
				{
					paramCache_->setBool("combineSimulation", false);
					paramCache_->setFloat("simulationTiling", 1.f);
				}
				paramCache_->setBool("useSimulation", true);
				paramCache_->setTexture("simulationMap", cells_[i].simTexture()->pTexture());
			}
			else
				paramCache_->setBool("useSimulation", false);


			if (usingSim)
			{
				//TODO: clean this up..... move things into the cell class..
				float miny = cells_[i].min().y;
				float maxy = cells_[i].max().y;
				float minx = cells_[i].min().x;
				float maxx = cells_[i].max().x;

				float simGridSize =   ceilf( config_.simCellSize_ / config_.tessellation_ ) * config_.tessellation_;

				float s = (config_.textureTessellation_) / (simGridSize);

				float cx = -(minx + (simGridSize)*0.5f);
				float cy = -(miny + (simGridSize)*0.5f);

				Vector4 x1( s, 0.f, 0.f, cx / config_.textureTessellation_ );
				Vector4 y1( 0.f, s, 0.f, cy / config_.textureTessellation_ );

				//x1.set( s, 0.f, 0.f, cx / config_.textureTessellation_ );
				//y1.set( 0.f, s, 0.f, cy / config_.textureTessellation_ );
				paramCache_->setVector( "simulationTransformX", &x1 );
				paramCache_->setVector( "simulationTransformY", &y1 );
			}

			if (cells_[i].bind())
			{
				if (bound)
					Waters::s_effect_->commitChanges();
				else
				{				
					bound=true;
					Waters::s_effect_->begin();
				}

				for (uint32 pass=0; pass<Waters::s_effect_->nPasses();pass++)
				{
					Waters::s_effect_->beginPass(pass);
					if ( s_enableDrawPrim )
						cells_[i].draw( vertexBuffers_[ currentVBuffer ]->size() );
					Waters::s_effect_->endPass();
				}
			}
		}

		if (bound)
			Waters::s_effect_->end();

		// Reset some states to their defaults.
		Moo::rc().popRenderState();
		Moo::rc().setTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );

		vertexBuffers_[ 0 ]->deactivate();

		waterDraw.stop();

#ifdef EDITOR_ENABLED
		// draw some lines for better selection feedback in the tool
		if (highlight())
		{
			float xoff = config_.size_.x * 0.5f;
			float yoff = config_.size_.y * 0.5f;

			for (uint i = 0; i < cells_.size(); i++)
			{
				Vector3 cX0Y0(
				   cells_[i].min().x - xoff, -0.5f, cells_[i].min().y - yoff );
				Vector3 cX1Y1(
				   cells_[i].max().x - xoff,  0.5f, cells_[i].max().y - yoff );

				BoundingBox bb( cX0Y0, cX1Y1 );

				Geometrics::wireBox( bb, 0x00C0FFC0 );
			}
		}
#endif // EDITOR_ENABLED

	}

#ifdef DEBUG_WATER
	//draw debug lines for cell connections
	if (s_debugSim_)
	{
		for (uint i=0; i<cells_.size(); i++)
		{
			if (!cells_[i].isActive())
				continue;

			float xoff = config_.size_.x*0.5f;
			float yoff = config_.size_.y*0.5f;

			Vector3 cX0Y0(cells_[i].min().x - xoff, 1.f, cells_[i].min().y - yoff);
			Vector3 cX0Y1(cells_[i].min().x - xoff, 1.f, cells_[i].max().y - yoff);
			Vector3 cX1Y0(cells_[i].max().x - xoff, 1.f, cells_[i].min().y - yoff);
			Vector3 cX1Y1(cells_[i].max().x - xoff, 1.f, cells_[i].max().y - yoff);

			cX0Y0 = transform_.applyPoint(cX0Y0);
			cX0Y1 = transform_.applyPoint(cX0Y1);
			cX1Y0 = transform_.applyPoint(cX1Y0);
			cX1Y1 = transform_.applyPoint(cX1Y1);

			Moo::PackedColour col = 0xffffffff;

			if (cells_[i].edgeActivation())
				col = 0xff0000ff;


			LineHelper::instance().drawLine( cX0Y0, cX0Y1, col );
			LineHelper::instance().drawLine( cX0Y0, cX1Y0, col );
			LineHelper::instance().drawLine( cX1Y0, cX1Y1, col );
			LineHelper::instance().drawLine( cX0Y1, cX1Y1, col );
		}
	}
#endif //DEBUG_WATER
	waterWatch.stop();
	Moo::rc().pop();

#ifdef EDITOR_ENABLED
	if( colourise != 0 )
	{
		FogController::instance().commitFogToDevice();
	}
	// Highlighting is done once. The flag will be set for the next frame if
	// it's needed.
	highlight( false );
#endif //EDITOR_ENABLED
	//{
	//	if (Waters::instance().insideVolume())
	//	{
	//		FogController::instance().setOverride(true);
	//		FogController::instance().commitFogToDevice();
	//	}
	//}
}

bool Water::needSceneUpdate() const 
{ 
	BW_GUARD;
	return needSceneUpdate_ && drawMark_ == Waters::nextDrawMark();
}

void Water::needSceneUpdate( bool value ) 
{ 
	needSceneUpdate_ = value; 
}


/**
 *  Is water loading done in the background?
 */
/*static*/ bool Water::backgroundLoad()
{
    return s_backgroundLoad_;
}

/**
 *  Set whether loading is done in the background.  You should do this at the
 *  start of the app; doing it while the app is running will lead to problems.
 */
/*static*/ void Water::backgroundLoad(bool background)
{
    s_backgroundLoad_ = background;
}


/**
 *	Adds a movement to this water body, if passes through it
 */
bool Water::addMovement( const Vector3& from, const Vector3& to, const float diameter )
{
	BW_GUARD;

	if (!initialised_)
		return false;

	//TODO: wheres the best place to put this movements reset?
	WaterCell::WaterCellVector::iterator cellIt = cells_.begin();
	for (; cellIt != cells_.end(); cellIt++)
	{
		(*cellIt).updateMovements();
	}

	if (!visible())
		return false;

	Vector4 worldCamPos = Moo::rc().invView().row(3);
	Vector3 worldCamPos3(worldCamPos.x, worldCamPos.y, worldCamPos.z);
	float dist = (to - worldCamPos3).length();
	if (dist > Water::s_maxSimDistance_)
		return false;

	// Only add it if it's in our bounding box
	Vector3 cto = invTransform_.applyPoint( to );
	//check distance from water surface.
	if (cto.y > 0.0f || fabs(cto.y) > 10.0f)
		return false;
	Vector3 cfrom = invTransform_.applyPoint( from );
	
	float displacement = (to - from).length();
	bool stationary = displacement < 0.001f;
#ifdef SPLASH_ENABLED
	bool forceSplash = false;
	if (stationary)
	{
		if ( Waters::instance().checkImpactTimer() )
		{
			stationary = false;
			displacement = 0.15f;
			forceSplash = true;
		}		
	}
#endif //SPLASH_ENABLED

	bool inVolume = bbActual_.clip( cfrom, cto );
	if (!stationary && inVolume)	
	{
#ifdef SPLASH_ENABLED
		// a copy of the movements for the splashes
		Vector4 impact(to.x,to.y,to.z,0);
		impact.w = displacement;
		bool shouldSplash = true;
		if (config_.useEdgeAlpha_)
		{
			// Get the position in the water's local space
			Vector3 localpos = invTransform_.applyPoint( 
									Vector3(impact.x, impact.y, impact.z) );

			//TODO: need to change the water wave animation to be easily 
			// reproducable on the CPU.. this is not the case at the moment 
			// so currently I'm turning off the splash particles based
			// on a approximation of the edge .
			
			 // move to [0, config_.size_.x] range
			localpos.x += config_.size_.x * 0.5f;
			 // move to [0, config_.size_.y] range
			localpos.z += config_.size_.y * 0.5f;

			// Check the closest point first.
			int xIdx = min( (int)((localpos.x / config_.tessellation_) + 0.5f), (int)gridSizeX_ );
			int zIdx = min( (int)((localpos.z / config_.tessellation_) + 0.5f), (int)gridSizeZ_ );
			int finalIdx = xIdx +  gridSizeX_*zIdx;			
			if ( finalIdx < (int)wAlpha_.size() &&
				((uint32(wAlpha_[finalIdx]) >> 24) < 255 ) )
			{
				shouldSplash = false;
			}
			else
			{
				// Retrieve the indices of the 4 points on the water around the splash location.
				int xIdxLow = (int)floorf(localpos.x / config_.tessellation_);
				int zIdxLow = (int)floorf(localpos.z / config_.tessellation_);
				int xIdxHigh = min( (int)ceilf(localpos.x / config_.tessellation_), (int)gridSizeX_ );
				int zIdxHigh = min( (int)ceilf(localpos.z / config_.tessellation_), (int)gridSizeZ_ );

				int idx1 = xIdxLow	+  gridSizeX_*zIdxLow;
				int idx2 = xIdxLow	+  gridSizeX_*zIdxHigh;
				int idx3 = xIdxHigh +  gridSizeX_*zIdxLow;
				int idx4 = xIdxHigh +  gridSizeX_*zIdxHigh;
				if (idx4 < (int)wAlpha_.size())
				{
					// Averaging out the 4 points around the splash..				
					// TODO: replace with a more accurate determination of depth/location
					uint32 avg = uint32(wAlpha_[idx1]) >> 24;
					avg += uint32(wAlpha_[idx2]) >> 24;
					avg += uint32(wAlpha_[idx3]) >> 24;
					avg += uint32(wAlpha_[idx4]) >> 24;
					avg /= 4;
					//190 seems to produce the best cut off point
					if (avg < 190)
					{
						shouldSplash = false;
					}
				}
			}
		}
		if (shouldSplash)
		{
			Waters::instance().addSplash(impact, transform_._42, forceSplash);
		}

		if (forceSplash)
			return true; //avoid making a simulated pulse..
#endif //SPLASH_ENABLED
	
		//add the movement to the right cell... (using "to" for now)
		Vector3 halfsize( config_.size_.x*0.5f, 0, config_.size_.y*0.5f );
		cto += halfsize;
		cfrom += halfsize;

		float simGridSize = ceilf( config_.simCellSize_ / config_.tessellation_ ) * config_.tessellation_;
		float invSimSize=1.f/simGridSize;

		int xcell = int( cto.x * invSimSize);
		int ycell = int( cto.z * invSimSize);
		uint cell = xcell + int((ceilf(config_.size_.x * invSimSize)))*ycell;
		if (cell < cells_.size())
		{	

//			float invSimSizeX=1.f/cells_[cell].size().x;
//			float invSimSizeY=1.f/cells_[cell].size().y;


			//Calculate the position inside this cell
			cto.x = (cto.x*invSimSize - xcell);
			cto.z = (cto.z*invSimSize - ycell);
			cfrom.x = (cfrom.x*invSimSize - xcell);
			cfrom.z = (cfrom.z*invSimSize - ycell);

			cto.y = displacement;

			cells_[cell].addMovement( cfrom, cto, diameter );
		}
		return true;
	}
	return inVolume;
}


#if UMBRA_ENABLE
/**
 *	Add a terrain chunk item to the list for umbra culling purposes.
 */
void Water::addTerrainItem( ChunkTerrain* item )
{
	BW_GUARD_PROFILER( Water_addTerrainItem );
	TerrainVector::iterator found = std::find(	terrainItems_.begin(),
												terrainItems_.end(),
												item );
	if (found == terrainItems_.end())
		terrainItems_.push_back(item);
}


/**
 *	Remove a terrain chunk item from the occluder list.
 */
void Water::eraseTerrainItem( ChunkTerrain* item )
{
	BW_GUARD_PROFILER( Water_eraseTerrainItem );
	TerrainVector::iterator found = std::find(	terrainItems_.begin(),
												terrainItems_.end(),
												item );
	if (found != terrainItems_.end())
		terrainItems_.erase(found);
}

/**
 *	Disable all the terrain UMBRA occluders.
 */
void Water::disableOccluders() const
{
	BW_GUARD;
	TerrainVector::const_iterator it = terrainItems_.begin();
	for (; it != terrainItems_.end(); it++)
	{
		ChunkTerrainPtr terrain = (*it);
		terrain->disableOccluder();
	}	
}


/**
 *	Enable all the terrain UMBRA occluders.
 */
void Water::enableOccluders() const
{
	BW_GUARD;
	TerrainVector::const_iterator it = terrainItems_.begin();
	for (; it != terrainItems_.end(); it++)
	{
		ChunkTerrainPtr terrain = (*it);
		terrain->enableOccluder();
	}	
}

#endif //UMBRA_ENABLE

// -----------------------------------------------------------------------------
// Section: Waters
// -----------------------------------------------------------------------------

/**
 *	A callback method for the graphics setting quality option
 */
void Waters::setQualityOption(int optionIndex)
{
	BW_GUARD;
	s_highQuality_=false;
	shaderCap_ = Moo::EffectManager::instance().PSVersionCap();
	if ( shaderCap_ >= 1 && optionIndex < 3)
	{
		if (optionIndex == 0 && Moo::MRTSupport::instance().isEnabled())
		{
			//High quality settings..
			s_highQuality_=true;
			WaterSceneRenderer::s_textureSize_ = 1.f;
			WaterSceneRenderer::s_drawTrees_ = true;
			WaterSceneRenderer::s_drawDynamics_ = true;
			WaterSceneRenderer::s_drawPlayer_ = true;
			WaterSceneRenderer::s_simpleScene_ = false;
		}
		else if (optionIndex == 1)
		{
			//Mid quality settings..
			WaterSceneRenderer::s_textureSize_ = 1.f;
			WaterSceneRenderer::s_drawTrees_ = true;
			WaterSceneRenderer::s_drawDynamics_ = false;
			WaterSceneRenderer::s_drawPlayer_ = true;
			WaterSceneRenderer::s_simpleScene_ = false;
		}
		else if (optionIndex == 2)
		{
			//TODO: LOD trees....

			//Low quality settings..
			WaterSceneRenderer::s_textureSize_ = 0.5f;
			WaterSceneRenderer::s_drawTrees_ = true;
			WaterSceneRenderer::s_drawDynamics_ = false;
			WaterSceneRenderer::s_drawPlayer_ = true;
			WaterSceneRenderer::s_simpleScene_ = true;
		}
	}
	else
	{
		//Lowest quality settings..
		WaterSceneRenderer::s_textureSize_ = 0.25f;
		WaterSceneRenderer::s_drawTrees_ = false;
		WaterSceneRenderer::s_drawDynamics_ = false;
		WaterSceneRenderer::s_drawPlayer_ = false;
		WaterSceneRenderer::s_simpleScene_ = true;

		s_simulationLevel_=0;
	}
}


/**
 *	A callback method for the graphics setting simulation quality option
 */
void Waters::setSimulationOption(int optionIndex)
{
	BW_GUARD;
	if (Moo::EffectManager::instance().PSVersionCap() >= 3)
	{
		if (optionIndex==0)
		{
			// we need to restart if we went from disabled
			// to enabled
			if ( s_simulationLevel_ == 0 )
			{
				simSettings_->needsRestart( true );
			}
			s_simulationLevel_ = 2;
			//TODO: tweak the max sim textures...
			//SimulationManager::instance().setMaxTextures(4);
		}
		else if (optionIndex==1)
		{
			// we need to restart if we went from disabled
			// to enabled
			if ( s_simulationLevel_ == 0 )
			{
				simSettings_->needsRestart( true );
			}
			s_simulationLevel_ = 1;
			//SimulationManager::instance().setMaxTextures(3);
		}
		else // if (optionIndex==2)
			s_simulationLevel_ = 0;
	}
	else
		s_simulationLevel_ = 0;

	//SimulationManager::instance().resetSimulations();
	//SimulationManager::instance().recreateBlocks();	
}


/*virtual*/ void Waters::onSelectPSVersionCap(int psVerCap)
{
	BW_GUARD;
	setQualityOption( qualitySettings_->activeOption() );
	setSimulationOption( simSettings_->activeOption() );
}


/**
 *	Retrieves the Waters object instance
 */
inline Waters& Waters::instance()
{
	BW_GUARD;
	static Waters inst;
	return inst;
}


/**
 *	Cleanup
 */
void Waters::fini( )
{
	BW_GUARD;
	s_simulationEffect_ = NULL;
	s_effect_ = NULL;

	listeners_.clear();

	SimulationManager::fini();
#ifdef SPLASH_ENABLED
	splashManager_.fini();
#endif

	Moo::EffectManager::instance().delListener(this);

	Water::s_quadVertexBuffer_ = NULL;
}


/**
 *	Load the required resources
 */
void Waters::loadResources( void * self )
{
	BW_GUARD;
	Waters * selfWaters = static_cast< Waters * >( self );

	bool res=false;
	s_simulationEffect_ = new Moo::EffectMaterial;
	res = s_simulationEffect_->initFromEffect( s_simulationEffect );
	if (!res)
		s_simulationEffect_ = NULL;

	s_effect_ = new Moo::EffectMaterial;
	res = s_effect_->initFromEffect( s_waterEffect );
	if (!res)
	{
		CRITICAL_MSG( "Water::doCreateTables()"
		" couldn't load effect file "
		"%s\n", s_waterEffect.value().c_str() );
	}
}


/**
 *	Called upon finishing the resource loading
 */
void Waters::loadResourcesComplete( void * self )
{
}


/**
 *	Initialise the water settings
 */
void Waters::init( )
{
	BW_GUARD;
	SimulationManager::init();

	//
	// Register graphics settings
	//		

	// water quality settings
	bool supported = Moo::rc().vsVersion() >= 0x200 && Moo::rc().psVersion() >= 0x200;
	bool supportedHQ = Moo::rc().vsVersion() >= 0x300 && Moo::rc().psVersion() >= 0x300;

	qualitySettings_ = 
		Moo::makeCallbackGraphicsSetting(
		"WATER_QUALITY", "Water Quality", *this, 
			&Waters::setQualityOption, 
			supported ? (supportedHQ ? 0 : 1) : 2, false, false);


	qualitySettings_->addOption("HIGH",  "High", supportedHQ);
	qualitySettings_->addOption("MEDIUM", "Medium", supported);
	qualitySettings_->addOption("LOW", "Low", true);
	qualitySettings_->addOption("LOWEST", "Lowest", true);
	Moo::GraphicsSetting::add(qualitySettings_);
	setQualityOption(qualitySettings_->activeOption());

	// simulation toggle
	typedef Moo::GraphicsSetting::GraphicsSettingPtr GraphicsSettingPtr;
	simSettings_ = 
		Moo::makeCallbackGraphicsSetting(
			"WATER_SIMULATION", "Water Simulation", *this, 
			&Waters::setSimulationOption, 
			supported ? 0 : 2, false, false);

	bool simulationSupported = Moo::rc().psVersion() >= 0x300 && (Moo::rc().supportsTextureFormat( D3DFMT_A16B16G16R16F ));

	simSettings_->addOption("HIGH",  "High", simulationSupported);
	simSettings_->addOption("LOW",  "Low", simulationSupported);
	simSettings_->addOption("OFF", "Off", true);
	Moo::GraphicsSetting::add(simSettings_);
	setSimulationOption(simSettings_->activeOption());
	// number of reflections settings ??????

    if (Water::backgroundLoad())
    {
	    // do heavy setup stuff in the background
	    BgTaskManager::instance().addBackgroundTask(
			new CStyleBackgroundTask( 
			    &Waters::loadResources, this,
			    &Waters::loadResourcesComplete, this ) );
    }
    else
    {
	    loadResources(this);
	    loadResourcesComplete(this);
    }

	createSplashSystem();

	Moo::EffectManager::instance().addListener(this);
}


/**
 *	Waters constructor
 */
Waters::Waters() :
	drawWireframe_( false ),
	movementImpact_( 20.0f ),
	rainAmount_( 0.f ),
	impactTimer_( 0.f ),
	time_( 0.f )
{
	BW_GUARD;
	static bool firstTime = true;
	if (firstTime)
	{
		MF_WATCH( "Client Settings/Water/draw", s_drawWaters_, 
					Watcher::WT_READ_WRITE,
					"Draw water?" );
		MF_WATCH( "Client Settings/Water/wireframe", drawWireframe_,
					Watcher::WT_READ_WRITE,
					"Draw water in wire frame mode?" );
		MF_WATCH( "Client Settings/Water/character impact", movementImpact_,
					Watcher::WT_READ_WRITE,
					"Character simulation-impact scale" );
#ifdef SPLASH_ENABLED
		MF_WATCH( "Client Settings/Water/Splash/auto interval", s_autoImpactInterval_,
					Watcher::WT_READ_WRITE,
					"Interval for automatically generated splashes when stationary" );
#endif //#ifdef SPLASH_ENABLED

#ifdef DEBUG_WATER
		MF_WATCH( "Client Settings/Water/Stats/water count", s_waterCount,
			Watcher::WT_READ_ONLY, "Water Stats");
		MF_WATCH( "Client Settings/Water/Stats/visible count", s_waterVisCount,
			Watcher::WT_READ_ONLY, "Water Stats");
		MF_WATCH( "Client Settings/Water/Stats/cell count", s_activeCellCount,
			Watcher::WT_READ_ONLY, "Water Stats");
		MF_WATCH( "Client Settings/Water/Stats/edge cell count", s_activeEdgeCellCount,
			Watcher::WT_READ_ONLY, "Water Stats");
		MF_WATCH( "Client Settings/Water/Stats/impact movement count", s_movementCount,
			Watcher::WT_READ_ONLY, "Water Stats");
#endif //DEBUG_WATER

		firstTime = false;
	}
}


/**
 *	Waters destructor
 */
Waters::~Waters()
{
	BW_GUARD;
	s_simulationEffect_ = NULL;
	s_effect_ = NULL;
}


/**
 * Adds a movement to all the waters.
 *
 * @param from	World space position, defining the start of the line moving through the water.
 * @param to	World space position, defining the end of the line moving through the water.
 * @param diameter	The diameter of the object moving through the water.
 */
void Waters::addMovement( const Vector3& from, const Vector3& to, const float diameter )
{
	BW_GUARD;
	Waters::iterator it = this->begin();
	Waters::iterator end = this->end();
	while (it!=end)
	{
		(*it++)->addMovement( from, to, diameter );
	}
}


//ever-increasing id used to identify volume listeners.
uint32 Waters::VolumeListener::s_id = 0;


/**
 * Checks whether the position provided by a matrix provider is inside
 * this water volume or not.
 * @param MatrixProvider, providing the world space position.
 * @return True if the position represented by the matrix provider is inside.
 */
bool Water::isInside( MatrixProviderPtr pMat )
{
	BW_GUARD;
	Matrix m;
	pMat->matrix( m );
	Vector3 v = invTransform_.applyPoint( m.applyToOrigin() );
	return bbActual_.intersects(v);
}


/**
 * Checks whether the position provided by a matrix provider is inside the
 * boundary of the water or not.
 * @param Matrix, providing the world space position.
 * @return True if the position represented by the matrix provider is inside 
 * the boundary.
 */
bool Water::isInsideBoundary( Matrix m )
{
	BW_GUARD;
	Vector3 v = invTransform_.applyPoint( m.applyToOrigin() );

	BoundingBox checkBox( v, v );
	checkBox.addYBounds( FLT_MAX );
	checkBox.addYBounds( -FLT_MAX );

	return bbActual_.intersects( checkBox );
}


/**
 * This method checks all water volume listeners against all current water
 * volumes, and fires off callbacks as necessary.
 */
void Waters::checkAllListeners()
{
	BW_GUARD;
	struct CallbackArguments
	{
	public:
		CallbackArguments( 
			PyObjectPtr	pCallback,
			bool entering,
			PyObjectPtr pWater
			):
			pCallback_( pCallback ),
			entering_( entering ),
			pWater_( pWater )
		{
		}

		PyObjectPtr	pCallback_;
		bool entering_;
		PyObjectPtr pWater_;
	};
	
	std::vector<CallbackArguments> firedCallbacks;

	VolumeListeners::iterator it = listeners_.begin();
	VolumeListeners::iterator end = listeners_.end();
	while (it != end)
	{
		VolumeListener& l= *it;
		
		if (!l.inside())
		{
			//If not in any water at all, check all waters.
			for (uint i=0; i < this->size(); i++)
			{
				Water* water = (*this)[i];
				bool inside = water->isInside(l.source());
				if (inside)
				{
					firedCallbacks.push_back( CallbackArguments(l.callback(),true,water->pPyVolume()) );
					l.water( water );
					break;
				}
			}
		}
		else
		{
			//If already in water, only check if you are getting out of that water.
			bool foundWater = false;
			for (uint i=0; i < this->size(); i++)
			{
				Water* water = (*this)[i];
				if (l.water() == water)
				{
					foundWater = true;
					bool inside = water->isInside(l.source());
					if (!inside)
					{
						firedCallbacks.push_back( CallbackArguments(l.callback(),false,water->pPyVolume()) );
						l.water( NULL  );
					}
					break;					
				}
			}

			//If the water is not in our Waters list, then the water has been
			//deleted.  We notify the callback that it has left the water.			
			if (!foundWater)
			{
				firedCallbacks.push_back( CallbackArguments(l.callback(),false,NULL) );
				l.water(NULL);
			}
		}			
		it++;
	}	

	for (size_t i=0; i<firedCallbacks.size(); i++)
	{
		CallbackArguments& cbData = firedCallbacks[i];
		PyObject * callback = cbData.pCallback_.getObject();
		Py_INCREF( callback );
		PyObject * args = PyTuple_New(2);
		PyTuple_SET_ITEM( args, 0, Script::getData(cbData.entering_) );
		PyTuple_SET_ITEM( args, 1, Script::getData(cbData.pWater_) );
		Script::call( callback, args, "Water listener callback: " ); 	
	}

	firedCallbacks.clear();
}


/*~ function BigWorld addWaterVolumeListener
 *  @components{ client }
 *  This function registers a listener on the water.  A MatrixProvider
 *	provides the source location, and the callback function is called when
 *	the source location enters or leaves a water volume.
 *	The source may only be in one water volume at any one time, if you
 *	overlapping waters, the results will be undefined.
 *	The callback function takes two arguments: a boolean describing whether
 *	the source location has just entered the water or not , and a PyWaterVolume
 *	which is a python wrapper of the water object the source location has just
 *	entered or left from.
 *	@param	dynamicSource	A MatrixProvider providing the listener's location.
 *	@param	callback		A callback fn that takes one boolean argument and one PyWaterVolume argument.
 */
/**
 *	This method adds a water volume listener.
 *	@param	dynamicSource	A MatrixProvider providing the listener's location.
 *	@param	callback		A callback fn that takes one boolean argument and one PyWaterVolume argument.
 *	@return	id				ID used to represent and later delete the listener.
 */
uint32 Waters::addWaterVolumeListener( MatrixProviderPtr dynamicSource, PyObjectPtr callback )
{	
	BW_GUARD;
	Waters::instance().listeners_.push_back( VolumeListener(dynamicSource, callback) );
	return Waters::instance().listeners_.back().id();
}

PY_MODULE_STATIC_METHOD( Waters, addWaterVolumeListener, BigWorld )


/*~ function BigWorld delWaterVolumeListener
 *  @components{ client }
 *  This function removes a listener from the water.  The id
 *	received when registering the listener should be passed in.
 *	@param	id				id formerly returned by addWaterVolumeListener 
 */
/**
 *	This method adds a water volume listener.
 *	@param	id				id formerly returned by addWaterVolumeListener 
 */
void Waters::delWaterVolumeListener( uint32 id )
{
	BW_GUARD;
	VolumeListeners::iterator it = Waters::instance().listeners_.begin();
	VolumeListeners::iterator end = Waters::instance().listeners_.end();
	while (it != end)
	{
		VolumeListener& l= *it;
		if (l.id() == id)		
		{
			Waters::instance().listeners_.erase(it);
			return;
		}
		++it;
	}	
}

PY_MODULE_STATIC_METHOD( Waters, delWaterVolumeListener, BigWorld )


// Static list used to cache up all the water surfaces that need to be drawn
static VectorNoDestructor< Water * > s_waterDrawList;


/**
 * Simulate the water interactions.
 * @param dTime delta time.
 */
void Waters::tick( float dTime )
{
	BW_GUARD;
	WATER_STAT(s_waterVisCount=0);
	WATER_STAT(s_activeCellCount=0);
	WATER_STAT(s_activeEdgeCellCount=0);
	WATER_STAT(s_movementCount=0);

	if (s_drawWaters_)
	{
		time_ += dTime;

		for (uint i=0; i < this->size(); i++)
		{
			(*this)[i]->visible(false);
		}
		for (uint i=0; i < s_waterDrawList.size(); i++)
		{
			s_waterDrawList[i]->updateVisibility();
		}
	}

	this->checkAllListeners();
}


/**
 * Simulate the water interactions.
 * @param dTime delta time.
 */
void Waters::updateSimulations( float dTime )
{	
	BW_GUARD;
	if (s_drawWaters_)
	{
		for( uint32 i = 4; i < Moo::rc().maxSimTextures(); i++ )
		{
			Moo::rc().setTexture( i, NULL );
		}
		if (simulationEnabled())
		{
			for (uint i=0; i < this->size(); i++)
			{
				(*this)[i]->updateSimulation( dTime );
			}
		}
	}
}

uint Waters::drawCount() const
{
	return s_waterDrawList.size();
}


/**
 *	This static method adds a water object to the list of waters to draw
 */
void Waters::addToDrawList( Water * pWater )
{
	BW_GUARD;
	if ( pWater->drawSelf_ && !(s_nextMark_ == pWater->drawMark()) )
	{
		s_waterDrawList.push_back( pWater );
		pWater->drawMark( s_nextMark_ );
		pWater->clearVisibleChunks();
	}
}


/**
 *	This method draws all the stored water objects under this object's
 *	auspices.
 */
void Waters::drawDrawList( float dTime )
{
	BW_GUARD;
	if (s_drawWaters_)
	{
		Moo::LightContainerPtr pRCSLC = Moo::rc().specularLightContainer();
		static Moo::LightContainerPtr pSLC = new Moo::LightContainer;
		if (ChunkManager::instance().cameraSpace())
		{
			pSLC->directionals().clear();
			pSLC->ambientColour( ChunkManager::instance().cameraSpace()->ambientLight() );
			if (ChunkManager::instance().cameraSpace()->sunLight())
				pSLC->addDirectional( ChunkManager::instance().cameraSpace()->sunLight() );		
			
			Moo::rc().specularLightContainer( pSLC );			
		}
		for (uint i = 0; i < s_waterDrawList.size(); i++)
		{
			s_waterDrawList[i]->draw( *this, dTime );
		}
		Moo::rc().specularLightContainer( pRCSLC );
	}
	WATER_STAT(s_waterCount=this->size());

	s_lastDrawMark_ = s_nextMark_;
	s_nextMark_++;
	s_waterDrawList.clear();

	impactTimer_ += dTime;

#ifdef SPLASH_ENABLED
	splashManager_.draw(dTime);
#endif
}


/**
 * Draw the refraction mask for this water surface.
 *
 */
void Water::drawMask()
{
	BW_GUARD;
	Moo::rc().push();
	const Vector2 wSize = size();
	const Vector3 wPos = position();
	float wOrient = orientation();
	float xscale = wSize.x * 0.5f, zscale = wSize.y * 0.5f;

	Matrix trans(Matrix::identity);
	trans.setScale(xscale, 1.f, zscale);
	trans.preRotateX( DEG_TO_RAD(90) );
	trans.postRotateY( wOrient );
	trans.postTranslateBy( wPos );
	Moo::rc().world( trans );
	
	Moo::rc().device()->SetTransform( D3DTS_PROJECTION, &Moo::rc().projection() );
	Moo::rc().device()->SetTransform( D3DTS_VIEW, &Moo::rc().view() );
	Moo::rc().device()->SetTransform( D3DTS_WORLD, &Moo::rc().world() );

	Moo::rc().drawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	Moo::rc().pop();

}


/**
 * Draw the refraction masking for all the water surfaces.
 *
 */
void Waters::drawMasks()
{
	BW_GUARD;
	if (s_drawWaters_ && Water::s_quadVertexBuffer_ &&
		 s_waterDrawList.size())
	{
		Moo::rc().setFVF(VERTEX_TYPE::fvf());
		// Render the vertex buffer
		Water::s_quadVertexBuffer_->activate();

		Moo::Material::setVertexColour();
		
		// Mask needs the alpha to be written out.
		Moo::rc().setTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );
		Moo::rc().setTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

		Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA );
		Moo::rc().setRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		Moo::rc().setRenderState( D3DRS_ZWRITEENABLE, FALSE );

		for (uint i = 0; i < s_waterDrawList.size(); i++)
		{
			s_waterDrawList[i]->drawMask( );
		}
		Water::s_quadVertexBuffer_->deactivate();
	}
}


/**
 * Update variables.
 */
void Water::tick()
{
	BW_GUARD;
	// transform the cam position into the waters local space.
	camPos_ = invTransform_.applyPoint(	Vector3(Moo::rc().invView().row(3).x,
												Moo::rc().invView().row(3).y,
												Moo::rc().invView().row(3).z ) );

	if (initialised_)
	{
		for (uint i=0; i<cells_.size(); i++)
		{
			cells_[i].updateDistance(camPos_);
		}
	}
}


/**
 * Test to see if the camera position is inside this water volume.
 *
 */
bool Water::checkVolume()
{
	BW_GUARD;
	bool insideVol = bbActual_.distance( camPos_ ) == 0.f;
	return insideVol;
}


/**
 * Check the volume of all the water positions.
 */
void Waters::checkVolumes()
{
	BW_GUARD;
	Vector4 fogColour(1,1,1,1);
	uint i = 0;
	insideVolume(false);
	for (; i < this->size(); i++)
	{
		(*this)[i]->tick();

		if (!Waters::instance().insideVolume())
			Waters::instance().insideVolume( (*this)[i]->checkVolume() );
	}


//TODO: fog stuff
//	if (s_change && insideVolume())
//	{
//		fogColour = (*this)[i-1]->getFogColour();
//
//		//FogController::instance().overrideFog(s_near,s_far, Colour::getUint32(s_col) );
//		FogController::instance().overrideFog(s_near,s_far, 0xffffffff );
//		//FogController::instance().overrideFog(s_near,s_far, Colour::getUint32FromNormalised(fogColour) );
//	}
//	else
//		FogController::instance().setOverride(false);
}

// water.cpp
