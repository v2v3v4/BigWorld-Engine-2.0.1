/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "network/event_dispatcher.hpp"
#include "network/logger_message_forwarder.hpp"
#include "network/machine_guard.hpp"
#include "network/watcher_nub.hpp"

class BWLogWriter;

/**
 *	This is the main class of the message_logger process. It is responsible for
 *	receiving log messages from other components.
 */

class Logger : public WatcherRequestHandler
{
public:
	Logger();
	virtual ~Logger();

	bool init( int argc, char * argv[] );
	bool handleNextMessage();

	void shouldRoll( bool status ) { shouldRoll_ = status; }

protected:
	virtual void processExtensionMessage( int messageID,
			char * data, int dataLen, const Mercury::Address & addr );

public:
	class Component : public LoggerComponentMessage
	{
	public:
		static Watcher & watcher();
		const char *name() const { return componentName_.c_str(); }

		// TODO: Fix this
		bool commandAttached() const	{ return true; }
		void commandAttached( bool value );
	};

private:
	class FindHandler : public MachineGuardMessage::ReplyHandler
	{
	public:
		FindHandler( Logger &logger ) : logger_( logger ) {}
		virtual bool onProcessStatsMessage(
			ProcessStatsMessage &psm, uint32 addr );
	private:
		Logger &logger_;
	};

	class TagsHandler : public MachineGuardMessage::ReplyHandler
	{
	public:
		TagsHandler( Logger &logger ) : logger_( logger ) {}
		virtual bool onTagsMessage( TagsMessage &tm, uint32 addr );
	private:
		Logger &logger_;
	};

	void initClusterGroups();
	void initComponents();

	void handleBirth( const Mercury::Address & addr );
	void handleDeath( const Mercury::Address & addr );

	void handleLogMessage( MemoryIStream &is, const Mercury::Address & addr );
	void handleRegisterRequest(
			char * data, int dataLen, const Mercury::Address & addr );

	bool shouldConnect( const Component & component ) const;

	bool sendAdd( const Mercury::Address & addr );
	void sendDel( const Mercury::Address & addr );

	void delComponent( const Mercury::Address & addr, bool send = true );
	void delComponent( Component * pComponent );

	bool commandReattachAll() const { return true; }
	void commandReattachAll( bool value );

	bool resetFileDescriptors();

	Endpoint & socket()		{ return watcherNub_.udpSocket(); }

	// Watcher
	int size() const	{ return components_.size(); }

	std::string interfaceName_;
	WatcherNub watcherNub_;
	Mercury::EventDispatcher dispatcher_;

	// ID of the processes whose messages should be logged. The default is the
	// empty string, a special value that causes logging all processes,
	// regardless of loggerID.
	LoggerID loggerID_;

	uint logUser_;
	bool logAllUsers_;
	std::vector< std::string > logNames_;
	std::vector< std::string > doNotLogNames_;
	bool quietMode_;
	bool daemonMode_;
	bool shouldRoll_;

	bool createEndpointAndQueryMsg(
		MachineGuardMessage &mgm, uint32 addr,
		MachineGuardMessage::ReplyHandler &handler );

	bool shouldLogFromGroup( const Mercury::Address &addr );

	typedef std::vector< std::string > StringList;
	struct MachineGroups
	{
		// Groups that have been defined for the current machine
		StringList groups_;

		// Last time we asked the machine for it's updated list
		// of groups.
		uint64 lastPollTime_;
	};

	typedef std::map< uint32, MachineGroups * > MachineGroupsMap;
	MachineGroupsMap machineGroups_;

	// The list of group names this MessageLogger process should service.
	StringList groupNames_;

	std::string configFile_;

	std::string outputFilename_;
	std::string errorFilename_;

	std::string addLoggerData_;
	std::string delLoggerData_;

	typedef std::map< Mercury::Address, Component > Components;
	Components components_;

	bool shouldLogMessagePriority_[ NUM_MESSAGE_PRIORITY ];

	BWLogWriter *pLogWriter_;
};


#endif // LOGGER_HPP
