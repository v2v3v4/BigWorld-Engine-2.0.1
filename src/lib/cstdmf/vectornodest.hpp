/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef VECTORNODEST_HPP
#define VECTORNODEST_HPP


#include <vector>
#include <algorithm>

#include "cstdmf/avector.hpp"

#include "cstdmf/stdmf.hpp"
#include "cstdmf/debug.hpp"

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif


/**
 *  This class implements a vector that doesn't delete its elements.
 */
template<class _Ty, class _A = std::allocator<_Ty>, class _B = std::vector< _Ty, _A > >
class VectorNoDestructor : public _B
{
	typedef _B BASE_TYPE;
	typedef typename BASE_TYPE::size_type size_type;

public:
	typedef typename BASE_TYPE::iterator iterator;
	typedef typename BASE_TYPE::const_iterator const_iterator;
	typedef typename BASE_TYPE::reverse_iterator reverse_iterator;
	typedef typename BASE_TYPE::const_reverse_iterator const_reverse_iterator;

	VectorNoDestructor() :
		nElements_( 0 )
	{

	}

	INLINE void push_back( const _Ty & element )
	{
		MF_ASSERT_DEBUG( nElements_ >= 0 );
		if (BASE_TYPE::size() <= (size_type)nElements_)
		{
			BASE_TYPE::push_back( element );
			nElements_ = BASE_TYPE::size();
		}
		else
		{
			*( this->begin() + nElements_ ) = element;
			nElements_++;
		}
	}

	INLINE iterator end()
	{
		return this->begin() + nElements_;
	}

	INLINE const_iterator end() const
	{
		return this->begin() + nElements_;
	}

	INLINE reverse_iterator rbegin()
	{
		return reverse_iterator(this->begin() + nElements_);
	}

	INLINE const_reverse_iterator rbegin() const
	{
		return const_reverse_iterator(this->begin() + nElements_);
	}

	INLINE reverse_iterator rend()
	{
		return reverse_iterator(this->begin());
	}

	INLINE const_reverse_iterator rend() const
	{
		return const_reverse_iterator(this->begin());
	}


	INLINE size_type size()
	{
		return nElements_;
	}

	INLINE size_type size() const
	{
		return nElements_;
	}

	INLINE bool empty() const
	{
		return nElements_ == 0;
	}

	INLINE void resize( size_type sz )
	{
		if (BASE_TYPE::size() < sz )
		{
			BASE_TYPE::resize( sz );
		}

		nElements_ = sz;
	}

	INLINE void clear()
	{
		nElements_ = 0;
	}

	INLINE _Ty &back()
	{
		MF_ASSERT_DEBUG( nElements_ > 0 );
		return *(this->begin() + nElements_ - 1 );
	}

	INLINE const _Ty &back() const
	{
		MF_ASSERT_DEBUG( nElements_ > 0 );
		return *(this->begin() + nElements_ - 1 );
	}

	INLINE void pop_back()
	{
		MF_ASSERT_DEBUG( nElements_ > 0 );
		nElements_ --;
	}

	INLINE void erase( typename BASE_TYPE::iterator _P )
	{
		std::copy(_P + 1, end(), _P);
		nElements_ --;
	}

	INLINE void assign( typename BASE_TYPE::iterator _first, typename BASE_TYPE::iterator _last )
	{
		reserve( nElements_ + std::distance(_first, _last ) );
		clear();
		for ( typename BASE_TYPE::iterator _it = _first;
			_it != _last; ++_it )
		{
			push_back( *_it );
		}
	}

private:
	int nElements_;
};







#ifndef _WIN32
#define NOALIGN
#endif


#ifdef NOALIGN
template< class _Ty, class _A = std::allocator<_Ty> >
class AVectorNoDestructor : public VectorNoDestructor< _Ty, _A, std::vector<_Ty,_A> >
{
};

#else

template< class _Ty, class _A = std::aallocator<_Ty> >
class AVectorNoDestructor : public VectorNoDestructor< _Ty, _A, std::avector<_Ty,_A> >
{
};

#endif	// NOALIGN









#endif
