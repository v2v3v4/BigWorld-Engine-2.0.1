/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONSOLIDATE_DBS__FILE_RECEIVER_MGR_HPP
#define CONSOLIDATE_DBS__FILE_RECEIVER_MGR_HPP

#include "secondary_db_info.hpp"

#include "cstdmf/shared_ptr.hpp"
#include "cstdmf/stdmf.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace Mercury
{
	class EventDispatcher;
}

class DBConsolidatorErrors;
class FileReceiver;
class FileTransferProgressReporter;

typedef std::vector< std::string >	FileNames;

/**
 *	Receives secondary database files.
 */
class FileReceiverMgr
{
public:
	FileReceiverMgr( Mercury::EventDispatcher & dispatcher,
			FileTransferProgressReporter & progressReporter,
			const SecondaryDBInfos & secondaryDBs,
			const std::string & consolidationDir );
	~FileReceiverMgr();

	Mercury::EventDispatcher & dispatcher()
		{ return dispatcher_; }

	FileTransferProgressReporter & progressReporter()
		{ return progressReporter_; }

	const std::string & consolidationDir() const
		{ return consolidationDir_; }

	bool finished() const;

	const FileNames & receivedFilePaths() const
		{ return receivedFilePaths_; }

	bool cleanUpLocalFiles();
	bool cleanUpRemoteFiles( const DBConsolidatorErrors & errorDBs );

	// Called by TcpListener
	void onAcceptedConnection( int socket, uint32 ip, uint16 port );

	// Called by FileReceiver
	void onFileReceiverStart( FileReceiver & receiver );
	void onFileReceived( FileReceiver & receiver );

	// Called to notify us of an error.
	void onFileReceiveError();

	// Map of file location to host IP address.
	bool hasUnstartedDBs() const
		{ return ((unfinishedDBs_.size() - startedReceivers_.size()) > 0); }

	typedef std::map< std::string, uint32 >	SourceDBs;
	SourceDBs getUnstartedDBs() const;

	typedef std::set< FileReceiver* > ReceiverSet;
	const ReceiverSet& startedReceivers() const 
		{ return startedReceivers_; }

	static void onFailedBind( uint32 ip, uint16 port );
	static void onFailedAccept( uint32 ip, uint16 port );

private:
	Mercury::EventDispatcher &		dispatcher_;
	FileTransferProgressReporter & 	progressReporter_;
	std::string						consolidationDir_;
	SourceDBs						unfinishedDBs_;
	ReceiverSet						startedReceivers_;
	typedef std::vector< FileReceiver * > Receivers;
	Receivers						completedReceivers_;
	FileNames						receivedFilePaths_;
};


#endif // CONSOLIDATE_DBS__FILE_RECEIVER_MGR_HPP
