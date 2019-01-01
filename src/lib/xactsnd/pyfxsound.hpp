/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PYFXSOUND_HPP
#define PYFXSOUND_HPP

#pragma warning( disable:4786 )

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

#include "math/vector3.hpp"

#include "soundmgr.hpp"


class PyFxSound : public PyObjectPlus
{
	Py_Header( PyFxSound, PyObjectPlus )

public:
	PyFxSound( const char* tag, PyTypePlus* pType = &PyFxSound::s_type_ );

	// PyObjectPlus overrides
	PyObject*			pyGetAttribute( const char* attr );
	int					pySetAttribute( const char* attr, PyObject * value );

	//  Methods
	void play();
	PY_METHOD_DECLARE( py_play );

	void stop();
	PY_METHOD_DECLARE( py_stop );


	//  Attributes
//	PY_RO_ATTRIBUTE_DECLARE( snd_->isPlaying(), isPlaying );

//	const Vector3& position() const;
//	void position( const Vector3& newPosition );
//	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Vector3, position, position )

//	void position(const Vector3& newPosition, float dTime);

//	const Vector3& velocity() const  { return snd_->velocity(instance_); }
//	void velocity(const Vector3& newVelocity)  { snd_->velocity(newVelocity, instance_); }
//	PY_RO_ATTRIBUTE_DECLARE( velocity(), velocity );
#if 0
	PyObject* pyGet_frequency();
	int pySet_frequency( PyObject* value );
#endif//0
	const uint instance() const { return instance_; };

	PY_FACTORY_DECLARE()

	bool isValidForPy() const;	/// check class validity (sets exception)

private:
	~PyFxSound();

	PyFxSound(const PyFxSound&);
	PyFxSound& operator=(const PyFxSound&);

	//FxSound* snd_;	
	int instance_;		// -1 indicates invalid instance (ie not playing)
	Vector3 position_;

};


#ifdef CODE_INLINE
#include "pyfxsound.ipp"
#endif




#endif // PYFXSOUND_HPP
