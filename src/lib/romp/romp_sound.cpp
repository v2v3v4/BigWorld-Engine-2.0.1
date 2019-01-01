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

#pragma warning(disable: 4786)	// remove "truncated identifier" warnings from STL
#pragma warning(disable: 4503)	// class name too long

#include "particle/particle_system_manager.hpp"
#include "fmodsound/sound_manager.hpp"

#include "romp_sound.hpp"

SmartPointer< RompSound > RompSound::soundProvider_ = NULL;

/*virtual*/ void RompSound::playParticleSound(	const char*		label,
									const Vector3&	pos,
									float			velocitySq,
									int				srcIdx,
									uint			materialKind)
{
#if FMOD_SUPPORT
	SoundManager::instance().play( label, pos );
#endif // FMOD_SUPPORT
}

/*static*/ SmartPointer < RompSound > RompSound::getProvider()
{
	if (soundProvider_)
		return soundProvider_;
	else // Falback to the default...
		return ParticleSystemManager::instance().rompSoundDefault();
}

/*static*/ void RompSound::setProvider( RompSound* soundProv )
{
	soundProvider_ = soundProv;
}


//romp_sound.cpp
