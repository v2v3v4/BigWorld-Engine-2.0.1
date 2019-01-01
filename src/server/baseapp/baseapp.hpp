/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BASEAPP_HPP
#define BASEAPP_HPP

#include "Python.h"

#include "backed_up_base_app.hpp"
#include "baseapp_int_interface.hpp"
#include "baseappmgr_gateway.hpp"
#include "bases.hpp"
#include "bwtracer.hpp"
#include "rate_limit_message_filter.hpp"

#include "connection/baseapp_ext_interface.hpp"

#include "cstdmf/shared_ptr.hpp"
#include "cstdmf/singleton.hpp"

#include "network/channel_owner.hpp"

#include "server/common.hpp"
#include "server/entity_app.hpp"

#include <string>
#include <map>

class Archiver;
class BackedUpBaseApps;
class BackupHashChain;
class BackupSender;
class Base;
class BaseAppConfig;
class BaseMessageForwarder;
class CellEntityMailBox;
class DeadCellApps;
class EntityCreator;
class EntityType;
class GlobalBases;
class LoginHandler;
class OffloadedBackups;
class Pickler;
class Proxy;
class PythonServer;
class SharedDataManager;
class SqliteDatabase;
class TimeKeeper;
class WorkerThread;

struct BaseAppInitData;

typedef SmartPointer< Base > BasePtr;
typedef SmartPointer< EntityType > EntityTypePtr;
typedef SmartPointer< PyObject > PyObjectPtr;
typedef SmartPointer< Proxy > ProxyPtr;

typedef Mercury::ChannelOwner DBMgr;


/**
 *	This class implement the main singleton object in the base application.
 *	Its main responsibility is to manage all of the bases on the local process.
 */
class BaseApp : public EntityApp,
	public TimerHandler,
	public Mercury::ChannelTimeOutHandler,
	public Singleton< BaseApp >
{
public:
	typedef BaseAppConfig Config;

	ENTITY_APP_HEADER( BaseApp, baseApp )

	BaseApp( Mercury::EventDispatcher & mainDispatcher,
		  Mercury::NetworkInterface & internalInterface );
	virtual ~BaseApp();

	void shutDown();

	static void postDestruction();

	/* Internal Interface */

	// create this client's proxy
	void createBaseWithCellData( const Mercury::Address& srcAddr,
			const Mercury::UnpackedMessageHeader& header,
			BinaryIStream & data );

	// Handles request create a base from DB
	void createBaseFromDB( const Mercury::Address& srcAddr,
			const Mercury::UnpackedMessageHeader& header,
			BinaryIStream & data );

	void logOnAttempt( const Mercury::Address& srcAddr,
			const Mercury::UnpackedMessageHeader& header,
			BinaryIStream & data );

	void addGlobalBase( BinaryIStream & data );
	void delGlobalBase( BinaryIStream & data );

	void callWatcher( const Mercury::Address& srcAddr,
		const Mercury::UnpackedMessageHeader& header,
		BinaryIStream & data );

	void handleCellAppMgrBirth(
		const BaseAppIntInterface::handleCellAppMgrBirthArgs & args );
	void handleBaseAppMgrBirth(
		const BaseAppIntInterface::handleBaseAppMgrBirthArgs & args );
	void addBaseAppMgrRebirthData( BinaryOStream & stream );

	void handleCellAppDeath( BinaryIStream & data );

	void emergencySetCurrentCell( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data );

	void startup( const BaseAppIntInterface::startupArgs & args );

	// shut down this app nicely
	void shutDown( const BaseAppIntInterface::shutDownArgs & args );
	void controlledShutDown( const Mercury::Address& srcAddr,
		const Mercury::UnpackedMessageHeader& header,
		BinaryIStream & data );

	void setCreateBaseInfo( BinaryIStream & data );

	// New-style BaseApp backup
	void startBaseEntitiesBackup(
			const BaseAppIntInterface::startBaseEntitiesBackupArgs & args );

	void stopBaseEntitiesBackup(
			const BaseAppIntInterface::stopBaseEntitiesBackupArgs & args );

	void backupBaseEntity( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void ackOffloadBackup( BinaryIStream & data );

	void makeLocalBackup( Base & base );

	void createBaseFromStream( EntityID id, BinaryIStream & stream );

	void stopBaseEntityBackup( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			const BaseAppIntInterface::stopBaseEntityBackupArgs & args );

	void handleBaseAppBirth( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void handleBaseAppDeath( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void setBackupBaseApps( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void startOffloading( BinaryIStream & stream );

	Mercury::Address backupAddrFor( EntityID entityID ) const;

	// Shared Data message handlers
	void setSharedData( BinaryIStream & data );
	void delSharedData( BinaryIStream & data );

	// set the proxy to receive future messages
	void setClient( const BaseAppIntInterface::setClientArgs & args );

	/* External Interface */
	// let the proxy know who we really are
	void baseAppLogin( const Mercury::Address& srcAddr,
			const Mercury::UnpackedMessageHeader& header,
			BinaryIStream & data );

	// let the proxy know who we really are
	void authenticate( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		const BaseAppExtInterface::authenticateArgs & args );

	/* C++ stuff */

	bool initScript();

	float getLoad() const						{ return load_; }

	Mercury::NetworkInterface & intInterface()	{ return interface_; }
	Mercury::NetworkInterface & extInterface()	{ return extInterface_; }

	static Mercury::Channel & getChannel( const Mercury::Address & addr )
	{
		return BaseApp::instance().intInterface().findOrCreateChannel( addr );
	}

	BaseAppMgrGateway & baseAppMgr()				{ return baseAppMgr_; }
	const BaseAppMgrGateway & baseAppMgr() const	{ return baseAppMgr_; }

	const Mercury::Address & cellAppMgrAddr() const	{ return cellAppMgr_; }

	DBMgr & dbMgr()						{ return *dbMgr_.pChannelOwner(); }

	SqliteDatabase* pSqliteDB() const			{ return pSqliteDB_; }
	void commitSecondaryDB( bool commit )		{ commitSecondaryDB_ = commit; }

	BackupHashChain & backupHashChain()			{ return *pBackupHashChain_; }

	void addressDead( const Mercury::Address & address );

	void addBase( Base * pNewBase );
	void addProxy( Proxy * pNewProxy, const Mercury::Address & clientAddr );

	void removeBase( Base * pToGo );
	void removeProxy( Proxy * pToGo );

	void addPendingLogin( Proxy * pProxy, const Mercury::Address & addr );

	void impendingDataPresentLocally( uint32 version );
	bool allImpendingDataSent( int urgency );
	bool allReloadedByClients( int urgency );

	static void reloadResources( void * arg );
	void reloadResources();

	void setBaseForCall( Base * pBase, bool isExternalCall );
	Base * getBaseForCall( bool okayIfNull = false );
	ProxyPtr getProxyForCall( bool okayIfNull = false );
	ProxyPtr clearProxyForCall();
	ProxyPtr getAndCheckProxyForCall( Mercury::UnpackedMessageHeader & header,
									  const Mercury::Address & srcAddr );

	std::string pickle( PyObject * pObj ) const;
	PyObject * unpickle( const std::string & str ) const;

	const Bases & bases() const		{ return bases_; }

	shared_ptr< EntityCreator > pEntityCreator() const
									{ return pEntityCreator_; }

	bool nextTickPending() const;

	WorkerThread & workerThread()	{ return *pWorkerThread_; }

	GlobalBases * pGlobalBases() const	{ return pGlobalBases_; }

	bool inShutDownPause() const
			{ return (shutDownTime_ != 0) && (this->time() >= shutDownTime_); }

	bool hasStarted() const				{ return waitingFor_ == 0; }
	bool isShuttingDown() const			{ return shutDownTime_ != 0; }

	void startGameTickTimer();
	void ready( int component );
	void registerSecondaryDB();

	EntityID getID();
	void putUsedID( EntityID id );

	enum
	{
		READY_BASE_APP_MGR	= 0x1,
	};

	bool isRecentlyDeadCellApp( const Mercury::Address & addr ) const;

	Mercury::Address 
	getExternalAddressFor( const Mercury::Address & intAddr ) const;

	virtual void requestRetirement();
	bool isRetiring() const 			{ return isRetiring_; }

	void addForwardingMapping( EntityID entityID, 
		const Mercury::Address & addr );

	bool forwardBaseMessageIfNecessary( EntityID entityID, 
		const Mercury::Address & srcAddr, 
		const Mercury::UnpackedMessageHeader & header, 
		BinaryIStream & data );

	void forwardedBaseMessage( BinaryIStream & data );

	bool backupBaseNow( Base & base, 
						Mercury::ReplyMessageHandler * pHandler = NULL );
	void sendAckOffloadBackup( EntityID entityID, 
							   const Mercury::Address & dstAddr );

	bool entityWasOffloaded( EntityID entityID ) const;

private:
	friend class AddToBaseAppMgrHelper;

	// From ServerApp
	virtual bool init( int argc, char *argv[] );
	virtual void onSignalled( int sigNum );

	bool finishInit( const BaseAppInitData & initData );

	bool initSecondaryDB();

	bool findOtherProcesses();

	int serveInterfaces();
	void addWatchers();
	void backup();
	void archive();

	// Override from EntityApp
	virtual ManagerAppGateway & managerAppGateway() { return baseAppMgr_; }

	// Override from TimerHandler
	virtual void handleTimeout( TimerHandle handle, void * arg );
	void tickGameTime();

	// Override from ChannelTimeOutHandler
	virtual void onTimeOut( Mercury::Channel * pChannel );

	void checkSendWindowOverflows();

	void tickRateLimitFilters();
	void sendIdleProxyChannels();

	// Data members

	Mercury::NetworkInterface		extInterface_;

	// Must be before dbMgr_ as dbMgr_ destructor can cancel pending requests
	// and call back to EntityCreator's idClient_.
	shared_ptr< EntityCreator >     pEntityCreator_;

	BaseAppMgrGateway				baseAppMgr_;
	Mercury::Address				cellAppMgr_;
	AnonymousChannelClient			dbMgr_;

	SqliteDatabase *				pSqliteDB_;
	bool							commitSecondaryDB_;

	BWTracerHolder					bwtracer_;

	typedef std::map< Mercury::Address, Proxy * > Proxies;
	Proxies 						proxies_;

	Bases 							bases_;

	BaseAppID						id_;

	BasePtr							baseForCall_;
	EntityID						forwardingEntityIDForCall_;
	bool							baseForCallIsProxy_;
	bool							isExternalCall_;

	PythonServer *					pPythonServer_;

	GameTime						commitTime_;
	GameTime						shutDownTime_;
	Mercury::ReplyID				shutDownReplyID_;
	bool							isRetiring_;
	TimeKeeper *					pTimeKeeper_;
	TimerHandle						gameTimer_;

	enum TimeOutType
	{
		TIMEOUT_GAME_TICK,
	};

	WorkerThread * 					pWorkerThread_;

	GlobalBases *					pGlobalBases_;

	// Misc
	Pickler *						pPickler_;

	bool							isBootstrap_;
	bool							didAutoLoadEntitiesFromDB_;
	int								waitingFor_;

	float							load_;

	shared_ptr< LoginHandler >      pLoginHandler_;
	shared_ptr< BackedUpBaseApps >  pBackedUpBaseApps_;
	shared_ptr< DeadCellApps >      pDeadCellApps_;
	shared_ptr< BackupSender >      pBackupSender_;
	shared_ptr< Archiver >          pArchiver_;
	shared_ptr< SharedDataManager > pSharedDataManager_;
	shared_ptr< BaseMessageForwarder > pBaseMessageForwarder_;
	shared_ptr< BackupHashChain > 	pBackupHashChain_;

	// This is just like BackedUpBaseApps, but it stores backups for entities
	// that we have offloaded.
	shared_ptr< OffloadedBackups >  pOffloadedBackups_;

	typedef std::map< Mercury::Address, Mercury::Address > BaseAppExtAddresses;
	BaseAppExtAddresses baseAppExtAddresses_;
};


#ifdef CODE_INLINE
#include "baseapp.ipp"
#endif


#endif // BASEAPP_HPP
