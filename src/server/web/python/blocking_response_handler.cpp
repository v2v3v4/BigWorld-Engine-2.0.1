/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "Python.h"

#include "blocking_response_handler.hpp"

#include "entitydef/data_description.hpp"
#include "entitydef/method_description.hpp"

/**
 *	Constructor.
 *	@param methodDesc 			The method description to be handled.
 *	@param networkInterface 	The interface to listen for a reply on.
 */
BlockingResponseHandler::BlockingResponseHandler(
		const MethodDescription & methodDesc,
		Mercury::NetworkInterface & networkInterface ):
	Mercury::BlockingReplyHandler( networkInterface, NULL ),
	methodDesc_( methodDesc ),
	pReturnValueDict_( NULL )
{}


/**
 *	Destructor.
 */
BlockingResponseHandler::~BlockingResponseHandler()
{}


/**
 *	Returns a new reference to the reply dictionary object, or NULL if the
 *	reply has not been received.
 */
PyObjectPtr BlockingResponseHandler::getDict()
{ 
	return pReturnValueDict_;
}


/*
 *	This method overrides the BlockingReplyHandler method.
 */
void BlockingResponseHandler::onMessage( const Mercury::Address & source,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data,
		void * arg )
{
	// in case there's already one
	pReturnValueDict_ = PyObjectPtr( PyDict_New(),
		PyObjectPtr::STEAL_REFERENCE );

	int numReturnValues = methodDesc_.returnValues();
	for (int i = 0; i < numReturnValues; ++i)
	{
		DataTypePtr type = methodDesc_.returnValueType( i );
		std::string name = methodDesc_.returnValueName( i );

		PyObjectPtr value = type->createFromStream( data, false );

		PyDict_SetItemString( pReturnValueDict_.get() , name.c_str(),
			value.get() );
	}
}


// blocking_response_handler.cpp
