/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_OBJECT_AUTO_PTR_HPP
#define PY_OBJECT_AUTO_PTR_HPP

#include <Python.h>

/**
 *	Calls Py_XDECREF() on the given PyObject pointer when destroyed. Useful
 *	when the underlying PyObject is created as an automatic variable.
 */
class PyObjectAutoPtr
{
public:
	typedef PyObject * PyObjectPlainPtr;

	PyObjectAutoPtr( PyObject * pyObj, bool shouldIncRef=false ):
			pyObj_( pyObj )
	{
		if (shouldIncRef)
		{
			Py_XINCREF( pyObj_ );
		}
	}

	~PyObjectAutoPtr()
	{
		Py_XDECREF( pyObj_ );
	}

	PyObject * get() 					{ return pyObj_; }

	operator PyObjectPlainPtr() const 	{ return pyObj_; }
	operator bool() const 				{ return pyObj_ != NULL; }
	bool operator!() const 				{ return pyObj_ == NULL; }

private:
	PyObject * 	pyObj_;
};

#endif // PY_OBJECT_AUTO_PTR_HPP
