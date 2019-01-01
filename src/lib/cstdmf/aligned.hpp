/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ALIGNED_HPP
#define ALIGNED_HPP

#if !defined( _WIN32) || defined ( _XBOX )

class Aligned
{
};

#else // !no-op

#include <malloc.h>
#include "cstdmf/debug.hpp"

class __declspec( align( 16 ) ) Aligned
{
public:
	void* operator new( size_t s )
	{
		if (s == 0)
			s = 1;

		return _aligned_malloc( s, 16 );
	}

	void* operator new( size_t s, void* at )
	{
		return at;
	}

	void* operator new[]( size_t s )
	{
		if (s == 0)
			s = 1;

		return _aligned_malloc( s, 16 );
	}

	void operator delete( void* p )
	{
		if (p)
			_aligned_free( p );
	}

	void operator delete( void* p, void* q )
	{
	}

	void operator delete[]( void* p )
	{
		if (p)
			_aligned_free( p );
	}
};

#endif // no-op

#endif // ALIGNED_HPP
