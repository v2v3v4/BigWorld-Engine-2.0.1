/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// logger_message_forwarder.cpp

#include "pch.hpp"

#include "cstdmf/config.hpp"

#if ENABLE_WATCHERS

#include "cstdmf/watcher.hpp"

#include "network/event_dispatcher.hpp"
#include "network/logger_message_forwarder.hpp"
#include "network/portmap.hpp"

#include <cstring>
#include <time.h>

#include "network/bsd_snprintf.h"

DECLARE_DEBUG_COMPONENT2( "Network", 0 );

/// Logger Message Forwarder Singleton
BW_SINGLETON_STORAGE( LoggerMessageForwarder )

namespace // anonymous
{

const uint8 MESSAGE_LOGGER_VERSION_STRING_ID = 7;

} // end anonymous namespace 

/**
 * * NOTE *
 * Any time you change the serialised format of this class, you must also update
 * LoggerComponentMessage in bigworld/tools/server/pycommon/messages.py to
 * match, otherwise server tools will not be able to talk to the logger.
 */
void LoggerComponentMessage::write( BinaryOStream &os ) const
{
	// In case MessageLogger gets a component register message for a previous
	// version that it either received from network or read from disk, upgrade
	// it and write it out as the current version.
	uint8 writeOutVersion = MESSAGE_LOGGER_VERSION;

	os << writeOutVersion << loggerID_ << uid_ << pid_ << componentName_;
}

void LoggerComponentMessage::read( BinaryIStream &is )
{
	is >> version_;

	if (version_ < MESSAGE_LOGGER_VERSION_STRING_ID)
	{
		uint8 loggerIDNum;
		is >> loggerIDNum;

		char buf[4];
		bw_snprintf( buf, sizeof( buf ), "%hu", loggerIDNum );
		loggerID_.assign( buf );
	}
	else
	{
		is >> loggerID_;
	}

	is >> uid_ >> pid_ >> componentName_;
}

// This is here so that the watcher can find Mercury::watcherValueToString
// when compiling using vc 2002
using namespace Mercury;

// -----------------------------------------------------------------------------
// Section: SimpleLoggerMessageForwarder
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
SimpleLoggerMessageForwarder::SimpleLoggerMessageForwarder(
		const std::string & appName,
		Endpoint & endpoint,
		LoggerID loggerID,
		bool enabled ) :
	appName_( appName ),
	loggerID_( loggerID ),
	appID_( 0 ),
	enabled_( enabled ),
	endpoint_( endpoint )
{
	TRACE_MSG( "Finding loggers ...\n" );
	// find all loggers on the network.
	this->findLoggerInterfaces();
	TRACE_MSG( "Total # loggers on network: %"PRIzu"\n", loggers_.size() );

	DebugFilter::instance().addMessageCallback( this );
}


/**
 *	Destructor
 */
SimpleLoggerMessageForwarder::~SimpleLoggerMessageForwarder()
{
	DebugFilter::instance().deleteMessageCallback( this );

	// Cleanup all the Forwarding String Handlers
	HandlerCache::iterator cacheIter = handlerCache_.begin();
	while (cacheIter != handlerCache_.end())
	{
		delete cacheIter->second;
		cacheIter++;
	}
}


/**
 *	This method is called for each log message. It forwards a message to each
 *	attached logger.
 */
bool SimpleLoggerMessageForwarder::handleMessage( int componentPriority,
	int messagePriority, const char * format, va_list argPtr )
{
	if (loggers_.empty() || !enabled_)
		return false;

	this->parseAndSend(this->findForwardingStringHandler( format ),
					   componentPriority, messagePriority, argPtr );

	return false;
}


bool SimpleLoggerMessageForwarder::isSuppressible( const std::string & format ) const
{
	return false;
}




// -----------------------------------------------------------------------------
// Section: LoggerMessageForwarder
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
LoggerMessageForwarder::LoggerMessageForwarder(
		std::string appName,
		Endpoint & endpoint,
		EventDispatcher & dispatcher,
		LoggerID loggerID,
		bool enabled,
		unsigned spamFilterThreshold ) :
	SimpleLoggerMessageForwarder(appName, endpoint, loggerID, enabled ),
	dispatcher_( dispatcher ),
	spamTimerHandle_(),
	spamFilterThreshold_( spamFilterThreshold ),
	spamHandler_( "* Suppressed %d in last 1s: %s" )
{
	this->init();
}


/**
 *	Destructor
 */
LoggerMessageForwarder::~LoggerMessageForwarder()
{
	// Stop spam suppression timer
	spamTimerHandle_.cancel();
}


/**
 *
 */
void SimpleLoggerMessageForwarder::registerAppID( int id )
{
	appID_ = id;

	for (Loggers::iterator it = loggers_.begin(); it != loggers_.end(); ++it)
	{
		this->sendAppID( *it );
	}
}


/**
 *  This method adds a string to this object's suppression prefix list.
 */
void LoggerMessageForwarder::addSuppressionPattern( std::string prefix )
{
	SuppressionPatterns::iterator iter = std::find(
		suppressionPatterns_.begin(), suppressionPatterns_.end(), prefix );

	if (iter == suppressionPatterns_.end())
	{
		suppressionPatterns_.push_back( prefix );
		this->updateSuppressionPatterns();
	}
	else
	{
		WARNING_MSG( "LoggerMessageForwarder::addSuppressionPattern: "
			"Not re-adding pattern '%s'\n",
			prefix.c_str() );
	}
}


/**
 *  This method deletes a string from the object's suppression prefix list.
 */
void LoggerMessageForwarder::delSuppressionPattern( std::string prefix )
{
	SuppressionPatterns::iterator iter = std::find(
		suppressionPatterns_.begin(), suppressionPatterns_.end(), prefix );

	if (iter != suppressionPatterns_.end())
	{
		suppressionPatterns_.erase( iter );
		this->updateSuppressionPatterns();
	}
	else
	{
		ERROR_MSG( "LoggerMessageForwarder::delSuppressionPattern: "
			"Tried to erase unknown suppression pattern '%s'\n",
			prefix.c_str() );
	}
}


/**
 *  This method updates FormatStringHandler::isSuppressible_ for all existing
 *  handlers based on the current list of suppression patterns.  Typically, this
 *  is only called from addSuppressionPattern() immediately after app startup,
 *  so the list is empty or small.
 */
void LoggerMessageForwarder::updateSuppressionPatterns()
{
	for (HandlerCache::iterator iter = handlerCache_.begin();
		 iter != handlerCache_.end(); ++iter)
	{
		ForwardingStringHandler & handler = *iter->second;
		handler.isSuppressible( this->isSuppressible( handler.fmt() ) );
	}
}


void LoggerMessageForwarder::init()
{
	MF_WATCH( "logger/add", *this,
			&LoggerMessageForwarder::watcherHack,
			&LoggerMessageForwarder::watcherAddLogger,
			"Used by MessageLogger to add itself as a logging destination" );
	MF_WATCH( "logger/del", *this,
			&LoggerMessageForwarder::watcherHack,
			&LoggerMessageForwarder::watcherDelLogger,
			"Used by MessageLogger to remove itself as a logging destination" );
	MF_WATCH( "logger/size", *this, &LoggerMessageForwarder::size,
		   "The number of loggers this process is sending to" );
	MF_WATCH( "logger/enabled", enabled_, Watcher::WT_READ_WRITE,
		   "Whether or not to forward messages to attached logs" );

	MF_WATCH( "logger/filterThreshold", DebugFilter::instance(),
			MF_ACCESSORS( int, DebugFilter, filterThreshold ),
	   "Controls the level at which messages are sent to connected loggers.\n"
	   "A higher value reduces the volume of messages sent." );

	MF_WATCH( "logger/spamThreshold", spamFilterThreshold_,
		Watcher::WT_READ_WRITE,
		"The maximum number of a particular message that will be sent to the "
		"logs each second." );

	MF_WATCH( "logger/addSuppressionPattern", *this,
		&LoggerMessageForwarder::suppressionWatcherHack,
		&LoggerMessageForwarder::addSuppressionPattern,
		"Adds a new spam suppression pattern to this logger" );

	MF_WATCH( "logger/delSuppressionPattern", *this,
		&LoggerMessageForwarder::suppressionWatcherHack,
		&LoggerMessageForwarder::delSuppressionPattern,
		"Removes a spam suppression pattern from this logger" );

	MF_WATCH( "config/hasDevelopmentAssertions", DebugFilter::instance(),
			MF_ACCESSORS( bool, DebugFilter, hasDevelopmentAssertions ),
	   "If true, the process will be stopped when a development-time "
		   "assertion fails" );

	// Register a timer for doing spam suppression
	spamTimerHandle_ = dispatcher_.addTimer( 1000000 /* 1s */,
		this );
}


/**
 *	This method adds a logger that we should forward to.
 */
void SimpleLoggerMessageForwarder::addLogger( const Mercury::Address & addr )
{
	Loggers::iterator iter =
		std::find( loggers_.begin(), loggers_.end(), addr );

	if (iter != loggers_.end())
	{
		WARNING_MSG( "LoggerMessageForwarder::addLogger: Re-adding %s\n",
				addr.c_str() );
	}
	else
	{
		loggers_.push_back( addr );
	}

	// tell the logger about us.
	MemoryOStream os;
	os << (int)MESSAGE_LOGGER_REGISTER;

	LoggerComponentMessage lrm;
	lrm.version_ = MESSAGE_LOGGER_VERSION;
	lrm.loggerID_ = loggerID_;
	lrm.pid_ = mf_getpid();
	lrm.uid_ = getUserId();
	lrm.componentName_ = appName_;
	lrm.write( os );

	endpoint_.sendto( os.data(), os.size(), addr.port, addr.ip );

	INFO_MSG( "LoggerMessageForwarder::addLogger: "
				"Added %s. # loggers = %"PRIzu"\n",
			addr.c_str(), loggers_.size() );

	// This must be after the INFO_MSG above, otherwise the logger won't know
	// enough about this app to set the app ID.
	if (appID_ > 0)
		this->sendAppID( addr );
}


/**
 *	This method removes a logger that we have been forwarding to.
 */
void SimpleLoggerMessageForwarder::delLogger( const Mercury::Address & addr )
{
	Loggers::iterator iter =
		std::find( loggers_.begin(), loggers_.end(), addr );

	if (iter != loggers_.end())
	{
		loggers_.erase( iter );
		INFO_MSG( "LoggerMessageForwarder::delLogger: "
				"Removed %s. # loggers = %"PRIzu"\n",
			addr.c_str(), loggers_.size() );
	}
}

void SimpleLoggerMessageForwarder::sendAppID( const Mercury::Address &addr )
{
	MF_ASSERT( appID_ != 0 );

	MemoryOStream os;
	os << (int)MESSAGE_LOGGER_APP_ID << appID_;

	if (endpoint_.sendto( os.data(), os.size(), addr.port, addr.ip ) !=
		os.size())
	{
		ERROR_MSG( "LoggerMessageForwarder::registerAppID: "
			"Failed to send app ID to %s\n", addr.c_str() );
	}
}

/**
 *	This method is a work-around for implementing a write-only watcher.
 */
Mercury::Address LoggerMessageForwarder::watcherHack() const
{
	return Mercury::Address( 0, 0 );
}

bool LoggerMessageForwarder::FindLoggerHandler::onProcessStatsMessage(
	ProcessStatsMessage &psm, uint32 addr )
{
	if (psm.pid_ != 0)
	{
		Mercury::Address address( addr, psm.port_ );
		lmf_.addLogger( address );
	}
	return true;
}


/**
 *	This method finds all loggers that are currently running on the network.
 */
void SimpleLoggerMessageForwarder::findLoggerInterfaces()
{
	// Construct mgm message asking for LoggerInterfaces.
	ProcessStatsMessage psm;
	psm.param_ = psm.PARAM_USE_CATEGORY | psm.PARAM_USE_NAME;
	psm.category_ = psm.WATCHER_NUB;
	psm.name_ = MESSAGE_LOGGER_NAME;

	FindLoggerHandler handler( *this );
	int reason;
	if ((reason = psm.sendAndRecv( endpoint_, BROADCAST, &handler )) !=
		Mercury::REASON_SUCCESS)
	{
		ERROR_MSG( "LoggerMessageForwarder::findLoggerInterfaces: "
			"MGM::sendAndRecv() failed (%s)\n",
			Mercury::reasonToString( (Mercury::Reason&)reason ) );
	}
}

ForwardingStringHandler *
SimpleLoggerMessageForwarder::findForwardingStringHandler(
	const char * format )
{
	HandlerCache::iterator it = handlerCache_.find( format );
	if (it != handlerCache_.end())
	{
		return it->second;
	}
	ForwardingStringHandler * pHandler =
		new ForwardingStringHandler( format, this->isSuppressible( format ) );
	handlerCache_[ format ] = pHandler;
	return pHandler;
}

/**
 *	This method is called for each log message. It forwards a message to each
 *	attached logger.
 */
bool LoggerMessageForwarder::handleMessage( int componentPriority,
	int messagePriority, const char * format, va_list argPtr )
{
	if (loggers_.empty() || !enabled_)
		return false;

	// Find/create the handler object for this format string
	ForwardingStringHandler * pHandler =
		this->findForwardingStringHandler( format );

	// This must be done before the call to isSpamming() for this logic to be
	// the exact opposite of that in handleTimeout()
	pHandler->addRecentCall();

	// If this isn't considered to be spam, parse and send.
	if (!this->isSpamming( pHandler ))
	{
		this->parseAndSend(
			pHandler, componentPriority, messagePriority, argPtr );

		// If this is the first time this handler has been used this second, put
		// it in the used handlers collection.
		if (pHandler->numRecentCalls() == 1)
		{
			recentlyUsedHandlers_.push_back( pHandler );
		}
	}

	return false;
}


/**
 *  This method is called each second to summarise info about the log messages
 *  that have been spamming in the last second.
 */
void LoggerMessageForwarder::handleTimeout( TimerHandle handle, void * arg )
{
	// Send a message about each handler that exceeded its quota, and reset all
	// call counts.
	for (RecentlyUsedHandlers::iterator iter = recentlyUsedHandlers_.begin();
		 iter != recentlyUsedHandlers_.end(); ++iter)
	{
		ForwardingStringHandler * pHandler = *iter;

		if (this->isSpamming( pHandler ))
		{
			this->parseAndSend( &spamHandler_, 0, MESSAGE_PRIORITY_DEBUG,
				pHandler->numRecentCalls() - spamFilterThreshold_,
				pHandler->fmt().c_str() );
		}

		pHandler->clearRecentCalls();
	}

	recentlyUsedHandlers_.clear();
}


/**
 *  This method returns true if the given format string should be suppressed if
 *  it exceeds the spam suppression threshold.  This is used to set the
 *  isSuppressible_ member of ForwardingStringHandler.  It is not used when
 *  deciding whether or not to suppress a particular log message from being sent.
 */
bool LoggerMessageForwarder::isSuppressible( const std::string & format ) const
{
	for (SuppressionPatterns::const_iterator iter = suppressionPatterns_.begin();
		 iter != suppressionPatterns_.end(); ++iter)
	{
		const std::string & patt = *iter;

		// If there is an empty pattern in the suppression list, then
		// everything is a candidate for suppression.
		if (patt.empty())
		{
			return true;
		}

		// This basically means format.startswith( patt )
		if (format.size() >= patt.size() &&
			std::mismatch( patt.begin(), patt.end(), format.begin() ).first ==
				patt.end())
		{
			return true;
		}
	}

	return false;
}


/**
 *  This method assembles the stream for a log message and sends it to all known
 *  loggers.
 */
void SimpleLoggerMessageForwarder::parseAndSend( ForwardingStringHandler * pHandler,
	int componentPriority, int messagePriority, va_list argPtr )
{
	MemoryOStream os;
	LoggerMessageHeader hdr;

	hdr.componentPriority_ = componentPriority;
	hdr.messagePriority_ = messagePriority;

	os << (int)MESSAGE_LOGGER_MSG <<
		hdr.componentPriority_ << hdr.messagePriority_ << pHandler->fmt();

	pHandler->parseArgs( argPtr, os );

	for (Loggers::const_iterator iter = loggers_.begin();
		 iter != loggers_.end(); ++iter)
	{
		endpoint_.sendto( os.data(), os.size(), iter->port, iter->ip );
	}
}


/**
 *  This method is the same as above, except that the var args aren't already
 *  packaged up.  This allows us to send log messages without actually going via
 *  the *_MSG macros.  This is used to send the spam summaries.
 */
void SimpleLoggerMessageForwarder::parseAndSend( ForwardingStringHandler * pHandler,
	int componentPriority, int messagePriority, ... )
{
	va_list argPtr;
	va_start( argPtr, messagePriority );

	this->parseAndSend( pHandler, componentPriority, messagePriority, argPtr );

	va_end( argPtr );
}

#ifdef MF_SERVER

// -----------------------------------------------------------------------------
// Section: BWMessageForwarder
// -----------------------------------------------------------------------------

#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "server/bwconfig.hpp"

BWMessageForwarder::BWMessageForwarder(
		const char * componentName, const char * configPath,
		bool isForwarding,
		Mercury::EventDispatcher & dispatcher,
		Mercury::NetworkInterface & networkInterface ) :
	watcherNub_(),
	pForwarder_( NULL )
{
	std::string path( configPath );

	std::string monitoringInterfaceName =
		BWConfig::get( (path + "/monitoringInterface").c_str(),
				BWConfig::get( "monitoringInterface", "" ) );

	networkInterface.setLossRatio(
				BWConfig::get( (path + "/internalLossRatio").c_str(),
				BWConfig::get( "internalLossRatio", 0.f ) ) );
	networkInterface.setLatency(
			BWConfig::get( (path + "/internalLatencyMin").c_str(),
				BWConfig::get( "internalLatencyMin", 0.f ) ),
			BWConfig::get( (path + "/internalLatencyMax").c_str(),
				BWConfig::get( "internalLatencyMax", 0.f ) ) );

	networkInterface.setIrregularChannelsResendPeriod(
			BWConfig::get( (path + "/irregularResendPeriod").c_str(),
				BWConfig::get( "irregularResendPeriod",
					1.5f / BWConfig::get( "gameUpdateHertz", 10.f ) ) ) );

	networkInterface.shouldUseChecksums(
		BWConfig::get( (path + "/shouldUseChecksums").c_str(),
			BWConfig::get( "shouldUseChecksums", true ) ) );

	Mercury::Channel::setInternalMaxOverflowPackets(
		BWConfig::get( "maxChannelOverflow/internal",
		Mercury::Channel::getInternalMaxOverflowPackets() ));

	Mercury::Channel::setIndexedMaxOverflowPackets(
		BWConfig::get( "maxChannelOverflow/indexed",
		Mercury::Channel::getIndexedMaxOverflowPackets() ));

	Mercury::Channel::setExternalMaxOverflowPackets(
		BWConfig::get( "maxChannelOverflow/external",
		Mercury::Channel::getExternalMaxOverflowPackets() ));

	Mercury::Channel::assertOnMaxOverflowPackets(
		BWConfig::get( "maxChannelOverflow/isAssert",
		Mercury::Channel::assertOnMaxOverflowPackets() ));

	if (monitoringInterfaceName == "")
	{
		monitoringInterfaceName =
				inet_ntoa( (struct in_addr &)networkInterface.address().ip );
	}

	this->watcherNub().init( monitoringInterfaceName.c_str(), 0 );

	unsigned spamFilterThreshold =
		BWConfig::get( (path + "/logSpamThreshold").c_str(),
			BWConfig::get( "logSpamThreshold", 20 ) );

	pForwarder_.reset(
		new LoggerMessageForwarder( componentName,
			this->watcherNub().udpSocket(), dispatcher,
			BWConfig::get( "loggerID", "" ),
			isForwarding, spamFilterThreshold ) );

	DataSectionPtr pSuppressionPatterns =
		BWConfig::getSection( (path + "/logSpamPatterns").c_str(),
			BWConfig::getSection( "logSpamPatterns" ) );

	if (pSuppressionPatterns)
	{
		for (DataSectionIterator iter = pSuppressionPatterns->begin();
			 iter != pSuppressionPatterns->end(); ++iter)
		{
			pForwarder_->addSuppressionPattern( (*iter)->asString() );
		}
	}
}
#endif // MF_SERVER

#endif /* ENABLE_WATCHERS */

// logger_message_forwarder.cpp
