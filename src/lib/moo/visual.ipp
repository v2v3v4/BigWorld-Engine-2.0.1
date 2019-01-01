/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// visual.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif



namespace Moo
{

INLINE
NodePtr Visual::rootNode( ) const
{
	return rootNode_;
}

INLINE
AnimationPtr Visual::animation( uint32 i ) const
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( animations_.size() > i )
	{
		return NULL;
	}
	return animations_[ i ];
}

INLINE
uint32 Visual::nAnimations( ) const
{
	return animations_.size();
}

INLINE
void Visual::addAnimation( AnimationPtr animation )
{
	MF_ASSERT_DEV( animation );
	if( animation )
		animations_.push_back( animation );
}

INLINE
const BoundingBox& Visual::boundingBox() const
{
	return bb_;
}

INLINE
void Visual::boundingBox( const BoundingBox& bb )
{
	bb_ = bb;
}

INLINE
uint32 Visual::nPortals( void ) const
{
	return portals_.size();
}

INLINE
const PortalData& Visual::portal( uint32 i ) const
{
	return portals_[ i ];
}

}

// visual.ipp