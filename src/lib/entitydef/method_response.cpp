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

#include "Python.h"

#include "method_response.hpp"

#include "data_description.hpp"
#include "method_description.hpp"

#include "network/bundle.hpp"
#include "network/network_interface.hpp"

DECLARE_DEBUG_COMPONENT( 0 )


// -----------------------------------------------------------------------------
// Section: MethodResponse
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( MethodResponse )

PY_BEGIN_METHODS( MethodResponse )
	PY_METHOD( done )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( MethodResponse )
PY_END_ATTRIBUTES()

PY_SCRIPT_CONVERTERS( MethodResponse )

/**
 *	Constructor.
 *
 *	@param replyID 				The reply ID.
 *	@param replyAddr 			The reply address.
 *	@param networkInterface 	The network interface to listen for a reply on.
 *	@param methodDesc 			The description of the method.
 */
MethodResponse::MethodResponse( int replyID,
		const Mercury::Address & replyAddr,
		Mercury::NetworkInterface & networkInterface,
		const MethodDescription & methodDesc ):
	PyObjectPlus( &MethodResponse::s_type_, false ),
	replyID_( replyID ),
	replyAddr_( replyAddr ),
	interface_( networkInterface ),
	methodDesc_( methodDesc ),
	returnValueData_()
{
	uint numReturnValues = methodDesc.returnValues();

	for (uint i = 0 ; i < numReturnValues; ++i)
	{
		DataTypePtr pDataType = methodDesc.returnValueType( i );
		MF_ASSERT( pDataType );

		const std::string & returnValueName = methodDesc.returnValueName( i );
		returnValueData_[returnValueName] = pDataType->pDefaultValue();
	}

	// There may be situations where the number of returnValueData elements is
	// not equal to the number reported by methodDesc - ie if two return values
	// have the same name - should check for this.
	if (numReturnValues != returnValueData_.size())
	{
		ERROR_MSG( "MethodResponse::MethodResponse(): "
				"Method description reports %d return values, but %"PRIzu" "
				"value data objects are present",
			numReturnValues, returnValueData_.size() );
	}
}


/**
 *	Destructor.
 */
MethodResponse::~MethodResponse()
{
}


/**
 *	This method overrides the PyObjectPlus::pyGetAttribute method.
 *
 *	@param attr		The attribute name.
 *	@return			A new reference to the Python object if it exists, or NULL
 *					otherwise (with an exception thrown).
 */
PyObject * MethodResponse::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();

	// see if we have it
	ReturnValueData::iterator iReturnValue = 
		returnValueData_.find( attr );

	if (iReturnValue != returnValueData_.end())
	{
		PyObjectPtr pData = iReturnValue->second;
		Py_INCREF( pData.get() );
		return pData.get();
	}

	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	This method overrides the PyObjectPlus::pySetAttribute method.
 *
 *	@param	attr	the attribute name
 *	@param	value	the python object to set to
 *
 *	@return	0 on success, nonzero on failure
 */
int MethodResponse::pySetAttribute( const char * attr, PyObject * value )
{
	// See if it's one of our standard attributes
	PY_SETATTR_STD();

	// see if we have it
	MethodResponse::ReturnValueData::iterator iReturnValue =
		returnValueData_.find( attr );

	if (iReturnValue != returnValueData_.end())
	{
		iReturnValue->second = PyObjectPtr( value );
		return 0;
	}

	return PyObjectPlus::pySetAttribute( attr, value );
}


/**
 *	Send off the method response back to the requester.
 */
PyObject * MethodResponse::py_done( PyObject * args )
{
	// create a reply bundle
	Mercury::Bundle b;
	b.startReply( replyID_ );

	int numReturnValues = methodDesc_.returnValues();

	// stream each return value's..err.. value onto the bundle
	for (int i = 0; i < numReturnValues; ++i)
	{
		DataTypePtr pType = methodDesc_.returnValueType( i );
		PyObjectPtr pValue = returnValueData_[methodDesc_.returnValueName( i )];

		pType->addToStream( pValue.get(), b, false );
	}

	// send the reply message back to sender
	interface_.send( replyAddr_, b );

	Py_RETURN_NONE;
}

// method_response.cpp
