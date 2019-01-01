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
#include "worldeditor/world/items/editor_chunk_light.hpp"
#include "worldeditor/world/items/editor_chunk_substance.ipp"
#include "worldeditor/world/world_manager.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/editor/item_editor.hpp"
#include "worldeditor/editor/item_properties.hpp"
#include "appmgr/options.hpp"
#include "model/super_model.hpp"
#include "chunk/chunk_model.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/base_chunk_space.hpp"
#include "resmgr/string_provider.hpp"
#include "resmgr/resource_cache.hpp"
#include "moo/animation_manager.hpp"

#if UMBRA_ENABLE
#include <umbraModel.hpp>
#include <umbraObject.hpp>
#include "chunk/chunk_umbra.hpp"
#include "chunk/umbra_chunk_item.hpp"
#endif

static const uint32	GIZMO_INNER_COLOUR = 0xbf10ff10;
static const float	GIZMO_INNER_RADIUS = 2.0f;
static const uint32	GIZMO_OUTER_COLOUR = 0xbf1010ff;
static const float	GIZMO_OUTER_RADIUS = 4.0f;


namespace
{
	float adjustIntoRange( const float& value )
	{
		float result = value;
		while (result > GRID_RESOLUTION)
		{
			result -= GRID_RESOLUTION;
		}
		while (result < 0.0f)
		{
			result += GRID_RESOLUTION;
		}
		return result;
	}


	// return true if radius changed
	bool adjustRadius( const Vector3& posn, float& radius )
	{
		bool result = false;
		float posValue = adjustIntoRange(posn.x);

		if (posValue - radius < -GRID_RESOLUTION)
		{
			radius = posValue + GRID_RESOLUTION;
			result = true;
		}
		if (posValue + radius > GRID_RESOLUTION * 2)
		{
			radius = GRID_RESOLUTION * 2 - posValue;
			result = true;
		}
		posValue = adjustIntoRange(posn.z);
		if (posValue - radius < -GRID_RESOLUTION)
		{
			radius = posValue + GRID_RESOLUTION;
			result = true;
		}
		if (posValue + radius > GRID_RESOLUTION * 2)
		{
			radius = GRID_RESOLUTION * 2 - posValue;
			result = true;
		}
		return result;
	}
} // anonymous namespace


// -----------------------------------------------------------------------------
// Section: LightColourWrapper
// -----------------------------------------------------------------------------


/**
 *	This helper class gets and sets a light colour.
 *	Since all the lights have a 'colour' accessor, but there's no
 *	base class that collects them (for setting anyway), we use a template.
 */
template <class LT> class LightColourWrapper :
	public UndoableDataProxy<ColourProxy>
{
public:
	explicit LightColourWrapper( SmartPointer<LT> pItem ) :
		pItem_( pItem )
	{
	}

	virtual Moo::Colour EDCALL get() const
	{
		BW_GUARD;

		return pItem_->pLight()->colour();
	}

	virtual void EDCALL setTransient( Moo::Colour v )
	{
		BW_GUARD;

		pItem_->pLight()->colour( v );
	}

	virtual bool EDCALL setPermanent( Moo::Colour v )
	{
		BW_GUARD;

		// make it valid
		if (v.r < 0.f) v.r = 0.f;
		if (v.r > 1.f) v.r = 1.f;
		if (v.g < 0.f) v.g = 0.f;
		if (v.g > 1.f) v.g = 1.f;
		if (v.b < 0.f) v.b = 0.f;
		if (v.b > 1.f) v.b = 1.f;
		v.a = 1.f;

		// set it
		this->setTransient( v );

		// flag the chunk as having changed
		WorldManager::instance().changedChunk( pItem_->chunk() );
		pItem_->edPostModify();

		pItem_->markInfluencedChunks();

		// update its data section
		pItem_->edSave( pItem_->pOwnSect() );

		return true;
	}

	virtual std::string EDCALL opName()
	{
		BW_GUARD;

		return LocaliseUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/SET_COLOUR", pItem_->edDescription() );
	}

private:
	SmartPointer<LT>	pItem_;
};


template<class BaseLight>
void EditorChunkLight<BaseLight>::syncInit()
{
	#if UMBRA_ENABLE
	BW_GUARD;

	delete pUmbraDrawItem_;

	pUmbraDrawItem_ = NULL;

	// Grab the visibility bounding box
	if (this->model_)
	{
		BoundingBox bb = BoundingBox::s_insideOut_;
		bb=this->model_->boundingBox();	

		// Set up object transforms
		Matrix m = pChunk_->transform();
		m.preMultiply( transform_ );

		// Create the umbra chunk item
		UmbraChunkItem* pUmbraChunkItem = new UmbraChunkItem();
		pUmbraChunkItem->init( this, bb, m, pChunk_->getUmbraCell());
		pUmbraDrawItem_ = pUmbraChunkItem;

		this->updateUmbraLenders();
	}

	#endif
}


// -----------------------------------------------------------------------------
// Section: EditorChunkDirectionalLight
// -----------------------------------------------------------------------------

IMPLEMENT_CHUNK_ITEM( EditorChunkDirectionalLight, directionalLight, 1 )


/**
 *	Save our data to the given data section
 */
bool EditorChunkDirectionalLight::edSave( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!edCommonSave( pSection ))
		return false;

	Moo::Colour vcol = pLight_->colour();
	pSection->writeVector3( "colour",
		Vector3( vcol.r, vcol.g, vcol.b ) * 255.f );

	pSection->writeVector3( "direction", pLight_->direction() );

	pSection->writeBool( "dynamic", dynamicLight() );
	pSection->writeBool( "static", staticLight() );	
	pSection->writeBool( "specular", specularLight() );	

	pSection->writeFloat( "multiplier", pLight_->multiplier() );

	return true;
}


/**
 *	Add our properties to the given editor
 */
bool EditorChunkDirectionalLight::edEdit( class GeneralEditor & editor )
{
	BW_GUARD;

	if (!edCommonEdit( editor ))
	{
		return false;
	}

	editor.addProperty( new ColourProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/COLOUR" ),
		new LightColourWrapper<EditorChunkDirectionalLight>( this ) ) );

	MatrixProxy * pMP = new ChunkItemMatrix( this );
	editor.addProperty( new GenRotationProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/DIRECTION" ), pMP ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/STATIC" ), 
		new AccessorDataProxy< EditorChunkDirectionalLight, BoolProxy >(
			this, "static", 
			&EditorChunkDirectionalLight::staticLightGet, 
			&EditorChunkDirectionalLight::staticLightSet ) ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/DYNAMIC" ),
		new AccessorDataProxy< EditorChunkDirectionalLight, BoolProxy >(
			this, "dynamic", 
			&EditorChunkDirectionalLight::dynamicLightGet, 
			&EditorChunkDirectionalLight::dynamicLightSet ) ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/SPECULAR" ),
		new AccessorDataProxy< EditorChunkDirectionalLight, BoolProxy >(
			this, "specular", 
			&EditorChunkDirectionalLight::specularLightGet, 
			&EditorChunkDirectionalLight::specularLightSet ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/MULTIPLIER" ),
		new AccessorDataProxy<EditorChunkDirectionalLight,FloatProxy>(
			this, "multiplier", 
			&EditorChunkDirectionalLight::getMultiplier, 
			&EditorChunkDirectionalLight::setMultiplier ) ) );

	return true;
}


/**
 *	Get the current transform
 */
const Matrix & EditorChunkDirectionalLight::edTransform()
{
	BW_GUARD;

	/*Vector3 dir = pLight_->direction();
	Vector3 up( 0.f, 1.f, 0.f );
	if (fabs(up.dotProduct( dir )) > 0.9f) up = Vector3( 0.f, 0.f, 1.f );
	Vector3 across = dir.crossProduct( up );
	across.normalise();
	transform_[0] = across;
	transform_[1] = across.crossProduct(dir);
	transform_[2] = dir;*/
	if (pChunk_ != NULL)
	{
		const BoundingBox & bb = pChunk_->boundingBox();
		transform_.translation(
			pChunk_->transformInverse().applyPoint(
				(bb.maxBounds() + bb.minBounds()) / 2.f ) );
	}
	else
	{
		transform_.translation( Vector3::zero() );
	}

	return transform_;
}


/**
 *	Change our transform, temporarily or permanently
 */
bool EditorChunkDirectionalLight::edTransform(
	const Matrix & m, bool transient )
{
	BW_GUARD;

	// if this is only a temporary change, keep it in the same chunk
	if (transient)
	{
		transform_ = m;
		pLight_->direction( transform_.applyToUnitAxisVector(2) );
		pLight_->worldTransform( pChunk_->transform() );
		this->syncInit();
		return true;
	}

	// it's permanent, but we have no position so we're not changing chunks
	transform_ = m;
	pLight_->direction( transform_.applyToUnitAxisVector(2) );
	pLight_->worldTransform( pChunk_->transform() );

	WorldManager::instance().changedChunk( pChunk_ );

	// and move ourselves into the right chunk. we have to do this
	// even if it's the same chunk so the col scene gets recreated
	// (and other things methinks)
	Chunk * pChunk = pChunk_;
	pChunk->delStaticItem( this );
	pChunk->addStaticItem( this );

	// Recalculate static lighting for our chunk
	StaticLighting::markChunk( pChunk_ );
	this->syncInit();
	return true;
}

void EditorChunkDirectionalLight::loadModel()
{
	BW_GUARD;

	model_ = Model::get( "resources/models/directional_light.model" );
	ResourceCache::instance().addResource( model_ );
}

bool EditorChunkDirectionalLight::load( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!EditorChunkMooLight<ChunkDirectionalLight>::load( pSection ))
		return false;

	// Set the transform
	Vector3 dir = pLight_->direction();
	dir.normalise();

	Vector3 up( 0.f, 1.f, 0.f );
	if (fabsf( up.dotProduct( dir ) ) > 0.9f)
		up = Vector3( 0.f, 0.f, 1.f );

	Vector3 xaxis = up.crossProduct( dir );
	xaxis.normalise();

	transform_[1] = xaxis;
	transform_[0] = xaxis.crossProduct( dir );
	transform_[0].normalise();
	transform_[2] = dir;
	transform_.translation( Vector3(0,0,0) );

	pLight_->multiplier( pSection->readFloat( "multiplier", 1.f ) );

	return true;
}


// ----------------------------------------------------------------------------
// LightRadiusOperation
// ----------------------------------------------------------------------------

/**
* This class used to undo the light's radius during item moving
*/

template < class LT > class LightRadiusOperation : public UndoRedo::Operation, public Aligned
{
public:
	LightRadiusOperation( SmartPointer< LT > pItem, float oldInnerRadius, float oldOuterRadius ):
		UndoRedo::Operation( int(typeid(LightRadiusOperation).name()) ),
		pItem_( pItem ),
		oldInnerRadius_( oldInnerRadius ),
		oldOuterRadius_( oldOuterRadius )
	{
		BW_GUARD;

		addChunk( pItem->chunk() );
	}
private:
	virtual void undo()
	{
		BW_GUARD;

		// first add the current state of this block to the undo/redo list
		UndoRedo::instance().add( new LightRadiusOperation< LT >(
			pItem_, pItem_->pLight()->innerRadius(), pItem_->pLight()->outerRadius() ) );

		if (pItem_->chunk())
		{
			pItem_->pLight()->innerRadius( oldInnerRadius_ );
			pItem_->pLight()->outerRadius( oldOuterRadius_ );
		}
	}


	virtual bool iseq( const UndoRedo::Operation & oth ) const
	{
		return pItem_ ==
			static_cast<const LightRadiusOperation&>( oth ).pItem_;
	}


	SmartPointer<LT>	pItem_;
	float			oldInnerRadius_;
	float			oldOuterRadius_;

};

// ----------------------------------------------------------------------------
// ChunkLightMatrix
// ----------------------------------------------------------------------------


/**
* This class used to replace ChunkItemMatrix during items moving.
* Registering undo-operation of light's radius in ChunkItemMatrix::commitState().
*/
template <class LT> class ChunkLightMatrix : public ChunkItemMatrix
{
public:
	ChunkLightMatrix( SmartPointer<LT> pItem ) : 
		ChunkItemMatrix( ChunkItemPtr( pItem ) ),
		pItem_(pItem),
		originInnerRadius_( pItem->pLight()->innerRadius() ),
		originOuterRadius_( pItem->pLight()->outerRadius() )
	{
	}


	void EDCALL recordState()
	{
		BW_GUARD;

		originInnerRadius_ = pItem_->pLight()->innerRadius();
		originOuterRadius_ = pItem_->pLight()->outerRadius();
		ChunkItemMatrix::recordState();
	}


	bool EDCALL commitState( bool revertToRecord, bool addUndoBarrier )
	{
		BW_GUARD;

		if (!ChunkItemMatrix::haveRecorded_)
		{
			recordState();
		}

		if (!revertToRecord)
		{
			UndoRedo::instance().add(
				new LightRadiusOperation< LT >( pItem_, originInnerRadius_, originOuterRadius_ ) );
		}

		return ChunkItemMatrix::commitState( revertToRecord, addUndoBarrier );

	}


private:
	SmartPointer< LT >	pItem_;
	float				originInnerRadius_;
	float				originOuterRadius_;

};

// -----------------------------------------------------------------------------
// Section: EditorChunkOmniLight
// -----------------------------------------------------------------------------

IMPLEMENT_CHUNK_ITEM( EditorChunkOmniLight, omniLight, 1 )

bool EditorChunkOmniLight::load( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!EditorChunkPhysicalMooLight<ChunkOmniLight>::load( pSection ))
		return false;

	pLight_->multiplier( pSection->readFloat( "multiplier", 1.f ) );

	return true;
}

/**
 *	Save our data to the given data section
 */
bool EditorChunkOmniLight::edSave( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!edCommonSave( pSection ))
		return false;

	Moo::Colour vcol = pLight_->colour();
	pSection->writeVector3( "colour",
		Vector3( vcol.r, vcol.g, vcol.b ) * 255.f );

	pSection->writeVector3( "position", pLight_->position() );
	pSection->writeFloat( "innerRadius", pLight_->innerRadius() );
	pSection->writeFloat( "outerRadius", pLight_->outerRadius() );

	pSection->writeBool( "dynamic", dynamicLight() );
	pSection->writeBool( "static", staticLight() );
	pSection->writeBool( "specular", specularLight() );	
	pSection->writeInt( "priority", pLight_->priority() );

	pSection->writeFloat( "multiplier", pLight_->multiplier() );

	return true;
}


/**
 *	This helper class gets and sets a radius. It works on any chunk item
 *	that has a pLight member which returns an object with innerRadius
 *	and outerRadius.
 */
template <class LT> class LightRadiusWrapper :
	public UndoableDataProxy<FloatProxy>
{
public:
	LightRadiusWrapper( SmartPointer<LT> pItem, bool isOuter ) :
		pItem_( pItem ),
		isOuter_( isOuter )
	{
	}

	virtual float EDCALL get() const
	{
		BW_GUARD;

		if (isOuter_)
		{
			return pItem_->pLight()->outerRadius();
		}
		else
		{
			return pItem_->pLight()->innerRadius();
		}
	}

	virtual void EDCALL setTransient( float f )
	{
		BW_GUARD;

		// check the radius range not exceed 2 chunks distance
		Vector3 posn = pItem_->pLight()->position();

		adjustRadius( posn, f );

		if (isOuter_)
		{
			pItem_->pLight()->outerRadius( f );
		}
		else
		{
			pItem_->pLight()->innerRadius( f );
		}

		// update world inner and outer
		Matrix world = pItem_->chunk()->transform();
		pItem_->pLight()->worldTransform( world );
	}

	virtual bool EDCALL setPermanent( float f )
	{
		BW_GUARD;

		// make sure it's valid
		if (f < 0.f) return false;

		// mark old chucks for recalc
		StaticLighting::markChunks( pItem_->chunk(), pItem_->pLight() );

		// toss the chunk to refresh the light states
		Chunk* chunk = pItem_->chunk();
		chunk->delStaticItem( pItem_ );
		chunk->addStaticItem( pItem_ );

		// set it
		this->setTransient( f );

		// flag the chunk as having changed
		WorldManager::instance().changedChunk( pItem_->chunk() );
		pItem_->edPostModify();

		// mark new chucks for recalc
		StaticLighting::markChunks( pItem_->chunk(), pItem_->pLight() );

		// update its data section
		pItem_->edSave( pItem_->pOwnSect() );

		return true;
	}

	virtual std::string EDCALL opName()
	{
		BW_GUARD;

		if( isOuter_ )
		{
			return LocaliseUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/SET_OUTER_RADIUS",
				pItem_->edDescription() );
		}
		return LocaliseUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/SET_INNER_RADIUS",
			pItem_->edDescription() );
	}

private:
	SmartPointer<LT>	pItem_;
	bool				isOuter_;
};



/**
 *	Add our properties to the given editor
 */
bool EditorChunkOmniLight::edEdit( class GeneralEditor & editor )
{
	BW_GUARD;

	if (this->edFrozen())
		return false;

	if (!edCommonEdit( editor ))
	{
		return false;
	}

	editor.addProperty( new ColourProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/COLOUR" ),
		new LightColourWrapper<EditorChunkOmniLight>( this ) ) );

	MatrixProxy * pMP = new ChunkLightMatrix<EditorChunkOmniLight>( this );
	editor.addProperty( new GenPositionProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/POSITION" ), pMP ) );

	editor.addProperty( new GenRadiusProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/INNER_RADIUS" ),
		new LightRadiusWrapper<EditorChunkOmniLight>( this, false ), pMP,
		GIZMO_INNER_COLOUR, GIZMO_INNER_RADIUS ) );
	editor.addProperty( new GenRadiusProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/OUTER_RADIUS" ),
		new LightRadiusWrapper<EditorChunkOmniLight>( this, true ), pMP,
		GIZMO_OUTER_COLOUR, GIZMO_OUTER_RADIUS ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/STATIC" ),
		new AccessorDataProxy< EditorChunkOmniLight, BoolProxy >(
			this, "static", 
			&EditorChunkOmniLight::staticLightGet, 
			&EditorChunkOmniLight::staticLightSet ) ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/DYNAMIC" ),
		new AccessorDataProxy< EditorChunkOmniLight, BoolProxy >(
			this, "dynamic", 
			&EditorChunkOmniLight::dynamicLightGet, 
			&EditorChunkOmniLight::dynamicLightSet ) ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/SPECULAR" ),
		new AccessorDataProxy< EditorChunkOmniLight, BoolProxy >(
			this, "specular", 
			&EditorChunkOmniLight::specularLightGet, 
			&EditorChunkOmniLight::specularLightSet ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/MULTIPLIER" ),
		new AccessorDataProxy<EditorChunkOmniLight,FloatProxy>(
			this, "multiplier", 
			&EditorChunkOmniLight::getMultiplier, 
			&EditorChunkOmniLight::setMultiplier ) ) );

	editor.addProperty( new GenIntProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/PRIORITY" ),
		new AccessorDataProxy<EditorChunkOmniLight, IntProxy>(
			this, "priority", 
			&EditorChunkOmniLight::getPriority, 
			&EditorChunkOmniLight::setPriority ) ) );

	return true;
}


/**
 *	Get the current transform
 */
const Matrix & EditorChunkOmniLight::edTransform()
{
	BW_GUARD;

	transform_.setIdentity();
	transform_.translation( pLight_->position() );

	return transform_;
}

/*
template<class LightType>
bool influencesReadOnlyChunk( Chunk* srcChunk, Vector3 atPosition, LightType light,
	std::set<Chunk*>& searchedChunks = std::set<Chunk*>(),
	int currentDepth = 0 )
{
	searchedChunks.insert( srcChunk );

	if (!EditorChunkCache::instance( *srcChunk ).edIsLocked())
		return true;

	// Stop if we've reached the maximum portal traversal depth
	if (currentDepth == StaticLighting::STATIC_LIGHT_PORTAL_DEPTH)
		return false;

	for (Chunk::piterator pit = srcChunk->pbegin(); pit != srcChunk->pend(); pit++)
	{
		if (!pit->hasChunk() || !pit->pChunk->isBound())
			continue;

		// Don't mark outside chunks
		//if (pit->pChunk->isOutsideChunk())
		//	continue;

		// ^^^ TEMP! commented out for testing only

		// We've already marked it, skip
		if ( searchedChunks.find( pit->pChunk ) != searchedChunks.end() )
			continue;

		if ( !pit->pChunk->boundingBox().intersects( atPosition, light->worldOuterRadius() ) )
			continue;

		if ( influencesReadOnlyChunk( pit->pChunk, atPosition, light, searchedChunks, currentDepth + 1 ) )
			return true;
	}

	return false;
}
*/



/**
 *	Change our transform, temporarily or permanently
 */
bool EditorChunkOmniLight::edTransform( const Matrix & m, bool transient )
{
	BW_GUARD;

	Vector3 opos = pLight_->position();
	float oradius = pLight_->outerRadius();

	// it's permanent, so find out where we belong now
	Chunk * pOldChunk = pChunk_;

	Vector3 posn = m.applyToOrigin();
	Chunk * pNewChunk = this->edDropChunk( posn );
	if (pNewChunk == NULL) return false;

	// check light's inner radius range not exceed 2 chunks after item moved
	float f = pLight_->innerRadius();

	if (adjustRadius( posn, f ))
	{
		pLight_->innerRadius( f );
	}

	// check light's outer radius range not exceed 2 chunks after item moved
	f = pLight_->outerRadius();

	if (adjustRadius( posn, f ))
	{
		pLight_->outerRadius( f );
	}

	// if this is only a temporary change, keep it in the same chunk
	if (transient)
	{
		transform_ = m;
		pLight_->position( transform_.applyToOrigin() );
		pLight_->worldTransform( pChunk_->transform() );
		this->syncInit();

		// Make sure the light gets pushed around the caches as neccessary.
		if ( chunk() != NULL )
		{
			ChunkLightCache::instance( *chunk() ).moveOmni(
					pLight_, opos, oradius, true
				);
		}

		return true;
	}

	// make sure the chunks aren't readonly
	if (!EditorChunkCache::instance( *pOldChunk ).edIsWriteable() 
		|| !EditorChunkCache::instance( *pNewChunk ).edIsWriteable())
		return false;

	// Ok, harder question: if we influence any non locked chunks in our old or new positions,
	// we can't move (+ do the same thing for edEdit, and use readonly props in the same case)

	/*
	if (influencesReadOnlyChunk( pOldChunk, pLight_->worldPosition(), pLight_ ))
		return false;
	if (influencesReadOnlyChunk( pNewChunk, pNewChunk->transform().applyPoint( pLight_->position() ), pLight_ ))
		return false;
	*/

	// ^^^ Ignored the problem by making static lights only traverse a single
	// portal, thus our 1 grid square barrier will take care of everything


	// Update static lighting in the old location
	markInfluencedChunks();


	// ok, accept the transform change then
	transform_.multiply( m, pOldChunk->transform() );
	transform_.postMultiply( pNewChunk->transformInverse() );
	pLight_->position( transform_.applyToOrigin() );
	pLight_->worldTransform( pNewChunk->transform() );

	// note that both affected chunks have seen changes
	WorldManager::instance().changedChunk( pOldChunk );
	WorldManager::instance().changedChunk( pNewChunk );

	edMove( pOldChunk, pNewChunk );

	// Update static lighting in the new location
	markInfluencedChunks();
	this->syncInit();

	return true;
}


void EditorChunkOmniLight::loadModel()
{
	BW_GUARD;

	ModelPtr model;
	ModelPtr modelSmall;
	if (staticLight())
	{
		model = Model::get( "resources/models/static.model" );
		modelSmall = Model::get( "resources/models/static_small.model" );
		ResourceCache::instance().addResource( model );
		ResourceCache::instance().addResource( modelSmall );
	}
	else if (dynamicLight())
	{
		model = Model::get( "resources/models/dynamic.model" );
		modelSmall = Model::get( "resources/models/dynamic_small.model" );
		ResourceCache::instance().addResource( model );
		ResourceCache::instance().addResource( modelSmall );
	}
	else if (specularLight())
	{
		model = Model::get( "resources/models/dynamic.model" );
		modelSmall = Model::get( "resources/models/dynamic_small.model" );
		ResourceCache::instance().addResource( model );
		ResourceCache::instance().addResource( modelSmall );
	}
	if( model_ != model )
	{
		if( pChunk_ )
			ChunkModelObstacle::instance( *pChunk_ ).delObstacles( this );
		model_ = model;
		if( pChunk_ )
			this->addAsObstacle();
	}
	if( modelSmall_ != modelSmall )
	{
		if( pChunk_ )
			ChunkModelObstacle::instance( *pChunk_ ).delObstacles( this );
		modelSmall_ = modelSmall;
		if( pChunk_ )
			this->addAsObstacle();
	}
}


bool EditorChunkOmniLight::usesLargeProxy() const
{
	BW_GUARD;

	if (staticLight())
	{
		return OptionsLightProxies::staticLargeVisible();
	}
	else if (dynamicLight())
	{
		return OptionsLightProxies::dynamicLargeVisible();
	}
	else if (specularLight())
	{
		return OptionsLightProxies::specularLargeVisible();
	}

	return false;
}


// -----------------------------------------------------------------------------
// Section: EditorChunkSpotLight
// -----------------------------------------------------------------------------

IMPLEMENT_CHUNK_ITEM( EditorChunkSpotLight, spotLight, 1 )


bool EditorChunkSpotLight::load( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!EditorChunkPhysicalMooLight<ChunkSpotLight>::load( pSection ))
		return false;

	// Set the transform
	Vector3 dir = pLight_->direction();
	dir.normalise();

	Vector3 up( 0.f, 1.f, 0.f );
	if (fabsf( up.dotProduct( dir ) ) > 0.9f)
		up = Vector3( 1.f, 0.f, 0.f );

	Vector3 xaxis = up.crossProduct( dir );
	xaxis.normalise();

	transform_[0] = xaxis;
	transform_[1] = dir.crossProduct( xaxis );
	transform_[2] = dir;
	transform_[2].normalise();
	transform_.translation( pLight_->position() );

	pLight_->multiplier( pSection->readFloat( "multiplier", 1.f ) );

	return true;
}



/**
 *	Save our data to the given data section
 */
bool EditorChunkSpotLight::edSave( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!edCommonSave( pSection ))
		return false;

	Moo::Colour vcol = pLight_->colour();
	pSection->writeVector3( "colour",
		Vector3( vcol.r, vcol.g, vcol.b ) * 255.f );

	pSection->writeVector3( "position", pLight_->position() );
	pSection->writeVector3( "direction", pLight_->direction() );
	pSection->writeFloat( "innerRadius", pLight_->innerRadius() );
	pSection->writeFloat( "outerRadius", pLight_->outerRadius() );
	pSection->writeFloat( "cosConeAngle", pLight_->cosConeAngle() );

	pSection->writeBool( "dynamic", dynamicLight() );
	pSection->writeBool( "static", staticLight() );
	pSection->writeBool( "specular", specularLight() );	

	pSection->writeFloat( "multiplier", pLight_->multiplier() );
	pSection->writeInt( "priority", pLight_->priority() );

	pSection->writeFloat( "editorOnly/version", 2.0f );

	return true;
}


/**
 *	This class is the data under a spot light's angle property.
 */
class SLAngleWrapper : public UndoableDataProxy<FloatProxy>
{
public:
	explicit SLAngleWrapper( SmartPointer<EditorChunkSpotLight> pItem ) :
		pItem_( pItem )
	{
	}

	virtual float EDCALL get() const
	{
		BW_GUARD;
		return RAD_TO_DEG( acosf( pItem_->pLight()->cosConeAngle() ) * 2.f);

	}

	virtual void EDCALL setTransient( float f )
	{
		BW_GUARD;
		pItem_->pLight()->cosConeAngle( cosf( DEG_TO_RAD(f) / 2.f ) );
	}

	virtual bool EDCALL setPermanent( float f )
	{
		BW_GUARD;

		// complain if it's invalid
		if (f < 0.01f || f > 180.f - 0.01f) return false;

		// set it
		this->setTransient( f );

		// flag the chunk as having changed
		WorldManager::instance().changedChunk( pItem_->chunk() );
		pItem_->edPostModify();

		pItem_->markInfluencedChunks();

		// update its data section
		pItem_->edSave( pItem_->pOwnSect() );

		return true;
	}

	virtual std::string EDCALL opName()
	{
		BW_GUARD;

		return LocaliseUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/SET_CONE_ANGLE",
			pItem_->edDescription() );
	}

private:
	SmartPointer<EditorChunkSpotLight>	pItem_;
};



/**
 *	Add our properties to the given editor
 */
bool EditorChunkSpotLight::edEdit( class GeneralEditor & editor )
{
	BW_GUARD;

	if (this->edFrozen())
		return false;

	if (!edCommonEdit( editor ))
	{
		return false;
	}

	editor.addProperty( new ColourProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/COLOUR" ),
		new LightColourWrapper<EditorChunkSpotLight>( this ) ) );

	MatrixProxy * pMP = new ChunkLightMatrix<EditorChunkSpotLight>( this );
	editor.addProperty( new GenPositionProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/POSITION" ), pMP ) );
	editor.addProperty( new GenRotationProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/DIRECTION" ), pMP ) );

	editor.addProperty( new GenRadiusProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/INNER_RADIUS" ),
		new LightRadiusWrapper<EditorChunkSpotLight>( this, false ), pMP,
		GIZMO_INNER_COLOUR, GIZMO_INNER_RADIUS ) );
	editor.addProperty( new GenRadiusProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/OUTER_RADIUS" ),
		new LightRadiusWrapper<EditorChunkSpotLight>( this, true ), pMP,
		GIZMO_OUTER_COLOUR, GIZMO_OUTER_RADIUS ) );

	editor.addProperty( new AngleProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/CONE_ANGLE" ),
		new SLAngleWrapper( this ), pMP ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/STATIC" ),
		new AccessorDataProxy< EditorChunkSpotLight, BoolProxy >(
			this, "static", 
			&EditorChunkSpotLight::staticLightGet, 
			&EditorChunkSpotLight::staticLightSet ) ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/DYNAMIC" ),
		new AccessorDataProxy< EditorChunkSpotLight, BoolProxy >(
			this, "dynamic", 
			&EditorChunkSpotLight::dynamicLightGet, 
			&EditorChunkSpotLight::dynamicLightSet ) ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/SPECULAR" ),
		new AccessorDataProxy< EditorChunkSpotLight, BoolProxy >(
			this, "specular", 
			&EditorChunkSpotLight::specularLightGet, 
			&EditorChunkSpotLight::specularLightSet ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/MULTIPLIER" ),
		new AccessorDataProxy<EditorChunkSpotLight,FloatProxy>(
			this, "multiplier", 
			&EditorChunkSpotLight::getMultiplier, 
			&EditorChunkSpotLight::setMultiplier ) ) );

	editor.addProperty( new GenIntProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/PRIORITY" ),
		new AccessorDataProxy<EditorChunkSpotLight, IntProxy>(
			this, "priority", 
			&EditorChunkSpotLight::getPriority, 
			&EditorChunkSpotLight::setPriority ) ) );


	return true;
}


/**
 *	Get the current transform
 */
const Matrix & EditorChunkSpotLight::edTransform()
{
	return transform_;
}


/**
 *	Change our transform, temporarily or permanently
 */
bool EditorChunkSpotLight::edTransform( const Matrix & m, bool transient )
{
	BW_GUARD;

	Vector3 opos = pLight_->position();
	float oradius = pLight_->outerRadius();

	Vector3 posn = m.applyToOrigin();

	// it's permanent, so find out where we belong now
	Chunk * pOldChunk = pChunk_;
	Chunk * pNewChunk = this->edDropChunk( posn );
	if (pNewChunk == NULL) return false;

	// check light's inner radius range not exceed 2 chunks after item moved
	float f = pLight_->innerRadius();

	if (adjustRadius( posn, f ))
	{
		pLight_->innerRadius( f );
	}

	// check light's outer radius range not exceed 2 chunks after item moved
	f = pLight_->outerRadius();

	if (adjustRadius( posn, f ))
	{
		pLight_->outerRadius( f );
	}

	// if this is only a temporary change, keep it in the same chunk
	if (transient)
	{
		transform_ = m;
		pLight_->position( transform_.applyToOrigin() );
		pLight_->direction( transform_.applyToUnitAxisVector( 2 ) );
		pLight_->worldTransform( pChunk_->transform() );


		this->syncInit();

		// Make sure the light gets pushed around the caches as neccessary.
		if ( chunk() != NULL )
		{
			ChunkLightCache::instance( *chunk() ).moveSpot(
					pLight_, opos, oradius, true
				);
		}
		return true;
	}

	// make sure the chunks aren't readonly
	if (!EditorChunkCache::instance( *pOldChunk ).edIsWriteable() 
		|| !EditorChunkCache::instance( *pNewChunk ).edIsWriteable())
		return false;

	// Update static lighting in the old location
	markInfluencedChunks();

	// ok, accept the transform change then
	transform_.multiply( m, pOldChunk->transform() );
	transform_.postMultiply( pNewChunk->transformInverse() );
	pLight_->position( transform_.applyToOrigin() );
	pLight_->direction( transform_.applyToUnitAxisVector( 2 ) );
	pLight_->worldTransform( pNewChunk->transform() );

	// note that both affected chunks have seen changes
	WorldManager::instance().changedChunk( pOldChunk );
	WorldManager::instance().changedChunk( pNewChunk );

	edMove( pOldChunk, pNewChunk );

	// Update static lighting in the new location
	markInfluencedChunks();
	this->syncInit();
	return true;
}


void EditorChunkSpotLight::loadModel()
{
	BW_GUARD;

	model_ = Model::get( "resources/models/spot_light.model" );
	modelSmall_ = Model::get( "resources/models/spot_light_small.model" );
	ResourceCache::instance().addResource( model_ );
	ResourceCache::instance().addResource( modelSmall_ );
}

bool EditorChunkSpotLight::edShouldDraw()
{
	BW_GUARD;

	if (!EditorChunkPhysicalMooLight<ChunkSpotLight>::edShouldDraw())
	{
		return false;
	}

	if (!OptionsLightProxies::visible())
		return false;

	if (OptionsLightProxies::spotVisible())
		return true;

	return false;
}


bool EditorChunkSpotLight::usesLargeProxy() const
{
	BW_GUARD;

	return OptionsLightProxies::spotLargeVisible();
}


// -----------------------------------------------------------------------------
// Section: PulseColourWrapper
// -----------------------------------------------------------------------------

class ChunkPulseLightFrameOperation : public UndoRedo::Operation
{
	EditorChunkPulseLight& pulseLight_;
	EditorChunkPulseLight::Frames frames_;
public:
	ChunkPulseLightFrameOperation( EditorChunkPulseLight& pulseLight )
		: pulseLight_( pulseLight ), frames_( pulseLight.getFrames() ),
		UndoRedo::Operation( (int)(typeid(EditorChunkPulseLight).name()) )
	{}

private:

	virtual void undo()
	{
		BW_GUARD;

		UndoRedo::instance().add( new ChunkPulseLightFrameOperation( pulseLight_ ) );

		PropertyModifyGuard propertyModifyGuard;

		pulseLight_.setFrames( frames_ );
	}

	virtual bool iseq( const UndoRedo::Operation & oth ) const
	{		
		return false;
	}
};


class ChunkPulseLightFrameProxy : public Vector2Proxy
{
	Vector2& value_;
	Vector2 transient_;
	EditorChunkPulseLight& pulseLight_;
public:
	ChunkPulseLightFrameProxy( Vector2& value, EditorChunkPulseLight& pulseLight )
		: value_( value ), transient_( value ), pulseLight_( pulseLight )
	{}
	virtual Vector2 EDCALL get() const
	{
		return transient_;
	}
	virtual void EDCALL set( Vector2 value, bool transient, bool addBarrier )
	{
		BW_GUARD;

		transient_ = value;

		if (!transient)
		{
			UndoRedo::instance().add( new ChunkPulseLightFrameOperation( pulseLight_ ) );

			if (addBarrier)
			{
				UndoRedo::instance().barrier(
					LocaliseUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/MODIFY_PULSE_ANIMATION" ),
					false );
			}

			value_ = value;
			pulseLight_.onFrameChanged();
		}
	}
};


class ChunkPulseLightFramesProxy : public ArrayProxy
{
	typedef std::vector<GeneralProperty*> Properties;
	Properties properties_;
	EditorChunkPulseLight& pulseLight_;
public:
	ChunkPulseLightFramesProxy( EditorChunkPulseLight& pulseLight ) :
		pulseLight_( pulseLight )
	{}

	~ChunkPulseLightFramesProxy()
	{}

	virtual void elect( GeneralProperty* parent )
	{
		BW_GUARD;

		EditorChunkPulseLight::Frames& frames = pulseLight_.getFrames();

		static const int MAX_PROPNAME = 256;
		char propName[ MAX_PROPNAME + 1 ];
		int idx = 1;
		for (EditorChunkPulseLight::Frames::iterator iter = frames.begin();
			iter != frames.end(); ++iter)
		{
			bw_snprintf( propName, MAX_PROPNAME, "frame%03d", idx++ );
			propName[ MAX_PROPNAME ] = '\0';
			properties_.push_back(
				new Vector2Property( propName,
					new ChunkPulseLightFrameProxy( *iter, pulseLight_ ) ) );
			properties_.back()->setGroup(
				parent->getGroup() + bw_utf8tow( parent->name() ) );
			properties_.back()->elect();
		}
	}

	virtual void expel( GeneralProperty* parent )
	{
		BW_GUARD;

		for (Properties::iterator iter = properties_.begin();
			iter != properties_.end(); ++iter)
		{
			(*iter)->expel();
			(*iter)->deleteSelf();
		}

		properties_.clear();
	}

	virtual void select( GeneralProperty* )
	{}

	virtual bool addItem()
	{
		BW_GUARD;

		EditorChunkPulseLight::Frames& frames = pulseLight_.getFrames();

		UndoRedo::instance().add( new ChunkPulseLightFrameOperation( pulseLight_ ) );

		UndoRedo::instance().barrier(
			LocaliseUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/MODIFY_PULSE_ANIMATION" ),
			false );

		if (frames.empty())
		{
			frames.push_back( Vector2( 0.f, 0.f ) );
		}
		else
		{
			frames.push_back( Vector2( frames.rbegin()->x + 1.f, 1.f ) );
		}

		pulseLight_.onFrameChanged();

		return true;
	}

	virtual void delItem( int index )
	{
		BW_GUARD;

		EditorChunkPulseLight::Frames& frames = pulseLight_.getFrames();

		if (index >= 0 && index < (int)frames.size())
		{
			UndoRedo::instance().add( new ChunkPulseLightFrameOperation( pulseLight_ ) );
			UndoRedo::instance().barrier(
				LocaliseUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/MODIFY_PULSE_ANIMATION" ),
				false );
			frames.erase( frames.begin() + index );
			pulseLight_.onFrameChanged();
		}
	}

	virtual bool delItems()
	{
		BW_GUARD;

		EditorChunkPulseLight::Frames& frames = pulseLight_.getFrames();

		if (!frames.empty())
		{
			UndoRedo::instance().add( new ChunkPulseLightFrameOperation( pulseLight_ ) );
			UndoRedo::instance().barrier(
				LocaliseUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/MODIFY_PULSE_ANIMATION" ),
				false );
			frames.clear();
			pulseLight_.onFrameChanged();
		}

		return true;
	}
};


/**
 *	This helper class gets and sets a light colour on a pulse light.
 */
class PulseColourWrapper :
	public UndoableDataProxy<ColourProxy>
{
public:
	explicit PulseColourWrapper( SmartPointer<EditorChunkPulseLight> pItem ) :
		pItem_( pItem )
	{
	}

	virtual Moo::Colour EDCALL get() const
	{
		BW_GUARD;

		return pItem_->colour();
	}

	virtual void EDCALL setTransient( Moo::Colour v )
	{
		BW_GUARD;

		pItem_->colour( v );
	}

	virtual bool EDCALL setPermanent( Moo::Colour v )
	{
		BW_GUARD;

		// make it valid
		if (v.r < 0.f) v.r = 0.f;
		if (v.r > 1.f) v.r = 1.f;
		if (v.g < 0.f) v.g = 0.f;
		if (v.g > 1.f) v.g = 1.f;
		if (v.b < 0.f) v.b = 0.f;
		if (v.b > 1.f) v.b = 1.f;
		v.a = 1.f;

		// set it
		this->setTransient( v );

		// flag the chunk as having changed
		WorldManager::instance().changedChunk( pItem_->chunk() );
		pItem_->edPostModify();

		pItem_->markInfluencedChunks();

		// update its data section
		pItem_->edSave( pItem_->pOwnSect() );

		return true;
	}

	virtual std::string EDCALL opName()
	{
		BW_GUARD;

		return LocaliseUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/SET_COLOUR",
			pItem_->edDescription() );
	}

private:
	SmartPointer<EditorChunkPulseLight>	pItem_;
};

// -----------------------------------------------------------------------------
// Section: EditorChunkPulseLight
// -----------------------------------------------------------------------------

IMPLEMENT_CHUNK_ITEM( EditorChunkPulseLight, pulseLight, 1 )

bool EditorChunkPulseLight::load( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!EditorChunkPhysicalMooLight<ChunkPulseLight>::load( pSection ))
		return false;

	this->staticLight( false );

	pLight_->multiplier( pSection->readFloat( "multiplier", 1.f ) );

	if (!pSection->openSection( "timeScale"))
	{// load the legacy animations
		pSection = BWResource::openSection( "system/data/pulse_light.xml" );
	}

	frames_.clear();

	if (pSection)
	{
		timeScale_ = pSection->readFloat( "timeScale", 1.f );
		duration_ = pSection->readFloat( "duration", 0.f );
		animation_ = pSection->readString( "animation",
			"system/animation/lightnoise.animation" );

		pSection->readVector2s( "frame", frames_ );
	}
	else
	{
		timeScale_ = 1.f;
		duration_ = 0.f;
	}

	return true;
}

/**
 *	Save our data to the given data section
 */
bool EditorChunkPulseLight::edSave( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!edCommonSave( pSection ))
		return false;

	Moo::Colour vcol = colour();
	pSection->writeVector3( "colour",
		Vector3( vcol.r, vcol.g, vcol.b ) * 255.f );

	pSection->writeVector3( "position", position_ );
	pSection->writeFloat( "innerRadius", pLight_->innerRadius() );
	pSection->writeFloat( "outerRadius", pLight_->outerRadius() );

	pSection->writeFloat( "multiplier", pLight_->multiplier() );

	pSection->writeFloat( "timeScale", timeScale_ );
	pSection->writeFloat( "duration", duration_ );
	pSection->writeString( "animation", animation_ );
	pSection->writeInt( "priority", pLight_->priority() );

	pSection->deleteSections( "frame" );
	pSection->writeVector2s( "frame", frames_ );

	return true;
}


/**
 *	Add our properties to the given editor
 */
bool EditorChunkPulseLight::edEdit( class GeneralEditor & editor )
{
	BW_GUARD;

	if (this->edFrozen())
		return false;

	if (!edCommonEdit( editor ))
	{
		return false;
	}

	editor.addProperty( new ColourProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/COLOUR" ),
		new PulseColourWrapper( this ) ) );

	MatrixProxy * pMP = new ChunkLightMatrix<EditorChunkPulseLight>( this );
	editor.addProperty( new GenPositionProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/POSITION" ), pMP ) );

	editor.addProperty( new GenRadiusProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/INNER_RADIUS" ),
		new LightRadiusWrapper<EditorChunkPulseLight>( this, false ), pMP,
		GIZMO_INNER_COLOUR, GIZMO_INNER_RADIUS ) );
	editor.addProperty( new GenRadiusProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/OUTER_RADIUS" ),
		new LightRadiusWrapper<EditorChunkPulseLight>( this, true ), pMP,
		GIZMO_OUTER_COLOUR, GIZMO_OUTER_RADIUS ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/MULTIPLIER" ),
		new AccessorDataProxy<EditorChunkPulseLight,FloatProxy>(
			this, "multiplier", 
			&EditorChunkPulseLight::getMultiplier, 
			&EditorChunkPulseLight::setMultiplier ) ) );

	editor.addProperty( new GenIntProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/PRIORITY" ),
		new AccessorDataProxy<EditorChunkPulseLight, IntProxy>(
			this, "priority", 
			&EditorChunkPulseLight::getPriority, 
			&EditorChunkPulseLight::setPriority ) ) );

	editor.addProperty( new GenFloatProperty(
		"timeScale",
		new AccessorDataProxy<EditorChunkPulseLight,FloatProxy>(
			this, "timeScale", 
			&EditorChunkPulseLight::getTimeScale, 
			&EditorChunkPulseLight::setTimeScale ) ) );

	editor.addProperty( new GenFloatProperty(
		"duration",
		new AccessorDataProxy<EditorChunkPulseLight,FloatProxy>(
			this, "duration", 
			&EditorChunkPulseLight::getDuration, 
			&EditorChunkPulseLight::setDuration ) ) );

	editor.addProperty( new TextProperty(
		"animation",
		new AccessorDataProxy<EditorChunkPulseLight,StringProxy>(
			this, "animation", 
			&EditorChunkPulseLight::getAnimation, 
			&EditorChunkPulseLight::setAnimation) ) );

	editor.addProperty( new ArrayProperty( "frames",
		new ChunkPulseLightFramesProxy( *this ) ) );

	return true;
}


/**
 *	Get the current transform
 */
const Matrix & EditorChunkPulseLight::edTransform()
{
	transform_.setIdentity();
	transform_.translation( position_ );

	return transform_;
}

/**
 *	Change our transform, temporarily or permanently
 */
bool EditorChunkPulseLight::edTransform( const Matrix & m, bool transient )
{
	BW_GUARD;

	Vector3 posn = m.applyToOrigin();
	// it's permanent, so find out where we belong now
	Chunk * pOldChunk = pChunk_;
	Chunk * pNewChunk = this->edDropChunk( posn );
	if (pNewChunk == NULL) return false;

	// check light's inner radius range not exceed 2 chunks after item moved
	float f = pLight_->innerRadius();

	if (adjustRadius( posn, f ))
	{
		pLight_->innerRadius( f );
	}

	// check light's outer radius range not exceed 2 chunks after item moved
	f = pLight_->outerRadius();

	if (adjustRadius( posn, f ))
	{
		pLight_->outerRadius( f );
	}

	// if this is only a temporary change, keep it in the same chunk
	if (transient)
	{
		transform_ = m;
		position_ = transform_.applyToOrigin();
		pLight_->position( position_ + animPosition_ );
		pLight_->worldTransform( pChunk_->transform() );
		this->syncInit();
		return true;
	}

	// make sure the chunks aren't readonly
	if (!EditorChunkCache::instance( *pOldChunk ).edIsWriteable() 
		|| !EditorChunkCache::instance( *pNewChunk ).edIsWriteable())
		return false;

	// Ok, harder question: if we influence any non locked chunks in our old or new positions,
	// we can't move (+ do the same thing for edEdit, and use readonly props in the same case)

	/*
	if (influencesReadOnlyChunk( pOldChunk, pLight_->worldPosition(), pLight_ ))
		return false;
	if (influencesReadOnlyChunk( pNewChunk, pNewChunk->transform().applyPoint( pLight_->position() ), pLight_ ))
		return false;
	*/

	// ^^^ Ignored the problem by making static lights only traverse a single
	// portal, thus our 1 grid square barrier will take care of everything


	// Update static lighting in the old location
	markInfluencedChunks();


	// ok, accept the transform change then
	transform_.multiply( m, pOldChunk->transform() );
	transform_.postMultiply( pNewChunk->transformInverse() );
	position_ =  transform_.applyToOrigin();
	pLight_->position( position_ + animPosition_ );
	pLight_->worldTransform( pNewChunk->transform() );

	// note that both affected chunks have seen changes
	WorldManager::instance().changedChunk( pOldChunk );
	WorldManager::instance().changedChunk( pNewChunk );

	edMove( pOldChunk, pNewChunk );

	// Update static lighting in the new location
	markInfluencedChunks();
	this->syncInit();

	return true;
}


void EditorChunkPulseLight::loadModel()
{
	BW_GUARD;

	model_ = Model::get( "resources/models/dynamic.model" );
	modelSmall_ = Model::get( "resources/models/dynamic_small.model" );
	ResourceCache::instance().addResource( model_ );
	ResourceCache::instance().addResource( modelSmall_ );
}

bool EditorChunkPulseLight::edShouldDraw()
{
	BW_GUARD;

	if (!EditorChunkPhysicalMooLight<ChunkPulseLight>::edShouldDraw())
	{
		return false;
	}

	if (!OptionsLightProxies::visible())
		return false;

	if (OptionsLightProxies::pulseVisible())
		return true;

	return false;
}


bool EditorChunkPulseLight::usesLargeProxy() const
{
	BW_GUARD;

	return OptionsLightProxies::pulseLargeVisible();
}


bool EditorChunkPulseLight::setTimeScale( const float& timeScale )
{
	BW_GUARD;

	timeScale_ = timeScale;

	onFrameChanged();

	return true;
}


bool EditorChunkPulseLight::setDuration( const float& duration )
{
	BW_GUARD;

	duration_ = duration;

	onFrameChanged();

	return true;
}


bool EditorChunkPulseLight::setAnimation( const std::string& animation )
{
	BW_GUARD;

	animation_ = animation;

	Moo::NodePtr pNode = new Moo::Node;

	pAnimation_ = Moo::AnimationManager::instance().get( animation_, pNode );
	markInfluencedChunks();

	return true;
}


bool EditorChunkPulseLight::setFrames( const Frames& frames )
{
	BW_GUARD;

	frames_ = frames;

	onFrameChanged();

	return true;
}


void EditorChunkPulseLight::onFrameChanged()
{
	BW_GUARD;

	colourAnimation_.clear();

	for (uint32 i = 0; i <frames_.size(); i ++)
	{
		colourAnimation_.addKey( frames_[i].x * timeScale_, frames_[i].y );
	}

	if (colourAnimation_.getTotalTime() != 0.f)
	{
		colourAnimation_.loop( true, duration_ > 0.f ? duration_ * timeScale_ : colourAnimation_.getTotalTime() );
	}

	if (colourAnimation_.getTotalTime() == 0.f)
	{
		colourAnimation_.addKey( 0.f, 1.f );
		colourAnimation_.addKey( 1.f, 1.f );
	}

	markInfluencedChunks();

	// ok, flag the chunk as having changed then
	WorldManager::instance().changedChunk( chunk() );

	// update its data section
	edSave( pOwnSect() );
}


// -----------------------------------------------------------------------------
// Section: EditorChunkAmbientLight
// -----------------------------------------------------------------------------

ChunkItemFactory EditorChunkAmbientLight::factory_( "ambientLight", 1, EditorChunkAmbientLight::create );

ChunkItemFactory::Result EditorChunkAmbientLight::create( Chunk * pChunk, DataSectionPtr pSection )
{
	BW_GUARD;

	{
		MatrixMutexHolder lock( pChunk );
		std::vector<ChunkItemPtr> items = EditorChunkCache::instance( *pChunk ).staticItems();
		for( std::vector<ChunkItemPtr>::iterator iter = items.begin(); iter != items.end(); ++iter )
		{
			if( strcmp( (*iter) -> edClassName(), "ChunkAmbientLight" ) == 0 )
			{
				// Check to see for loading errors (Two ambient lights in one chunk file)
				if ( !pChunk->loaded() )
				{
					WARNING_MSG( "Chunk %s contains two or more ambient lights.\n", pChunk->identifier().c_str() );
 					return ChunkItemFactory::SucceededWithoutItem(); 
				}
				(*iter)->edPreDelete();
				pChunk->delStaticItem( *iter );

				return ChunkItemFactory::Result( (*iter), "The old ambient light was removed", true );
			}
		}
	}

	EditorChunkAmbientLight* pItem = new EditorChunkAmbientLight();

	if( pItem->load( pSection ) )
	{
		pChunk->addStaticItem( pItem );
		return ChunkItemFactory::Result( pItem );
	}

	delete pItem;
	return ChunkItemFactory::Result( NULL, "Failed to create ambient light" );
}

/**
 *	Save our data to the given data section
 */
bool EditorChunkAmbientLight::edSave( DataSectionPtr pSection )
{
	BW_GUARD;

	MF_ASSERT( pSection );

	if (!edCommonSave( pSection ))
		return false;

	Moo::Colour vcol = colour_;
	pSection->writeVector3( "colour",
		Vector3( vcol.r, vcol.g, vcol.b ) * 255.f );

	pSection->writeFloat( "multiplier", multiplier() );
	return true;
}


/**
 *	Add our properties to the given editor
 */
bool EditorChunkAmbientLight::edEdit( class GeneralEditor & editor )
{
	BW_GUARD;

	if (this->edFrozen())
		return false;

	if (!edCommonEdit( editor ))
	{
		return false;
	}

	// An ambient's light position shouldn't be editable.
	//MatrixProxy * pMP = new ChunkItemMatrix( this );
	//editor.addProperty( new GenPositionProperty(
	//	LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/POSITION" ), pMP ) );

	editor.addProperty( new ColourProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/COLOUR" ),
		new LightColourWrapper<EditorChunkAmbientLight>( this ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_LIGHT/MULTIPLIER" ),
		new AccessorDataProxy<EditorChunkAmbientLight,FloatProxy>(
			this, "multiplier", 
			&EditorChunkAmbientLight::getMultiplier, 
			&EditorChunkAmbientLight::setMultiplier ) ) );
	return true;
}

bool EditorChunkAmbientLight::edShouldDraw()
{
	BW_GUARD;

	if (!EditorChunkLight<ChunkAmbientLight>::edShouldDraw())
	{
		return false;
	}

	if (!OptionsLightProxies::visible())
		return false;

	if (OptionsLightProxies::ambientVisible())
		return true;

	return false;
}

namespace
{
	/**
	 * Make m refer to the centre of chunk
	 * Used to ensure the ambient light always sits in the centre of a chunk
	 */
	void setToCentre( Matrix& m, Chunk* chunk )
	{
		BW_GUARD;

		m.setIdentity();
		if ( chunk != NULL )
		{
			if ( !chunk->isOutsideChunk() )
			{
				// it's a shell, so get the 'real' bounding box by getting the
				// bounds of the first shell model in it
				EditorChunkCache& cc = EditorChunkCache::instance( *chunk );
				MatrixMutexHolder lock( chunk );
				std::vector<ChunkItemPtr> items = cc.staticItems();
				for ( std::vector<ChunkItemPtr>::iterator i = items.begin();
					i != items.end(); ++i )
				{
					if ( (*i)->isShellModel() )
					{
						BoundingBox bb;
						(*i)->edBounds( bb );
						m.translation( (bb.maxBounds() + bb.minBounds()) / 2.f );
						break;
					}
				}
			}
			else
			{
				// outside chunk, so use the old formula, just in case
				const BoundingBox& bb = chunk->localBB();
				m.translation( (bb.maxBounds() + bb.minBounds()) / 2.f );
			}
		}
		else
		{
			m.translation( Vector3::zero() );
		}
	}
}

/**
 *	Get the current transform
 */
const Matrix & EditorChunkAmbientLight::edTransform()
{
	return transform_;
}


/**
 *	Change our transform, temporarily or permanently
 */
bool EditorChunkAmbientLight::edTransform(
	const Matrix & m, bool transient )
{
	BW_GUARD;

	// it's permanent, so find out where we belong now
	Chunk * pOldChunk = pChunk_;
	Chunk * pNewChunk = this->edDropChunk( m.applyToOrigin() );
	if (pNewChunk == NULL) return false;

	if (transient)
	{
		transform_ = m;
		this->syncInit();
		return true;
	}

	// make sure the chunks aren't readonly
	if (!EditorChunkCache::instance( *pOldChunk ).edIsWriteable() 
		|| !EditorChunkCache::instance( *pNewChunk ).edIsWriteable())
		return false;

	// TODO: Don't accept the change if there's already an ambient light in
	// the new chunk

	// Update static lighting in the old location
	markInfluencedChunks();

	// ok, accept the transform change then
	setToCentre( transform_, pNewChunk );

	// note that both affected chunks have seen changes
	WorldManager::instance().changedChunk( pOldChunk );
	WorldManager::instance().changedChunk( pNewChunk );

	edMove( pOldChunk, pNewChunk );

	// Update static lighting in the new location
	markInfluencedChunks();
	this->syncInit();
	return true;
}


void EditorChunkAmbientLight::loadModel()
{
	BW_GUARD;

	model_ = Model::get( "resources/models/ambient_light.model" );
	modelSmall_ = Model::get( "resources/models/ambient_light_small.model" );
	ResourceCache::instance().addResource( model_ );
	ResourceCache::instance().addResource( modelSmall_ );
}


bool EditorChunkAmbientLight::usesLargeProxy() const
{
	BW_GUARD;

	return OptionsLightProxies::ambientLargeVisible();
}


/**
 *	Set our colour
 */
void EditorChunkAmbientLight::colour( const Moo::Colour & c )
{
	BW_GUARD;

	// set our colour
	colour_ = c;

	// now fix up the light container's own ambient colour
	this->toss( pChunk_ );

	// and tell the light cache that it's stale
	ChunkLightCache::instance( *pChunk_ ).bind( false );
}

void EditorChunkAmbientLight::toss( Chunk * pChunk )
{
	BW_GUARD;

	if (pChunk)
		setToCentre( transform_, pChunk );

	if (pChunk_)
	{
		StaticLighting::StaticChunkLightCache & clc =
			StaticLighting::StaticChunkLightCache::instance( *pChunk_ );

		clc.lights()->ambient( D3DCOLOR( 0x00000000 ) );
	}

	this->EditorChunkSubstance<ChunkAmbientLight>::toss( pChunk );

	if (pChunk_)
	{
		StaticLighting::StaticChunkLightCache & clc =
			StaticLighting::StaticChunkLightCache::instance( *pChunk_ );

		clc.lights()->ambient( colour() * multiplier() );
	}
}

bool EditorChunkAmbientLight::load( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!EditorChunkLight<ChunkAmbientLight>::load( pSection ))
		return false;

	multiplier( pSection->readFloat( "multiplier", 1.f ) );

	return true;
}

bool EditorChunkAmbientLight::setMultiplier( const float& m )
{
	BW_GUARD;

	multiplier(m);

	WorldManager::instance().changedChunk( chunk() );

	markInfluencedChunks();

	// update its data section
	edSave( pOwnSect() );

	this->toss( pChunk_ );

	// and tell the light cache that it's stale
	ChunkLightCache::instance( *pChunk_ ).bind( false );

	return true;
}

// editor_chunk_light.cpp
