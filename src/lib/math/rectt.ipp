/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RECTT_IPP
#define RECTT_IPP


/**
 *	This constructs an empty rectangle.
 */
template< typename COORDINATE >
inline BW::RectT< COORDINATE >::RectT()
{
	x_.set( 0, 0 );
	y_.set( 0, 0 );
}


/**
 *	This constructs a rectangle.
 *
 *	@param xMin		The minimum x coordinate.
 *	@param yMin		The minimum y coordinate.
 *	@param xMax		The maximum x coordinate.
 *	@param yMax		The maximum y coordinate.
 */
template< typename COORDINATE >
inline BW::RectT< COORDINATE >::RectT( 
	typename BW::RectT< COORDINATE >::Coordinate xMin, 
	typename BW::RectT< COORDINATE >::Coordinate yMin, 
	typename BW::RectT< COORDINATE >::Coordinate xMax, 
	typename BW::RectT< COORDINATE >::Coordinate yMax )
{
	x_.set( xMin, xMax );
	y_.set( yMin, yMax );
}


/**
 *	This gets the area of the rectangle.
 *
 *	@return			The area of the rectangle.
 */
template< typename COORDINATE >
inline typename BW::RectT< COORDINATE >::Coordinate 
	BW::RectT< COORDINATE >::area() const	
{ 
	return x_.length() * y_.length(); 
}


/**
 *	This expands or contracts the rectangle.
 *
 *	@param value	The amount to expand or contract by.  If value is positive
 *					then the rectangle is expanded.  If value is negative then
 *					the rectangle is contracted.
 */
template< typename COORDINATE >
inline void BW::RectT< COORDINATE >::inflateBy( 
	typename BW::RectT< COORDINATE >::Coordinate value )
{
	x_.inflateBy( value );
	y_.inflateBy( value );
}


/**
 *	This returns whether this rectangle intersects another rectangle.
 *
 *	@param rect		The other rectangle to test intersection against.
 *	@return			True if this rectangle and rect intersect.
 */
template< typename COORDINATE >
inline bool BW::RectT< COORDINATE >::intersects( const RectT & rect ) const
{
	return 
		(rect.xMin_ <= xMax_     ) &&
		(xMin_      <= rect.xMax_) &&
		(rect.yMin_ <= yMax_     ) &&
		(yMin_      <= rect.yMax_);
}


/**
 *	This returns whether the given point is inside the rectangle.
 *
 *	@param x		The x coordinate of the point.
 *	@param y		The y coordinate of the point.
 *	@return			True if the point is inside the rectangle, false if it is
 *					outside.  The edges and corners are considered to be 
 *					inside the rectangle.
 */
template< typename COORDINATE >
inline bool BW::RectT< COORDINATE >::contains( 
	typename BW::RectT< COORDINATE >::Coordinate x, 
	typename BW::RectT< COORDINATE >::Coordinate y ) const
{
	return x_.contains( x ) && y_.contains( y );
}


/**
 *	This returns the distance of the given point to the edges of the rectangle.
 *
 *	@param x		The x coordinate of the point.
 *	@param y		The y coordinate of the point.
 *	@return			The L_\inf distance of the point from the edges.
 */
template< typename COORDINATE >
inline float BW::RectT< COORDINATE >::distTo( 
	typename BW::RectT< COORDINATE >::Coordinate x, 
	typename BW::RectT< COORDINATE >::Coordinate y ) const
{
	return std::max( x_.distTo( x ), y_.distTo( y ) );
}


/**
 *	This sets the rectangle to be the intersection of the two rectangles.  If 
 *	the two rectangles do no intersect then the rectangle is set to be 
 *	(0, 0, 0, 0).
 *
 *	@param r1		Rectangle 1.
 *	@param r2		Rectangle 2.
 */
template< typename COORDINATE >
inline void BW::RectT< COORDINATE >::setToIntersection( 
	const RectT & r1,
	const RectT & r2 )
{
	xMin_ = std::max( r1.xMin_, r2.xMin_ );
	yMin_ = std::max( r1.yMin_, r2.yMin_ );
	xMax_ = std::min( r1.xMax_, r2.xMax_ );
	yMax_ = std::min( r1.yMax_, r2.yMax_ );

	if ((xMax_ < xMin_) || (yMax_ < yMin_))
	{
		x_.set( 0.f, 0.f );
		y_.set( 0.f, 0.f );
	}
}


/**
 *	This sets the rectangle to be the smallest rectangle containing the two
 *	rectangles.
 *
 *	@param r1		Rectangle 1.
 *	@param r2		Rectangle 2.		
 */
template< typename COORDINATE >
inline void BW::RectT< COORDINATE >::setToUnion( 
	const RectT & r1, 
	const RectT & r2 )
{
	xMin_ = std::min( r1.xMin_, r2.xMin_ );
	yMin_ = std::min( r1.yMin_, r2.yMin_ );
	xMax_ = std::max( r1.xMax_, r2.xMax_ );
	yMax_ = std::max( r1.yMax_, r2.yMax_ );
}


/**
 *	This gets the given range of the rectangle.
 *
 *	@param getY		Determines whether we get the y or x range.
 *	@return			The y range of the rectangle if getY is true,
 *					the x range of the rectangle if getY is false.
 */
template< typename COORDINATE >
inline typename BW::RectT< COORDINATE >::Range & 
	BW::RectT< COORDINATE >::range1D( bool getY )
{ 
	return getY ? y_ : x_; 
}


/**
 *	This gets the given range of the rectangle.
 *
 *	@param getY		Determines whether we get the y or x range.
 *	@return			The y range of the rectangle if getY is true,
 *					the x range of the rectangle if getY is false.
 */
template< typename COORDINATE >
inline const typename BW::RectT< COORDINATE >::Range & 
	BW::RectT< COORDINATE >::range1D( bool getY ) const	
{ 
	return getY ? y_ : x_; 
}


/**
 *	This gets the x minimum of the rectangle.
 *
 *	@return			The x minimum of the rectangle.
 */
template< typename COORDINATE >
inline typename BW::RectT< COORDINATE >::Coordinate 
	BW::RectT< COORDINATE >::xMin() const
{ 
	return xMin_; 
}


/**
 *	This gets the x maximum of the rectangle.
 *
 *	@return			The x maximum of the rectangle.
 */
template< typename COORDINATE >
inline typename BW::RectT< COORDINATE >::Coordinate 
	BW::RectT< COORDINATE >::xMax() const
{ 
	return xMax_; 
}


/**
 *	This gets the y minimum of the rectangle.
 *
 *	@return			The y minimum of the rectangle.
 */
template< typename COORDINATE >
inline typename BW::RectT< COORDINATE >::Coordinate 
	BW::RectT< COORDINATE >::yMin() const
{ 
	return yMin_; 
}


/**
 *	This gets the y maximum of the rectangle.
 *
 *	@return			The y maximum of the rectangle.
 */
template< typename COORDINATE >
inline typename BW::RectT< COORDINATE >::Coordinate 
	BW::RectT< COORDINATE >::yMax() const
{ 
	return yMax_; 
}


/**
 *	This sets the x minimum of the rectangle.
 *
 *	@param value	The new x minimum.
 */
template< typename COORDINATE >
inline void BW::RectT< COORDINATE >::xMin(
	typename BW::RectT< COORDINATE >::Coordinate value )
{ 
	xMin_ = value; 
}


/**
 *	This sets the x maximum of the rectangle.
 *
 *	@param value	The new x maximum.
 */
template< typename COORDINATE >
inline void BW::RectT< COORDINATE >::xMax(
	typename BW::RectT< COORDINATE >::Coordinate value )
{ 
	xMax_ = value; 
}


/**
 *	This sets the y minimum of the rectangle.
 *
 *	@param value	The new y minimum.
 */
template< typename COORDINATE >
inline void BW::RectT< COORDINATE >::yMin(
	typename BW::RectT< COORDINATE >::Coordinate value )
{ 
	yMin_ = value; 
}


/**
 *	This sets the y maximum of the rectangle.
 *
 *	@param value	The new y maximum.
 */
template< typename COORDINATE >
inline void BW::RectT< COORDINATE >::yMax(
	typename BW::RectT< COORDINATE >::Coordinate value )
{ 
	yMax_ = value; 
}


/**
 *	This gets the x range of the rectangle.
 *
 *	@return			The x range of the rectangle.
 */
template< typename COORDINATE >
inline const typename BW::RectT< COORDINATE >::Range & 
	BW::RectT< COORDINATE >::xRange() const
{ 
	return x_; 
}


/**
 *	This gets the y range of the rectangle.
 *
 *	@return			The y range of the rectangle.
 */
template< typename COORDINATE >
inline const typename BW::RectT< COORDINATE >::Range & 
	BW::RectT< COORDINATE >::yRange() const
{ 
	return y_; 
}


#endif // RECTT_IPP
