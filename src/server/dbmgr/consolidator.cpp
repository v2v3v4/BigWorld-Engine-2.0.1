/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "consolidator.hpp"

#include "database.hpp"

#include "dbmgr_lib/idatabase.hpp"

#include "cstdmf/debug.hpp"
#include "network/event_dispatcher.hpp"
#include "resmgr/bwresource.hpp"
#include "server/util.hpp"

#include <sys/types.h>
#include <sys/wait.h>

DECLARE_DEBUG_COMPONENT( 0 )


// -----------------------------------------------------------------------------
// Section: Constants
// -----------------------------------------------------------------------------
namespace
{

#define CONSOLIDATE_DBS_FILENAME_STR 	"consolidate_dbs"
#define CONSOLIDATE_DBS_RELPATH_STR 	"commands/"CONSOLIDATE_DBS_FILENAME_STR
const int CONSOLIDATE_DBS_EXEC_FAILED_EXIT_CODE = 100;

} // end anonymous namespace


// -----------------------------------------------------------------------------
// Section: Consolidator
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
Consolidator::Consolidator( Database & database ) :
	database_( database ),
	pid_( 0 ),
	pStdErr_( NULL )
{
	SignalProcessor::instance().addSignalHandler( SIGCHLD, this );

	this->startProcess();
}


/**
 *	Destructor
 */
Consolidator::~Consolidator()
{
	if (pid_ != 0)
	{
		this->kill();
	}

	this->closeFile();
}


/**
 *	This method closes the file to the child process.
 */
void Consolidator::closeFile()
{
	if (pStdErr_)
	{
		this->dispatcher().deregisterFileDescriptor( fileno( pStdErr_ ) );
		fclose( pStdErr_ );
		pStdErr_ = NULL;
	}
}


/**
 *	Return the database's event dispatcher.
 */
Mercury::EventDispatcher & Consolidator::dispatcher()
{
	return database_.mainDispatcher();
}


/**
 *	This method runs an external command to consolidate data from secondary
 * 	databases.
 */
bool Consolidator::startProcess()
{
	database_.getIDatabase().unlockDB(); 	// So consolidate process 
											// can access it.

	int filedes[2];

	// Create a pipe to capture the child's stderr
	if (pipe( filedes ) != 0)
	{
		ERROR_MSG( "Consolidator::startProcess: "
				"Failed to create a pipe to consolidate_dbs's stderr\n" );
		return false;
	}

	if ((pid_ = fork()) == 0)
	{
		this->execInFork( filedes[0], filedes[1] );
	}

	pStdErr_ = fdopen( filedes[0], "r" );
	this->dispatcher().registerFileDescriptor( filedes[0], this );
	close( filedes[1] );

	return true;
}


/**
 *	This method is called in the forked process to execute the consolidate_dbs
 *	process.
 */
void Consolidator::execInFork( int readFD, int writeFD )
{
	std::vector< std::string > cmdArgs;

	// Add resource paths.
	// NOTE: BWResource::getPathAsCommandLine() has some weird code which
	// made it unsuitable for us.
	{
		int numPaths = BWResource::getPathNum();
		if (numPaths > 0)
		{
			cmdArgs.push_back( "--res" );

			std::stringstream ss;
			ss << BWResource::getPath( 0 );
			for (int i = 1; i < numPaths; ++i)
			{
				ss << BW_RES_PATH_SEPARATOR << BWResource::getPath( i );
			}
			cmdArgs.push_back( ss.str() );
		}
	}

	// Redirect stderr to the pipe
	close( readFD );
	dup2( writeFD, 2 );
	close( writeFD );

	// Find path
	std::string path = Util::exeDir();

	// Change to it
	if (chdir( path.c_str() ) == -1)
	{
		printf( "Failed to change directory to %s\n", path.c_str() );
		exit(1);
	}

	// Add the exe name
	path += "/"CONSOLIDATE_DBS_RELPATH_STR;

	// Close parent sockets
	close( Database::instance().interface().socket() );

	// Make arguments into const char * array.
	const char ** argv = new const char *[cmdArgs.size() + 2];
	int i = 0;
	argv[i++] = path.c_str();
	for (std::vector< std::string >::iterator pCurArg = cmdArgs.begin();
			pCurArg != cmdArgs.end(); ++pCurArg, ++i)
	{
		argv[i] = pCurArg->c_str();
	}
	argv[i] = NULL;

	int result = execv( path.c_str(),
			const_cast< char * const * >( argv ) );

	if (result == -1)
	{
		exit( CONSOLIDATE_DBS_EXEC_FAILED_EXIT_CODE );
	}

	exit(1);
}


/**
 *	This method shuts this process down.
 */
void Consolidator::kill()
{
	if (pid_ != 0)
	{
		WARNING_MSG( "Consolidator::~Consolidator: "
				"Stopping ongoing consolidation process %d\n", pid_ );
		::kill( pid_, SIGINT );
	}
}


/**
 *	Handle a signal dispatched from the SignalProcessor.
 */
void Consolidator::handleSignal( int sigNum )
{
	int		status;
	pid_t 	childPID = ::waitpid( pid_, &status, WNOHANG );

	if (childPID == -1)
	{
		// Not us.
		return;
	}

	MF_ASSERT( childPID == pid_ );
	
	SignalProcessor::instance().clearSignalHandler( SIGCHLD, this );

	database_.onConsolidateProcessEnd( 
		this->onChildProcessExit( status ) );
}


/**
 * 	Notification that a child process has exited.
 */
bool Consolidator::onChildProcessExit( int status )
{
	bool isOK = false;
	if (WIFEXITED( status ))
	{
		int exitCode = WEXITSTATUS( status );
		if (exitCode == 0)
		{
			isOK = true;
		}
		else if (exitCode == CONSOLIDATE_DBS_EXEC_FAILED_EXIT_CODE)
		{
			std::string fullPath =
				Util::exeDir() + "/"CONSOLIDATE_DBS_RELPATH_STR;

			ERROR_MSG(
				"Consolidator::onChildProcessExit: Failed to execute %s.\n"
				"Please ensure that the " CONSOLIDATE_DBS_FILENAME_STR
				" executable exists and is runnable. You may need to build it "
				"manually as it not part of the standard package.\n",
					fullPath.c_str() );
		}
		else
		{
			ERROR_MSG( "Consolidator::onChildProcessExit: "
					"Consolidate process exited with code %d\n"
					"Please examine the logs for ConsolidateDBs or run "
					CONSOLIDATE_DBS_FILENAME_STR " manually to determine "
					"the cause of the error\n",
					exitCode );
		}
	}
	else if (WIFSIGNALED( status ))
	{
		ERROR_MSG( "Consolidator::onChildProcessExit: "
				"Consolidate process was terminated by signal %d\n",
				int( WTERMSIG( status ) ) );
		isOK = false;
	}

	if (isOK)
	{
		TRACE_MSG( "Finished data consolidation\n" );
	}
	else
	{
		this->outputErrorLogs();
	}

	pid_ = 0;
	this->closeFile();

	int attempt = 0;
	const int MAX_ATTEMPTS = 20;

	// Re-acquire lock to DB
	while (!database_.getIDatabase().lockDB() &&
			attempt < MAX_ATTEMPTS)
	{
		WARNING_MSG( "Consolidator::onChildProcessExit: "
				"Failed to re-lock database. Retrying (%d/%d)...\n",
			   ++attempt, MAX_ATTEMPTS );
		sleep(1);
	}

	return isOK;
}


/**
 *	This method is called when there is data waiting on the pipe from the child
 *	process.
 */
int Consolidator::handleInputNotification( int fd )
{
	this->readFromChildProcess();
	return 0;
}


/**
 *	This method reads log data from the child process.
 */
bool Consolidator::readFromChildProcess()
{
	if (!pStdErr_ || feof( pStdErr_ ))
	{
		return false;
	}

	char buffer[ 1024 ];
	int count = fread( buffer, sizeof( char ), sizeof( buffer ), pStdErr_ );
	stderrOutput_.write( buffer, count );

	return true;
}


/**
 *	This method outputs any error logs from the child process.
 */
void Consolidator::outputErrorLogs()
{
	while (this->readFromChildProcess())
	{
		/* Do nothing */;
	}

	std::string str = stderrOutput_.str();

	if (!str.empty())
	{
		std::string::size_type pos = str.find( "ERROR" );

		if (pos == std::string::npos)
		{
			pos = str.find( "WARNING" );

			if (pos == std::string::npos)
			{
				pos = 0;
			}
		}

		ERROR_MSG( "Log output from consolidate process:\n%s\n",
				str.c_str() + pos );
	}
}


// consolidator.cpp
