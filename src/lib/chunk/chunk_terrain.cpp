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
#include "chunk_terrain.hpp"
#ifndef CODE_INLINE
#include "chunk_terrain.ipp"
#endif

#include "chunk.hpp"
#include "chunk_manager.hpp"
#include "chunk_obstacle.hpp"
#include "chunk_space.hpp"
#include "chunk_terrain_common.hpp"
#include "grid_traversal.hpp"
#include "cstdmf/diary.hpp"
#include "cstdmf/guard.hpp"
#include "terrain/base_terrain_block.hpp"
#include "terrain/base_terrain_renderer.hpp"
#include "moo/render_context.hpp"
#include "terrain/terrain_collision_callback.hpp"
#include "terrain/terrain_data.hpp"
#include "terrain/terrain_finder.hpp"
#include "terrain/terrain_height_map.hpp"
#include "terrain/terrain_hole_map.hpp"
#include "moo/visual.hpp"
#include "physics2/hulltree.hpp"
#include "resmgr/datasection.hpp"
#include "romp/water_scene_renderer.hpp"

#include "umbra_config.hpp"

#if UMBRA_ENABLE
#include "umbra_chunk_item.hpp"
#include <umbraObject.hpp>
#include "chunk_umbra.hpp"
#endif


int ChunkTerrain_token;

bool use_water_culling = true;

DECLARE_DEBUG_COMPONENT2( "Chunk", 0 )

PROFILER_DECLARE( ChunkTerrain_draw, "ChunkTerrain Draw" );

// -----------------------------------------------------------------------------
// Section: ChunkTerrain
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
ChunkTerrain::ChunkTerrain() :
	ChunkItem( WANTS_DRAW ),
	bb_( Vector3::zero(), Vector3::zero() )
#if UMBRA_ENABLE
	,
	umbraHasHoles_(false)
#endif
{
	BW_GUARD;
	static bool firstTime = true;
	if (firstTime)
	{
		MF_WATCH( "Render/Terrain/Use water culling",
				use_water_culling, Watcher::WT_READ_WRITE,
				"Perform extra culling for the terrain blocks in the water scene. " );
		firstTime=false;
	}
}


PROFILER_DECLARE( ChunkTerrain_destruct, "ChunkTerrain_destruct" );

/**
 *	Destructor.
 */
ChunkTerrain::~ChunkTerrain()
{
	// Note, we explicitly dereference pointers so ensuing destruction can be
	// profiled.
	PROFILER_SCOPED( ChunkTerrain_destruct );
	block_				= NULL;
#if UMBRA_ENABLE
	pUmbraWriteModel_	= NULL;
#endif
}

/**
 *	Draw method
 */
void ChunkTerrain::draw()
{
	BW_GUARD_PROFILER( ChunkTerrain_draw );

	static DogWatch drawWatch( "ChunkTerrain" );
	ScopedDogWatch watcher( drawWatch );

    Terrain::TerrainHoleMap &thm = block_->holeMap();
	if (!thm.allHoles())
	{
        Matrix world( pChunk_->transform() );

		if (Moo::rc().reflectionScene() && use_water_culling)
		{
			float height = WaterSceneRenderer::currentScene()->waterHeight();

			BoundingBox bounds( this->bb() );

			//TODO: check to see if this transform is needed at all to get the height range info..
			bounds.transformBy( world );

			bool onPlane = bounds.minBounds().y == height || bounds.maxBounds().y == height;
			bool underWater = ( WaterSceneRenderer::currentCamHeight() < height);

			bool minAbovePlane = bounds.minBounds().y > height;
			bool maxAbovePlane = bounds.maxBounds().y > height;

			bool abovePlane = minAbovePlane && maxAbovePlane;
			bool belowPlane = !minAbovePlane && !maxAbovePlane;

			if (!onPlane)
			{
				if (underWater)
				{
					if (Moo::rc().mirroredTransform() && abovePlane) //reflection
						return;
					if (!Moo::rc().mirroredTransform() && belowPlane) //refraction
						return;
				}
				else
				{
					if (Moo::rc().mirroredTransform() && belowPlane) //reflection
						return;
					if (!Moo::rc().mirroredTransform() && abovePlane) //refraction
						return;
				}
			}
		}
		// Add the terrain block to the terrain's drawlist.
		Terrain::BaseTerrainRenderer::instance()->addBlock( block_.getObject(), world );
	}
}

uint32 ChunkTerrain::typeFlags() const
{
	BW_GUARD;
	if ( Terrain::BaseTerrainRenderer::instance()->version() == 200 )
		return ChunkItemBase::TYPE_DEPTH_ONLY;
	else
		return 0;
}

#if UMBRA_ENABLE

/**
 * Disable the UMBRA occluder model
 */
void ChunkTerrain::disableOccluder()
{
	BW_GUARD;
	pUmbraDrawItem_->pUmbraObject()->object()->setWriteModel( NULL );
}


/**
 * Enable the UMBRA occluder model.
 */
void ChunkTerrain::enableOccluder()
{
	BW_GUARD;
	if (pUmbraWriteModel_.exists())
		pUmbraDrawItem_->pUmbraObject()->object()->setWriteModel( pUmbraWriteModel_->model() );
}

#endif //UMBRA_ENABLE


/**
 *	This method calculates the block's bounding box, and sets into bb_.
 */
void ChunkTerrain::calculateBB()
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( block_ )
	{
		return;
	}

    bb_ = block_->boundingBox();

	// regenerate the collision scene since our bb is different now
	if (pChunk_ != NULL)
	{
		pChunk_->updateBoundingBoxes(this);
	}
}

namespace 
{
	uint32 charToHex( char c )
	{
		uint32 res = 0;
		if (c >= '0' && c <= '9')
		{
			res = uint32( c - '0' );
		}
		else if (c >= 'a' && c <= 'f')
		{
			res = uint32( c - 'a' ) + 0xa;
		}

		return res;
	}
}


bool ChunkTerrain::outsideChunkIDToGrid( const std::string& chunkID, 
											int32& x, int32& z )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( chunkID.size() == 8 )
	{
		return false;
	}

	x = int32( charToHex( chunkID[0] ) ) << 12;
	x |= int32( charToHex( chunkID[1] ) ) << 8;
	x |= int32( charToHex( chunkID[2] ) ) << 4;
	x |= int32( charToHex( chunkID[3] ) );

	if (x & 0x8000)
		x |= 0xffff0000;

	z = int32( charToHex( chunkID[4] ) ) << 12;
	z |= int32( charToHex( chunkID[5] ) ) << 8;
	z |= int32( charToHex( chunkID[6] ) ) << 4;
	z |= int32( charToHex( chunkID[7] ) );

	if (z & 0x8000)
		z |= 0xffff0000;

	return true;
}


#if EDITOR_ENABLED

/*static*/
Terrain::BaseTerrainBlockPtr ChunkTerrain::loadTerrainBlockFromChunk( Chunk * pChunk )
{
	if (!pChunk)
	{
		return NULL;
	}

	DataSectionPtr pChunkDS = BWResource::openSection(
						pChunk->mapping()->path() + pChunk->identifier() + ".chunk");
	if (!pChunkDS)
	{
		return NULL;
	}

	DataSectionPtr pTerrainDS = pChunkDS->openSection( "terrain" );
	if (!pTerrainDS)
	{
		return NULL;
	}

	std::string resName = pTerrainDS->readString( "resource" );	

	Terrain::TerrainSettingsPtr terrainSettings = pChunk->space()->terrainSettings();

	Terrain::BaseTerrainBlockPtr pBlock =
		Terrain::BaseTerrainBlock::loadBlock( 
					pChunk->mapping()->path() + resName,
					pChunk->transform(),
					ChunkManager::instance().cameraTrans().applyToOrigin(),
					terrainSettings,
					NULL );

	return pBlock;
}

#endif


bool ChunkTerrain::doingBackgroundTask() const
{
	return block_ ? block_->doingBackgroundTask() : false;
}

/**
 *	This method loads this terrain block.
 */
bool ChunkTerrain::load( DataSectionPtr pSection, 
						Chunk * pChunk, std::string* errorString )
{	
	BW_GUARD;

	if (!Terrain::BaseTerrainRenderer::instance()->version())
	{// terrain setting initialisation failure
		return false;
	}

	DiaryEntryPtr de = Diary::instance().add( "terrain" );

	std::string resName = pSection->readString( "resource" );	

	Terrain::TerrainSettingsPtr terrainSettings = pChunk->space()->terrainSettings();

	// Allocate the terrainblock.
	block_ = Terrain::BaseTerrainBlock::loadBlock( 
					pChunk->mapping()->path() + resName,
					pChunk->transform(),
					ChunkManager::instance().cameraTrans().applyToOrigin(),
					terrainSettings,
					errorString );

	if (!block_)
	{	
		if ( errorString )
		{
			*errorString = "Could not load terrain block " + resName + 
				" Reason: " + *errorString + "\n";
			de->stop();
			return false;
		}
	}

	this->calculateBB();
#if UMBRA_ENABLE
	if ( ChunkUmbra::softwareMode() ) 
	{
		this->block_->createUMBRAMesh( umbraMesh_ );
		this->umbraHasHoles_ = !this->block_->holeMap().noHoles();
	}
#endif

#if FMOD_SUPPORT
	if (SoundManager::instance().terrainOcclusionEnabled())
	{
		soundOccluder_.construct(	this->block_->heightMap(),
									terrainSettings->directSoundOcclusion(),
									terrainSettings->reverbSoundOcclusion() );

		soundOccluder_.update( pChunk->transform() );
	}
#endif

	de->stop();
	return true;
}


/*
 *	This method gets called when the chunk is bound, this is a good place
 *	to create our umbra objects.
 */
void ChunkTerrain::syncInit()
{
	BW_GUARD;	
#if UMBRA_ENABLE
	// Delete any old umbra draw item
	delete pUmbraDrawItem_;

	if ( ChunkUmbra::softwareMode())
	{
		if ( umbraMesh_.testIndices_.size() == 0 )
				return;
				
		UmbraModelProxyPtr pUmbraModel = UmbraModelProxy::getMeshModel( &umbraMesh_.testVertices_.front(), 
			&umbraMesh_.testIndices_.front(), umbraMesh_.testVertices_.size(), 
			umbraMesh_.testIndices_.size() / 3);

		pUmbraWriteModel_ = NULL;
		if (!this->umbraHasHoles_)
		{
			pUmbraWriteModel_ = UmbraModelProxy::getMeshModel( &umbraMesh_.writeVertices_.front(), 
				&umbraMesh_.writeIndices_.front(), umbraMesh_.writeVertices_.size(), 
				umbraMesh_.writeIndices_.size() / 3);
		}

		umbraMesh_.testIndices_.clear();
		umbraMesh_.writeIndices_.clear();
		umbraMesh_.testVertices_.clear();
		umbraMesh_.writeVertices_.clear();

		UmbraChunkItem* pUmbraChunkItem = new UmbraChunkItem;
		pUmbraChunkItem->init( this, pUmbraModel, this->pChunk_->transform(), pChunk_->getUmbraCell() );

		if (pUmbraWriteModel_)
			pUmbraChunkItem->pUmbraObject()->object()->setWriteModel( pUmbraWriteModel_->model() );

		pUmbraDrawItem_ = pUmbraChunkItem;
	} 
	else
	{		
		UmbraChunkItem* pUmbraChunkItem = new UmbraChunkItem;
		pUmbraChunkItem->init( this, bb(), this->pChunk_->transform(), pChunk_->getUmbraCell() );
		pUmbraDrawItem_ = pUmbraChunkItem;

		pChunk_->addUmbraDrawItem( pUmbraChunkItem );
	}
	this->updateUmbraLenders();
#endif
}


/**
 *	Called when we are put in or taken out of a chunk
 */
void ChunkTerrain::toss( Chunk * pChunk )
{
	BW_GUARD;
	if (pChunk_ != NULL)
	{
		ChunkTerrainCache::instance( *pChunk_ ).pTerrain( NULL );
	}

	this->ChunkItem::toss( pChunk );

	if (pChunk_ != NULL)
	{
		ChunkTerrainCache::instance( *pChunk ).pTerrain( this );
	}
}

bool ChunkTerrain::addYBounds( BoundingBox& bb ) const
{
	bb.addYBounds( this->bb().minBounds().y );
	bb.addYBounds( this->bb().maxBounds().y );
	return true;
}

#undef IMPLEMENT_CHUNK_ITEM_ARGS
#define IMPLEMENT_CHUNK_ITEM_ARGS (pSection, pChunk, &errorString)
IMPLEMENT_CHUNK_ITEM( ChunkTerrain, terrain, 0 )


// -----------------------------------------------------------------------------
// Section: ChunkTerrainCache
// -----------------------------------------------------------------------------

#include "chunk_terrain_obstacle.hpp"

/**
 *	Constructor
 */
ChunkTerrainCache::ChunkTerrainCache( Chunk & chunk ) :
	pChunk_( &chunk ),
	pTerrain_( NULL ),
	pObstacle_( NULL )
{
}

/**
 *	Destructor
 */
ChunkTerrainCache::~ChunkTerrainCache()
{
}


/**
 *	This method is called when our chunk is focussed. We add ourselves to the
 *	chunk space's obstacles at that point.
 */
int ChunkTerrainCache::focus()
{
	BW_GUARD;
	if (!pTerrain_ || !pObstacle_) return 0;

	const Vector3 & mb = pObstacle_->bb_.minBounds();
	const Vector3 & Mb = pObstacle_->bb_.maxBounds();

/*
	// figure out the border
	HullBorder	border;
	for (int i = 0; i < 6; i++)
	{
		// calculate the normal and d of this plane of the bb
		int ax = i >> 1;

		Vector3 normal( 0, 0, 0 );
		normal[ ax ] = (i&1) ? -1.f : 1.f;;

		float d = (i&1) ? -Mb[ ax ] : mb[ ax ];

		// now apply the transform to the plane
		Vector3 ndtr = pObstacle_->transform_.applyPoint( normal * d );
		Vector3 ntr = pObstacle_->transform_.applyVector( normal );
		border.push_back( PlaneEq( ntr, ntr.dotProduct( ndtr ) ) );
	}
*/

	// we assume that we'll be in only one column
	Vector3 midPt = pObstacle_->transform_.applyPoint( (mb + Mb) / 2.f );

	ChunkSpace::Column * pCol = pChunk_->space()->column( midPt );
	MF_ASSERT_DEV( pCol );

	// ok, just add the obstacle then
	if( pCol )
		pCol->addObstacle( *pObstacle_ );

	//dprintf( "ChunkTerrainCache::focus: "
	//	"Adding hull of terrain (%f,%f,%f)-(%f,%f,%f)\n",
	//	mb[0],mb[1],mb[2], Mb[0],Mb[1],Mb[2] );

	// which counts for just one
	return 1;
}


/**
 *	This method sets the terrain pointer
 */
void ChunkTerrainCache::pTerrain( ChunkTerrain * pT )
{
	BW_GUARD;
	if (pT != pTerrain_)
	{
		if (pObstacle_)
		{
			// flag column as stale first
			const Vector3 & mb = pObstacle_->bb_.minBounds();
			const Vector3 & Mb = pObstacle_->bb_.maxBounds();
			Vector3 midPt = pObstacle_->transform_.applyPoint( (mb + Mb) / 2.f );
			ChunkSpace::Column * pCol = pChunk_->space()->column( midPt, false );
			if (pCol != NULL) pCol->stale();

			pObstacle_ = NULL;
		}

		pTerrain_ = pT;

		if (pTerrain_ != NULL)
		{
            // Completely flat terrain will not work with the collision system.
            // In this case offset the y coordinates a little.
            if (pTerrain_->bb_.minBounds().y == pTerrain_->bb_.maxBounds().y)
            {
                pTerrain_->bb_.addYBounds(pTerrain_->bb_.minBounds().y + 1.0f);
            }
			pObstacle_ = new ChunkTerrainObstacle( *pTerrain_->block_,
				pChunk_->transform(), &pTerrain_->bb_, pT );

			if (pChunk_->focussed()) this->focus();
		}
	}
}


// -----------------------------------------------------------------------------
// Section: Static initialisers
// -----------------------------------------------------------------------------

/// Static cache instance accessor initialiser
ChunkCache::Instance<ChunkTerrainCache> ChunkTerrainCache::instance;



// -----------------------------------------------------------------------------
// Section: TerrainFinder
// -----------------------------------------------------------------------------

#include "chunk_manager.hpp"

/**
 *	This class implements the TerrainFinder interface. Its purpose is to be an
 *	object that Moo can use to access the terrain. It is implemented like this
 *	so that other libraries do not need to know about the Chunk library.
 */
class TerrainFinderInstance : public Terrain::TerrainFinder
{
public:
	TerrainFinderInstance()
	{
		Terrain::BaseTerrainBlock::setTerrainFinder( *this );
	}

	virtual Terrain::TerrainFinder::Details findOutsideBlock( const Vector3 & pos )
	{
		BW_GUARD;
		Terrain::TerrainFinder::Details details;

		// TODO: At the moment, assuming the space the camera is in.
		ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();

		//find the chunk
		if (pSpace)
		{
			ChunkSpace::Column* pColumn = pSpace->column( pos, false );

			if ( pColumn != NULL )
			{
				Chunk * pChunk = pColumn->pOutsideChunk();

				if (pChunk != NULL)
				{
					//find the terrain block
					ChunkTerrain * pChunkTerrain =
						ChunkTerrainCache::instance( *pChunk ).pTerrain();

					if (pChunkTerrain != NULL)
					{
						details.pBlock_ = pChunkTerrain->block().getObject();
						details.pInvMatrix_ = &pChunk->transformInverse();
						details.pMatrix_ = &pChunk->transform();
					}
				}
			}
		}

		return details;
	}
};


static TerrainFinderInstance s_terrainFinder;

// chunk_terrain.cpp
