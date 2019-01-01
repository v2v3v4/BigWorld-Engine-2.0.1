/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FLORA_BLOCK_HPP
#define FLORA_BLOCK_HPP

#include "math/boundbox.hpp"

struct FloraVertex;
class Ecotype;
class Vector2;

/**
 * TODO: to be documented.
 */
class FloraBlock
{
public:
	FloraBlock( class Flora* flora );
	void init( const Vector2& pos, uint32 offset );
	uint32 offset() const { return offset_; }	
	void fill( uint32 numVertsAllowed );
	void invalidate();
	bool needsRefill() const		{ return bRefill_; }

	const Vector2&	center() const	{ return center_; }
	void center( const Vector2& c );

	void cull();
	bool culled() const		{ return culled_; }
	void culled( bool c )	{ culled_ = c; }

	int	 blockID() const	{ return blockID_; }
	const BoundingBox& bounds() const	{ return bb_; }
private:
	bool nextTransform( const Vector2& center, Matrix& ret, Vector2& retEcotypeSamplePt );
	Vector2	center_;	
	BoundingBox	bb_;
	bool	culled_;
	bool	bRefill_;
	std::vector<Ecotype*>	ecotypes_;
	int		blockID_;
	uint32	offset_; // number of vertexes offset from the start of the VB
	class Flora* flora_;
};

typedef FloraBlock*	pFloraBlock;

#endif
