/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _PICKLER_HEADER
#define _PICKLER_HEADER

#include "pyobject_plus.hpp"
#include "Python.h"
#include <string>

/**
 * 	This class is a wrapper around the Python pickle methods.
 * 	Essentially, it serialises and deserialises Python objects
 * 	into STL strings.
 *
 * 	@ingroup script
 */
class Pickler
{
public:
	static std::string 		pickle( PyObject * pObj );
	static PyObject * 		unpickle( const std::string & str );

	static bool			init();
	static void			finalise();

private:
	static PyObject *	s_pPickleMethod;
	static PyObject *	s_pUnpickleMethod;
	static int			s_refCount;
};


/**
 *	This class implements a simple Python object that handles the case where
 *	pickle data cannot be unpickled. This object is created to hold onto the
 *	pickled data.
 */
class FailedUnpickle : public PyObjectPlus
{
	Py_Header( FailedUnpickle, PyObjectPlus )

public:
	FailedUnpickle( const std::string & pickleData,
			PyTypePlus * pType = &FailedUnpickle::s_type_ ) :
		PyObjectPlus( pType ),
		pickleData_( pickleData )
	{
	}

	const std::string & pickleData() const	{ return pickleData_; }

private:
	std::string pickleData_;
};


#endif
