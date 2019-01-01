/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_EVENT_CATEGORY_HPP
#define PY_EVENT_CATEGORY_HPP


#include "fmod_config.hpp"

#if FMOD_SUPPORT

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "sound_manager.hpp"

#include <fmod_event.hpp>

/**
 *  A PyEventCategory is a wrapper for an FMOD::EventCategory, and a
 *  virtual folder used to organize event data into generic categories
 *  such as music. Allows volume and pitch changes to be applied to
 *  all events within the group.
 */
class PyEventCategory : public PyObjectPlus
{
	Py_Header( PyEventCategory, PyObjectPlus )

public:
    PyEventCategory( SoundManager::EventCategory * pEventCategory, PyTypePlus * pType = &PyEventCategory::s_type_ );   

	void fini();

    // PyObjectPlus overrides
	PyObject*			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );
    
    //  Methods
    void stopAllEvents();
	PY_AUTO_METHOD_DECLARE( RETVOID, stopAllEvents, END  )

	//  Attributes
    bool muted() const;
    void muted( bool newMute );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, muted, muted ); 

    bool paused() const;
    void paused( bool newPaused );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, paused, paused );
    
	float volume() const;
	void volume( float newValue );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, volume, volume );

    float pitch() const;
    void pitch( float newPitch);
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, pitch, pitch ); 

	PY_FACTORY_DECLARE()

protected:
    SoundManager::EventCategory *eventCategory_;

private:
    PyEventCategory();
	~PyEventCategory();

	PyEventCategory(const PyEventCategory&);
	PyEventCategory& operator=(const PyEventCategory&);

private:
	mutable bool muted_;
	mutable bool paused_;
	mutable float volume_;
	mutable float pitch_;
};

typedef SmartPointer< PyEventCategory > PyEventCategoryPtr;

#endif // FMOD_SUPPORT

#endif // PY_EVENT_CATEGORY_HPP
