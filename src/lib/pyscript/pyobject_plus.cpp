/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 * 	@file
 *
 * 	This file implements the PyObjectPlus class.
 *
 * 	@ingroup script
 */


#include "pch.hpp"


// Check reference count and thread safety if in debug mode
// NOTE: this define must match the define in python Object.c file
#ifdef _DEBUG
	#define DEBUG_REFS 1
#endif //_DEBUG


#include "pyobject_plus.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/memory_counter.hpp"
#include "stl_to_py.hpp"
#include "script.hpp"


DECLARE_DEBUG_COMPONENT2( "Script", 0 )


// -----------------------------------------------------------------------------
// Section: Static data
// -----------------------------------------------------------------------------

// Base needs to be 0.
PY_GENERAL_TYPEOBJECT_WITH_BASE( PyObjectPlus, 0 )

PY_BEGIN_METHODS( PyObjectPlus )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyObjectPlus )
PY_END_ATTRIBUTES()

memoryCounterDefine( pyObjPlus, Entity );

// -----------------------------------------------------------------------------
// Section: Construction/Destruction
// -----------------------------------------------------------------------------

extern void watchTypeCount( const char * basePrefix,
	const char * typeName, int & var );

//	Define DEBUG_REFS to get debug messages whenever the 
//	reference counter of a specific PyObject is incremented
//	or decremented. Use the setRefWatchedObject to set the
//	PyObject to be watched. See Python/include/object.h.
//  Note: Enabling DEBUG_REFS also checks that the reference counts, as well
//  as adding/deleting objects, are done from the main thread only to avoid
//  race conditions inside python, and it will assert if not.
#ifdef DEBUG_REFS
	extern "C" void check_thread()
	{
		bool python_accessed_from_main_thread =
			MainThreadTracker::isCurrentThreadMain();
		assert( python_accessed_from_main_thread );
	}

	PyObject * s_ref_watched_object = NULL;

	void setRefWatchedObject(PyObject * object)
	{
		s_ref_watched_object = object;
	}

	extern "C" void intercept_incref(void * object)
	{
		check_thread();
		if ((PyObject*)object == s_ref_watched_object)
		{
			DEBUG_MSG(
				"Watched object being increfed: %"PRIzd"\n", 
				((PyObject*)object)->ob_refcnt+1);
		}
	}
	extern "C" void intercept_decref(void * object)
	{
		check_thread();
		if ((PyObject*)object == s_ref_watched_object)
		{
			DEBUG_MSG(
				"Watched object being decrefed: %"PRIzd"\n", 
				((PyObject*)object)->ob_refcnt-1);
		}
	}
#endif // DEBUG_REFS


/**
 *	Constructor.
 *
 *	@param pType	The type of this object.
 *
 *	@param isInitialised	Specifies whether this object has had its Python
 *		data initialised. If the object was created with new, this should be
 *		false. If it was created with PyType_GenericAlloc or something similar,
 *		this should be true.
 */
PyObjectPlus::PyObjectPlus( PyTypePlus * pType, bool isInitialised )
{
	if (PyType_Ready( pType ) < 0)
	{
		ERROR_MSG( "PyObjectPlus: Type %s is not ready\n", pType->tp_name );
	}

	if (!isInitialised)
	{
		PyObject_Init( this, pType );
	}

#ifdef TRACK_MEMORY_BLOCKS
	if (!PyObject_IS_GC( this ))
	{
		if (pType->tp_name == NULL) return; // in static inits...?
		int & isize = pType->tp_itemsize;
		if (isize++ == 0)
		{
			watchTypeCount( "PyObjCounts/", pType->tp_name, isize );
		}
		else if (isize == 0)
		{
			isize = 1;
		}
	}
#endif
}


/**
 *	Destructor should only be called from Python when its reference count
 *	reaches zero.
 */
PyObjectPlus::~PyObjectPlus()
{
	MF_ASSERT_DEV(this->ob_refcnt == 0);

#ifdef TRACK_MEMORY_BLOCKS
	if (!PyObject_IS_GC( this ))
	{
		int & isize = ob_type->tp_itemsize;
		if (isize-- == 1)
		{
			// 0 implies never seen, so it is not allowed...
			// we don't want to recompile all the headers so we don't
			// use say 0x80000000 to mean unseen
			isize = -1;
		}
	}
#endif

#ifdef Py_TRACE_REFS
	MF_ASSERT( _ob_next == NULL && _ob_prev == NULL );
#endif
}


// -----------------------------------------------------------------------------
// Section: PyObjectPlus - general
// -----------------------------------------------------------------------------

/**
 *	This method returns the attribute with the given name.
 *
 *	@param attr	The name of the attribute.
 *
 *	@return		The value associated with the input name.
 */
PyObject * PyObjectPlus::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();

	// ####Python2.3 We should pass in the PyObject for the name
	PyObject * pName = PyString_FromString( attr );
	PyObject * pResult = PyObject_GenericGetAttr( this, pName );
	Py_DECREF( pName );

	return pResult;
}


/**
 *	This method sets the attribute with the given name to the input value.
 */
int PyObjectPlus::pySetAttribute( const char * attr, PyObject * value )
{
	PY_SETATTR_STD();
	// ####Python2.3 We should pass in the PyObject for the name
	PyObject * pName = PyString_InternFromString( attr );
	int result = PyObject_GenericSetAttr( this, pName, value );
	Py_DECREF( pName );

	return result;
}


/**
 *	This method deletes the attribute with the given name.
 */
int PyObjectPlus::pyDelAttribute( const char * attr )
{
	return this->pySetAttribute( attr, NULL );
}


/**
 *	This method returns the representation of this object as a string.
 */
PyObject * PyObjectPlus::pyRepr()
{
	char	str[512];
	bw_snprintf( str, sizeof(str), "%s at 0x%p",
			this->typeName(), this );

	return PyString_FromString( str );
}

/**
 *	This method is called just before the PyObject is deleted. It is useful
 *	for cleaning up resources before the class destructor is called.
 */
void PyObjectPlus::pyDel() 
{
}


#if 0
/**
 *	Static function to check if the input object is the same type or
 *	a derived type of the type object in the second parameter
 */
bool PyObjectPlus::Check( PyObject * pObject, PyTypeObject * pType )
{
	// first make sure we're looking at a PyObjectPlus
	if (pObject->ob_type->tp_dealloc != PyObjectPlus::_pyDestructor)
	{
		return false;
	}

	// now call its (virtual) parent function to get its parents
	PyTypePlus * pTP = ((PyObjectPlus*)pObject)->pyType();

	// and see if the input type is among them
	while (true)
	{
		if (pType == &pTP->ownType) return true;

		if (pTP->pSuperType == pTP) break;

		pTP = pTP->pSuperType;
	}

	return false;
}
#endif


/**
 *	Base class implementation of _pyNew. Always fails.
 */
PyObject * PyObjectPlus::_pyNew( PyTypeObject * pType, PyObject *, PyObject * )
{
	PyErr_Format( PyExc_TypeError, "%s() "
		"Cannot directly construct objects of this type",
		pType->tp_name );
	return NULL;
}


// -----------------------------------------------------------------------------
// Section: PyDirInfo
// -----------------------------------------------------------------------------


/**
 *	Constructor
 */
PyDirInfo::PyDirInfo() :
	membersHolder_( new PySTLSequenceHolder< Names >( members_, NULL, false ) ),
	methodsHolder_( new PySTLSequenceHolder< Names >( methods_, NULL, false ) ),
	pToAdd_( NULL )
{
}


/**
 *	Destructor
 */
PyDirInfo::~PyDirInfo()
{
	delete methodsHolder_;
	delete membersHolder_;
}


/**
 *	This method makes sure that any strings we're supposed to add have been
 *
 *	@see add
 */
void PyDirInfo::checkAdd()
{
	if (pToAdd_ != NULL)
	{
		const_cast<PyDirInfo*>(pToAdd_)->checkAdd();
		members_.insert( members_.end(),
			pToAdd_->members_.begin(), pToAdd_->members_.end() );
		methods_.insert( methods_.end(),
			pToAdd_->methods_.begin(), pToAdd_->methods_.end() );

		pToAdd_ = NULL;
	}
}


/**
 *	This method returns a python sequence of the members
 */
PyObject * PyDirInfo::members() const
{
	const_cast<PyDirInfo*>(this)->checkAdd();

	// for now always make a new tuple ... I don't want to cache
	// it 'coz it won't get disposed before python is finalised
	//return PyTuple_FromStringVector( members_ );
	return Script::getData( *membersHolder_ );
}

/**
 *	This method returns a python sequence of the methods
 *
 *	@see members
 */
PyObject * PyDirInfo::methods() const
{
	const_cast<PyDirInfo*>(this)->checkAdd();

	//return PyTuple_FromStringVector( methods_ );
	return Script::getData( *methodsHolder_ );
}


/**
 *	This method adds all the information from the input dirinfo to ourselves
 *
 *	To avoid static initialisation order problems, currently we just
 *	take a pointer to the structure and only add them when the information
 *	is requested.
 */
void PyDirInfo::add( const PyDirInfo & odi )
{
	if (&odi == this) return;	// happens for PyObjectPlus's initialisation

	pToAdd_ = &odi;
}


/**
 *	This method adds the member of the given name
 */
void PyDirInfo::addMember( const char * name )
{
	members_.push_back( name );
}

/**
 *	This method adds the method of the given name
 */
void PyDirInfo::addMethod( const char * name )
{
	methods_.push_back( name );
}



// pyobject_plus.cpp
