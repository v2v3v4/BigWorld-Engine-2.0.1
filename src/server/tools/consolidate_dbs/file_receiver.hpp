/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONSOLIDATE_DBS_FILE_RECEIVER_HPP
#define CONSOLIDATE_DBS_FILE_RECEIVER_HPP

#include "msg_receiver.hpp"

#include "network/basictypes.hpp"
#include "network/endpoint.hpp"
#include "network/interfaces.hpp"

#include <cstdio>
#include <string>

class FileReceiverMgr;

/**
 * 	Receives a secondary database file.
 */
class FileReceiver : public Mercury::InputNotificationHandler
{
public:
	FileReceiver( int socket, uint32 ip, uint16 port, FileReceiverMgr & mgr );
	virtual ~FileReceiver();

	const Mercury::Address & srcAddr() const
		{ return srcAddr_; }

	const std::string & destPath() const
		{ return destPath_; }

	const std::string & srcPath() const
		{ return srcPath_; }

	uint64 lastActivityTime() const 
		{ return lastActivityTime_; }

	bool deleteRemoteFile();
	bool deleteLocalFile();
	void abort();

	// Mercury::InputNotificationHandler override
	virtual int handleInputNotification( int fd );

private:
	size_t recvCommand();
	size_t recvSrcPathLen();
	size_t recvSrcPath();
	size_t recvFileLen();
	size_t recvFileContents();
	size_t recvErrorLen();
	size_t recvErrorStr();

	bool closeFile();

	typedef size_t (FileReceiver::*MessageProcessorFn)();

	Endpoint			endPoint_;
	FileReceiverMgr & 	mgr_;
	MsgReceiver			msgReceiver_;
	MessageProcessorFn	pMsgProcessor_;
	const char *		curActionDesc_;
	uint64				lastActivityTime_;
	Mercury::Address	srcAddr_;	// Cached for error info
	std::string			srcPath_;
	std::string			destPath_;
	uint32				expectedFileSize_;
	uint32				currentFileSize_;
	FILE *				destFile_;
};


#endif // CONSOLIDATE_DBS_FILE_RECEIVER_HPP
