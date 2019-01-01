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
#include "chunk_water.hpp"

#include "cstdmf/guard.hpp"

#include "moo/render_context.hpp"

#include "romp/geometrics.hpp"

#include "chunk.hpp"
#include "chunk_obstacle.hpp"
#include "chunk_space.hpp"
#include "chunk_manager.hpp"

// -----------------------------------------------------------------------------
// Section: ChunkWater
// -----------------------------------------------------------------------------

int ChunkWater_token;

/**
 *	Constructor.
 */
ChunkWater::ChunkWater( std::string uid ) :
	VeryLargeObject( uid, "water" ),
	pWater_( NULL ),
	pChunk_( NULL )
{
}


/**
 *	Constructor.
 */
ChunkWater::ChunkWater( ) :
	pWater_( NULL )	
{
	uid_ = "";
	type_ = "water";
}


/**
 *	Destructor.
 */
ChunkWater::~ChunkWater()
{
	if (pWater_ != NULL)
	{
		Water::deleteWater(pWater_);
		pWater_ = NULL;
	}
}


/**
 *	Load method
 */
bool ChunkWater::load( DataSectionPtr pSection, Chunk * pChunk )
{
	BW_GUARD;
	if (pChunk==NULL)
		return false;

	// clear existing water if present
	if (pWater_ != NULL)
		shouldRebuild(true);

	// load new settings (water created on first draw)
	DataSectionPtr pReqSec;

	pReqSec = pSection->openSection( "position" );
	if (!pReqSec)
	{
		config_.position_ = pChunk->boundingBox().centre();
	}
	else
	{
		config_.position_ = pChunk->mapping()->mapper().applyPoint( pReqSec->asVector3() );
	}

	float lori = pSection->readFloat( "orientation", 0.f );
	Vector3 oriVec( sinf( lori ), 0.f, cosf( lori ) );
	Vector3 worldVec = pChunk->mapping()->mapper().applyVector(oriVec);
	config_.orientation_ = atan2f( worldVec.x, worldVec.z );

	pReqSec = pSection->openSection( "size" );
	if (!pReqSec) return false;

	Vector3 sizeV3 = pReqSec->asVector3();
	config_.size_ = Vector2( sizeV3.x, sizeV3.z );

	config_.fresnelConstant_ = pSection->readFloat( "fresnelConstant", 0.3f );
	config_.fresnelExponent_ = pSection->readFloat( "fresnelExponent", 5.f );

	config_.reflectionTint_ = pSection->readVector4( "reflectionTint", Vector4(1,1,1,1) );
	config_.reflectionScale_ = pSection->readFloat( "reflectionStrength", 0.04f );

	config_.refractionTint_ = pSection->readVector4( "refractionTint", Vector4(1,1,1,1) );
	config_.refractionScale_ = pSection->readFloat( "refractionStrength", 0.04f );

	config_.tessellation_ = pSection->readFloat( "tessellation", 10.f );
	config_.consistency_ = pSection->readFloat( "consistency", 0.95f );

	config_.textureTessellation_ = pSection->readFloat( "textureTessellation", config_.tessellation_);
	

	float oldX = pSection->readFloat( "scrollSpeedX", -1.f );	
	float oldY = pSection->readFloat( "scrollSpeedY", 1.f );

	config_.scrollSpeed1_ = pSection->readVector2( "scrollSpeed1", Vector2(oldX,0.5f) );	
	config_.scrollSpeed2_ = pSection->readVector2( "scrollSpeed2", Vector2(oldY,0.f) );	
	config_.waveScale_ = pSection->readVector2( "waveScale", Vector2(1.f,0.75f) );

	//config_.windDirection_ = pSection->readFloat( "windDirection", 0.0f );	
	config_.windVelocity_ = pSection->readFloat( "windVelocity", 0.02f );

	config_.sunPower_ = pSection->readFloat( "sunPower", 32.f );
	config_.sunScale_ = pSection->readFloat( "sunScale", 1.0f );

	config_.waveTexture_ = pSection->readString( "waveTexture", "system/maps/waves2.dds" );

	config_.simCellSize_ = pSection->readFloat( "cellsize", 100.f );
	config_.smoothness_ = pSection->readFloat( "smoothness", 0.f );

	config_.foamTexture_ = pSection->readString( "foamTexture", "system/maps/water_foam2.dds" );	
	
	config_.reflectionTexture_ = pSection->readString( "reflectionTexture", "system/maps/cloudyhillscubemap2.dds" );

	config_.deepColour_ = pSection->readVector4( "deepColour", Vector4(0.f,0.20f,0.33f,1.f) );

	config_.depth_ = pSection->readFloat( "depth", 10.f );
	config_.fadeDepth_ = pSection->readFloat( "fadeDepth", 0.f );

	config_.foamIntersection_ = pSection->readFloat( "foamIntersection", 0.25f );
	config_.foamMultiplier_ = pSection->readFloat( "foamMultiplier", 0.75f );
	config_.foamTiling_ = pSection->readFloat( "foamTiling", 1.f );	
	config_.bypassDepth_ = pSection->readBool( "bypassDepth", false );

	config_.useEdgeAlpha_ = pSection->readBool( "useEdgeAlpha", true );
	
	config_.useCubeMap_ = pSection->readBool( "useCubeMap", false );	
	
	config_.useSimulation_ = pSection->readBool( "useSimulation", true );

	config_.visibility_ = pSection->readInt( "visibility", Water::ALWAYS_VISIBLE );
	if ( config_.visibility_ != Water::ALWAYS_VISIBLE &&
		config_.visibility_ != Water::INSIDE_ONLY &&
		config_.visibility_ != Water::OUTSIDE_ONLY )
	{
		config_.visibility_ = Water::ALWAYS_VISIBLE;
	}

	config_.reflectBottom_ = pSection->readBool( "reflectBottom", true );

	config_.transparencyTable_ = pChunk->mapping()->path() + '_' + uid_ + ".odata";

	if (!BWResource::fileExists( config_.transparencyTable_ ))
	{
		config_.transparencyTable_ = pChunk->mapping()->path() + uid_ + ".odata";
	}

	pChunk_ = pChunk;
	return true;
}


void ChunkWater::syncInit(ChunkVLO* pVLO)
{
	BW_GUARD;
}

BoundingBox ChunkWater::chunkBB( Chunk* pChunk )
{
	BW_GUARD;
	BoundingBox bb = BoundingBox::s_insideOut_;
	BoundingBox cbb = pChunk->boundingBox();

	Vector3 size( config_.size_.x * 0.5f, 0, config_.size_.y * 0.5f );
	BoundingBox wbb( -size, size );

	Matrix m;
	m.setRotateY( config_.orientation_ );
	m.postTranslateBy( config_.position_ );

	wbb.transformBy( m );
    
	if (wbb.intersects( cbb ))
	{
		bb.setBounds( 
			Vector3(	std::max( wbb.minBounds().x, cbb.minBounds().x ),
						std::max( wbb.minBounds().y, cbb.minBounds().y ),
						std::max( wbb.minBounds().z, cbb.minBounds().z ) ),
			Vector3(	std::min( wbb.maxBounds().x, cbb.maxBounds().x ),
						std::min( wbb.maxBounds().y, cbb.maxBounds().y ),
						std::min( wbb.maxBounds().z, cbb.maxBounds().z ) ) );
		bb.transformBy( pChunk->transformInverse() );
	}

	return bb;
}


bool ChunkWater::addYBounds( BoundingBox& bb ) const
{
	BW_GUARD;
	bb.addYBounds( config_.position_.y );

	return true;
}


/**
 *	Draw (and update) this body of water
 */
void ChunkWater::draw( Chunk* pChunk )
{
	BW_GUARD;
	static DogWatch drawWatch( "ChunkWater" );
	ScopedDogWatch watcher( drawWatch );

	if (Moo::rc().reflectionScene())
		return;

	// create the water if this is the first time
	if (pWater_ == NULL)
	{
		pWater_ = new Water( config_, new ChunkRompTerrainCollider( /*pSpace*/ ) );	// TODO: Uncomment pSpace!

		this->objectCreated();
	}
	else if ( shouldRebuild() )
	{
		pWater_->rebuild( config_ );
		shouldRebuild(false);
		this->objectCreated();
	}

	// and remember to draw it after the rest of the solid scene
	if ( !s_simpleDraw )
	{
		Waters::addToDrawList( pWater_ );
		if (!pChunk->isOutsideChunk())
		{
			pWater_->addVisibleChunk( pChunk );
		}
		else
		{
			pWater_->outsideVisible( true );
		}
	}
	//RA: turned off simple draw for now....TODO: FIX
}


/**
 *	Apply a disturbance to this body of water
 */
void ChunkWater::sway( const Vector3 & src, const Vector3 & dst, const float diameter )
{
	BW_GUARD;
	if (pWater_ != NULL)
	{
		pWater_->addMovement( src, dst, diameter );
	}
}


#ifdef EDITOR_ENABLED
/**
 *	This method regenerates the water ... later
 */
void ChunkWater::dirty()
{
	BW_GUARD;
	if (pWater_ != NULL)
		shouldRebuild(true);
}
#endif //EDITOR_ENABLED

/**
 *	This static method creates a body of water from the input section and adds
 *	it to the given chunk.
 */
bool ChunkWater::create( Chunk * pChunk, DataSectionPtr pSection, std::string uid )
{
	BW_GUARD;
	//TODO: check it isnt already created?
	ChunkWater * pItem = new ChunkWater( uid );	
	if (pItem->load( pSection, pChunk ))
		return true;
	delete pItem;
	return false;
}


void ChunkWater::simpleDraw( bool state )
{
	ChunkWater::s_simpleDraw = state;
}


/// Static factory initialiser
VLOFactory ChunkWater::factory_( "water", 0, ChunkWater::create );
/// This variable is set to true if we would like to draw cheaply
/// ( e.g. during picture-in-picture )
bool ChunkWater::s_simpleDraw = false;
//
//
//bool ChunkWater::oldCreate( Chunk * pChunk, DataSectionPtr pSection )
//{
//	bool converted = pSection->readBool("deprecated",false);
//	if (converted)
//		return false;
//
//	//TODO: generalise the want flags...
//	ChunkVLO * pVLO = new ChunkVLO( 5 );	
//	if (!pVLO->legacyLoad( pSection, pChunk, std::string("water") ))
//	{
//		delete pVLO;
//		return false;
//	}
//	else
//	{
//		pChunk->addStaticItem( pVLO );
//		return true;
//	}
//	return false;
//}
//
///// Static factory initialiser
//ChunkItemFactory ChunkWater::oldWaterFactory_( "water", 0, ChunkWater::oldCreate );

// chunk_water.cpp
