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

#pragma warning (disable:4355)	// this used in initialiser list

#include "chunk_particles.hpp"

#include "particle_system_manager.hpp"
#include "meta_particle_system.hpp"
#include "chunk/chunk.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/string_provider.hpp"
#include "romp/geometrics.hpp"
#ifdef EDITOR_ENABLED
#include "appmgr/options.hpp"
#endif

#include "chunk/umbra_config.hpp"
#if UMBRA_ENABLE
#include "chunk/umbra_chunk_item.hpp"
#endif

#include "moo/visual_channels.hpp"

DECLARE_DEBUG_COMPONENT2( "Particle", 0 )

PROFILER_DECLARE( ChunkParticles_tick, "ChunkParticles Tick" );

// -----------------------------------------------------------------------------
// Section: ChunkParticles
// -----------------------------------------------------------------------------
char * g_chunkParticlesLabel = "ChunkParticles";

int ChunkParticles_token = 0;
static uint32 s_staggerIdx = 0;
static BasicAutoConfig<int> s_staggerInterval( "environment/chunkParticleStagger", 50 );


/**
 *	Constructor.
 */
ChunkParticles::ChunkParticles() :
	ChunkItem( (WantFlags)(WANTS_DRAW | WANTS_TICK) ),
	seedTime_( 0.1f ),	
	staggerIdx_( s_staggerIdx++ ),
	reflectionVisible_( false )
{
	ParticleSystemManager::init();
}


/**
 *	Destructor.
 */
ChunkParticles::~ChunkParticles()
{
	ParticleSystemManager::fini();
}


/**
 *	This method ensures that particle systems that are chunk items
 *	get seeded once they are placed in the world.  This is done
 *	for off-screen particles, in a staggered fashion.
 */
void ChunkParticles::tick( float dTime )
{
	BW_GUARD_PROFILER( ChunkParticles_tick );

	if ( !system_ )
		return;

	if (seedTime_ > 0.f)
	{		
		if ((staggerIdx_++ % s_staggerInterval.value()) == 0)
		{		
			system_->setDoUpdate();
			seedTime_ -= dTime;
		}
	}	

	bool updated = system_->tick( dTime );

	if (pChunk_)
	{
		// Update the visibility bounds of our parent chunk
		pChunk_->space()->updateOutsideChunkInQuadTree( pChunk_ );
	}

#if UMBRA_ENABLE
	// Update umbra object
	if (!pUmbraDrawItem_)
	{
		// Set up our unit boundingbox for the attachment, we use a unit bounding box
		// and scale it using transforms since this is a dynamic object.
		UmbraChunkItem* pUmbraChunkItem = new UmbraChunkItem;
		pUmbraChunkItem->init( this, BoundingBox( Vector3(0,0,0), Vector3(1,1,1)), Matrix::identity, NULL );
		pUmbraDrawItem_ = pUmbraChunkItem;
		updated = true;
	}
	
	if (pChunk_ != NULL)
	{
		if ( updated )
		{
			// Get the object to cell transform
			Matrix m = Matrix::identity;
			BoundingBox bb( BoundingBox::s_insideOut_ );
			system_->worldVisibilityBoundingBox(bb);

			// Create the scale transform from the bounding box
			Vector3 scale(0,0,0);
			if (!bb.insideOut())
				scale = bb.maxBounds() - bb.minBounds();

			// Const number to see if the bounding box is too big for umbra.
			const float TOO_BIG_FOR_UMBRA = 100000.f;

			if (scale.x != 0.f && scale.y != 0.f && scale.z != 0.f &&
				scale.x < TOO_BIG_FOR_UMBRA && scale.y < TOO_BIG_FOR_UMBRA && scale.z < TOO_BIG_FOR_UMBRA)
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
	}
	else
	{
		pUmbraDrawItem_->updateCell( NULL );
	}
#endif	
}


/**
 *	Draw the BoundingBox and VisibilityBox for the Particle System
 *	The parameters passed in are ignored, the details coming directly from the ChunkParticle.
 */
void ChunkParticles::drawBoundingBoxes( const BoundingBox &bb, const BoundingBox &vbb, const Matrix &spaceTrans ) const 
{
	BW_GUARD;	
#ifdef EDITOR_ENABLED
	static uint32 s_settingsMark = -1;
	static bool s_useDebugBB = false;

	if (Moo::rc().frameTimestamp() != s_settingsMark)
	{
		s_useDebugBB = !!Options::getOptionInt("debugBB", 0 );
		s_settingsMark = Moo::rc().frameTimestamp();
	}

	if (s_useDebugBB)
	{
		Matrix previousWorld = Moo::rc().world();
		Moo::rc().world( Matrix::identity );
		
		BoundingBox drawBB = BoundingBox::s_insideOut_;
		system_->localBoundingBox( drawBB );
		drawBB.transformBy( worldTransform_ );
		Geometrics::wireBox( drawBB, 0x00ffff00 );
		
		BoundingBox drawVBB = BoundingBox::s_insideOut_;
		system_->worldVisibilityBoundingBox( drawVBB );
		Geometrics::wireBox( drawVBB, 0x000000ff );
		
		Moo::rc().world( previousWorld );
	}
#endif
}


/**
 *	Load method
 */
bool ChunkParticles::load( DataSectionPtr pSection )
{
	BW_GUARD;
	localTransform_ = pSection->readMatrix34( "transform", Matrix::identity );

	resourceID_ = pSection->readString( "resource" );
	if (resourceID_.empty())
		return false;

	// Particles used to not have a path, support that by defaulting to particles/
	if (resourceID_.find( '/' ) == std::string::npos)
		resourceID_ = "particles/" + resourceID_;

	resourceDS_ = BWResource::openSection( resourceID_ );
	if (!resourceDS_)
		return false;

	// ok, we're committed to loading now.
	seedTime_ = resourceDS_->readFloat( "seedTime", seedTime_ );
	reflectionVisible_ = pSection->readBool( "reflectionVisible", reflectionVisible_ );

	system_ = new MetaParticleSystem();
	system_->load( resourceDS_, resourceID_ );
	system_->isStatic(true);
	system_->attach(this);
	
	return true;
}


/*virtual*/ void ChunkParticles::draw()
{
	BW_GUARD;
	if (Moo::rc().reflectionScene() && !reflectionVisible_)
		return;

	if ( !system_ )
		return;

	Moo::SortedChannel::reflecting(reflectionVisible_);

	float distance = 
		(worldTransform_.applyToOrigin() -
		Moo::rc().invView().applyToOrigin()).length();	

	system_->draw( worldTransform_, distance );
	
	Moo::SortedChannel::reflecting(true);
}


void ChunkParticles::load( const std::string& resourceName )
{
	BW_GUARD;
	std::string	resourceID = resourceName;
	if (resourceID.empty())
		return;

	// Particles used to not have a path, support that by defaulting to particles/
	if (resourceID.find( '/' ) == std::string::npos)
		resourceID = "particles/" + resourceID;

	DataSectionPtr pSystemRoot = BWResource::openSection( resourceID );
	if (!pSystemRoot)
		return;

	seedTime_ = pSystemRoot->readFloat( "seedTime", seedTime_ );
	system_->load( BWResource::getFilename( resourceID ),
		BWResource::getFilePath( resourceID ) );
	system_->isStatic(true);
}


/** 
 *	Toss method
 */
void ChunkParticles::toss( Chunk * pChunk )
{
	BW_GUARD;
	Chunk * oldChunk = pChunk_;
	bool wasNowhere = pChunk_ == NULL;

	// call base class method
	this->ChunkItem::toss( pChunk );

	Matrix m = localTransform_;
	if (pChunk_ != NULL)
	{
		if (wasNowhere)
		{
			m.postMultiply( pChunk_->transform() );
			this->setMatrix(m);		
			if ( system_ )
				system_->enterWorld();
		}
		else
		{
			if ( system_ )
				system_->leaveWorld();

			m.postMultiply( pChunk_->transform() );
			this->setMatrix(m);
			if ( system_ )
				system_->enterWorld();
		}
	}
	else
	{
		if (!wasNowhere)
		{
			if ( system_ )
				system_->leaveWorld();
		}
	}	
}


/*
 *	label (to determine the type of chunk item)
 */
const char * ChunkParticles::label() const
{
	return g_chunkParticlesLabel; 
}


/**
 *	This static method creates a particle system from the input section and adds
 *	it to the given chunk.
 */
ChunkItemFactory::Result ChunkParticles::create( Chunk * pChunk, DataSectionPtr pSection )
{
	BW_GUARD;
	SmartPointer<ChunkParticles> pParticles = new ChunkParticles();

	if (!pParticles->load( pSection ))
	{
		
		std::string err = LocaliseUTF8( L"PARTICLE/CHUNK_PARTICLES/FAILED_LOAD" ); 
		if ( pSection )
		{
			err += '\'' + pSection->readString( "resource" ) + '\'';
		}
		else
		{
			err += "(NULL)";
		}

		return ChunkItemFactory::Result( NULL, err );
	}
	else
	{ 
		pChunk->addStaticItem( &*pParticles );
		return ChunkItemFactory::Result( pParticles );
	}
}

bool ChunkParticles::addYBounds( BoundingBox& bb ) const
{
	if (system_)
	{
		BoundingBox me;
		//We are using the work visibility bounding box as this returns us the coorect Y for the 
		//system as chunks are mapped to 0 in the y axis. 
		system_->worldVisibilityBoundingBox( me );
		bb.addYBounds( me.minBounds().y );
		bb.addYBounds( me.maxBounds().y );
		return true;
	}
	else 
	{
		WARNING_MSG("ChunkParticles::addYBounds but no system_ available");
		return false;
	}
}

/// Static factory initialiser
ChunkItemFactory ChunkParticles::factory_(
	"particles", 0, ChunkParticles::create );

// chunk_particles.cpp
