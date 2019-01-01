/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "cstdmf/guard.hpp"
#include "py_sound_list.hpp"

#if FMOD_SUPPORT

// -----------------------------------------------------------------------------
// Section: PySoundList
// -----------------------------------------------------------------------------

PySoundList::PySoundList() :
	std::list< PySound * >(),
	stopOnDestroy_( true )
{
	BW_GUARD;	
}

/**
 *  If required, the destructor cleans up any sounds still remaining in the
 *  list.
 */
PySoundList::~PySoundList()
{
	BW_GUARD;
	if (stopOnDestroy_)
	{
		this->stopAll();
	}
	else
	{
		for (iterator it = this->begin(); it != this->end(); ++it)
		{
            (*it)->decRef();
		}
	}
}

bool PySoundList::removeSound( PySound *pPySound )
{    
    BW_GUARD;
	PySoundList::iterator itr;
    
	for(itr = this->begin(); itr != this->end(); itr++)
    {
	    // It's fine for an event to not be in the list ... that just means it
	    // was erased during an update(), probably because its Event* had been
	    // stolen by a newer Event.
        if (pPySound == *itr)
        {
            (*itr)->decRef();
		    this->erase( itr );
            return true;
        }
    }

    return false;
}


bool PySoundList::removeSound( SoundManager::Event *pEvent )
{    
    BW_GUARD;
	PySoundList::iterator itr;
    
    void *data;
    FMOD_RESULT result = pEvent->getUserData(&data);
    SoundManager::FMOD_ErrCheck(result, "PySoundList::removeSound: Couldn't retrieve user data");
    PySound *pySound = static_cast< PySound * >( data );

	for(itr = this->begin(); itr != this->end(); itr++)
    {
	    // It's fine for an event to not be in the list ... that just means it
	    // was erased during an update(), probably because its Event* had been
	    // stolen by a newer Event.
        if (pySound == *itr)
        {
		    this->erase( itr );
            result = pEvent->stop();
            SoundManager::FMOD_ErrCheck(result, "PySoundList::removeSound: Couldn't stop event");
            pySound->decRef();
            return true;
        }
    }

    return false;
}


/**
 *  Appends an Event to this list.
 */
void PySoundList::push_back( PySound *pySound )
{
	BW_GUARD;
    pySound->incRef();
	std::list< PySound * >::push_back( pySound );
}


/**
 *  Update positions for any sounds that are still playing.
 */
bool PySoundList::update( const Vector3 &position, const Vector3 &orientation, float deltaTime )
{
	BW_GUARD;
	bool ok = true;

	iterator it = this->begin();
	while (it != this->end())
	{
		PySound *pySound = *it;
        if ( !pySound->unloaded() && pySound->update( position, orientation, deltaTime ))
		{
			++it;
		}
		else
		{
			// If we get to here, the event must have had it's channel stolen.
			it = this->erase( it );
            pySound->decRef();
		}
	}

	return ok;
}

/**
 *  Stop and clear all sound events.
 */
bool PySoundList::stopAll()
{
	BW_GUARD;
	bool ok = true;

	for (iterator it = this->begin(); it != this->end(); ++it)
	{
		ok &= (*it)->stop();
        (*it)->decRef();
	}

	if (!ok)
	{
		ERROR_MSG( "PySoundList::stopAll: "
			"Some events failed to stop\n" );
	}

	this->clear();

	return ok;
}

#endif // FMOD_SUPPORT

// pysoundlist.cpp