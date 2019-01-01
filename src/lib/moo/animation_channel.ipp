/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

namespace Moo
{
	INLINE
	const std::string&	AnimationChannel::identifier( ) const
	{
		return identifier_;
	}

	INLINE
	void AnimationChannel::identifier( const std::string& identifier )
	{
		identifier_ = identifier;
	}

}
/*animation_channel.ipp*/
