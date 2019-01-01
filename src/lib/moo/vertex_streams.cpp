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
#include "vertex_streams.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/memory_counter.hpp"
#include "render_context.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/quick_file_writer.hpp"
#include "vertex_formats.hpp"

DECLARE_DEBUG_COMPONENT2( "Moo", 0 )

namespace Moo
{

const std::string UV2Stream::TYPE_NAME = "uv2";
const std::string ColourStream::TYPE_NAME = "colour";

/**
 * template specialisation for UV2
 */
template <>
void VertexStreamHolder<UV2Stream>::remapVertices( uint32 offset, uint32 nVerticesBeforeMapping, const std::vector< uint32 >& mappingNewToOld )
{
	this->remap<UV2Stream>(offset,nVerticesBeforeMapping,mappingNewToOld);
}

/**
 *  template specialisation for colour
 */
template <>
void VertexStreamHolder<ColourStream>::remapVertices( uint32 offset, uint32 nVerticesBeforeMapping, const std::vector< uint32 >& mappingNewToOld )
{
	this->remap<ColourStream>(offset,nVerticesBeforeMapping,mappingNewToOld);
}


/**
 *  template specialisation for UV2
 */
template <>
void VertexStreamHolder<UV2Stream>::load( const void* data, uint32 nVertices )
{
	this->loadInternal<UV2Stream>(data, nVertices);
}

/**
 *  template specialisation for colour
 */
template <>
void VertexStreamHolder<ColourStream>::load( const void* data, uint32 nVertices )
{
	this->loadInternal<ColourStream>(data, nVertices);
}


/**
 * rearrange the order of the stream to match the given new->old remapping.
 */
template <typename T>
void VertexStream::remap( uint32 offset, uint32 nVerticesBeforeMapping, const std::vector< uint32 >& mappingNewToOld )
{
	T::TYPE* pOrigData = (T::TYPE*)pData_->data();
	
	std::vector<T::TYPE> newVerts;

	// copy the data before this
	for (size_t i = 0; i < offset; ++i)
	{
		newVerts.push_back( pOrigData[i] );
	}
	// copy the actual data being remapped.
	for (size_t i = 0; i < mappingNewToOld.size(); ++i)
	{			
		T::TYPE newData;
		newData = pOrigData[ offset + mappingNewToOld[i] ];
		
		newVerts.push_back( newData );
	}
	
	// now copy the last bit of data after our target range.
	for (size_t i = offset + nVerticesBeforeMapping; i < count_; ++i)
	{			
		newVerts.push_back( pOrigData[i] );
	}
	
	// finally, rebuild the binary.
	QuickFileWriter qfw;
	qfw << newVerts;
	pData_ = qfw.output();
	count_ = newVerts.size();
}


/**
 * load the data from the data block and create the vertex buffer.
 */
template <typename T>
void VertexStream::loadInternal( const void* data, uint32 nVertices )
{
	stream_ = T::STREAM_NUMBER;
	type_ = T::TYPE_NAME;

	const T::TYPE * pSrcData = reinterpret_cast< const T::TYPE* >( data );
		
	//create the buffer
	DWORD usageFlag = rc().mixedVertexProcessing() ? D3DUSAGE_SOFTWAREPROCESSING : 0;

	Moo::VertexBuffer pVertexBuffer;
	HRESULT res = E_FAIL;

	// Create the vertex buffer
	if (SUCCEEDED( res = pVertexBuffer.create( 
		nVertices * sizeof( T::TYPE ), usageFlag, 0, 
		D3DPOOL_MANAGED ) ))
	{
		// Set up the smartpointer to the vertex buffer
		vertexBuffer_ = pVertexBuffer;

		// Try to lock the vertexbuffer.
		VertexLock<T::TYPE> vl( vertexBuffer_, 0, nVertices*sizeof(T::TYPE), 0 );
		if (vl)
		{
			memcpy( vl, pSrcData, sizeof( T::TYPE ) * nVertices );
			stride_ = sizeof( T::TYPE );
		}
	}
}


/**
 * 
 */
StreamContainer::StreamContainer()
{
	pSoftwareDecl_ = VertexDeclaration::get( "xyznuv" );
	pSoftwareDeclTB_ = VertexDeclaration::get( "xyznuvtb" );
}


/**
 * Create the required declarations (combining where necessary).
 */
void StreamContainer::updateDeclarations( VertexDeclaration* pDecl )
{
	if (pSoftwareDecl_)
	{
		pSoftwareDecl_ = VertexDeclaration::combine( pSoftwareDecl_, pDecl );
	}

	if (pSoftwareDeclTB_)
	{
		pSoftwareDeclTB_ = VertexDeclaration::combine( pSoftwareDeclTB_, pDecl );
	}
}

} //namespace Moo
