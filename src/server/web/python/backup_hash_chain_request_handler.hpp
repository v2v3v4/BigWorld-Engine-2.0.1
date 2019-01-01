/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BACKUP_HASH_CHAIN_REQUEST_HANDLER_HPP
#define BACKUP_HASH_CHAIN_REQUEST_HANDLER_HPP

#include "network/interfaces.hpp"

class BackupHashChain;
class BinaryIStream;

namespace Mercury
{
class Address;
class NubException;
class UnpackedMessageHeader;
}

class BackupHashChainRequestHandler :
	public Mercury::ShutdownSafeReplyMessageHandler
{
public:

	BackupHashChainRequestHandler( BackupHashChain & backupHashChain );
	~BackupHashChainRequestHandler() {}

	// From Mercury::ReplyMessageHandler
	virtual void handleMessage( const Mercury::Address & addr,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data, void * arg );

	virtual void handleException( const Mercury::NubException & exception, 
		void * arg );

	bool isOK() const { return isOK_; }

private:
	BackupHashChain & 	backupHashChain_;
	bool 				isOK_;
};

#endif // BACKUP_HASH_CHAIN_REQUEST_HANDLER_HPP
