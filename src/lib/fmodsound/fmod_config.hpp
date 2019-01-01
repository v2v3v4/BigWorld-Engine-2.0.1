/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FMOD_CONFIG_HPP
#define FMOD_CONFIG_HPP

// Set this define to 1 if you wish to use the FMOD sound library in your game.
// Please see http://www.fmod.org for tools downloads and licensing details.

#define FMOD_SUPPORT 0

#define ENABLE_JFIQ_HANDLING 1

#if FMOD_VERSION > 0x00042000
# define FMOD_SUPPORT_CUES       1
# define FMOD_SUPPORT_MEMORYINFO 1
#else
# define FMOD_SUPPORT_CUES       0
# define FMOD_SUPPORT_MEMORYINFO 0
#endif

#if defined(MF_SERVER) 
#undef FMOD_SUPPORT
#define FMOD_SUPPORT 0
#endif

#endif // FMOD_CONFIG_HPP
