/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "login_handler.hpp"

#include "database.hpp"

#include "dbmgr_lib/db_entitydefs.hpp"

#include "baseappmgr/baseappmgr_interface.hpp"

// -----------------------------------------------------------------------------
// Section: LoginHandler
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
LoginHandler::LoginHandler(
			LogOnParamsPtr pParams,
			const Mercury::Address & clientAddr,
			const Mercury::Address & replyAddr,
			bool offChannel,
			Mercury::ReplyID replyID ) :
		ekey_( 0, 0 ),
		pParams_( pParams ),
		clientAddr_( clientAddr ),
		replyAddr_( replyAddr ),
		offChannel_( offChannel ),
		replyID_( replyID ),
		bundle_(),
		baseRef_(),
		putEntityHandler_( *this, &LoginHandler::onPutEntityComplete ),
		reserveBaseMailboxHandler_( *this, &LoginHandler::onReservedBaseMailbox ),
		setBaseMailboxHandler_( *this, &LoginHandler::onSetBaseMailbox ),
		pStrmDbID_( NULL ),
		shouldCreateNewOnLoadFailure_( false )
{
}


/**
 *	Destructor.
 */
LoginHandler::~LoginHandler()
{
}


/**
 *	Start the login process
 */
void LoginHandler::login()
{
	Database::instance().pBillingSystem()->getEntityKeyForAccount(
		pParams_->username(), pParams_->password(), clientAddr_, *this );

	// When getEntityKeyForAccount() completes, onGetEntityKeyForAccount*()
	// will be called.
}


/*
 *	IGetEntityKeyForAccountHandler override
 */
void LoginHandler::onGetEntityKeyForAccountSuccess( const EntityKey & ekey )
{
	this->loadEntity( EntityDBKey( ekey ) );
}


/*
 *	IGetEntityKeyForAccountHandler override
 */
void LoginHandler::onGetEntityKeyForAccountCreateNew( EntityTypeID typeID,
		bool shouldRemember )
{
	this->createNewEntity( typeID, shouldRemember );
}


/*
 *	IGetEntityKeyForAccountHandler override
 */
void LoginHandler::onGetEntityKeyForAccountLoadFromUsername(
		EntityTypeID typeID, const std::string & username,
		bool shouldCreateNewOnLoadFailure )
{
	shouldCreateNewOnLoadFailure_ = shouldCreateNewOnLoadFailure;
	pParams_->username( username );
	this->loadEntity( EntityDBKey( typeID, 0, username ) );
}


/**
 *	This method loads the entity with the given key.
 */
void LoginHandler::loadEntity( const EntityDBKey & ekey )
{
	ekey_ = ekey;

	// Start "create new base" message even though we're not sure entity
	// exists. This is to take advantage of getEntity() streaming properties
	// into the bundle directly.
	pStrmDbID_ = Database::prepareCreateEntityBundle( ekey_.typeID,
		ekey_.dbID, clientAddr_, this, bundle_, pParams_ );

	// Get entity data
	pBaseRef_ = &baseRef_;

	const char * pPasswordOverride = NULL;

	if (Database::instance().getEntityDefs().entityTypeHasPassword(
				ekey_.typeID ))
	{
		pPasswordOverride = pParams_->password().c_str();
	}

	Database::instance().getEntity( ekey,
			&bundle_, true, pPasswordOverride, *this );
	// When getEntity() completes, onGetEntityCompleted() is called.
}


/*
 *	IGetEntityKeyForAccountHandler override
 */
void LoginHandler::onGetEntityKeyForAccountFailure( LogOnStatus status,
	const std::string & errorMsg )
{
	const char * msg;
	bool isError = false;

	switch (status)
	{
		case LogOnStatus::LOGIN_REJECTED_NO_SUCH_USER:
			msg = "Unknown user.";
			break;

		case LogOnStatus::LOGIN_REJECTED_INVALID_PASSWORD:
			msg = "Invalid password.";
			break;

		case LogOnStatus::LOGIN_REJECTED_DB_GENERAL_FAILURE:
			msg = "Unexpected database failure.";
			isError = true;
			break;

		default:
			msg = "Unspecified error";
			break;
	}

	if (!errorMsg.empty())
	{
		msg = errorMsg.c_str();
	}

	if (isError)
	{
		ERROR_MSG( "LoginHandler::onMapLoginToEntityKeyComplete: "
						"Failed for username '%s': (%d) %s\n",
					pParams_->username().c_str(), int( status ), msg );
	}
	else
	{
		NOTICE_MSG( "LoginHandler::onMapLoginToEntityKeyComplete: "
						"Failed for username '%s': (%d) %s\n",
					pParams_->username().c_str(), int( status ), msg );
	}

	Database::instance().sendFailure( replyID_, replyAddr_, offChannel_,
		status, msg );
	delete this;
}


/**
 *	Database::GetEntityHandler override
 */
void LoginHandler::onGetEntityCompleted( bool isOK,
					const EntityDBKey & entityKey,
					const EntityMailBoxRef * pBaseEntityLocation )
{
	if (!isOK)
	{
		if (shouldCreateNewOnLoadFailure_)
		{
			this->createNewEntity( ekey_.typeID, false );
		}
		else
		{
			ERROR_MSG( "LoginHandler::onGetEntityCompleted: "
							"Entity '%"FMT_DBID"' does not exist\n",
						ekey_.dbID );

			this->sendFailureReply(
				LogOnStatus::LOGIN_REJECTED_NO_SUCH_USER,
				"Failed to load entity." );
		}
		return;
	}

	ekey_ = entityKey;

	if (pBaseEntityLocation != NULL)
	{
		baseRef_ = *pBaseEntityLocation;
		pBaseRef_ = &baseRef_;
	}
	else
	{
		pBaseRef_ = NULL;
	}

	if (pStrmDbID_)
	{
		// Means ekey.dbID was 0 when we called prepareCreateEntityBundle()
		// Now fix up everything.
		*pStrmDbID_ = entityKey.dbID;
	}

	this->checkOutEntity();
}


/**
 *	This function checks out the login entity. Must be called after
 *	entity has been successfully retrieved from the database.
 */
void LoginHandler::checkOutEntity()
{
	if ((pBaseRef_ == NULL) &&
		Database::instance().onStartEntityCheckout( ekey_ ))
	{
		// Not checked out and not in the process of being checked out.
		Database::setBaseRefToLoggingOn( baseRef_, ekey_.typeID );
		pBaseRef_ = &baseRef_;

		Database::instance().setBaseEntityLocation( ekey_, baseRef_,
				reserveBaseMailboxHandler_ );
		// When completes, onReservedBaseMailbox() is called.
	}
	else	// Checked out
	{
		Database::instance().onLogOnLoggedOnUser( ekey_.typeID, ekey_.dbID,
			pParams_, clientAddr_, replyAddr_, offChannel_, replyID_,
			pBaseRef_ );

		delete this;
	}
}


/**
 *	This method is called when writing the entity has finished.
 */
void LoginHandler::onPutEntityComplete( bool isOK, DatabaseID dbID )
{
	MF_ASSERT( pStrmDbID_ );
	*pStrmDbID_ = dbID;
	if (isOK)
	{
		ekey_.dbID = dbID;

		Database::instance().pBillingSystem()->setEntityKeyForAccount(
			pParams_->username(), pParams_->password(),	ekey_ );

		this->sendCreateEntityMsg();
	}
	else
	{	// Failed "rememberEntity" function.
		ERROR_MSG( "LoginHandler::onPutEntityComplete: "
						"Failed to write default entity for "
						"username '%s'\n",
					pParams_->username().c_str());

		this->onReservedBaseMailbox( isOK, dbID );
		// Let them log in anyway since this is meant to be a convenience
		// feature during product development.
	}
}


/**
 *	This method is called when the record in bigworldLogOns has been created
 *	or returned.
 */
void LoginHandler::onReservedBaseMailbox( bool isOK, DatabaseID dbID )
{
	if (isOK)
	{
		this->sendCreateEntityMsg();
	}
	else
	{
		Database::instance().onCompleteEntityCheckout( ekey_, NULL );
		// Something horrible like database disconnected or something.
		this->sendFailureReply(
				LogOnStatus::LOGIN_REJECTED_DB_GENERAL_FAILURE,
				"Unexpected database failure." );
	}
}


/**
 *	This method is called when the record in bigworldLogOns has been set.
 */
void LoginHandler::onSetBaseMailbox( bool isOK, DatabaseID dbID )
{
	Database::instance().onCompleteEntityCheckout( ekey_,
			isOK ? &baseRef_ : NULL );
	if (isOK)
	{
		this->sendReply();
	}
	else
	{
		// Something horrible like database disconnected or something.
		this->sendFailureReply(
				LogOnStatus::LOGIN_REJECTED_DB_GENERAL_FAILURE,
				"Unexpected database failure." );
	}
}


/**
 *	This method sends the BaseAppMgrInterface::createEntity message.
 *	Assumes bundle has the right data.
 */
inline void LoginHandler::sendCreateEntityMsg()
{
	INFO_MSG( "Database::logOn: %s\n", pParams_->username().c_str() );

	Database::instance().baseAppMgr().send( &bundle_ );
}


/**
 *	This method sends the reply to the LoginApp. Assumes bundle already
 *	has the right data.
 *
 *	This should also be the last thing this object is doing so this
 *	also destroys this object.
 */
inline void LoginHandler::sendReply()
{
	if (offChannel_)
	{
		Database::instance().interface().send( replyAddr_, bundle_ );
	}
	else
	{
		Database::getChannel( replyAddr_ ).send( &bundle_ );
	}
	delete this;
}


/**
 *	This method sends a failure reply to the LoginApp.
 *
 *	This should also be the last thing this object is doing so this
 *	also destroys this object.
 */
inline void LoginHandler::sendFailureReply( LogOnStatus status,
		const char * msg )
{
	Database::instance().sendFailure( replyID_, replyAddr_, offChannel_,
		status, msg );
	delete this;
}


/**
 *	This function creates a new login entity for the user.
 */
void LoginHandler::createNewEntity( EntityTypeID entityTypeID,
		bool shouldRemember )
{
	ekey_.typeID = entityTypeID;

	if (pStrmDbID_ == NULL)
	{
		pStrmDbID_ = Database::prepareCreateEntityBundle( entityTypeID,
			0, clientAddr_, this, bundle_, pParams_ );
	}

	bool isDefaultEntityOK;

	if (shouldRemember)
	{
		// __kyl__ (13/7/2005) Need additional MemoryOStream because I
		// haven't figured out how to make a BinaryIStream out of a
		// Mercury::Bundle.
		MemoryOStream strm;
		isDefaultEntityOK = Database::instance().defaultEntityToStrm(
			ekey_.typeID, pParams_->username(), strm, &pParams_->password() );

		if (isDefaultEntityOK)
		{
			bundle_.transfer( strm, strm.size() );

			// If <rememberUnknown> is true we need to append the game time
			// so that secondary databases have a timestamp associated to
			// them.
			strm << GameTime( 0 );
			strm.rewind();

			// Put entity data into DB and set baseref to "logging on".
			Database::setBaseRefToLoggingOn( baseRef_, ekey_.typeID );
			pBaseRef_ = &baseRef_;

			Database::instance().putEntity( ekey_,
				/* entityID: */ 0,
				/* pStream: */ &strm,
				/* pBaseMailbox: */ &baseRef_,
				/* removeBaseMailbox:*/ false,
				/* updateAutoLoad */ UPDATE_AUTO_LOAD_RETAIN,
				/* handler */ putEntityHandler_ );
			// When putEntity() completes, onPutEntityComplete() is called.
		}
	}
	else
	{
		*pStrmDbID_ = 0;

		// No need for additional MemoryOStream. Just stream into bundle.
		isDefaultEntityOK = Database::instance().defaultEntityToStrm(
			ekey_.typeID, pParams_->username(), bundle_,
			&pParams_->password() );

		if (isDefaultEntityOK)
		{
			this->sendCreateEntityMsg();
		}
	}

	if (!isDefaultEntityOK)
	{
		ERROR_MSG( "LoginHandler::createNewEntity: "
					"Failed to create default entity for username '%s'\n",
				pParams_->username().c_str());

		this->sendFailureReply( LogOnStatus::LOGIN_CUSTOM_DEFINED_ERROR,
				"Failed to create default entity" );
	}
}


/*
 *	Mercury::ReplyMessageHandler override.
 */
void LoginHandler::handleMessage( const Mercury::Address & source,
	Mercury::UnpackedMessageHeader & header,
	BinaryIStream & data,
	void * arg )
{
	Mercury::Address proxyAddr;

	data >> proxyAddr;

	if (proxyAddr.ip == 0)
	{
		LogOnStatus::Status status;
		switch (proxyAddr.port)
		{
			case BaseAppMgrInterface::CREATE_ENTITY_ERROR_NO_BASEAPPS:
				status = LogOnStatus::LOGIN_REJECTED_NO_BASEAPPS;
				break;
			case BaseAppMgrInterface::CREATE_ENTITY_ERROR_BASEAPPS_OVERLOADED:
				status = LogOnStatus::LOGIN_REJECTED_BASEAPP_OVERLOAD;
				break;
			default:
				status = LogOnStatus::LOGIN_CUSTOM_DEFINED_ERROR;
				break;
		}

		this->handleFailure( &data, status );
	}
	else
	{
		data >> baseRef_;

		bundle_.clear();
		bundle_.startReply( replyID_ );

		// Assume success.
		bundle_ << (uint8)LogOnStatus::LOGGED_ON;
		bundle_ << proxyAddr;
		// session key (if there is one)
		bundle_.transfer( data, data.remainingLength() );

		if (ekey_.dbID != 0)
		{
			pBaseRef_ = &baseRef_;
			Database::instance().setBaseEntityLocation( ekey_,
					baseRef_, setBaseMailboxHandler_ );
			// When completes, onSetBaseMailbox() is called.
		}
		else
		{
			// Must be "createUnknown", and "rememberUnknown" is false.
			this->sendReply();
		}
	}
}


/*
 *	Mercury::ReplyMessageHandler override.
 */
void LoginHandler::handleException( const Mercury::NubException & ne,
	void * arg )
{
	MemoryOStream mos;
	mos << "BaseAppMgr timed out creating entity.";
	this->handleFailure( &mos,
			LogOnStatus::LOGIN_REJECTED_BASEAPPMGR_TIMEOUT );
}


void LoginHandler::handleShuttingDown( const Mercury::NubException & ne,
	void * arg )
{
	INFO_MSG( "LoginHandler::handleShuttingDown: Ignoring\n" );
	delete this;
}


/**
 *	Handles a failure to create entity base.
 */
void LoginHandler::handleFailure( BinaryIStream * pData,
		LogOnStatus reason )
{
	bundle_.clear();
	bundle_.startReply( replyID_ );

	bundle_ << (uint8)reason;

	bundle_.transfer( *pData, pData->remainingLength() );

	if (ekey_.dbID != 0)
	{
		pBaseRef_ = NULL;
		Database::instance().clearBaseEntityLocation( ekey_,
				setBaseMailboxHandler_ );
		// When completes, onSetBaseMailbox() is called.
	}
	else
	{
		// Must be "createUnknown" and "rememberUnknown" is false.
		this->sendReply();
	}
}


// -----------------------------------------------------------------------------
// Section: PutEntityHandler
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
PutEntityHandler::PutEntityHandler( LoginHandler & loginHandler,
		MemberFunc memberFunc ) :
	loginHandler_( loginHandler ),
	memberFunc_( memberFunc )
{
}


/**
 *	This method calls through to the appropriate member of LoginHandler.
 */
void PutEntityHandler::onPutEntityComplete( bool isOK, DatabaseID dbID )
{
	(loginHandler_.*memberFunc_)( isOK, dbID );
}

// login_handler.cpp
