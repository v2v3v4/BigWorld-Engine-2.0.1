/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "machined_utils.hpp"

#include "blocking_reply_handler.hpp"
#include "bundle.hpp"
#include "machine_guard.hpp"

#include "cstdmf/bwversion.hpp"

namespace Mercury
{

namespace MachineDaemon
{

// ProcessMessage Handler for registerWithMachined to determine whether it
// received a valid response.
class ProcessMessageHandler : public MachineGuardMessage::ReplyHandler
{
public:
	ProcessMessageHandler() { hasResponded_ = false; }
	bool onProcessMessage( ProcessMessage &pm, uint32 addr );

	bool hasResponded_;
};


/**
 *  Handler to notify registerWithMachined() of the receipt of an expected
 *  message.
 */
bool ProcessMessageHandler::onProcessMessage(
		ProcessMessage &pm, uint32 addr )
{
	hasResponded_ = true;
	return false;
}


/**
 *	This function registers a socket with BWMachined.
 */
Reason registerWithMachined( const Address & srcAddr,
		const std::string & name, int id, bool isRegister )
{
	if (name.empty())
	{
		return REASON_SUCCESS;
	}

#ifdef MF_SERVER
	// Do not call blocking reply handler after registering with bwmachined as
	// other processes can now find us and send other messages.
	BlockingReplyHandler::safeToCall( false );
#endif

	ProcessMessage pm;

	pm.param_ = (isRegister ? pm.REGISTER : pm.DEREGISTER) |
		pm.PARAM_IS_MSGTYPE;
	pm.category_ = ProcessMessage::SERVER_COMPONENT;
	pm.port_ = srcAddr.port;
	pm.name_ = name;
	pm.id_ = id;

	pm.majorVersion_ = BWVersion::majorNumber();
	pm.minorVersion_ = BWVersion::minorNumber();
	pm.patchVersion_ = BWVersion::patchNumber();

	ProcessMessageHandler pmh;

	// send and wait for the reply
	const uint32 destAddr = LOCALHOST;

	Reason response = pm.sendAndRecv( srcAddr.ip, destAddr, &pmh );

	return pmh.hasResponded_ ? response : REASON_TIMER_EXPIRED;
}


/**
 *	This function deregisters a socket with BWMachined.
 */
Reason deregisterWithMachined( const Address & srcAddr,
		const std::string & name, int id )
{
	return name.empty() ?
		REASON_SUCCESS :
		registerWithMachined( srcAddr, name, id, /*isRegister:*/ false );
}


// -----------------------------------------------------------------------------
// Section: Birth and Death
// -----------------------------------------------------------------------------

/**
 *	This method is used to register a birth or death listener with machined.
 */
Reason registerListener( const Address & srcAddr,
		Bundle & bundle, int addrStart,
		const char * ifname, bool isBirth, bool anyUID = false )
{
	// finalise the bundle first
	bundle.finalise();
	const Packet * p = bundle.firstPacket_.get();

	MF_ASSERT( p->flags() == 0 );

	// prepare the message for machined
	ListenerMessage lm;
	lm.param_ = (isBirth ? lm.ADD_BIRTH_LISTENER : lm.ADD_DEATH_LISTENER) |
		lm.PARAM_IS_MSGTYPE;
	lm.category_ = lm.SERVER_COMPONENT;
	lm.uid_ = anyUID ? lm.ANY_UID : getUserId();
	lm.pid_ = mf_getpid();
	lm.port_ = srcAddr.port;
	lm.name_ = ifname;

	const int addrLen = 6;
	unsigned int postSize = p->totalSize() - addrStart - addrLen;

	lm.preAddr_ = std::string( p->data(), addrStart );
	lm.postAddr_ = std::string( p->data() + addrStart + addrLen, postSize );

	const uint32 destAddr = LOCALHOST;
	return lm.sendAndRecv( srcAddr.ip, destAddr, NULL );
}


/**
 *  This method registers a callback with machined to be called when a certain
 *	type of process is started.
 *
 *	@note This needs to be fixed up if rebind is called on this nub.
 */
Reason registerBirthListener( const Address & srcAddr,
		Bundle & bundle, int addrStart, const char * ifname )
{
	return registerListener( srcAddr, bundle, addrStart, ifname, true );
}


/**
 *  This method registers a callback with machined to be called when a certain
 *	type of process stops unexpectedly.
 *
 *	@note This needs to be fixed up if rebind is called on this nub.
 */
Reason registerDeathListener( const Address & srcAddr,
		Bundle & bundle, int addrStart, const char * ifname )
{
	return registerListener( srcAddr, bundle, addrStart, ifname, false );
}


/**
 *	This method registers a callback with machined to be called when a certain
 *	type of process is started.
 *
 *	@param ie		The interface element of the callback message. The message
 *				must be callable with one parameter of type Mercury::Address.
 *	@param ifname	The name of the interface to watch for.
 */
Reason registerBirthListener( const Address & srcAddr,
		const InterfaceElement & ie, const char * ifname )
{
	Mercury::Bundle bundle;

	bundle.startMessage( ie, RELIABLE_NO );
	int startOfAddress = bundle.size();
	bundle << Mercury::Address::NONE;

	return registerBirthListener( srcAddr, bundle, startOfAddress, ifname );
}


/**
 *  This method registers a callback with machined to be called when a certain
 *	type of process stops unexpectedly.
 *
 *	@param ie		The interface element of the callback message. The message
 *				must be callable with one parameter of type Mercury::Address.
 *	@param ifname	The name of the interface to watch for.
 */
Reason registerDeathListener( const Address & srcAddr,
		const InterfaceElement & ie, const char * ifname )
{
	Mercury::Bundle bundle;

	bundle.startMessage( ie, RELIABLE_NO );
	int startOfAddress = bundle.size();
	bundle << Mercury::Address::NONE;

	return registerDeathListener( srcAddr, bundle, startOfAddress, ifname );
}


/**
 *
 */
class FindInterfaceHandler : public MachineGuardMessage::ReplyHandler
{
public:
	FindInterfaceHandler( Address & address ) :
		found_( false ), address_( address ) {}

	virtual bool onProcessStatsMessage( ProcessStatsMessage &psm, uint32 addr )
	{
		if (psm.pid_ != 0)
		{
			address_.ip = addr;
			address_.port = psm.port_;
			address_.salt = 0;
			found_ = true;
			DEBUG_MSG( "Found interface %s at %s:%d\n",
				psm.name_.c_str(), inet_ntoa( (in_addr&)addr ),
				ntohs( address_.port ) );

			if ((psm.interfaceVersion_ != 0) &&
				(MERCURY_INTERFACE_VERSION != psm.interfaceVersion_))
			{
				ERROR_MSG( "Interface %s has version %d. Expected %d\n",
						psm.name_.c_str(), psm.interfaceVersion_,
						MERCURY_INTERFACE_VERSION );
				return false;
			}

			if (!psm.username_.empty() &&
					(psm.username_ != getUsername()))
			{
				ERROR_MSG( "Interface %s has username %s. Expected %s\n",
						psm.name_.c_str(), psm.username_.c_str(),
						getUsername() );
				return false;
			}
		}

		return true;
	}

	bool found() const	{ return found_; }

private:
	bool found_;
	Address &address_;
};


/**
 * 	This method finds the specified interface on the network.
 * 	WARNING: This function always blocks.
 *
 * 	@return	A Mercury::Reason.
 */
Reason findInterface( const char * name, int id,
		Address & address, int retries, bool verboseRetry )
{
	ProcessStatsMessage pm;
	pm.param_ = pm.PARAM_USE_CATEGORY |
		pm.PARAM_USE_UID |
		pm.PARAM_USE_NAME |
		(id < 0 ? 0 : pm.PARAM_USE_ID);
	pm.category_ = pm.SERVER_COMPONENT;
	pm.uid_ = getUserId();
	pm.name_ = name;
	pm.id_ = id;

	int attempt = 0;
	FindInterfaceHandler handler( address );

	retries = std::max( retries, 1 );

	while (++attempt <= retries)
	{
		pm.sendAndRecv( 0, BROADCAST, &handler );

		if (handler.found())
		{
			return REASON_SUCCESS;
		}

		if (verboseRetry)
		{
			INFO_MSG( "MachineDaemon::findInterface: "
					"Failed to find %s for UID %d on attempt %d.\n",
				name, pm.uid_, attempt );
		}

		// Sleep a little because sendAndReceiveMGM() is too fast now! :)
#if defined( PLAYSTATION3 )
		sys_timer_sleep( 1 );
#elif !defined( _WIN32 )
		sleep( 1 );
#else
		Sleep( 1000 );
#endif
	}

	return REASON_TIMER_EXPIRED;
}


/**
 *	This class is used by queryForInternalInterface.
 */
class QueryInterfaceHandler : public MachineGuardMessage::ReplyHandler
{
public:
	QueryInterfaceHandler( int requestType );
	bool onQueryInterfaceMessage( QueryInterfaceMessage & message,
			uint32 addr );

	bool hasResponded() const	{ return hasResponded_; }
	u_int32_t address() const	{ return address_; }

private:
	bool hasResponded_;
	u_int32_t address_;
	char request_;
};


/**
 *	Constructor.
 *
 *	@param requestType The type of request to make.
 */
QueryInterfaceHandler::QueryInterfaceHandler( int requestType ) :
	hasResponded_( false ),
	request_( requestType )
{
}


/**
 *	This method is called when a reply is received from bwmachined from our
 *	request.
 */
bool QueryInterfaceHandler::onQueryInterfaceMessage(
	QueryInterfaceMessage & message, uint32 addr )
{
	address_ = message.address_;
	hasResponded_ = true;

	return false;
}


/**
 *	This function queries the bwmachined daemon for what it thinks is the
 *	internal interface.
 *
 *	@param addr A reference to where the address will be written on success.
 *
 *	@return True on success.
 */
bool queryForInternalInterface( u_int32_t & addr )
{
	Endpoint ep;
	u_int32_t ifaddr;

	ep.socket( SOCK_DGRAM );

	// First we have to send a message over the local interface to
	// bwmachined to ask for which interface to treat as 'internal'.
	if (ep.getInterfaceAddress( "lo", ifaddr ) != 0)
	{
		WARNING_MSG( "MachineDaemon::queryForInternalInterface: "
			"Could not get 'lo' by name, defaulting to 127.0.0.1\n" );
		ifaddr = LOCALHOST;
	}

	QueryInterfaceMessage message;
	QueryInterfaceHandler handler( QueryInterfaceMessage::INTERNAL );

	if (REASON_SUCCESS != message.sendAndRecv( ep, ifaddr, &handler ))
	{
		ERROR_MSG( "MachineDaemon::queryForInternalInterface: "
			"Failed to send interface discovery message to bwmachined.\n" );
		return false;
	}

	if (handler.hasResponded())
	{
		addr = handler.address();
		return true;
	}

	return false;
}

} // namespace MachineDaemon

} // namespace Mercury

// machined_utils.cpp
