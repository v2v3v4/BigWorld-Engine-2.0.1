/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// animation.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif



namespace Moo {


	INLINE
	void Animation::totalTime( float time )
	{
		totalTime_ = time;
	}

	INLINE
	float Animation::totalTime( ) const
	{
		return totalTime_;
	}

	INLINE
	const std::string Animation::identifier( ) const
	{
		return identifier_;
	}

	INLINE
	void Animation::identifier( const std::string& identifier )
	{
		identifier_ = identifier;
	}

	INLINE
	const std::string Animation::internalIdentifier( ) const
	{
		return internalIdentifier_;
	}


	INLINE
	void Animation::internalIdentifier( const std::string& ii )
	{
		internalIdentifier_ = ii;
	}


	INLINE
	void Animation::animate( const BlendedAnimations & alist )
	{
		animate( alist.begin(), alist.end() );
	}

};

// animation.ipp
