/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_EVENT_GROUP_HPP
#define PY_EVENT_GROUP_HPP

#include "fmod_config.hpp"

#if FMOD_SUPPORT

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "sound_manager.hpp"
#include "py_sound.hpp"

#include <fmod_event.hpp>

/**
 *  A PyEventGroup is a wrapper for an FMOD::EventGroup, and provides an
 *  interface to control the loading and unloading of data by group. Groups
 *  are created using the event hierachy in the FMOD Designer tool.
 */
class PyEventGroup : public PyObjectPlus
{
	Py_Header( PyEventGroup, PyObjectPlus )

public:
    PyEventGroup( SoundManager::EventGroup * pEventGroup, PyTypePlus * pType = &PyEventGroup::s_type_ );   

	void fini();

	// PyObjectPlus overrides
	PyObject*			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );
    
    //  Methods
    void loadEventData(bool blocking = true);
	PY_AUTO_METHOD_DECLARE( RETVOID, loadEventData, OPTARG( bool, true, END ) )

    void freeEventData(bool resetEvents = false);
	PY_AUTO_METHOD_DECLARE( RETVOID, freeEventData, OPTARG( bool, false, END ) )

	//  Attributes
    unsigned int memoryUsed();
	PY_RO_ATTRIBUTE_DECLARE( memoryUsed(), memoryUsed ); 
    
    bool isPlaying();
	PY_RO_ATTRIBUTE_DECLARE( isPlaying(), isPlaying );
    
    bool isLoaded();
	PY_RO_ATTRIBUTE_DECLARE( isLoaded(), isLoaded );

	PY_FACTORY_DECLARE()

	void invalidate();

protected:
    SoundManager::EventGroup *eventGroup_;

private:
    PyEventGroup();
	~PyEventGroup();

	PyEventGroup(const PyEventGroup&);
	PyEventGroup& operator=(const PyEventGroup&);
};

typedef SmartPointer< PyEventGroup > PyEventGroupPtr;

#endif // FMOD_SUPPORT

#endif // PY_EVENT_GROUP_HPP
