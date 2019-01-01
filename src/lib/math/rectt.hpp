/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RECTT_HPP
#define RECTT_HPP


#include "range1dt.hpp"


namespace BW
{

/**
 *	This class represents a rectangle in 2 dimensions.
 */
template<typename COORDINATE>
class RectT
{
public:
	typedef COORDINATE				Coordinate;
	typedef Range1DT< Coordinate >	Range;

	RectT();
	RectT( Coordinate xMin, Coordinate yMin, Coordinate xMax, Coordinate yMax );

	Coordinate		area() const;

	void			inflateBy( Coordinate value );

	bool			intersects( const RectT & rect ) const;
	bool			contains( Coordinate x, Coordinate y ) const;

	float			distTo( Coordinate x, Coordinate y ) const;

	void			setToIntersection( const RectT & r1, const RectT & r2 );
	void			setToUnion( const RectT & r1, const RectT & r2 );

	Range &			range1D( bool getY );
	const Range &	range1D( bool getY ) const;

	Coordinate 		xMin() const;
	Coordinate 		xMax() const;
	Coordinate 		yMin() const;
	Coordinate 		yMax() const;

	void 			xMin( Coordinate value );
	void 			xMax( Coordinate value );
	void 			yMin( Coordinate value );
	void 			yMax( Coordinate value );

	const Range &	xRange() const;
	const Range &	yRange() const;

	union
	{
		struct { Coordinate xMin_, xMax_; };
		Range x_;
	};

	union
	{
		struct { Coordinate yMin_, yMax_; };
		Range y_;
	};
};


typedef RectT< int >	RectInt;
typedef RectT< float >	Rect;

} // namespace BW


#include "rectt.ipp"


#endif // RECTT_HPP
