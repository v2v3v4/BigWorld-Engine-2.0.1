/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOTOR_HPP
#define MOTOR_HPP


#include "pyscript/pyobject_plus.hpp"


class PyModel;

/*~ class BigWorld.Motor
 *
 *	This is the abstract base class for objects that change a models position
 *	and orientation.
 *	Models contain a list of motors, and each tick every motor gets a chance to
 *  update the position and rotation of the model, as well as the animation
 *  that the model is playing.  Without any motors, Models do not follow
 *	the entity, but remain at their initial position and orientation.
 *
 *	By default, a newly created Model has one Motor, which is an ActionMatcher.
 */
/**
 *	This is a base class for objects that can move a model around.
 *	When it is attached, it stores a model pointer; however, it
 *	doesn't keep a reference to it because that would be a circular
 *	reference.
 *
 *	@see PyModel
 */
class Motor : public PyObjectPlus
{
	Py_Header( Motor, PyObjectPlus )

public:
	Motor( PyTypePlus * pType ) : PyObjectPlus( pType ), pOwner_( NULL ) {}
		// no default argument as this is an abstract class

	virtual void attach( PyModel * pOwner )
		{ pOwner_ = pOwner; this->attached(); }

	virtual void detach()
		{ pOwner_ = NULL; this->detached(); }

	/**
	 *	Call this function to turn the motor one frame. It should
	 *	only be called by the owning PyModel.
	 */
	virtual void rev( float dTime ) = 0;


	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PyObject * pyGet_owner();
	PY_RO_ATTRIBUTE_SET( owner );

	PyModel * pOwner() { return pOwner_; }

protected:
	virtual void attached() {}
	virtual void detached()	{}

	PyModel * pOwner_;	///< The model that owns us, or NULL.
};



#endif // MOTOR_HPP
