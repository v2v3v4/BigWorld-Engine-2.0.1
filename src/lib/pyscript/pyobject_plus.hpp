/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PYOBJECT_PLUS_HPP
#define PYOBJECT_PLUS_HPP

#include "Python.h"

#include "cstdmf/debug.hpp"
#include "cstdmf/stringmap.hpp"

#include "compatibility.hpp"
#include "pyobject_base.hpp"
#include "pyobject_pointer.hpp"

// ####Python2.3 Get rid of this define
#define PyTypePlus PyTypeObject

// -----------------------------------------------------------------------------
// Section: Macros
// -----------------------------------------------------------------------------

/**
 *	General helper macros
 */

/// This method performs a fast string comparison.
inline bool streq( const char * strA, const char * strB )
{
	// Do the initial character comparision first for speed.
	return *strA == *strB && strcmp( strA, strB ) == 0;
}

/// This macro returns the Python Py_None object.
#define Py_Return			{ Py_INCREF( Py_None ); return Py_None; }

/// This macro prints the current Python error if there is one.
#define PY_ERROR_CHECK()													\
{																			\
 	if (PyErr_Occurred())													\
 	{																		\
		ERROR_MSG( "%s(%d): Script Error\n", __FILE__, __LINE__ );			\
		PyErr_Print();														\
	}																		\
}



/**
 *	PyObjectPlus-derived class declaration macros
 */

/// This must be the first line of each PyObjectPlus-derived class.
#define Py_Header( CLASS, SUPER_CLASS )										\
	public:																	\
		static void _tp_dealloc( PyObject * pObj )							\
		{																	\
			static_cast< CLASS * >( pObj )->pyDel();						\
			delete static_cast< CLASS * >( pObj );							\
		}																	\
																			\
		static PyObject * _tp_getattro( PyObject * pObj,					\
				PyObject * name )											\
		{																	\
			return static_cast<CLASS*>(pObj)->pyGetAttribute(				\
					PyString_AS_STRING( name ) );							\
		}																	\
																			\
		static int _tp_setattro( PyObject * pObj,							\
			PyObject * name,												\
			PyObject * value )												\
		{																	\
			const char * attr = PyString_AS_STRING( name );					\
			return (value != NULL) ?										\
				static_cast<CLASS*>(pObj)->pySetAttribute( attr, value ) :	\
				static_cast<CLASS*>(pObj)->pyDelAttribute( attr );			\
		}																	\
																			\
		static PyObject * _tp_repr( PyObject * pObj )						\
		{																	\
			return static_cast<CLASS *>(pObj)->pyRepr();					\
		}																	\
																			\
	Py_InternalHeader( CLASS, SUPER_CLASS )									\

/// This can be used for classes that do not derive from PyObjectPlus
/// @see PY_FAKE_PYOBJECTPLUS_BASE_DECLARE
#define Py_FakeHeader( CLASS, SUPER_CLASS )									\
	public:																	\
		const char * typeName() const { return #CLASS; }					\
																			\
	Py_InternalHeader( CLASS, SUPER_CLASS )									\

/// This internal macro does common header stuff for real and fake
/// PyObjectPlus classes
#define Py_InternalHeader( CLASS, SUPER_CLASS )								\
	public:																	\
		static PyTypeObject				s_type_;							\
																			\
		static bool Check( PyObject * pObject )								\
		{																	\
			if (pObject)													\
				return PyObject_TypeCheck( pObject, &s_type_ );				\
			return false;													\
		}																	\
																			\
		static void Super_addDirInfo()										\
		{																	\
			s_attributes_.di_.add( Super::s_attributes_.di_ );				\
		}																	\
																			\
		typedef SUPER_CLASS Super;											\
																			\
																			\
	Py_CommonHeader( CLASS )


#define Py_InstanceHeader( CLASS ) Py_Header( CLASS, PyObjectPlus )


// This macro is used by both of the macros above
#define Py_CommonHeader( CLASS )											\
	private:																\
		typedef CLASS This;													\
																			\
	public:																	\
		typedef PyObject * (CLASS::*GetMethod)();							\
		typedef int (CLASS::*SetMethod)( PyObject * value );				\
																			\
		typedef PyAttributeAccessor< CLASS > ThisAttributeAccessor;			\
		/** @internal */													\
		class ThisAttributeMap : public StringHashMap<ThisAttributeAccessor>\
		{																	\
		public:																\
			ThisAttributeMap()												\
			{																\
				CLASS::s_InitAttributes_();									\
				CLASS::s_InitMethods_();									\
				CLASS::Super_addDirInfo();									\
			}																\
																			\
			PyDirInfo		di_;											\
		};																	\
																			\
		static ThisAttributeMap			s_attributes_;						\
																			\
																			\
		PyObject * pyGet___members__()										\
			{ return this->pyAdditionalMembers(								\
				s_attributes_.di_.members() ); }							\
		PY_RO_ATTRIBUTE_SET( __members__ )									\
																			\
  		PyObject * pyGet___methods__()										\
			{ return this->pyAdditionalMethods(								\
				s_attributes_.di_.methods() ); }							\
		PY_RO_ATTRIBUTE_SET( __methods__ )									\
																			\
																			\
		int set_py_any( PyObject * /*pValue*/ )								\
		{																	\
			PyErr_Format( PyExc_TypeError,									\
				"Sorry, that method attribute in %s is read-only",			\
				this->typeName() );											\
			return -1;														\
		}																	\
																			\
		static void s_InitMethods_();										\
		/* This ridiculous function is all due to VC6's stupidity with */	\
		/*	the constructors of inner classes of templates classes.	*/		\
		static void s_InitAttributes_();									\
																			\
	private:																\




/// This macro declares a method taking one object argument.
#define PY_UNARY_FUNC_METHOD( METHOD_NAME )									\
	PyObject * METHOD_NAME();												\
	static PyObject * _##METHOD_NAME( PyObject * self )						\
	{																		\
		return ((This*)self)->METHOD_NAME();								\
	}


/// This macro declares a method taking two object arguments.
#define PY_BINARY_FUNC_METHOD( METHOD_NAME )								\
	PyObject * METHOD_NAME( PyObject * a );									\
	static PyObject * _##METHOD_NAME( PyObject * self, PyObject * a )		\
	{																		\
		return ((This*)self)->METHOD_NAME( a );								\
	}


/// This macro declares a method taking three object arguments.
#define PY_TERNARY_FUNC_METHOD( METHOD_NAME )								\
	PyObject * METHOD_NAME( PyObject * a, PyObject * b );					\
	static PyObject * _##METHOD_NAME( PyObject * self, PyObject * a, PyObject * b ) \
	{																		\
		return ((This*)self)->METHOD_NAME( a, b );							\
	}


/// This macro declares a size inquiry method (taking no arguments).
#define PY_SIZE_INQUIRY_METHOD( METHOD_NAME )								\
	Py_ssize_t METHOD_NAME();												\
	static Py_ssize_t _##METHOD_NAME( PyObject * self )						\
	{																		\
		return ((This*)self)->METHOD_NAME();								\
	}

/// This macro declares a method taking no arguments.
#define PY_INQUIRY_METHOD( METHOD_NAME )									\
	int METHOD_NAME();														\
	static int _##METHOD_NAME( PyObject * self )							\
	{																		\
		return ((This*)self)->METHOD_NAME();								\
	}

/// This macro declares a coercion method.
#define PY_COERCION_METHOD( METHOD_NAME )									\
	static int METHOD_NAME( PyObject *& self, PyObject *& a );				\
	static int _##METHOD_NAME( PyObject ** self, PyObject ** a )			\
	{																		\
		return METHOD_NAME( *self, *a );									\
	}


/// This macro declares a method taking one int argument.
#define PY_INTARG_FUNC_METHOD( METHOD_NAME )								\
	PyObject * METHOD_NAME( Py_ssize_t a );									\
	static PyObject * _##METHOD_NAME( PyObject * self, Py_ssize_t a )		\
	{																		\
		return ((This*)self)->METHOD_NAME( a );								\
	}


/// This macro declares a method taking two int arguments.
#define PY_INTINTARG_FUNC_METHOD( METHOD_NAME )								\
	PyObject * METHOD_NAME( Py_ssize_t a, Py_ssize_t b );					\
	static PyObject * _##METHOD_NAME( PyObject * self, 						\
		Py_ssize_t a, Py_ssize_t b )										\
	{																		\
		return ((This*)self)->METHOD_NAME( a, b );							\
	}


/// This macro declares a method taking an int and an object.
#define PY_INTOBJARG_PROC_METHOD( METHOD_NAME )								\
	int METHOD_NAME( Py_ssize_t a, PyObject * b );							\
	static int _##METHOD_NAME( PyObject * self,								\
			Py_ssize_t a, PyObject * b )									\
	{																		\
		return ((This*)self)->METHOD_NAME( a, b );							\
	}


/// This macro declares a method taking two ints and an object.
#define PY_INTINTOBJARG_PROC_METHOD( METHOD_NAME )							\
	int METHOD_NAME( Py_ssize_t a, Py_ssize_t b, PyObject * c );			\
	static int _##METHOD_NAME( PyObject * self,								\
				Py_ssize_t a, Py_ssize_t b, PyObject * c )					\
	{																		\
		return ((This*)self)->METHOD_NAME( a, b, c );						\
	}


/// This macro declares a method taking one object argument (don't ask).
#define PY_OBJOBJ_PROC_METHOD( METHOD_NAME )								\
	int METHOD_NAME( PyObject * a );										\
	static int _##METHOD_NAME( PyObject * self, PyObject * a )				\
	{																		\
		return ((This*)self)->METHOD_NAME( a );								\
	}


/* More macros that may need to be declared... (from object.h)
typedef int (*getreadbufferproc)(PyObject *, int, void **);
typedef int (*getwritebufferproc)(PyObject *, int, void **);
typedef int (*getsegcountproc)(PyObject *, int *);
typedef int (*getcharbufferproc)(PyObject *, int, const char **);
typedef int (*objobjproc)(PyObject *, PyObject *);
typedef int (*visitproc)(PyObject *, void *);
typedef int (*traverseproc)(PyObject *, visitproc, void *);
*/




/// This macro declares a standard python method.
#define PY_METHOD_DECLARE( METHOD_NAME )									\
	PY_METHOD_DECLARE_WITH_DOC( METHOD_NAME, NULL )							\

#define PY_METHOD_DECLARE_WITH_DOC( METHOD_NAME, DOC_STRING )				\
	PyObject * METHOD_NAME( PyObject * args );								\
																			\
	static PyObject * _##METHOD_NAME( PyObject * self,						\
		PyObject * args, PyObject * /*kwargs*/ )							\
	{																		\
		return ((This *)self)->METHOD_NAME( args );							\
	}																		\
																			\
	PY_METHOD_ATTRIBUTE_WITH_DOC( METHOD_NAME, DOC_STRING )					\

/// This macro declares an auto-parsed standard python method
#define PY_AUTO_METHOD_DECLARE( RET, NAME, ARGS )							\
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RET, NAME, ARGS, NULL )				\

#define PY_AUTO_METHOD_DECLARE_WITH_DOC( RET, NAME, ARGS, DOC_STRING )		\
	static PyObject * _py_##NAME(											\
		PyObject * self, PyObject * args, PyObject * /*kwargs*/ )			\
	{																		\
		This * pThis = (This*)self;											\
		PY_AUTO_DEFINE_INT( RET, NAME, pThis->NAME, ARGS )					\
	}																		\
																			\
	PY_METHOD_ATTRIBUTE_WITH_DOC( py_##NAME, DOC_STRING )					\

/// This macro declares a python method that uses keywords.
#define PY_KEYWORD_METHOD_DECLARE( METHOD_NAME )							\
	PY_KEYWORD_METHOD_DECLARE_WITH_DOC( METHOD_NAME, NULL )					\

#define PY_KEYWORD_METHOD_DECLARE_WITH_DOC( METHOD_NAME, DOC_STRING )		\
	PyObject * METHOD_NAME( PyObject * args, PyObject * kwargs );			\
																			\
	static PyObject * _##METHOD_NAME( PyObject * self,						\
		PyObject * args, PyObject * kwargs )								\
	{																		\
		return ((This *)self)->METHOD_NAME( args, kwargs );					\
	}																		\
																			\
	PY_METHOD_ATTRIBUTE_WITH_FLAGS_WITH_DOC( METHOD_NAME, METH_VARARGS|METH_KEYWORDS, DOC_STRING ) \

/// This macro can be overridden to temporarily change the base method
/// definition. This can be useful for classes that do not derive from
/// PyObject but still want to contain indirect Python functionality
#define PY_METHOD_ATTRIBUTE PY_METHOD_ATTRIBUTE_BASE
#define PY_METHOD_ATTRIBUTE_WITH_DOC PY_METHOD_ATTRIBUTE_BASE_WITH_DOC

/// This macro defines the attribute function for a method
#define PY_METHOD_ATTRIBUTE_WITH_FLAGS( METHOD_NAME, FLAGS )				\
	PY_METHOD_ATTRIBUTE_WITH_FLAGS_WITH_DOC( METHOD_NAME, NULL )			\

#define PY_METHOD_ATTRIBUTE_WITH_FLAGS_WITH_DOC(							\
									METHOD_NAME, FLAGS, DOC_STRING )		\
	PyObject * get_##METHOD_NAME()											\
	{																		\
		static PyMethodDef md = {											\
			#METHOD_NAME,													\
			(PyCFunction)&_##METHOD_NAME,									\
			FLAGS,															\
			PY_GET_DOC( DOC_STRING ) };													\
		return PyCFunction_New( &md, this );								\
	}

#define PY_METHOD_ATTRIBUTE_BASE( METHOD_NAME )								\
	PY_METHOD_ATTRIBUTE_BASE_WITH_DOC( METHOD_NAME, NULL )

#define PY_METHOD_ATTRIBUTE_BASE_WITH_DOC( METHOD_NAME, DOC_STRING )		\
	PY_METHOD_ATTRIBUTE_WITH_FLAGS_WITH_DOC( METHOD_NAME, METH_VARARGS, DOC_STRING )


/// This macro declares a static python method
#define PY_STATIC_METHOD_DECLARE( NAME )									\
	static PyObject * NAME( PyObject * pArgs );								\
																			\
	static PyObject * _##NAME( PyObject *,									\
		PyObject * args, PyObject * )										\
	{																		\
		return This::NAME( args );											\
	}																		\

/// This macro declares an auto-parsed static python method
#define PY_AUTO_STATIC_METHOD_DECLARE( RET, NAME, ARGS )					\
	static PyObject * _py_##NAME(											\
		PyObject * self, PyObject * args, PyObject * kwargs )				\
	{																		\
		PY_AUTO_DEFINE_INT( RET, NAME, This::NAME, ARGS )					\
	}																		\


/// This macro declares the python factory method (pyNew) for this type
#define PY_FACTORY_DECLARE()												\
	PY_STATIC_METHOD_DECLARE( pyNew )										\
	PY_FACTORY_METHOD_LINK_DECLARE()										\

/// This macro declares the python factory method (New) as auto-parsed
#define PY_AUTO_FACTORY_DECLARE( CLASS_NAME, ARGS )							\
	static PyObject * _pyNew( PyObject *, PyObject * args )					\
	{																		\
		PY_AUTO_DEFINE_INT( RETOWN, CLASS_NAME, This::New, ARGS )			\
	}																		\
	PY_FACTORY_METHOD_LINK_DECLARE()										\


/// This macro declares the default factory method for the given class
/// It calls the constructor with no (or default) arguments
#define PY_DEFAULT_CONSTRUCTOR_FACTORY_DECLARE()							\
	static PyObject * _pyNew( PyObject *, PyObject * args )					\
	{																		\
		return new This();													\
	}																		\
	PY_FACTORY_METHOD_LINK_DECLARE()										\

/// This macro declares the default factory method for the given class
/// It calls the constructor with no (or default) arguments
#define PY_AUTO_CONSTRUCTOR_FACTORY_DECLARE( CLASS_NAME, ARGS )				\
	static PyObject * _pyNew( PyObject *, PyObject * args )					\
	{																		\
		PY_AUTO_DEFINE_INT( RETOWN, CLASS_NAME, new This, ARGS )			\
	}																		\
	PY_FACTORY_METHOD_LINK_DECLARE()										\

/// This macro is used internally by factory declaration macros
#define PY_FACTORY_METHOD_LINK_DECLARE()									\
	static PyFactoryMethodLink s_link_pyNew;								\


/// This macro declares 'setstate' and 'getstate' methods for simple pickling
#define PY_GETSETSTATE_METHODS_DECLARE()									\
	PY_METHOD_DECLARE( py___getstate__ )									\
	PY_METHOD_DECLARE( py___setstate__ )									\

/// This macro declares a 'reduce' method used for nontrivial pickling
/// The pyPickleReduce method should return a tuple of picklable arguments
/// to be passed to the construction function when the object is unpickled.
#define PY_PICKLING_METHOD_DECLARE( CONS_NAME )								\
	PyObject * pyPickleReduce();											\
																			\
	static PyObject * _py___reduce_ex__( PyObject * self, PyObject * )		\
	{																		\
		PyObject * pConsArgs = ((This*)self)->pyPickleReduce();				\
		return Script::buildReduceResult( #CONS_NAME, pConsArgs );			\
	}																		\
																			\
	PY_METHOD_ATTRIBUTE( py___reduce_ex__ )									\



/// This macro declares a python attribute that is implemented elsewhere
#define PY_DEFERRED_ATTRIBUTE_DECLARE( NAME )								\
	PyObject * PY_ATTR_SCOPE pyGet_##NAME();								\
	int PY_ATTR_SCOPE pySet_##NAME( PyObject * value );						\

/// This macro declares a class member that is read-write accessible from
///	python as an attribute
#define PY_RW_ATTRIBUTE_DECLARE( MEMBER, NAME )								\
	PY_READABLE_ATTRIBUTE_GET( MEMBER, NAME )								\
	PY_WRITABLE_ATTRIBUTE_SET( MEMBER, NAME )								\

/// This macro declares a class member that is read-write accessible from
///	python as an attribute
#define PY_RW_ATTRIBUTE_REF_DECLARE( MEMBER, NAME )							\
	PY_READABLE_ATTRIBUTE_REF_GET( MEMBER, NAME )							\
	PY_WRITABLE_ATTRIBUTE_SET( MEMBER, NAME )								\

/// This macro declares a class member that is read-only accessible from
///	python as an attribute
#define PY_RO_ATTRIBUTE_DECLARE( MEMBER, NAME )								\
	PY_RO_ATTRIBUTE_GET( MEMBER, NAME )										\
	PY_RO_ATTRIBUTE_SET( NAME )												\

/// This macro declares a class member that is write-only accessible from
///	python as an attribute
#define PY_WO_ATTRIBUTE_DECLARE( MEMBER, NAME )								\
	PY_WO_ATTRIBUTE_GET( NAME )												\
	PY_WRITABLE_ATTRIBUTE_SET( MEMBER, NAME )								\

/// This macro declares a class function that is write-only accessible from
///	python as an attribute
#define PY_WO_ATTRIBUTE_SETTER_DECLARE( TYPE, MEMBER, NAME )				\
	PY_WO_ATTRIBUTE_GET( NAME )												\
	PY_WRITABLE_ATTRIBUTE_SETTER( TYPE, MEMBER, NAME )						\

/// This macro defines the function that gets a readable attribute
#define PY_READABLE_ATTRIBUTE_GET( MEMBER, NAME )							\
	PyObject * PY_ATTR_SCOPE pyGet_##NAME()									\
		{ return Script::getData( MEMBER ); }								\

/// This macro defines the function that gets a readable attribute
#define PY_RO_ATTRIBUTE_GET( MEMBER, NAME )									\
	PyObject * PY_ATTR_SCOPE pyGet_##NAME()									\
		{ return Script::getReadOnlyData( MEMBER ); }						\

/// This macro defines the function that gets a readable attribute
#define PY_READABLE_ATTRIBUTE_REF_GET( MEMBER, NAME )						\
	PyObject * PY_ATTR_SCOPE pyGet_##NAME()									\
		{ return Script::getDataRef( this, &MEMBER ); }						\

/// This macro defines the error function that is called when an attempt
///	is made to set a read-only attribute
#define PY_RO_ATTRIBUTE_SET( NAME )											\
	int PY_ATTR_SCOPE pySet_##NAME( PyObject * /*value*/ )					\
	{																		\
		PyErr_Format( PyExc_TypeError,										\
			"Sorry, the attribute " #NAME " in %s is read-only",			\
			this->typeName() );												\
		return -1;															\
	}																		\

/// This macro defines the error function that is called when an attempt
///	is made to get a write-only attribute
#define PY_WO_ATTRIBUTE_GET( NAME )											\
	PyObject * PY_ATTR_SCOPE pyGet_##NAME()									\
	{																		\
		PyErr_Format( PyExc_TypeError,										\
			"Sorry, the attribute " #NAME " in %s is write-only",			\
			this->typeName() );												\
		return NULL;														\
	}																		\

/// This macro defines the function that sets a writable attribute
#define PY_WRITABLE_ATTRIBUTE_SET( MEMBER, NAME )							\
	int PY_ATTR_SCOPE pySet_##NAME( PyObject * value )						\
		{ return Script::setData( value, MEMBER, #NAME ); }					\

/// This macro defines the function that calls a setter function
#define PY_WRITABLE_ATTRIBUTE_SETTER( TYPE, SETTER, NAME )					\
	int PY_ATTR_SCOPE pySet_##NAME( PyObject * value )						\
		{																	\
			TYPE convertedValue;											\
			Script::setData( value, convertedValue, #NAME );				\
			return SETTER( convertedValue );								\
		}																	\


/// This macro declares a class member that is read-write accessible from
///	python as an attribute, and accessed using accessors
#define PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( TYPE, MEMBERFN, NAME )			\
	PY_READABLE_ATTRIBUTE_GET( this->MEMBERFN(), NAME )						\
	PY_WRITABLE_ACCESSOR_ATTRIBUTE_SET( TYPE, MEMBERFN, NAME )				\


/// This macro defines the function that sets an attribute through an accessor
#define PY_WRITABLE_ACCESSOR_ATTRIBUTE_SET( TYPE, MEMBERFN, NAME )			\
	int PY_ATTR_SCOPE pySet_##NAME( PyObject * value )						\
	{																		\
		typedef TYPE LocalTypeDef;	/* for calling constructor */			\
		TYPE newVal = LocalTypeDef();										\
		int ret = Script::setData( value, newVal, #NAME );					\
		if (ret == 0) this->MEMBERFN( newVal );								\
		return ret;															\
	}																		\


/// This macro can be defined to a class name to use the attribute
/// declaration macros below as method definitions
#define PY_ATTR_SCOPE


/**
 *	PyObjectPlus-derived class static data instantiation macros
 */

/// This high-level macro defines a simple type object
#define PY_TYPEOBJECT 				PY_GENERAL_TYPEOBJECT

#define PY_TYPEOBJECT_WITH_DOC 		PY_GENERAL_TYPEOBJECT_WITH_DOC

/// This high-level macro defines a type object with sequence methods
#define PY_TYPEOBJECT_WITH_SEQUENCE( THIS_CLASS, SEQ )						\
	PY_TYPEOBJECT_WITH_SEQUENCE_WITH_DOC( THIS_CLASS, SEQ, 0 )

#define PY_TYPEOBJECT_WITH_SEQUENCE_WITH_DOC( THIS_CLASS, SEQ, DOC )		\
	PY_TYPEOBJECT_SPECIALISE_SEQ( THIS_CLASS, SEQ )							\
	PY_TYPEOBJECT_SPECIALISE_DOC( THIS_CLASS, DOC )							\
	PY_GENERAL_TYPEOBJECT( THIS_CLASS )

/// This high-level macro defines a type object with mapping methods
#define PY_TYPEOBJECT_WITH_MAPPING( THIS_CLASS, MAP )						\
	PY_TYPEOBJECT_WITH_MAPPING_WITH_DOC( THIS_CLASS, MAP, 0 )

#define PY_TYPEOBJECT_WITH_MAPPING_WITH_DOC( THIS_CLASS, MAP, DOC )			\
	PY_TYPEOBJECT_SPECIALISE_DOC( THIS_CLASS, DOC )							\
	PY_TYPEOBJECT_SPECIALISE_MAP( THIS_CLASS, MAP )							\
	PY_GENERAL_TYPEOBJECT( THIS_CLASS )

/// This high-level macro defines a type object with a call method
#define PY_TYPEOBJECT_WITH_CALL( THIS_CLASS )								\
	PY_TYPEOBJECT_WITH_CALL_WITH_DOC( THIS_CLASS, 0 )

#define PY_TYPEOBJECT_WITH_CALL_WITH_DOC( THIS_CLASS, DOC )					\
	PY_TYPEOBJECT_SPECIALISE_DOC( THIS_CLASS, DOC )							\
	PY_TYPEOBJECT_SPECIALISE_CALL( THIS_CLASS, &THIS_CLASS::_pyCall )		\
	PY_GENERAL_TYPEOBJECT( THIS_CLASS )

/// This macro defines a type object that supports the iterator protocol
#define PY_TYPEOBJECT_WITH_ITER( THIS_CLASS, GETITER, ITERNEXT )			\
	PY_TYPEOBJECT_WITH_ITER_WITH_DOC( THIS_CLASS, GETITER, ITERNEXT, 0 )

#define PY_TYPEOBJECT_WITH_ITER_WITH_DOC( THIS_CLASS, GETITER, ITERNEXT, 	\
		DOC )																\
	PY_TYPEOBJECT_SPECIALISE_DOC( THIS_CLASS, DOC )							\
	PY_TYPEOBJECT_SPECIALISE_ITER( THIS_CLASS, GETITER, ITERNEXT )			\
	PY_GENERAL_TYPEOBJECT( THIS_CLASS )

/// Low-level macros for specialising PyObjectTypeUtil functions.

#define PY_TYPEOBJECT_SPECIALISE_BASIC_SIZE( THIS_CLASS, BASIC_SIZE)		\
	PY_TYPEOBJECT_SPECIALISE_SIMPLE( THIS_CLASS,							\
		basicSize, int, BASIC_SIZE )

#define PY_TYPEOBJECT_SPECIALISE_ITER( THIS_CLASS, GETITER, ITERNEXT )		\
	PY_TYPEOBJECT_SPECIALISE_SIMPLE( THIS_CLASS,							\
		getIterFunction, getiterfunc, GETITER )								\
	PY_TYPEOBJECT_SPECIALISE_SIMPLE( THIS_CLASS,							\
		iterNextFunction, iternextfunc, ITERNEXT )

#define PY_TYPEOBJECT_SPECIALISE_CMP( THIS_CLASS, CMP )						\
	PY_TYPEOBJECT_SPECIALISE_SIMPLE( THIS_CLASS,							\
		compareFunction, cmpfunc, CMP )

#define PY_TYPEOBJECT_SPECIALISE_REPR_AND_STR( THIS_CLASS, REPR, STR )		\
	PY_TYPEOBJECT_SPECIALISE_SIMPLE( THIS_CLASS,							\
		reprFunction, reprfunc, REPR )										\
	PY_TYPEOBJECT_SPECIALISE_SIMPLE( THIS_CLASS,							\
		strFunction, reprfunc, STR )

#define PY_TYPEOBJECT_SPECIALISE_NUM( THIS_CLASS, NUM )						\
	PY_TYPEOBJECT_SPECIALISE_SIMPLE( THIS_CLASS, 							\
		asNumber, PyNumberMethods *, NUM )

#define PY_TYPEOBJECT_SPECIALISE_SEQ( THIS_CLASS, SEQ )						\
	PY_TYPEOBJECT_SPECIALISE_SIMPLE( THIS_CLASS, 							\
		asSequence, PySequenceMethods *, SEQ )

#define PY_TYPEOBJECT_SPECIALISE_MAP( THIS_CLASS, MAP )						\
	PY_TYPEOBJECT_SPECIALISE_SIMPLE( THIS_CLASS, 							\
		asMapping, PyMappingMethods *, MAP )

#define PY_TYPEOBJECT_SPECIALISE_CALL( THIS_CLASS, CALL )					\
	PY_TYPEOBJECT_SPECIALISE_SIMPLE( THIS_CLASS, 							\
		callFunction, ternaryfunc, CALL )

#define PY_TYPEOBJECT_SPECIALISE_FLAGS( THIS_CLASS, FLAGS )					\
	PY_TYPEOBJECT_SPECIALISE_SIMPLE( THIS_CLASS,							\
		flags, long, FLAGS )

#define PY_TYPEOBJECT_SPECIALISE_DOC( THIS_CLASS, DOC )						\
	PY_TYPEOBJECT_SPECIALISE_SIMPLE( THIS_CLASS, doc, const char *, DOC )

#define PY_TYPEOBJECT_SPECIALISE_SIMPLE( THIS_CLASS, TEMPLATE_FUNC,  		\
		RET_TYPE, RET_VALUE )												\
	namespace PyTypeObjectUtil 												\
	{																		\
		template<>															\
		RET_TYPE TEMPLATE_FUNC< THIS_CLASS >()								\
		{																	\
			return RET_VALUE; 												\
		}																	\
	} // end namespace PyTypeObjectUtil



/// This low-level macro defines the PyTypeObject for this class.
#define PY_GENERAL_TYPEOBJECT( THIS_CLASS )									\
	PY_GENERAL_TYPEOBJECT_WITH_BASE( THIS_CLASS, &Super::s_type_ ) 			\

/// This low-level macro defines the PyTypeObject for this class, with the
/// given docstring.
#define PY_GENERAL_TYPEOBJECT_WITH_DOC( THIS_CLASS, DOC )					\
	PY_GENERAL_TYPEOBJECT_WITH_BASE_WITH_NAME( THIS_CLASS, &Super::s_type_, \
		#THIS_CLASS )

/// This low-level macro defines the PyTypeObject for this class, with the
/// given base class.
#define PY_GENERAL_TYPEOBJECT_WITH_BASE( THIS_CLASS, BASE )					\
	PY_GENERAL_TYPEOBJECT_WITH_BASE_WITH_NAME( THIS_CLASS, BASE, 			\
		#THIS_CLASS )

/// This low-level macro defines the PyTypeObject for this class, with the
///	given base class and the / given docstring.
#define PY_GENERAL_TYPEOBJECT_WITH_BASE_WITH_NAME( THIS_CLASS, BASE, NAME )	\
																			\
	PyTypeObject THIS_CLASS::s_type_ =										\
	{																		\
		PyObject_HEAD_INIT(&PyType_Type)									\
		0,									/* ob_size */					\
		const_cast< char * >( NAME ),		/* tp_name */					\
		PyTypeObjectUtil::basicSize< THIS_CLASS >(),						\
											/* tp_basicsize */				\
		0,									/* tp_itemsize */				\
																			\
		/* methods */														\
		THIS_CLASS::_tp_dealloc,			/* tp_dealloc */				\
		0,									/* tp_print */					\
		0,									/* tp_getattr */				\
		0,									/* tp_setattr */				\
		PyTypeObjectUtil::compareFunction< THIS_CLASS >(),					\
											/* tp_compare */				\
		PyTypeObjectUtil::reprFunction< THIS_CLASS >(),						\
											/* tp_repr */					\
		PyTypeObjectUtil::asNumber< THIS_CLASS >(),							\
											/* tp_as_number */				\
		PyTypeObjectUtil::asSequence< THIS_CLASS >(),						\
											/* tp_as_sequence */			\
		PyTypeObjectUtil::asMapping< THIS_CLASS >(),						\
											/* tp_as_mapping */				\
		0,									/* tp_hash */					\
		PyTypeObjectUtil::callFunction< THIS_CLASS >(),						\
											/* tp_call */					\
		PyTypeObjectUtil::strFunction< THIS_CLASS >(),						\
											/* tp_str */					\
		THIS_CLASS::_tp_getattro,			/* tp_getattro */				\
		THIS_CLASS::_tp_setattro,			/* tp_setattro */				\
		0,									/* tp_as_buffer */				\
		PyTypeObjectUtil::flags< THIS_CLASS >(),							\
											/* tp_flags */					\
		const_cast< char * >(												\
				PY_GET_DOC( PyTypeObjectUtil::doc< THIS_CLASS >() ) ),		\
											/* tp_doc */					\
		0,									/* tp_traverse */				\
		0,									/* tp_clear */					\
		0,									/* tp_richcompare */			\
		PyTypeObjectUtil::weakListOffset( (THIS_CLASS*)100 ),				\
											/* tp_weaklistoffset */			\
		PyTypeObjectUtil::getIterFunction< THIS_CLASS >(),					\
											/* tp_iter */					\
		PyTypeObjectUtil::iterNextFunction< THIS_CLASS >(),					\
											/* tp_iternext */				\
		0,									/* tp_methods */				\
		0,									/* tp_members */				\
		0,									/* tp_getset */					\
		BASE,								/* tp_base */					\
		0,									/* tp_dict */					\
		0,									/* tp_descr_get */				\
		0,									/* tp_descr_set */				\
		0,									/* tp_dictoffset */				\
		0,									/* tp_init */					\
		0,									/* tp_alloc */					\
		(newfunc)THIS_CLASS::_pyNew,		/* tp_new */					\
		0,									/* tp_free */					\
		0,									/* tp_is_gc */					\
		0,									/* tp_bases */					\
		0,									/* tp_mro */					\
		0,									/* tp_cache */					\
		0,									/* tp_subclasses */				\
		0,									/* tp_weaklist */				\
		0									/* tp_del */					\
	};


namespace PyTypeObjectUtil
{

template< typename T >
int basicSize()
{
	return sizeof( T );
}

template< typename T >
cmpfunc compareFunction()
{
	return 0;
}

template< typename T >
reprfunc reprFunction()
{
	return &T::_tp_repr;
}

template< typename T >
PyNumberMethods * asNumber()
{
	return 0;
}

template< typename T >
PySequenceMethods * asSequence()
{
	return 0;
}

template< typename T >
PyMappingMethods * asMapping()
{
	return 0;
}

template< typename T >
ternaryfunc callFunction()
{
	return 0;
}

template< typename T >
reprfunc strFunction()
{
	return 0;
}

template< typename T >
long flags()
{
	return Py_TPFLAGS_DEFAULT;
}

template< typename T >
const char * doc()
{
	return 0;
}

template< typename T >
getiterfunc getIterFunction()
{
	return 0;
}

template< typename T >
iternextfunc iterNextFunction()
{
	return 0;
}

inline Py_ssize_t weakListOffset( void* )
{
	return 0;
}

} // end namespace PyTypeObjectUtil


// This internal macro adds the named attribute to the map
#define PY_ATTRIBUTE_INSERT( NAME, GET, SET )								\
	s_attributes_.insert( std::pair< const char *, ThisAttributeAccessor >(	\
		#NAME, ThisAttributeAccessor( &ThisClass::GET, &ThisClass::SET ) ) )\


/// This macro initiates the method definitions for the given class
#define PY_BEGIN_METHODS( THIS_CLASS )										\
	void THIS_CLASS::s_InitMethods_()										\
	{																		\
		typedef THIS_CLASS ThisClass;										\

/// This macro defines a Python method
#define PY_METHOD( NAME )													\
	PY_ATTRIBUTE_INSERT( NAME, get_py_##NAME, set_py_any );					\
	s_attributes_.di_.addMethod( #NAME );									\

/// This macro defines an alias for a Python method
#define PY_METHOD_ALIAS( NAME, ALIAS )										\
	PY_ATTRIBUTE_INSERT( ALIAS, get_py_##NAME, set_py_any );				\
	s_attributes_.di_.addMethod( #ALIAS );									\

/// This macro concludes the method definitions
#define PY_END_METHODS()													\
	}																		\


/// This macro initiates the attribute definitions for the given class.
#define PY_BEGIN_ATTRIBUTES( THIS_CLASS )									\
	THIS_CLASS::ThisAttributeMap THIS_CLASS::s_attributes_;					\
	void THIS_CLASS::s_InitAttributes_()									\
	{																		\
		typedef THIS_CLASS ThisClass;										\

/// This macro defines a Python attribute
#define PY_ATTRIBUTE( NAME )												\
		PY_ATTRIBUTE_INSERT( NAME, pyGet_##NAME, pySet_##NAME );			\
		s_attributes_.di_.addMember( #NAME );								\

/// This macro defines a Python attribute
#define PY_ATTRIBUTE_ALIAS( NAME, NEW_NAME )								\
		PY_ATTRIBUTE_INSERT( NEW_NAME, pyGet_##NAME, pySet_##NAME );		\
		s_attributes_.di_.addMember( #NEW_NAME );							\

/// This macro terminates the attribute definitions
#define PY_END_ATTRIBUTES()													\
		PY_ATTRIBUTE_INSERT( __members__, pyGet___members__, pySet___members__ );	\
		PY_ATTRIBUTE_INSERT( __methods__, pyGet___methods__, pySet___members__ );	\
	}																		\


/// This macro defines the factory method for the given class
#define PY_FACTORY( THIS_CLASS, MODULE_NAME )								\
	PY_FACTORY_NAMED( THIS_CLASS,											\
		THIS_CLASS::s_type_.tp_name, MODULE_NAME )							\

/// This macro defines the factory method with python name for the given class
#define PY_FACTORY_NAMED( THIS_CLASS, METHOD_NAME, MODULE_NAME )			\
	PyFactoryMethodLink THIS_CLASS::s_link_pyNew = PyFactoryMethodLink(		\
		#MODULE_NAME, METHOD_NAME, &THIS_CLASS::s_type_ );					\
	/* can't call the constructor directly with VC6 templates (don't ask) */\
	PY_CLASS_TOKEN( THIS_CLASS )											\

/// This macro defines a token for this class, but not inside it
#define PY_CLASS_TOKEN_BASE( THIS_CLASS )									\
	int THIS_CLASS##_token = true;											\

/// This macro uses the macro above. Template classes will need to redefine it
#define PY_CLASS_TOKEN( THIS_CLASS )										\
	PY_CLASS_TOKEN_BASE( THIS_CLASS )										\


/// This macro defines getstate and setstate methods
#define PY_GETSETSTATE_METHODS()											\
	PY_METHOD( __getstate__ )												\
	PY_METHOD( __setstate__ )												\

/// This macro implements standard getstate and setstate methods using
/// script converters (only works with pod types).
#define PY_CONVERTERS_GETSETSTATE_METHODS( THIS_CLASS, POD_TYPE )			\
	PyObject * THIS_CLASS::py___getstate__( PyObject * args )				\
	{																		\
		POD_TYPE basicType;													\
		if (Script::setData( this, basicType, #THIS_CLASS " setstate" ) != 0)\
			return NULL;													\
		return PyString_FromStringAndSize(									\
			(char*)&basicType, sizeof(basicType) );							\
	}																		\
																			\
	PyObject * THIS_CLASS::py___setstate__( PyObject * args )				\
	{																		\
		PyObject * soleArg;													\
		if (PyTuple_Size( args ) != 1 ||									\
			!PyString_Check( soleArg = PyTuple_GET_ITEM( args, 0 ) ) ||		\
			PyString_Size( soleArg ) != sizeof( POD_TYPE ))					\
		{																	\
			PyErr_SetString( PyExc_TypeError, #THIS_CLASS " getstate "		\
				"expects a single string argument of correct length" );		\
		}																	\
		/* not sure how to use getData here... */							\
		/* *this = (POD_TYPE*)PyString_AsString( soleArg ); */				\
		PyObject * goodValue = Script::getData(								\
			(POD_TYPE*)PyString_AsString( soleArg ) );						\
		this->copy( *goodValue );	/* hmmm */								\
		/* this would prolly be better done with normal pickling then...*/	\
		/* ... ah well, next commit :) */									\
		Py_Return;															\
	}																		\

/// This macro defines a pickling reduce method in the method definitions list
#define PY_PICKLING_METHOD()												\
	PY_METHOD( __reduce_ex__ )												\




/**
 *	pyGetAttribute and pySetAttribute method helper macros
 */

///	This macro does standard pyGetAttribute processing
#define PY_GETATTR_STD()													\
	ThisAttributeMap::iterator found = s_attributes_.find( attr );			\
	if (found != s_attributes_.end())										\
	{																		\
		return (this->*(*found).second.get)();								\
	}																		\
	(void)0																	\


///	This macro does standard pySetAttribute processing
#define PY_SETATTR_STD()													\
	ThisAttributeMap::iterator found = s_attributes_.find( attr );			\
	if (found != s_attributes_.end())										\
	{																		\
		return (this->*(*found).second.set)( value );						\
	}																		\
	(void)0																	\





// -----------------------------------------------------------------------------
// Section: PyAttributeAccessor
// -----------------------------------------------------------------------------

/**
 *	This template class stores get and set methods for an attribute
 */
template < class CLASS >
class PyAttributeAccessor
{
public:
	typedef PyObject * (CLASS::*GetMethod)();
	typedef int (CLASS::*SetMethod)( PyObject * value );

	PyAttributeAccessor( GetMethod get_in, SetMethod set_in ):
		get(get_in), set(set_in) {}

	GetMethod	get;
	SetMethod	set;
};


// -----------------------------------------------------------------------------
// Section: PyDirInfo
// -----------------------------------------------------------------------------

/**
 *	This class holds lists of strings that are used by the 'dir' command
 */
class PyDirInfo
{
public:
	PyDirInfo();
	~PyDirInfo();

	PyObject * members() const;
	PyObject * methods() const;

	void add( const PyDirInfo & odi );

	void addMember( const char * name );
	void addMethod( const char * name );

	typedef std::vector< std::string > Names;
	const Names & membersVector() const	{ return members_; }
	const Names & methodsVector() const	{ return methods_; }

private:
	Names	members_;
	Names	methods_;

	class PySTLSequenceHolderBase * membersHolder_;
	class PySTLSequenceHolderBase * methodsHolder_;

	const PyDirInfo	* pToAdd_;
	void checkAdd();
};



// -----------------------------------------------------------------------------
// Section: PyObjectPlus
// -----------------------------------------------------------------------------

/*~ class NoModule.PyObjectPlus
 *	@components{ all }
 *
 *	This is the abstract base class for all Python classes which are implemented
 *	in the engine.  Every other class inherits, directly or indirectly
 *	from PyObjectPlus.
 */
/**
 *	This class is a base class for all implementations of our Python types.
 *	It helps make implementing a Python type easy.
 *
 * 	@ingroup script
 */
class PyObjectPlus : public PyObject
{
	Py_Header( PyObjectPlus, PyObjectPlus )

	public:
		PyObjectPlus( PyTypeObject * pType, bool isInitialised = false );

		/// This method increments the reference count on this object.
		void incRef() const				{ Py_INCREF( (PyObject*)this ); }
		/// This method decrements the reference count on this object.
		void decRef() const				{ Py_DECREF( (PyObject*)this ); }
		/// This method returns the reference count on this object.
		Py_ssize_t refCount() const		{ return ((PyObject*)this)->ob_refcnt; }

		PyObject * pyGetAttribute( const char * attr );
		int pySetAttribute( const char * attr, PyObject * value );
		int pyDelAttribute( const char * attr );
		void pyDel();

		PyObject * pyRepr();

		// PyTypeObject * pyType()	{ return ob_type; }

		const char * typeName() const
		{
			return ob_type->tp_name;
		}

		// ####Python2.3 Do we still want these? There is an official way to do
		// this now. Need to fully remove it.
		/// This virtual method can be overridden to add the names
		/// of any additional members to a __members__ query
		PyObject * pyAdditionalMembers( PyObject * pBaseSeq )
		{
			return pBaseSeq;
		}

		/// This virtual method can be overridden to add the names
		/// of any additional methods to a __methods__ query
		PyObject * pyAdditionalMethods( PyObject * pBaseSeq )
		{
			return pBaseSeq;
		}

		static PyObject * _pyNew( PyTypeObject * t, PyObject *, PyObject * );
		static PyObjectPtr coerce( PyObject * pObject )
		{
			return pObject;
		}

	protected:

		/**
		 * This destructor is protected because nobody should call it
		 * directly except Python, when the reference count reaches zero.
		 */
		~PyObjectPlus();
};

typedef PyObjectPlus PyInstancePlus;

/**
 *	Little class to make sure objects are ordered in the right way
 */
class PyObjectPlusWithVD : public PyObjectPlus
{
public:
	PyObjectPlusWithVD( PyTypePlus * pType ) : PyObjectPlus( pType ) { }
	virtual ~PyObjectPlusWithVD() { }
};


/**
 *	Classes to enable derivated class with weak reference
 */
class PyObjectPlusWithWeakReference : public PyObjectPlus
{
	PyObject*		weakreflist_;

public:
	PyObjectPlusWithWeakReference( PyTypePlus * pType ) :
		PyObjectPlus( pType ), weakreflist_( NULL ) { }
	virtual ~PyObjectPlusWithWeakReference()
		{ if (weakreflist_) PyObject_ClearWeakRefs( this ); }
	static Py_ssize_t weakListOffset()
		{ return ( (char*) & ((PyObjectPlusWithWeakReference*)256)->weakreflist_
			- (char*)(PyObject*)(PyObjectPlusWithWeakReference*)256 ); }
};


namespace PyTypeObjectUtil
{

inline Py_ssize_t weakListOffset( PyObjectPlusWithWeakReference* )
{
	return PyObjectPlusWithWeakReference::weakListOffset();
}

}


/**
 *	This macro allows a class that isn't a Python object to appear as if it
 *	were, for the purposes of defining attributes and methods with the macros
 *	in this file.
 *
 *	This can be useful for extensions to PyObjectPlus instances that are not
 *	themselves Python objects. The dummy methods defined here are never called,
 *	they are just to please the macros, particularly PY_TYPEOBJECT.
 *
 *	It needs only to be used in once in a base class and is not necessary in
 *	derived classes. It can be used with or without Py_HeaderSimple. Use it
 *	without if your base class is abstract.
 */
#define PY_FAKE_PYOBJECTPLUS_BASE_DECLARE()								\
	public:																\
		static destructor	_pyDestructor;								\
		static getattrfunc	_pyGetAttribute;							\
		static setattrfunc	_pySetAttribute;							\
		static reprfunc		_pyRepr;									\
																		\
		static void _tp_dealloc( PyObject * ) { }						\
																		\
		static PyObject * _tp_getattro( PyObject *, PyObject * )		\
			{ return NULL; }											\
																		\
		static int _tp_setattro( PyObject *, PyObject *, PyObject * )	\
			{ return -1; }												\
																		\
		static PyObject * _tp_repr( PyObject * )						\
			{ return NULL; }											\
																		\
		static PyObject * _pyNew( PyTypeObject * )						\
			{ return NULL; }											\
																		\
	private:															\


/**
 *	This macro is the counterpart definition to the
 *	PY_FAKE_PYOBJECTPLUS_BASE_DECLARE declaration.
 *
 *	@see PY_FAKE_PYOBJECTPLUS_BASE_DECLARE
 */
#define PY_FAKE_PYOBJECTPLUS_BASE( CLASS_NAME )							\
	destructor	CLASS_NAME::_pyDestructor = NULL;						\
	getattrfunc	CLASS_NAME::_pyGetAttribute = NULL;						\
	setattrfunc	CLASS_NAME::_pySetAttribute = NULL;						\
	reprfunc	CLASS_NAME::_pyRepr = NULL;								\


#endif // PYOBJECT_PLUS_HPP
