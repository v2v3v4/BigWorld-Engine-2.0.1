/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PYFASHION_HPP
#define PYFASHION_HPP

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

#include "cstdmf/smartpointer.hpp"
class PyModel;
typedef SmartPointer<class Fashion> FashionPtr;

/*~ class BigWorld.PyFashion
 *
 *	This class is the abstract base class for objects that change the 
 *	appearance of a model. In particular, PyDye, ActionQueuer, PyMorphControl
 *	and Tracker all inherit from it.
 */
/**
 *	This class is the base class for python objects that wrap (or can
 *	otherwise provide) a fashion.
 */
class PyFashion : public PyObjectPlus
{
	Py_Header( PyFashion, PyObjectPlus )

public:
	PyFashion( PyTypePlus * pType ) :
		PyObjectPlus( pType ) { }

	enum PyFashionEra
	{
		NEVER = -1,
		EARLY = 0,
		LATE = 1
	};

	/// This method ticks this fashion
	virtual void tick( float dTime, float lod ) { }

	/// This method returns a smart pointer to the underlying fashion
	virtual FashionPtr fashion() = 0;

	/// This method returns the era of the fashion
	virtual PyFashionEra fashionEra()			{ return EARLY; }

	/**
	 *	This method makes a copy of the fashion for the given model,
	 *	and returns a new reference to the copy. If no copy is necessary,
	 *	then another reference to the same fashion may be returned.
	 *	If NULL is returned, an exception must be set.
	 *	This method can also be used as an 'attach' method for fashions
	 *	which prefer to implement attach/detach semantics.
	 *
	 *	@note Do not store a reference to the PyModel, as it would be
	 *	circular.
	 */
	virtual PyFashion * makeCopy( PyModel * pModel, const char * attrName );

	/**
	 *	This method tells us that our model is no longer interested in us,
	 *	and isn't going to call us anymore.
	 */
	virtual void disowned() { }


	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );
};


#endif // PYFASHION_HPP
