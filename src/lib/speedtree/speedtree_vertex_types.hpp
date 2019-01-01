/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPEEDTREE_VERTEX_TYPES_HPP
#define SPEEDTREE_VERTEX_TYPES_HPP

#include "moo/index_buffer_wrapper.hpp"
#include "moo/vertex_buffer_wrapper.hpp"
#include "moo/moo_math.hpp"
#include "moo/texture_manager.hpp"
#include "moo/vertex_formats.hpp"

#include <d3dx9math.h>

using Moo::IndexBufferWrapper;
using Moo::VertexBufferWrapper;
using Moo::BaseTexturePtr;

namespace speedtree {

/**
 * This class represents a single allocation slot in the vertex/index 
 * buffer used by the speedtree.
 *
 * This is used to allow dynamic streaming (in/out) of different tree types.
 */
class BufferSlot : public SafeReferenceCount
{
public:
	BufferSlot(): start_(0), count_(0), next_(NULL), prev_(NULL) {}

	~BufferSlot() 
	{
		// Slot should not be active when destroyed.
		MF_ASSERT( !next_ );
		MF_ASSERT( !prev_ );
	}

	void init( int start, int count )
	{
		start_ = start;
		count_ = count;
	}

	void erased()
	{
		if ( next_ )
		{
			next_->updateStart( *this );
			next_->setPrev( prev_ );
		}

		if ( prev_ )
			prev_->setNext( next_ );
		
		clear();
	}

	void setPrev( SmartPointer<BufferSlot> prev )
	{ 
		MF_ASSERT_DEBUG( prev.getObject() != this );
		prev_ = prev;
	}
	void setNext( SmartPointer<BufferSlot> next )
	{
		MF_ASSERT_DEBUG( next.getObject() != this );
		next_ = next;
	}
	SmartPointer<BufferSlot> next() const { return next_; }
	SmartPointer<BufferSlot> prev() const { return prev_; }
	int start() const { return start_; }
	int count() const { return count_; }	
	void count(int count) { count_ = count; }
private:
	void updateStart( BufferSlot& deletedSlot )
	{
		start_ -= deletedSlot.count();
		if (next_)
			next_->updateStart( deletedSlot );			
	}

	void clear()
	{
		start_=0;
		count_=0;
		next_ = NULL;
		prev_ = NULL;
	}

	int start_;
	int count_;
	SmartPointer<BufferSlot> next_, prev_;
};

/**
 * This class is used to store the vertex/index buffer for the various
 * speedtree renderable components.
 *
 * The usage is monitored via the BufferSlot class to allow streaming.
 */
template < class T >
class STVector : public std::vector< T >
{
public:
	STVector() : dirty_(true), first_(0), last_(0) {}
	//TODO: consider using the STL for this allocation list management
	void eraseSlot( SmartPointer<BufferSlot> slot )
	{
		std::vector< T >::iterator it = this->begin() + slot->start();
		this->erase( it, it + slot->count() );

		if (last_ == slot)
		{
			last_ = slot->prev();
			if (last_)
				last_->setNext( NULL );
		}

		if (first_ == slot)
		{
			first_ = slot->next();
			if (first_)
				first_->setPrev( NULL );
		}

		if (first_ == last_)
		{
			last_ = NULL;
		}
		slot->erased();
		dirty(true);
	}

	SmartPointer<BufferSlot> newSlot( int size = 0 )
	{
		SmartPointer<BufferSlot> newSlot = new BufferSlot();
		newSlot->init( this->size(), size );
		addSlot( newSlot );
		return newSlot;
	}

	void dirty(bool val) { dirty_ = val; }
	bool dirty() const { return dirty_; }

private:
	void addSlot( SmartPointer<BufferSlot> slot )
	{
		if (first_ == NULL)
		{
			first_ = slot;
			last_ = NULL;
			
			first_->setNext( NULL );
			first_->setPrev( NULL );
		}			
		else
		{
			if (last_ == NULL)
			{
				last_ = slot;
				first_->setNext( last_ );
				last_->setPrev( first_ );
			}
			else
			{
				slot->setPrev( last_ );
				last_->setNext( slot );
				last_ = slot;
			}				
			slot->setNext( NULL );
		}
		dirty(true);
	}

	bool dirty_;

	SmartPointer<BufferSlot> first_, last_;
};

/**
 *	The vertex type used to render branches and fronds.
 */
struct BranchVertex
{
	Vector3 position_;
	Vector3 normal_;
	FLOAT   texCoords_[2];
	FLOAT   windIndex_;
	FLOAT   windWeight_;
	
#if SPT_ENABLE_NORMAL_MAPS
	Vector3 binormal_;
	Vector3	tangent_;
#endif // SPT_ENABLE_NORMAL_MAPS

	static DWORD fvf()
	{
		return
			D3DFVF_XYZ |
			D3DFVF_NORMAL |
			D3DFVF_TEXCOORDSIZE2(0) |
			D3DFVF_TEXCOORDSIZE2(1) |
#if SPT_ENABLE_NORMAL_MAPS
			D3DFVF_TEXCOORDSIZE3(2) |
			D3DFVF_TEXCOORDSIZE3(3) |
			D3DFVF_TEX4;
#else // SPT_ENABLE_NORMAL_MAPS
			D3DFVF_TEX2;
#endif // SPT_ENABLE_NORMAL_MAPS
	}
};

/**
 *	The vertex type used to render leaves.
 */
struct LeafVertex
{
	Vector3 position_;
	Vector3 normal_;
	FLOAT   texCoords_[2];
	FLOAT   windInfo_[2];
	FLOAT   rotInfo_[2];
	FLOAT   extraInfo_[3];
	FLOAT   geomInfo_[2]; 
	FLOAT   pivotInfo_[2];
#if SPT_ENABLE_NORMAL_MAPS
	Vector3 binormal_;
	Vector3	tangent_;
#endif // SPT_ENABLE_NORMAL_MAPS

	static DWORD fvf()
	{
		return
			D3DFVF_XYZ |
			D3DFVF_NORMAL |
			D3DFVF_TEXCOORDSIZE2(0) |
			D3DFVF_TEXCOORDSIZE2(1) |
			D3DFVF_TEXCOORDSIZE2(2) |
			D3DFVF_TEXCOORDSIZE3(3) |
			D3DFVF_TEXCOORDSIZE2(4) |
			D3DFVF_TEXCOORDSIZE2(5) |
#if SPT_ENABLE_NORMAL_MAPS
			D3DFVF_TEXCOORDSIZE3(6) |
			D3DFVF_TEXCOORDSIZE3(7) |
			D3DFVF_TEX8;
#else // SPT_ENABLE_NORMAL_MAPS
			D3DFVF_TEX6;
#endif // SPT_ENABLE_NORMAL_MAPS
	}
};


/**
 *	The vertex type used to render 
 *	normal billboards (non optimised).
 */
struct BillboardVertex
{
	Vector3 position_;
	Vector3 lightNormal_;
	Vector3 alphaNormal_;
	FLOAT   texCoords_[2];
	Vector3 binormal_;
	Vector3 tangent_;

	static DWORD fvf()
	{
		return
			D3DFVF_XYZ |
			D3DFVF_NORMAL |
			D3DFVF_TEXCOORDSIZE3(0) |
			D3DFVF_TEXCOORDSIZE2(1) |
			D3DFVF_TEXCOORDSIZE3(2) |
			D3DFVF_TEXCOORDSIZE3(3) |
			D3DFVF_TEX4;
	}

	static float s_minAlpha_;
	static float s_maxAlpha_;
};

typedef Moo::VertexBufferWrapper< BillboardVertex > BBVertexBuffer;

template< typename VertexType >
struct TreePartRenderData
{
	TreePartRenderData()
	{
#ifdef OPT_BUFFERS
		verts_ = NULL;
#endif

#if ENABLE_RESOURCE_COUNTERS	
		RESOURCE_COUNTER_ADD( ResourceCounters::DescriptionPool("Speedtree/TreePartRenderData", (uint)ResourceCounters::SYSTEM),
			sizeInBytes() )
#endif
	}
	~TreePartRenderData()
	{
		verts_ = NULL;
#if ENABLE_RESOURCE_COUNTERS	
		RESOURCE_COUNTER_SUB( ResourceCounters::DescriptionPool("Speedtree/TreePartRenderData", (uint)ResourceCounters::SYSTEM),
			sizeInBytes() )
#endif
	}
	typedef VertexBufferWrapper< VertexType >   VertexBufferWrapper;
	typedef SmartPointer< VertexBufferWrapper > VertexBufferPtr;
	typedef SmartPointer< IndexBufferWrapper >  IndexBufferPtr;
	typedef std::vector< IndexBufferPtr >       IndexBufferVector;
	
	
	typedef STVector< uint32 >				IndexVector;
	typedef STVector< VertexType >			VertexVector;

#ifdef OPT_BUFFERS
	void unload( VertexVector& verts, IndexVector& indices );
#endif


	/**
	 * TODO: to be documented.
	 */
	struct LodData
	{
#ifdef OPT_BUFFERS
		SmartPointer<BufferSlot> index_;
#endif
		//TODO: remove these... (billboards using them atm)
		VertexBufferPtr   vertices_;
		IndexBufferVector strips_;
	};

	/**
	 *	return the bytes used by this structure excluding vertex/index buffer
	 */
	int sizeInBytes() const
	{
		return sizeof( *this ) + lod_.capacity() * sizeof( lod_[0] );
	}
	typedef std::vector< LodData > LodDataVector;

	/**
	 *	Checks there is some vertex data in the LODs.
	 */
	bool checkVerts() const
	{
	#ifdef OPT_BUFFERS
		return (verts_ && verts_->count()>0);
	#else
		uint count=0;
		LodDataVector::const_iterator it=lod_.begin();
		while ( it!=lod_.end() && count==0 )
		{
			if ( (it)->vertices_ )
				count += (it)->vertices_->size();
			it++;
		}
		return (count>0);
	#endif
	}

	BaseTexturePtr  diffuseMap_;
#if SPT_ENABLE_NORMAL_MAPS
	BaseTexturePtr  normalMap_;
#endif // SPT_ENABLE_NORMAL_MAPS
	LodDataVector   lod_;

#ifdef OPT_BUFFERS
	SmartPointer<BufferSlot> verts_;
#endif

	// serialisation
	int size() const;
	char * write(char * pdata, const VertexVector& verts, const IndexVector& indices) const;
	const char * read(const char * data, VertexVector& verts, IndexVector& indices);

	// old versions: (still used by the billboards at the moment... TODO: remove)
	int oldSize() const;
	char * oldWrite(char * pdata) const;
	const char * oldRead(const char * data);
};

// Render Data;
typedef TreePartRenderData< BranchVertex >    BranchRenderData;
typedef TreePartRenderData< LeafVertex >      LeafRenderData;
typedef TreePartRenderData< BillboardVertex > BillboardRenderData;

} // namespace speedtree

#endif //SPEEDTREE_VERTEX_TYPES_HPP
