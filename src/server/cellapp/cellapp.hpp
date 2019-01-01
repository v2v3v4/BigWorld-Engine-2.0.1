/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/*
 *	Class: CellApp - represents the Cell application.
 *
 *	This class is used as a singleton. This object represents the entire Cell
 *	application. It's main functionality is to handle the interface to this
 *	application and redirect the calls.
 */

#ifndef CELLAPP_HPP
#define CELLAPP_HPP

#include <Python.h>

#include "cstdmf/memory_stream.hpp"
#include "cstdmf/singleton.hpp"

#include "entitydef/entity_description_map.hpp"
#include "pyscript/pickler.hpp"

#include "server/common.hpp"
#include "server/id_client.hpp"

#include "cells.hpp"
#include "cellapp_interface.hpp"
#include "cellappmgr_gateway.hpp"
#include "emergency_throttle.hpp"
#include "profile.hpp"
#include "updatable.hpp"
#include "updatables.hpp"

#include "cellapp_death_listener.hpp"

#include "server/entity_app.hpp"

class Cell;
class CellAppChannels;
class Entity;
class SharedData;
class Space;
class Spaces;
class TimeKeeper;
struct CellAppInitData;
class CellViewerServer;
class PythonServer;

typedef Mercury::ChannelOwner DBMgr;

class BufferedEntityMessages;
class BufferedGhostMessages;
class CellAppConfig;

/**
 *	This singleton class represents the entire application.
 */
class CellApp : public EntityApp, public TimerHandler,
	public Singleton< CellApp >
{
public:
	typedef CellAppConfig Config;

private:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK,
		TIMEOUT_TRIM_HISTORIES,
		TIMEOUT_LOADING_TICK
	};

public:
	ENTITY_APP_HEADER( CellApp, cellApp )

	/// @name Construction/Initialisation
	//@{
	CellApp( Mercury::EventDispatcher & mainDispatcher,
			Mercury::NetworkInterface & interface );
	virtual ~CellApp();

	bool finishInit( const CellAppInitData & initData );

	void onGetFirstCell( bool isFromDB );

	//@}

	/// @name Message handlers
	//@{
	void addCell( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );
	void startup( const CellAppInterface::startupArgs & args );
	void setGameTime( const CellAppInterface::setGameTimeArgs & args );

	void handleCellAppMgrBirth(
		const CellAppInterface::handleCellAppMgrBirthArgs & args );
	void addCellAppMgrRebirthData( BinaryOStream & stream );

	void handleCellAppDeath(
		const CellAppInterface::handleCellAppDeathArgs & args );

	void handleBaseAppDeath( BinaryIStream & data );

	virtual void shutDown();
	void shutDown( const CellAppInterface::shutDownArgs & args );
	void controlledShutDown(
			const CellAppInterface::controlledShutDownArgs & args );

	void setSharedData( BinaryIStream & data );
	void delSharedData( BinaryIStream & data );

	void setBaseApp( const CellAppInterface::setBaseAppArgs & args );

	void onloadTeleportedEntity( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header, BinaryIStream & data );

	void cellAppMgrInfo( const CellAppInterface::cellAppMgrInfoArgs & args );

	//@}

	/// @name Utility methods
	//@{
	Entity * findEntity( EntityID id ) const;

	void entityKeys( PyObject * pList ) const;
	void entityValues( PyObject * pList ) const;
	void entityItems( PyObject * pList ) const;

	std::string pickle( PyObject * args );
	PyObject * unpickle( const std::string & str );
	PyObject * newClassInstance( PyObject * pyClass,
			PyObject * pDictionary );

	bool reloadScript( bool isFullReload );
	//@}

	/// @name Accessors
	//@{
	Cell *	findCell( SpaceID id ) const;
	Space *	findSpace( SpaceID id ) const;

	static Mercury::Channel & getChannel( const Mercury::Address & addr )
	{
		return CellApp::instance().interface_.findOrCreateChannel( addr );
	}

	CellAppMgrGateway & cellAppMgr()				{ return cellAppMgr_; }
	DBMgr & dbMgr()							{ return *dbMgr_.pChannelOwner(); }

	float getLoad() const						{ return persistentLoad_; }

	uint64 lastGameTickTime() const				{ return lastGameTickTime_; }

	Cells & cells()								{ return cells_; }
	const Cells & cells() const					{ return cells_; }

	bool hasStarted() const					{ return gameTimer_.isSet(); }
	bool isShuttingDown() const					{ return shutDownTime_ != 0; }

	int numRealEntities() const;

	float maxCellAppLoad() const				{ return maxCellAppLoad_; }
	float emergencyThrottle() const			{ return throttle_.value(); }

	const Mercury::InterfaceElement & entityMessage( int index ) const;

	const Mercury::Address & baseAppAddr() const	{ return baseAppAddr_; }

	IDClient & idClient()				{ return idClient_; }
	//@}

	/// @name Update methods
	//@{
	bool registerForUpdate( Updatable * pObject, int level = 0 );
	bool deregisterForUpdate( Updatable * pObject );

	bool nextTickPending() const;	// are we running out of time?
	//@}

	/// @name Misc
	//@{
	void destroyCell( Cell * pCell );

	void detectDeadCellApps(
		const std::vector< Mercury::Address > & addrs );
	//@}

	BufferedEntityMessages & bufferedEntityMessages()
		{ return *pBufferedEntityMessages_; }
	BufferedGhostMessages & bufferedGhostMessages()
		{ return *pBufferedGhostMessages_; }

	void callWatcher( const Mercury::Address& srcAddr,
		const Mercury::UnpackedMessageHeader& header,
		BinaryIStream & data );

	bool shouldOffload() const { return shouldOffload_; }
	void shouldOffload( bool b ) { shouldOffload_ = b; }

	int id() const				{ return id_; }

	virtual void onSignalled( int sigNum );

private:
	// EntityApp
	virtual ManagerAppGateway & managerAppGateway() { return cellAppMgr_; }

	virtual bool init( int argc, char *argv[] );

	// Methods
	void initExtensions();
	bool initScript();

	void addWatchers();

	void checkSendWindowOverflows();

	void checkPython();

	int secondsToTicks( float seconds, int lowerBound );

	void startGameTime();

	void sendShutdownAck( ShutDownStage stage );

	bool inShutDownPause() const
	{
		return (shutDownTime_ != 0) && (time_ == shutDownTime_);
	}

	size_t numSpaces() const;

	/// @name Overrides
	//@{
	void handleTimeout( TimerHandle handle, void * arg );
	//@}
	void handleGameTickTimeSlice();
	void handleTrimHistoriesTimeSlice();

	void tickShutdown();

	double calcTickPeriod();
	double calcTransientLoadTime();
	double calcSpareTime();
	double calcThrottledLoadTime();

	void checkTickWarnings( double persistentLoadTime, double tickTime,
		   double spareTime );

	void addToLoad( float timeSpent, float & result ) const;
	void updateLoad();

	void updateBoundary();
	void callTimers();
	void tickBackup();
	void checkOffloads();
	void syncTime();

	float numSecondsBehind() const;

	// Data
	Cells			cells_;
	Spaces *		pSpaces_;

	// Must be before dbMgr_ as dbMgr_ destructor can cancel pending requests
	// and call back to idClient_.
	IDClient				idClient_;

	CellAppMgrGateway		cellAppMgr_;
	AnonymousChannelClient dbMgr_;

	GameTime		shutDownTime_;
	TimeKeeper * 	pTimeKeeper_;

	Pickler * pPickler_;

	Updatables updatables_;

	// Used for throttling back
	EmergencyThrottle throttle_;

	uint64					lastGameTickTime_;
	timeval					oldTimeval_;

	PythonServer *			pPythonServer_;

	SharedData *			pCellAppData_;
	SharedData *			pGlobalData_;

	Mercury::Address		baseAppAddr_;

	int						backupIndex_;

	TimerHandle				gameTimer_;
	TimerHandle				loadingTimer_;
	TimerHandle				trimHistoryTimer_;
	uint64					reservedTickTime_;

	CellViewerServer *		pViewerServer_;
	CellAppID				id_;

	float					persistentLoad_;
	float					transientLoad_;
	float					totalLoad_;

	float					maxCellAppLoad_;

	bool					shouldOffload_;
	bool 					hasAckedCellAppMgrShutDown_;


	BufferedEntityMessages * pBufferedEntityMessages_;
 	BufferedGhostMessages *  pBufferedGhostMessages_;

	CellAppChannels *		pCellAppChannels_;

	friend class CellAppResourceReloader;
};


#ifdef CODE_INLINE
#include "cellapp.ipp"
#endif

#endif // CELLAPP_HPP
