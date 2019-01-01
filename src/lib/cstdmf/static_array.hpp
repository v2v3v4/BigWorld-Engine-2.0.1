/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __CSTDMF_STATIC_ARRAY_HP___
#define __CSTDMF_STATIC_ARRAY_HP___

#include "stdmf.hpp"
#include "debug.hpp"

/**
 * This is a simple constant size array - essentially a checked wrapper around
 * a C style array with some "sugar" methods. 
 *
 * This array has no additional space overhead, and range checking is only 
 * done in DEBUG configurations.
 *
 * Example usage: 
 *	See unit tests.
 */
template < typename TYPE, size_t COUNT >
class StaticArray
{
public:
	typedef TYPE ElementType ;
	typedef TYPE value_type;
	typedef TYPE* iterator;
	typedef const TYPE * const_iterator;
	typedef TYPE& reference;
	typedef const TYPE& const_reference;
	
	enum {	ARRAY_SIZE = COUNT };
	
	StaticArray< TYPE, COUNT>( size_t size = COUNT ) : 
		size_( size )
	{
	}

	/**
	 * These methods provide indexed access.
	 */
	inline ElementType& operator[] (const size_t i )
	{
		MF_ASSERT_DEBUG( i < size_ );
		return data_[i];
	}
	
	inline const ElementType& operator[] (const size_t i ) const
	{
		MF_ASSERT_DEBUG( i < size_ );
		return data_[i];
	}
	
	/**
	 * This method returns the number of elements in this array.
	 */
	inline size_t size() const
	{
		return size_;
	}
	
	void resize( size_t size ) {size_ = size;}
	
	/**
	 * This method fills all elements of this array with a given value.
	 */
	inline void assign( ElementType assignValue )
	{
		for ( size_t i = 0; i < size_; i++ )
		{
			data_[i] = assignValue;
		}
	}
	
	iterator begin() {return data_;}
	const_iterator begin() const {return data_;}
	
	typename StaticArray<TYPE, COUNT>::iterator end() 
	{return data_+size_;}
	typename StaticArray<TYPE, COUNT>::const_iterator end() const 
	{return data_+size_;}
		
private:
	size_t size_;
	ElementType data_[ ARRAY_SIZE ];
};


#endif
