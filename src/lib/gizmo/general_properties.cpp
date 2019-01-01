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
#include "general_properties.hpp"
#include "resmgr/bwresource.hpp"
#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT2( "gizmo", 0 )


// -----------------------------------------------------------------------------
// Section: GenPositionProperty
// -----------------------------------------------------------------------------



/**
 *	Constructor.
 */
GenPositionProperty::GenPositionProperty( const std::string & name,
		MatrixProxyPtr pMatrix, float size ) :
	GeneralProperty( name ),
	pMatrix_( pMatrix ),
	size_( size )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}

/**
 *	Get python version of property
 */
PyObject * EDCALL GenPositionProperty::pyGet()
{
	BW_GUARD;

	Matrix m;
	pMatrix_->getMatrix( m );
	return Script::getData( m.applyToOrigin() );
}

/**
 *	Set property from python version
 */
int EDCALL GenPositionProperty::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*=true*/ )
{
	BW_GUARD;

	Vector3 v;
	std::string errStr( "GeneralEditor." );
	errStr += name_;

	int ret = Script::setData( value, v, errStr.c_str() );
	if (ret == 0)
	{
		pMatrix_->recordState();

		Matrix invCtx;
		pMatrix_->getMatrixContextInverse( invCtx );

		Matrix curPose;
		pMatrix_->getMatrix( curPose, transient );

		curPose.translation( invCtx.applyPoint( v ) );

		pMatrix_->setMatrix( curPose );

		pMatrix_->commitState( false, addBarrier );

		// could set python exception if op failed. either change
		// commitState to return a bool (easiest) or get matrix
		// again and compare to what we set.
	}
	return ret;
}


GENPROPERTY_VIEW_FACTORY( GenPositionProperty )



/**
 *	Constructor.
 */
GenRotationProperty::GenRotationProperty( const std::string & name,
		MatrixProxyPtr pMatrix, bool allowGizmo ) :
	GeneralProperty( name ),
	pMatrix_( pMatrix ),
	allowGizmo_( allowGizmo )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Get python version of property
 */
PyObject * EDCALL GenRotationProperty::pyGet()
{
	BW_GUARD;

	Matrix m;
	pMatrix_->getMatrix( m );
	Vector3 rot;
	rot.x = m.yaw();
	rot.y = m.pitch();
	rot.z = m.roll();
	return Script::getData( rot );
}

/**
 *	Set property from python version
 */
int EDCALL GenRotationProperty::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*=true*/ )
{
	BW_GUARD;

	Vector3 v;
	std::string errStr( "GeneralEditor." );
	errStr += name_;

	int ret = Script::setData( value, v, errStr.c_str() );
	if (ret == 0)
	{
		pMatrix_->recordState();

		Matrix curPose;
		pMatrix_->getMatrix( curPose, transient );

		//current pose is a world rotation
		Vector3 currRot;
		currRot.x = curPose.yaw();
		currRot.y = curPose.pitch();
		currRot.z = curPose.roll();

		//find the required difference in rotation
		Vector3 diff( v.x - currRot.x, v.y - currRot.y, v.z - currRot.z );
		
		//make a rotation matrix
		Matrix dRot;
		dRot.setRotate( diff.x, diff.y, diff.z );		

		curPose.preMultiply( dRot );
		pMatrix_->setMatrix( curPose );

		pMatrix_->commitState( false, addBarrier );

		// could set python exception if op failed. either change
		// commitState to return a bool (easiest) or get matrix
		// again and compare to what we set.
	}
	return ret;
}



GENPROPERTY_VIEW_FACTORY( GenRotationProperty )


/**
 *	Constructor.
 */
GenScaleProperty::GenScaleProperty( const std::string & name,
		MatrixProxyPtr pMatrix, bool allowNonUniformScale /*= true*/,
		bool allowUniformScale /*= true*/ ) :
	GeneralProperty( name ),
	pMatrix_( pMatrix ),
	allowNonUniformScale_( allowNonUniformScale ),
	allowUniformScale_( allowUniformScale )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}

/**
 *	Get python version of property
 */
PyObject * EDCALL GenScaleProperty::pyGet()
{
	BW_GUARD;

	Matrix m;
	pMatrix_->getMatrix( m );
	Vector3 scale;
	scale.x = m.applyToUnitAxisVector(0).length();
	scale.y = m.applyToUnitAxisVector(1).length();
	scale.z = m.applyToUnitAxisVector(2).length();
	return Script::getData( scale );
}

/**
 *	Set property from python version
 */
int EDCALL GenScaleProperty::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*=true*/ )
{
	BW_GUARD;

	Vector3 v;
	std::string errStr( "GeneralEditor." );
	errStr += name_;

	int ret = Script::setData( value, v, errStr.c_str() );
	if (ret == 0)
	{
		if ( ::almostZero(v.x) ||
			::almostZero(v.y) ||
			::almostZero(v.z) )
		{
			PyErr_SetString( PyExc_TypeError, "GenScaleProperty::pySet() "
				"one of the scale factors was zero." );
			return 1;
		}		

		Matrix curPose;
		pMatrix_->getMatrix( curPose, transient );

		Vector3 currScale;
		currScale.x = curPose.applyToUnitAxisVector(0).length();
		currScale.y = curPose.applyToUnitAxisVector(1).length();
		currScale.z = curPose.applyToUnitAxisVector(2).length();

		if ( ::almostZero(currScale.x) ||
			::almostZero(currScale.y) ||
			::almostZero(currScale.z) )
		{
			PyErr_SetString( PyExc_TypeError, "GenScaleProperty::pySet() "
				"the scale factor of one axis of the existing pose was zero.");
			return 1;
		}

		pMatrix_->recordState();

		Matrix mScale;
		mScale.setScale( v.x / currScale.x, v.y / currScale.y, v.z / currScale.z );		

		curPose.preMultiply( mScale );

		pMatrix_->setMatrix( curPose );

		pMatrix_->commitState( false, addBarrier );

		// could set python exception if op failed. either change
		// commitState to return a bool (easiest) or get matrix
		// again and compare to what we set.
	}
	return ret;
}



GENPROPERTY_VIEW_FACTORY( GenScaleProperty )




/**
 *	Constructor
 */
StaticTextProperty::StaticTextProperty( const std::string & name,
		StringProxyPtr text, bool isDate ) :
	GeneralROProperty( name ),
	text_( text ),
	isDate_( isDate )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


const ValueType & StaticTextProperty::valueType() const
{
	if (isDate_)
	{
		RETURN_VALUETYPE( DATE_STRING );
	}
	else
	{
		RETURN_VALUETYPE( STRING );
	}
}


/**
 *	Python get method
 */
PyObject * EDCALL StaticTextProperty::pyGet()
{
	BW_GUARD;

	if ( text_ )
		return PyString_FromString( text_->get().c_str() );
	return NULL;
}


GENPROPERTY_VIEW_FACTORY( StaticTextProperty )



/**
 *	Constructor
 */
TextLabelProperty::TextLabelProperty( const std::string & name, 
		void * userObject, bool highlight ) :
	GeneralROProperty( name ),
	userObject_( userObject ),
	highlight_( highlight )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


GENPROPERTY_VIEW_FACTORY( TextLabelProperty )



/**
 *	Constructor.
 */
GenFloatProperty::GenFloatProperty( const std::string & name,
		FloatProxyPtr pFloat ) :
	GeneralProperty( name ),
	pFloat_( pFloat )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Get python version of property
 */
PyObject * EDCALL GenFloatProperty::pyGet()
{
	BW_GUARD;

	return Script::getData( pFloat_->get() );
}

/**
 *	Set property from python version
 */
int EDCALL GenFloatProperty::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*=true*/ )
{
	BW_GUARD;

	float f;
	std::string errStr( "GeneralEditor." );
	errStr += name_;
	int ret = Script::setData( value, f, errStr.c_str() );
	if (ret == 0)
	{
		pFloat_->set( f, transient, addBarrier );
	}
	return ret;
}


GENPROPERTY_VIEW_FACTORY( GenFloatProperty )





/**
 *	Constructor.
 */
GenRadiusProperty::GenRadiusProperty( const std::string & name,
		FloatProxyPtr pFloat,
		MatrixProxyPtr pCenter,
		uint32 widgetColour,
		float widgetRadius  ) :
	GenFloatProperty( name, pFloat ),
	pCenter_( pCenter ),
	widgetColour_( widgetColour ),
	widgetRadius_( widgetRadius )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


GENPROPERTY_VIEW_FACTORY( GenRadiusProperty )





/**
 *	Constructor.
 */
ColourProperty::ColourProperty( const std::string & name,
		ColourProxyPtr pColour ) :
	GeneralProperty( name ),
	pColour_( pColour ),
	pVector4_( NULL )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}

ColourProperty::ColourProperty( const std::string & name,
		Vector4ProxyPtr pVector4 ) :
	GeneralProperty( name ),
	pColour_( NULL ),
	pVector4_( pVector4 )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Get python version of property
 */
PyObject * EDCALL ColourProperty::pyGet()
{
	BW_GUARD;

	if (pColour_)
	{
		return Script::getData(
			Vector4( static_cast<float*>( pColour_->get() ) ) * 255.f );
	}
	else
	{
		return Script::getData(
			Vector4( static_cast<float*>( pVector4_->get() ) ) * 255.f );
	}
}

/**
 *	Set property from python version
 */
int EDCALL ColourProperty::pySet( PyObject * value, bool transient /*= false*/,
												bool addBarrier /*= true*/ )
{
	BW_GUARD;

	Vector4 v;
	std::string errStr( "GeneralEditor." );
	errStr += name_;
	int ret = Script::setData( value, v, errStr.c_str() );
	if (ret == 0)
	{
		if (pColour_)
		{
			pColour_->set( Moo::Colour( v ) / 255.f, transient, addBarrier );
		}
		else
		{
			pVector4_->set( v / 255.f, transient, addBarrier );
		}
	}
	return ret;
}


GENPROPERTY_VIEW_FACTORY( ColourProperty )



/**
 *	Constructor.
 */
ColourScalarProperty::ColourScalarProperty( const std::string & name,
		const std::string& scalarName,
		ColourScalarProxyPtr pColour ) :
	GeneralProperty( name ),
	scalarName_( scalarName ),
	pColour_( pColour )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Get python version of property
 */
PyObject * EDCALL ColourScalarProperty::pyGet()
{
	BW_GUARD;

	Vector4 col( pColour_->get() );
	Vector4 ret( col.x * 255.f, col.y * 255.f, col.z * 255.f, col.w );
	return Script::getData( ret );
}


/**
 *	Set property from python version
 */
int EDCALL ColourScalarProperty::pySet( PyObject * value, bool transient /* = false */,
												bool addBarrier /*= true*/  )
{
	BW_GUARD;

	Vector4 v;
	std::string errStr( "GeneralEditor." );
	errStr += name_;
	int ret = Script::setData( value, v, errStr.c_str() );
	if (ret == 0)
	{
		Vector4 col( v );
		col.x /= 255.f;
		col.y /= 255.f;
		col.z /= 255.f;
		pColour_->set( col, transient, addBarrier );		
	}
	return ret;
}


GENPROPERTY_VIEW_FACTORY( ColourScalarProperty )


/**
 *	Constructor.
 */
Vector4Property::Vector4Property( const std::string & name,
		Vector4ProxyPtr pVector4 ) :
	GeneralProperty( name ),
	pVector4_( pVector4 )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Get python version of property
 */
PyObject * EDCALL Vector4Property::pyGet()
{
	BW_GUARD;

	return Script::getData(
		Vector4( static_cast<float*>( pVector4_->get() ) ) );
}

/**
 *	Set property from python version
 */
int EDCALL Vector4Property::pySet( PyObject * value,
					bool transient /*= false*/, bool addBarrier /*= true*/ )
{
	BW_GUARD;

	Vector4 v;
	std::string errStr( "GeneralEditor." );
	errStr += name_;
	int ret = Script::setData( value, v, errStr.c_str() );
	if (ret == 0)
	{
		pVector4_->set( v, transient, addBarrier );
	}
	return ret;
}


GENPROPERTY_VIEW_FACTORY( Vector4Property )


/**
 *	Constructor.
 */
Vector3Property::Vector3Property( const std::string & name,
		Vector3ProxyPtr pVector3 ) :
	GeneralProperty( name ),
	pVector3_( pVector3 )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Get python version of property
 */
PyObject * EDCALL Vector3Property::pyGet()
{
	BW_GUARD;

	return Script::getData(
		Vector3( static_cast<float*>( pVector3_->get() ) ) );
}

/**
 *	Set property from python version
 */
int EDCALL Vector3Property::pySet( PyObject * value,
					bool transient /*= false*/, bool addBarrier /*= true*/ )
{
	BW_GUARD;

	Vector3 v;
	std::string errStr( "GeneralEditor." );
	errStr += name_;
	int ret = Script::setData( value, v, errStr.c_str() );
	if (ret == 0)
	{
		pVector3_->set( v, transient, addBarrier );
	}
	return ret;
}


GENPROPERTY_VIEW_FACTORY( Vector3Property )


/**
 *	Constructor.
 */
Vector2Property::Vector2Property( const std::string & name,
		Vector2ProxyPtr pVector2 ) :
	GeneralProperty( name ),
	pVector2_( pVector2 )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Get python version of property
 */
PyObject * EDCALL Vector2Property::pyGet()
{
	BW_GUARD;

	return Script::getData(
		Vector2( static_cast<float*>( pVector2_->get() ) ) );
}

/**
 *	Set property from python version
 */
int EDCALL Vector2Property::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*=true*/ )
{
	BW_GUARD;

	Vector2 v;
	std::string errStr( "GeneralEditor." );
	errStr += name_;
	int ret = Script::setData( value, v, errStr.c_str() );
	if (ret == 0)
	{
		pVector2_->set( v, transient, addBarrier );
	}
	return ret;
}


GENPROPERTY_VIEW_FACTORY( Vector2Property )



/**
 *	Constructor.
 */
GenMatrixProperty::GenMatrixProperty( const std::string & name,
		MatrixProxyPtr pMatrix ) :
	GeneralProperty( name ),
	pMatrix_( pMatrix )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Get python version of property
 */
PyObject * EDCALL GenMatrixProperty::pyGet()
{
	BW_GUARD;

	Matrix m;
	pMatrix_->getMatrix(m);
	return Script::getData(m);
}

/**
 *	Set property from python version
 */
int EDCALL GenMatrixProperty::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*=true*/ )
{
	BW_GUARD;

	Matrix m;
	std::string errStr( "GeneralEditor." );
	errStr += name_;
	int ret = Script::setData( value, m, errStr.c_str() );
	if (ret == 0)
	{
		pMatrix_->setMatrix(m);
	}
	return ret;
}


GENPROPERTY_VIEW_FACTORY( GenMatrixProperty )



/**
 *	Constructor.
 */
AngleProperty::AngleProperty( const std::string & name,
		FloatProxyPtr pFloat,
		MatrixProxyPtr pCenter ) :
	GenFloatProperty( name, pFloat ),
	pCenter_( pCenter )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


GENPROPERTY_VIEW_FACTORY( AngleProperty )




/**
 *	Constructor
 */
TextProperty::TextProperty( const std::string & name,
		StringProxyPtr text ) :
	GeneralProperty( name ),
	text_( text ),
	fileFilter_(L""),
	defaultDir_(L""),
	canTextureFeed_( false )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}

/**
 *	Python get method
 */
PyObject * EDCALL TextProperty::pyGet()
{
	BW_GUARD;

	return Script::getData( text_->get() );
}

/**
 *	Python set method
 */
int EDCALL TextProperty::pySet( PyObject * value, bool transient /* = false */,
												bool addBarrier /*= true*/ )
{
	BW_GUARD;

	std::string s;
	std::string errStr( "GeneralEditor." );
	errStr += name_;
	int ret = Script::setData( value, s, errStr.c_str() );
	if (ret == 0)
	{
		text_->set( s, transient, addBarrier );
	}
	return ret;
}

GENPROPERTY_VIEW_FACTORY( TextProperty )




/**
 *	Constructor
 */
IDProperty::IDProperty( const std::string & name,
		StringProxyPtr text ) :
	GeneralProperty( name ),
	text_( text )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}

/**
 *	Python get method
 */
PyObject * EDCALL IDProperty::pyGet()
{
	BW_GUARD;

	return Script::getData( text_->get() );
}

/**
 *	Python set method
 */
int EDCALL IDProperty::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*=true*/ )
{
	BW_GUARD;

	std::string s;
	std::string errStr( "GeneralEditor." );
	errStr += name_;
	int ret = Script::setData( value, s, errStr.c_str() );
	if (ret == 0)
	{
		text_->set( s, transient, addBarrier );
	}
	return ret;
}

GENPROPERTY_VIEW_FACTORY( IDProperty )



/**
 *	Constructor
 */
GroupProperty::GroupProperty( const std::string & name ) :
	GeneralProperty( name )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}

#if 0
/**
 *	Python get method
 */
PyObject * GroupProperty::pyGet()
{
	return NULL;
}

/**
 *	Python set method
 */
int GroupProperty::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*=true*/ )
{
	return 0;
}
#endif //0

GENPROPERTY_VIEW_FACTORY( GroupProperty )



/**
 *	Constructor
 */
ListTextProperty::ListTextProperty( const std::string & name, StringProxyPtr text, 
				 const std::vector<std::wstring> & possibleValues ) :
	GeneralProperty( name ),
	text_( text ),
	possibleValues_( possibleValues )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}

/**
 *	Python get method
 */
PyObject * EDCALL ListTextProperty::pyGet()
{
	BW_GUARD;

	return Script::getData( text_->get() );
}

/**
 *	Python set method
 */
int EDCALL ListTextProperty::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*= true*/ )
{
	BW_GUARD;

	std::string s;
	std::string errStr( "GeneralEditor." );
	errStr += name_;
	int ret = Script::setData( value, s, errStr.c_str() );
	if (ret == 0)
	{
		text_->set( s, transient, addBarrier );
	}
	return ret;
}

GENPROPERTY_VIEW_FACTORY( ListTextProperty )



/**
 *	Constructor.
 */
ResourceProperty::ResourceProperty( const std::string & name,
		StringProxyPtr pString, const std::string & extension,
		const Checker & checker ) :
	GeneralProperty( name ),
	pString_( pString ),
	extension_( extension ),
	checker_( checker )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Get python version of property
 */
PyObject * EDCALL ResourceProperty::pyGet()
{
	BW_GUARD;

	return Script::getData( pString_->get() );
}

/**
 *	Set property from python version
 */
int EDCALL ResourceProperty::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*=true*/ )
{
	BW_GUARD;

	std::string res;
	std::string errStr( "GeneralEditor." );
	errStr += name_;
	int ret = Script::setData( value, res, errStr.c_str() );
	if (ret == 0)
	{
		// check that the extension is right
		int rl = res.length();
		int el = extension_.length();
		if (rl < el || res.substr( rl-el ) != extension_)
		{
			PyErr_Format( PyExc_ValueError, "%s must be set to a string "
				"ending in '%s'", errStr.c_str(), extension_.c_str() );
			return -1;
		}

		// check that the datasection is right
		DataSectionPtr pSect = BWResource::openSection( res );
		if (!pSect)
		{
			PyErr_Format( PyExc_ValueError, "%s must be set to a "
				"valid resource name", errStr.c_str() );
			return -1;
		}
		if (!checker_.check( pSect ))
		{
			PyErr_Format( PyExc_ValueError, "%s cannot be set to %s "
				"because it is the wrong kind of resource for it",
				errStr.c_str(), res.c_str() );
			return -1;
		}

		// ok, set away then
		pString_->set( res, transient, addBarrier );
	}
	return ret;
}

ResourceProperty::Checker ResourceProperty::Checker::instance;


GENPROPERTY_VIEW_FACTORY( ResourceProperty )




/**
 *	Constructor.
 */
GenBoolProperty::GenBoolProperty( const std::string & name,
		BoolProxyPtr pBool ) :
	GeneralProperty( name ),
	pBool_( pBool )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Get python version of property
 */
PyObject * EDCALL GenBoolProperty::pyGet()
{
	BW_GUARD;

	return Script::getData( pBool_->get() );
}

/**
 *	Set property from python version
 */
int EDCALL GenBoolProperty::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*=true*/ )
{
	BW_GUARD;

	bool b;
	std::string errStr( "GeneralEditor." );
	errStr += name_;
	int ret = Script::setData( value, b, errStr.c_str() );
	if (ret == 0)
	{
		pBool_->set( b, transient, addBarrier );
	}
	return ret;
}


GENPROPERTY_VIEW_FACTORY( GenBoolProperty )



/**
 *	Constructor.
 */
GenIntProperty::GenIntProperty( const std::string & name,
		IntProxyPtr pInt ) :
	GeneralProperty( name ),
	pInt_( pInt )
{
	BW_GUARD;

    //DEBUG_MSG( "creating views for INT property\n" );
	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Get python version of property
 */
PyObject * EDCALL GenIntProperty::pyGet()
{
	BW_GUARD;

	return Script::getData( pInt_->get() );
}

/**
 *	Set property from python version
 */
int EDCALL GenIntProperty::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*=true*/ )
{
	BW_GUARD;

	int i = 0;
	std::string errStr( "GeneralEditor." );
	errStr += name_;
	int ret = Script::setData( value, i, errStr.c_str() );
	if (ret == 0)
	{
		pInt_->set( i, transient , addBarrier );
	}
	return ret;
}


GENPROPERTY_VIEW_FACTORY( GenIntProperty )



/**
 *	Constructor.
 */
GenUIntProperty::GenUIntProperty( const std::string & name,
		UIntProxyPtr pUInt ) :
	GeneralProperty( name ),
	pUInt_( pUInt )
{
	BW_GUARD;

    //DEBUG_MSG( "creating views for INT property\n" );
	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Get python version of property
 */
PyObject * EDCALL GenUIntProperty::pyGet()
{
	BW_GUARD;

	return Script::getData( pUInt_->get() );
}

/**
 *	Set property from python version
 */
int EDCALL GenUIntProperty::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*=true*/ )
{
	BW_GUARD;

	uint32 i;
	std::string errStr( "GeneralEditor." );
	errStr += name_;
	int ret = Script::setData( value, i, errStr.c_str() );
	if (ret == 0)
	{
		pUInt_->set( i, transient , addBarrier );
	}
	return ret;
}


GENPROPERTY_VIEW_FACTORY( GenUIntProperty )


/**
 *	Constructor for IntProxy.
 */
ChoiceProperty::ChoiceProperty( const std::string & name,
		IntProxyPtr pInt, DataSectionPtr pChoices, 
		bool sanitiseNames_ /*= false*/ ) :
	GeneralProperty( name ),
	pInt_( pInt ),
	pChoices_( pChoices ),
	sanitise_( sanitiseNames_ )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Constructor for UIntProxy
 */
ChoiceProperty::ChoiceProperty( const std::string & name,
		UIntProxyPtr pUInt, DataSectionPtr pChoices, 
		bool sanitiseNames_ /*= false*/ ) :
	GeneralProperty( name ),
	pUInt_( pUInt ),
	pChoices_( pChoices ),
	sanitise_( sanitiseNames_ )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


DataSectionPtr ChoiceProperty::pChoices() 
{ 
	return pChoices_; 
}


/**
 *	Get python version of property
 */
PyObject * EDCALL ChoiceProperty::pyGet()
{
	BW_GUARD;

	uint32 v = proxyGet();

	// find the selection that matches it
	for (DataSectionIterator it = pChoices_->begin();
		it != pChoices_->end();
		it++)
	{
		if ((*it)->asInt() == v)
		{
			return PyString_FromString( getName((*it)->sectionName(), *it).c_str() );
		}
	}

	// if it doesn't have a legal value set it to the first one (I guess)
	if (pChoices_->countChildren() != 0)
	{
		DataSectionPtr firstChild = pChoices_->openChild(0);
		return PyString_FromString(
			getName(firstChild->sectionName(), firstChild).c_str() );
	}

	// return an empty string then
	return PyString_FromString( "" );
}


/**
 *	Set property from python version
 */
int EDCALL ChoiceProperty::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*=true*/ )
{
	BW_GUARD;

	// find out what we're looking to match
	bool isInt = false;
	bool isStr = false;
	int asInt = 0;
	std::string asStr;
	if (Script::setData( value, asInt ) == 0)
	{
		isInt = true;
	}
	else
	{
		PyErr_Clear();
		if (PyString_Check( value ))
		{
			asStr = PyString_AsString( value );
			isStr = true;
		}
	}
	if (!isInt && !isStr)
	{
		PyErr_Format( PyExc_TypeError, "GeneralEditor.%s "
			"must be set to an int or a string", name_.c_str() );
		return -1;
	}

	// find either the string or int in the data section
	for (DataSectionIterator it = pChoices_->begin();
		it != pChoices_->end();
		it++)
	{
		if (isInt && (*it)->asInt() == asInt)
		{
			proxySet( asInt, transient, addBarrier );
			return 0;
		}

		if (isStr && getName((*it)->sectionName(), *it) == asStr)
		{
			proxySet( (*it)->asInt(), transient, addBarrier );
			return 0;
		}
	}

	// generate an error if it wasn't found
	std::string choiceStr;
	int nch = pChoices_->countChildren();
	for (int index = 0; index < nch; index++)
	{
		DataSectionPtr childPtr = pChoices_->openChild( index );
		choiceStr += getName(childPtr->sectionName(), childPtr);
		if (index < nch - 1) choiceStr += ", ";
		if (index == nch-2) choiceStr += "or ";
	}
	if (nch == 0) choiceStr = "[NO CHOICES]";

	PyErr_Format( PyExc_ValueError, "GeneralEditor.%s must be set to %s",
		name_, choiceStr.c_str() );
	return -1;
}


std::string ChoiceProperty::getName(const std::string & name, DataSectionPtr section) const
{
	BW_GUARD;

	if (sanitise_)
		return section->unsanitise(name);
	else
		return name;
}


GENPROPERTY_VIEW_FACTORY( ChoiceProperty )



/**
 *	Constructor.
 */
PythonProperty::PythonProperty( const std::string & name,
		PythonProxyPtr pProxy ) :
	GeneralProperty( name ),
	pProxy_( pProxy )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Get python version of property
 */
PyObject * EDCALL PythonProperty::pyGet()
{
	BW_GUARD;

	PyObject * pObj = pProxy_->get().getObject();
	Py_XINCREF( pObj );
	return pObj;
}


/**
 *	Set property from python version
 */
int EDCALL PythonProperty::pySet( PyObject * value, bool transient /* = false */,
													bool addBarrier /*= true*/ )
{
	BW_GUARD;

	pProxy_->set( value, transient, addBarrier );
	return 0;
}


GENPROPERTY_VIEW_FACTORY( PythonProperty )


/**
 *	Constructor.
 *
 *	@param name		name of the property
 *	@param proxy	proxy to use to forward changes to the property.
 *	@param item		chunk item the property belongs to.
 */
ArrayProperty::ArrayProperty( const std::string & name, ArrayProxyPtr proxy ) :
	GeneralProperty( name ),
	proxy_( proxy )
{
	BW_GUARD;

	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Elects the property and calls elect in the proxy so the array's items
 *	are added as PropertyItems as well.
 */
void ArrayProperty::elect()
{
	BW_GUARD;

	GeneralProperty::elect();
	proxy_->elect( this );
}


/**
 *	Expels the array's items property items and the expelts itself.
 */
void ArrayProperty::expel()
{
	BW_GUARD;

	proxy_->expel( this );
	GeneralProperty::expel();
}


/**
 *	Called when the array property item is selected. It calls the proxy, which
 *	might create extra gizmos, etc.
 */
void ArrayProperty::select()
{
	BW_GUARD;

	GeneralProperty::select();
	proxy_->select( this );
}


/**
 *	Get the python object equivalent to the current value of this property
 */
PyObject* ArrayProperty::pyGet()
{
	BW_GUARD;

	return Script::getData( proxy_->asString() );
}


/**
 *	Returns the proxy used in this property.
 *
 *	@return		proxy used in this property.
 */
ArrayProxyPtr ArrayProperty::proxy() const
{
	BW_GUARD;

	return proxy_;
}


GENPROPERTY_VIEW_FACTORY( ArrayProperty )


int PropertyModifyGuard::s_stackDepth_ = 0;


// general_properties.cpp
