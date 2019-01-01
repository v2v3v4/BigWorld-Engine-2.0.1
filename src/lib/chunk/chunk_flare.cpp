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
#include "chunk_flare.hpp"

#include "resmgr/bwresource.hpp"
#include "moo/render_context.hpp"
#include "math/colour.hpp"
#include "romp/lens_effect_manager.hpp"

#if UMBRA_ENABLE
#include "umbra_chunk_item.hpp"
#endif

#include "chunk.hpp"

#include "cstdmf/guard.hpp"



// -----------------------------------------------------------------------------
// Section: ChunkFlare
// -----------------------------------------------------------------------------

int ChunkFlare_token = 0;

/**
 *	Constructor.
 */
ChunkFlare::ChunkFlare() :
	ChunkItem( WANTS_DRAW ),
	position_( Vector3::zero() ),
	colour_( 255.f, 255.f, 255.f ),
	colourApplied_( false )
{
}


/**
 *	Destructor.
 */
ChunkFlare::~ChunkFlare()
{
}


/**
 *	Load method
 */
bool ChunkFlare::load( DataSectionPtr pSection, Chunk* pChunk )
{
	BW_GUARD;
	std::string	resourceID = pSection->readString( "resource" );
	if (resourceID.empty()) return false;

	DataSectionPtr pFlareRoot = BWResource::openSection( resourceID );
	if (!pFlareRoot) return false;

	// ok, we're committed to loading now.
	// since we support reloading, remove old effects first
	for (uint i = 0; i < lensEffects_.size(); i++)
	{
		LensEffectManager::instance().forget( i + (uint32)this );
	}
	lensEffects_.clear();

	LensEffect le;
	if (le.load( pFlareRoot ))
	{
		le.maxDistance( pSection->readFloat( "maxDistance", le.maxDistance() ) );
		le.area( pSection->readFloat( "area", le.area() ) );
		le.fadeSpeed( pSection->readFloat( "fadeSpeed", le.fadeSpeed() ) );
		lensEffects_.push_back( le );
	}

	position_ = pSection->readVector3( "position" );
	DataSectionPtr pColourSect = pSection->openSection( "colour" );
	if (pColourSect)
	{
		colour_ = pColourSect->asVector3();
		colourApplied_ = true;
	}
	else
	{
		colour_.setZero();
		colourApplied_ = false;
	}
	
	return true;
}


void ChunkFlare::syncInit()
{
	BW_GUARD;	
#if UMBRA_ENABLE
	delete pUmbraDrawItem_;
	BoundingBox bb( Vector3(-0.5f,-0.5f,-0.5f), Vector3(0.5f,0.5f,0.5f) );

	// Set up object transforms
	Matrix m = this->pChunk_->transform();
	Matrix tr;
	tr.setTranslate( position_ );
	m.preMultiply( tr );

	UmbraChunkItem* pUmbraChunkItem = new UmbraChunkItem;
	pUmbraChunkItem->init( this, bb, m, pChunk_->getUmbraCell() );
	pUmbraDrawItem_ = pUmbraChunkItem;
	this->updateUmbraLenders();
#endif
}

/**
 *	The draw function ... add our lens effects to the list
 */
void ChunkFlare::draw()
{
	BW_GUARD;
	//during some rendering passes we ignore all lens flares...
	if ( s_ignore )
		return;

	static DogWatch drawWatch( "ChunkFlare" );
	ScopedDogWatch watcher( drawWatch );

	uint32 leid = (uint32)this;

	Vector4 tintColour( colour_ / 255.f, 1.f );

	LensEffects::iterator it = lensEffects_.begin();
	LensEffects::iterator end = lensEffects_.end();
	while( it != end )
	{
		LensEffect & le = *it++;

		uint32 oldColour;

		if (colourApplied_)
		{
			// Multiply our tinting colour with the flare's base colour
			oldColour = le.colour();
			Vector4 flareColour( Colour::getVector4Normalised( oldColour ) );
			flareColour.w *= tintColour.w;
			flareColour.x *= tintColour.x;
			flareColour.y *= tintColour.y;
			flareColour.z *= tintColour.z;
			le.colour( Colour::getUint32FromNormalised( flareColour ) );
		}

		LensEffectManager::instance().add(
			leid++, Moo::rc().world().applyPoint( position_ ), le );

		if (colourApplied_)
		{
			le.colour( oldColour );
		}
	}
}


/**
 *	This static method creates a flare from the input section and adds
 *	it to the given chunk.
 */
ChunkItemFactory::Result ChunkFlare::create( Chunk * pChunk, DataSectionPtr pSection )
{
	BW_GUARD;
	ChunkFlare * pFlare = new ChunkFlare();

	if (!pFlare->load( pSection, pChunk ))
	{
		delete pFlare;
		return ChunkItemFactory::Result( NULL,
			"Failed to load flare " + pSection->readString( "resource" ) );
	}
	else
	{
		pChunk->addStaticItem( pFlare );
		return ChunkItemFactory::Result( pFlare );
	}
}


/// Static factory initialiser
ChunkItemFactory ChunkFlare::factory_( "flare", 0, ChunkFlare::create );


void ChunkFlare::ignore( bool state )
{
	ChunkFlare::s_ignore = state;
}

bool ChunkFlare::s_ignore = false;

// chunk_flare.cpp
