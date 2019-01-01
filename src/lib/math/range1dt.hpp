/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RANGE1DT_HPP
#define RANGE1DT_HPP


namespace BW
{


/**
 *	This class is used to represent a 1 dimensional range.
 */
template<typename COORDINATE>
class Range1DT
{
public:
	typedef COORDINATE		Coordinate;

	union
	{
		Coordinate	data_[2];
		struct { Coordinate min_, max_; };
	};

	void			set( Coordinate minValue, Coordinate maxValue );

	Coordinate &	operator[]( int i );
	Coordinate		operator[]( int i ) const;

	Coordinate		length() const;
	Coordinate		midPoint() const;

	void			inflateBy( Coordinate value );

	bool			contains( Coordinate pt ) const;
	bool			contains( const Range1DT & range ) const;

	Coordinate		distTo( Coordinate pt ) const;
};


typedef Range1DT< float >	Range1D;


} // namespace BW


#include "range1dt.ipp"


#endif // RANGE1DT_HPP
