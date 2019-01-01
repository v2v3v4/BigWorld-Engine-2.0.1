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

#include "pyfxsound.hpp"

#ifndef CODE_INLINE
#include "pyfxsound.ipp"
#endif

#include "cstdmf/debug.hpp"
#include "cstdmf/dogwatch.hpp"

#include "pyscript/script.hpp"



DECLARE_DEBUG_COMPONENT2( "PyFxSound", 0 );


// -----------------------------------------------------------------------------
// PyFxSound
// -----------------------------------------------------------------------------


PY_TYPEOBJECT( PyFxSound );

PY_BEGIN_METHODS( PyFxSound )
	PY_METHOD( play )
	PY_METHOD( stop )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyFxSound )
	PY_ATTRIBUTE( position )
	PY_ATTRIBUTE( frequency )
	PY_ATTRIBUTE( isPlaying )
	PY_ATTRIBUTE( velocity )
PY_END_ATTRIBUTES()

PY_FACTORY( PyFxSound, BigWorld )


#pragma warning (disable:4355)	// this used in initialiser list



/** Constructor
 */
PyFxSound::PyFxSound( const char* tag, PyTypePlus* pType ) :
	PyObjectPlus( pType ),
	snd_( soundMgr().findFxSound(tag) ),
	instance_( -1 ),
	position_( 0, 0, 0 )
{
}

/** Destructor
 */
PyFxSound::~PyFxSound()
{
	this->stop();
}


/** Start a sound playing
 */
void PyFxSound::play()
{
	if (snd_ != NULL)
	{
		// TODO: What happens if we already have an instance_ != -1?
		instance_ = snd_->play(0, position_);
	}
}

/** Start a sound playing for python
 */
PyObject* PyFxSound::py_play(PyObject* args)
{
	if (!this->isValidForPy()) return NULL;

	this->play();

	Py_Return;
}


/** Stop a sound playing
 */
void PyFxSound::stop()
{
	if (instance_ != -1)
	{
		MF_ASSERT(snd_);
		snd_->stop(instance_);
		instance_ = -1;
	}
}

/** Stop a sound playing for python
 */
PyObject* PyFxSound::py_stop(PyObject* args)
{
	this->stop();
	Py_Return;
}


/**
 *	Position accessor
 */
const Vector3& PyFxSound::position() const
{
	return instance_ == -1 ? position_ : snd_->position( instance_ );
}

/**
 *	Position accessor
 */
void PyFxSound::position( const Vector3& newPosition )
{
	position_ = newPosition;

	if (instance_ != -1)
		snd_->position( newPosition, instance_ );
}

/**
 *	Position accessor
 */
void PyFxSound::position( const Vector3& newPosition, float dTime )
{
	position_ = newPosition;

	if (instance_ != -1)
		snd_->position( newPosition, dTime, instance_ );
}


/**
 *	This allows scripts to get various properties of a model
 */
PyObject* PyFxSound::pyGetAttribute( const char* attr )
{
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}




/**
 *	This allows scripts to set various properties of a model
 */
int PyFxSound::pySetAttribute( const char* attr, PyObject* value )
{
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}


/**
 *	Specialised get method for the 'frequency' attribute
 */
PyObject* PyFxSound::pyGet_frequency()
{
	if (instance_ == -1)
	{
		PyErr_SetString( PyExc_AttributeError, "PyFxSound.frequency: "
			"Can only get frequency on a playing sound" );
		return NULL;
	}

	return Script::getData( snd_->frequency(instance_) );
}


/**
 *	Specialised set method for the 'frequency' attribute
 */
int PyFxSound::pySet_frequency( PyObject* value )
{
	if (instance_ == -1)
	{
		PyErr_SetString( PyExc_AttributeError, "PyFXSound.frequency: "
			"Can only set frequency on a playing sound" );
		return -1;
	}

	uint freq;
	int ret = Script::setData( value, freq, "PyFXSound.frequency" );
	if (ret == 0)
		snd_->frequency( freq, instance_ );

	return ret;
}


/**
 *	Make a new PyFxSound
 */
PyObject * PyFxSound::pyNew( PyObject* args )
{
	char* tag;

	if (!PyArg_ParseTuple( args, "s", &tag ))
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.PyFxSound: "
			"Argument parsing error." );
		return NULL;
	}

	PyFxSound* snd = new PyFxSound( tag );

	if (!snd->isValidForPy())
	{
		PyErr_Format( PyExc_ValueError, "BigWorld.PyFxSound: "
			"No such sound: %s", tag );
		Py_DECREF( snd );
		return NULL;
	}

	return snd;
}


/** class checking
 */
bool PyFxSound::isValidForPy() const
{
	if (!snd_)
	{
		PyErr_SetString( PyExc_NameError, "named sound not found" );
		return false;
	}

	return true;
}


// -----------------------------------------------------------------------------
// Sound Manager python module functions
// -----------------------------------------------------------------------------


/**
 *	Plays the named sound fx
 */
static PyObject* py_playFx( PyObject * args )
{
	char* tag;
	float x, y, z;

	if (!PyArg_ParseTuple( args, "s(fff)", &tag , &x, &y, &z ))
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.playFx: Argument parsing error." );
		return NULL;
	}

	//TRACE_MSG( "py_playFx(%s)\n", tag );

	soundMgr().playFx(tag, Vector3(x, y, z));

	Py_Return;
}
PY_MODULE_FUNCTION( playFx, BigWorld )


/**
 *	Plays the named sound fx with a delay
 */
static PyObject* py_playFxDelayed( PyObject* args )
{
	char* tag;
	float x, y, z, delay;

	if (!PyArg_ParseTuple( args, "sf(fff)", &tag, &delay, &x, &y, &z ))
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.playFxDelayed: "
			"Argument parsing error." );
		return NULL;
	}

	//TRACE_MSG( "py_playFxDelayed(%s)\n", tag );

	soundMgr().playFxDelayed(tag, delay, Vector3(x, y, z));

	Py_Return;
}
PY_MODULE_FUNCTION( playFxDelayed, BigWorld )


/**
 *	Plays the named sound fx with a delay and attenuation
 */
static PyObject* py_playFxDelayedAtten( PyObject* args )
{
	char* tag;
	float x, y, z, delay, attenuation;

	if (!PyArg_ParseTuple( args, "sff(fff)", &tag, &delay, &attenuation, &x, &y, &z ))
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.playFxDelayed: "
			"Argument parsing error." );
		return NULL;
	}

	//TRACE_MSG( "py_playFxDelayed(%s)\n", tag );

	soundMgr().playFxDelayedAtten(tag, delay, attenuation, Vector3(x, y, z));

	Py_Return;
}
PY_MODULE_FUNCTION( playFxDelayedAtten, BigWorld )



/**
 *	Plays the named Simple sound
 */
static PyObject * py_playSimple( PyObject * args )
{
	char* tag;

	if (!PyArg_ParseTuple( args, "s", &tag ))
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.playSimple: "
			"Argument parsing error." );
		return NULL;
	}

	TRACE_MSG( "py_playSimple(%s)\n", tag );

	soundMgr().playSimple(tag);

	Py_Return;
}
PY_MODULE_FUNCTION( playSimple, BigWorld )

/*pyfxsound.cpp*/
