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
#include "morph_animation_channel.hpp"
#include "morph_vertices.hpp"
#include "cstdmf/binaryfile.hpp"

#ifndef CODE_INLINE
#include "morph_animation_channel.ipp"
#endif


namespace Moo
{

// -----------------------------------------------------------------------------
// Section: MorphAnimationChannel
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
MorphAnimationChannel::MorphAnimationChannel()
{
}


/**
 *	Destructor.
 */
MorphAnimationChannel::~MorphAnimationChannel()
{
	memoryCounterSub( animation );
	memoryClaim( influences_ );
}


void MorphAnimationChannel::nodeless( float time, float blendRatio ) const
{
	BW_GUARD;	
#ifndef MF_SERVER
	if (blendRatio <= 0.f) return;

	float res = 0;
	if (influences_.size())
	{
		if (time < 0)
			res = influences_.front();
		else if (time >= (influences_.size() - 1) )
		{
			res = influences_.back();
		}
		else
		{
			float frac = time - floorf( time );
			int t = int( time - frac );
			res = influences_[t] * (1.f - frac) + influences_[t+1] * frac;
		}
	}

	if (identifier().length())
	{
		MorphVertices::addMorphValue( identifier(), res, blendRatio );
	}
#endif
}

bool MorphAnimationChannel::load( BinaryFile & bf )
{
	BW_GUARD;
	if( !this->AnimationChannel::load( bf ) )
		return false;
	bf.readSequence( influences_ );

	memoryCounterAdd( animation );
	memoryClaim( influences_ );

	return !!bf;
}

bool MorphAnimationChannel::save( BinaryFile & bf ) const
{
	BW_GUARD;
	if( !this->AnimationChannel::save( bf ) )
		return false;
	bf.writeSequence( influences_ );
	return !!bf;
}

void MorphAnimationChannel::preCombine( const AnimationChannel & rOther )
{
}

void MorphAnimationChannel::postCombine( const AnimationChannel & rOther )
{
}

AnimationChannel * MorphAnimationChannel::duplicate() const
{
	BW_GUARD;
	return new MorphAnimationChannel( *this );
}

MorphAnimationChannel::TypeRegisterer MorphAnimationChannel::s_rego_( 2, New );

int MorphAnimationChannel_token;

} // namespace Moo

// morph_animation_channel.cpp
