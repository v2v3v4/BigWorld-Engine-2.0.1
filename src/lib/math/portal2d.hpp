/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PORTAL2D_HPP
#define PORTAL2D_HPP

#include "cstdmf/vectornodest.hpp"

#include "vector2.hpp"


/**
 *	This class is a vector of points that form a convex portal in 2D.
 *	It is used to determine visibility for portal-based rendering.
 */
class Portal2D
{
public:
	typedef VectorNoDestructor< Vector2 > V2Vector;

	Portal2D();
	Portal2D( const Portal2D & other );
	Portal2D & operator =( const Portal2D & other );
	~Portal2D();


	void setDefaults();
	void addPoint( const Vector2 &v );
	void erasePoints()					{ points_.clear(); }

	bool combine( Portal2D *p1, Portal2D *p2 );
	//bool combine( const Portal2D & p1, const Portal2D & p2 );
	
	bool contains( const Vector2 & v ) const;
	const V2Vector & points() const		{ return points_; }

	uint refs()							{ return refs_; }
	void refs( uint r )					{ refs_ = r; }

private:
	V2Vector	points_;
	uint		refs_;
};

#ifdef CODE_INLINE
#include "portal2d.ipp"
#endif


#endif // PORTAL2D_HPP
