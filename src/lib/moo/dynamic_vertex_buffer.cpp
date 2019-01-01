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

#include "dynamic_vertex_buffer.hpp"
#include "render_context.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/memory_counter.hpp"

#ifndef CODE_INLINE
#include "dynamic_vertex_buffer.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "moo", 0 )

#ifndef EDITOR_ENABLED
memoryCounterDefine( dynamicVB, Base );
#endif

PROFILER_DECLARE( DynamicVertexBufferBase_lock, "DynamicVertexBufferBase Lock" );
PROFILER_DECLARE( DynamicVertexBufferBase_lock2, "DynamicVertexBufferBase Lock 2" );
PROFILER_DECLARE( DynamicVertexBufferBase_unlock, "DynamicVertexBufferBase Unlock" );

namespace Moo
{

// This static list holds all the instances created. It is used when it is
// needed to have access to all vertex buffers, such as when finalising the
// application to delete all instances.
std::list< DynamicVertexBufferBase* > DynamicVertexBufferBase::dynamicVBS_;

// This static will be set to true after the fini method is called.
static bool s_finalised = false;


/**
 *	This method releases all resources held by all dynamic vertex buffers.
 */
void DynamicVertexBufferBase::releaseAll( )
{
	BW_GUARD;
	std::list< DynamicVertexBufferBase* >::iterator it = dynamicVBS_.begin();

	while( it != dynamicVBS_.end() )
	{
		(*it)->release();
		it++;
	}
}

/**
 *	This method resets the lock on all dynamic vertex buffers.
 */
void DynamicVertexBufferBase::resetLockAll( )
{
	BW_GUARD;
	std::list< DynamicVertexBufferBase* >::iterator it = dynamicVBS_.begin();

	while( it != dynamicVBS_.end() )
	{
		(*it)->resetLock();
		it++;
	}
}


/**
 *	This method deletes all created vertex buffers.
 */
/*static*/ void DynamicVertexBufferBase::fini( )
{
	BW_GUARD;
	s_finalised = true;
	
	// Each time a vertex buffer gets deleted, it removes itself from the list,
	// so just looping until the list is empty.
	while (!dynamicVBS_.empty())
	{
		delete dynamicVBS_.front();
	}
}


void DynamicVertexBufferBase::deleteUnmanagedObjects( )
{
	release();
}

DynamicVertexBufferBase::DynamicVertexBufferBase()
: locked_( false ),
  lockBase_( 0 ),
  maxElements_( 2000 ),
  lockIndex_( 0 ),
  reset_( true )
{
	BW_GUARD;
	dynamicVBS_.push_back( this );
}

DynamicVertexBufferBase::~DynamicVertexBufferBase()
{
	BW_GUARD;
	release();
	std::list< DynamicVertexBufferBase* >::iterator it = std::find( dynamicVBS_.begin(), dynamicVBS_.end(), this );
	if( it != dynamicVBS_.end() )
		dynamicVBS_.erase( it );
}


/**
 *	This method returns a DynamicVertexBuffer instance.
 */
template < class VV >
DynamicVertexBuffer< VV > &
DynamicVertexBuffer< VV >::instance()
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( !s_finalised )
	{
		MF_EXIT( "access after finalisation" );
	}

	// The static instance is created here, it registers itself in the base
	// class static list of vertex buffers, and gets deleted when the base
	// class fini method is called.
	static DynamicVertexBuffer<VV>* s_instance = 
										new DynamicVertexBuffer<VV>();
	return *s_instance;
}


/**
 *	This method returns a DynamicSoftwareVertexBuffer instance.
 */
template < class VV >
DynamicSoftwareVertexBuffer< VV > &
DynamicSoftwareVertexBuffer< VV >::instance()
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( !s_finalised )
	{
		MF_EXIT( "access after finalisation" );
	}

	// The static instance is created here, it registers itself in the base
	// class static list of vertex buffers, and gets deleted when the base
	// class fini method is called.
	static DynamicSoftwareVertexBuffer<VV>* s_instance =
										new DynamicSoftwareVertexBuffer<VV>();
	return *s_instance;
}


HRESULT DynamicVertexBufferBase::create( )
{
	BW_GUARD;
	// release the buffer just in case
	release();
	HRESULT hr = E_FAIL;

	// is the device ready?
	if( Moo::rc().device() != NULL )
	{
		Moo::VertexBuffer buffer;

		DWORD usage = D3DUSAGE_DYNAMIC;
		if (softwareBuffer_)
			usage |= D3DUSAGE_SOFTWAREPROCESSING;
		if (readOnly_)
			usage |= D3DUSAGE_WRITEONLY;

		D3DPOOL pool;

		if (softwareBuffer_)
			pool = D3DPOOL_SYSTEMMEM;
		else
			pool = D3DPOOL_DEFAULT;

		// Try to create the vertexbuffer with maxElements_ number of elements
		if( SUCCEEDED( hr = buffer.create( maxElements_ * vertexSize_, usage, 0, pool, "vertex buffer/dynamic" ) ) )
		{
			vertexBuffer_ = buffer;
			lockBase_ = 0;
			locked_ = false;
			memoryCounterAdd( dynamicVB );
			memoryClaim( buffer );
		}
	}
	return hr;
}

BYTE* DynamicVertexBufferBase::lock( uint32 nLockElements )
{
	BW_GUARD_PROFILER( DynamicVertexBufferBase_lock );

	IF_NOT_MF_ASSERT_DEV( nLockElements != 0 )
	{
		return NULL;
	}

	IF_NOT_MF_ASSERT_DEV( locked_ == false )
	{
		return NULL;
	}

	BYTE* lockedVertices = NULL;
	// Only try to lock if the device is ready.
	if( Moo::rc().device() != NULL )
	{
		bool lockBuffer = true;

		// Can we fit the number of elements into the vertexbuffer?
		if( maxElements_ < nLockElements )
		{
			// If not release the buffer so it can be recreated with the right size.
			release();
			maxElements_ = nLockElements;
		}

		// Do we have a buffer?
		if( !vertexBuffer_.valid() )
		{
			// Create a new buffer
			if( FAILED( create( ) ) )
			{
				// Something went wrong, do not try to lock the buffer.
				lockBuffer = false;
			}
		}

		// Should we attempt to lock?
		if( lockBuffer )
		{
			DWORD lockFlags = 0;

			// Can we fit our vertices in the sofar unused portion of the vertexbuffer?

			if (!lockFromStart_)
			{
				if( ( lockBase_ + nLockElements ) <= maxElements_ )
				{
					// Lock from the current position
					lockFlags = D3DLOCK_NOOVERWRITE;
				}
				else
				{
					// Lock from the beginning.
					lockFlags = D3DLOCK_DISCARD;
				}
			}
			else if (lockModeDiscard_)
			{
				lockFlags = D3DLOCK_DISCARD;
			}

			BYTE* vertices = NULL;

			// Try to lock the buffer, if we succeed update the return value to use the locked vertices
			if( SUCCEEDED( vertexBuffer_.lock( 0, nLockElements * vertexSize_, (void**)&vertices, lockFlags ) ) )
			{				
				lockIndex_ = 0;
				lockBase_ += nLockElements;
				lockedVertices = vertices;
				locked_ = true;
			}
		}
	}
	return lockedVertices;
}

BYTE* DynamicVertexBufferBase::lock2( uint32 nLockElements )
{
	BW_GUARD_PROFILER( DynamicVertexBufferBase_lock2 );

	IF_NOT_MF_ASSERT_DEV( nLockElements != 0 )
	{
		return NULL;
	}

	IF_NOT_MF_ASSERT_DEV( locked_ == false )
	{
		return NULL;
	}

	BYTE* lockedVertices = NULL;

	// Only try to lock if the device is ready.
	if( Moo::rc().device() != NULL )
	{
		bool lockBuffer = true;

		// Can we fit the number of elements into the vertexbuffer?
		if( reset_ )
		{
			lockBase_ = 0;
		}
		
		if( maxElements_ < nLockElements + lockBase_  )
		{
			int newSize = nLockElements + lockBase_;

			// If not release the buffer so it can be recreated with the right size.
			release();
			maxElements_ = newSize;
		}

		// Do we have a buffer?
		if( !vertexBuffer_.valid() )
		{
			// Create a new buffer
			if( FAILED( create( ) ) )
			{
				// Something went wrong, do not try to lock the buffer.
				lockBuffer = false;
			}
		}

		// Should we attempt to lock?
		if( lockBuffer )
		{
			DWORD lockFlags = 0;

			// Can we fit our vertices in the sofar unused portion of the vertexbuffer?
			if( (!reset_) && (( lockBase_ + nLockElements ) <= maxElements_) )
			{
				// Lock from the current position
				lockFlags = D3DLOCK_NOOVERWRITE;
			}
			else
			{
				// Lock from the beginning.
				lockFlags = D3DLOCK_DISCARD;
				lockBase_ = 0;
				reset_ = false;
			}

			BYTE* vertices = NULL;

			// Try to lock the buffer, if we succeed update the return value to use the locked vertices
			if( SUCCEEDED( vertexBuffer_.lock( lockBase_ * vertexSize_, nLockElements * vertexSize_, (void**)&vertices, lockFlags ) ) )
			{
				lockIndex_ = lockBase_;
				lockBase_ += nLockElements;
				lockedVertices = vertices;
				locked_ = true;
			}
			
		}
	}
	return lockedVertices;
}

std::ostream& operator<<(std::ostream& o, const DynamicVertexBufferBase& t)
{
	o << "DynamicVertexBufferBase\n";
	return o;
}

// I hate VC6
template class DynamicVertexBuffer<VertexXYZNUVI>;
template class DynamicVertexBuffer<VertexXYZNUVITBPC>;
template class DynamicVertexBuffer<VertexXYZNUVIIIWWPC>;
template class DynamicVertexBuffer<VertexXYZNUVIIIWWTBPC>;
template class DynamicVertexBuffer<VertexXYZNUV2>;
template class DynamicVertexBuffer<VertexXYZNUVTB>;
template class DynamicVertexBuffer<VertexXYZND>;
template class DynamicVertexBuffer<VertexXYZNDS>;

template class DynamicSoftwareVertexBuffer<VertexXYZNUVI>;
template class DynamicSoftwareVertexBuffer<VertexXYZNUVIIIWWPC>;
template class DynamicSoftwareVertexBuffer<VertexXYZNUVITBPC>;
template class DynamicSoftwareVertexBuffer<VertexXYZNUVIIIWWTBPC>;

template <class VertexType> 
DynamicVertexBufferBase2<VertexType>& 
DynamicVertexBufferBase2<VertexType>::instance()
{
	IF_NOT_MF_ASSERT_DEV( !s_finalised )
	{
		MF_EXIT( "access after finalisation" );
	}
	if( Moo::rc().mixedVertexProcessing() )
		return DynamicSoftwareVertexBuffer<VertexType>::instance();
	return DynamicVertexBuffer<VertexType>::instance();
}

template class DynamicVertexBufferBase2<VertexTex7>;
template class DynamicVertexBufferBase2<VertexXYZUV2>;
template class DynamicVertexBufferBase2<VertexTUV>;
template class DynamicVertexBufferBase2<VertexXYZUV>;
template class DynamicVertexBufferBase2<VertexUV4>;
template class DynamicVertexBufferBase2<VertexXYZNDUV>;
template class DynamicVertexBufferBase2<VertexXYZDUV2>;
template class DynamicVertexBufferBase2<VertexTLUV>;
template class DynamicVertexBufferBase2<VertexTLUV2>;
template class DynamicVertexBufferBase2<VertexXYZ>;
template class DynamicVertexBufferBase2<VertexXYZNUV>;
template class DynamicVertexBufferBase2<VertexXYZNUVTBPC>;
template class DynamicVertexBufferBase2<VertexXYZL>;
template class DynamicVertexBufferBase2<VertexTL>;
template class DynamicVertexBufferBase2<VertexXYZDUV>;
template class DynamicVertexBufferBase2<VertexXYZDSUV>;
template class DynamicVertexBufferBase2<VertexXYZDP>;
template class DynamicVertexBufferBase2<VertexXYZND>;
template class DynamicVertexBufferBase2< FourTapVertex >;
template class DynamicVertexBufferBase2< EightTapVertex >;

}

// dynamic_vertex_buffer.cpp
