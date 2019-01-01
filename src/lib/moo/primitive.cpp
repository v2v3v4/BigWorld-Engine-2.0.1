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
#include "primitive.hpp"

#include "resmgr/bwresource.hpp"
#include "cstdmf/debug.hpp"
#include "render_context.hpp"
#include "resmgr/primitive_file.hpp"
#include "primitive_manager.hpp"
#include "primitive_file_structs.hpp"

#include "math/boundbox.hpp"

#include "vertices.hpp"

#ifndef CODE_INLINE
#include "primitive.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Moo", 0 )

namespace Moo
{

static bool s_enableDrawPrim = true;
static bool s_firstTime = true;

/**
 * Construct an empty object.
 */
Primitive::Primitive( const std::string& resourceID )
: resourceID_( resourceID ),
  maxVertices_( 0 ),
  primType_( D3DPT_TRIANGLELIST ),
  nIndices_( 0 )
{
	BW_GUARD;
	if (s_firstTime)
	{
		MF_WATCH( "Render/Performance/DrawPrim Primitive", s_enableDrawPrim,
			Watcher::WT_READ_WRITE,
			"Allow Primitive to call drawIndexedPrimitive()." );

		s_firstTime = false;
	}
}

/**
 * Destruct this object, and tell manager.
 */
Primitive::~Primitive()
{
	BW_GUARD;
	// let the manager know we're gone
	PrimitiveManager::del( this );
}

/**
 *	This method loads the primitive from a .primitive file.
 *
 *	@returns E_FAIL if unsuccessful.
 */
HRESULT Primitive::load( )
{
	BW_GUARD;
	release();
	HRESULT res = E_FAIL;

	// Is there a valid device pointer?
	if( Moo::rc().device() == (void*)NULL )
	{
		return res;
	}

	// find our data
	BinaryPtr indices;
	uint noff = resourceID_.find( ".primitives/" );
	if (noff < resourceID_.size())
	{
		// everything is normal
		noff += 11;
		DataSectionPtr sec = PrimitiveFile::get( resourceID_.substr( 0, noff ) );
		if ( sec )
			indices = sec->readBinary( resourceID_.substr( noff+1 ) );
	}
	else
	{
		// find out where the data should really be stored
		std::string fileName, partName;
		splitOldPrimitiveName( resourceID_, fileName, partName );

		// read it in from this file
		indices = fetchOldPrimitivePart( fileName, partName );
	}

	// Open the binary resource for this primitive
	//BinaryPtr indices = BWResource::instance().rootSection()->readBinary( resourceID_ );
	if( indices )
	{
		// Get the index header
		const IndexHeader* ih = reinterpret_cast< const IndexHeader* >( indices->data() );

		// Create the right type of primitive
		if( std::string( ih->indexFormat_ ) == "list" || std::string( ih->indexFormat_ ) == "list32" )
		{
			D3DFORMAT format = std::string( ih->indexFormat_ ) == "list" ? D3DFMT_INDEX16 : D3DFMT_INDEX32;
			primType_ = D3DPT_TRIANGLELIST;

			if (Moo::rc().maxVertexIndex() <= 0xffff && 
 				format == D3DFMT_INDEX32 ) 
 			{ 
 				ERROR_MSG( "Primitives::load - unable to create index buffer as 32 bit indices " 
 					"were requested and only 16 bit indices are supported\n" ); 
 				return res; 
 			} 

			DWORD usageFlag = rc().mixedVertexProcessing() ? D3DUSAGE_SOFTWAREPROCESSING : 0;

			// Create the index buffer
			if( SUCCEEDED( res = indexBuffer_.create( ih->nIndices_, format, usageFlag, D3DPOOL_MANAGED ) ) )
			{
				// store the number of elements in the index buffer
				nIndices_ = ih->nIndices_;

				// Get the indices
				const char* pSrc = reinterpret_cast< const char* >( ih + 1 );

				indices_.assign(pSrc, ih->nIndices_, format);

			    IndicesReference ind = indexBuffer_.lock();

				// Fill in the index buffer.
				ind.fill( pSrc, ih->nIndices_ );
				res = indexBuffer_.unlock( );

				// Add the buffer to the preload list so that it can get uploaded
				// to video memory
				indexBuffer_.addToPreloadList();

				pSrc += ih->nIndices_ * ind.entrySize();

				// Get the primitive groups
				const PrimitiveGroup* pGroup = reinterpret_cast< const PrimitiveGroup* >( pSrc );
				for( int i = 0; i < ih->nTriangleGroups_; i++ )
				{
					primGroups_.push_back( *(pGroup++) );
				}
			
				maxVertices_ = 0;
				
				// Go through the primitive groups to find the number of vertices referenced by the index buffer.
				PrimGroupVector::iterator it = primGroups_.begin();
				PrimGroupVector::iterator end = primGroups_.end();

				while( it != end )
				{
					maxVertices_ = max( (uint32)( it->startVertex_ + it->nVertices_ ), maxVertices_ );
					it++;
				}
			}
		}
	}
	else
	{
		ERROR_MSG( "Failed to read binary resource: %s\n", resourceID_.c_str() );
	}
	return res;
}

void Primitive::calcGroupOrigins( const VerticesPtr verts )
{
	BW_GUARD;
	const Vertices::VertexPositions& vertexPositions = verts->vertexPositions();

	PrimGroupVector::iterator it = primGroups_.begin();
	PrimGroupVector::iterator end = primGroups_.end();
	while ( it != end )
	{
		const PrimitiveGroup& group = (*it);
		BoundingBox bb;
		
		for ( int i = group.startVertex_; 
				i < group.startVertex_ + group.nVertices_; 
				++i )
		{
			bb.addBounds( vertexPositions[i] );
		}
		groupOrigins_.push_back( bb.centre() );

		it++;
	}
}

/**
 * This method sets all primitive groups for drawing and loads if necessary.
 */
HRESULT Primitive::setPrimitives( )
{
	BW_GUARD;
	if( !indexBuffer_.valid() )
	{
		HRESULT hr = load();
		if( FAILED ( hr ) )
			return hr;
	}

	IF_NOT_MF_ASSERT_DEV( indexBuffer_.valid() )
	{
		MF_EXIT( "index buffer loaded ok, but not valid?" );
	}

	return indexBuffer_.set();
}

/**
 *	This method draws an individual primitive group, given by index.
 *
 *	@param groupIndex The index of the group to draw.
 *	@returns	E_FAIL if indexes have not been set or groupIndex is invalid, or
 *				if drawing fails for some other reason.
 */
HRESULT Primitive::drawPrimitiveGroup( uint32 groupIndex )
{
	BW_GUARD;	
#ifdef _DEBUG
	IF_NOT_MF_ASSERT_DEV( Moo::rc().device() != NULL )
	{
		return E_FAIL;
	}
	IF_NOT_MF_ASSERT_DEV( groupIndex < primGroups_.size() )
	{
		return E_FAIL;
	}
	IF_NOT_MF_ASSERT_DEV( indexBuffer_.isCurrent() )
	{
		return E_FAIL;
	}
#endif

	// Draw the primitive group
	PrimitiveGroup& pg = primGroups_[ groupIndex ];

	if( pg.nVertices_ && pg.nPrimitives_ && s_enableDrawPrim )
	{
		return Moo::rc().drawIndexedPrimitive( primType_, 0, pg.startVertex_, pg.nVertices_, pg.startIndex_, pg.nPrimitives_ );
	}
	return S_OK;
}

/**
 * This method releases all index data and low level resources.
 */
HRESULT Primitive::release( )
{
	BW_GUARD;
	maxVertices_ = 0;
	primGroups_.clear();
	groupOrigins_.clear();
	indexBuffer_.release();
	return S_OK;
}

}


// primitive.cpp
