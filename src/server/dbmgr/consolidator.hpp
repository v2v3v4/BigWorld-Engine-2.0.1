/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONSOLIDATOR_HPP
#define CONSOLIDATOR_HPP

#include "network/interfaces.hpp"
#include "server/signal_processor.hpp"

#include <memory>
#include <sstream>

class Database;

namespace Mercury
{
class EventDispatcher;
}


/**
 *	This class handles running consolidate_dbs child process.
 *	this application.
 */
class Consolidator : public Mercury::InputNotificationHandler, 
		public SignalHandler
{
public:
	Consolidator( Database & database );
	virtual ~Consolidator();

	bool onChildProcessExit( int status );

	pid_t pid() const	{ return pid_; }

	virtual int handleInputNotification( int fd );


private:
	bool startProcess();
	void execInFork( int readFD, int writeFD );
	void kill();
	void closeFile();
	
	Mercury::EventDispatcher & dispatcher();

	bool readFromChildProcess();
	void outputErrorLogs();
	
	virtual void handleSignal( int sigNum );


// Member data
	Database & database_;

	pid_t pid_;

	std::stringstream stderrOutput_;
	FILE * pStdErr_;
};

#endif // CONSOLIDATOR_HPP
