/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "py_sound.hpp"
#include "fmod_config.hpp"
#include "cstdmf/guard.hpp"

#if FMOD_SUPPORT

#include "py_sound_parameter.hpp"
#include "py_event_project.hpp"
#include "py_event_group.hpp"
#include "py_event_category.hpp"
#include "py_sound_list.hpp"

#include "duplo/pymodel.hpp"

#include <fmod_errors.h>
#include <fmod.hpp>

DECLARE_DEBUG_COMPONENT2( "PySound", 0 );

/*~ class FMOD.Sound
 *
 *  A Sound is a handle to a sound event.  It can be used to play, stop,
 *  and adjust parameters on sounds.  It is basically an interface to
 *  FMOD::Event.  For more information about the concept of sound events and
 *  other API intricacies, please see the FMOD Ex API documentation, available
 *  from www.fmod.org.
 *
 *  The one major point of difference between a Sound and an FMOD::Event is
 *  that a Sound can be retriggered by calling play() without calling stop()
 *  in between.  What actually happens when you do this is determined by the
 *  "Max Playbacks" property of the particular underlying Event.  Method calls
 *  and/or attribute accesses (other than play()) on a Sound will always refer
 *  to the most recently triggered Event; if you want to retain handles to
 *  multiple instances of the same Event, you will need to create multiple
 *  Sound objects instead of triggering all the events from a single Sound
 *  object.
 */
PY_TYPEOBJECT( PySound )

PY_BEGIN_METHODS( PySound )

	/*~ function Sound.getParentProject
	 *
	 *  Returns the EventProject which contains this Sound.
	 *
	 *  @return			A PyEventProject object.
	 */
	PY_METHOD( getParentProject )
    
	/*~ function Sound.getParentGroup
	 *
	 *  Returns the EventProject which contains this Sound.
	 *
	 *  @return			A PyEventGroup object.
	 */
	PY_METHOD( getParentGroup )
    
	/*~ function Sound.getParentCategory
	 *
	 *  Returns the PyEventCategory which contains this Sound.
	 *
	 *  @return			A PyEventCategory object.
	 */
	PY_METHOD( getParentCategory )

	/*~ function Sound.param
	 *
	 *  Returns the SoundParameter object corresponding to the parameter of
	 *  the given name.
	 *
	 *  @param	name	The name of the parameter.
	 *
	 *  @return			A SoundParameter object.
	 */
	PY_METHOD( param )

	/*~ function Sound.play
	 *
	 *  Start playing the sound event.
	 *
	 *  @return		A boolean indicating success.
	 */
	PY_METHOD( play )

	/*~ function Sound.stop
	 *
	 *  Stop the sound event.
	 *
	 *  @return		A boolean indicating success.
	 */
	PY_METHOD( stop )

	/*~ function Sound.setCallback
	 *
	 *  Set a callback to be triggered by the FMOD::Event
	 *
	 *  @return		Nothing.
	 */
	PY_METHOD( setCallback )

PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PySound )

	/*~ attribute Sound.muted
	 *
	 *  True if the PySound is muted.
	 *
	 *  @type	Bool
	 */
	PY_ATTRIBUTE( muted )

	/*~ attribute Sound.paused
	 *
	 *  True if the PySound is paused.
	 *
	 *  @type	Bool
	 */
	PY_ATTRIBUTE( paused )

	/*~ attribute Sound.pitch
	 *
	 *  The volume that this sound is (or will be) playing at.
	 *
	 *  @type	Float in the range [0.0, 1.0]
	 */
	PY_ATTRIBUTE( pitch )

	/*~ attribute Sound.volume
	 *
	 *  The volume that this sound is (or will be) playing at.
	 *
	 *  @type	Float in the range [0.0, 1.0]
	 */
	PY_ATTRIBUTE( volume )

	/*~ attribute Sound.duration
	 *
	 *  Returns the length of this sound event in seconds.  Will return < 0 on
	 *  error, which can be caused by asking for the duration of a looping
	 *  sound.
	 *
	 *  @type	Float
	 */
	PY_ATTRIBUTE( duration )

	/*~ attribute Sound.isVirtual
	 *
	 *  True if the sound has lost it's FMOD event handle. This usually occurs when
     *  the max playbacks of the event has been exceeded and the handle has been
     *  stolen. You can attemp to reset the sound to get a valid handle or release 
     *  it if you are finished with it. The properties of the sound cannot be 
     *  accessed when it is virtual. 
     *
     *  If the 'just fail if quietest' max playback behaviour is specified the 
     *  there is no need to reset the sound. BigWorld will try to get a new event
     *  handle every tick. The loudest instances of the event will always be valid.
	 *
	 *  @type	Bool
	 */
	PY_ATTRIBUTE( isVirtual )

	/*~ attribute Sound.name
	 *
	 *  The name of the underlying FMOD::Event.
	 */
	PY_ATTRIBUTE( name )

	/*~ attribute Sound.state
	 *
	 *  Read-only access to the FMOD state of this event.
	 */
	PY_ATTRIBUTE( state )

	/*~ attribute Sound.position
	 *
	 *  Read-write access to the 3D position of this sound event.  Writing to
	 *  this attribute will only have an effect if the event's mode has been set
	 *  to 3D in the FMOD Designer tool.  Reading this attribute on a 2D sound
	 *  will return (0,0,0).
	 */
	PY_ATTRIBUTE( position )

PY_END_ATTRIBUTES()

PY_SCRIPT_CONVERTERS_DECLARE( PySound );
PY_SCRIPT_CONVERTERS( PySound );

static PyFactoryMethodLink PySound_MethodLink("_FMOD", "Sound", &PySound::s_type_);



PySound::PySound( SoundManager::Event *pEvent, const std::string &path,
#if ENABLE_JFIQ_HANDLING
                    bool JFIQ, 
#endif
                    PyTypePlus* pType ) :
	PyObjectPlus( pType ),
	pEvent_( pEvent ),
	pModel_( NULL ),
	pPosition_( NULL ),
	played_( false ),
#if ENABLE_JFIQ_HANDLING
    stopped_( false ),
    justFailIfQuietest_( JFIQ ),
    virtual_( JFIQ ),
#else
    virtual_( false ),
#endif
	reset_( false ),
    unloaded_( false )
{
	BW_GUARD;
	MF_ASSERT( pEvent );

	FMOD_RESULT result;

	result = pEvent->setUserData( this );
    SoundManager::FMOD_ErrCheck(result, "PySound::PySound(): Unable to setUserData\n");

    result = pEvent->setCallback( eventCallback, NULL );
    if (SoundManager::FMOD_ErrCheck(result, "PySound::PySound(): Unable to setCallback\n"))
	{
#if 0   //NB: this was removed so name() works when virtual
		// If unloading is enabled, then we need to store the absolute path to the
		// underlying FMOD::Event* so we can reacquire handles when retriggered.
		if (SoundManager::instance().allowUnload())
#endif
		{
			if (!SoundManager::instance().absPath( path, path_ ))
			{
				ERROR_MSG( "PySound::PySound: "
					"Couldn't get absolute path to sound event\n" );
			}
		}

		// Otherwise, we just store a reference to the parent sound group and the
		// index of this event in that group, as it makes for faster handle
		// re-acquisition.
      
        /*if( !SoundManager::instance().allowUnload() 
#if ENABLE_JFIQ_HANDLING
            || justFailIfQuietest_ 
#endif
                                    )
        */
		{
			result = pEvent_->getInfo( &index_, NULL, NULL );
			SoundManager::FMOD_ErrCheck(result, "PySound::PySound: "
					"Couldn't get event index");

			result = pEvent_->getParentGroup( &pGroup_ );
            SoundManager::FMOD_ErrCheck(result, "PySound::PySound: "
					"Couldn't get parent group");
		}
	}   
  
#if ENABLE_JFIQ_HANDLING
    if ( justFailIfQuietest_ )
        SoundManager::instance().addToJFIQList( this );
#endif
}

PySound::~PySound()
{
	BW_GUARD;

	if (pPosition_)
	{
		delete pPosition_;
	}

	if ( !SoundManager::pInstance() || !SoundManager::instance().isInitialised() )
	{
		WARNING_MSG("PySound::~PySound: Destructor called after SoundManager::fini!\n");
		return;
	}
#if ENABLE_JFIQ_HANDLING
    if ( justFailIfQuietest_ )
        SoundManager::instance().removeFromJFIQList( this );
#endif

	if ( pEvent_ )
	{
		pEvent_->setUserData( NULL );
        pEvent_->setCallback( NULL , NULL);
		SoundManager::instance().release( pEvent_ );
	}
}

void PySound::fini()
{
	if ( pEvent_ )
	{
		pEvent_->setUserData( NULL );
        pEvent_->setCallback( NULL , NULL);
		pEvent_ = NULL;
	}
}

/**
 *  Returns a PySoundParameter* for the provided parameter name.
 */
PyObject* PySound::param( const std::string& name )
{
	BW_GUARD;
	if (!this->refresh( PySoundParameter::REFRESH_MASK ))
		return SoundManager::pyError();

	FMOD::EventParameter *p;
	FMOD_RESULT result = pEvent_->getParameter( name.c_str(), &p );

	if (result == FMOD_OK)
	{
		return new PySoundParameter( p, this );
	}
	else
	{
		PyErr_Format( PyExc_ValueError,
			"FMOD::Event::getParameter() failed: %s",
			FMOD_ErrorString( result ) );

		return SoundManager::pyError();
	}
}

PyObject* PySound::getParentProject()
{
    FMOD::EventProject *pProject;
    FMOD_RESULT result = pGroup_->getParentProject( &pProject );
    if (!SoundManager::FMOD_ErrCheck(result, "PySound::getParentProject: Couldn't get project"))
        return SoundManager::pyError();

    char * name;
    result = pProject->getInfo( 0, &name );
    if (!SoundManager::FMOD_ErrCheck(result, "PySound::getParentProject: Couldn't get project name"))
        return SoundManager::pyError();

    std::string path(name);
    path.insert( 0, "/" );
    return (PyObject *)SoundManager::instance().pyEventProject( path );
}

PyObject* PySound::getParentGroup()
{
    void * data;
    FMOD_RESULT result = pGroup_->getUserData(&data);
    if (!SoundManager::FMOD_ErrCheck(result, "PySound::getParentGroup: Couldn't get group user data"))
        return SoundManager::pyError();

    if (data)
    {
        PyEventGroup * pyEventGroup = static_cast< PyEventGroup* >( data );
        Py_INCREF( pyEventGroup );
        return static_cast< PyObject* >( pyEventGroup );
    }
    
    // Construct the path from the parent group names
    char * name;
    result = pGroup_->getInfo( 0, &name );        
    if (!SoundManager::FMOD_ErrCheck(result, "PySound::getParentGroup: Couldn't get group name"))
        return SoundManager::pyError();

    std::string path( name );
    SoundManager::EventGroup * pGroup = pGroup_;
    while (1)
    {        
        FMOD::EventGroup *pParent = NULL;
        result = pGroup->getParentGroup( &pParent );
        if (!SoundManager::FMOD_ErrCheck(result, "PySound::getParentGroup: Couldn't get parent group"))
            return SoundManager::pyError();
        if (!pParent) // ( no parent )
            break;
        pGroup = pParent;
        result = pGroup->getInfo( 0, &name );        
        if (!SoundManager::FMOD_ErrCheck(result, "PySound::getParentGroup: Couldn't get group name"))
            return SoundManager::pyError();
        path.insert( 0, "/" );
        path.insert( 0, name );
    }
    
    FMOD::EventProject *pProject;
    result = pGroup->getParentProject( &pProject );
    if (!SoundManager::FMOD_ErrCheck(result, "PySound::getParentGroup: Couldn't get parent project"))
        return SoundManager::pyError();

    result = pProject->getInfo( 0, &name );        
    if (!SoundManager::FMOD_ErrCheck(result, "PySound::getParentGroup: Couldn't get project name"))
        return SoundManager::pyError();

    path.insert( 0, "/" );
    path.insert( 0, name );
    path.insert( 0, "/" );

    return SoundManager::instance().pyEventGroup( path );
}

PyObject* PySound::getParentCategory()
{
    FMOD::EventCategory *pCategory;
    FMOD_RESULT result = pEvent_->getCategory( &pCategory );
    if (!SoundManager::FMOD_ErrCheck(result, "PySound::getParentCategory"))
        return SoundManager::pyError();

    char * name;
    result = pCategory->getInfo( 0, &name );
    if (!SoundManager::FMOD_ErrCheck(result, "PySound::getParentCategory"))
        return SoundManager::pyError();

    return SoundManager::instance().pyEventCategory( name );
}

#if ENABLE_JFIQ_HANDLING
bool PySound::isJFIQ() const
{
    return justFailIfQuietest_;
}
#endif

bool PySound::isVirtual()
{
    return virtual_;
}

/**
 *  Play this sound.
 */
bool PySound::play()
{
	BW_GUARD;
	FMOD_RESULT result;
#if ENABLE_JFIQ_HANDLING
    if (justFailIfQuietest_)
    {
        stopped_ = false;
        return true;
    }
    else
#endif
    {
	    if (!this->refresh())
		    return false;

	    result = pEvent_->start();

        if (SoundManager::FMOD_ErrCheck( result , "PySound::play: Couldn't start event" ))
	    {
		    played_ = true;
            return true;
        }

	    return false;
    }
}


/**
 *  Stop this sound.
 */
bool PySound::stop()
{
	BW_GUARD;
	if (!pEvent_)
	{
		ERROR_MSG( "PySound::stop: pEvent_ is NULL.\n" );
		return false;
	}

#if ENABLE_JFIQ_HANDLING
    stopped_ = true;
    if (virtual_)
    {
        return true;
    }
#endif

    FMOD_RESULT result = pEvent_->stop();
    return SoundManager::FMOD_ErrCheck( result , "PySound::stop: Couldn't stop event" );    
}


void PySound::setModel( PyModel *const pModel )
{
    pModel_ = pModel; 
}

/**
 *  Ensure that the Event* handle inside this PySound is ready to be played.
 *  The first time this is called, this is pretty much a no-op, so it is cheap
 *  for one-shot sounds.
 */
bool PySound::refresh( SoundManager::EventState okMask )
{
	BW_GUARD;
	FMOD_RESULT result;
	FMOD_EVENT_STATE state = 0;

	if (!pEvent_)
	{
		ERROR_MSG( "PySound::refresh: pEvent_ is NULL.\n" );
		return false;
	}

#if ENABLE_JFIQ_HANDLING
    // If it is virtual it doesn't have a valid event handle  
    // so return false.
    if (virtual_)
        return false;
#endif

	result = pEvent_->getState( &state );

	// Cheap breakout if the event is already in an acceptable state.
	if ( !reset_ && (result == FMOD_OK) && (state & okMask) )
	{
		void * userData = NULL;
		// also, check that the userdata has not been reset by anyone
		result = pEvent_->getUserData( &userData );

		if ( ( result == FMOD_OK ) && ( userData == this ) )
		{
			// If the event has already been played we need to re-attach it, as the
			// sound's "on stop" callback will have removed it from the list of
			// attached sounds and without this its 3D attributes will not be set.
			return (played_ && pModel_) ? pModel_->attachSound( this ) : true;
		}
	}

#if ENABLE_JFIQ_HANDLING
    // If it is JFIQ and got this far just return. It will attempt to get
    // get a valid handle next frame via SoundManager->update().
    if (justFailIfQuietest_) 
    {
        virtual_ = true;
        return false;
    }
#endif

	// Release old sound event.
	if (pEvent_)
	{
		SoundManager::instance().release( pEvent_ );
		pEvent_->setUserData( NULL );
		pEvent_->setCallback( NULL, NULL );
		pEvent_ = NULL;
	}

	// Get a new Event handle
	if (SoundManager::instance().allowUnload())
	{
		SoundManager::Event *pEvent = SoundManager::instance().get( path_ );

        if (pEvent)
		{
			pEvent_ = pEvent;
		}
		else
		{
			ERROR_MSG( "PySound::refresh: "
				"Couldn't re-acquire Event handle\n" );
			return false;
		}
	}
	else
	{
		pEvent_ = SoundManager::instance().get( pGroup_, index_ );
		if ( !pEvent_ )
		{
			ERROR_MSG( "PySound::refresh: "
				"Couldn't re-acquire Event handle: %s\n",
				FMOD_ErrorString( result ) );
			return false;
		}
	}

	result = pEvent_->setUserData( this );
    if ( !SoundManager::FMOD_ErrCheck( result , "PySound::refresh: Unable to set event userdata" ))
    {
        // PySound will be removed from PySoundList and deleted
        return false;
    }

    result = pEvent_->setCallback(eventCallback, NULL);
    if ( !SoundManager::FMOD_ErrCheck( result , "PySound::refresh: Unable to set event callback" ))
    {
        // PySound will be removed from PySoundList and deleted
        return false;
    }
    else
    {
		reset_ = false;
        virtual_ = false;
    }
	
	// Attach the new event to the model if there is one
	if (pModel_ && !pModel_->attachSound( this ))
		return false;

	// Set the 3D position for the new event if necessary
	if (pPosition_ && !( update( *pPosition_ )))
		return false;

	return true;
}

bool PySound::update(const Vector3& position, const Vector3& orientation, const float deltaTime)
{    
	BW_GUARD;
#if ENABLE_JFIQ_HANDLING
    if ( virtual_ ) //skip for the JFIQ events
    {
        lastPosition_ = position; //update position
        return true;
    }
#endif

    FMOD_VECTOR *pos = 0, *vel = 0, *dir = 0;
    
    pos = (FMOD_VECTOR*)&position;

    if (deltaTime != 0.f)
    {
        Vector3 velocity = calculateVelocity(position, deltaTime);
        vel = (FMOD_VECTOR*)&velocity;
    }
    
    // After Velocity calculation, set the lastPosition.
    lastPosition_ = position;

    if( orientation != Vector3::zero() )
    {
#if 1
        // Normalise the Orientation Vector.
        Vector3 normalisedOrientation = orientation;
        normalisedOrientation.normalise(); // NB: expensive!pensive!
        dir = (FMOD_VECTOR*)&normalisedOrientation; 
#else        
        // Verify that it's already normalised.
        float lengthsqrd = orientation.x * orientation.x +
                           orientation.y * orientation.y +
                           orientation.z * orientation.z;

        if(fabs(1.f-lengthsqrd) < 0.001f)
        {            
            dir = (FMOD_VECTOR*)&orientation; 
        }
        else
        {
		    ERROR_MSG( "PySound::update: Orientation vector not normalised!");
        }
#endif
    }

    FMOD_RESULT result = pEvent_->set3DAttributes( pos, vel, dir );

    // Check if the handle has been invalidated by a group unload.
    if ( result == FMOD_ERR_INVALID_HANDLE )
    {
#if !ENABLE_JFIQ_HANDLING
        if (!refresh())
            return false;
#else
        if (!refresh())
            return ( justFailIfQuietest_ ? true : false ); // expect JFIQ to return false from refresh()
        if (!stopped_) 
#endif
        {
            result = pEvent_->start(); 
            SoundManager::FMOD_ErrCheck(result, "PySound::update: Unable to start event " );
        }
        result = pEvent_->set3DAttributes( pos, vel, dir );
    }
   
    return SoundManager::FMOD_ErrCheck(result, "PySound::update: unable to set event 3D attributes" );
}

Vector3 PySound::calculateVelocity(const Vector3& position, const float deltaTime )
{    
	BW_GUARD;
    Vector3 velocity(Vector3::zero());

    if( deltaTime != 0 && lastPosition_ != Vector3(0,0,0) )
    {
        if( deltaTime > 0 )
        {
            velocity = ( position - lastPosition_ ) / deltaTime;
        }
    }

	float maxSpeed = SoundManager::instance().maxSoundSpeed();

    if (maxSpeed > 0 && 
		velocity.lengthSquared() > (maxSpeed*maxSpeed))
    {   
        // Assume the sound source has teleported.
        velocity = Vector3::zero();
    }
        
    return velocity;
}


/**
 *  This get a new event handle if the event handle has been
 *  invalidated. If a group is specified it will only be 
 *  reset if it's in that group.
 */
void PySound::reset(SoundManager::EventGroup * eventGroup)
{
    if (!eventGroup)
	    reset_ = true;
    else if(!virtual_)
    {
        // Check if this event's group or any parent of that group
        // match the event group passed in.
        SoundManager::EventGroup * group  = pGroup_;
        while (group)
        {
            if(group == eventGroup)
            {
                // This reset occur the next time update() is called.
                reset_ = true;
                break;
            }
            group->getParentGroup(&group);
        }         
    }
}

/**
 *  Getter for unloaded state.
 */
bool PySound::unloaded()
{
    return unloaded_;
}

/**
 *  Setter for unloaded state.
 */
void PySound::unloaded( bool unload )
{
    unloaded_ = unload;
}

/**
 *  Getter for muted state.
 */
bool PySound::muted()
{
	BW_GUARD;
    if (virtual_)
        return false;
	bool muted;
	FMOD_RESULT result = pEvent_->getMute( &muted );
    
    SoundManager::FMOD_ErrCheck( result, "PySound::muted" );
    return muted;
}


/**
 *  Setter for muted state.
 */
void PySound::muted( bool newMute )
{
	BW_GUARD;
    if (virtual_)
    {        
		PyErr_Format( PyExc_AttributeError,
			"PySound %s is virtual, cannot mute", name() );
        return;
    }
	FMOD_RESULT result = pEvent_->setMute( newMute );
    SoundManager::FMOD_ErrCheck( result, "PySound::muted" );
}


/**
 *  Getter for paused state.
 */
bool PySound::paused()
{
	BW_GUARD;
    if (virtual_)
        return false;
	bool paused;
	FMOD_RESULT result = pEvent_->getPaused( &paused );
    
    SoundManager::FMOD_ErrCheck( result, "PySound::paused" );
    return paused;
}


/**
 *  Setter for paused state.
 */
void PySound::paused( bool newPaused )
{
	BW_GUARD;
    if (virtual_)
    {        
		PyErr_Format( PyExc_AttributeError,
			"PySound %s is virtual, cannot pause", name() );
        return;
    }
	FMOD_RESULT result = pEvent_->setPaused( newPaused );
    SoundManager::FMOD_ErrCheck( result, "PySound::paused" );
}


/**
 *  Getter for event pitch.
 */
float PySound::pitch()
{
	BW_GUARD;
	float pitch;
    if (virtual_)
        return 0.f;
	FMOD_RESULT result = pEvent_->getPitch( &pitch );
    
    if (!SoundManager::FMOD_ErrCheck( result, "PySound::pitch" ))
    {
        pitch = -1;
    }
    return  pitch;
}


/**
 *  Setter for event pitch. Note: This is time variant and low CPU
 */
void PySound::pitch( float newPitch )
{
	BW_GUARD;
    if (virtual_)
    {        
		PyErr_Format( PyExc_AttributeError,
			"PySound %s is virtual, Cannot set pitch", name() );
        return;
    }
	FMOD_RESULT result = pEvent_->setPitch( newPitch );
    SoundManager::FMOD_ErrCheck( result, "PySound::pitch" );
}


/**
 *  Getter for event volume.
 */
float PySound::volume()
{
	BW_GUARD;
	if (!pEvent_ || virtual_)
	{
		ERROR_MSG( "PySound::volume: pEvent_ is NULL.\n" );
		return -1.f;
	}

	float vol;
	FMOD_RESULT result = pEvent_->getVolume( &vol );
    
    if (!SoundManager::FMOD_ErrCheck( result, "PySound::volume" ))
    {
        vol = -1;
    }
    return vol;
}


/**
 *  Setter for event volume.
 */
void PySound::volume( float vol )
{
	BW_GUARD;
	if (!pEvent_ || virtual_)
	{
		PyErr_Format( PyExc_AttributeError,
			"PySound %s is virtual, Cannot set volume", name() );
		return;
	}

	FMOD_RESULT result = pEvent_->setVolume( vol );
    SoundManager::FMOD_ErrCheck( result, "PySound::volume" );
}


/**
 *  Returns the length of this sound event in seconds.  Will return < 0 on error,
 *  which can be caused by asking for the duration of a looping sound.
 */
float PySound::duration()
{
	BW_GUARD;
	if (!pEvent_ || virtual_)
	{
		ERROR_MSG( "PySound::duration: pEvent_ is NULL.\n" );
		return -1.f;
	}

	FMOD_EVENT_INFO eventInfo;
	memset( &eventInfo, 0, sizeof( eventInfo ) );
	FMOD_RESULT result = pEvent_->getInfo( NULL, NULL, &eventInfo );

    return SoundManager::FMOD_ErrCheck( result, "PySound::duration" ) ? eventInfo.lengthms / 1000.0f : -1;
}


/**
 *  Returns the name of the underlying FMOD::Event.  Uses memory managed by FMOD
 *  so don't hang onto this for long.
 */
const char *PySound::name() const
{
	BW_GUARD;
	static const char *err = "<error>";
    static const char *nameless = "<name unknown>";
	if (!pEvent_ || virtual_)
	{
		ERROR_MSG( "PySound::name: pEvent_ is NULL.\n" );
		return err;
	}

	char *name;

    if (virtual_)
    {
        if (path_.empty())
            return nameless;
        else
        {
            size_t found;
            found = path_.find_last_of("/\\");
            if (found != path_.npos)
                return path_.substr(found + 1).c_str();
            else
                return path_.c_str();
        }
    }

	FMOD_RESULT result = pEvent_->getInfo( NULL, &name, NULL );
	if (result == FMOD_OK)
		return name;
	else
	{
        SoundManager::FMOD_ErrCheck(result, "PySound::name: %s\n");
		return err;
	}
}


/**
 *  Returns a string containing the names of all the FMOD_EVENT_STATE_* flags 
 *	that are set for this Event.
 */
std::string PySound::state()
{
	BW_GUARD;
	if (!pEvent_)
	{
		ERROR_MSG( "PySound::volume: pEvent_ is NULL.\n" );
		return "NULL";
	}

	FMOD_EVENT_STATE state;
	FMOD_RESULT result = pEvent_->getState( &state );
	std::string ret;

	if (state & FMOD_EVENT_STATE_READY) ret += "ready ";
	if (state & FMOD_EVENT_STATE_LOADING) ret += "loading ";
	if (state & FMOD_EVENT_STATE_ERROR) ret += "error ";
	if (state & FMOD_EVENT_STATE_PLAYING) ret += "playing ";
	if (state & FMOD_EVENT_STATE_CHANNELSACTIVE) ret += "channelsactive ";
	if (state & FMOD_EVENT_STATE_INFOONLY) ret += "infoonly ";

#if ENABLE_JFIQ_HANDLING
    if (stopped_ || result == FMOD_ERR_INVALID_HANDLE)
        ret += "stopped ";
    else if (virtual_) ret += "virtual ";
#endif

	if (!ret.empty())
		ret.erase( ret.size() - 1 );

	return ret;
}


/**
 *  This method returns the 3D position this sound is playing at.
 */
PyObject * PySound::pyGet_position()
{
	BW_GUARD;
	// Just return the position of our model if we're attached.
	if (pModel_)
	{
		return pModel_->pyGet_position();
	}

	if (pPosition_)
	{
		return Script::getData( *pPosition_ );
	}
	else
	{
		PyErr_Format( PyExc_AttributeError,
			"PySound '%s' has no 3D position set",
			name() );

		return NULL;
	}
}


/**
 *  This method sets the 3D position this sound is playing at.
 */
int PySound::pySet_position( PyObject * position )
{
	BW_GUARD;
	// Not allowed to set 3D attributes for attached sounds
	if (pModel_)
	{
		PyErr_Format( PyExc_AttributeError,
			"Can't set 3D position for PySound '%s' "
			"(it is already attached to %s)",
			name() , pModel_->name().c_str() );

		return -1;
	}

	// Create position vector if necessary
	Vector3 * pPosition = pPosition_ ? pPosition_ : new Vector3();

	if (Script::setData( position, *pPosition, "position" ) == -1)
	{
		// Clean up position vector if necessary
		if (pPosition != pPosition_)
		{
			delete pPosition;
		}

		return -1;
	}

	if (update( *pPosition ))
	{
		pPosition_ = pPosition;
		return 0;
	}
	else
	{
		PyErr_Format( PyExc_RuntimeError,
			"Failed to set 3D position for %s",
			name() );

		delete pPosition;
		pPosition_ = NULL;

		return -1;
	}
}

/*
        CALLBACK STUFF
*/
static void FMOD_EVENT_CALLBACKTYPE_initMaps();
PY_BEGIN_ENUM_MAP( FMOD_EVENT_CALLBACKTYPE, FMOD_EVENT_CALLBACKTYPE_ )
    PY_ENUM_VALUE( FMOD_EVENT_CALLBACKTYPE_SYNCPOINT )
    PY_ENUM_VALUE( FMOD_EVENT_CALLBACKTYPE_SOUNDDEF_START )		
    PY_ENUM_VALUE( FMOD_EVENT_CALLBACKTYPE_SOUNDDEF_END )		
    PY_ENUM_VALUE( FMOD_EVENT_CALLBACKTYPE_STOLEN )
    PY_ENUM_VALUE( FMOD_EVENT_CALLBACKTYPE_EVENTFINISHED ) 
    PY_ENUM_VALUE( FMOD_EVENT_CALLBACKTYPE_NET_MODIFIED )
    PY_ENUM_VALUE( FMOD_EVENT_CALLBACKTYPE_SOUNDDEF_CREATE )
    PY_ENUM_VALUE( FMOD_EVENT_CALLBACKTYPE_SOUNDDEF_RELEASE )
    PY_ENUM_VALUE( FMOD_EVENT_CALLBACKTYPE_SOUNDDEF_INFO ) 
    PY_ENUM_VALUE( FMOD_EVENT_CALLBACKTYPE_EVENTSTARTED )
    PY_ENUM_VALUE( FMOD_EVENT_CALLBACKTYPE_SOUNDDEF_SELECTINDEX )
    PY_ENUM_VALUE( FMOD_EVENT_CALLBACKTYPE_OCCLUSION )
PY_END_ENUM_MAP()

namespace EnumEventCallback
{
    PY_ENUM_CONVERTERS_DECLARE( FMOD_EVENT_CALLBACKTYPE )
    PY_ENUM_CONVERTERS_SCATTERED( FMOD_EVENT_CALLBACKTYPE )
}

/*
    Callback function for use with FMOD
*/
/* static */ FMOD_RESULT F_CALLBACK PySound::eventCallback(
  FMOD_EVENT *  event, 
  FMOD_EVENT_CALLBACKTYPE  type, 
  void *  param1, 
  void *  param2, 
  void *  userdata
)
{
	BW_GUARD;
    void * data;
    PySound *instance = NULL;
    ((FMOD::Event *)event)->getUserData(&data);

    MF_ASSERT(data);
    instance = static_cast< PySound* >( data );
#if ENABLE_JFIQ_HANDLING
    if( type == FMOD_EVENT_CALLBACKTYPE_STOLEN /*&& instance->justFailIfQuietest_*/ )
    {
        instance->virtual_ = true;
        SoundManager::instance().release((FMOD::Event *)event);
    }
#endif
    instance->doCallback(type);
    return FMOD_OK;
}

void PySound::doCallback( FMOD_EVENT_CALLBACKTYPE type )
{
    BW_GUARD;
    if (callbackMap_.count(type))
    {
		Py_INCREF( callbackMap_[type].get() );
        Script::call( callbackMap_[type].get(), 
            Py_BuildValue( "(O)", (PyObject*)this ),
			"PySound::doCallback: ", true );
    }
}

void PySound::setCallback( PyObjectPtr type, PyObjectPtr callback)
{
    BW_GUARD;
    FMOD_EVENT_CALLBACKTYPE enumType;
    if (EnumEventCallback::Script::setData( type.get(),
			enumType, "PySound::setCallback" ) )
    {
        ERROR_MSG("PySound::setCallback cannot convert "
            "argument %s to event callback type", enumType);
        return;
    }
    callbackMap_[enumType] = callback;
}


/*
    Just Fail If Quietest
*/
#if ENABLE_JFIQ_HANDLING
bool PySound::updateJFIQ()
{
	BW_GUARD;
    MF_ASSERT( justFailIfQuietest_ );

    /*
        If PySound isn't virtual then don't bother getting a handle.
    */
    if( stopped_ || !virtual_ )
        return true;

    FMOD_RESULT result;

    result = pGroup_->getEventByIndex( index_, FMOD_EVENT_INFOONLY, &pEvent_ );
    if( !SoundManager::FMOD_ErrCheck( result, "PySound::updateJFIQ" ))
        return false;

    result = pEvent_->set3DAttributes((FMOD_VECTOR*)&lastPosition_, NULL, NULL); // TODO: include orientation
    if( !SoundManager::FMOD_ErrCheck( result, "PySound::updateJFIQ" ))
        return false;
     
    if( !pGroup_ )
        return false;

	if (pEvent_)
	{
		SoundManager::instance().release( pEvent_ );
		pEvent_->setUserData( NULL );
		pEvent_->setCallback( NULL, NULL );
		pEvent_ = NULL;
	}

    result = pGroup_->getEventByIndex( index_, FMOD_EVENT_DEFAULT, &pEvent_ );

    if( result == FMOD_ERR_EVENT_FAILED) // Event 'Just failed' because it is the quietest.
    {
        virtual_ = true;
        return true;
    }

    if( !SoundManager::FMOD_ErrCheck( result, "PySound::updateJFIQ" ))            
        return false;
     
    virtual_ = false;

	result = pEvent_->setUserData( this );
    if( !SoundManager::FMOD_ErrCheck( result, "PySound::updateJFIQ" ))
        return false;

    result = pEvent_->setCallback( eventCallback, NULL );
    if( !SoundManager::FMOD_ErrCheck( result, "PySound::updateJFIQ" ))
        return false;

    result = pEvent_->start();
    if( !SoundManager::FMOD_ErrCheck( result, "PySound::updateJFIQ" ))
        return false;

    return true;
}
#endif



PyObject* PySound::pyGetAttribute( const char* attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}

int PySound::pySetAttribute( const char* attr, PyObject* value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}

/*~ function FMOD.playSound
 *
 *  Plays a sound.
 *
 *  The semantics for sound naming are similar to filesystem paths, in that
 *  there is support for both 'relative' and 'absolute' paths.  If a path is
 *  passed without a leading '/', the sound is considered to be part of the
 *  default project (see FMOD.setDefaultProject), for example 'guns/fire'
 *  would refer to the 'guns/fire' event from the default project.  If your game
 *  uses only a single FMOD project, you should refer to events using relative
 *  paths, as it is (slightly) more efficient than the absolute method.
 *
 *  If your game uses multiple projects and you want to refer to a sample in a
 *  project other than the default, you can say '/project2/guns/fire' instead.
 *
 *  @param	path	The path to the sound event you want to trigger.
 *
 *  @return			The PySound for the sound event.
 */
static PyObject* py_playSound( PyObject * args )
{
	BW_GUARD;

	char* tag;

	if (!PyArg_ParseTuple( args, "s", &tag ))
		return NULL;
    
    PyObject *pyObject = SoundManager::instance().createPySound( tag );
    if (pyObject == SoundManager::pyError())
		return SoundManager::pyError();

    PySound *pySound = static_cast<PySound *>( pyObject );

    pySound->play();
	return pySound;
}
PY_MODULE_FUNCTION( playSound, _FMOD );

// Alias provided for backwards compatibility
//PY_MODULE_FUNCTION_ALIAS( playSound, playSimple, BigWorld );

// This is an alias to hook in BigWorld.playUISound() from
// bigworld/src/client/bwsound.cpp
//PY_MODULE_FUNCTION_ALIAS( playSound, playUISound, BigWorld );


/*~ function FMOD.getSound
 *
 *  Returns a sound handle.
 *
 *  @param	path	The path to the sound event you want to trigger (see
 *  				FMOD.playSound() for syntax)
 *
 *  @return			The PySound for the sound event.
 */
static PyObject* py_getSound( PyObject * args )
{
	BW_GUARD;
	char* tag;

	if (!PyArg_ParseTuple( args, "s", &tag ))
		return NULL;

    PySound *pySound = static_cast<PySound *>( SoundManager::instance().createPySound( tag ) );

    if (pySound == NULL)
    {
        return SoundManager::pyError();
    }
    
    return pySound;
}

PY_MODULE_FUNCTION( getSound, _FMOD )

// Alias provided for backwards compatibility
//PY_MODULE_FUNCTION_ALIAS( getSound, getSimple, BigWorld );


/*~ function FMOD.loadEventProject
 *
 *  Load the specified event project file.  Note that the argument to this 
 *	function is the name of the project, *not* its filename.  So basically you 
 *	are passing in the filename minus the .fev extension.
 *
 *  @param	filename	The name of the soundbank.
 */
static PyObject* py_loadEventProject( PyObject *args )
{
	BW_GUARD;
	char *filename;

	if (!PyArg_ParseTuple( args, "s", &filename ))
		return NULL;

	if (SoundManager::instance().loadEventProject( filename ))
		Py_RETURN_NONE;
	else
		return SoundManager::pyError();
}

PY_MODULE_FUNCTION( loadEventProject, _FMOD );
//PY_MODULE_FUNCTION_ALIAS( loadEventProject, loadSoundBank, BigWorld );


/*~ function FMOD.unloadEventProject
 *
 *  Unload the named event project.
 *
 *  @param	name	The name of the project
 */
static PyObject* py_unloadEventProject( PyObject *args )
{
	BW_GUARD;
	char *name;

	if (!PyArg_ParseTuple( args, "s", &name ))
		return NULL;

	if (SoundManager::instance().unloadEventProject( name ))
		Py_RETURN_NONE;
	else
		return SoundManager::pyError();
}

PY_MODULE_FUNCTION( unloadEventProject, _FMOD );
//PY_MODULE_FUNCTION_ALIAS( unloadEventProject, unloadSoundBank, BigWorld );


/*~ function FMOD.reloadEventProject
 *
 *  Reload the named event project.  Use this if you have created or deleted 
 *	events in a project using the FMOD Designer tool and you want to get the
 *  changes in your app.  If you are just tweaking properties on existing sound
 *  events you can audition them using the network audition mode of the FMOD
 *  Designer tool without needing to call this function.
 *
 *  @param	name	The name of the project
 */
static PyObject* py_reloadEventProject( PyObject *args )
{
	BW_GUARD;
	char *name;

	if (!PyArg_ParseTuple( args, "s", &name ))
		return NULL;

	if (!SoundManager::instance().unloadEventProject( name ))
		return SoundManager::pyError();

	if (!SoundManager::instance().loadEventProject( name ))
		return SoundManager::pyError();

	Py_RETURN_NONE;
}

PY_MODULE_FUNCTION( reloadEventProject, _FMOD );
//PY_MODULE_FUNCTION_ALIAS( reloadEventProject, reloadSoundBank, BigWorld );


/*~ function FMOD.loadSoundGroup
 *
 *  Loads the wave data for the specified event group (and all descendant
 *  groups) into memory, so that the sounds will play instantly when triggered.
 *
 *  If called with just a project name as the argument, all sound groups in that
 *  project will be loaded.
 *
 *  @param	path	The path to the event group
 */
static PyObject *py_loadSoundGroup( PyObject * args )
{
	BW_GUARD;
	char *group;

	if (!PyArg_ParseTuple( args, "s", &group ))
		return NULL;

	if (SoundManager::instance().loadWaveData( group ))
		Py_RETURN_NONE;
	else
		return SoundManager::pyError();
}

PY_MODULE_FUNCTION( loadSoundGroup, _FMOD );


/*~ function FMOD.unloadSoundGroup
 *
 *  Unloads the wave data for the specified event group (and all descendant
 *  groups) from memory.  Note that sounds can still be played from this group
 *  later on, however the first playback for each event will block whilst the
 *  wave data is loaded from disk.
 *
 *  @param	path	The path to the event group
 */
static PyObject *py_unloadSoundGroup( PyObject * args )
{
	BW_GUARD;
	char *group;

	if (!PyArg_ParseTuple( args, "s", &group ))
		return NULL;

	if (SoundManager::instance().unloadWaveData( group ))
		Py_RETURN_NONE;
	else
		return SoundManager::pyError();
}

PY_MODULE_FUNCTION( unloadSoundGroup, _FMOD );


/*~ function FMOD.setDefaultSoundProject
 *
 *  Sets the default sound project that relatively-named events will be read
 *  from.
 *
 *  See FMOD.playSound() for more information about sound event naming
 *  conventions.
 *
 *  See the 'soundMgr' section of engine_config.xml for how to load sound
 *  projects into your game.
 *
 *  @param	name	The name of the project to be set as the default.
 */
static PyObject *py_setDefaultSoundProject( PyObject * args )
{
	BW_GUARD;
	char *project;

	if (!PyArg_ParseTuple( args, "s", &project ))
		return NULL;

	if (SoundManager::instance().setDefaultProject( project ))
	{
		Py_RETURN_NONE;
	}
	else
	{
		PyErr_Format( PyExc_RuntimeError,
			"setDefaultProject( '%s' ) failed, see debug output for details",
			project );

		return SoundManager::pyError();
	}
}

PY_MODULE_FUNCTION( setDefaultSoundProject, _FMOD );


/*~ function FMOD.setMasterVolume
 *
 *  Sets the master volume applied to all sounds.
 *
 *  @param	vol		A float in the range [0.0, 1.0].
 */
static PyObject *py_setMasterVolume( PyObject *args )
{
	BW_GUARD;
	float vol;

	if (!PyArg_ParseTuple( args, "f", &vol ))
		return NULL;

	if (SoundManager::instance().setMasterVolume( vol ))
		Py_RETURN_NONE;
	else
		return SoundManager::pyError();
}

PY_MODULE_FUNCTION( setMasterVolume, _FMOD );


/**
 *  This method is only provided for backwards-compatibility with BW1.7 and
 *  earlier sound code.
 */
static PyObject* py_getFxSoundDuration( PyObject *args )
{
	BW_GUARD;
	PyObject *pHandle = py_getSound( args );
	if (pHandle == NULL || pHandle == SoundManager::pyError())
		return pHandle;

	// Now that we have a valid sound handle, just return its duration
	PyObject *ret = PyFloat_FromDouble( ((PySound*)pHandle)->duration() );
	Py_DECREF( pHandle );
	return ret;
}

PY_MODULE_FUNCTION( getFxSoundDuration, _FMOD );
//PY_MODULE_FUNCTION_ALIAS( getFxSoundDuration, getSoundDuration, BigWorld );

#endif // FMOD_SUPPORT
