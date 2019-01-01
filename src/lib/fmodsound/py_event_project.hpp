/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_EVENT_PROJECT_HPP
#define PY_EVENT_PROJECT_HPP

#include "fmod_config.hpp"

#if FMOD_SUPPORT

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "py_sound.hpp"

#include <fmod_event.hpp>



/**
 *  A PyEventProject is a wrapper for an FMOD::EventGroup.
 */
class PyEventProject : public PyObjectPlus
{
	Py_Header( PyEventProject, PyObjectPlus )

public:
    PyEventProject( SoundManager::EventProject * pEventProject, PyTypePlus * pType = &PyEventProject::s_type_ );   

	void fini();

    // PyObjectPlus overrides
	PyObject*			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );
    
    //  Methods
    void stopAllEvents(bool immediate = true);
	PY_AUTO_METHOD_DECLARE( RETVOID, stopAllEvents, OPTARG( bool, true, END ) )

    void release();
	PY_AUTO_METHOD_DECLARE( RETVOID, release, END )

	//  Attributes
    unsigned int memoryUsed();
	PY_RO_ATTRIBUTE_DECLARE( memoryUsed(), memoryUsed ); 

	PY_FACTORY_DECLARE()

protected:
    SoundManager::EventProject *eventProject_;

private:
    PyEventProject();
	~PyEventProject();

	PyEventProject(const PyEventProject&);
	PyEventProject& operator=(const PyEventProject&);
};

typedef SmartPointer< PyEventProject > PyEventProjectPtr;

#endif // FMOD_SUPPORT

#endif // PY_EVENT_PROJECT_HPP
