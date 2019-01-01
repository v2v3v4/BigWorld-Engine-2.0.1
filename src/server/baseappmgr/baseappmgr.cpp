/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "baseappmgr.hpp"

#ifndef CODE_INLINE
#include "baseappmgr.ipp"
#endif

#include "baseapp.hpp"
#include "baseappmgr_config.hpp"
#include "baseappmgr_interface.hpp"
#include "reply_handlers.hpp"
#include "watcher_forwarding_baseapp.hpp"

#include "cellapp/cellapp_interface.hpp"

#include "cellappmgr/cellappmgr_interface.hpp"

#include "baseapp/baseapp_int_interface.hpp"

#include "loginapp/login_int_interface.hpp"

#include "dbmgr/db_interface.hpp"

#include "common/doc_watcher.hpp"

#include "cstdmf/memory_stream.hpp"
#include "cstdmf/timestamp.hpp"

#include "server/shared_data_type.hpp"

#include "network/channel_sender.hpp"
#include "network/event_dispatcher.hpp"
#include "network/machine_guard.hpp"
#include "network/machined_utils.hpp"
#include "network/nub_exception.hpp"
#include "network/portmap.hpp"
#include "network/watcher_nub.hpp"

#include "server/backup_hash_chain.hpp"
#include "server/bwconfig.hpp"
#include "server/reviver_subject.hpp"
#include "server/stream_helper.hpp"
#include "server/time_keeper.hpp"

#include <limits>

DECLARE_DEBUG_COMPONENT( 0 )

/// BaseAppMgr Singleton.
BW_SINGLETON_STORAGE( BaseAppMgr )

namespace // (anonymous)
{

/**
 *	This function asks the machined process at the destination IP address to
 * 	send a signal to the BigWorld process at the specified port.
 */
bool sendSignalViaMachined( const Mercury::Address& dest, int sigNum )
{
	SignalMessage sm;
	sm.signal_ = sigNum;
	sm.port_ = dest.port;
	sm.param_ = sm.PARAM_USE_PORT;

	Endpoint tempEP;
	tempEP.socket( SOCK_DGRAM );

	if (tempEP.good() && tempEP.bind() == 0)
	{
		sm.sendto( tempEP, htons( PORT_MACHINED ), dest.ip );
		return true;
	}

	return false;
}


} // end namespace (anonymous)


// -----------------------------------------------------------------------------
// Section: Construction/Destruction
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
BaseAppMgr::BaseAppMgr( Mercury::EventDispatcher & mainDispatcher,
		Mercury::NetworkInterface & interface ) :
	ManagerApp( mainDispatcher, interface ),
	cellAppMgr_( interface ),
	cellAppMgrReady_( false ),
	dbMgr_(),
	baseApps_(),
	pBackupHashChain_( new BackupHashChain() ),
	sharedBaseAppData_(),
	sharedGlobalData_(),
	lastBaseAppID_( 0 ),
	allowNewBaseApps_( true ),
	globalBases_(),
	pTimeKeeper_( NULL ),
	syncTimePeriod_( 0 ),
	bestBaseAppAddr_( 0, 0 ),
	isRecovery_( false ),
	hasInitData_( false ),
	hasStarted_( false ),
	shouldShutDownOthers_( false ),
	deadBaseAppAddr_( Mercury::Address::NONE ),
	archiveCompleteMsgCounter_( 0 ),
	shutDownTime_( 0 ),
	shutDownStage_( SHUTDOWN_NONE ),
	baseAppOverloadStartTime_( 0 ),
	loginsSinceOverload_( 0 ),
	hasMultipleBaseAppMachines_( false ),
	gameTimer_()
{
	cellAppMgr_.channel().isLocalRegular( false );
	cellAppMgr_.channel().isRemoteRegular( false );

	INFO_MSG( "Address = %s\n", interface_.address().c_str() );
}


/**
 *	Destructor.
 */
BaseAppMgr::~BaseAppMgr()
{
	gameTimer_.cancel();

	if (shouldShutDownOthers_)
	{
		BaseAppIntInterface::shutDownArgs	baseAppShutDownArgs = { false };

		{
			BaseApps::iterator iter = baseApps_.begin();
			while (iter != baseApps_.end())
			{
				iter->second->bundle() << baseAppShutDownArgs;
				iter->second->send();

				++iter;
			}
		}

		if (cellAppMgr_.channel().isEstablished())
		{
			Mercury::Bundle	& bundle = cellAppMgr_.bundle();
			CellAppMgrInterface::shutDownArgs cellAppmgrShutDownArgs = { false };
			bundle << cellAppmgrShutDownArgs;
			cellAppMgr_.send();
		}
	}

	// Make sure channels shut down cleanly
	interface_.processUntilChannelsEmpty();

	if (pTimeKeeper_)
	{
		delete pTimeKeeper_;
		pTimeKeeper_ = NULL;
	}
}


/**
 *	This method initialises this object.
 *
 *	@return True on success, false otherwise.
 */
bool BaseAppMgr::init( int argc, char * argv[] )
{
	if (!this->ManagerApp::init( argc, argv ))
	{
		return false;
	}

	if (!interface_.isGood())
	{
		ERROR_MSG( "Failed to open internal interface.\n" );
		return false;
	}

	ReviverSubject::instance().init( &interface_, "baseAppMgr" );

	for (int i = 0; i < argc; ++i)
	{
		if (strcmp( argv[i], "-recover" ) == 0)
		{
			isRecovery_ = true;
			break;
		}
	}

	INFO_MSG( "isRecovery = %s\n", isRecovery_ ? "True" : "False" );

	// register dead app callback with machined
	Mercury::MachineDaemon::registerDeathListener( interface_.address(),
				BaseAppMgrInterface::handleBaseAppDeath,
				"BaseAppIntInterface" );

	if (!BW_INIT_ANONYMOUS_CHANNEL_CLIENT( dbMgr_, interface_,
				BaseAppMgrInterface, DBInterface, 0 ))
	{
		INFO_MSG( "BaseAppMgr::init: Database not ready yet.\n" );
	}

	BaseAppMgrInterface::registerWithInterface( interface_ );

	Mercury::Reason reason =
		BaseAppMgrInterface::registerWithMachined( interface_, 0 );

	if (reason != Mercury::REASON_SUCCESS)
	{
		ERROR_MSG( "BaseAppMgr::init: Unable to register. "
				"Is machined running?\n" );
		return false;
	}

	{
		Mercury::MachineDaemon::registerBirthListener( interface_.address(),
			BaseAppMgrInterface::handleCellAppMgrBirth, "CellAppMgrInterface" );

		Mercury::Address cellAppMgrAddr;
		reason = Mercury::MachineDaemon::findInterface(
					"CellAppMgrInterface", 0, cellAppMgrAddr );

		if (reason == Mercury::REASON_SUCCESS)
		{
			cellAppMgr_.addr( cellAppMgrAddr );
		}
		else if (reason == Mercury::REASON_TIMER_EXPIRED)
		{
			INFO_MSG( "BaseAppMgr::init: CellAppMgr not ready yet.\n" );
		}
		else
		{
			ERROR_MSG( "BaseAppMgr::init: "
				"Failed to find CellAppMgr interface: %s\n",
				Mercury::reasonToString( (Mercury::Reason &)reason ) );

			return false;
		}

		Mercury::MachineDaemon::registerBirthListener( interface_.address(),
			BaseAppMgrInterface::handleBaseAppMgrBirth,
			"BaseAppMgrInterface" );
	}

	BW_INIT_WATCHER_DOC( "baseappmgr" );

	BW_REGISTER_WATCHER( 0, "baseappmgr", "Base App Manager", "baseAppMgr",
			mainDispatcher_, interface_.address() );

	this->addWatchers();

	return true;
}


// -----------------------------------------------------------------------------
// Section: Helpers
// -----------------------------------------------------------------------------


/**
 *  This method returns the BaseApp for the given address, or NULL if none
 *  exists.
 */
BaseApp * BaseAppMgr::findBaseApp( const Mercury::Address & addr )
{
	BaseApps::iterator it = baseApps_.find( addr );
	if (it != baseApps_.end())
		return it->second.get();
	else
		return NULL;
}


/**
 *	This method finds the least loaded BaseApp.
 *
 *	@return The least loaded BaseApp. If none exists, NULL is returned.
 */
BaseApp * BaseAppMgr::findBestBaseApp() const
{
	const BaseApp * pBest = NULL;

	float lowestLoad = 0.f;
	BaseAppMgr::BaseApps::const_iterator iter = baseApps_.begin();

	while (iter != baseApps_.end())
	{
		float currLoad = iter->second->load();

		if (!iter->second->isRetiring() && 
				(!pBest || currLoad < lowestLoad))
		{
			lowestLoad = currLoad;
			pBest = iter->second.get();
		}

		++iter;
	}

	return const_cast< BaseApp * >( pBest );
}


/**
 *	This method is called when a BaseApp wants to retire.
 */
void BaseAppMgr::onBaseAppRetire( BaseApp & baseApp )
{
	if (bestBaseAppAddr_ == baseApp.addr())
	{
		this->updateBestBaseApp();
	}

	this->adjustBackupLocations( baseApp.addr(), 
		ADJUST_BACKUP_LOCATIONS_OP_RETIRE );
}


/**
 *	This method updates the best BaseApp for creating bases through
 *	createBaseAnywhere calls. 
 */
void BaseAppMgr::updateBestBaseApp()
{
	BaseApp * pBest = this->findBestBaseApp();

	// TODO:BAR Should consider the possibility that the last app is 
	// now being retired, which will result in pBest == NULL. Maybe
	// retiring the last BaseApp should be a special case where we 
	// trigger a controlled shutdown of the whole server.

	if ((pBest != NULL) &&
			(bestBaseAppAddr_ != pBest->addr()) &&
			cellAppMgr_.channel().isEstablished())
	{
		bestBaseAppAddr_ = pBest->addr();
		Mercury::Bundle & bundle = cellAppMgr_.bundle();
		bundle.startMessage( CellAppMgrInterface::setBaseApp );
		bundle << bestBaseAppAddr_;
		cellAppMgr_.send();
	}
}


/**
 *	This method returns the approximate number of bases on the server.
 */
int BaseAppMgr::numBases() const
{
	int count = 0;

	BaseAppMgr::BaseApps::const_iterator iter = baseApps_.begin();

	while (iter != baseApps_.end())
	{
		count += iter->second->numBases();

		iter++;
	}

	return count;
}


/**
 *	This method returns the approximate number of proxies on the server.
 */
int BaseAppMgr::numProxies() const
{
	int count = 0;

	BaseAppMgr::BaseApps::const_iterator iter = baseApps_.begin();

	while (iter != baseApps_.end())
	{
		count += iter->second->numProxies();

		iter++;
	}

	return count;
}


/**
 *	This method returns the minimum Base App load.
 */
float BaseAppMgr::minBaseAppLoad() const
{
	float load = 2.f;

	BaseAppMgr::BaseApps::const_iterator iter = baseApps_.begin();

	while (iter != baseApps_.end())
	{
		load = std::min( load, iter->second->load() );

		++iter;
	}

	return load;
}


/**
 *	This method returns the average Base App load.
 */
float BaseAppMgr::avgBaseAppLoad() const
{
	float load = 0.f;

	BaseAppMgr::BaseApps::const_iterator iter = baseApps_.begin();

	while (iter != baseApps_.end())
	{
		load += iter->second->load();

		++iter;
	}

	return baseApps_.empty() ? 0.f : load/baseApps_.size();
}


/**
 *	This method returns the maximum Base App load.
 */
float BaseAppMgr::maxBaseAppLoad() const
{
	float load = 0.f;

	BaseAppMgr::BaseApps::const_iterator iter = baseApps_.begin();

	while (iter != baseApps_.end())
	{
		load = std::max( load, iter->second->load() );

		++iter;
	}

	return load;
}


/**
 *	This method returns an ID for a new BaseApp.
 */
BaseAppID BaseAppMgr::getNextID()
{
	// Figure out an ID for it
	bool foundNext = false;

	while (!foundNext)
	{
		lastBaseAppID_ = (lastBaseAppID_+1) & 0x0FFFFFFF; 	// arbitrary limit

		foundNext = true;

		// TODO: Should add back support for making sure that we do not have
		// duplicate IDs. This is not too critical as this is now only really
		// used by the human user to make things easier.

		//foundNext = (baseApps_.find( lastBaseAppID_ ) == baseApps_.end());
	}

	return lastBaseAppID_;
}


/**
 *  This method sends a Mercury message to all known baseapps.  The message
 *  payload is taken from the provided MemoryOStream.  If pExclude is non-NULL,
 *  nothing will be sent to that app.  If pReplyHandler is non-NULL, we start a
 *  request instead of starting a regular message.
 */
void BaseAppMgr::sendToBaseApps( const Mercury::InterfaceElement & ifElt,
	MemoryOStream & args, const BaseApp * pExclude,
	Mercury::ReplyMessageHandler * pHandler )
{
	for (BaseApps::iterator it = baseApps_.begin(); it != baseApps_.end(); ++it)
	{
		BaseApp & baseApp = *it->second;

		// Skip if we're supposed to exclude this app
		if (pExclude == &baseApp)
			continue;

		// Stream message onto bundle and send
		Mercury::Bundle & bundle = baseApp.bundle();

		if (!pHandler)
			bundle.startMessage( ifElt );
		else
			bundle.startRequest( ifElt, pHandler );

		// Note: This does not stream off from "args". This is so that we can
		// read the same data multiple times.
		bundle.addBlob( args.data(), args.size() );

		baseApp.send();
	}

	args.finish();
}


/**
 *	This method adds the watchers that are related to this object.
 */
void BaseAppMgr::addWatchers()
{
	Watcher * pRoot = &Watcher::rootWatcher();

	this->ServerApp::addWatchers( *pRoot );

	// number of local proxies
	MF_WATCH( "numBaseApps", *this, &BaseAppMgr::numBaseApps );

	MF_WATCH( "numBases", *this, &BaseAppMgr::numBases );
	MF_WATCH( "numProxies", *this, &BaseAppMgr::numProxies );

	MF_WATCH( "config/shouldShutDownOthers", shouldShutDownOthers_ );

	MF_WATCH( "baseAppLoad/min", *this, &BaseAppMgr::minBaseAppLoad );
	MF_WATCH( "baseAppLoad/average", *this, &BaseAppMgr::avgBaseAppLoad );
	MF_WATCH( "baseAppLoad/max", *this, &BaseAppMgr::maxBaseAppLoad );

	WatcherPtr pBaseAppWatcher = BaseApp::pWatcher();

	// map of these for locals
	pRoot->addChild( "baseApps", new MapWatcher<BaseApps>( baseApps_ ) );
	pRoot->addChild( "baseApps/*",
			new BaseDereferenceWatcher( pBaseAppWatcher ) );

	// other misc stuff
	MF_WATCH( "lastBaseAppIDAllocated", lastBaseAppID_ );

	pRoot->addChild( "cellAppMgr", Mercury::Channel::pWatcher(),
		&cellAppMgr_.channel() );

	pRoot->addChild( "forwardTo", new BAForwardingWatcher() );
}


/**
 *	This method calculates whether the BaseAppMgr is currently overloaded.
 */
bool BaseAppMgr::calculateOverloaded( bool baseAppsOverloaded )
{
	if (baseAppsOverloaded)
	{
		uint64 overloadTime;

		// Start rate limiting logins
		if (baseAppOverloadStartTime_ == 0)
		{
			baseAppOverloadStartTime_ = timestamp();
		}

		overloadTime = timestamp() - baseAppOverloadStartTime_;
		INFO_MSG( "CellAppMgr::Overloaded for %"PRIu64"ms\n",
			overloadTime/(stampsPerSecond()/1000) );

		if ((overloadTime > Config::overloadTolerancePeriodInStamps()) ||
			(loginsSinceOverload_ >= Config::overloadLogins()))
		{
			return true;
		}
		else
		{
			// If we're not overloaded
			loginsSinceOverload_++;

			INFO_MSG( "BaseAppMgr::Logins since overloaded " \
					"(allowing max of %d): %d\n",
				Config::overloadLogins(),
				loginsSinceOverload_ );
		}
	}
	else
	{
		// Not overloaded, clear the timer
		baseAppOverloadStartTime_ = 0;
		loginsSinceOverload_ = 0;
	}

	return false;
}


/**
 *	This method overrides the TimerHandler method to handle timer events.
 */
void BaseAppMgr::handleTimeout( TimerHandle /*handle*/, void * arg )
{
	// Are we paused for shutdown?
	if ((shutDownTime_ != 0) && (shutDownTime_ == time_))
		return;

	switch (reinterpret_cast<uintptr>( arg ))
	{
		case TIMEOUT_GAME_TICK:
		{
			++time_;

			if (time_ % Config::timeSyncPeriodInTicks() == 0)
			{
				pTimeKeeper_->synchroniseWithMaster();
			}

			this->checkForDeadBaseApps();

			if (time_ % Config::updateCreateBaseInfoPeriodInTicks() == 0)
			{
				this->updateCreateBaseInfo();
			}

			// TODO: Don't really need to do this each tick.
			this->updateBestBaseApp();
		}
		break;
	}
}


/**
 *	This method is called periodically to check whether or not any base
 *	applications have timed out.
 */
void BaseAppMgr::checkForDeadBaseApps()
{
	uint64 currTime = ::timestamp();
	uint64 lastHeardTime = 0;
	BaseApps::iterator iter = baseApps_.begin();

	while (iter != baseApps_.end())
	{
		lastHeardTime = std::max( lastHeardTime,
				iter->second->channel().lastReceivedTime() );
		++iter;
	}

	const uint64 timeSinceAnyHeard = currTime - lastHeardTime;

	iter = baseApps_.begin();

	while (iter != baseApps_.end())
	{
		BaseApp * pApp = iter->second.get();

		if (pApp->hasTimedOut( currTime,
								Config::baseAppTimeoutInStamps(),
								timeSinceAnyHeard ))
		{
			INFO_MSG( "BaseAppMgr::checkForDeadBaseApps: %s has timed out.\n",
					pApp->addr().c_str() );
			this->handleBaseAppDeath( pApp->addr() );
			// Only handle one timeout per check because the above call will
			// likely change the collection we are iterating over.
			return;
		}

		iter++;
	}
}


/**
 *	This method handles a message from a Base App that informs us on its current
 *	load.
 */
void BaseAppMgr::informOfLoad( const Mercury::Address & addr,
		const Mercury::UnpackedMessageHeader & header,
		const BaseAppMgrInterface::informOfLoadArgs & args )
{
	BaseApps::iterator iter = baseApps_.find( addr );

	if (iter != baseApps_.end())
	{
		iter->second->updateLoad( args.load, args.numBases, args.numProxies );
	}
	else
	{
		ERROR_MSG( "BaseAppMgr::informOfLoad: No BaseApp with address %s\n",
				addr.c_str() );
	}
}


// -----------------------------------------------------------------------------
// Section: Handler methods
// -----------------------------------------------------------------------------

/**
 *	This method handles the createEntity message. It is called by DBMgr when
 *	logging in.
 */
void BaseAppMgr::createEntity( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data )
{
	Mercury::Address baseAppAddr( 0, 0 );

	BaseApp * pBest = this->findBestBaseApp();

	if (pBest == NULL)
	{
		ERROR_MSG( "BaseAppMgr::createEntity: Could not find a BaseApp.\n");
		baseAppAddr.port =
				BaseAppMgrInterface::CREATE_ENTITY_ERROR_NO_BASEAPPS;

		Mercury::ChannelSender sender( BaseAppMgr::getChannel( srcAddr ) );
		Mercury::Bundle & bundle = sender.bundle();

		bundle.startReply( header.replyID );
		bundle << baseAppAddr;
		bundle << "No BaseApp could be found to add to.";

		return;
	}

	bool baseAppsOverloaded = (pBest->load() > Config::baseAppOverloadLevel());

	if (this->calculateOverloaded( baseAppsOverloaded ))
	{
		INFO_MSG( "BaseAppMgr::createEntity: All baseapps overloaded "
				"(best load=%.02f > overload level=%.02f.\n",
			pBest->load(), Config::baseAppOverloadLevel() );
		baseAppAddr.port =
			BaseAppMgrInterface::CREATE_ENTITY_ERROR_BASEAPPS_OVERLOADED;

		Mercury::ChannelSender sender( BaseAppMgr::getChannel( srcAddr ) );
		Mercury::Bundle & bundle = sender.bundle();

		bundle.startReply( header.replyID );
		bundle << baseAppAddr;
		bundle << "All BaseApps overloaded.";

		return;
	}

	// Copy the client endpoint address
	baseAppAddr = pBest->externalAddr();

	CreateBaseReplyHandler * pHandler =
		new CreateBaseReplyHandler( srcAddr, header.replyID,
			baseAppAddr );

	// Tell the BaseApp about the client's new proxy
	Mercury::Bundle	& bundle = pBest->bundle();
	bundle.startRequest( BaseAppIntInterface::createBaseWithCellData,
			pHandler );

	bundle.transfer( data, data.remainingLength() );
	pBest->send();

	// Update the load estimate.
	pBest->addEntity();
}


/**
 *	This method handles an add message from a BaseApp. It returns the new ID
 *	that the BaseApp has.
 */
void BaseAppMgr::add( const Mercury::Address & srcAddr,
	const Mercury::UnpackedMessageHeader & header,
	const BaseAppMgrInterface::addArgs & args )
{
	const Mercury::ReplyID replyID = header.replyID;

	MF_ASSERT( srcAddr == args.addrForCells );

	// If we're not allowing BaseApps to connect at the moment, just send back a
	// zero-length reply.
	if (!cellAppMgr_.channel().isEstablished() || !hasInitData_)
	{
		INFO_MSG( "BaseAppMgr::add: Not allowing BaseApp at %s to register "
				"yet\n", srcAddr.c_str() );

		Mercury::ChannelSender sender( BaseAppMgr::getChannel( srcAddr ) );
		sender.bundle().startReply( replyID );

		return;
	}

//	TRACE_MSG( "BaseAppMgr::add:\n" );
	if (!allowNewBaseApps_ || (shutDownStage_ != SHUTDOWN_NONE))
		return;	// just let it time out

	// Let the Cell App Manager know about the first base app. This is so that
	// the cell app can know about a base app. We will probably improve this
	// later.
	if (baseApps_.empty())
	{
		Mercury::Bundle	& bundle = cellAppMgr_.bundle();
		CellAppMgrInterface::setBaseAppArgs setBaseAppArgs;
		setBaseAppArgs.addr = args.addrForCells;
		bundle << setBaseAppArgs;
		cellAppMgr_.send();

		bestBaseAppAddr_ = args.addrForCells;
	}

	// Add it to our list of BaseApps
	BaseAppID id = this->getNextID();
	BaseAppPtr pBaseApp(
		new BaseApp( *this, args.addrForCells, args.addrForClients, id ) );
	baseApps_[ srcAddr ] = pBaseApp;

	DEBUG_MSG( "BaseAppMgr::add:\n"
			"\tAllocated id     = %u\n"
			"\tBaseApps in use  = %"PRIzu"\n"
			"\tInternal address = %s\n"
			"\tExternal address = %s\n",
		id,
		baseApps_.size(),
		pBaseApp->addr().c_str(),
		pBaseApp->externalAddr().c_str() );

	// Stream on the reply
	Mercury::Bundle & bundle = pBaseApp->bundle();
	bundle.startReply( replyID );

	BaseAppInitData initData;
	initData.id = id;
	initData.time = time_;
	initData.isReady = hasStarted_;

	bundle << initData;

	// Now stream on globals as necessary
	if (!globalBases_.empty())
	{
		GlobalBases::iterator iter = globalBases_.begin();

		while (iter != globalBases_.end())
		{
			bundle.startMessage( BaseAppIntInterface::addGlobalBase );
			bundle << iter->first << iter->second;

			++iter;
		}
	}

	if (!sharedBaseAppData_.empty())
	{
		SharedData::iterator iter = sharedBaseAppData_.begin();

		while (iter != sharedBaseAppData_.end())
		{
			bundle.startMessage( BaseAppIntInterface::setSharedData );
			bundle << SharedDataType( SHARED_DATA_TYPE_BASE_APP ) <<
				iter->first << iter->second;
			++iter;
		}
	}

	if (!sharedGlobalData_.empty())
	{
		SharedData::iterator iter = sharedGlobalData_.begin();

		while (iter != sharedGlobalData_.end())
		{
			bundle.startMessage( BaseAppIntInterface::setSharedData );
			bundle << SharedDataType( SHARED_DATA_TYPE_GLOBAL ) <<
				iter->first << iter->second;
			++iter;
		}
	}

	BaseApps::iterator iter;
	for (iter = baseApps_.begin(); iter != baseApps_.end(); ++iter)
	{
		BaseAppPtr pOtherBaseApp = iter->second;
		if (pOtherBaseApp != pBaseApp)
		{
			bundle.startMessage( BaseAppIntInterface::handleBaseAppBirth );
			bundle << iter->first << pOtherBaseApp->externalAddr();

			pOtherBaseApp->bundle().startMessage( BaseAppIntInterface::handleBaseAppBirth );
			pOtherBaseApp->bundle() << pBaseApp->addr() << pBaseApp->externalAddr();
			pOtherBaseApp->send();
		}
	}

	// This sends a bundle and so must be after initial send.
	this->adjustBackupLocations( pBaseApp->addr(), 
		ADJUST_BACKUP_LOCATIONS_OP_ADD );

	pBaseApp->send();
}


namespace
{
	// Used to sort the BaseApps.
	// TODO: Currently, this is only using the reported CPU load. We probably
	// want to consider the number of proxy and base entities on the machine.
	template <class T>
	struct loadCmp
	{
		inline bool operator()( T * p1, T * p2 )
		{
			return p1->load() < p2->load();
		}
	};
}


/**
 *	This method updates information on the BaseApps about which other BaseApps
 *	they should create base entities on.
 */
void BaseAppMgr::updateCreateBaseInfo()
{
	// Description of createBaseAnywhere scheme:
	// Currently/Initially, a very simple scheme is being implemented. This may
	// be modified with some additional ideas if it is not effective enough.
	// This initial scheme is quite simple but it may be enough. A lot of the
	// balancing occurs from players logging in and out. These are always added
	// to the least loaded BaseApp.
	//
	// The basic scheme is that each BaseApp has a BaseApp assigned to it where
	// it should create Base entities. Only some of the BaseApps are destination
	// BaseApps.
	//
	// There are two configuration options createBaseRatio and
	// updateCreateBaseInfoPeriod. The createBaseRatio is the number of BaseApps
	// that a destination BaseApp will have pointing to it. For example, if this
	// createBaseRatio is 4, the least loaded quarter of the machines will each
	// have 4 BaseApps choosing them as the destination to create Base entities.
	//
	// updateCreateBaseInfoPeriod controls how often this information is
	// updated.
	//
	// Possible additions:
	// A situation that the initial scheme does not handle too well is when a
	// new BaseApp is added to a system that has a lot of heavily loaded
	// BaseApps. This unloaded BaseApp is consider equal to the other heavily
	// loaded BaseApps that are still members of the destination set. It may be
	// good enough that this fixes itself eventually as the loaded BaseApps come
	// in and out of the destination set. The logging in of players would also
	// help the situation.
	//
	// The BaseApps only know about one other BaseApp. We could let them know
	// about a number and they could create base entities on these randomly,
	// perhaps based on load. They could also create the base entities locally
	// if they are currently underloaded.
	//
	// Currently, members of the destination set are all considered equal. We
	// could consider their load in deciding how many BaseApps should have them
	// as a destination.
	//
	// Instead of this information being updated to all BaseApps at a regular
	// period, this information could be updated as needed. The BaseApps could
	// be kept sorted and the destination set updated as the BaseApp loads
	// changed. Only some BaseApps would need to be updated.


	// Get all of the BaseApps in a vector.
	// TODO: Could consider maintaining this vector as an optimisation.
	typedef std::vector< BaseApp * > BaseAppsVec;
	BaseAppsVec apps;
	apps.reserve( baseApps_.size() );

	BaseApps::iterator iter = baseApps_.begin();

	while (iter != baseApps_.end())
	{
		apps.push_back( iter->second.get() );
		++iter;
	}

	// Here the BaseApps are sorted so that we can find the least loaded
	// BaseApps. It does not really need to be completely sorted but it's
	// easy for now.
	std::sort( apps.begin(), apps.end(), loadCmp<BaseApp>() );

	int totalSize = apps.size();
	int destSize = int(totalSize/Config::createBaseRatio() + 0.99f);
	destSize = std::min( totalSize, std::max( 1, destSize ) );

	// Randomly shuffle so that the BaseApps are assigned to a random
	// destination BaseApp. It is probably good to have this randomisation to
	// help avoid degenerate cases.
	std::random_shuffle( apps.begin() + destSize, apps.end() );

	// Send this information to the BaseApps.
	for (size_t i = 0; i < apps.size(); ++i)
	{
		Mercury::Bundle & bundle = apps[ i ]->bundle();
		int destIndex = i % destSize;

		bundle.startMessage( BaseAppIntInterface::setCreateBaseInfo );
		bundle << apps[ destIndex ]->addr();

		apps[ i ]->send();
	}
}


/**
 *	This method is called to inform this BaseAppMgr about a base app during
 *	recovery from the death of an old BaseAppMgr.
 */
void BaseAppMgr::recoverBaseApp( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & /*header*/,
			BinaryIStream & data )
{
	if (!isRecovery_)
	{
		WARNING_MSG( "BaseAppMgr::recoverBaseApp: "
				"Recovering when we were not started with -recover\n" );
		isRecovery_ = true;
	}

	Mercury::Address		addrForCells;
	Mercury::Address		addrForClients;
	BaseAppID				id;

	data >> addrForCells >> addrForClients >> id >> time_;

	// hasStarted_ = true;
	this->startTimer();

	DEBUG_MSG( "BaseAppMgr::recoverBaseApp: %s, id = %u\n",
		addrForCells.c_str(), id );

	lastBaseAppID_ = std::max( id, lastBaseAppID_ );

	if (baseApps_.find( addrForCells ) != baseApps_.end())
	{
		ERROR_MSG( "BaseAppMgr::recoverBaseApp: "
				"Already know about BaseApp at %s\n", addrForCells.c_str() );
		return;
	}

	BaseAppPtr pBaseApp( new BaseApp( *this, addrForCells, addrForClients, 
		id ) );
	baseApps_[ addrForCells ] = pBaseApp;

	data >> pBaseApp->backupHash() >> pBaseApp->newBackupHash();

	// Read all of the shared BaseApp data
	{
		uint32 numEntries;
		data >> numEntries;

		std::string key;
		std::string value;

		for (uint32 i = 0; i < numEntries; ++i)
		{
			data >> key >> value;
			sharedBaseAppData_[ key ] = value;
		}
	}

	// TODO: This is mildly dodgy. It's getting its information from the
	// BaseApps but would probably be more accurate if it came from the
	// CellAppMgr. It may clobber a valid change that has been made by the
	// CellAppMgr.

	// Read all of the shared Global data
	{
		uint32 numEntries;
		data >> numEntries;

		std::string key;
		std::string value;

		for (uint32 i = 0; i < numEntries; ++i)
		{
			data >> key >> value;
			sharedGlobalData_[ key ] = value;
		}
	}

	while (data.remainingLength() > 0)
	{
		std::pair< std::string, EntityMailBoxRef > value;
		data >> value.first >> value.second;

		MF_ASSERT( value.second.addr == srcAddr );

		if (!globalBases_.insert( value ).second)
		{
			WARNING_MSG( "BaseAppMgr::recoverBaseApp: "
					"Try to recover global base %s twice\n",
				value.first.c_str() );
		}
	}
}


/**
 *	This method handles the message from a BaseApp that it wants to be deleted.
 */
void BaseAppMgr::del( const Mercury::Address & addr,
		const Mercury::UnpackedMessageHeader & header,
		const BaseAppMgrInterface::delArgs & args )
{
	TRACE_MSG( "BaseAppMgr::del: %u\n", args.id );

	// First try real Base Apps.

	if (this->onBaseAppDeath( addr ))
	{
		DEBUG_MSG( "BaseAppMgr::del: now have %"PRIzu" base apps\n",
				baseApps_.size() );
	}
	else
	{
		ERROR_MSG( "BaseAppMgr: Error deleting %s id = %u\n",
			addr.c_str(), args.id );
	}
}


/**
 *	This method adjusts who each BaseApp is backing up to. It is called whenever
 *	BaseApp is added or removed.
 *
 *	This is used by the new-style backup.
 */
void BaseAppMgr::adjustBackupLocations( const Mercury::Address & addr,
		AdjustBackupLocationsOp op )
{
	// The current scheme is that every BaseApp backs up to every other BaseApp.
	// Ideas for improvement:
	// - May want to cap the number of BaseApps that a BaseApp backs up to.
	// - May want to limit how much the hash changes backups. Currently, all old
	//    backups are discarded but if an incremental hash is used, the amount
	//    of lost backup information can be reduced.
	// - Incremental hash could be something like the following: when we have a
	//    non power-of-2 number of backups, we assume that some previous ones
	//    are repeated to always get a power of 2.
	//    Let n be number of buckets and N be next biggest power of 2.
	//    bucket = hash % N
	//    if bucket >= n:
	//      bucket -= N/2;
	//    When another bucket is added, an original bucket that was managing two
	//    virtual buckets now splits the load with the new bucket. When a bucket
	//    is removed, a bucket that was previously managing one virtual bucket
	//    now handles two.
	BaseApp * pBaseApp = NULL;

	if (op == ADJUST_BACKUP_LOCATIONS_OP_ADD)
	{
		pBaseApp = baseApps_[ addr ].get();
		MF_ASSERT( pBaseApp );
	}

	BaseApps::iterator iter = baseApps_.begin();

	bool hadMultipleBaseAppMachines = hasMultipleBaseAppMachines_;

	hasMultipleBaseAppMachines_ = false;

	// We check if everything is on the same machine
	while (iter != baseApps_.end())
	{
		if (baseApps_.begin()->first.ip != iter->first.ip)
		{
			hasMultipleBaseAppMachines_ = true;
			break;
		}
		++iter;
	}

	if (hasMultipleBaseAppMachines_ && !hadMultipleBaseAppMachines)
	{
		INFO_MSG( "Baseapps detected on multiple machines, switching to "
				  "multi-machine backup strategy.\n" );
	}

	if (!hasMultipleBaseAppMachines_ && hadMultipleBaseAppMachines)
	{
		INFO_MSG( "Baseapps detected on only one machine, falling back to "
				  "single-machine backup strategy.\n" );
	}

	iter = baseApps_.begin();

	while (iter != baseApps_.end())
	{
		if (addr != iter->first && !iter->second->isOffloading())
		{
			BaseApp & baseApp = *iter->second;

			if (baseApp.newBackupHash().empty())
			{
				baseApp.newBackupHash() = baseApp.backupHash();
			}
			else
			{
				// Stay with the previous newBackupHash
				WARNING_MSG( "BaseAppMgr::adjustBackupLocations: "
							"%s was still transitioning to a new hash.\n",
						iter->first.c_str() );
			}

			if (hasMultipleBaseAppMachines_ && !hadMultipleBaseAppMachines)
			{
				// If backing up to was allowed previously, we can assume that
				// it was because there was no good places to backup
				MF_ASSERT( op == ADJUST_BACKUP_LOCATIONS_OP_ADD );
				baseApp.newBackupHash().clear();
			}
			else if (!hasMultipleBaseAppMachines_ && hadMultipleBaseAppMachines)
			{
				// If backing up to the same machine was prohibited previously,
				// make a fully connected set
				BaseApps::iterator inner = baseApps_.begin();

				while (inner != baseApps_.end())
				{
					if ((inner != iter) && (inner->first != addr) && 
						inner->second->isRetiring())
					{
						baseApp.newBackupHash().push_back( inner->first );
					}
					++inner;
				}
			}

			switch (op)
			{
			case ADJUST_BACKUP_LOCATIONS_OP_ADD:
				if ((addr.ip != iter->first.ip) || 
						!hasMultipleBaseAppMachines_)
				{
					baseApp.newBackupHash().push_back( addr );
					// Do not back up to retiring baseapps.
					if (!baseApp.isRetiring())
					{
						pBaseApp->newBackupHash().push_back( iter->first );
					}
				}
				break;

			case ADJUST_BACKUP_LOCATIONS_OP_CRASH:
				// Could use a find() function, but none currently exists

				if (baseApp.backupHash().erase( addr ))
				{
					// The current backup is no longer valid.
					baseApp.backupHash().clear();
				}
				// Fall through

			case ADJUST_BACKUP_LOCATIONS_OP_RETIRE:
				baseApp.newBackupHash().erase( addr );
				break;

			default:
				CRITICAL_MSG( "BaseAppMgr::adjustBackupLocations: "
					"Invalid operation type: %d", op );
				break;
			}

			Mercury::Bundle & bundle = baseApp.bundle();
			bundle.startMessage( BaseAppIntInterface::setBackupBaseApps );
			bundle << baseApp.newBackupHash();
			baseApp.send();
		}

		++iter;
	}

	if (op == ADJUST_BACKUP_LOCATIONS_OP_ADD)
	{
		Mercury::Bundle & bundle = pBaseApp->bundle();
		bundle.startMessage( BaseAppIntInterface::setBackupBaseApps );
		bundle << pBaseApp->newBackupHash();
		pBaseApp->send();
	}
}

/**
 *	This method checks and handles the case where a BaseApp may have stopped.
 */
bool BaseAppMgr::onBaseAppDeath( const Mercury::Address & addr )
{
	BaseApps::iterator iter = baseApps_.find( addr );

	if (iter == baseApps_.end())
	{
		return false;
	}

	INFO_MSG( "BaseAppMgr::onBaseAppDeath: baseapp%02d @ %s\n",
		iter->second->id(), addr.c_str() );

	BaseApp & baseApp = *iter->second;
	bool controlledShutDown = false;

	if (Config::hardKillDeadBaseApps())
	{
		// Make sure it's really dead, otherwise backup will have
		// trouble taking over its address.
		INFO_MSG( "BaseAppMgr::onBaseAppDeath: Sending SIGQUIT to %s\n",
				baseApp.addr().c_str() );
		if (!sendSignalViaMachined( baseApp.addr(), SIGQUIT ))
		{
			ERROR_MSG( "BaseAppMgr::onBaseAppDeath: Failed to send "
					"SIGQUIT to %s\n", baseApp.addr().c_str() );
		}
	}

	if (Config::shutDownServerOnBaseAppDeath())
	{
		controlledShutDown = true;
		NOTICE_MSG( "BaseAppMgr::onBaseAppDeath: "
				"shutDownServerOnBaseAppDeath is enabled. "
				"Shutting down server\n" );
	}
	else if (baseApp.backupHash().empty())
	{
		// TODO: What should be done if there is no backup or it's not
		// yet ready.
		if (baseApp.newBackupHash().empty())
		{
			ERROR_MSG( "BaseAppMgr::onBackupBaseAppDeath: "
					"No backup for %s\n", addr.c_str() );
		}
		else
		{
			ERROR_MSG( "BaseAppMgr::onBackupBaseAppDeath: "
					"Backup not ready for %s\n", addr.c_str() );
		}

		if (Config::shutDownServerOnBadState())
		{
			controlledShutDown = true;
		}
	}

	pBackupHashChain_->adjustForDeadBaseApp( addr, baseApp.backupHash() );

	{
		Mercury::Bundle & bundle = cellAppMgr_.bundle();
		bundle.startMessage(
				CellAppMgrInterface::handleBaseAppDeath );
		bundle << addr;
		bundle << baseApp.backupHash();
		cellAppMgr_.send();
	}

	// Tell all other baseapps that the dead one is gone.
	unsigned int numBaseAppsAlive = baseApps_.size() - 1;
	if (numBaseAppsAlive > 0 && !controlledShutDown)
	{
		MemoryOStream args;
		args << addr << baseApp.backupHash();

		this->sendToBaseApps(
			BaseAppIntInterface::handleBaseAppDeath, args, &baseApp );

		deadBaseAppAddr_ = addr;
		archiveCompleteMsgCounter_ = numBaseAppsAlive;
	}

	// Adjust globalBases_ for new mapping
	{
		GlobalBases::iterator gbIter = globalBases_.begin();

		while (gbIter != globalBases_.end())
		{
			EntityMailBoxRef & mailbox = gbIter->second;

			if (mailbox.addr == addr)
			{
				Mercury::Address newAddr =
					baseApp.backupHash().addressFor( mailbox.id );
				mailbox.addr.ip = newAddr.ip;
				mailbox.addr.port = newAddr.port;
			}

			++gbIter;
		}
	}

	baseApps_.erase( iter );

	this->adjustBackupLocations( addr, ADJUST_BACKUP_LOCATIONS_OP_CRASH );

	iter = baseApps_.begin();
	while (iter != baseApps_.end())
	{
		iter->second->stopBackup( addr );

		++iter;
	}

	if (controlledShutDown)
	{
		this->controlledShutDownServer();
	}
	else
	{
		this->updateCreateBaseInfo();
	}

	return true;
}


/**
 *	This method handles a BaseApp finishing its controlled shutdown.
 */
void BaseAppMgr::removeControlledShutdownBaseApp(
		const Mercury::Address & addr )
{
	TRACE_MSG( "BaseAppMgr::removeControlledShutdownBaseApp: %s\n",
			addr.c_str() );

	baseApps_.erase( addr );
}


/**
 *	This method shuts down this process.
 */
void BaseAppMgr::shutDown( bool shutDownOthers )
{
	INFO_MSG( "BaseAppMgr::shutDown: shutDownOthers = %d\n",
			shutDownOthers );
	shouldShutDownOthers_ = shutDownOthers;
	mainDispatcher_.breakProcessing();
}


/**
 *	This method responds to a shutDown message.
 */
void BaseAppMgr::shutDown( const BaseAppMgrInterface::shutDownArgs & args )
{
	this->shutDown( args.shouldShutDownOthers );
}


/**
 *	This method responds to a message telling us what stage of the controlled
 *	shutdown process the server is at.
 */
void BaseAppMgr::controlledShutDown(
		const BaseAppMgrInterface::controlledShutDownArgs & args )
{
	INFO_MSG( "BaseAppMgr::controlledShutDown: stage = %s\n", 
		ServerApp::shutDownStageToString( args.stage ) );

	switch (args.stage)
	{
		case SHUTDOWN_REQUEST:
		{
			Mercury::Bundle & bundle = cellAppMgr_.bundle();
			CellAppMgrInterface::controlledShutDownArgs args;
			args.stage = SHUTDOWN_REQUEST;
			bundle << args;
			cellAppMgr_.send();
			break;
		}

		case SHUTDOWN_INFORM:
		{
			shutDownStage_ = args.stage;
			shutDownTime_ = args.shutDownTime;

			// Inform all base apps.
			{
				SyncControlledShutDownHandler * pHandler =
					new SyncControlledShutDownHandler( args.stage,
							baseApps_.size() );

				// Inform normal base apps.
				MemoryOStream payload;
				payload << args.stage << args.shutDownTime;
				this->sendToBaseApps( BaseAppIntInterface::controlledShutDown,
					payload, NULL, pHandler );
			}

			break;
		}

		case SHUTDOWN_PERFORM:
		{
			this->startAsyncShutDownStage( SHUTDOWN_DISCONNECT_PROXIES );
			break;
		}

		case SHUTDOWN_TRIGGER:
		{
			this->controlledShutDownServer();
			break;
		}

		case SHUTDOWN_NONE:
		case SHUTDOWN_DISCONNECT_PROXIES:
			break;
	}
}


/**
 *
 */
void BaseAppMgr::startAsyncShutDownStage( ShutDownStage stage )
{
	std::vector< Mercury::Address > addrs;
	addrs.reserve( baseApps_.size() );

	BaseApps::iterator iter = baseApps_.begin();

	while (iter != baseApps_.end())
	{
		addrs.push_back( iter->first );

		++iter;
	}

	// This object deletes itself.
	new AsyncControlledShutDownHandler( stage, addrs );
}


/**
 *  Trigger a controlled shutdown of the entire server.
 */
void BaseAppMgr::controlledShutDownServer()
{
	// First try to trigger controlled shutdown via the loginapp
	Mercury::Address loginAppAddr;
	Mercury::Reason reason = Mercury::MachineDaemon::findInterface(
				"LoginIntInterface", -1, loginAppAddr );

	if (reason == Mercury::REASON_SUCCESS)
	{
		Mercury::ChannelSender sender( BaseAppMgr::getChannel( loginAppAddr ) );
		Mercury::Bundle & bundle = sender.bundle();

		bundle.startMessage( LoginIntInterface::controlledShutDown );

		INFO_MSG( "BaseAppMgr::controlledShutDownServer: "
			"Triggering server shutdown via LoginApp @ %s\n",
			loginAppAddr.c_str() );

		return;
	}
	else
	{
		ERROR_MSG( "BaseAppMgr::controlledShutDownServer: "
			"Couldn't find a LoginApp to trigger server shutdown\n" );
	}

	// Next try to trigger shutdown via the DBMgr
	if (this->dbMgr().channel().isEstablished())
	{
		Mercury::Bundle & bundle = this->dbMgr().bundle();
		DBInterface::controlledShutDownArgs::start( bundle ).stage =
			SHUTDOWN_REQUEST;
		this->dbMgr().send();

		INFO_MSG( "BaseAppMgr::controlledShutDownServer: "
				"Triggering server shutdown via DBMgr\n" );
		return;
	}
	else
	{
		ERROR_MSG( "BaseAppMgr::controlledShutDownServer: "
			"Couldn't find the DBMgr to trigger server shutdown\n" );
	}

	// Alright, the shutdown starts with me then
	BaseAppMgrInterface::controlledShutDownArgs args;
	args.stage = SHUTDOWN_REQUEST;
	INFO_MSG( "BaseAppMgr::controlledShutDownServer: "
		"Starting controlled shutdown here (no LoginApp or DBMgr found)\n" );
	this->controlledShutDown( args );
}

/**
 *	This method replies whether if the server has been started.
 */
void BaseAppMgr::requestHasStarted( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader& header,
		BinaryIStream & data )
{
	Mercury::ChannelSender sender( BaseAppMgr::getChannel( srcAddr ) );
	Mercury::Bundle & bundle = sender.bundle();

	bundle.startReply( header.replyID );
	bundle << hasStarted_;

	return;
}

/**
 *	This method processes the initialisation data from DBMgr.
 */
void BaseAppMgr::initData( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data )
{
	if (hasInitData_)
	{
		ERROR_MSG( "BaseAppMgr::initData: Ignored subsequent initialisation "
				"data from %s\n", addr.c_str() );
		return;
	}

	// Save DBMgr config and time for BaseApps
	GameTime gameTime;
	data >> gameTime;
	if (time_ == 0)
	{
		// __kyl__(12/8/2008) XML database always sends 0 as the game time.
		time_ = gameTime;
		INFO_MSG( "BaseAppMgr::initData: game time=%.1f\n",
				this->gameTimeInSeconds() );
	}
	// else
		// Recovery case. We should be getting the game time from BaseApps.

	int32	maxAppID;
	data >> maxAppID;
	if (maxAppID > lastBaseAppID_)
	{
		// __kyl__(12/8/2008) XML database always sends 0 as the max app ID.
		lastBaseAppID_ = maxAppID;
		INFO_MSG( "BaseAppMgr::initData: lastBaseAppIDAllocated=%d\n",
				lastBaseAppID_ );
	}

	hasInitData_ = true;
}

/**
 *	This method processes a message from the DBMgr that restores the spaces
 * 	(and space data). This comes via the BaseAppMgr mainly because
 * 	DBMgr doesn't have a channel to CellAppMgr and also because BaseAppMgr
 * 	tells DBMgr when to "start" the system.
 */
void BaseAppMgr::spaceDataRestore( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data )
{
	MF_ASSERT( !hasStarted_ && hasInitData_ );

	// Send spaces information to CellAppMgr
	{
		Mercury::Bundle & bundle = cellAppMgr_.bundle();
		bundle.startMessage( CellAppMgrInterface::prepareForRestoreFromDB );
		bundle << time_;
		bundle.transfer( data, data.remainingLength() );
		cellAppMgr_.send();
	}
}


/**
 *	This method handles a message to set a shared data value. This may be
 *	data that is shared between all BaseApps or all BaseApps and CellApps. The
 *	BaseAppMgr is the authoritative copy of BaseApp data but the CellAppMgr is
 *	the authoritative copy of global data (i.e. data shared between all BaseApps
 *	and all CellApps).
 */
void BaseAppMgr::setSharedData( BinaryIStream & data )
{
	bool sendToBaseApps = true;
	SharedDataType dataType;
	std::string key;
	std::string value;
	data >> dataType >> key >> value;

	if (dataType == SHARED_DATA_TYPE_BASE_APP)
	{
		sharedBaseAppData_[ key ] = value;
	}
	else if (dataType == SHARED_DATA_TYPE_GLOBAL)
	{
		sharedGlobalData_[ key ] = value;
	}
	else if ((dataType == SHARED_DATA_TYPE_GLOBAL_FROM_BASE_APP) ||
		(dataType == SHARED_DATA_TYPE_CELL_APP))
	{
		if (dataType == SHARED_DATA_TYPE_GLOBAL_FROM_BASE_APP)
		{
			dataType = SHARED_DATA_TYPE_GLOBAL;
		}

		// Because BaseApps don't have channels to the CellAppMgr
		// we will forward these messages on its behalf
		Mercury::Bundle & bundle = cellAppMgr_.bundle();
		bundle.startMessage( CellAppMgrInterface::setSharedData );
		bundle << dataType << key << value;
		cellAppMgr_.send();

		// Make sure we don't tell the BaseApps about this yet, wait
		// for CellAppMgr to notify us.
		sendToBaseApps = false;
	}
	else
	{
		ERROR_MSG( "BaseAppMgr::setSharedData: Invalid dataType %d\n",
				dataType );
		return;
	}

	if (sendToBaseApps)
	{
		MemoryOStream payload;
		payload << dataType << key << value;

		this->sendToBaseApps( BaseAppIntInterface::setSharedData, payload );
	}
}


/**
 *	This method handles a message to delete a shared data value. This may be
 *	data that is shared between all BaseApps or all BaseApps and CellApps. The
 *	BaseAppMgr is the authoritative copy of BaseApp data but the CellAppMgr is
 *	the authoritative copy of global data (i.e. data shared between all BaseApps
 *	and all CellApps).
 */
void BaseAppMgr::delSharedData( BinaryIStream & data )
{
	bool sendToBaseApps = true;
	SharedDataType dataType;
	std::string key;
	data >> dataType >> key;

	if (dataType == SHARED_DATA_TYPE_BASE_APP)
	{
		sharedBaseAppData_.erase( key );
	}
	else if (dataType == SHARED_DATA_TYPE_GLOBAL)
	{
		sharedGlobalData_.erase( key );
	}
	else if ((dataType == SHARED_DATA_TYPE_GLOBAL_FROM_BASE_APP) ||
		(dataType == SHARED_DATA_TYPE_CELL_APP))
	{
		if (dataType == SHARED_DATA_TYPE_GLOBAL_FROM_BASE_APP)
		{
			dataType = SHARED_DATA_TYPE_GLOBAL;
		}

		// Because BaseApps don't have channels to the CellAppMgr
		// we will forward these messages on its behalf
		Mercury::Bundle & bundle = cellAppMgr_.bundle();
		bundle.startMessage( CellAppMgrInterface::delSharedData );
		bundle << dataType << key;
		cellAppMgr_.send();

		// Make sure we don't tell the BaseApps about this yet, wait
		// for CellAppMgr to notify us.
		sendToBaseApps = false;
	}
	else
	{
		ERROR_MSG( "BaseAppMgr::delSharedData: Invalid dataType %d\n",
				dataType );
		return;
	}

	MemoryOStream payload;
	payload << dataType << key;

	if (sendToBaseApps)
	{
		this->sendToBaseApps( BaseAppIntInterface::delSharedData, payload );
	}
}


/**
 *	This class is used to handle the changes to the hash once new hash has been
 *	primed.
 */
class FinishSetBackupDiffVisitor : public BackupHash::DiffVisitor
{
public:
	FinishSetBackupDiffVisitor( const Mercury::Address & realBaseAppAddr ) :
   		realBaseAppAddr_( realBaseAppAddr )
	{}

	virtual void onAdd( const Mercury::Address & addr,
			uint32 index, uint32 virtualSize, uint32 prime )
	{
		BaseApp * pBaseApp = BaseAppMgr::instance().findBaseApp( addr );

		if (pBaseApp)
		{
			Mercury::Bundle & bundle = pBaseApp->bundle();
			BaseAppIntInterface::startBaseEntitiesBackupArgs & args =
				args.start( bundle );

			args.realBaseAppAddr = realBaseAppAddr_;
			args.index = index;
			args.hashSize = virtualSize;
			args.prime = prime;
			args.isInitial = false;

			pBaseApp->send();

			pBaseApp->startBackup( realBaseAppAddr_ );
		}
		else
		{
			ERROR_MSG( "FinishSetBackupDiffVisitor::onAdd: No BaseApp for %s\n",
					addr.c_str() );
		}
	}

	virtual void onChange( const Mercury::Address & addr,
			uint32 index, uint32 virtualSize, uint32 prime )
	{
		this->onAdd( addr, index, virtualSize, prime );
	}

	virtual void onRemove( const Mercury::Address & addr,
			uint32 index, uint32 virtualSize, uint32 prime )
	{
		BaseApp * pBaseApp = BaseAppMgr::instance().findBaseApp( addr );

		if (pBaseApp)
		{
			Mercury::Bundle & bundle = pBaseApp->bundle();
			BaseAppIntInterface::stopBaseEntitiesBackupArgs & args =
				args.start( bundle );

			args.realBaseAppAddr = realBaseAppAddr_;
			args.index = index;
			args.hashSize = virtualSize;
			args.prime = prime;
			args.isPending = false;

			pBaseApp->send();
			pBaseApp->stopBackup( realBaseAppAddr_ );
		}
	}

private:
	Mercury::Address realBaseAppAddr_;
};



/**
 *	This method handles a message from a BaseApp informing us that it is ready
 *	to use its new backup hash.
 */
void BaseAppMgr::useNewBackupHash( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data )
{
	BackupHash backupHash;
	BackupHash newBackupHash;

	data >> backupHash >> newBackupHash;

	BaseApp * pBaseApp = this->findBaseApp( addr );

	if (pBaseApp)
	{
		FinishSetBackupDiffVisitor visitor( addr );
		backupHash.diff( newBackupHash, visitor );
		pBaseApp->backupHash().swap( newBackupHash );
		pBaseApp->newBackupHash().clear();
		pBaseApp->checkToStartOffloading();
	}
	else
	{
		WARNING_MSG( "BaseAppMgr::useNewBackupHash: "
				"No BaseApp %s. It may have just died.\n", addr.c_str() );
	}
}


/**
 *	This method handles a message from a BaseApp informing us that it has
 *	completed a full archive cycle. Only BaseApps with secondary databases
 *	enabled should send this message.
 */
void BaseAppMgr::informOfArchiveComplete( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data )
{
	BaseApp * pBaseApp = this->findBaseApp( addr );

	if (!pBaseApp)
	{
		ERROR_MSG( "BaseAppMgr::informOfArchiveComplete: No BaseApp with "
				"address %s\n",	addr.c_str() );
		return;
	}

	Mercury::Address deadBaseAppAddr;
	data >> deadBaseAppAddr;

	// Only interested in the last death
	if (deadBaseAppAddr != deadBaseAppAddr_)
	{
		return;
	}

	--archiveCompleteMsgCounter_;

	if (archiveCompleteMsgCounter_ == 0)
	{
		// Tell DBMgr which secondary databases are still active
		Mercury::Bundle & bundle = this->dbMgr().bundle();
		bundle.startMessage( DBInterface::updateSecondaryDBs );

		bundle << uint32(baseApps_.size());

		for (BaseApps::iterator iter = baseApps_.begin();
			   iter != baseApps_.end(); ++iter)
		{
			bundle << iter->second->id();
		}

		this->dbMgr().send();
	}
}


/**
 *	This method handles a request to send back the backup hash chain to the
 *	requesting address. 
 */
void BaseAppMgr::requestBackupHashChain( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data )
{
	Mercury::Bundle bundle;

	// TODO: Make the requesting app retry until it gets a response, as it is
	// usually the Web Integration module making the requests, and it can't do
	// processing until after it's received its reply, so we can't make this
	// reliable.
	bundle.startReply( header.replyID, Mercury::RELIABLE_NO );
	bundle << *pBackupHashChain_;

	this->interface().send( addr, bundle );
}


/**
 *	This method responds to a message from the DBMgr that tells us to start.
 */
void BaseAppMgr::startup( const Mercury::Address & addr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data )
{
	if (hasStarted_)
	{
		WARNING_MSG( "BaseAppMgr::ready: Already ready.\n" );
		return;
	}

	bool didAutoLoadEntitiesFromDB;
	data >> didAutoLoadEntitiesFromDB;

	INFO_MSG( "BaseAppMgr is starting\n" );

	this->startTimer();

	// Start the CellAppMgr
	{
		Mercury::Bundle & bundle = cellAppMgr_.bundle();
		bundle.startMessage( CellAppMgrInterface::startup );
		cellAppMgr_.send();
	}

	// Start the BaseApps.
	{
		if (baseApps_.empty())
		{
			CRITICAL_MSG( "BaseAppMgr::ready: "
				"No Base apps running when started.\n" );
		}

		// Tell all the baseapps to start up, but only one is the bootstrap
		bool bootstrap = true;
		for (BaseApps::iterator it = baseApps_.begin();	it != baseApps_.end(); ++it)
		{
			BaseApp & baseApp = *it->second;
			Mercury::Bundle & bundle = baseApp.bundle();
			BaseAppIntInterface::startupArgs & args = args.start( bundle );
			args.bootstrap = bootstrap;
			args.didAutoLoadEntitiesFromDB = didAutoLoadEntitiesFromDB;

			baseApp.send();
			bootstrap = false;
		}
	}
}


/**
 *	This method starts the game timer.
 */
void BaseAppMgr::startTimer()
{
	if (!hasStarted_)
	{
		hasStarted_ = true;
		gameTimer_ = mainDispatcher_.addTimer( 1000000/Config::updateHertz(),
				this,
				reinterpret_cast< void * >( TIMEOUT_GAME_TICK ) );
		pTimeKeeper_ = new TimeKeeper( interface_, gameTimer_, time_,
				Config::updateHertz(), cellAppMgr_.addr(),
				&CellAppMgrInterface::gameTimeReading );
	}
}


/**
 *	This method handles a request from the DBMgr for our status. The status from
 *	the CellAppMgr is retrieved and then both returned.
 */
void BaseAppMgr::checkStatus( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data )
{
	if (cellAppMgr_.channel().isEstablished())
	{
		Mercury::Bundle & bundle = cellAppMgr_.bundle();
		bundle.startRequest( CellAppMgrInterface::checkStatus,
			   new CheckStatusReplyHandler( addr, header.replyID ) );
		bundle.transfer( data, data.remainingLength() );
		cellAppMgr_.send();
	}
	else
	{
		IF_NOT_MF_ASSERT_DEV( this->dbMgr().addr() == addr )
		{
			return;
		}

		Mercury::Bundle & bundle = this->dbMgr().bundle();
		bundle.startReply( header.replyID );
		bundle << uint8(false) << this->numBaseApps() << 0;
		bundle << "No CellAppMgr";
		this->dbMgr().send();
	}
}


/**
 *	This method is called to let the BaseAppMgr know that there is a new
 *	CellAppMgr.
 */
void BaseAppMgr::handleCellAppMgrBirth(
	const BaseAppMgrInterface::handleCellAppMgrBirthArgs & args )
{
	INFO_MSG( "BaseAppMgr::handleCellAppMgrBirth: %s\n", args.addr.c_str() );

	if (!cellAppMgr_.channel().isEstablished() && (args.addr.ip != 0))
	{
		INFO_MSG( "BaseAppMgr::handleCellAppMgrBirth: "
					"CellAppMgr is now ready.\n" );
	}

	cellAppMgr_.addr( args.addr );

	if (pTimeKeeper_)
	{
		pTimeKeeper_->masterAddress( cellAppMgr_.addr() );
	}

	// Reset the bestBaseAppAddr to allow the CellAppMgr to be
	// notified next game tick.
	bestBaseAppAddr_.ip = 0;
	bestBaseAppAddr_.port = 0;
}


/**
 *	This method is called when another BaseAppMgr is started.
 */
void BaseAppMgr::handleBaseAppMgrBirth(
	const BaseAppMgrInterface::handleBaseAppMgrBirthArgs & args )
{
	if (args.addr != interface_.address())
	{
		WARNING_MSG( "BaseAppMgr::handleBaseAppMgrBirth: %s\n",
				args.addr.c_str() );
		this->shutDown( false );
	}
}


/**
 *	This method is called when a cell application has died unexpectedly.
 */
void BaseAppMgr::handleCellAppDeath( const Mercury::Address & /*addr*/,
		const Mercury::UnpackedMessageHeader & /*header*/,
		BinaryIStream & data )
{
	TRACE_MSG( "BaseAppMgr::handleCellAppDeath:\n" );

	// Make a local memory stream with the data so we can add it to the bundle
	// for each BaseApp.
	MemoryOStream payload;
	payload.transfer( data, data.remainingLength() );

	this->sendToBaseApps( BaseAppIntInterface::handleCellAppDeath, payload );
}


/**
 *  This method is called by machined to inform us of a base application that
 *  has died unexpectedly.
 */
void BaseAppMgr::handleBaseAppDeath(
				const BaseAppMgrInterface::handleBaseAppDeathArgs & args )
{
	this->handleBaseAppDeath( args.addr );
}


/**
 *	This method handles a BaseApp dying unexpectedly.
 */
void BaseAppMgr::handleBaseAppDeath( const Mercury::Address & addr )
{
	if (shutDownStage_ != SHUTDOWN_NONE)
		return;

	INFO_MSG( "BaseAppMgr::handleBaseAppDeath: dead app on %s\n",
			addr.c_str() );

	this->onBaseAppDeath( addr );
}


/**
 *	This method attempts to add a global base.
 */
void BaseAppMgr::registerBaseGlobally( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data )
{
	// Figure out which baseapp sent this message
	BaseApp * pSender = this->findBaseApp( srcAddr );
	IF_NOT_MF_ASSERT_DEV( pSender )
	{
		ERROR_MSG( "BaseAppMgr::registerBaseGlobally: "
			"Got message from unregistered app @ %s, registration aborted\n",
			srcAddr.c_str() );

		return;
	}

	std::pair< std::string, EntityMailBoxRef > value;

	data >> value.first >> value.second;

	INFO_MSG( "BaseAppMgr::registerBaseGlobally: Registered base %d from %s\n",
		value.second.id, srcAddr.c_str() );

	int8 successCode = 0;

	if (globalBases_.insert( value ).second)
	{
		successCode = 1;

		MemoryOStream args;
		args << value.first << value.second;

		this->sendToBaseApps( BaseAppIntInterface::addGlobalBase,
			args, pSender );
	}
	else
	{
		INFO_MSG( "BaseAppMgr::registerBaseGlobally: Failed\n" );
	}

	// Send the ack back to the sender.
	Mercury::Bundle & bundle = pSender->bundle();
	bundle.startReply( header.replyID );
	bundle << successCode;
	pSender->send();
}


/**
 *	This method attempts to remove a global base.
 */
void BaseAppMgr::deregisterBaseGlobally( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & /*header*/,
			BinaryIStream & data )
{
	std::string label;
	data >> label;

	INFO_MSG( "BaseAppMgr::deregisterBaseGlobally: %s\n", srcAddr.c_str() );

	if (globalBases_.erase( label ))
	{
		BaseApp * pSrc = this->findBaseApp( srcAddr );
		MemoryOStream payload;
		payload << label;

		this->sendToBaseApps( BaseAppIntInterface::delGlobalBase,
				payload, pSrc );
	}
	else
	{
		ERROR_MSG( "BaseAppMgr::deregisterBaseGlobally: Failed to erase %s\n",
			srcAddr.c_str() );
	}
}


/**
 *	This method returns the BaseApp or BackupBaseApp associated with the input
 *	address.
 */
Mercury::ChannelOwner *
		BaseAppMgr::findChannelOwner( const Mercury::Address & addr )
{
	BaseApps::iterator iter = baseApps_.find( addr );

	return (iter != baseApps_.end()) ?  iter->second.get() : NULL;
}


/**
 *	This static method returns a channel to the input address. If one does not
 *	exist, it is created.
 */
Mercury::Channel & BaseAppMgr::getChannel( const Mercury::Address & addr )
{
	return BaseAppMgr::instance().interface().findOrCreateChannel( addr );
}

// baseappmgr.cpp
