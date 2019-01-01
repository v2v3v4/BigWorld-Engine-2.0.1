/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VALUE_OR_NULL_HPP
#define VALUE_OR_NULL_HPP

#include "cstdmf/stdmf.hpp"

/**
 *	This class is used to represent values that might also be NULL.
 */
template <class T>
class ValueOrNull
{
public:
	ValueOrNull() : isNull_( true ) {}
	explicit ValueOrNull( const T & x ) : value_( x ), isNull_( false ) {}

	void setNull()				{ isNull_ = true; }
	void setValue( const T& x ) { value_ = x; isNull_ = 0; }

	bool isNull() const			{ return isNull_; }

	const T * get() const		{ return isNull_ ? NULL : &value_; }

protected:
	T value_;
	bool isNull_;
};

#endif // VALUE_OR_NULL_HPP
