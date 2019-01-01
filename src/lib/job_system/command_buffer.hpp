/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOO_COMMAND_BUFFER_HPP
#define MOO_COMMAND_BUFFER_HPP

#include "cstdmf/stdmf.hpp"
#include <xmmintrin.h>

//-----------------------------------------------------------------------------

#if ENABLE_STACK_TRACKER

#include "cstdmf/stack_tracker.hpp"
#include <map>

class TraceCount
{
public:
	void addTrace( uint size );
	void reset();
	void print();

private:
	struct Trace
	{
		uint count_;
		uint size_;
	};

	typedef std::map<std::string, Trace> TraceMap;

	TraceMap traces_;
};

inline void TraceCount::addTrace( uint size )
{
	std::string stack = StackTracker::buildReport();

	if ( traces_.count( stack ) )
	{
		traces_[stack].count_++;
		traces_[stack].size_ += size;
	}
	else
	{
		traces_[stack].count_ = 1;
		traces_[stack].size_ = size;
	}
}

inline void TraceCount::print()
{
	for ( TraceMap::iterator it = traces_.begin(); it != traces_.end(); ++it )
	{
		DEBUG_MSG( "%s: [%d, %d]\n", it->first.c_str(), it->second.count_, it->second.size_ );
	}
}

inline void TraceCount::reset()
{
	traces_.clear();
}

#endif

//-----------------------------------------------------------------------------

class CommandBuffer
{
public:
	void init( uint maxSize );
	void fini();

	void reset();

	void nextRead();
	void nextWrite();

	uint getRemaining();
	void* getCurrent();
	void seek( uint bytes );
	void skipPadding();

	void* getCurrentWrite();
	void seekWrite( uint bytes );
	void writePadding();

	void writeRaw( const void* data, uint size )
	{
		prepWrite( size );
		memcpy( writePtr(), data, size );
		writeOffset_ += size;
		writeRawSize_ += size;
	}

	template <class A>
	void write( A a )
	{
		prepWrite( sizeof( A ) );
		writeObject( a );
	}

	template <class A, class B>
	void write( A a, B b )
	{
		prepWrite( sizeof( A ) + sizeof( B ) );
		writeObject( a );
		writeObject( b );
	}

	template <class A, class B, class C>
	void write( A a, B b, C c )
	{
		prepWrite( sizeof( A ) + sizeof( B ) + sizeof( C ) );
		writeObject( a );
		writeObject( b );
		writeObject( c );
	}

	template <class A, class B, class C, class D>
	void write( A a, B b, C c, D d )
	{
		prepWrite( sizeof( A ) + sizeof( B ) + sizeof( C ) + sizeof( D ) );
		writeObject( a );
		writeObject( b );
		writeObject( c );
		writeObject( d );
	}

	template <class A, class B, class C, class D, class E>
	void write( A a, B b, C c, D d, E e )
	{
		prepWrite( sizeof( A ) + sizeof( B ) + sizeof( C ) + sizeof( D ) +
			sizeof ( E ) );
		writeObject( a );
		writeObject( b );
		writeObject( c );
		writeObject( d );
		writeObject( e );
	}

	template <class A, class B, class C, class D, class E, class F>
	void write( A a, B b, C c, D d, E e, F f )
	{
		prepWrite( sizeof( A ) + sizeof( B ) + sizeof( C ) + sizeof( D ) +
			sizeof ( E )  + sizeof( F ) );
		writeObject( a );
		writeObject( b );
		writeObject( c );
		writeObject( d );
		writeObject( e );
		writeObject( f );
	}

	template <class A, class B, class C, class D, class E, class F, class G>
	void write( A a, B b, C c, D d, E e, F f, G g )
	{
		prepWrite( sizeof( A ) + sizeof( B ) + sizeof( C ) + sizeof( D ) +
			sizeof ( E )  + sizeof( F ) + sizeof( G ) );
		writeObject( a );
		writeObject( b );
		writeObject( c );
		writeObject( d );
		writeObject( e );
		writeObject( f );
		writeObject( g );
	}

	template <class A, class B, class C, class D, class E, class F, class G, class H>
	void write( A a, B b, C c, D d, E e, F f, G g, H h )
	{
		prepWrite( sizeof( A ) + sizeof( B ) + sizeof( C ) + sizeof( D ) +
			sizeof ( E )  + sizeof( F ) + sizeof( G ) + sizeof( H ) );
		writeObject( a );
		writeObject( b );
		writeObject( c );
		writeObject( d );
		writeObject( e );
		writeObject( f );
		writeObject( g );
		writeObject( h );
	}

	template <class A> A read()
	{
		A in = *((A*)readPtr());
		readOffset_ += sizeof( A );
		_mm_prefetch( ( char* )readPtr() + 128, _MM_HINT_T0 );
		return in;
	}

	static void memcpyNTA( void* dst, const void* src, uint size );

	static const uint32 NUM_BUFFERS = 2;

private:
	char* writePtr()
	{
		return buffer_[writeIndex_] + writeOffset_;
	}

	char* readPtr()
	{
		return buffer_[readIndex_] + readOffset_;
	}

	static uint roundUp( uint value, uint align )
	{
		return ( value + align - 1 ) & ( ~( align - 1 ) );
	}

	void prepWrite( uint writeSize );

	template <class C>
	void writeObject( C& object )
	{
		C* out = (C*)writePtr();
		*out = object;
		writeOffset_ += sizeof( C );
		writeObjectSize_ += sizeof( C );
	}

private:
	static const uint GROW_STEP = 1*1024*1024;

	char* buffer_[NUM_BUFFERS];
	uint curSize_[NUM_BUFFERS];		// Bytes used in each buffer
	uint maxSize_;

	uint writeOffset_;
	uint readOffset_;

	uint writeIndex_;
	uint readIndex_;

	uint readSize_;

public:
#if ENABLE_STACK_TRACKER
	TraceCount traces_;
#endif
	uint writeObjectSize_;
	uint writeRawSize_;
	uint writeSeekSize_;
};

//-----------------------------------------------------------------------------

inline void CommandBuffer::init( uint maxSize )
{
	writeOffset_ = 0;
	readOffset_ = 0;
	writeIndex_ = 0;
	readIndex_ = 0;
	readSize_ = 0;

	for ( uint i = 0; i < NUM_BUFFERS; i++ )
	{
		buffer_[i] = (char*)bw_virtualAlloc( NULL, maxSize, MEM_RESERVE, PAGE_READWRITE );
		if ( !buffer_[i] )
		{
			CRITICAL_MSG( "Could not allocate command buffer address space\n" );
		}
		curSize_[i] = 0;
	}

	maxSize_ = maxSize;

	// Profiling info
	writeObjectSize_ = 0;
	writeRawSize_ = 0;
	writeSeekSize_ = 0;
}

//-----------------------------------------------------------------------------

inline void CommandBuffer::fini()
{
	for ( uint i = 0; i < NUM_BUFFERS; i++ )
	{
		bw_virtualFree( buffer_[i], 0, MEM_RELEASE );
	}
}

//-----------------------------------------------------------------------------

inline void CommandBuffer::reset()
{
	readSize_ = writeOffset_;

	readIndex_ = ( readIndex_ + 1 ) % NUM_BUFFERS;
	writeIndex_ = ( writeIndex_ + 1 ) % NUM_BUFFERS;

	readOffset_ = 0;
	writeOffset_ = 0;
}

//-----------------------------------------------------------------------------

inline void CommandBuffer::nextRead()
{
	readOffset_ = 0;
	readIndex_ = ( readIndex_ + 1 ) % NUM_BUFFERS;
}

//-----------------------------------------------------------------------------

inline void CommandBuffer::nextWrite()
{
	writeOffset_ = 0;
	writeIndex_ = ( writeIndex_ + 1 ) % NUM_BUFFERS;
}

//-----------------------------------------------------------------------------

inline uint CommandBuffer::getRemaining()
{
	return readSize_ - readOffset_;
}

//-----------------------------------------------------------------------------

inline void* CommandBuffer::getCurrent()
{
	return readPtr();
}

//-----------------------------------------------------------------------------

inline void CommandBuffer::seek( uint bytes )
{
	readOffset_ += bytes;
}

//-----------------------------------------------------------------------------

inline void* CommandBuffer::getCurrentWrite()
{
	return writePtr();
}

//-----------------------------------------------------------------------------

inline void CommandBuffer::skipPadding()
{
	readOffset_ = roundUp( readOffset_, 16 );
}

//-----------------------------------------------------------------------------

inline void CommandBuffer::seekWrite( uint bytes )
{
	prepWrite( bytes );
	writeOffset_ += bytes;
	writeSeekSize_ += bytes;

#if ENABLE_STACK_TRACKER
//	traces_.addTrace( bytes );
#endif
}

//-----------------------------------------------------------------------------

inline void CommandBuffer::writePadding()
{
	writeOffset_ = roundUp( writeOffset_, 16 );
}

//-----------------------------------------------------------------------------

inline void CommandBuffer::prepWrite( uint writeSize )
{
	uint reqSize = writeOffset_ + writeSize;

	if ( reqSize > curSize_[writeIndex_] )
	{
		// Round requested size up to next grow step
		uint newSize = roundUp( reqSize, GROW_STEP );

		// Make sure we have enough address space for it
		if ( newSize > maxSize_ )
		{
			CRITICAL_MSG( "Command buffer overflow\n" );
		}

		// Commit physical memory
		curSize_[writeIndex_] = newSize;

		void* mem = bw_virtualAlloc( buffer_[writeIndex_], curSize_[writeIndex_],
			MEM_COMMIT, PAGE_READWRITE );

		if ( !mem )
		{
			CRITICAL_MSG( "Command buffer out of memory\n" );
		}
	}
}

//-----------------------------------------------------------------------------

inline void CommandBuffer::memcpyNTA( void* dst, const void* src, uint size )
{
	uint nloops = ( size >> 7 ) + 1;
	uint nloops2 = ( ( size & 127 ) >> 4 ) + 1;

	__asm
	{
		mov				ecx,nloops
		mov				esi,src
		mov				edi,dst

		dec				ecx
		jz				remainder

		mov				edx,128
loop1:
		movdqa			xmm0,[esi]
		movdqa			xmm1,[esi+16]
		movdqa			xmm2,[esi+32]
		movdqa			xmm3,[esi+48]
		movdqa			xmm4,[esi+64]
		movdqa			xmm5,[esi+80]
		movdqa			xmm6,[esi+96]
		movdqa			xmm7,[esi+112]

		movntdq			[edi],xmm0
		movntdq			[edi+16],xmm1
		movntdq			[edi+32],xmm2
		movntdq			[edi+48],xmm3
		movntdq			[edi+64],xmm4
		movntdq			[edi+80],xmm5
		movntdq			[edi+96],xmm6
		movntdq			[edi+112],xmm7

		add				esi,edx
		add				edi,edx

		dec				ecx
		jnz				loop1

remainder:
		mov				ecx,nloops2
		dec				ecx
		jz				done

		mov				edx,16
loop2:
		movdqa			xmm0,[esi]
		add				esi,edx
		movntdq			[edi],xmm0
		add				edi,edx

		dec				ecx
		jnz				loop2

done:
	}
}
		
//-----------------------------------------------------------------------------

#endif // MOO_COMMAND_BUFFER_HPP
