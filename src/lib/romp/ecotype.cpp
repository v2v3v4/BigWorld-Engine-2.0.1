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
#include "ecotype.hpp"
#include "ecotype_generators.hpp"
#include "flora.hpp"
#include "flora_texture.hpp"
#include "flora_constants.hpp"
#include "flora_renderer.hpp"
#include "terrain/base_terrain_block.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/bgtask_manager.hpp"

DECLARE_DEBUG_COMPONENT2( "romp", 0 )

SimpleMutex Ecotype::s_deleteMutex_;


Ecotype::Ecotype( Flora* flora) :	
	textureResource_( "" ),
	pTexture_( NULL ),	
	refCount_( 0 ),
	id_( 0 ),
	uvOffset_( 0.f, 0.f ),	
	generator_( NULL ),
	loadInfo_( NULL ),
	isLoading_( false ),
	isInited_ ( false ),
	flora_( flora )
{
}


Ecotype::~Ecotype()
{
	if ( loadInfo_ )
		loadInfo_->ecotype_ = NULL;
	if (generator_)
		delete generator_;
}


void Ecotype::init(DataSectionPtr allEcotypesSection, uint8 id)
{		
	this->id_ = id;
	this->textureResource_ = "";

	BkLoadInfo * li = new BkLoadInfo;	
	li->pSection_ = allEcotypesSection;	
	li->loadingTask_ = new CStyleBackgroundTask( &Ecotype::backgroundInit, li,
						&Ecotype::onBackgroundInitComplete, li );
	li->ecotype_ = this;
	this->loadInfo_ = li;
	this->isLoading_ = true;
	this->isInited_ = true;

	BgTaskManager::instance().addBackgroundTask( li->loadingTask_ );
}


/**
 *	This method is called internally when our refCount goes to 0
 *	it releases all resources, and flags ourselves as requiring
 *	re-initialisation.
 *
 *	re-initialisation occurs when our refCount increases again.
 */
void Ecotype::finz()
{
	MF_ASSERT( refCount_ == 0 );
	MF_ASSERT( !this->isLoading_ );
	this->isInited_ = false;
	pTexture_ = NULL;
	textureResource_ = "";
	if (generator_)
	{
		delete generator_;
		generator_ = NULL;
	}
}


/**
 *	This static method is called by the background loading thread.
 *	It initialises an ecotype given the data section for all ecotypes,
 *	and an id ( lookup index into the ecotype data sections )
 *
 *	Note : allEcotypesSection is "environments/flora.xml"
 */
void Ecotype::backgroundInit( void * bkLoadInfo )
{
	SimpleMutexHolder smh( s_deleteMutex_ );

	BkLoadInfo* li = (BkLoadInfo*)bkLoadInfo;
	Ecotype* ecotype = li->ecotype_;

	if (!ecotype || !li->pSection_)
		return;

	DataSectionPtr pSection = li->pSection_->openChild( ecotype->id() );
	if ( pSection )
	{
		//TRACE_MSG( "Ecotype %s - background init\n", pSection->sectionName().c_str() );
		ecotype->generator( EcotypeGenerator::create(pSection, *ecotype) );
		return;
	}

	MF_ASSERT( !ecotype->generator() );
	ecotype->generator( new EmptyEcotype() );
	ecotype->textureResource() = "";
	ERROR_MSG( "Could not find ecotype definition for ecotype id %d\n", ecotype->id() );	
}


/**
 *	This static method is called by the background thread manager
 *	once the background loading task is complete.
 */
void Ecotype::onBackgroundInitComplete( void * bkLoadInfo )
{
	BkLoadInfo* li = (BkLoadInfo*)bkLoadInfo;

	if (li->ecotype_ && li->pSection_)
	{
		li->ecotype_->isLoading_ = false;
		li->ecotype_->loadInfo_ = NULL;
		DataSectionPtr pSection = li->pSection_->openChild( li->ecotype_->id() );
		if ( pSection )
		{
			//TRACE_MSG( "Ecotype %s - background init complete\n", pSection->sectionName().c_str() );
			Moo::BaseTexturePtr pTex = li->ecotype_->pTexture();
			//TRACE_MSG( "Ecotype %s - actual texture is %s\n", pSection->sectionName().c_str(), pTex ? pTex->resourceID().c_str() : "None" );
		}
	}

	delete li;	
}


void Ecotype::incRef()
{
	++refCount_;
	if (refCount_==1)
	{		
		MF_ASSERT( !this->isLoading_ );
		MF_ASSERT( this->isInited_ );
		//TRACE_MSG( "Ecotype %d - ref count went to 1.  Texture is %s\n", id_, pTexture_ ? pTexture_->resourceID().c_str() : "None" );
		grabTexture();
	}
}

void Ecotype::decRef()
{
	refCount_--;
	if (!refCount_)
	{
		//TRACE_MSG( "Ecotype %d - ref count went to 0\n", id_ );
		freeTexture();
		this->finz();
	}
}


/**
 *	This method asks the flora singleton for a piece of texture memory.
 *	Our texture is passed in, so the flora can swap in the texture.
 *	The allocateTextureBlock method returns a UV offset in the large texture.
 *	derived classes will want to update their vertices in response.
 *
 *	Note : this must be called from the main thread, otherwise the loading
 *	thread will be accessing video ram and that is bad.
 */
void Ecotype::grabTexture()
{
	if ( pTexture_ )
	{
		this->offsetUV( flora_->floraTexture()->allocate( id_, pTexture_ ) );
		//the flora texture will synchronously copy pTexture_ into its
		//conglomerate texture map.  we should no longer keep a hold of
		//the resource.
		pTexture_ = NULL;
	}	
}


/**
 *	This method tells the flora singleton we no longer need our allocated
 *	piece of texture memory.  Allow other ecotypes to use that memory.
 */
void Ecotype::freeTexture()
{
	if ( textureResource_ != "" )
	{				
		flora_->floraTexture()->deallocate( id_ );
	}
}


/**
 *	This method sets a new offset into the collated flora texture map
 *	for this ecotype.
 */
void Ecotype::offsetUV( Vector2& offset )
{
	uvOffset_ = offset;	
}


/**
 *	This method puts transformed vertices into the vertex array passed
 *	in.  It uses the underlying ecotype generator to do so.
 */
uint32 Ecotype::generate(
		class FloraVertexContainer* pVerts,
		uint32 idx,
		uint32 numVerts,
		const Matrix& objectToWorld,
		const Matrix& objectToChunk,
		BoundingBox& bb )
{
	if ( generator_ )
		return generator_->generate( uvOffset_, pVerts, idx, numVerts, objectToWorld, objectToChunk, bb );
	else
	{
		pVerts->clear(numVerts);
		return numVerts;
	}
}


/**
 *	This method returns true if the ecotype is empty.  Being empty means
 *	it will not draw any detail objects, and neighbouring ecotypes will not
 *	encroach.
 */
bool Ecotype::isEmpty() const
{
	if (generator_)
		return generator_->isEmpty();

	return true;
}