/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "animation_channel.hpp"

#ifndef CODE_INLINE
#include "animation_channel.ipp"
#endif

#include "cstdmf/binaryfile.hpp"

DECLARE_DEBUG_COMPONENT2( "Moo", 0 )

namespace Moo
{

/**
 *	Constructor
 */
AnimationChannel::AnimationChannel()
{
}

/**
 *	Copy constructor
 */
AnimationChannel::AnimationChannel( const AnimationChannel & other )
{
	identifier_ = other.identifier_;
}

/**
 *	Destructor
 */
AnimationChannel::~AnimationChannel()
{
}


bool AnimationChannel::load( BinaryFile & bf )
{
	bf >> identifier_;
	return !!bf;
}

bool AnimationChannel::save( BinaryFile & bf ) const
{
	bf << identifier_;
	return !!bf;
}


/**
 *	This static method constructs a new channel of the given type.
 */
AnimationChannel * AnimationChannel::construct( int type )
{
	BW_GUARD;
	ChannelTypes::iterator found = s_channelTypes_->find( type );
	if (found != s_channelTypes_->end())
		return (*found->second)();

	ERROR_MSG( "AnimationChannel::construct: "
		"Channel type %d is unknown.\n", type );
	return NULL;
	/*
	switch (type )
	{
	case 0:
		return new DiscreteAnimationChannel();
	case 1:
		return new InterpolatedAnimationChannel();
	case 2:
		return new MorphAnimationChannel();
	case 3:
		return new InterpolatedAnimationChannel( false );
	case 4:
		return new InterpolatedAnimationChannel( true, true );
	case 5:
		return new StreamedAnimationChannel();
	}

	return NULL;
	*/
}

/**
 *	Static method to register a channel type
 */
void AnimationChannel::registerChannelType( int type, Constructor cons )
{
	BW_GUARD;
	if (s_channelTypes_ == NULL) s_channelTypes_ = new ChannelTypes();
	(*s_channelTypes_)[ type ] = cons;
	INFO_MSG( "AnimationChannel: registered type %d\n", type );
}

/*static*/ void AnimationChannel::fini()
{
	BW_GUARD;
	delete s_channelTypes_;
	s_channelTypes_ = NULL;
}

/// static initialiser
AnimationChannel::ChannelTypes * AnimationChannel::s_channelTypes_ = NULL;

extern int MorphAnimationChannel_token;
extern int CueChannel_token;
static int s_channelTokens = MorphAnimationChannel_token | CueChannel_token;

} // namespace Moo

// animation_channel.cpp
