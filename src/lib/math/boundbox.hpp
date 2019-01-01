/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BOUNDBOX_HPP
#define BOUNDBOX_HPP

#include "mathdef.hpp"
#include "vector3.hpp"
#include "matrix.hpp"
#include "cstdmf/debug.hpp"

#include <float.h>


/**
 *	BoundingBox, implementation of an axis aligned boundingbox
 */
class BoundingBox
{
public:
	BoundingBox();
	BoundingBox( const Vector3 & min, const Vector3 & max );

	bool operator==( const BoundingBox& bb ) const;
	bool operator!=( const BoundingBox& bb ) const;

	const Vector3 & minBounds() const;
	const Vector3 & maxBounds() const;
	void setBounds( const Vector3 & min, const Vector3 & max );

    float width() const;
    float height() const;
    float depth() const;

	void addYBounds( float y );
	void addBounds( const Vector3 & v );
	void addBounds( const BoundingBox & bb );
    void expandSymmetrically( float dx, float dy, float dz );
    void expandSymmetrically( const Vector3 & v );
	void calculateOutcode( const Matrix & m ) const;    

	Outcode outcode() const;
	Outcode combinedOutcode() const;
	void outcode( Outcode oc );
	void combinedOutcode( Outcode oc );

	void transformBy( const Matrix & transform );

	bool intersects( const BoundingBox & box ) const;
	bool intersects( const Vector3 & v ) const;
	bool intersects( const Vector3 & v, float bias ) const;
	bool intersectsRay( const Vector3 & origin, const Vector3 & dir ) const;
	bool intersectsLine( const Vector3 & origin, const Vector3 & dest ) const;

	bool clip( Vector3 & start, Vector3 & extent, float bloat = 0.f ) const;

	float distance( const Vector3& point ) const;

	INLINE Vector3 centre() const;

	INLINE bool insideOut() const;

private:
	Vector3 min_;
	Vector3 max_;

	mutable Outcode oc_;
	mutable Outcode combinedOc_;

public:
	static const BoundingBox s_insideOut_;
};

#ifdef CODE_INLINE
#include "boundbox.ipp"
#endif




#endif
/*BoundBox.hpp*/
