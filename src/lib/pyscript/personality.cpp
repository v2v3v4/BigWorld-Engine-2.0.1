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
#include "personality.hpp"
#include "script.hpp"

namespace Personality
{

PyObject *s_pInstance_ = NULL;

const char * DEFAULT_NAME = DEFAULT_PERSONALITY_NAME;


/**
 *  Script FiniTimeJob to make sure this module is cleaned up before script
 *  shutdown.
 */
class PersonalityFiniTimeJob : public Script::FiniTimeJob
{
public:
	PersonalityFiniTimeJob( int rung = INT_MAX ) :
		Script::FiniTimeJob( rung )
	{ }

private:
	virtual void fini()
	{
		if (s_pInstance_)
		{
			Script::call( PyObject_GetAttrString( s_pInstance_, "onFini" ),
				PyTuple_New( 0 ), "onFini", true );
			Py_DECREF( s_pInstance_ );
		}
		delete this;
	}
};


/**
 *  Import and return the personality module.
 */
PyObject* import( const std::string &name )
{
	// Don't do this twice
	if (s_pInstance_)
	{
		WARNING_MSG( "Personality::init: Called twice\n" );
		return s_pInstance_;
	}

	s_pInstance_ =
		PyImport_ImportModule( const_cast< char * >( name.c_str() ) );

	if (s_pInstance_)
	{
		// Register fini time job to make sure this module is cleaned up
		new PersonalityFiniTimeJob();
	}
	else
	{
		ERROR_MSG( "Personality::import: "
			"Failed to import personality module '%s':\n",
			name.c_str() );
		PyErr_Print();
	}

	return s_pInstance_;
}


bool callOnInit( bool isReload )
{
	if (s_pInstance_)
	{
		return Script::call( PyObject_GetAttrString( s_pInstance_, "onInit" ),
				PyTuple_Pack( 1, isReload ? Py_True : Py_False ), 
				"onInit", true );
	}

	return true;
}


/**
 *  Get a borrowed reference to the personality module.  You must have
 *  successfully called Personality::import() before doing this.
 */
PyObject* instance()
{
	return s_pInstance_;
}

} // namespace Personality

// personality.cpp
