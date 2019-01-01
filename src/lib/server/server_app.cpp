/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "server_app.hpp"

#include "server_app_config.hpp"

#include "cstdmf/watcher.hpp"
#include "network/network_interface.hpp"
#include "network/event_dispatcher.hpp"

#include "server/signal_processor.hpp"

#include <signal.h>

namespace // (anonymous)
{

class ServerAppSignalHandler : public SignalHandler
{
public:
	ServerAppSignalHandler( ServerApp & serverApp ):
			serverApp_( serverApp )
	{}

	virtual ~ServerAppSignalHandler()
	{}

	virtual void handleSignal( int sigNum )
	{
		serverApp_.onSignalled( sigNum );
	}
private:
	ServerApp & serverApp_;
};

} // end namespace anonymous

/**
 *	Constructor.
 */
ServerApp::ServerApp( Mercury::EventDispatcher & mainDispatcher,
			Mercury::NetworkInterface & interface ) :
	time_( 0 ),
	mainDispatcher_( mainDispatcher ),
	interface_( interface )
{
	MF_WATCH( "gameTimeInTicks", time_, Watcher::WT_READ_ONLY );
	MF_WATCH( "gameTimeInSeconds", *this, &ServerApp::gameTimeInSeconds );

	interface_.pExtensionData( this );
}


/**
 *	Destructor.
 */
ServerApp::~ServerApp()
{}


/**
 *	Factory method for ServerApp subclasses to create their own signal
 *	handler instances. 
 *
 *	Subclasses should call enableSignalHandler() for each signal that this
 *	handler should handle. 
 *	
 *	The instance created is managed by the ServerApp instance.
 *
 *	The default ServerApp instance calls through to ServerApp::onSignalled().
 */
SignalHandler * ServerApp::createSignalHandler()
{
	return new ServerAppSignalHandler( *this );
}


/**
 *	This method adds the watchers associated with this class.
 */
void ServerApp::addWatchers( Watcher & watcher )
{
	watcher.addChild( "nub",
			Mercury::NetworkInterface::pWatcher(), &interface_ );
}


/**
 *	Initialisation function.
 *	
 *	This needs to be called from subclasses' overrides.
 */
bool ServerApp::init( int argc, char * argv[] )
{
	bool runFromMachined = false;

	for (int i = 1; i < argc; ++i)
	{
		if (strcmp( argv[i], "-machined" ))
		{
			runFromMachined = true;
		}
	}

	INFO_MSG( "ServerApp::init: Run from bwmachined = %s\n",
			watcherValueToString( runFromMachined ).c_str() );

	pSignalHandler_.reset( this->createSignalHandler() );
	
	// Handle signals
	this->enableSignalHandler( SIGINT );
	this->enableSignalHandler( SIGHUP );

	return true;
}


/**
 *	This is the default implementation of run. Derived classes to override
 *	this to implement their own functionality.
 */
bool ServerApp::run()
{
	mainDispatcher_.processUntilBreak();

	return true;
}


/**
 *	This method runs this application.
 */
bool ServerApp::runApp( int argc, char * argv[] )
{
	// calculate the clock speed
	stampsPerSecond();

	if (!this->init( argc, argv ))
	{
		ERROR_MSG( "Failed to initialise %s\n", this->getAppName() );
		return false;
	}

	INFO_MSG( "---- %s is running ----\n", this->getAppName() );

	bool result = this->run();

	interface_.prepareForShutdown();

	return result;
}


/**
 *	This method returns the current game time in seconds.
 */
double ServerApp::gameTimeInSeconds() const
{
	return double( time_ )/ServerAppConfig::updateHertz();
}


/**
 *	Convert a shutdown stage to a readable string.
 */
const char * ServerApp::shutDownStageToString( ShutDownStage value )
{
	switch( value )
	{
	case SHUTDOWN_NONE:
		return "SHUTDOWN_NONE";
	case SHUTDOWN_REQUEST:
		return "SHUTDOWN_REQUEST";
	case SHUTDOWN_INFORM:
		return "SHUTDOWN_INFORM";
	case SHUTDOWN_DISCONNECT_PROXIES:
		return "SHUTDOWN_DISCONNECT_PROXIES";
	case SHUTDOWN_PERFORM:
		return "SHUTDOWN_PERFORM";
	case SHUTDOWN_TRIGGER:
		return "SHUTDOWN_TRIGGER";
	default:
		return "(unknown)";
	}
}
	
	
/**
 *	Enables or disables the handling of a given signal by the ServerApp
 *	instance's signal handler.
 */
void ServerApp::enableSignalHandler( int sigNum, bool enable )
{
	if (pSignalHandler_.get() == NULL)
	{
		ERROR_MSG( "ServerApp::enableSignalHandler: no signal handler set\n" );
		return;
	}

	if (enable)
	{
		SignalProcessor::instance().addSignalHandler( sigNum, 
			pSignalHandler_.get() );
	}
	else
	{
		SignalProcessor::instance().clearSignalHandler( sigNum, 
			pSignalHandler_.get() );
	}
}


/**
 *	Shutdown the server application.
 *
 *	Can be overridden by subclasses to do app-specific shutdown logic.
 */
void ServerApp::shutDown()
{
	INFO_MSG( "ServerApp::shutDown: shutting down\n" );
	mainDispatcher_.breakProcessing();
}


/**
 *	Default signal handling.
 */
void ServerApp::onSignalled( int sigNum )
{
	switch (sigNum)
	{
	case SIGINT:
	case SIGHUP:
		this->shutDown();
	default:
		break;
	}
		
}

// server_app.cpp
