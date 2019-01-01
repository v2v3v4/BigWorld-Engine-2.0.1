/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "backup_hash_chain_request_handler.hpp"

#include "server/backup_hash_chain.hpp"

/**
 *	Constructor.
 */
BackupHashChainRequestHandler::BackupHashChainRequestHandler(
		BackupHashChain & backupHashChain ):
	Mercury::ShutdownSafeReplyMessageHandler(),
	backupHashChain_( backupHashChain ),
	isOK_( false )
{
}


/**
 *	Reply handler overridden from Mercury::ReplyMessageHandler
 */
void BackupHashChainRequestHandler::handleMessage( const Mercury::Address & addr,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data, void * arg )
{
	data >> backupHashChain_;
	isOK_ = true;
}


/**
 *	Exception handler overridden from Mercury::ReplyMessageHandler
 */
void BackupHashChainRequestHandler::handleException(
		const Mercury::NubException & exception, void * arg )
{
	isOK_ = false;
}

// backup_hash_chain_request_handler.cpp
