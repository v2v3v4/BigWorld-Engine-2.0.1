/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DEPENDENT_ARRAY_HPP
#define DEPENDENT_ARRAY_HPP

#include "cstdmf/stdmf.hpp"

#include <vector>

/**
 *	This class wraps a block of memory as an array.  The DependentArray is
 *	assumed to be followed with a uint16 size.
 */
template <class C> 
class DependentArray
{
public:
	typedef C value_type;
	typedef value_type * iterator;
	typedef const value_type * const_iterator;
	typedef typename std::vector<C>::size_type size_type;

	/**
	 *	This is the DependentArray default constructor.
	 */
	DependentArray() :
		data_( 0 )
	{
		_size() = 0;
	}

	/**
	 *	This constructs a DependentArray.
	 *
	 *	@param beg		An iterator to the beginning of the array.
	 *	@param end		An iterator to the end of the array.
	 */
	DependentArray( iterator beg, iterator end ) :
		data_( beg )
	{
		_size() = end - beg;
	}

	/**
	 *	This is the DependentArray copy constructor.  Note that copying is
	 *	shallow - the two DependentArrays will point to the same data.
	 *
	 *	@param other	The DependentArray to copy from.
	 */
	DependentArray( const DependentArray & other ) :
		data_( other.data_ )
	{
		_size() = other._size();
	}

	/**
	 *	This is the DependentArray assignment operator.  Note that copying is
	 *	shallow - the two DependentArrays will point to the same data.
	 *
	 *	@param other	The DependentArray to copy from.
	 *	@return			A reference to this object.
	 */
	DependentArray & operator=( const DependentArray & other )
	{
		new (this) DependentArray( other );
		return *this;
	}

	/**
	 *	This is the DependentArray destructor.
	 */
	~DependentArray()
		{ }

	/**
	 *	This gets the size of the DependentArray.
	 *
	 *	@return 		The size of the DependentArray.
	 */
	size_type size() const	{ return _size(); }

	/**
	 *	This gets the i'th element of the array.
	 *
	 *	@param i		The index of the array to get.
	 *	@return			A reference to the i'th element of the array.
	 */
	const value_type & operator[]( int i ) const { return data_[i]; }

	/**
	 *	This gets the i'th element of the array.
	 *
	 *	@param i		The index of the array to get.
	 *	@return			A reference to the i'th element of the array.
	 */
	value_type & operator[]( int i ){ return data_[i]; }

	/**
	 *	This gets the first element of the array.
	 *
	 *	@return			A const reference to the first element of the array.
	 */
	const value_type & front() const{ return *data_; }

	/**
	 *	This gets the first element of the array.
	 *
	 *	@return			A reference to the first element of the array.
	 */
	value_type & front()			{ return *data_; }

	/**
	 *	This gets the last element of the array.
	 *
	 *	@return			A const reference to the last element of the array.
	 */
	const value_type & back() const { return data_[_size()-1]; }

	/**
	 *	This gets the last element of the array.
	 *
	 *	@return			A reference to the last element of the array.
	 */
	value_type & back()				{ return data_[_size()-1]; }

	/**
	 *	This gets a const iterator to the first element of the array.
	 *
	 *	@return			A const iterator to the first element of the array.
	 */
	const_iterator begin() const	{ return data_; }

	/**
	 *	This gets a const iterator that refers to one past the last element of
	 *	the array.
	 *
	 *	@return			A const iterator that refers to one past the last
	 *					element of the array.
	 */
	const_iterator end() const		{ return data_+_size(); }

	/**
	 *	This gets an iterator to the first element of the array.
	 *
	 *	@return			An iterator to the first element of the array.
	 */
	iterator begin()				{ return data_; }

	/**
	 *	This gets a iterator that refers to one past the last element of
	 *	the array.
	 *
	 *	@return			A iterator that refers to one past the last element of
	 *					the array.
	 */
	iterator end()					{ return data_+_size(); }

private:
	value_type * data_;

	uint16 _size() const	{ return *(uint16*)(this+1); }
	uint16 & _size()		{ return *(uint16*)(this+1); }
};

#endif // DEPENDENT_ARRAY_HPP
