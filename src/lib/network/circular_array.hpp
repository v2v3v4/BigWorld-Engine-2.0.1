/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CIRCULAR_ARRAY_HPP
#define CIRCULAR_ARRAY_HPP

#include "cstdmf/stdmf.hpp"

namespace Mercury
{

/**
 *	@internal
 *	A template class to wrap a circular array of various sizes,
 *	as long as the size is a power of two.
 */
template <class T> class CircularArray
{
public:
	typedef CircularArray<T> OurType;

	CircularArray( uint size ) : data_( new T[size] ), mask_( size-1 ) 
	{ 
		memset( data_, 0, sizeof(T) * this->size() );
	}
	~CircularArray()	{ delete [] data_; }

	uint size() const	{ return mask_+1; }

	const T & operator[]( uint n ) const	{ return data_[n&mask_]; }
	T & operator[]( uint n )				{ return data_[n&mask_]; }
	void swap( OurType & other )
	{
		T * data = data_;
		uint mask = mask_;

		data_ = other.data_;
		mask_ = other.mask_;

		other.data_ = data;
		other.mask_ = mask;
	}

	void inflateToAtLeast( size_t newSize )
	{
		if (newSize > this->size())
		{
			size_t size = this->size();
			while (newSize > size)
			{
				size *= 2;
			}

			OurType newWindow( size );
			this->swap( newWindow );
		}
	}

	void doubleSize( uint32 startIndex )
	{
		OurType newWindow( this->size() * 2 );

		for (size_t i = 0; i < this->size(); ++i)
		{
			newWindow[ startIndex + i ] = (*this)[ startIndex + i ];
		}

		this->swap( newWindow );
	}
private:
	CircularArray( const OurType & other );
	OurType & operator=( const OurType & other );

	T * data_;
	uint mask_;
};

} // namespace Mercury

#endif // CIRCULAR_ARRAY_HPP
