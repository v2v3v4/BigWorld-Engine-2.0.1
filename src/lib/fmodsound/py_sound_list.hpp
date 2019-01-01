/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_SOUND_LIST_HPP
#define PY_SOUND_LIST_HPP

#include "fmod_config.hpp"

#if FMOD_SUPPORT
#include "py_sound.hpp"
#include "sound_manager.hpp"

class PySound;

class PySoundList : public std::list< PySound * >
{
public:
    friend class PySound; // to access destructor


	PySoundList();
	~PySoundList();

#if 0
    bool createPySound();
#endif


	void push_back( PySound *pPySound );
	bool update( const Vector3 &pos, const Vector3 &orientation = Vector3::zero(), float deltaTime = 0.f);
    bool removeSound( PySound *pPySound);
    bool removeSound( SoundManager::Event *pEvent );
	bool stopAll();
	void stopOnDestroy( bool enable ) { stopOnDestroy_ = enable; }

protected:
	bool stopOnDestroy_;
};

#endif // FMOD_SUPPORT
    
#endif // PY_SOUND_LIST_HPP