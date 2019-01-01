/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_MUSIC_SYSTEM_HPP
#define PY_MUSIC_SYSTEM_HPP

#include "fmod_config.hpp"
#if FMOD_SUPPORT

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "sound_manager.hpp"

#include <fmod_event.hpp>


/**
 *  A PyMusicSystem  is a wrapper for the FMOD::MusicSystem
 *
 *  Please see the FMOD Designer API documentation and the FMOD Designer
 *  User Manual, both are located in your FMOD install directory.
 */
class PyMusicSystem : public PyObjectPlus
{
	Py_Header( PyMusicSystem, PyObjectPlus )

public:
    PyMusicSystem( SoundManager::MusicSystem * pMusicSystem, PyTypePlus * pType = &PyMusicSystem::s_type_ );

	void fini();

	// PyObjectPlus overrides 
	PyObject*			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );

    //  Methods
    void promptCue( std::string name );
	PY_AUTO_METHOD_DECLARE( RETVOID, promptCue, ARG( std::string, END ) )

    float getParameterValue( const std::string& name );
    PY_AUTO_METHOD_DECLARE( RETDATA, getParameterValue, ARG( std::string, END ) )
    
    void setParameterValue( const std::string& name, float value );
    PY_AUTO_METHOD_DECLARE( RETVOID, setParameterValue, ARG( std::string, ARG( float, END ) ) )

    void setCallback( PyObjectPtr type, PyObjectPtr callback);
	PY_AUTO_METHOD_DECLARE( RETVOID, setCallback, ARG( PyObjectPtr, ARG( PyObjectPtr, END ) ) );

    void reset();
	PY_AUTO_METHOD_DECLARE( RETVOID, reset, END );

    void loadSoundData( bool blocking = true );
	PY_AUTO_METHOD_DECLARE( RETVOID, loadSoundData, OPTARG( bool, true, END ) );

    void freeSoundData( /*bool waitUntilReady = true*/ );
	PY_AUTO_METHOD_DECLARE( RETVOID, freeSoundData, END/*OPTARG( bool, true, END)*/ );

	//  Attributes
    bool muted();
    void muted( bool newMute );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, muted, muted ); 

    bool paused();
    void paused( bool newPaused );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, paused, paused );
    
	float volume();
	void volume( float newValue );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, volume, volume );
    
    unsigned int memoryUsed();
	PY_RO_ATTRIBUTE_DECLARE( memoryUsed(), memoryUsed ); 


protected:
    std::map<FMOD_MUSIC_CALLBACKTYPE, PyObjectPtr> callbackMap_;
    static FMOD_RESULT F_CALLBACK musicCallback(
      FMOD_MUSIC_CALLBACKTYPE  type, 
      void *  param1, 
      void *  param2, 
      void *  userdata);
    void doCallback( FMOD_MUSIC_CALLBACKTYPE type );

    SoundManager::MusicSystem* musicSystem_;

private:
	~PyMusicSystem();
    
    //  Hide copy constructor and assignment operator.
	PyMusicSystem(const PyMusicSystem&);
	PyMusicSystem& operator=(const PyMusicSystem&);

private:
	bool muted_;
	bool paused_;
	float volume_;
};

typedef SmartPointer< PyMusicSystem > PyMusicSystemPtr;

#endif // FMOD_SUPPORT

#endif // PY_MUSIC_SYSTEM_HPP
