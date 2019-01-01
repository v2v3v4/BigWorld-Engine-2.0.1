/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SOUND_MANAGER_HPP
#define SOUND_MANAGER_HPP

#include "fmod_config.hpp"

#if FMOD_SUPPORT
#include "cstdmf/singleton.hpp"
#include "cstdmf/concurrency.hpp"
#include "pyscript/script.hpp"
#include "math/vector3.hpp"
#include "resmgr/datasection.hpp"

#include <map>
#include <string>
#include <vector>
#include <list>

#include <fmod_event.hpp>
#include <fmod_event_net.hpp>
#include <fmod.hpp>
#include <fmod_errors.h>

class PySound;
class PyMusicSystem;
class PyEventGroup;
class PyEventProject;
class PyEventCategory;
class PyEventReverb;

/**
 *	This is the main entry point for the FMOD sound integration. Note
 *	that the sound library has been primarily designed as a Python interface,
 *	so care must be taken when calling function from C++ that are usually
 *	called directly from Python. They can set a Python exception on error
 *	which may result in leaving the Python VM in an exception state (which
 *	should be cleared with PyErr_Print or PyErr_Clear).
 */
class SoundManager : public Singleton< SoundManager >
{
public:
	enum ErrorLevel
	{
		ERROR_LEVEL_SILENT,
		ERROR_LEVEL_WARNING,
		ERROR_LEVEL_EXCEPTION
	};

	typedef FMOD::Event          Event;
	typedef FMOD::EventGroup     EventGroup;
	typedef FMOD::EventParameter EventParameter;
    typedef FMOD::EventCategory  EventCategory;
    typedef FMOD::EventProject   EventProject;
    typedef FMOD::EventReverb    EventReverb;
    typedef FMOD::MusicSystem    MusicSystem;
	typedef FMOD_EVENT_STATE	 EventState;

public:
	SoundManager();
	~SoundManager();

    bool initialise( DataSectionPtr config = NULL );
	void fini();
	bool update( float deltaTime );
	bool isInitialised();
	bool setPath( const std::string &path );

	// Controls whether projects can be unloaded
	void allowUnload( bool b );
	bool allowUnload() const;

	// Occlusion settings
	void terrainOcclusionEnabled( bool b ) { terrainOcclusionEnabled_ = b; }
	bool terrainOcclusionEnabled() const { return terrainOcclusionEnabled_; }
	void modelOcclusionEnabled( bool b ) { modelOcclusionEnabled_ = b; }
	bool modelOcclusionEnabled() const { return modelOcclusionEnabled_; }

	// Velocity checking
	float maxSoundSpeed() const { return maxSoundSpeed_; }
	void maxSoundSpeed( float f ) { maxSoundSpeed_ = f; }

	// if data is empty, register will queue the load in a background thread and then call back with the data
	void registerSoundBank( const std::string &filename, DataSectionPtr data, bool calledFromMainThread = false );
	bool unregisterSoundBank( const std::string &filename );

	void getSoundBanks( std::list< std::string > & soundBanks );
	bool hasSoundBank( const std::string & sbname ) const;

	void getSoundProjects( std::list< std::string > & soundProjects );
	void getSoundGroups( const std::string& project, std::list< std::string > & soundGroups );
	void getSoundNames( const std::string& project, const std::string& group, std::list< std::string > & soundNames );

	// Deprecated api
	bool loadSoundBank( const std::string &project );
	bool unloadSoundBank( const std::string &project );

	// Loading and unloading of event files and resources
	bool loadEventProject( const std::string &project );
	bool unloadEventProject( const std::string &project );

	bool loadWaveData( const std::string &group );
	bool unloadWaveData( const std::string &group );

	// Methods for sound related python objects. 
	// These always return a new reference.
    PyMusicSystem*	pyMusicSystem();
    PyObject*		createPySound( const std::string & path );
    PyObject*		pyEventGroup( const std::string & groupPath );
    PyObject*		pyEventProject( const std::string & projectName );
    PyObject*		pyEventCategory( const std::string & categoryPath );
    PyObject*		pyEventReverb( const std::string & name );

	Event* play( const std::string &name );
	Event* play( const std::string &name, const Vector3 &pos );
    Event* get( const std::string &name, FMOD_EVENT_MODE mode = FMOD_EVENT_DEFAULT);
	Event* get( EventGroup * pGroup, int index );
	void release( Event * pEvent );

	bool setDefaultProject( const std::string &name );

	bool setListenerPosition( const Vector3& position, const Vector3& forward,
		const Vector3& up, float deltaTime );

	void getListenerPosition( Vector3 *pPosition, Vector3 *pVelocity = NULL );

	bool setMasterVolume( float vol );

	void flagEventsForReload(EventGroup *eventGroup);

	bool absPath( const std::string &path, std::string &ret );

	// Access to the internal event system object.
	FMOD::EventSystem* getEventSystemObject();
    
public:

	static float dbToLinearLevel( float db );

	static void errorLevel( ErrorLevel level ) { s_errorLevel_ = level; }
	static ErrorLevel errorLevel() { return s_errorLevel_; }
	static void logError( const char* msg, ... );

	static bool FMOD_ErrCheck( FMOD_RESULT result, const char * format, ... );
	static bool FMOD_PyErrCheck( FMOD_RESULT result, const char * format, ... );
	static PyObject* pyError();

#if ENABLE_JFIQ_HANDLING
private:
    typedef std::list<PySound *> PySounds;
    PySounds justFailIfQuietestList_;
public:
    void addToJFIQList( PySound *pySound );
    void removeFromJFIQList( PySound *pySound );

#endif //ENABLE_JFIQ_HANDLING

private:
	bool loadUnload( const std::string &group, bool load );

	bool parsePath( const std::string &path, FMOD::EventProject **ppProject,
        FMOD::EventGroup **ppGroup, FMOD::Event **ppEvent,
		bool allowLoadProject = true, FMOD_EVENT_MODE mode = FMOD_EVENT_DEFAULT);

    void kill();

private:
	// Controls what happen when we hit an error
	static ErrorLevel s_errorLevel_;

    SimpleMutex	waitingSoundBankMutex_;
	typedef std::map< std::string, DataSectionPtr > SoundBankWaitingList;
    SoundBankWaitingList waitingSoundBanks_;

	typedef std::map< std::string, BinaryPtr > SoundBankMap;
	SoundBankMap soundBankMap_;
    typedef std::list< FMOD::Sound* > SoundList;
    typedef std::map< std::string, SoundList > StreamingSoundBankMap;
    StreamingSoundBankMap streamingSoundBankMap_;

	std::string mediaPath_;

	// Position listener was last set at
    Vector3 lastPosition_;

	// Velocity the listener was last going at (in units per second)
    Vector3 lastVelocity_;

	// Has the last position been set?
    bool lastSet_;

	FMOD::EventSystem* eventSystem_;
    // There can only be one loaded project with music data
    PyMusicSystem* musicSystem_;

	// The FMOD Project used for resolving relatively-named sounds
	FMOD::EventProject* defaultProject_;

	// Mapping used for caching FMOD::EventGroups
	typedef std::pair< FMOD::EventProject*, std::string > Group;
	typedef std::map< Group, EventGroup* > EventGroups;
	EventGroups eventGroups_;

    // List to keep track of all the PyEventCategories being created 
    // so we can clean them up later.
    typedef std::list< PyEventCategory* > PyEventCategories;
    PyEventCategories pyEventCategories_;

    // List to keep track of all the PyEventReverbs being created 
    // so we can clean them up later.
    typedef std::list< PyEventReverb* > PyEventReverbs;
    PyEventReverbs pyEventReverbs_;

	// Mapping of loaded FMOD::EventProjects
	typedef std::map< std::string, FMOD::EventProject* > EventProjects;
	EventProjects eventProjects_;

	typedef std::map< Event*, bool > Events;
	Events events_;

	// Indicates whether we have an FMOD net connection open
	bool listening_;

	// Can soundbanks be unloaded at runtime?
	bool allowUnload_;

	// Occlusion
	bool terrainOcclusionEnabled_;
	bool modelOcclusionEnabled_;

	// Velocity checking
	float maxSoundSpeed_;

	std::vector< std::string > projectFiles_;
};

#endif // FMOD_SUPPORT

#endif // SOUND_MANAGER_HPP
