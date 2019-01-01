/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RANGE1DT_IPP
#define RANGE1DT_IPP


/**
 *	This sets the values of a Range1DT.
 *
 *	@param minValue		The minimum value of the range.
 *	@param maxValue		The maximum value of the range.
 */
template< typename COORDINATE >
inline void BW::Range1DT< COORDINATE >::set( 
	typename BW::Range1DT< COORDINATE >::Coordinate minValue, 
	typename BW::Range1DT< COORDINATE >::Coordinate maxValue )
{
	data_[0] = minValue;
	data_[1] = maxValue;
}


/**
 *	This gets a reference to the i'th coordinate of the range.
 *
 *	@param i			The index of the coordinate to get.
 *	@return				If i is 0 then the minimum value is returned,
 *						if i is 1 then the maximum value is returned.
 *						Other values of i are not valid.
 */
template< typename COORDINATE >
inline typename BW::Range1DT< COORDINATE >::Coordinate& 
	BW::Range1DT< COORDINATE >::operator[]( int i ) 
{ 
	return data_[ i ]; 
}


/**
 *	This gets the i'th coordinate of the range.
 *
 *	@param i			The index of the coordinate to get.
 *	@return				If i is 0 then the minimum value is returned,
 *						if i is 1 then the maximum value is returned.
 *						Other values of i are not valid.
 */
template< typename COORDINATE >
inline typename BW::Range1DT< COORDINATE >::Coordinate 
	BW::Range1DT< COORDINATE >::operator[]( int i ) const
{ 
	return data_[ i ]; 
}


/**
 *	This gets the length of the range.
 *
 *	@return				The length of the range.
 */
template< typename COORDINATE >
inline typename BW::Range1DT< COORDINATE >::Coordinate 
	BW::Range1DT< COORDINATE >::length() const
{ 
	return data_[1] - data_[0]; 
}


/**
 *	This gets the mid point of the range.
 *
 *	@return				The mid point of the range.
 */
template< typename COORDINATE >
inline typename BW::Range1DT< COORDINATE >::Coordinate 
	BW::Range1DT< COORDINATE >::midPoint() const
{ 
	return (max_ + min_)/2; 
}


/**
 *	This expands the range by the given amount.
 *
 *	@param value		The amount to expand or contract by.
 */
template< typename COORDINATE >
inline void BW::Range1DT< COORDINATE >::inflateBy( 
	typename BW::Range1DT< COORDINATE >::Coordinate value )
{
	min_ -= value;
	max_ += value;
	max_ = std::max( min_, max_ );
}


/**
 *	This determines whether the given coordinate lies within the range.
 *
 *	@param pt			The point to test.
 *	@return				True if point lies within the range, false if it is
 *						outside.  The endpoints of	the range are considered to
 *						be inside.  
 */
template< typename COORDINATE >
inline bool BW::Range1DT< COORDINATE >::contains( 
	typename BW::Range1DT< COORDINATE >::Coordinate pt ) const	
{ 
	return (min_ <= pt) && (pt <= max_); 
}


/**
 *	This determines whether one range is inside another.
 *
 *	@param range		The range to test.
 *	@returns			True if the range lies within this range, false if
 *						it has values outside of this range.  The endpoints are
 *						considered to be inside the range.
 */
template< typename COORDINATE >
inline bool BW::Range1DT< COORDINATE >::contains( const Range1DT & range ) const
{ 
	return (min_ <= range.min_) && (range.max_ <= max_); 
}


/**
 *	This determines the distance from a point to the end points of the range.
 *
 *	@param pt			The point whose distance is to be calculated.
 *	@return				The distance of the point from the end points.
 */
template< typename COORDINATE >
inline typename BW::Range1DT< COORDINATE >::Coordinate 
	BW::Range1DT< COORDINATE >::distTo( 
		typename BW::Range1DT< COORDINATE >::Coordinate pt ) const	
{ 
	return std::max( pt - max_, min_ - pt ); 
}


#endif // RANGE1DT_IPP
