/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "db_interface.hpp"

#include "dbmgr_lib/db_status.hpp"

#include "connection/log_on_params.hpp"

#include "cstdmf/singleton.hpp"

#include "dbmgr_lib/idatabase.hpp"

#include "network/basictypes.hpp"
#include "network/channel_owner.hpp"

#include "resmgr/datasection.hpp"

#include "server/backup_hash.hpp"
#include "server/server_app.hpp"

#include <map>
#include <set>

class BWResource;
class Consolidator;
class DBMgrConfig;
class EntityDefs;
class GetEntityHandler;
class RelogonAttemptHandler;

namespace DBConfig
{
	class Server;
}

typedef Mercury::ChannelOwner BaseAppMgr;

/**
 *	This class is used to implement the main singleton object that represents
 *	this application.
 */
class Database : public ServerApp,
	public TimerHandler,
	public IDatabase::IGetBaseAppMgrInitDataHandler,
	public IDatabase::IUpdateSecondaryDBshandler,
	public Singleton< Database >
{
public:
	SERVER_APP_HEADER( DBMgr, dbMgr )

	typedef DBMgrConfig Config;

	Database( Mercury::EventDispatcher & dispatcher,
			Mercury::NetworkInterface & interface );
	virtual ~Database();

	BaseAppMgr & baseAppMgr() { return baseAppMgr_; }

	// Overrides
	void handleTimeout( TimerHandle handle, void * arg );

	void handleStatusCheck( BinaryIStream & data );

	// Lifetime messages
	void handleBaseAppMgrBirth( const DBInterface::handleBaseAppMgrBirthArgs & args );
	void handleDatabaseBirth( const DBInterface::handleDatabaseBirthArgs & args );
	void shutDown( const DBInterface::shutDownArgs & args );
	void startSystemControlledShutdown();
	void shutDownNicely();
	void shutDown();

	void controlledShutDown( const DBInterface::controlledShutDownArgs & args );
	void cellAppOverloadStatus( const DBInterface::cellAppOverloadStatusArgs & args );

	// Entity messages
	void logOn( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data );

	void logOn( const Mercury::Address & srcAddr,
		Mercury::ReplyID replyID,
		LogOnParamsPtr pParams,
		const Mercury::Address & addrForProxy,
		bool offChannel );

	void onLogOnLoggedOnUser( EntityTypeID typeID, DatabaseID dbID,
		LogOnParamsPtr pParams,
		const Mercury::Address & proxyAddr, const Mercury::Address& replyAddr,
		bool offChannel, Mercury::ReplyID replyID,
		const EntityMailBoxRef* pExistingBase );

	void onEntityLogOff( EntityTypeID typeID, DatabaseID dbID );

	bool calculateOverloaded( bool isOverloaded );

	void sendFailure( Mercury::ReplyID replyID,
		const Mercury::Address & dstAddr,
		bool offChannel,
		LogOnStatus reason, const char * pDescription = NULL );

	void loadEntity( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data );

	void writeEntity( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data );

	void deleteEntity( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		const DBInterface::deleteEntityArgs & args );
	void deleteEntityByName( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data );

	void lookupEntity( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		const DBInterface::lookupEntityArgs & args );
	void lookupEntityByName( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data );
	void lookupDBIDByName( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data );
	void lookupEntity( EntityDBKey & ekey, BinaryOStream & bos );

	// Misc messages
	void executeRawCommand( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data );

	void putIDs( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data );

	void getIDs( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data );

	void writeSpaces( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data );

	void writeGameTime( const DBInterface::writeGameTimeArgs & args );

	void handleBaseAppDeath( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void checkStatus( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void secondaryDBRegistration( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void updateSecondaryDBs( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );
	virtual void onUpdateSecondaryDBsComplete(
			const IDatabase::SecondaryDBEntries& removedEntries );

	void getSecondaryDBDetails( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header, BinaryIStream & data );

	// Accessors
	const EntityDefs& getEntityDefs() const
		{ return *pEntityDefs_; }
	EntityDefs& swapEntityDefs( EntityDefs& entityDefs )
	{
		EntityDefs& curDefs = *pEntityDefs_;
		pEntityDefs_ = &entityDefs;
		return curDefs;
	}

	Mercury::EventDispatcher & mainDispatcher()		{ return mainDispatcher_; }
	Mercury::NetworkInterface & interface()			{ return interface_; }
	static Mercury::NetworkInterface & getNub()	{ return Database::instance().interface(); }

	static Mercury::Channel & getChannel( const Mercury::Address & addr )
	{
		return Database::instance().interface().findOrCreateChannel( addr );
	}

	IDatabase& getIDatabase()	{ MF_ASSERT(pDatabase_); return *pDatabase_; }
	BillingSystem * pBillingSystem()		{ return pBillingSystem_; }

	// IDatabase pass-through methods. Call these instead of IDatabase methods
	// so that we can intercept and muck around with stuff.

	void getEntity( const EntityDBKey & entityKey,
			BinaryOStream * pStream,
			bool shouldGetBaseEntityLocation,
			const char * pPasswordOverride,
			GetEntityHandler & handler );

	void putEntity( const EntityKey & ekey,
			EntityID entityID,
			BinaryIStream * pStream,
			EntityMailBoxRef * pBaseMailbox,
			bool removeBaseMailbox,
			UpdateAutoLoad updateAutoLoad,
			IDatabase::IPutEntityHandler& handler );

	void setBaseEntityLocation( const EntityKey & entityKey,
			EntityMailBoxRef & mailbox,
			IDatabase::IPutEntityHandler & handler,
			UpdateAutoLoad updateAutoLoad = UPDATE_AUTO_LOAD_RETAIN )
	{
		this->putEntity( entityKey, mailbox.id, NULL, &mailbox, false, 
			updateAutoLoad,
			handler );
	}

	void clearBaseEntityLocation( const EntityKey & entityKey,
			IDatabase::IPutEntityHandler & handler )
	{
		this->putEntity( entityKey, 0, NULL, NULL, true, 
			UPDATE_AUTO_LOAD_RETAIN,
			handler );
	}

	void delEntity( const EntityDBKey & ekey, EntityID entityID,
			IDatabase::IDelEntityHandler& handler );

	// Watcher
	void hasStartBegun( bool hasStartBegun );
	bool hasStartBegun() const
	{
		return status_.status() > DBStatus::WAITING_FOR_APPS;
	}
	bool isConsolidating() const		{ return pConsolidator_.get() != NULL; }

	bool isReady() const
	{
		return status_.status() >= DBStatus::WAITING_FOR_APPS;
	}

	void startServerBegin( bool isRecover = false );
	void startServerEnd( bool isRecover, bool didAutoLoadEntitiesFromDB );
	void startServerError();

	// Sets baseRef to "pending base creation" state.
	static void setBaseRefToLoggingOn( EntityMailBoxRef& baseRef,
		EntityTypeID entityTypeID )
	{
		baseRef.init( 0, Mercury::Address( 0, 0 ),
			EntityMailBoxRef::BASE, entityTypeID );
	}

	bool defaultEntityToStrm( EntityTypeID typeID, const std::string& name,
		BinaryOStream& strm, const std::string* pPassword = 0 ) const;

	static DatabaseID* prepareCreateEntityBundle( EntityTypeID typeID,
		DatabaseID dbID, const Mercury::Address& addrForProxy,
		Mercury::ReplyMessageHandler* pHandler, Mercury::Bundle& bundle,
		LogOnParamsPtr pParams = NULL );

	RelogonAttemptHandler* getInProgRelogonAttempt( EntityTypeID typeID,
			DatabaseID dbID )
	{
		PendingAttempts::iterator iter =
				pendingAttempts_.find( EntityKey( typeID, dbID ) );
		return (iter != pendingAttempts_.end()) ? iter->second : NULL;
	}
	void onStartRelogonAttempt( EntityTypeID typeID, DatabaseID dbID,
		 	RelogonAttemptHandler* pHandler )
	{
		MF_VERIFY( pendingAttempts_.insert(
				PendingAttempts::value_type( EntityKey( typeID, dbID ),
						pHandler ) ).second );
	}
	void onCompleteRelogonAttempt( EntityTypeID typeID, DatabaseID dbID )
	{
		MF_VERIFY( pendingAttempts_.erase( EntityKey( typeID, dbID ) ) > 0 );
	}

	bool onStartEntityCheckout( const EntityKey& entityID )
	{
		return inProgCheckouts_.insert( entityID ).second;
	}
	bool onCompleteEntityCheckout( const EntityKey& entityID,
			const EntityMailBoxRef* pBaseRef );

	/**
	 *	This interface is used to receive the event that an entity has completed
	 *	checking out.
	 */
	struct ICheckoutCompletionListener
	{
		// This method is called when the onCompleteEntityCheckout() is
		// called for the entity that you've registered for via
		// registerCheckoutCompletionListener(). After this call, this callback
		// will be automatically deregistered.
		virtual void onCheckoutCompleted( const EntityMailBoxRef* pBaseRef ) = 0;
	};
	bool registerCheckoutCompletionListener( EntityTypeID typeID,
			DatabaseID dbID, ICheckoutCompletionListener& listener );

	bool hasMailboxRemapping() const	{ return !mailboxRemapInfo_.empty(); }
	void remapMailbox( EntityMailBoxRef& mailbox ) const;

	void onConsolidateProcessEnd( bool isOK );

private:
	virtual bool init( int argc, char * argv[] );
	virtual bool run();

	void finalise();

	void initSecondaryDBPrefix();
	bool initBillingSystem();

	void addWatchers( Watcher & watcher );

	void endMailboxRemapping();

	void sendInitData();
	virtual void onGetBaseAppMgrInitDataComplete( GameTime gameTime,
			int32 appID );


	// Data consolidation methods
	void consolidateData();
	bool startConsolidationProcess();
	bool sendRemoveDBCmd( uint32 destIP, const std::string& dbLocation );

	EntityDefs*			pEntityDefs_;
	IDatabase *			pDatabase_;
	BillingSystem *		pBillingSystem_;

	DBStatus			status_;

	BaseAppMgr			baseAppMgr_;

	bool				shouldSendInitData_;

	bool				shouldConsolidate_;

	TimerHandle				statusCheckTimerHandle_;

	friend class RelogonAttemptHandler;
	typedef std::map< EntityKey, RelogonAttemptHandler * > PendingAttempts;
	PendingAttempts pendingAttempts_;
	typedef std::set< EntityKey > EntityKeySet;
	EntityKeySet	inProgCheckouts_;

	typedef std::multimap< EntityKey, ICheckoutCompletionListener* >
			CheckoutCompletionListeners;
	CheckoutCompletionListeners checkoutCompletionListeners_;

	float				curLoad_;
	bool				anyCellAppOverloaded_;
	uint64				overloadStartTime_;

	typedef std::map< Mercury::Address, BackupHash >	MailboxRemapInfo;
	MailboxRemapInfo	mailboxRemapInfo_;
	int					mailboxRemapCheckCount_;

	std::string			secondaryDBPrefix_;
	uint				secondaryDBIndex_;

	std::auto_ptr< Consolidator >		pConsolidator_;
};

#ifdef CODE_INLINE
#include "database.ipp"
#endif

#endif // DATABASE_HPP
