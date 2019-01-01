/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// visual_bumper.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

// The different vertex formats have different extra information so we 
// need to copy that information
template<class VertexType, class BumpedVertexType> 
INLINE
void copyVertsExtra( const VertexType& v, BumpedVertexType& bv ) {}


template<>
INLINE
void copyVertsExtra( const Moo::VertexXYZNUVIIIWW& v, Moo::VertexXYZNUVIIIWWTB& bv ) 
{
	bv.index_ = v.index_;
	bv.index2_ = v.index2_;
	bv.index3_ = v.index3_;
	bv.weight_ = v.weight_;
	bv.weight2_ = v.weight2_;
}


// For some of the vertex formats, the normal is packed, and therefore needs to be unpacked
template<class NormalFormat>
INLINE
Vector3 unpackTheNormal( NormalFormat normal ) 
{
	return normal;
}


template<>
INLINE
Vector3 unpackTheNormal( uint32 normal ) 
{
	return Moo::unpackNormal( normal );
}

// visual_bumper.ipp
