/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "transfer_db_process.hpp"

#include "cstdmf/stdmf.hpp"

#include <cstring>

TransferDBProcess::TransferDBProcess( 
		const Mercury::Address & listeningAddr ) :
	shouldAbort_( false ),
	listeningAddr_( listeningAddr )
{
}


bool TransferDBProcess::transfer( uint32 remoteIP, const std::string & path )
{
	CreateWithArgsMessage cm;
	cm.uid_ = ::getUserId();
#if defined _DEBUG
	cm.config_ = "Debug";
#elif defined _HYBRID
	cm.config_ = "Hybrid";
#endif
	cm.recover_ = 0;
	// TODO: don't hard code this
	cm.name_ = "commands/transfer_db";
	cm.fwdIp_ = 0;
	cm.fwdPort_ = 0;

	cm.args_.resize( 3 );
	cm.args_[0] = "consolidate";
	cm.args_[1] = path;
	cm.args_[2] = listeningAddr_.c_str();

	shouldAbort_ = false;
	if (cm.sendAndRecv( 0, remoteIP, this ) != Mercury::REASON_SUCCESS)
	{
		ERROR_MSG( "TransferDBProcess::consolidate: "
				"Failed to send creation message to BWMachined on %s.\n",
			inet_ntoa( (in_addr &)remoteIP ) );
		return false;
	}

	// shouldAbort_ magically set by onPidMessage() callback.
	return !shouldAbort_;
}


/**
 *	This method is called when a remote process to transfer the secondary
 * 	DB file is started.
 */
bool TransferDBProcess::onPidMessage( PidMessage & pm, uint32 addr )
{
	in_addr	address;
	address.s_addr = addr;

	if (pm.running_)
	{
		TRACE_MSG( "TransferDBProcess::onPidMessage: "
				"Started remote file transfer process %hd on %s\n",
			pm.pid_, inet_ntoa( address ) );
	}
	else
	{
		ERROR_MSG( "DBMgr::onPidMessage: "
				"Failed to start remote file transfer process on %s\n",
			inet_ntoa( address ) );
		shouldAbort_ = true;
	}

	// Stop waiting for more responses. We only expect one.
	return false;
}

// remote_process.cpp
