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
#include "chunk_dynamic_obstacle.hpp"

#include "chunk/chunk_space.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "chunk/chunk.hpp"

#include "model/super_model.hpp"
#include "model/super_model_dye.hpp"

#include "moo/render_context.hpp"

#include "cstdmf/debug.hpp"

#include "chunk/umbra_config.hpp"

#if UMBRA_ENABLE
#include "chunk/umbra_chunk_item.hpp"
#endif

DECLARE_DEBUG_COMPONENT2( "Duplo", 0 )

PROFILER_DECLARE( ChunkDynamicObstacle_tick, "ChunkDynamicObstacle Tick" );
PROFILER_DECLARE( ChunkStaticObstacle_tick, "ChunkStaticObstacle Tick" );

// -----------------------------------------------------------------------------
// Section: ChunkDynamicObstacles declaration
// -----------------------------------------------------------------------------


class ChunkDynamicObstacles : public ChunkCache
{
public:
	ChunkDynamicObstacles( Chunk & chunk ) : pChunk_( &chunk ) { }
	~ChunkDynamicObstacles() { MF_ASSERT_DEV( cdos_.empty() ); }

	virtual int focus();

	void add( ChunkDynamicObstacle * cdo );
	void mod( ChunkDynamicObstacle * cdo, const Matrix & oldWorld );
	void del( ChunkDynamicObstaclePtr cdo );

	static Instance<ChunkDynamicObstacles> instance;

private:
	void addOne( const ChunkObstacle & cso );
	void modOne( const ChunkObstacle & cso, const Matrix & oldWorld );
	void delOne( const ChunkObstacle & cso );

	Chunk *				pChunk_;
	std::vector<ChunkDynamicObstaclePtr>	cdos_;
};

ChunkCache::Instance<ChunkDynamicObstacles> ChunkDynamicObstacles::instance;


// -----------------------------------------------------------------------------
// Section: ChunkDynamicObstacle
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
ChunkDynamicObstacle::ChunkDynamicObstacle( PyModelObstacle * pObstacle ) :
	ChunkDynamicEmbodiment( pObstacle, 
		(WantFlags)(WANTS_DRAW | WANTS_TICK | (4 << USER_FLAG_SHIFT)) )
{
	BW_GUARD;
	this->pObstacle()->attach();
#if UMBRA_ENABLE
	// Set up our unit boundingbox for the obstacle, we use a unit bounding box
	// and scale it using transforms since this may be a dynamic object.

	UmbraChunkItem* pUmbraChunkItem = new UmbraChunkItem;
	pUmbraChunkItem->init( this, BoundingBox( Vector3(0,0,0), Vector3(1,1,1)), Matrix::identity, NULL );
	pUmbraDrawItem_ = pUmbraChunkItem;

#endif
}


/**
 *	Destructor.
 */
ChunkDynamicObstacle::~ChunkDynamicObstacle()
{
	BW_GUARD;
	this->pObstacle()->detach();
}


/**
 * Chunk item tick method
 */
void ChunkDynamicObstacle::tick( float dTime )
{
	BW_GUARD_PROFILER( ChunkDynamicObstacle_tick );

	this->pObstacle()->tick( dTime );

	// Update the visibility bounds of our parent chunk
	if (pChunk_)
	{
		pChunk_->space()->updateOutsideChunkInQuadTree( pChunk_ );
	}

#if UMBRA_ENABLE
	// Update umbra object
	if (pChunk_ != NULL)
	{
		// Get the object to cell transform
		Matrix m = Matrix::identity;
		m.preMultiply( worldTransform() );

		BoundingBox bb;
		this->localBoundingBox( bb, true );

		// Create the scale transform from the bounding box
		Vector3 scale = bb.maxBounds() - bb.minBounds();

		if ((scale.x != 0.f && scale.y != 0.f && scale.z != 0.f) &&
			!(bb == BoundingBox::s_insideOut_) &&
			(scale.length() < 1000000.f))
		{
			// Set up our transform, the transform includes the scale of the bounding box
			Matrix m2;
			m2.setTranslate( bb.minBounds().x, bb.minBounds().y, bb.minBounds().z );
			m.preMultiply( m2 );

			m2.setScale( scale.x, scale.y, scale.z );
			m.preMultiply( m2 );

			pUmbraDrawItem_->updateCell( pChunk_->getUmbraCell() );
			pUmbraDrawItem_->updateTransform( m );
		}
		else
		{
			pUmbraDrawItem_->updateCell( NULL );
		}
	}
	else
	{
		pUmbraDrawItem_->updateCell( NULL );
	}
#endif
}

/**
 * Chunk item draw method
 */
void ChunkDynamicObstacle::draw()
{
	BW_GUARD;
	this->pObstacle()->draw( this->worldTransform() );
}

/**
 *	Chunk item toss method
 */
void ChunkDynamicObstacle::toss( Chunk * pChunk )
{
	BW_GUARD;
	if (pChunk_ != NULL)
		ChunkDynamicObstacles::instance( *pChunk_ ).del( this );

	this->ChunkDynamicEmbodiment::toss( pChunk );

	if (pChunk_ != NULL)
		ChunkDynamicObstacles::instance( *pChunk_ ).add( this );
}

void ChunkDynamicObstacle::enterSpace( ChunkSpacePtr pSpace, bool transient )
{
	BW_GUARD;
	if (!transient) this->pObstacle()->enterWorld( this );
	this->ChunkDynamicEmbodiment::enterSpace( pSpace, transient );
}

void ChunkDynamicObstacle::leaveSpace( bool transient )
{
	BW_GUARD;
	this->ChunkDynamicEmbodiment::leaveSpace( transient );
	if (!transient) this->pObstacle()->leaveWorld();
}

void ChunkDynamicObstacle::move( float dTime )
{
	BW_GUARD;
	// get in the right chunk
	this->sync();

	// update the positions of all our obstacles
	PyModelObstacle * pObstacle = this->pObstacle();
	PyModelObstacle::Obstacles & obs = pObstacle->obstacles();
	if (!obs.empty())
	{
		Matrix oldWorld = obs[0]->transform_;

		Matrix world = pObstacle->worldTransform();
		Matrix worldInverse; worldInverse.invert( world );
		for (uint i = 0; i < obs.size(); i++)
		{
			obs[i]->transform_ = world;
			obs[i]->transformInverse_ = worldInverse;
		}

		// make sure they are in the right columns
		if (pChunk_ != NULL)
			ChunkDynamicObstacles::instance( *pChunk_ ).mod( this, oldWorld );
	}

	// and call our base class method
	this->ChunkDynamicEmbodiment::move( dTime );
}


const Matrix & ChunkDynamicObstacle::worldTransform() const
{
	BW_GUARD;
	return this->pObstacle()->worldTransform();
}


void ChunkDynamicObstacle::localBoundingBox( BoundingBox & bb, bool skinny ) const
{
	BW_GUARD;
	pObstacle()->localBoundingBox( bb, skinny );
}


void ChunkDynamicObstacle::worldBoundingBox( BoundingBox & bb, const Matrix& world, bool skinny ) const
{
	BW_GUARD;
	pObstacle()->worldBoundingBox( bb, world, skinny );
}


/**
 *	Static method to convert from a PyObject to a ChunkEmbodiment
 */
int ChunkDynamicObstacle::convert( PyObject * pObj, ChunkEmbodimentPtr & pNew,
	const char * varName )
{
	BW_GUARD;
	if (!PyModelObstacle::Check( pObj )) return 0;

	PyModelObstacle * pModelObstacle = (PyModelObstacle*)pObj;
	if (!pModelObstacle->dynamic()) return 0;

	if (pModelObstacle->attached())
	{
		PyErr_Format( PyExc_TypeError, "%s must be set to a PyModelObstacle "
			"that is not attached elsewhere", varName );
		return -1;
	}

	pNew = new ChunkDynamicObstacle( pModelObstacle );
	return 0;
}

/// registerer for our type of ChunkEmbodiment
static ChunkEmbodimentRegisterer<ChunkDynamicObstacle> registerer;

// -----------------------------------------------------------------------------
// Section: ChunkDynamicObstacles
// -----------------------------------------------------------------------------

int ChunkDynamicObstacles::focus()
{
	BW_GUARD;
	// add all
	for (uint i = 0; i < cdos_.size(); i++)
	{
		PyModelObstacle::Obstacles & obs = cdos_[i]->pObstacle()->obstacles();
		for (uint j = 0; j < obs.size(); j++)
		{
			this->addOne( *obs[j] );
		}
	}
	return cdos_.size();
}

void ChunkDynamicObstacles::add( ChunkDynamicObstacle * cdo )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( pChunk_->isBound() )
	{
		return;
	}

	cdos_.push_back( cdo );

	if (pChunk_->focussed())
	{
		PyModelObstacle::Obstacles & obs = cdo->pObstacle()->obstacles();
		for (uint j = 0; j < obs.size(); j++)
		{
			this->addOne( *obs[j] );
		}
	}
}

void ChunkDynamicObstacles::mod( ChunkDynamicObstacle * cdo,
	const Matrix & oldWorld )
{
	BW_GUARD;
	if (pChunk_->focussed())
	{
		PyModelObstacle::Obstacles & obs = cdo->pObstacle()->obstacles();
		for (uint j = 0; j < obs.size(); j++)
		{
			this->modOne( *obs[j], oldWorld );
		}
	}
}

void ChunkDynamicObstacles::del( ChunkDynamicObstaclePtr cdo )
{
	BW_GUARD;
	uint i;
	for (i = 0; i < cdos_.size(); i++)
	{
		if (cdos_[i] == cdo) break;
	}
	IF_NOT_MF_ASSERT_DEV( i < cdos_.size() )
	{
		return;
	}
	cdos_[i] = cdos_.back();
	cdos_.pop_back();

	if (pChunk_->focussed())
	{
		PyModelObstacle::Obstacles & obs = cdo->pObstacle()->obstacles();
		for (uint j = 0; j < obs.size(); j++)
		{
			this->delOne( *obs[j] );
		}
	}
}


void ChunkDynamicObstacles::addOne( const ChunkObstacle & cso )
{
	BW_GUARD;
	// put obstacles in appropriate columns

	BoundingBox obstacleBB( cso.bb_ );
	obstacleBB.transformBy( cso.transform_ );

	for(	int x = BaseChunkSpace::pointToGrid( obstacleBB.minBounds().x );
			x <= BaseChunkSpace::pointToGrid( obstacleBB.maxBounds().x );
			x++ )
	{
		for(int z = BaseChunkSpace::pointToGrid( obstacleBB.minBounds().z );
			z <= BaseChunkSpace::pointToGrid( obstacleBB.maxBounds().z );
			z++ )
		{
			// This code is the same as in modOne, if you change one, 
			// change the other too
			Vector3 columnPoint(	( x + 0.5f ) * GRID_RESOLUTION,
									0.0f,
									( z + 0.5f ) * GRID_RESOLUTION );

			ChunkSpace::Column * pColumn = 
				pChunk_->space()->column( columnPoint, false );
			if( pColumn )
			{
				BoundingBox columnBB(	Vector3((x+0) * GRID_RESOLUTION,
												0.0f,
												(z+0) * GRID_RESOLUTION ),
										Vector3((x+1) * GRID_RESOLUTION,
												0.0f,
												(z+1) * GRID_RESOLUTION ) );
				columnBB.addYBounds( MIN_CHUNK_HEIGHT );
				columnBB.addYBounds( MAX_CHUNK_HEIGHT );

				if ( columnBB.intersects( obstacleBB ) )
					pColumn->addDynamicObstacle( cso );
			}
		}
	}
}

void ChunkDynamicObstacles::modOne(	const ChunkObstacle & cso,
									const Matrix & oldWorld )
{
	BW_GUARD;
	BoundingBox oldBB( cso.bb_ );
	BoundingBox newBB( cso.bb_ );

	oldBB.transformBy( oldWorld );
	newBB.transformBy( cso.transform_ );

	BoundingBox sumBB( oldBB );
	sumBB.addBounds( newBB );

	for(	int x = BaseChunkSpace::pointToGrid( sumBB.minBounds().x );
			x <= BaseChunkSpace::pointToGrid( sumBB.maxBounds().x );
			x++ )
	{
		for(int z = BaseChunkSpace::pointToGrid( sumBB.minBounds().z );
			z <= BaseChunkSpace::pointToGrid( sumBB.maxBounds().z );
			z++ )
		{
			// This code is the same as in addOne, if you change one, 
			// change the other too
			Vector3 columnPoint(	( x + 0.5f ) * GRID_RESOLUTION,
									0.0f,
									( z + 0.5f ) * GRID_RESOLUTION );

			ChunkSpace::Column * pColumn =
				pChunk_->space()->column( columnPoint, false );
			if( pColumn )
			{
				BoundingBox columnBB(	Vector3((x+0) * GRID_RESOLUTION,
												0.0f,
												(z+0) * GRID_RESOLUTION ),
										Vector3((x+1) * GRID_RESOLUTION,
												0.0f,
												(z+1) * GRID_RESOLUTION ) );
				columnBB.addYBounds( MIN_CHUNK_HEIGHT );
				columnBB.addYBounds( MAX_CHUNK_HEIGHT );

				bool inOld = columnBB.intersects( oldBB );
				bool inNew = columnBB.intersects( newBB );

				if (inNew && !inOld)
					pColumn->addDynamicObstacle( cso );

				if (!inNew && inOld)
					pColumn->delDynamicObstacle( cso );
			}
		}
	}
}

void ChunkDynamicObstacles::delOne( const ChunkObstacle & cso )
{
	BW_GUARD;
	// remove obstacles from appropriate columns

	BoundingBox obstacleBB( cso.bb_ );
	obstacleBB.transformBy( cso.transform_ );

	for(	int x = BaseChunkSpace::pointToGrid( obstacleBB.minBounds().x );
			x <= BaseChunkSpace::pointToGrid( obstacleBB.maxBounds().x );
			x++ )
	{
		for( int z = BaseChunkSpace::pointToGrid( obstacleBB.minBounds().z );
			z <= BaseChunkSpace::pointToGrid( obstacleBB.maxBounds().z );
			z++ )
		{
			Vector3 columnPoint(	x * GRID_RESOLUTION,
									0.0f,
									z * GRID_RESOLUTION );

			ChunkSpace::Column * pColumn =
				pChunk_->space()->column( columnPoint, false );
			if( pColumn )
			{
				BoundingBox columnBB(	columnPoint,
										Vector3( (x+1) * GRID_RESOLUTION,
										0.0f,
										(z+1) * GRID_RESOLUTION ) );
				columnBB.addYBounds( MIN_CHUNK_HEIGHT );
				columnBB.addYBounds( MAX_CHUNK_HEIGHT );

				if ( columnBB.intersects( obstacleBB ) )
					pColumn->delDynamicObstacle( cso );
			}
		}
	}
}


// -----------------------------------------------------------------------------
// Section: ChunkStaticObstacle
// -----------------------------------------------------------------------------

#include "chunk/chunk_model_obstacle.hpp"
#include "moo/node.hpp"


/**
 *	Constructor.
 */
ChunkStaticObstacle::ChunkStaticObstacle( PyModelObstacle * pObstacle ) :
	ChunkStaticEmbodiment( pObstacle, (WantFlags)(WANTS_DRAW | WANTS_TICK) ),
	worldTransform_( pObstacle->worldTransform() )
{
	BW_GUARD;
	this->pObstacle()->attach();
#if UMBRA_ENABLE
	// Set up our unit boundingbox for the obstacle, we use a unit bounding box
	// and scale it using transforms.
	UmbraChunkItem* pUmbraChunkItem = new UmbraChunkItem;
	pUmbraChunkItem->init( this, BoundingBox( Vector3(0,0,0), Vector3(1,1,1)), Matrix::identity, NULL );
	pUmbraDrawItem_ = pUmbraChunkItem;
#endif
}


/**
 *	Destructor.
 */
ChunkStaticObstacle::~ChunkStaticObstacle()
{
	BW_GUARD;
	this->pObstacle()->detach();
}


/**
 * Chunk item tick method
 */
void ChunkStaticObstacle::tick( float dTime )
{
	BW_GUARD_PROFILER( ChunkStaticObstacle_tick );

	this->pObstacle()->tick( dTime );
#if UMBRA_ENABLE
	// Update umbra object
	if (pChunk_ != NULL)
	{
		Matrix m = Matrix::identity;
		m.preMultiply( worldTransform() );

		BoundingBox bb;
		this->localBoundingBox( bb, true );

		// Create the scale transform from the bounding box
		Vector3 scale = bb.maxBounds() - bb.minBounds();

		if ((scale.x != 0.f && scale.y != 0.f && scale.z != 0.f) &&
			!(bb == BoundingBox::s_insideOut_) &&
			(scale.length() < 1000000.f))
		{
			// Set up our transform, the transform includes the scale of the bounding box
			Matrix m2;
			m2.setTranslate( bb.minBounds().x, bb.minBounds().y, bb.minBounds().z );
			m.preMultiply( m2 );

			m2.setScale( scale.x, scale.y, scale.z );
			m.preMultiply( m2 );
			pUmbraDrawItem_->updateCell( pChunk_->getUmbraCell() );
			pUmbraDrawItem_->updateTransform( m );
		}
		else
		{
			pUmbraDrawItem_->updateCell( NULL );
		}
	}
	else
	{
		pUmbraDrawItem_->updateCell( NULL );
	}
#endif
}

/**
 *	Chunk item draw method
 */
void ChunkStaticObstacle::draw()
{
	BW_GUARD;
	this->pObstacle()->draw( worldTransform_ );
}

/**
 *	Chunk item toss method
 */
void ChunkStaticObstacle::toss( Chunk * pChunk )
{
	BW_GUARD;
	if (pChunk_ != NULL)
		ChunkModelObstacle::instance( *pChunk_ ).delObstacles( this );

	this->ChunkStaticEmbodiment::toss( pChunk );

	if (pChunk_ != NULL)
	{
		PyModelObstacle * pObstacle = this->pObstacle();
		PyModelObstacle::Obstacles & obs = pObstacle->obstacles();
		if (!obs.empty())
		{
			ChunkModelObstacle & chunkModelObstacle =
				ChunkModelObstacle::instance( *pChunk_ );

			Matrix worldInverse; worldInverse.invert( worldTransform_ );
			for (uint i = 0; i < obs.size(); i++)
			{
				obs[i]->transform_ = worldTransform_;
				obs[i]->transformInverse_ = worldInverse;

				chunkModelObstacle.addObstacle( &*obs[i] );
			}
		}
	}
}


/**
 *	overridden lend method
 */
void ChunkStaticObstacle::lend( Chunk * pLender )
{
	BW_GUARD;
	if (pChunk_)
	{
		BoundingBox bb;
		pObstacle()->worldBoundingBox( bb, this->worldTransform_ );
		this->lendByBoundingBox( pLender, bb );
	}
}


void ChunkStaticObstacle::enterSpace( ChunkSpacePtr pSpace, bool transient )
{
	BW_GUARD;
	if (!transient) this->pObstacle()->enterWorld( this );
	this->ChunkStaticEmbodiment::enterSpace( pSpace, transient );
}

void ChunkStaticObstacle::leaveSpace( bool transient )
{
	BW_GUARD;
	this->ChunkStaticEmbodiment::leaveSpace( transient );
	if (!transient) this->pObstacle()->leaveWorld();
}


const Matrix & ChunkStaticObstacle::worldTransform() const
{
	return worldTransform_;
}


void ChunkStaticObstacle::localBoundingBox( BoundingBox & bb, bool skinny ) const
{
	BW_GUARD;
	pObstacle()->localBoundingBox( bb, skinny );
}


void ChunkStaticObstacle::worldBoundingBox( BoundingBox & bb, const Matrix& world, bool skinny ) const
{
	BW_GUARD;
	pObstacle()->worldBoundingBox( bb, world, skinny );
}


/**
 *	Static method to convert from a PyObject to a ChunkEmbodiment
 */
int ChunkStaticObstacle::convert( PyObject * pObj, ChunkEmbodimentPtr & pNew,
	const char * varName )
{
	BW_GUARD;
	if (!PyModelObstacle::Check( pObj )) return 0;

	PyModelObstacle * pModelObstacle = (PyModelObstacle*)pObj;
	if (pModelObstacle->dynamic()) return 0;

	if (pModelObstacle->attached())
	{
		PyErr_Format( PyExc_TypeError, "%s must be set to a PyModelObstacle "
			"that is not attached elsewhere", varName );
		return -1;
	}

	pNew = new ChunkStaticObstacle( pModelObstacle );
	return 0;
}

/// registerer for our type of ChunkEmbodiment
static ChunkEmbodimentRegisterer<ChunkStaticObstacle> registerer2;


// -----------------------------------------------------------------------------
// Section: PyModelObstacle
// -----------------------------------------------------------------------------

#include "pydye.hpp"

PY_TYPEOBJECT( PyModelObstacle )

PY_BEGIN_METHODS( PyModelObstacle )
	PY_METHOD( node )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyModelObstacle )

	/*~	attribute PyModelObstacle.sources
	 *	@type Read-only tuple of strings
	 *
	 *	This is the set of .model resources specified upon creation
	 *	@see BigWorld.PyModelObstacle
	 */
	PY_ATTRIBUTE( sources )

	/*~	attribute PyModelObstacle.matrix
	 *	@type MatrixProviderPtr
	 *
	 *	This is the transform matrix for the PyModelObstacle.  As the 
	 *	MatrixProvider is updated, the PyModelObstacle will follow it.
	 */
	PY_ATTRIBUTE( matrix )

	/*~	attribute PyModelObstacle.dynamic
	 *	@type bool
	 *
	 *	If true, PyModelObstacle will be treated as dynamic, otherwise static.
	 *	Note:  This value should be set upon creation.  Once it is attached/assigned, 
	 *	it cannot be changed.
	 */
	PY_ATTRIBUTE( dynamic )

	/*~	attribute PyModelObstacle.attached
	 *	@type Read-only bool
	 *
	 *	This flag is set to true when the PyModelObstacle is assigned to an Entity.  Once this occurs, it is placed 
	 *	in the collision scene and no further changes can be made to its dynamic nature.
	 */
	PY_ATTRIBUTE( attached )

	/*~	attribute PyModelObstacle.vehicleID
	 *	@type uint32
	 *
	 *	To place PyModelObstacle on a vehicle, set the vehicleID to the EntityID of the vehicle.
	 *	To alight from the vehicle, set it back to 0. This parameter is used to identify platforms 
	 *  when applying gravity e.g. MovingPlatform.py.
	 */
	PY_ATTRIBUTE( vehicleID )

	/*~	attribute PyModelObstacle.bounds
	 *	@type Read-only MatrixProvider
	 *
	 *	Stores the bounding box of the PyModelObstacle as a MatrixProvider.
	 */
	PY_ATTRIBUTE( bounds )

PY_END_ATTRIBUTES()

/*~	function BigWorld.PyModelObstacle
 *
 *	Creates and returns a new PyModelObstacle object, which is used to integrate a model into the collision scene.
 *
 *	Example:
 *	@{
 *		self.model = BigWorld.PyModelObstacle("models/body.model", "models/head.model", self.matrix, 1)
 *	@}
 *
 *	Once created and assigned (self.model = PyModelObstacle), the PyObstacleModel becomes attached, hence its 
 *	dynamic nature cannot be changed.  If left as static, the position of the PyObstacleModel will not be updated 
 *	as the MatrixProvider changes.  PyModelObstacles are reasonably expensive, hence should only be used when truly 
 *	required.
 *
 *	@param modelNames Any number of ModelNames can be used to build a SuperModel, 
 *					which is a PyModel made of other PyModels ( see BigWorld.Model() ).
 *					Each modelName should be a complete .model filename, including 
 *					resource path.
 *	@param matrix		(optional) The transform Matrix for the resultant PyModelObstacle, 
 *					used to update (if dynamic) or set (if static) the SuperModel's 
 *					position.
 *	@param dynamic	(optional) If true, PyModelObstacle will be treated as dynamic, 
 *					otherwise static.  Defaults to false (static).
 */
PY_FACTORY( PyModelObstacle, BigWorld )

PY_SCRIPT_CONVERTERS( PyModelObstacle )


/**
 *	Constructor
 */
PyModelObstacle::PyModelObstacle( SuperModel * pSuperModel,
		MatrixProviderPtr pMatrix, bool dynamic, PyTypePlus * pType ) :
	PyObjectPlus( pType ),
	pSuperModel_( pSuperModel ),
	pMatrix_( pMatrix ),
	dynamic_( dynamic ),
	attached_( false )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( pSuperModel_ != NULL )
	{
		return;
	}

	pSuperModel_->localBoundingBox( localBoundingBox_ );
}

/**
 *	Destructor
 */
PyModelObstacle::~PyModelObstacle()
{
	BW_GUARD;
	delete pSuperModel_;
}


void PyModelObstacle::localBoundingBox( BoundingBox& bb, bool skinny ) const
{
	//Note we ignore the skinny parameter for now, since PyModelObstacles can't
	//have attachments.  But as soon as they can have attachments, skinny needs
	//to be taken into account.
	bb = localBoundingBox_;	
}


void PyModelObstacle::worldBoundingBox( BoundingBox& bb, const Matrix& world, bool skinny ) const
{
	//Note we ignore the skinny parameter for now, since PyModelObstacles can't
	//have attachments.  But as soon as they can have attachments, skinny needs
	//to be taken into account.
	bb = localBoundingBox_;
	bb.transformBy( worldTransform() );
}


void PyModelObstacle::attach()
{
	BW_GUARD;
	MF_ASSERT_DEV( !attached_ );
	attached_ = true;
}

void PyModelObstacle::detach()
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( attached_ )
	{
		return;
	}

	attached_ = false;

	obstacles_.clear();
}

void PyModelObstacle::enterWorld( ChunkItem * pItem )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( attached_ )
	{
		return;
	}

	Matrix m = this->worldTransform();

	// extract and create all the obstacles
	int nModels = pSuperModel_->nModels();
	for (int i = 0; i < nModels; i++)
	{
		ModelPtr pModel = pSuperModel_->topModel( i );
		const BSPTree * pBSPTree = pModel->decompose();
		if (pBSPTree != NULL)
		{
			obstacles_.push_back( new ChunkBSPObstacle(
				*pBSPTree, m, &pModel->boundingBox(), pItem ) );
		}
	}
}

void PyModelObstacle::leaveWorld()
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( attached_ )
	{
		return;
	}

	// have to clear these when leaving world or else references to
	// chunk item will never disappear and we will never be detached
	obstacles_.clear();
}

/**
 *	Tick our dyes
 */
void PyModelObstacle::tick( float dTime )
{
	BW_GUARD;
	for (Dyes::iterator it = dyes_.begin(); it != dyes_.end(); it++)
	{
		it->second->tick( dTime, 0 );
	}
}

/**
 *	Draw ourselves using our dyes if we have any
 */
void PyModelObstacle::draw( const Matrix & worldTransform )
{
	BW_GUARD;
	Moo::rc().push();
	Moo::rc().world( worldTransform );

	FashionVector fv;
	for (Dyes::iterator it = dyes_.begin(); it != dyes_.end(); it++)
		fv.push_back( it->second->fashion() );

	pSuperModel_->draw( &fv, 0 );
	Moo::rc().pop();
}


/**
 *	Get this model's world transform
 */
const Matrix & PyModelObstacle::worldTransform() const
{
	BW_GUARD;
	static Matrix transform;
	if (pMatrix_)
		pMatrix_->matrix( transform );
	else
		transform = Matrix::identity;
	return transform;
}



/**
 *	Python get attribute method
 */
PyObject * PyModelObstacle::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	// try to find the dye matter or other fashion named 'attr'
	Dyes::iterator found2 = dyes_.find( attr );
	if (found2 != dyes_.end())
	{
		PyObject * pPyDye = &*found2->second;
		Py_INCREF( pPyDye );
		return pPyDye;
	}
	SuperModelDyePtr pDye = pSuperModel_->getDye( attr, "" );
	if (pDye)	// some question as to whether we should do this...
	{	// but I want to present illusion that matters are always there
		SmartPointer<PyDye> pPyDye = new PyDye( pDye, attr, "Default" );
		dyes_.insert( std::make_pair( attr, pPyDye ) );
		return pPyDye.getObject(); // ref count comes from constructor
	}

	return this->PyObjectPlus::pyGetAttribute( attr );
}

/** 
 *	Python set attribute method
 */
int PyModelObstacle::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	// check if it's a PyFashion
	if (PyDye::Check( value ))
	{
		// some fashions must be copied when set into models
		PyDye * pPyFashion = static_cast<PyDye*>( value );

		// we always have to make a copy of the dye object here, because
		// it may have been created from a different model/supermodel.
		pPyFashion = pPyFashion->makeCopy( *pSuperModel_, attr );
		if (pPyFashion == NULL) return -1;	// makeCopy sets exception

		Dyes::iterator found = dyes_.find( attr );
		if (found != dyes_.end()) found->second->disowned();
		dyes_[attr] = pPyFashion;
		Py_DECREF( pPyFashion );
		return 0;
	}

	// try to find the dye matter named 'attr'
	if (PyString_Check( value ) || value == Py_None)
	{
		const char * valStr = (value == Py_None ? "" :
			PyString_AsString( value ) );

		SuperModelDyePtr pDye = pSuperModel_->getDye( attr, valStr );
		if (pDye)	// only NULL if no such matter
		{
			SmartPointer<PyDye> newPyDye(
				new PyDye( pDye, attr, valStr ), true );

			Dyes::iterator found = dyes_.find( attr );
			if (found != dyes_.end()) found->second->disowned();
			dyes_[attr] = newPyDye;
			return 0;
		}
	}

	// check if it's none and in our list of fashions (and not a dye)
	if (value == Py_None)
	{
		Dyes::iterator found = dyes_.find( attr );
		if (found != dyes_.end())
		{
			found->second->disowned();
			dyes_.erase( found );
			return 0;
		}
	}

	return this->PyObjectPlus::pySetAttribute( attr, value );
}


/**
 *	Get sources of this model
 */
PyObject * PyModelObstacle::pyGet_sources()
{
	BW_GUARD;
	int nModels = pSuperModel_->nModels();
	PyObject * pTuple = PyTuple_New( nModels );
	for (int i = 0; i < nModels; i++)
	{
		PyTuple_SetItem( pTuple, i, Script::getData(
			pSuperModel_->topModel(i)->resourceID() ) );
	}
	return pTuple;
}

/**
 *	Helper class for the bounding box of a PyModelObstacle
 */
class ModelObstacleBBMProv : public MatrixProvider
{
public:
	ModelObstacleBBMProv( PyModelObstacle * pModel ) :
		MatrixProvider( false, &s_type_ ),
		pModel_( pModel ) { }

	~ModelObstacleBBMProv() { }

	virtual void matrix( Matrix & m ) const
	{
		BW_GUARD;	
/*		if (!pModel_->isInWorld())
		{
			m = Matrix::identity;
			m[3][3] = 0.f;
			return;
		}*/

		BoundingBox bb;
		pModel_->localBoundingBox(bb,true);
		m = pModel_->worldTransform();

		Vector3 bbMin = m.applyPoint( bb.minBounds() );
		m.translation( bbMin );

		Vector3 bbMax = bb.maxBounds() - bb.minBounds();
		m[0] *= bbMax.x;
		m[1] *= bbMax.y;
		m[2] *= bbMax.z;
	}

private:
	SmartPointer<PyModelObstacle>	pModel_;
};

/**
 *	Get the bounding box in a matrix provider
 */
PyObject * PyModelObstacle::pyGet_bounds()
{
	BW_GUARD;
	return new ModelObstacleBBMProv( this );
}


/**
 *	Set whether or not this model is dynamic
 */
int PyModelObstacle::pySet_dynamic( PyObject * val )
{
	BW_GUARD;
	bool dynamic;
	int ret = Script::setData( val, dynamic, "PyModelObstacle.dynamic" );
	if (ret != 0) return ret;

	if (attached_)
	{
		PyErr_SetString( PyExc_TypeError, "PyModelObstacle.dynamic "
			"cannot be set when obstacle is attached." );
		return -1;
	}

	dynamic_ = dynamic;
	return 0;
}


/**
 *	Helper class for a node in a ModelObstacle.
 *	Currently can only get its matrix not attach stuff to it.
 */
class ModelObstacleNode : public MatrixProvider, public Aligned
{
public:
	ModelObstacleNode( PyModelObstacle * pModel, const Matrix & local ) :
		MatrixProvider( false, &s_type_ ),
		pModel_( pModel ),
		local_( local )
	{
	}

	~ModelObstacleNode() { }

	virtual void matrix( Matrix & m ) const
	{
		m.multiply( local_, pModel_->worldTransform() );
	}

private:
	// Note: This packs better if Matrix is second. sizeof=80 vs sizeof=96
	SmartPointer<PyModelObstacle>	pModel_;
	Matrix							local_;
};

/**
 *	Get the matrix of our node
 */
PyObject * PyModelObstacle::node( const std::string & nodeName )
{
	BW_GUARD;
	Moo::NodePtr pNode = pSuperModel_->findNode( nodeName );
	if (!pNode)
	{
		PyErr_Format( PyExc_ValueError, "PyModelObstacle.node(): "
			"No node named %s in this Model.", nodeName.c_str() );
		return false;
	}

	pSuperModel_->dressInDefault();

	return new ModelObstacleNode( this, pNode->worldTransform() );
}


/**
 *	Static factory method
 */
PyObject * PyModelObstacle::pyNew( PyObject * args )
{
	BW_GUARD;
	bool bad = false;
	int nargs = PyTuple_Size( args );
	int i;

	std::vector<std::string> modelNames;
	for (i = 0; i < nargs; i++)
	{
		PyObject * ana = PyTuple_GetItem( args, i );
		if (!PyString_Check( ana )) break;
		modelNames.push_back( PyString_AsString( ana ) );
	}
	bad |= modelNames.empty();

	MatrixProviderPtr pMatrix = NULL;
	if (i < nargs)
	{
		int ret = Script::setData( PyTuple_GetItem( args, i ), pMatrix );
		if (ret != 0) bad = true;
		i++;
	}

	bool dynamic = false;
	if (i < nargs)
	{
		int ret = Script::setData( PyTuple_GetItem( args, i ), dynamic );
		if (ret != 0) bad = true;
		i++;
	}

	if (bad || i < nargs)
	{
		PyErr_SetString( PyExc_TypeError, "PyModelObstacle() "
			"expects some model names optionally followed by "
			"a MatrixProvider and a boolean dynamic flag" );
		return NULL;
	}

	SuperModel * pSuperModel = new SuperModel( modelNames );
	if (pSuperModel->nModels() != modelNames.size())
	{
		PyErr_Format( PyExc_ValueError, "PyModelObstacle() "
			"only found %d out of %d models requested",
			pSuperModel->nModels(), modelNames.size() );

		delete pSuperModel;
		return NULL;
	}

	return new PyModelObstacle( pSuperModel, pMatrix, dynamic );
}

// chunk_dynamic_obstacle.cpp
