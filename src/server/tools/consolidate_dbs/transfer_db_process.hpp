/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MF_TRANSFER_DB_PROCESS
#define MF_TRANSFER_DB_PROCESS

#include "network/basictypes.hpp"
#include "network/machine_guard.hpp"

/**
 *	Class to handle transferring a secondary database from a remote machine.
 */
class TransferDBProcess : public MachineGuardMessage::ReplyHandler
{
public:
	TransferDBProcess( const Mercury::Address & listeningAddr );

	bool transfer( uint32 remoteIP, const std::string & path );


private:
	// MachineGuardMessage::ReplyHandler interface
	bool onPidMessage( PidMessage & pm, uint32 addr );

// Member data
	bool shouldAbort_;
	Mercury::Address listeningAddr_;
};

#endif // MF_TRANSFER_DB_PROCESS
