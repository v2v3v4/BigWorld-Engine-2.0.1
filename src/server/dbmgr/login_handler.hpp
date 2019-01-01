/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOGIN_HANDLER_HPP
#define LOGIN_HANDLER_HPP

#include "get_entity_handler.hpp"

#include "connection/log_on_params.hpp"
#include "dbmgr_lib/billing_system.hpp"
#include "dbmgr_lib/idatabase.hpp"

#include "network/bundle.hpp"
#include "network/interfaces.hpp"

class LoginHandler;

/**
 *	This simple class is used to handle the different versions of
 *	onPutEntityComplete.
 */
class PutEntityHandler : public IDatabase::IPutEntityHandler
{
public:
	typedef void (LoginHandler::*MemberFunc)( bool isOK, DatabaseID );

	PutEntityHandler( LoginHandler & loginHandler, MemberFunc memberFunc );

	virtual void onPutEntityComplete( bool isOK, DatabaseID dbID );

private:
	LoginHandler & loginHandler_;
	MemberFunc memberFunc_;
};


/**
 *	This class is used to receive the reply from a createEntity call to
 *	BaseAppMgr.
 */
class LoginHandler : public Mercury::ReplyMessageHandler,
					 public IGetEntityKeyForAccountHandler,
					 public GetEntityHandler,
					 public IDatabase::IPutEntityHandler
{
public:
	LoginHandler(
			LogOnParamsPtr pParams,
			const Mercury::Address & clientAddr,
			const Mercury::Address & replyAddr,
			bool offChannel,
			Mercury::ReplyID replyID );
	~LoginHandler();

	void login();

	// Mercury::ReplyMessageHandler overrides
	virtual void handleMessage( const Mercury::Address & source,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data, void * arg );

	virtual void handleException( const Mercury::NubException & ne,
		void * arg );
	virtual void handleShuttingDown( const Mercury::NubException & ne,
		void * arg );

	// IGetEntityKeyForAccountHandler override
	virtual void onGetEntityKeyForAccountSuccess( const EntityKey & ekey );
	virtual void onGetEntityKeyForAccountLoadFromUsername(
			EntityTypeID typeID,
			const std::string & username,
			bool shouldCreateNewOnLoadFailure );
	virtual void onGetEntityKeyForAccountCreateNew( EntityTypeID typeID,
			bool shouldRemember );
	virtual void onGetEntityKeyForAccountFailure( LogOnStatus status,
			const std::string & errorMsg );

	// Database::getEntityHandler override
	virtual void onGetEntityCompleted( bool isOK,
				const EntityDBKey & entityKey,
				const EntityMailBoxRef * pBaseEntityLocation );

	// Called by the different PutEntityHandlers
	void onPutEntityComplete( bool isOK, DatabaseID dbID );
	void onReservedBaseMailbox( bool isOK, DatabaseID dbID );
	void onSetBaseMailbox( bool isOK, DatabaseID dbID );

private:
	void handleFailure( BinaryIStream * pData, LogOnStatus reason );
	void checkOutEntity();
	void createNewEntity( EntityTypeID entityTypeID, bool shouldRemember );
	void loadEntity( const EntityDBKey & ekey );
	void sendCreateEntityMsg();
	void sendReply();
	void sendFailureReply( LogOnStatus status, const char * msg = NULL );

	EntityKey			ekey_;
	LogOnParamsPtr		pParams_;
	Mercury::Address	clientAddr_;
	Mercury::Address	replyAddr_;
	bool				offChannel_;
	Mercury::ReplyID	replyID_;

	Mercury::Bundle		bundle_;
	EntityMailBoxRef	baseRef_;
	EntityMailBoxRef *	pBaseRef_;

	PutEntityHandler	putEntityHandler_;
	PutEntityHandler	reserveBaseMailboxHandler_;
	PutEntityHandler	setBaseMailboxHandler_;

	DatabaseID *		pStrmDbID_;

	bool				shouldCreateNewOnLoadFailure_;
};

#endif // LOGIN_HANDLER_HPP
