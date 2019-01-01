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
#include "resmgr/bwresource.hpp"
#include "resmgr/bin_section.hpp"
#include "static_light_values.hpp"
#include "moo/render_context.hpp"
#include "moo/resource_load_context.hpp"
#include "moo/vertex_buffer.hpp"

DECLARE_DEBUG_COMPONENT2( "Romp", 0 );


static const uint32 STATIC_LIGHTING_CACHE_MAGIC = 'LIGT';

// -----------------------------------------------------------------------------
// Section: StaticLightValueCache
// -----------------------------------------------------------------------------

void StaticLightValueCache::set( DWORD offset, DWORD size )
{
	BW_GUARD;

	if (vb_.valid() && offset + size <= sizeInBytes())
	{
		vb_.set( Moo::StaticVertexColours::STREAM_NUMBER, offset, sizeof( D3DCOLOR ) );
	}
}


void StaticLightValueCache::createVB( const D3DCOLOR *colours, size_t size )
{
	BW_GUARD;

	DX::Device* pDev = Moo::rc().device();

	vb_.release();

	if (size)
	{
		DWORD usageFlag = Moo::rc().mixedVertexProcessing() ? D3DUSAGE_SOFTWAREPROCESSING : 0;

	#ifndef EDITOR_ENABLED
		usageFlag |= D3DUSAGE_WRITEONLY;
	#endif

		Moo::VertexBuffer vb;
		HRESULT hr = vb.create( size,
			usageFlag,
			0,
			D3DPOOL_MANAGED,
			"vertex buffer/static light" );

		if (SUCCEEDED( hr ))
		{
			DX::newFrame();
			Moo::VertexLock<D3DCOLOR> vl( vb );
			if (vl)
			{
				memcpy( vl, colours, size );

				vb_ = vb;
			}
			else
			{
				ERROR_MSG( "StaticLightValues::vb: unable to lock vertex buffer\n" );
			}
		}
		else
		{
			ERROR_MSG( "StaticLightValues::vb: Unable to create vertex "
				"buffer with %d colour entries: %s", size, 
				DX::errorAsString( hr ).c_str() );
		}
		
		if (SUCCEEDED( hr ))
		{
			// Add the buffer to the preload list so that it can get uploaded
			// to video memory
			vb.addToPreloadList();
		}
	}
}


void StaticLightValueCache::invalidateData()
{
	BW_GUARD;

#ifndef EDITOR_ENABLED
	vb_.release();
#else//EDITOR_ENABLED
	invalid_ = true;
	ColourValueVector().swap( colours_ );
#endif//EDITOR_ENABLED
}

bool StaticLightValueCache::load( DataSectionPtr ds )
{
	BW_GUARD;

	if (ds)
	{
		ds = ds->openSection( "staticLightingCache", false, BinSection::creator() );

		if (ds)
		{
			BinaryPtr bp = ds->asBinary();
			unsigned char* data = (unsigned char*)bp->data();

			if (bp->len() > sizeof( STATIC_LIGHTING_CACHE_MAGIC ) &&
				*(uint32*)data == STATIC_LIGHTING_CACHE_MAGIC)
			{
				data += sizeof( STATIC_LIGHTING_CACHE_MAGIC );
				createVB( (D3DCOLOR*)data,
					bp->len() - sizeof( STATIC_LIGHTING_CACHE_MAGIC ) );

				return true;
			}
		}
	}

	return false;
}


#ifdef EDITOR_ENABLED

void StaticLightValueCache::startCalculating()
{
	BW_GUARD;

	invalidateData();
}


void StaticLightValueCache::endCalculating()
{
	BW_GUARD;

	invalid_ = false;
	createVB( &colours_[0], colours_.size() * sizeof( D3DCOLOR ));
	ColourValueVector().swap( colours_ );
}


void StaticLightValueCache::addDefault( size_t size )
{
	BW_GUARD;

	colours_.insert( colours_.end(), size, DEFAULT_LIGHT_VALUE );
}


D3DCOLOR* StaticLightValueCache::allocate( size_t size, D3DCOLOR ambient )
{
	BW_GUARD;

	size_t current = colours_.size();

	colours_.insert( colours_.end(), size, ambient );

	return &colours_[ current ];
}


size_t StaticLightValueCache::size() const
{
	BW_GUARD;

	return colours_.size();
}


bool StaticLightValueCache::save( DataSectionPtr ds )
{
	BW_GUARD;

	ds->deleteSections( "staticLighting" );
	ds->deleteSections( "staticLightingCache" );

	if (!vb_.valid() || invalid_)
	{// we don't have static lighting
		return true;
	}
	else
	{
		DX::newFrame();
		Moo::VertexLock<D3DCOLOR> vl( vb_ );

		if (vl)
		{
			BinaryPtr pSaveBlock = new BinaryBlock( NULL,
				sizeof( STATIC_LIGHTING_CACHE_MAGIC ) + vb_.size(),
				"BinaryBlock/StaticLightValues" );

			*(uint32*)pSaveBlock->data() = STATIC_LIGHTING_CACHE_MAGIC;

			unsigned char* vertexStart = (unsigned char*)pSaveBlock->data()
				+ sizeof( STATIC_LIGHTING_CACHE_MAGIC );

			memcpy( vertexStart, vl, vb_.size() );

			if (ds->writeBinary( "staticLightingCache", pSaveBlock ))
			{
				return true;
			}
			else
			{
				CRITICAL_MSG( "StaticLightValues::saveData: error while writing BinSection\n" );
			}
		}
		else
		{
			ERROR_MSG( "StaticLightValues::saveData: Unable to lock vertex\n" );
		}
	}

	return false;
}

#endif//EDITOR_ENABLED


// -----------------------------------------------------------------------------
// Section: StaticLightValues
// -----------------------------------------------------------------------------


/**
 *	Constructor.
 */
StaticLightValues::StaticLightValues( StaticLightValueCachePtr cache,
									 DataSectionPtr pData, uint32 vertexNum )
	: cache_( cache ), offset_( 0 ), size_( 0 )
{
	BW_GUARD;

	MF_ASSERT( cache_.exists() );

	if (pData.hasObject())
	{
		offset_ = pData->readUInt( "offset" );
		size_ = pData->readUInt( "size" );

		if (size_ != vertexNum * sizeof( D3DCOLOR ) ||
			(offset_ + size_ > cache->sizeInBytes()
#ifdef EDITOR_ENABLED
			&& offset_ + size_ > cache->size() * sizeof( D3DCOLOR )
#endif//EDITOR_ENABLED
			))
		{
			offset_ = size_ = 0;
			cache->invalidateData();
		}
	}
}


void StaticLightValues::unset()
{
	BW_GUARD;

	Moo::VertexBuffer::reset( STREAM_NUMBER );
}


void StaticLightValues::set()
{
	BW_GUARD;

	cache_->set( offset_, size_ );
}

#ifdef EDITOR_ENABLED

bool StaticLightValues::save( DataSectionPtr ds )
{
	BW_GUARD;

	ds = ds->newSection( "staticLighting" );

	ds->writeUInt( "offset", offset_ );
	ds->writeUInt( "size", size_ );

	return true;
}

#endif

// static_light_values.cpp
