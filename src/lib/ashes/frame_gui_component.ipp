/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// frame_gui_component.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

INLINE const std::string & FrameGUIComponent::edgeTextureName() const
{
	BW_GUARD;
	return edges_[0]->textureName();
}

INLINE void FrameGUIComponent::edgeTextureName( const std::string & name )
{
	BW_GUARD;
	for (int i = 0; i < 4; i++)
	{
		edges_[i]->textureName( name );
	}
}

INLINE const std::string & FrameGUIComponent::cornerTextureName() const
{
	BW_GUARD;
	return corners_[0]->textureName();
}

INLINE void FrameGUIComponent::cornerTextureName( const std::string & name )
{
	BW_GUARD;
	for (int i = 0; i < 4; i++)
	{
		corners_[i]->textureName( name );
	}
}


// frame_gui_component.ipp