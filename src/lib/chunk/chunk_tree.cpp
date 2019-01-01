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
#include "chunk_tree.hpp"
#include "chunk_obstacle.hpp"
#include "umbra_config.hpp"
#include "cstdmf/diary.hpp"
#include "cstdmf/guard.hpp"
#include "moo/render_context.hpp"
#include "physics2/bsp.hpp"
#include "speedtree/speedtree_renderer.hpp"
#include "romp/water_scene_renderer.hpp"

#if UMBRA_ENABLE
#include "umbra_chunk_item.hpp"
#endif

DECLARE_DEBUG_COMPONENT2( "Chunk", 0 )

// -----------------------------------------------------------------------------
// Section: ChunkTree
// -----------------------------------------------------------------------------

int ChunkTree_token;

PROFILER_DECLARE( ChunkTree_draw, "ChunkTree Draw" );

/**
 *	Constructor.
 */
ChunkTree::ChunkTree() :
	BaseChunkTree(),
	tree_(NULL),
	reflectionVisible_(false),
	errorInfo_(NULL)
{}


/**
 *	Destructor.
 */
ChunkTree::~ChunkTree()
{}


/**
 *	The overridden load method
 *	@param	pSection	pointer to the DataSection that contains the tree information
 *	@param	pChunk	the chunk contains the tree
 *	@param	errorString	the string to return the loading error if there is any
 *	@return	true if load successfully, otherwise false
 */
bool ChunkTree::load(DataSectionPtr pSection, Chunk * pChunk, std::string* errorString )
{
	BW_GUARD;
	DiaryEntryPtr de = Diary::instance().add( "tree" );

	// we want transform and reflection to be set even
	// if the tree fails to load. They may be used to
	// render a place holder model in case load fails

	// rotation inverse will be 
	// updated automatically
	this->setTransform(pSection->readMatrix34("transform", Matrix::identity));
	this->reflectionVisible_ = pSection->readBool(
		"reflectionVisible", reflectionVisible_);

	uint seed = pSection->readInt("seed", 1);
	std::string filename = pSection->readString("spt");

	bool result = this->loadTree(filename.c_str(), seed, pChunk);
	if ( !result && errorString )
	{
		*errorString = "Failed to load tree " + filename;
	}

	de->stop();
	return result;
}

/**
 *	this method loads the speedtree
 *	@param	filename	the speedtree filename
 *	@param	seed	the seed of the speed tree
 *	@param	chunk	the chunk contains the speedtree
 *	@return	true if load successfully, otherwise false
 */
bool ChunkTree::loadTree(const char * filename, int seed, Chunk * chunk)
{
	BW_GUARD;
	try
	{
		// load the speedtree
		using speedtree::SpeedTreeRenderer;
		std::auto_ptr< SpeedTreeRenderer > speedTree(new SpeedTreeRenderer);

		Matrix world = chunk ? chunk->transform() : Matrix::identity;
		world.preMultiply( this->transform() );

		speedTree->load( filename, seed, world );		
		this->tree_ = speedTree;

		// update collision data
		this->BaseChunkTree::setBoundingBox(this->tree_->boundingBox());
		this->BaseChunkTree::setBSPTree(&this->tree_->bsp());
		this->errorInfo_.reset(NULL);
	}
	catch (const std::runtime_error &err)
	{
		this->errorInfo_.reset(new ErrorInfo);
		this->errorInfo_->what     = err.what();
		this->errorInfo_->filename = filename;
		this->errorInfo_->seed     = seed;

		ERROR_MSG( "Error loading tree: %s\n", err.what() );
	}
	return this->errorInfo_.get() == NULL;
}

/**
 *	The overridden addYBounds method
 */
bool ChunkTree::addYBounds(BoundingBox& bb) const
{	
	BW_GUARD;
	BoundingBox treeBB = this->boundingBox();
	treeBB.transformBy(this->transform());
	bb.addYBounds(treeBB.minBounds().y);
	bb.addYBounds(treeBB.maxBounds().y);

	return true;
}

/**
 *	The overridden syncInit method
 */
void ChunkTree::syncInit()
{
	BW_GUARD;	
#if UMBRA_ENABLE
	// Delete any old umbra draw item
	delete pUmbraDrawItem_;

	Matrix m = this->pChunk_->transform();
	m.preMultiply( transform() );

	UmbraChunkItem* pUmbraChunkItem = new UmbraChunkItem;
	pUmbraChunkItem->init( this, boundingBox(), m, pChunk_->getUmbraCell() );
	pUmbraDrawItem_ = pUmbraChunkItem;
	this->updateUmbraLenders();

	pChunk_->addUmbraDrawItem( pUmbraChunkItem );
#endif
}

//bool use_water_culling = true;

/**
 *	The overridden draw method
 */
void ChunkTree::draw()
{
	BW_GUARD_PROFILER( ChunkTree_draw );

	if (this->tree_.get() != NULL && 
		(!Moo::rc().reflectionScene() || reflectionVisible_))
	{	
		if (Moo::rc().reflectionScene())
		{
			float height = WaterSceneRenderer::currentScene()->waterHeight();

			Matrix world( pChunk_->transform() );	
			BoundingBox bounds( this->boundingBox() );

			world.preMultiply(this->transform());
			bounds.transformBy( world );
			
			bool maxBelowPlane = bounds.maxBounds().y < height;
			if (maxBelowPlane)
				return;
		}

		BorrowedLightCombinerHolder borrowedLightCombiner( borrowedLightCombiner_, borrowers_ );

		Matrix world = Moo::rc().world();
		world.preMultiply(this->transform());
		this->tree_->draw(world,(uint32)this->pChunk_);
	}
}


/**
 *	The overridden lend method
 */
void ChunkTree::lend(Chunk * pLender)
{
	BW_GUARD;
	if (pChunk_ != NULL)
	{
		Matrix world(pChunk_->transform());
		world.preMultiply(this->transform());

		BoundingBox bb = this->boundingBox();
		bb.transformBy(world);

		this->lendByBoundingBox(pLender, bb);
	}
}

/**
 *	We support depth only
 */
uint32 ChunkTree::typeFlags() const
{
	return speedtree::SpeedTreeRenderer::depthOnlyPass()
			? ChunkItemBase::TYPE_DEPTH_ONLY : 0;
}

/**
 *	This method sets the world transform of the tree
 *	@param	transform	the new transform
 */
void ChunkTree::setTransform(const Matrix & transform)
{
	BW_GUARD;
	this->BaseChunkTree::setTransform(transform);
	if (this->tree_.get() != NULL)
	{
		Matrix world = pChunk_ ? pChunk_->transform() : Matrix::identity;
		world.preMultiply( this->transform() );
		this->tree_->resetTransform( world );
	}
}

/**
 *	This method returns the seed of the tree
 *	@return	the seed of the tree
 */
uint ChunkTree::seed() const
{
	BW_GUARD;
	MF_ASSERT_DEV(this->tree_.get() != NULL || this->errorInfo_.get() != NULL);
	return this->tree_.get() != NULL
		? this->tree_->seed()
		: this->errorInfo_->seed;
}

/**
 *	This method sets the seed of the tree
 *	@return	true if successfully set, otherwise false
 */
bool ChunkTree::seed(uint seed)
{
	BW_GUARD;
	bool result = false;
	if (this->tree_.get() != NULL)
	{
		result = this->loadTree(
			this->tree_->filename(), seed,
			this->ChunkItemBase::chunk());
	}
	return result;
}

/**
 *	This method returns the tree filename
 *	@return	the tree filename
 */
const char * ChunkTree::filename() const
{
	BW_GUARD;
	MF_ASSERT_DEV(this->tree_.get() != NULL || this->errorInfo_.get() != NULL);
	return this->tree_.get() != NULL
		? this->tree_->filename()
		: this->errorInfo_->filename.c_str();
}

/**
 *	This method sets the tree filename
 *	@return	true if successfully set, otherwise false
 */
bool ChunkTree::filename(const char * filename)
{
	BW_GUARD;
	MF_ASSERT_DEV(this->tree_.get() != NULL || this->errorInfo_.get() != NULL);
	int seed = this->tree_.get() != NULL 
		? this->tree_->seed() 
		: this->errorInfo_->seed;

	return this->loadTree(filename, seed, this->ChunkItemBase::chunk());
}

/**
 *	This method returns the reflection visible flag
 *	@return	true if the tree will be reflected by water, otherwise false
 */
bool ChunkTree::getReflectionVis() const
{
	return this->reflectionVisible_;
}

/**
 *	This method sets the reflection visible flag
 *	@return	true if set successfully, otherwise false
 */
bool ChunkTree::setReflectionVis(const bool & reflVis) 
{ 
	this->reflectionVisible_= reflVis;
	return true;
}

/**
 *	This method returns if the load is failed
 *	@return	true if load is FAILED, otherwise false
 */
bool ChunkTree::loadFailed() const
{
	return this->errorInfo_.get() != NULL;
}

/**
 *	This method returns the last error
 *	@return	a C string describes the last error
 */
const char * ChunkTree::lastError() const
{
	return this->errorInfo_.get() != NULL
		? this->errorInfo_->what.c_str()
		: "";
}

/// Static factory initialiser
#undef IMPLEMENT_CHUNK_ITEM_ARGS
#define IMPLEMENT_CHUNK_ITEM_ARGS (pSection, pChunk, &errorString)
IMPLEMENT_CHUNK_ITEM(ChunkTree, speedtree, 0)
