/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "retrying_remote_method.hpp"

#include "blocking_response_handler.hpp"
#include "mailbox.hpp"
#include "web_integration.hpp"

#include "entitydef/entity_description.hpp"
#include "entitydef/entity_description_map.hpp"
#include "entitydef/mailbox_base.hpp"
#include "entitydef/method_description.hpp"


PY_TYPEOBJECT_WITH_CALL( RetryingRemoteMethod )

PY_BEGIN_METHODS( RetryingRemoteMethod )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( RetryingRemoteMethod )
PY_END_ATTRIBUTES()


/**
 *	Constructor.
 */
RetryingRemoteMethod::RetryingRemoteMethod( WebEntityMailBox * pMailBox,
			const MethodDescription * pMethodDescription,
			PyTypePlus * pType /* = &s_type_*/ ) :
		PyObjectPlus( pType ),
		pMailBox_( pMailBox ),
		pMethodDescription_( pMethodDescription )
{
}


/**
 *	Destructor.
 */
RetryingRemoteMethod::~RetryingRemoteMethod() 
{
}


/**
 *	This method is called when a script wants to call this method
 *	on a remote script handler (entity/base).
 */
PyObject * RetryingRemoteMethod::pyCall( PyObject * args )
{
	if (!pMethodDescription_->areValidArgs( true, args, true ))
	{
		// areValidArgs sets Python error state
		return NULL;
	}

	return this->sendRetrying( args );
}


/**
 *	Send the method request. 
 *
 *	@param args 	The method argument tuple.
 *	@param retry 	If true, retries once on receiving a timeout or a "no such
 *					port" error.
 */
PyObject * RetryingRemoteMethod::sendRetrying( PyObject * args, 
		bool retry /* true */ )
{
	const EntityMailBoxRef ref = pMailBox_->ref();
	WebIntegration & instance = WebIntegration::instance();
	const EntityDescription & entityDesc = 
		instance.entityDescriptions().entityDescription( ref.type() );

	if (!this->createMethodCallStream( args ))
	{
		return NULL;
	}

	pMailBox_->sendStream();

	if (pMethodDescription_->returnValues() > 0)
	{
		MF_ASSERT( pHandler_.get() != NULL ); 	// Should have been set in
												// createMethodCallStream()

		Mercury::Reason err = pHandler_->waitForReply();

		if (err != Mercury::REASON_SUCCESS)
		{
			ERROR_MSG( "RetryingRemoteMethod::sendRetrying: "
					"Got error %s while waiting for reply for %s.%s "
					"for entity ID %d at %s (%s)\n",
				Mercury::reasonToString( err ),
				entityDesc.name().c_str(), 
				pMethodDescription_->name().c_str(),
				ref.id,
				ref.addr.c_str(),
				retry ? "retrying" : "failed" );
			
			if (retry &&
				((err == Mercury::REASON_NO_SUCH_PORT) ||
					(err == Mercury::REASON_TIMER_EXPIRED)))
			{
				// Query the BaseAppMgr and get the hash chain so we can remap
				// our mailboxes.
				if (!instance.checkBaseAppHashHistory())
				{
					// Failed to update, can't retry at this point.
					PyErr_SetString( PyExc_IOError, 
						"could not contact server" );
					return NULL;
				}

				return this->sendRetrying( args, /*retry:*/ false );
			}

			PyErr_Format( PyExc_IOError, 
				"error getting response from server: %s",
				Mercury::reasonToString( err ) );
			return NULL;
		}
		
		PyObjectPtr pDict = pHandler_->getDict().get();
		pHandler_.reset();

		Py_INCREF( pDict.get() );
		return pDict.get();
	}
	else
	{
		// Nothing to do.
		Py_RETURN_NONE;
	}
}


/**
 *	This method creates the stream to be used for sending the remote method.
 *
 *	@param args 	The arguments for the method call. 
 */
bool RetryingRemoteMethod::createMethodCallStream( PyObject * args )
{
	pHandler_.reset();

	if (pMethodDescription_->returnValues() > 0)
	{
		pHandler_.reset( new BlockingResponseHandler( *pMethodDescription_, 
			WebIntegration::instance().interface() ) );
	}

	BinaryOStream * pBOS = pMailBox_->getStream( *pMethodDescription_, 
		pHandler_.get() );

	if (pBOS == NULL)
	{
		PyErr_Format( PyExc_SystemError, "Could not get stream" );
		return false;
	}

	pMethodDescription_->addToStream( true, args, *pBOS );

	return true;
}



// retrying_remote_method.cpp
