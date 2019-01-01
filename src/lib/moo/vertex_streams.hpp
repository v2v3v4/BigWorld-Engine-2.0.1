/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VERTEX_STREAMS_HPP
#define VERTEX_STREAMS_HPP

#include "moo_math.hpp"
#include "moo_dx.hpp"
#include "vertex_buffer.hpp"
#include "cstdmf/smartpointer.hpp"
#include "vertex_declaration.hpp"

namespace Moo
{

/**
 * Stream details for the second UV data.
 */
struct UV2Stream
{
	static const std::string	TYPE_NAME;
	static const uint32			STREAM_NUMBER = 10;
	typedef Vector2				TYPE;
};

/**
 * Stream details for the vertex colour data.
 */
struct ColourStream
{		
	static const std::string	TYPE_NAME;
	static const uint32			STREAM_NUMBER = 11;
	typedef DWORD				TYPE;
};

/**
 * Vertex stream base.
 */
class VertexStream : public ReferenceCount
{
public:
	virtual void remapVertices( uint32 offset, uint32 nVerticesBeforeMapping, const std::vector< uint32 >& mappingNewToOld )=0;
	virtual void load( const void* data, uint32 nVertices )=0;

	void set()
	{
		if (vertexBuffer_.valid())
		{
			vertexBuffer_.set( stream_, 0, stride_ );
		}
	}
	void release()
	{
		vertexBuffer_.release();
	}

	void preload()
	{
		if (vertexBuffer_.valid())
			vertexBuffer_.addToPreloadList();
	}
	BinaryPtr data() { return pData_; }

private:
	std::string type_;
	uint32 stream_;
	uint32 stride_;
	Moo::VertexBuffer  vertexBuffer_;

protected:
	template<typename T>
	void loadInternal( const void* data, uint32 nVertices );

	template<typename T>
	void remap( uint32 offset, uint32 nVerticesBeforeMapping, const std::vector< uint32 >& mappingNewToOld );


	uint32 count_;
	BinaryPtr pData_;
};
typedef SmartPointer<VertexStream> VertexStreamPtr;


/**
 * Templated data holder
 */
template <class T>
class VertexStreamHolder : public VertexStream
{
public:
	VertexStreamHolder() 
	{
		pData_ = NULL;
		count_ = 0;
	}

	VertexStreamHolder( BinaryPtr pData, uint32 count ) 
	{
		pData_ = pData;
		count_ = count;
	}

	virtual void remapVertices( uint32 offset, uint32 nVerticesBeforeMapping, const std::vector< uint32 >& mappingNewToOld );
	virtual void load( const void* data, uint32 nVertices );
};


/**
 * Holder of all the streams for a vertex block.
 */
class StreamContainer
{
public:
	StreamContainer();

	void release()
	{
		for (uint i=0; i<streamData_.size(); ++i)
		{
			streamData_[i]->release();
		}
	}
	void set()
	{
		for (uint i=0; i<streamData_.size(); ++i)
		{
			streamData_[i]->set();
		}
	}
	void preload()
	{
		for (uint i=0; i<streamData_.size(); ++i)
		{
			streamData_[i]->preload();
		}
	}

	void updateDeclarations( VertexDeclaration* pDecl );

	std::vector<VertexStreamPtr> streamData_;
	VertexDeclaration* pSoftwareDecl_;
	VertexDeclaration* pSoftwareDeclTB_;
};

} // namespace Moo

#endif
/*vertex_streams.hpp*/
