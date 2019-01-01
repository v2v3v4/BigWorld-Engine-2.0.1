/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// minimap.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif


INLINE const std::string& Minimap::maskName() const
{
	static std::string noName("");
	if( mask_ )
		return mask_->resourceID();

	return noName;
}


INLINE const std::string& Minimap::simpleEntryMap() const
{
	static std::string noName("");
	if( simpleEntryMap_ )
		return simpleEntryMap_->resourceID();

	return noName;
}


INLINE Vector3 Minimap::viewpointPosition() const
{
	Matrix viewpoint;
	if (viewpoint_)
	{
		viewpoint_->matrix( viewpoint );
	}
	else
	{
		viewpoint = Moo::rc().invView();
	}
	return viewpoint.applyToOrigin();
}



// minimap.ipp