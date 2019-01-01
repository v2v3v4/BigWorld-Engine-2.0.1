/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "status_check_watcher.hpp"

#include "loginapp.hpp"
#include "dbmgr/db_interface.hpp"

// -----------------------------------------------------------------------------
// Section: ReplyHandler
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
StatusCheckWatcher::ReplyHandler::ReplyHandler(
		WatcherPathRequestV2 & pathRequest ) :
	pathRequest_( pathRequest )
{
}


/**
 *	This method handles the reply from the DBMgr from the checkStatus request.
 */
void StatusCheckWatcher::ReplyHandler::handleMessage(
		const Mercury::Address & source,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data,
		void * arg )
{
	// TODO: Should check that there are not too many arguments.

	bool value;
	std::stringstream outputStream;

	data >> value;

	// Convert from a sequence of strings to one output string.
	while (!data.error() && (data.remainingLength() > 0))
	{
		std::string line;
		data >> line;

		outputStream << line << std::endl;
	}

	const std::string & output = outputStream.str();
	this->sendResult( value, output );

	delete this;
}


/**
 *	This method handles the failure case for the checkStatus request to DBMgr.
 */
void StatusCheckWatcher::ReplyHandler::handleException(
		const Mercury::NubException & ne, void * arg )
{
	WARNING_MSG( "StatusCheckWatcher::ReplyHandler::handleException:\n" );
	this->sendResult( false, "No reply from DBMgr\n" );
	delete this;
}


/**
 *	This method sends the reply to the watcher query.
 */
void StatusCheckWatcher::ReplyHandler::sendResult( bool status,
		const std::string & output )
{
	BinaryOStream & resultStream = pathRequest_.getResultStream();
	resultStream << (uint8)WATCHER_TYPE_TUPLE;
	resultStream << (uint8)WT_READ_ONLY;

	const int TUPLE_COUNT = 2;

	int resultSize = BinaryOStream::calculatePackedIntSize( TUPLE_COUNT ) +
		CallableWatcher::tupleElementStreamSize( output.size() ) +
		CallableWatcher::tupleElementStreamSize( sizeof( bool ) );

	resultStream.writeStringLength( resultSize );

	resultStream.writeStringLength( TUPLE_COUNT );

	watcherValueToStream( resultStream, output, WT_READ_ONLY );
	watcherValueToStream( resultStream, status, WT_READ_ONLY );

	pathRequest_.setResult( "", WT_READ_ONLY, NULL, NULL );
}


// -----------------------------------------------------------------------------
// Section: StatusCheckWatcher
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
StatusCheckWatcher::StatusCheckWatcher() :
	CallableWatcher( LOCAL_ONLY, "Check the status of the server" )
{
}


/**
 *	This method handles a set call on this watcher.
 */
bool StatusCheckWatcher::setFromStream( void * base,
		const char * path,
		WatcherPathRequestV2 & pathRequest )
{
	LoginApp & app = LoginApp::instance();
	Mercury::Bundle & bundle = app.dbMgr().bundle();
	bundle.startRequest( DBInterface::checkStatus,
		   new ReplyHandler( pathRequest ) );
	app.dbMgr().send();

	return true;
}

// status_check_watcher.cpp
