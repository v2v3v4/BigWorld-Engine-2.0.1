/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BASE_APP_MGR_HPP
#define BASE_APP_MGR_HPP

#include "baseappmgr_interface.hpp"

#include "cstdmf/shared_ptr.hpp"
#include "cstdmf/singleton.hpp"

#include "network/channel_owner.hpp"
#include "server/common.hpp"
#include "server/manager_app.hpp"

#include <map>
#include <set>
#include <string>

class BackupHashChain;
class BaseApp;
class BaseAppMgrConfig;
class TimeKeeper;

typedef Mercury::ChannelOwner CellAppMgr;
typedef Mercury::ChannelOwner DBMgr;

namespace Mercury
{
class EventDispatcher;
class NetworkInterface;
}

/**
 *	This singleton class is the global object that is used to manage proxies and
 *	bases.
 */
class BaseAppMgr : public ManagerApp,
	public TimerHandler,
	public Singleton< BaseAppMgr >
{
public:
	MANAGER_APP_HEADER( BaseAppMgr, baseAppMgr )

	typedef BaseAppMgrConfig Config;

	BaseAppMgr( Mercury::EventDispatcher & mainDispatcher,
			Mercury::NetworkInterface & interface );
	virtual ~BaseAppMgr();

	void createEntity( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	// BaseApps register/unregister via the add/del calls.
	void add( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			const BaseAppMgrInterface::addArgs & args );

	void recoverBaseApp( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void del( const Mercury::Address & addr,
		const Mercury::UnpackedMessageHeader & header,
		const BaseAppMgrInterface::delArgs & args );
	void informOfLoad( const Mercury::Address & addr,
		const Mercury::UnpackedMessageHeader & header,
		const BaseAppMgrInterface::informOfLoadArgs & args );

	virtual void shutDown() // from ServerApp
		{ this->shutDown( false ); }


	void shutDown( bool shutDownOthers );
	void shutDown( const BaseAppMgrInterface::shutDownArgs & args );
	void startup( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void checkStatus( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void controlledShutDown(
			const BaseAppMgrInterface::controlledShutDownArgs & args );

	void requestHasStarted( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void initData( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void spaceDataRestore( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void setSharedData( BinaryIStream & data );
	void delSharedData( BinaryIStream & data );

	void useNewBackupHash( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void informOfArchiveComplete( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void requestBackupHashChain( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	BaseApp * findBaseApp( const Mercury::Address & addr );
	Mercury::ChannelOwner * findChannelOwner( const Mercury::Address & addr );

	static Mercury::Channel & getChannel( const Mercury::Address & addr );

	int numBaseApps() const					{ return baseApps_.size(); }

	int numBases() const;
	int numProxies() const;

	float minBaseAppLoad() const;
	float avgBaseAppLoad() const;
	float maxBaseAppLoad() const;

	CellAppMgr & cellAppMgr()	{ return cellAppMgr_; }
	DBMgr & dbMgr()				{ return *dbMgr_.pChannelOwner(); }

	// ---- Message Handlers ----
	void handleCellAppMgrBirth(
		const BaseAppMgrInterface::handleCellAppMgrBirthArgs & args );
	void handleBaseAppMgrBirth(
		const BaseAppMgrInterface::handleBaseAppMgrBirthArgs & args );

	void handleCellAppDeath( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );
	void handleBaseAppDeath(
		const BaseAppMgrInterface::handleBaseAppDeathArgs & args );
	void handleBaseAppDeath( const Mercury::Address & addr );

	void registerBaseGlobally( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void deregisterBaseGlobally( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void runScript( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void startAsyncShutDownStage( ShutDownStage stage );

	void controlledShutDownServer();

	bool onBaseAppDeath( const Mercury::Address & addr );

	void removeControlledShutdownBaseApp( const Mercury::Address & addr );

	BaseApp * findBestBaseApp() const;

	void onBaseAppRetire( BaseApp & baseApp );

private:
	virtual bool init( int argc, char* argv[] );

	enum TimeOutType
	{
		TIMEOUT_GAME_TICK
	};

	enum AdjustBackupLocationsOp
	{
		ADJUST_BACKUP_LOCATIONS_OP_ADD,
		ADJUST_BACKUP_LOCATIONS_OP_RETIRE,
		ADJUST_BACKUP_LOCATIONS_OP_CRASH
	};

	virtual void handleTimeout( TimerHandle handle, void * arg );

	void startTimer();
	void checkBackups(); // Old-style backup
	void adjustBackupLocations( const Mercury::Address & addr, 
			AdjustBackupLocationsOp op );

	void checkForDeadBaseApps();

	void updateCreateBaseInfo();

	void runScriptAll( std::string script );
	void runScriptSingle( std::string script );

	void runScript( const std::string & script, int8 broadcast );

	void updateBestBaseApp();

	typedef shared_ptr< BaseApp > BaseAppPtr;

public:
	typedef std::map< Mercury::Address, BaseAppPtr > BaseApps;
	BaseApps & baseApps()	{ return baseApps_; }

private:

	BaseAppID getNextID();

	void sendToBaseApps( const Mercury::InterfaceElement & ifElt,
		MemoryOStream & args, const BaseApp * pExclude = NULL,
		Mercury::ReplyMessageHandler * pHandler = NULL );

	void addWatchers();

	bool calculateOverloaded( bool baseAppsOverloaded );

	CellAppMgr				cellAppMgr_;
	bool					cellAppMgrReady_;
	AnonymousChannelClient	dbMgr_;

	BaseApps 			baseApps_;

	std::auto_ptr< BackupHashChain >	pBackupHashChain_;

	typedef std::map< std::string, std::string > SharedData;
	SharedData 		sharedBaseAppData_; // Authoritative copy
	SharedData 		sharedGlobalData_; // Copy from CellAppMgr

	BaseAppID 		lastBaseAppID_;	//  last id allocated for a BaseApp

	bool			allowNewBaseApps_;

	typedef std::map< std::string, EntityMailBoxRef > GlobalBases;
	GlobalBases globalBases_;

	TimeKeeper *	pTimeKeeper_;
	int				syncTimePeriod_;

	float			baseAppOverloadLevel_;

	Mercury::Address bestBaseAppAddr_;

	bool			isRecovery_;
	bool			hasInitData_;
	bool			hasStarted_;
	bool			shouldShutDownOthers_;

	Mercury::Address	deadBaseAppAddr_;
	unsigned int		archiveCompleteMsgCounter_;

	GameTime		shutDownTime_;
	ShutDownStage	shutDownStage_;

	// Entity creation rate limiting when baseapps are overload
	uint64			baseAppOverloadStartTime_;
	int				loginsSinceOverload_;

	// Whether we've got baseapps on multiple machines
	bool			hasMultipleBaseAppMachines_;

	TimerHandle		gameTimer_;

	friend class CreateBaseReplyHandler;
};

#ifdef CODE_INLINE
#include "baseappmgr.ipp"
#endif

#endif // BASE_APP_MGR_HPP
