/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVER_APP_HPP
#define SERVER_APP_HPP

#include "common.hpp"

#include "network/network_interface.hpp"
#include "network/unpacked_message_header.hpp"

#include <memory>
#include <string>

class SignalHandler;
class Watcher;

namespace Mercury
{
class EventDispatcher;
class NetworkInterface;
}

#define SERVER_APP_HEADER( APP_NAME, CONFIG_PATH ) 							\
	static const char * appName()				{ return #APP_NAME; }		\
	static const char * configPath()			{ return #CONFIG_PATH; }	\
	virtual const char * getAppName() const		{ return #APP_NAME; }		\
	virtual const char * getConfigPath() const	{ return #CONFIG_PATH; }	\

/**
 *	This class is the base class of the main application class for BigWorld
 *	server-side processes, and supplies many common data and functions such as
 *	the event dispatcher, network interface as well as signal handling.
 *
 *	Subclasses should use the SERVER_APP_HEADER macro to supply the process
 *	name and configuration path root.
 *
 *	e.g. 
 *	class MyServerApp : public ServerApp
 *	{
 *	public:
 *		SERVER_APP_HEADER( MyServerApp, myServerApp )
 *	}
 */
class ServerApp
{
public:
	ServerApp( Mercury::EventDispatcher & mainDispatcher,
			Mercury::NetworkInterface & interface );
	virtual ~ServerApp();

	Mercury::EventDispatcher & mainDispatcher()		{ return mainDispatcher_; }
	Mercury::NetworkInterface & interface()			{ return interface_; }

	template <class APP_TYPE>
	static APP_TYPE & getApp( const Mercury::UnpackedMessageHeader & header )
	{
		return *static_cast< APP_TYPE * >(
				reinterpret_cast< ServerApp * >(
					header.pInterface->pExtensionData() ) );
	}

	// This static method is called after the app has been destroyed.
	static void postDestruction() {}

	// Implemented by SERVER_APP_HEADER macro.
	virtual const char * getAppName() const = 0;
	virtual const char * getConfigPath() const = 0;

	virtual bool init( int argc, char * argv[] );
	virtual bool run();

	virtual void shutDown();

	bool runApp( int argc, char * argv[] );

	GameTime time() const				{ return time_; }
	double gameTimeInSeconds() const;

	static const char * shutDownStageToString( ShutDownStage value );

	virtual void onSignalled( int sigNum );

protected:

	void enableSignalHandler( int sigNum, bool enable=true );

	virtual SignalHandler * createSignalHandler();

	void addWatchers( Watcher & watcher );

	GameTime time_;

	Mercury::EventDispatcher & mainDispatcher_;
	Mercury::NetworkInterface & interface_;

private:
	std::auto_ptr< SignalHandler > pSignalHandler_;
};

#endif // SERVER_APP_HPP
