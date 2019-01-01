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
 * 	This file defines the PyVector class.
 *
 * 	@ingroup script
 */

#ifndef SCRIPT_MATH_HPP
#define SCRIPT_MATH_HPP


#include "pyobject_plus.hpp"
#include "script.hpp"
#include "math/matrix.hpp"
#include "math/planeeq.hpp"


// Reference this variable to link in the Math classes.
extern int Math_token;

/* These macros allow a VALID_ANGLE_ARG to appear in a PY_AUTO declaration.
 * They are defined here rather than globally in script.hpp because they use
 * the definition of "valid angle" from isValidAngle, which is statically
 * declared in src/lib/math/matrix.cpp, and so we don't want to leak that
 * definition to the rest of the world.
 *
 * See script.hpp for how the rest of the PY_AUTO stuff works.
 */


/// macros for arg count up to optional args
#define PYAUTO_OPTARGC_PARSE_VALID_ANGLE_ARG(T,R) (PYAUTO_OPTARGC_DO,ARG,U,R)
#define PYAUTO_OPTARGC_PARSE_OPTVALID_ANGLE_ARG(T,DEF,R) (PYAUTO_OPTARGC_DO,END,U,U)

#define PYAUTO_OPTARGC_PARSE_VALID_ANGLE_VECTOR3_ARG(T,R) (PYAUTO_OPTARGC_DO,ARG,U,R)
#define PYAUTO_OPTARGC_PARSE_OPTVALID_ANGLE_VECTOR3_ARG(T,DEF,R) (PYAUTO_OPTARGC_DO,END,U,U)

/// macros for arg count of all args
#define PYAUTO_ALLARGC_PARSE_VALID_ANGLE_ARG(T,R) (PYAUTO_ALLARGC_DO,ARG,U,R)
#define PYAUTO_ALLARGC_PARSE_OPTVALID_ANGLE_ARG(T,DEF,R) (PYAUTO_ALLARGC_DO,ARG,U,R)

#define PYAUTO_ALLARGC_PARSE_VALID_ANGLE_VECTOR3_ARG(T,R) (PYAUTO_ALLARGC_DO,ARG,U,R)
#define PYAUTO_ALLARGC_PARSE_OPTVALID_ANGLE_VECTOR3_ARG(T,DEF,R) (PYAUTO_ALLARGC_DO,ARG,U,R)

/// macros for the types of arguments
#define PYAUTO_ARGTYPES_PARSE_VALID_ANGLE_ARG(T,R) (PYAUTO_ARGTYPES_DO,ARG,T,R)
#define PYAUTO_ARGTYPES_PARSE_OPTVALID_ANGLE_ARG(T,DEF,R) (PYAUTO_ARGTYPES_DO,ARG,T,R)

#define PYAUTO_ARGTYPES_PARSE_VALID_ANGLE_VECTOR3_ARG(T,R) (PYAUTO_ARGTYPES_DO,ARG,T,R)
#define PYAUTO_ARGTYPES_PARSE_OPTVALID_ANGLE_VECTOR3_ARG(T,DEF,R) (PYAUTO_ARGTYPES_DO,ARG,T,R)

/// macros for writing the actual argument parsing code
#define PYAUTO_WRITE_PARSE_VALID_ANGLE_ARG(T,R) (PYAUTO_WRITE_DO,VALID_ANGLE_ARG,T,R)
#define PYAUTO_WRITE_PARSE_OPTVALID_ANGLE_ARG(T,DEF,R) (PYAUTO_WRITE_DO,OVALID_ANGLE_ARG,(T,DEF),R)

#define PYAUTO_WRITE_PARSE_VALID_ANGLE_VECTOR3_ARG(T,R) (PYAUTO_WRITE_DO,VALID_ANGLE_VECTOR3_ARG,T,R)
#define PYAUTO_WRITE_PARSE_OPTVALID_ANGLE_VECTOR3_ARG(T,DEF,R) (PYAUTO_WRITE_DO,OVALID_ANGLE_VECTOR3_ARG,(T,DEF),R)

// This definition is stolen from math/matrix.cpp
#define PYAUTO_WRITE_VALID_ANGLE(N,T)								\
	if (arg##N < -100.f || arg##N > 100.f)							\
	{																\
		PyErr_Format( PyExc_TypeError,								\
			"() argument " #N " must be a valid angle" );			\
		return NULL;												\
	}																\

#define PYAUTO_WRITE_VALID_ANGLE_VECTOR3(N,T,ELEMENT)				\
	if (arg##N[ELEMENT] < -100.f || arg##N[ELEMENT] > 100.f)		\
	{																\
		PyErr_Format( PyExc_TypeError,								\
			"() argument " #N " element " #ELEMENT " must be a "	\
				"valid angle" );									\
		return NULL;												\
	}																\

#define PYAUTO_WRITE_DO_VALID_ANGLE_ARG(N,A,R)						\
	PYAUTO_WRITE_STD(N,A)											\
	PYAUTO_WRITE_VALID_ANGLE(N,A)									\
	CHAIN_##N PYAUTO_WRITE_PARSE_##R								\

#define PYAUTO_WRITE_DO_OVALID_ANGLE_ARG(N,A,R)						\
	PYAUTO_WRITE_OPT(N,PAIR_1 A, PAIR_2 A)							\
	PYAUTO_WRITE_VALID_ANGLE(N,PAIR_1 A)							\
	CHAIN_##N PYAUTO_WRITE_PARSE_##R								\

#define PYAUTO_WRITE_DO_VALID_ANGLE_VECTOR3_ARG(N,A,R)				\
	PYAUTO_WRITE_STD(N,A)											\
	PYAUTO_WRITE_VALID_ANGLE_VECTOR3(N,A,0)							\
	PYAUTO_WRITE_VALID_ANGLE_VECTOR3(N,A,1)							\
	PYAUTO_WRITE_VALID_ANGLE_VECTOR3(N,A,2)							\
	CHAIN_##N PYAUTO_WRITE_PARSE_##R								\

#define PYAUTO_WRITE_DO_OVALID_ANGLE_VECTOR3_ARG(N,A,R)				\
	PYAUTO_WRITE_OPT(N,PAIR_1 A, PAIR_2 A)							\
	PYAUTO_WRITE_VALID_ANGLE_VECTOR3(N,PAIR_1 A,0)					\
	PYAUTO_WRITE_VALID_ANGLE_VECTOR3(N,PAIR_1 A,1)					\
	PYAUTO_WRITE_VALID_ANGLE_VECTOR3(N,PAIR_1 A,2)					\
	CHAIN_##N PYAUTO_WRITE_PARSE_##R								\

/// macros for calling the function with the parsed arguments
#define PYAUTO_CALLFN_PARSE_VALID_ANGLE_ARG(T,R) (PYAUTO_CALLFN_DO,ARG,U,R)
#define PYAUTO_CALLFN_PARSE_OPTVALID_ANGLE_ARG(T,DEF,R) (PYAUTO_CALLFN_DO,ARG,U,R)

#define PYAUTO_CALLFN_PARSE_VALID_ANGLE_VECTOR3_ARG(T,R) (PYAUTO_CALLFN_DO,ARG,U,R)
#define PYAUTO_CALLFN_PARSE_OPTVALID_ANGLE_VECTOR3_ARG(T,DEF,R) (PYAUTO_CALLFN_DO,ARG,U,R)

#define PYAUTO_CALLFN_PARSE1_VALID_ANGLE_ARG(T,R) (PYAUTO_CALLFN_DO,ARG1,U,R)
#define PYAUTO_CALLFN_PARSE1_OPTVALID_ANGLE_ARG(T,DEF,R) (PYAUTO_CALLFN_DO,ARG1,U,R)

#define PYAUTO_CALLFN_PARSE1_VALID_ANGLE_VECTOR3_ARG(T,R) (PYAUTO_CALLFN_DO,ARG1,U,R)
#define PYAUTO_CALLFN_PARSE1_OPTVALID_ANGLE_VECTOR3_ARG(T,DEF,R) (PYAUTO_CALLFN_DO,ARG1,U,R)


/**
 *	This helper class has virtual functions to provide a matrix. This
 *	assists many classes in getting out matrices from a variety of
 *	different objects.
 */
class MatrixProvider : public PyObjectPlus
{
	Py_Header( MatrixProvider, PyObjectPlus )

public:
	MatrixProvider( bool autoTick, PyTypePlus * pType );
	virtual ~MatrixProvider();

	virtual PyObject * pyGetAttribute( const char * attr )
		{ return PyObjectPlus::pyGetAttribute( attr ); }
	virtual int pySetAttribute( const char * attr, PyObject * value )
		{ return PyObjectPlus::pySetAttribute( attr, value ); }
	virtual int pyDelAttribute( const char * attr )
		{ return PyObjectPlus::pyDelAttribute( attr ); }

	virtual void tick( float /*dTime*/ ) { }
	virtual void matrix( Matrix & m ) const = 0;

	virtual void getYawPitch( float& yaw, float& pitch) const
	{
		Matrix m;
		this->matrix(m);
		yaw = m.yaw();
		pitch = m.pitch();
	}

private:
	bool autoTick_;
};

typedef SmartPointer<MatrixProvider> MatrixProviderPtr;

PY_SCRIPT_CONVERTERS_DECLARE( MatrixProvider )

/**
 *	This class allows scripts to create and manipulate their own matrices
 */
class PyMatrix : public MatrixProvider, public Matrix
{
	Py_Header( PyMatrix, MatrixProvider )


public:
	PyMatrix( PyTypePlus * pType = &s_type_ ) :
		MatrixProvider( false, pType ), Matrix( Matrix::identity )	{ }
	void set( const Matrix & m )			{ *static_cast<Matrix*>(this) = m; }
	virtual void matrix( Matrix & m ) const	{ m = *this; }

	PyObject *	pyGetAttribute( const char * attr );
	int			pySetAttribute( const char * attr, PyObject * value );

	// Need to overload this so that we can set the Python exception state.
	bool invert()
	{
		if (!this->Matrix::invert())
		{
			PyErr_SetString( PyExc_ValueError, "Matrix could not be inverted" );
			return false;
		}

		return true;
	}

	void set( MatrixProviderPtr mpp )
		{ Matrix m; mpp->matrix( m ); this->set( m ); }
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, set, NZARG( MatrixProviderPtr, END ),
		"This method sets the matrix from a MatrixProvider." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, setZero, END,
		"This method sets the matrix to all zeroes." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, setIdentity, END,
		"This method sets the matrix to the identity matrix." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, setScale, ARG( Vector3, END ),
		"This method sets the matrix to a scaling matrix." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, setTranslate, ARG( Vector3, END ),
		"This method sets the matrix to a translation matrix." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, setRotateX, VALID_ANGLE_ARG( float, END ),
		"This method sets the matrix to a rotation matrix around the X axis." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, setRotateY, VALID_ANGLE_ARG( float, END ),
		"This method sets the matrix to a rotation matrix around the Y axis." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, setRotateZ, VALID_ANGLE_ARG( float, END ),
		"This method sets the matrix to a rotation matrix around the Z axis." )
	void setRotateYPR( const Vector3 & ypr )
		{ this->setRotate( ypr[0], ypr[1], ypr[2] ); }
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, setRotateYPR, VALID_ANGLE_VECTOR3_ARG( Vector3, END ),
		"This method sets the matrix to a rotation matrix given yaw, pitch and roll." )
	void preMultiply( MatrixProviderPtr mpp )
		{ Matrix m; mpp->matrix( m ); this->Matrix::preMultiply( m ); }
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, preMultiply,
		NZARG( MatrixProviderPtr, END ),
		"This method multiplies the existing matrix by the passed in one."  )
	void postMultiply( MatrixProviderPtr mpp )
		{ Matrix m; mpp->matrix( m ); this->Matrix::postMultiply( m ); }
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, postMultiply,
		NZARG( MatrixProviderPtr, END ),
		"This method multiplies the passed in matrix by the existing one." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETOK, invert, END,
		"This method sets the matrix to the inverse of itself." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, lookAt, ARG( Vector3,
		ARG( Vector3, ARG( Vector3, END ) ) ),
		"This method sets the matrix to a look-at matrix, useful as a view matrix." )

	void setElement( int col, int row, float value) { (*this)[col&3][row&3] = value; }
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, setElement, ARG( uint, ARG( uint, ARG( float, END ) ) ),
		"This method sets the element at column, row to the third parameter." )

	float get( int col, int row ) const { return (*this)[col&3][row&3]; }
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETDATA, get, ARG( uint, ARG( uint, END ) ),
		"This method returns the element at column, row." )
	PY_RO_ATTRIBUTE_DECLARE( getDeterminant(), determinant )

	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETDATA, applyPoint, ARG( Vector3, END ),
		"This method applies the matrix to the specified point.  The point "
		"should be a Vector3, but will be treated as if it were a Vector4 "
		"with the fourth component being 1.0." )
	Vector4 applyV4Point( const Vector4 & p )
		{ Vector4 ret; this->applyPoint( ret, p ); return ret; }
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETDATA, applyV4Point, ARG( Vector4, END ),
		"This method transforms the passed in Vector4 by the matrix." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETDATA, applyVector, ARG( Vector3, END ),
		"This method applies the matrix to the specified vector.  The vector "
		"is a Vector3, and the last row of the matrix is ignored in the "
		"calculations.")

	const Vector3 & applyToAxis( uint axis )
		{ return this->applyToUnitAxisVector( axis & 3 ); }
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETDATA, applyToAxis, ARG( uint, END ),
		"This method returns the axis represented by column n in the matrix." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETDATA, applyToOrigin, END,
		"This method returns the translation of the matrix." )

	PY_RO_ATTRIBUTE_DECLARE( isMirrored(), isMirrored )

	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, orthogonalProjection,
		ARG( float, ARG( float, ARG( float, ARG( float, END ) ) ) ),
		"This method sets the matrix to an orthogonal projection matrix." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, perspectiveProjection,
		ARG( float, ARG( float, ARG( float, ARG( float, END ) ) ) ),
		"This method sets the matrix to a perspective projection matrix." )

	const Vector3 & translation() const
		{ return this->Matrix::applyToOrigin(); }
	void translation( const Vector3 & v )
		{ this->Matrix::translation( v ); }
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Vector3, translation, translation )

	PY_RO_ATTRIBUTE_DECLARE( yaw(), yaw )
	PY_RO_ATTRIBUTE_DECLARE( pitch(), pitch )
	PY_RO_ATTRIBUTE_DECLARE( roll(), roll )

	static PyObject * _pyCall( PyObject * self, PyObject * args, PyObject * kw)
		{ return _py_get( self, args, kw ); }

	PY_METHOD_DECLARE( py___getstate__ )
	PY_METHOD_DECLARE( py___setstate__ )

	PY_FACTORY_DECLARE()
};


/**
 *	This class implements a Vector class for use in Python
 *
 * 	@ingroup script
 */
template <class V> class PyVector : public PyObjectPlus
{
	Py_Header( PyVector< V >, PyObjectPlus )

public:
	typedef V Member;

	PyVector( bool isReadOnly, PyTypePlus * pType ) :
		PyObjectPlus( pType ), isReadOnly_( isReadOnly ) {}
	virtual ~PyVector() { };

	virtual V getVector() const = 0;
	virtual bool setVector( const V & v ) = 0;

	bool isReadOnly() const		{ return isReadOnly_; }
	virtual bool isReference() const	{ return false; }

	PyObject * pyGet_x();
	int pySet_x( PyObject * value );

	PyObject * pyGet_y();
	int pySet_y( PyObject * value );

	PyObject * pyGet_z();
	int pySet_z( PyObject * value );

	PyObject * pyGet_w();
	int pySet_w( PyObject * value );

// 	PY_FACTORY_DECLARE()
	PyObject * pyNew( PyObject * args );

	// PyObjectPlus overrides
	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );

	PY_UNARY_FUNC_METHOD( pyStr )

	// Used for as_number
	// PY_BINARY_FUNC_METHOD( py_add )
	// PY_BINARY_FUNC_METHOD( py_subtract )
	// PY_BINARY_FUNC_METHOD( py_multiply )
	static PyObject * _py_add( PyObject * a, PyObject * b );
	static PyObject * _py_subtract( PyObject * a, PyObject * b );
	static PyObject * _py_multiply( PyObject * a, PyObject * b );
	static PyObject * _py_divide( PyObject * a, PyObject * b );

	PY_UNARY_FUNC_METHOD( py_negative )
	PY_UNARY_FUNC_METHOD( py_positive )

	PY_INQUIRY_METHOD( py_nonzero )

	PY_BINARY_FUNC_METHOD( py_inplace_add )
	PY_BINARY_FUNC_METHOD( py_inplace_subtract )
	PY_BINARY_FUNC_METHOD( py_inplace_multiply )
	PY_BINARY_FUNC_METHOD( py_inplace_divide )

	// Used for as_sequence
	PY_SIZE_INQUIRY_METHOD( py_sq_length )
	PY_INTARG_FUNC_METHOD( py_sq_item )
	PY_INTINTARG_FUNC_METHOD( py_sq_slice )
	PY_INTOBJARG_PROC_METHOD( py_sq_ass_item )

	// Script methods
	PY_METHOD_DECLARE_WITH_DOC( py_set, 
		"This method sets the Vector to the given vector value." )
	PY_METHOD_DECLARE_WITH_DOC( py_scale,
		"This method scales the Vector by the given scalar." )
	PY_METHOD_DECLARE_WITH_DOC( py_dot,
		"This method performs a dot-product between the Vector and the given "
		"vector value." )
	PY_METHOD_DECLARE_WITH_DOC( py_normalise,
		"This method normalises the Vector." )

	PY_METHOD_DECLARE_WITH_DOC( py_tuple,
		"This method returns a tuple representation of the vector." )
	PY_METHOD_DECLARE_WITH_DOC( py_list,
		"This method returns a list representation of the vector." )

	PY_METHOD_DECLARE_WITH_DOC( py_cross2D,
		"This method performs a 2D cross-product of the vector, as long as it "
		"is a Vector2 or Vector3." )

	PY_METHOD_DECLARE_WITH_DOC( py_distTo,
		"This method returns the distance to the input vector." )
	PY_METHOD_DECLARE_WITH_DOC( py_distSqrTo,
		"This method returns the distance squared to the input vector." )

	PY_METHOD_DECLARE_WITH_DOC( py_flatDistTo,
		"This method returns the distance between the points on the XZ plane. "
		"It is only implemented for Vector3." )
	PY_METHOD_DECLARE_WITH_DOC( py_flatDistSqrTo,
		"This method returns the distance squared between the points on "
		"the XZ plane. It is only implemented for Vector3." )
	PY_METHOD_DECLARE_WITH_DOC( py_setPitchYaw,
		"This method sets the vector to be a unit vector pointing to the "
		"given pitch and yaw. It is only implemented for Vector3." )

	// PY_METHOD_DECLARE( py___reduce__ )
	PY_METHOD_DECLARE( py___getstate__ )
	PY_METHOD_DECLARE( py___setstate__ )

	PY_RO_ATTRIBUTE_DECLARE( this->getVector().length(), length )
	PY_RO_ATTRIBUTE_DECLARE( this->getVector().lengthSquared(), lengthSquared )

	PY_RO_ATTRIBUTE_DECLARE( this->isReadOnly(), isReadOnly )
	PY_RO_ATTRIBUTE_DECLARE( this->isReference(), isReference )

	PyObject * pyGet_yaw();
	PyObject * pyGet_pitch();
	PY_RO_ATTRIBUTE_SET( yaw )
	PY_RO_ATTRIBUTE_SET( pitch )

	static PyObject * _pyNew( PyTypeObject * pType,
			PyObject * args, PyObject * kwargs );

private:
	PyVector( const PyVector & toCopy );
	PyVector & operator=( const PyVector & toCopy );

	bool safeSetVector( const V & v );

	bool isReadOnly_;

	static const int NUMELTS;
};


/**
 *	This class implements a PyVector that has the vector attribute as a member.
 */
template <class V>
class PyVectorCopy : public PyVector< V >
{
public:
	PyVectorCopy( bool isReadOnly = false,
			PyTypePlus * pType = &PyVector<V>::s_type_ ) :
		PyVector<V>( isReadOnly, pType ), v_( V::zero() ) {}

	PyVectorCopy( const V & v, bool isReadOnly = false,
			PyTypePlus * pType = &PyVector<V>::s_type_ ) :
		PyVector<V>( isReadOnly, pType ), v_( v ) {}

	virtual V getVector() const
	{
		return v_;
	}

	virtual bool setVector( const V & v )
	{
		v_ = v;
		return true;
	}

private:
	V v_;
};


/**
 *	This class implements a PyVector that takes its value from a member of
 *	another Python object. This is useful for exposing a Vector2, Vector3 or
 *	Vector4 member C++ class and allowing the underlying member to be modified
 *	when the Python Vector is modified.
 */
template <class V>
class PyVectorRef : public PyVector< V >
{
public:
	PyVectorRef( PyObject * pOwner, V * pV, bool isReadOnly = false,
			PyTypePlus * pType = &PyVector<V>::s_type_ ) :
		PyVector<V>( isReadOnly, pType ), pOwner_( pOwner ), pV_( pV ) {}

	virtual V getVector() const
	{
		return *pV_;
	}

	virtual bool setVector( const V & v )
	{
		*pV_ = v;

		return true;
	}

	virtual bool isReference() const	{ return true; }

private:
	PyObjectPtr pOwner_;
	V * pV_;
};


/**
 *	This class extends PyVector4 is add colour attributes that are aliases for
 *	the PyVector4 attributes.
 */
class PyColour : public PyVector< Vector4 >
{
	Py_Header( PyColour, PyVector< Vector4 > )

	virtual PyObject * pyGetAttribute( const char * attr );
	virtual int pySetAttribute( const char * attr, PyObject * value );

public:
	PyColour( bool isReadOnly = false, PyTypePlus * pType = &s_type_ ) :
		PyVector< Vector4 >( isReadOnly, pType ) {}
};


//-----------------------------------------------------------------------------
// Section: Vector4Provider
//-----------------------------------------------------------------------------

/**
 *	This abstract class provides a Vector4
 */
class Vector4Provider : public PyObjectPlus
{
	Py_Header( Vector4Provider, PyObjectPlus )
public:
	Vector4Provider( bool autoTick, PyTypePlus * pType );
	virtual ~Vector4Provider();

	virtual PyObject * pyGetAttribute( const char * attr );
	virtual int pySetAttribute( const char * attr, PyObject * value );

	PyObject * pyGet_value();
	PY_RO_ATTRIBUTE_SET( value );

	virtual void tick( float /*dTime*/ ) { }
	virtual void output( Vector4 & val ) = 0;

	static PyObjectPtr coerce( PyObject * pObject );

private:
	bool autoTick_;
};

typedef SmartPointer<Vector4Provider> Vector4ProviderPtr;
typedef std::pair<float,Vector4ProviderPtr> Vector4Animation_Keyframe;
namespace Script
{
	int setData( PyObject * pObject, Vector4Animation_Keyframe & rpVal,
		const char * varName = "" );
	PyObject * getData( const Vector4Animation_Keyframe & pVal );
}

PY_SCRIPT_CONVERTERS_DECLARE( Vector4Provider )


/**
 *	This is the basic concrete Vector4 provider.
 */
class Vector4Basic : public Vector4Provider
{
	Py_Header( Vector4Basic, Vector4Provider )
public:
	Vector4Basic( const Vector4 & val = Vector4(0,0,0,0), PyTypePlus * pType = &s_type_ );

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ATTRIBUTE_DECLARE( value_, value )

	virtual void output( Vector4 & val ) { val = value_; }
	void set( const Vector4 & val )		{ value_ = val; }
	Vector4* pValue()					{ return &value_; }
private:
	Vector4 value_;
};

typedef SmartPointer<Vector4Basic> Vector4BasicPtr;
PY_SCRIPT_CONVERTERS_DECLARE( Vector4Basic )


/**
 *	This class allows scripts to create and manipulate a plane
 */
class PyPlane : public PyObjectPlus, PlaneEq
{
	Py_Header( PyPlane, PyObjectPlus)

public:
	PyPlane( PyTypePlus * pType = &s_type_ ) :
		PyObjectPlus( pType )	{ }

	PyObject *	pyGetAttribute( const char * attr );
	int			pySetAttribute( const char * attr, PyObject * value );

	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, init, ARG(Vector3, ARG(Vector3, ARG(Vector3, END ))),
		"This method initialises the Plane")

	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETDATA, intersectRay, ARG( Vector3, ARG(Vector3, END )),
		"This method finds an intersection between a ray from source to dest and the plane")

	PY_FACTORY_DECLARE()
};


/**
 *	This class holds onto all Vector4/Matrix Providers
 *	and ticks them.
 */
class ProviderStore
{
public:
	static void tick( float dTime );
	static void add( MatrixProvider* );
	static void add( Vector4Provider* );
	static void del( MatrixProvider* );
	static void del( Vector4Provider* );
private:
	typedef std::vector< Vector4Provider*> Vector4Providers;
	static Vector4Providers v4s_;
	typedef std::vector< MatrixProvider* > MatrixProviders;
	static MatrixProviders ms_;
};


#endif // SCRIPT_MATH_HPP
