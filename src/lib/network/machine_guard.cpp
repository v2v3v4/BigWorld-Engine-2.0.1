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

#include "machine_guard.hpp"

#include "cstdmf/bwversion.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/memory_stream.hpp"
#include "cstdmf/timestamp.hpp"

#include "network/misc.hpp"
#include "network/portmap.hpp"

#include <set>
#include <sstream>

DECLARE_DEBUG_COMPONENT2( "Network", 0 );

#ifdef PLAYSTATION3
#define select socketselect
#endif

// -----------------------------------------------------------------------------
// Section: MGMPacket
// -----------------------------------------------------------------------------

MGMPacket::~MGMPacket()
{
	if (!dontDeleteMessages_)
	{
		for (unsigned i=0; i < messages_.size(); i++)
			if (delInfo_[i])
				delete messages_[i];
	}
}

void MGMPacket::read( MemoryIStream &is )
{
	is >> flags_ >> buddy_;
	while (is.remainingLength())
	{
		uint16 msglen; is >> msglen;
		const char *msgdata = (const char *)is.retrieve( msglen );
		if (is.error())
		{
			ERROR_MSG( "MGMPacket::read: "
				"Not enough bytes on stream (wanted %d)\n",	msglen );
			return;
		}

		MemoryIStream msgstream( msgdata, msglen );

		MachineGuardMessage *pMgm = MachineGuardMessage::create( msgstream );

		if (pMgm == NULL)
		{
			ERROR_MSG( "MGMPacket::read: Invalid message. msglen = %d\n",
					msglen );
			return;
		}

		this->append( *pMgm, true );
	}
}


/**
 * Write the contents of the MGMPacket to the output stream to send.
 *
 * @returns True on success, False on error.
 */
bool MGMPacket::write( MemoryOStream &os ) const
{
	os << flags_ << (s_buddy_ != BROADCAST ? s_buddy_ : buddy_);
	for (unsigned i=0; i < messages_.size(); i++)
	{
		int offset = os.size(); os.reserve( sizeof( uint16 ) );
		messages_[i]->write( os );
		uint16 msglen = os.size() - offset - sizeof( uint16 );
		*(uint16*)((char*)os.data() + offset) = BW_HTONS( msglen );
	}

	bool isOversized = (os.size() > MGMPacket::MAX_SIZE);
	if (isOversized)
	{
		ERROR_MSG( "MGMPacket::write: Wrote MGMPacket %d bytes long/%"PRIzu" "
			"messages. You need to implement MGM fragmentation\n",
			os.size(), messages_.size() );
	}

	return !isOversized;
}

void MGMPacket::append( MachineGuardMessage &mgm, bool shouldDelete )
{
	messages_.push_back( &mgm );
	delInfo_.push_back( shouldDelete );
}

void MGMPacket::setBuddy( uint32 buddy )
{
	s_buddy_ = buddy;
}

uint32 MGMPacket::s_buddy_ = BROADCAST;

// -----------------------------------------------------------------------------
// Section: MachineGuardMessage
// -----------------------------------------------------------------------------

MachineGuardMessage::MachineGuardMessage( uint8 message, uint8 flags, uint16 seq ) :
	message_( message ), flags_( flags ), seq_( seq ), seqSent_( false )
{
	if (seq_ == 0)
		this->refreshSeq();
}

void MachineGuardMessage::refreshSeq()
{
	if (s_seqTicker_ == 0)
	{
		srand( (int)timestamp() );
		s_seqTicker_ = rand() % 0xffff;
	}

	s_seqTicker_ = (s_seqTicker_ + 1) % 0xffff;
	if (s_seqTicker_ == 0)
		s_seqTicker_++;

	seq_ = s_seqTicker_;
	seqSent_ = false;
}

MachineGuardMessage* MachineGuardMessage::create( BinaryIStream &is )
{
	// Just peek the message and then let the bound methods do the rest of the work
	uint8 message = (uint8)is.peek();
	MachineGuardMessage *pMgm = NULL;

	if (is.error())
		return NULL;

	switch (message)
	{
	case HIGH_PRECISION_MACHINE_MESSAGE:
		pMgm = new HighPrecisionMachineMessage(); break;

	case WHOLE_MACHINE_MESSAGE:
		pMgm = new WholeMachineMessage(); break;

	case PROCESS_MESSAGE:
		pMgm = new ProcessMessage(); break;

	case PROCESS_STATS_MESSAGE:
		pMgm = new ProcessStatsMessage(); break;

	case LISTENER_MESSAGE:
		pMgm = new ListenerMessage(); break;

	case CREATE_MESSAGE:
		pMgm = new CreateMessage(); break;

	case SIGNAL_MESSAGE:
		pMgm = new SignalMessage(); break;

	case TAGS_MESSAGE:
		pMgm = new TagsMessage(); break;

	case USER_MESSAGE:
		pMgm = new UserMessage(); break;

	case PID_MESSAGE:
		pMgm = new PidMessage(); break;

	case RESET_MESSAGE:
		pMgm = new ResetMessage(); break;

	case ERROR_MESSAGE:
		pMgm = new ErrorMessage(); break;

	case MACHINED_ANNOUNCE_MESSAGE:
		pMgm = new MachinedAnnounceMessage(); break;

	case QUERY_INTERFACE_MESSAGE:
		pMgm = new QueryInterfaceMessage(); break;

	case CREATE_WITH_ARGS_MESSAGE:
		pMgm = new CreateWithArgsMessage(); break;

	default:
		pMgm = new UnknownMessage( Message( message ) ); break;
	}

	// Do message specific streaming
	pMgm->read( is );
	if (is.error())
	{
		delete pMgm;
		pMgm = new UnknownMessage( Message( message ) );
		pMgm->read( is );
	}

	return pMgm;
}

MachineGuardMessage* MachineGuardMessage::create( void *buf, int length )
{
	MemoryIStream is( buf, length );
	return create( is );
}

const char *MachineGuardMessage::typeStr() const
{
	static char buf[32];
	switch (message_)
	{
		case HIGH_PRECISION_MACHINE_MESSAGE: strcpy( buf, "HIGH_PRECISION" ); break;
		case WHOLE_MACHINE_MESSAGE: strcpy( buf, "WHOLE_MACHINE" ); break;
		case PROCESS_MESSAGE: strcpy( buf, "PROCESS" ); break;
		case PROCESS_STATS_MESSAGE: strcpy( buf, "PROCESS_STATS" ); break;
		case LISTENER_MESSAGE: strcpy( buf, "LISTENER" ); break;
		case CREATE_MESSAGE: strcpy( buf, "CREATE" ); break;
		case SIGNAL_MESSAGE: strcpy( buf, "SIGNAL" ); break;
		case TAGS_MESSAGE: strcpy( buf, "TAGS" ); break;
		case USER_MESSAGE: strcpy( buf, "USER" ); break;
		case PID_MESSAGE: strcpy( buf, "PID" ); break;
		case RESET_MESSAGE: strcpy( buf, "RESET" ); break;
		case MACHINED_ANNOUNCE_MESSAGE : strcpy( buf, "ANNOUNCE" ); break;
		case QUERY_INTERFACE_MESSAGE: strcpy( buf, "QUERY_INTERFACE" ); break;
		default: strcpy( buf, "** UNKNOWN **" ); break;
	}

	return buf;
}

void MachineGuardMessage::writeImpl( BinaryOStream &os )
{
	// Assuming that calling write() means you're about to send this message
	if (seqSent_ && !this->outgoing())
		this->refreshSeq();
	os << message_ << flags_ << seq_;
	seqSent_ = true;
}

void MachineGuardMessage::readImpl( BinaryIStream &is )
{
	is >> message_ >> flags_ >> seq_;
}

const char *MachineGuardMessage::c_str() const
{
	strcpy( s_buf_, this->typeStr() );
	return s_buf_;
}

bool MachineGuardMessage::sendto( Endpoint &ep, uint16 port, uint32 addr,
	uint8 packFlags )
{
	MGMPacket packet;
	MemoryOStream os;

	packet.flags_ = packFlags;
	packet.append( *this );
	packet.write( os );

	return ep.sendto( os.data(), os.size(), port, addr ) == os.size();
}


/**
 *	This method sends this MachineGuardMessage message to the bwmachined at the
 *	input address. The reply messages received are handled by the provided
 *	handler.
 *
 *	Note: If sending to BROADCAST, REASON_TIMER_EXPIRED will be returned if not
 *	all bwmachined daemons reply, even if some are successful.
 */
Mercury::Reason MachineGuardMessage::sendAndRecv( Endpoint &ep, uint32 destaddr,
	ReplyHandler *pHandler )
{
	if (destaddr == BROADCAST)
		ep.setbroadcast( true );

	char recvbuf[ MGMPacket::MAX_SIZE ];

	// The set of addresses we've heard from, and expect to
	std::set< uint32 > replied, waiting;

	int countdown = 3;
	while (countdown--)
	{
		if (!this->sendto( ep, htons( PORT_MACHINED ), destaddr,
				MGMPacket::PACKET_STAGGER_REPLIES ))
		{
			ERROR_MSG( "MachineGuardMessage::sendAndRecv: "
					"Failed to send entire MGM (#%d tries left)\n",
				countdown );
			continue;
		}

		timeval tv = { 1, 0 };
		fd_set fds;
		int fd = int( ep );
		FD_ZERO( &fds );
		FD_SET( fd, &fds );

		while (select( fd+1, &fds, NULL, NULL, &tv ) == 1)
		{
			uint32 srcaddr, buddyaddr;

			int len = ep.recvfrom( recvbuf, sizeof( recvbuf ), NULL,
				&(u_int32_t&)srcaddr );

			if (len == -1)
			{
				WARNING_MSG( "MachineGuardMessage::sendAndRecv: "
						"recvfrom failed (%s)\n",
					strerror( errno ) );
				continue;
			}

			bool badseq = false;
			bool continueProcessing = true;

			MemoryIStream is( (const char *)recvbuf, len );
			MGMPacket packet( is );
			buddyaddr = packet.buddy_;

			// If the packet is bad, drop it now
			if (is.error())
			{
				is.finish();
				continue;
			}

			for (unsigned i=0; i < packet.messages_.size(); i++)
			{
				MachineGuardMessage &reply = *packet.messages_[i];

				if (reply.seq_ != seq_)
				{
					WARNING_MSG( "MachineGuardMessage::sendAndRecv: "
						"Bad seq (%d, wanted %d) from %s: %s\n",
						reply.seq_, seq_, inet_ntoa( (in_addr&)srcaddr ),
						reply.c_str() );
					badseq = true;
					break;
				}

				if (reply.flags_ & reply.MESSAGE_NOT_UNDERSTOOD)
				{
					ERROR_MSG( "MachineGuardMessage::sendAndRecv: "
						"Machined on %s did not understand %s\n",
						inet_ntoa( (in_addr&)srcaddr ), reply.c_str() );
					continue;
				}

				if (pHandler)
				{
					continueProcessing = pHandler->handle( reply, srcaddr );
					if (!continueProcessing)
					{
						break;
					}
				}
			}

			if (badseq)
			{
				is.finish();
				continue;
			}

			replied.insert( srcaddr );
			if (waiting.count( srcaddr ))
			{
				waiting.erase( srcaddr );
			}

			if (replied.count( buddyaddr ) == 0 && buddyaddr != 0)
			{
				waiting.insert( buddyaddr );
			}

			// Terminate if we're sure we're done
			if (!continueProcessing || destaddr != BROADCAST ||
				(!replied.empty() && waiting.empty()))
			{
				return Mercury::REASON_SUCCESS;
			}

			is.finish();
		}
	}

	ERROR_MSG( "MachineGuardMessage::sendAndRecv: timed out!\n" );
	return Mercury::REASON_TIMER_EXPIRED;
}


Mercury::Reason MachineGuardMessage::sendAndRecv( uint32 srcip, uint32 destaddr,
	ReplyHandler *pHandler )
{
	// Set up socket
	Endpoint ep;
	ep.socket( SOCK_DGRAM );
	if (!ep.good() || ep.bind( 0, srcip ) != 0)
		return Mercury::REASON_GENERAL_NETWORK;

	return this->sendAndRecv( ep, destaddr, pHandler );
}


/**
 *  This method does and send and recv from the same IP address as the provided
 *  endpoint, but creates a new socket, guaranteeing that it will be on a
 *  different port.
 */
Mercury::Reason MachineGuardMessage::sendAndRecvFromEndpointAddr(
	Endpoint & ep, uint32 destaddr, ReplyHandler * pHandler )
{
	uint32 epAddr;
	if (ep.getlocaladdress( NULL, (u_int32_t*)&epAddr ))
	{
		ERROR_MSG( "MachineGuardMessage::sendAndRecvFromEndpointAddr: "
			"Couldn't get local address of provided endpoint\n" );

		return Mercury::REASON_GENERAL_NETWORK;
	}

	return this->sendAndRecv( epAddr, destaddr, pHandler );
}


char MachineGuardMessage::s_buf_[ 1024 ];
uint16 MachineGuardMessage::s_seqTicker_ = 0;

// -----------------------------------------------------------------------------
// Section: MachineGuardMessage::ReplyHandler
// -----------------------------------------------------------------------------

bool MachineGuardMessage::ReplyHandler::handle(
	MachineGuardMessage &mgm, uint32 addr )
{
	switch (mgm.message_)
	{
		case MachineGuardMessage::HIGH_PRECISION_MACHINE_MESSAGE:
			return onHighPrecisionMachineMessage(
				static_cast< HighPrecisionMachineMessage& >( mgm ), addr );
		case MachineGuardMessage::WHOLE_MACHINE_MESSAGE:
			return onWholeMachineMessage(
				static_cast< WholeMachineMessage& >( mgm ), addr );
		case MachineGuardMessage::PROCESS_MESSAGE:
			return onProcessMessage(
				static_cast< ProcessMessage& >( mgm ), addr );
		case MachineGuardMessage::PROCESS_STATS_MESSAGE:
			return onProcessStatsMessage(
				static_cast< ProcessStatsMessage& >( mgm ), addr );
		case MachineGuardMessage::LISTENER_MESSAGE:
			return onListenerMessage(
				static_cast< ListenerMessage& >( mgm ), addr );
		case MachineGuardMessage::CREATE_MESSAGE:
			return onCreateMessage(
				static_cast< CreateMessage& >( mgm ), addr );
		case MachineGuardMessage::SIGNAL_MESSAGE:
			return onSignalMessage(
				static_cast< SignalMessage& >( mgm ), addr );
		case MachineGuardMessage::TAGS_MESSAGE:
			return onTagsMessage(
				static_cast< TagsMessage& >( mgm ), addr );
		case MachineGuardMessage::USER_MESSAGE:
			return onUserMessage(
				static_cast< UserMessage& >( mgm ), addr );
		case MachineGuardMessage::PID_MESSAGE:
			return onPidMessage(
				static_cast< PidMessage& >( mgm ), addr );
		case MachineGuardMessage::RESET_MESSAGE:
			return onResetMessage(
				static_cast< ResetMessage& >( mgm ), addr );
		case MachineGuardMessage::ERROR_MESSAGE:
			return onErrorMessage(
				static_cast< ErrorMessage& >( mgm ), addr );
		case MachineGuardMessage::MACHINED_ANNOUNCE_MESSAGE:
			return onMachinedAnnounceMessage(
				static_cast< MachinedAnnounceMessage& >( mgm ), addr );
		case MachineGuardMessage::QUERY_INTERFACE_MESSAGE:
			return onQueryInterfaceMessage(
				static_cast< QueryInterfaceMessage& >( mgm ), addr );
		case MachineGuardMessage::CREATE_WITH_ARGS_MESSAGE:
			return onCreateWithArgsMessage(
				static_cast< CreateWithArgsMessage& >( mgm ), addr );
		default:
			return onUnhandledMsg( mgm, addr );
	}
}

bool MachineGuardMessage::ReplyHandler::onUnhandledMsg(
	MachineGuardMessage &mgm, uint32 addr )
{
	ERROR_MSG( "MGM::ReplyHandler: Unhandled message from %s: %s\n",
		inet_ntoa( (in_addr&)addr ), mgm.c_str() );
	return true;
}

bool MachineGuardMessage::ReplyHandler::onHighPrecisionMachineMessage(
	HighPrecisionMachineMessage &hpmm, uint32 addr ){
	return onUnhandledMsg( hpmm, addr ); }
bool MachineGuardMessage::ReplyHandler::onWholeMachineMessage(
	WholeMachineMessage &wmm, uint32 addr ){
	return onUnhandledMsg( wmm, addr ); }
bool MachineGuardMessage::ReplyHandler::onProcessMessage(
	ProcessMessage &pm, uint32 addr ){
	return onUnhandledMsg( pm, addr ); }
bool MachineGuardMessage::ReplyHandler::onProcessStatsMessage(
	ProcessStatsMessage &psm, uint32 addr ){
	return onUnhandledMsg( psm, addr ); }
bool MachineGuardMessage::ReplyHandler::onListenerMessage(
	ListenerMessage &lm, uint32 addr ){
	return onUnhandledMsg( lm, addr ); }
bool MachineGuardMessage::ReplyHandler::onCreateMessage(
	CreateMessage &cm, uint32 addr ){
	return onUnhandledMsg( cm, addr ); }
bool MachineGuardMessage::ReplyHandler::onSignalMessage(
	SignalMessage &sm, uint32 addr ){
	return onUnhandledMsg( sm, addr ); }
bool MachineGuardMessage::ReplyHandler::onTagsMessage(
	TagsMessage &tm, uint32 addr ){
	return onUnhandledMsg( tm, addr ); }
bool MachineGuardMessage::ReplyHandler::onUserMessage(
	UserMessage &um, uint32 addr ){
	return onUnhandledMsg( um, addr ); }
bool MachineGuardMessage::ReplyHandler::onPidMessage(
	PidMessage &pm, uint32 addr ){
	return onUnhandledMsg( pm, addr ); }
bool MachineGuardMessage::ReplyHandler::onResetMessage(
	ResetMessage &rm, uint32 addr ){
	return onUnhandledMsg( rm, addr ); }
bool MachineGuardMessage::ReplyHandler::onErrorMessage(
	ErrorMessage &rm, uint32 addr ){
	return onUnhandledMsg( rm, addr ); }
bool MachineGuardMessage::ReplyHandler::onMachinedAnnounceMessage(
	MachinedAnnounceMessage &mam, uint32 addr ){
	return onUnhandledMsg( mam, addr ); }
bool MachineGuardMessage::ReplyHandler::onQueryInterfaceMessage(
	QueryInterfaceMessage &qim, uint32 addr ){
	return onUnhandledMsg( qim, addr ); }
bool MachineGuardMessage::ReplyHandler::onCreateWithArgsMessage(
	CreateWithArgsMessage &cwam, uint32 addr ){
	return onUnhandledMsg( cwam, addr ); }


// -----------------------------------------------------------------------------
// Section: QueryInterfaceMessage
// -----------------------------------------------------------------------------
QueryInterfaceMessage::QueryInterfaceMessage() :
	MachineGuardMessage( MachineGuardMessage::QUERY_INTERFACE_MESSAGE ),
	address_( INTERNAL )
{
}

QueryInterfaceMessage::~QueryInterfaceMessage()
{
}

void QueryInterfaceMessage::writeImpl( BinaryOStream &os )
{
	MachineGuardMessage::writeImpl( os );

	os << address_;
}

void QueryInterfaceMessage::readImpl( BinaryIStream &is )
{
	MachineGuardMessage::readImpl( is );

	is >> address_;
}

const char *QueryInterfaceMessage::c_str() const
{
	bw_snprintf( MachineGuardMessage::s_buf_, sizeof(MachineGuardMessage::s_buf_), "QueryInterfaceMessage" );
	return MachineGuardMessage::s_buf_;
}




// -----------------------------------------------------------------------------
// Section: HighPrecisionMachineMessage
// -----------------------------------------------------------------------------

HighPrecisionMachineMessage::HighPrecisionMachineMessage() :
	MachineGuardMessage( MachineGuardMessage::HIGH_PRECISION_MACHINE_MESSAGE ),
	nCpus_( 0 ), nInterfaces_( 0 )
{
}

HighPrecisionMachineMessage::~HighPrecisionMachineMessage()
{
}

void HighPrecisionMachineMessage::writeImpl( BinaryOStream &os )
{
	MachineGuardMessage::writeImpl( os );

	os << hostname_ << cpuSpeed_ << nCpus_ << nInterfaces_
	   << mem_ << version_ << inDiscards_ << outDiscards_;

	for (uint i=0; i < nCpus_; i++)
		os << cpuLoads_[i];

	// This is where the message starts differing from the WholeMachineMessage
	os << ioWaitTime_;

	for (uint i=0; i < nInterfaces_; i++)
	{
		const HighPrecisionInterfaceStats &is = ifStats_[i];
		os << is.name_ << is.bitsIn_ << is.bitsOut_ <<
			is.packIn_ << is.packOut_;
	}
}

void HighPrecisionMachineMessage::readImpl( BinaryIStream &is )
{
	MachineGuardMessage::readImpl( is );

	is >> hostname_ >> cpuSpeed_ >> nCpus_ >> nInterfaces_
	   >> mem_ >> version_ >> inDiscards_ >> outDiscards_;

	cpuLoads_.resize( nCpus_, 0 );

	ifStats_.resize( nInterfaces_ );

	for (uint i=0; i < nCpus_; i++)
		is >> cpuLoads_[i];

	// This is where the message starts differing from the WholeMachineMessage
	is >> ioWaitTime_;

	for (uint i=0; i < nInterfaces_; i++)
	{
		HighPrecisionInterfaceStats &ifs = ifStats_[i];
		is >> ifs.name_ >> ifs.bitsIn_ >> ifs.bitsOut_ >>
			ifs.packIn_ >> ifs.packOut_;
	}
}

const char *HighPrecisionMachineMessage::c_str() const
{
	bw_snprintf( MachineGuardMessage::s_buf_,
				sizeof(MachineGuardMessage::s_buf_),
				"HighPrecisionMachineMessage" );
	return MachineGuardMessage::s_buf_;
}

// -----------------------------------------------------------------------------
// Section: WholeMachineMessage
// -----------------------------------------------------------------------------

WholeMachineMessage::WholeMachineMessage() :
	MachineGuardMessage( MachineGuardMessage::WHOLE_MACHINE_MESSAGE ),
	nCpus_( 0 ), nInterfaces_( 0 )
{
}

WholeMachineMessage::~WholeMachineMessage()
{
}

void WholeMachineMessage::writeImpl( BinaryOStream &os )
{
	MachineGuardMessage::writeImpl( os );

	os << hostname_ << cpuSpeed_ << nCpus_ << nInterfaces_
	   << mem_ << version_ << inDiscards_ << outDiscards_;

	for (int i=0; i < nCpus_; i++)
		os << cpuLoads_[i];

	for (int i=0; i < nInterfaces_; i++)
	{
		const LowPrecisionInterfaceStats &is = ifStats_[i];
		os << is.name_ << is.bitsIn_ << is.bitsOut_ <<
			is.packIn_ << is.packOut_;
	}
}

void WholeMachineMessage::readImpl( BinaryIStream &is )
{
	MachineGuardMessage::readImpl( is );

	is >> hostname_ >> cpuSpeed_ >> nCpus_ >> nInterfaces_
	   >> mem_ >> version_ >> inDiscards_ >> outDiscards_;

	cpuLoads_.resize( nCpus_, 0 );

	ifStats_.resize( nInterfaces_ );

	for (int i=0; i < nCpus_; i++)
		is >> cpuLoads_[i];

	for (int i=0; i < nInterfaces_; i++)
	{
		LowPrecisionInterfaceStats &ifs = ifStats_[i];
		is >> ifs.name_ >> ifs.bitsIn_ >> ifs.bitsOut_ >>
			ifs.packIn_ >> ifs.packOut_;
	}
}

const char *WholeMachineMessage::c_str() const
{
	bw_snprintf( MachineGuardMessage::s_buf_,
				sizeof(MachineGuardMessage::s_buf_),
				"WholeMachineMessage" );
	return MachineGuardMessage::s_buf_;
}


// -----------------------------------------------------------------------------
// Section: ProcessMessage
// -----------------------------------------------------------------------------

ProcessMessage::ProcessMessage() :
		MachineGuardMessage( PROCESS_MESSAGE ),
		param_( 0 ), // Needs to be set
		category_( 0 ), // Needs to be set
		uid_( getUserId() ),
		pid_( mf_getpid() ),
		port_( 0 ), // Needs to be set
		id_( 0 ), // Needs to be set
		name_(), // Needs to be set

		majorVersion_( 0 ),
		minorVersion_( 0 ),
		patchVersion_( 0 ),

		interfaceVersion_( MERCURY_INTERFACE_VERSION ),
		username_( getUsername() ),
		defDigest_() // Needs to be set
{
}

void ProcessMessage::writeImpl( BinaryOStream &os )
{
	MachineGuardMessage::writeImpl( os );
	os << param_ << category_ << uid_ << pid_ << port_ << id_ << name_;
}

void ProcessMessage::readImpl( BinaryIStream &is )
{
	MachineGuardMessage::readImpl( is );
	is >> param_ >> category_ >> uid_ >> pid_ >> port_ >> id_ >> name_;
}

void ProcessMessage::writeExtra( BinaryOStream & os )
{
	int32 extraData = sizeof( majorVersion_ ) +
		sizeof( minorVersion_ ) +
		sizeof( patchVersion_ ) +
		sizeof( interfaceVersion_ ) +
		BinaryOStream::calculateStreamedSizeOfString( username_.size() ) +
		BinaryOStream::calculateStreamedSizeOfString( defDigest_.size() );

	os << extraData << majorVersion_ << minorVersion_ << patchVersion_ <<
		interfaceVersion_ << username_ << defDigest_;
}

void ProcessMessage::readExtra( BinaryIStream & is )
{
	if (is.remainingLength() > 0)
	{
		int32 extraData;
		is >> extraData;
		int origSize = is.remainingLength();
	   	is >> majorVersion_ >> minorVersion_ >> patchVersion_ >>
			interfaceVersion_ >> username_ >> defDigest_;

		extraData -= origSize - is.remainingLength();

		// To handle future version, stream off any data that we were not
		// expecting.
		if (extraData < 0)
		{
			is.error( true );
		}
		else
		{
			is.retrieve( extraData );
		}
	}
}

const char *ProcessMessage::c_str() const
{
	bw_snprintf( MachineGuardMessage::s_buf_, sizeof(MachineGuardMessage::s_buf_),
		"%s (%s) (pid:%d)",
		name_.c_str(),
		ProcessMessage::categoryToString( category_ ),
		pid_ );
	return MachineGuardMessage::s_buf_;
}

bool ProcessMessage::matches( const ProcessMessage &query ) const
{
	if (query.param_ & query.PARAM_USE_UID && query.uid_ != uid_)
		return false;
	if (query.param_ & query.PARAM_USE_PID && query.pid_ != pid_)
		return false;
	if (query.param_ & query.PARAM_USE_ID && query.id_ != id_)
		return false;
	if (query.param_ & query.PARAM_USE_NAME && query.name_ != name_)
		return false;
	if (query.param_ & query.PARAM_USE_CATEGORY && query.category_ != category_)
		return false;
	if (query.param_ & query.PARAM_USE_PORT && query.port_ != port_)
		return false;
	return true;
}


const char *ProcessMessage::categoryToString( uint8 category )
{
	static const std::string sc = "SERVER_COMPONENT", wn = "WATCHER_NUB",
		err = "UNKNOWN";
	if (category == SERVER_COMPONENT)
		return sc.c_str();
	else if (category == WATCHER_NUB)
		return wn.c_str();
	else
		return err.c_str();
}

// -----------------------------------------------------------------------------
// Section: ProcessStatsMessage
// -----------------------------------------------------------------------------

void ProcessStatsMessage::writeImpl( BinaryOStream &os )
{
	ProcessMessage::writeImpl( os );
	os << cpu_ << mem_;
}

void ProcessStatsMessage::readImpl( BinaryIStream &is )
{
	ProcessMessage::readImpl( is );
	is >> cpu_ >> mem_;
}

const char *ProcessStatsMessage::c_str() const
{
	bw_snprintf( MachineGuardMessage::s_buf_, sizeof(MachineGuardMessage::s_buf_),
		"%s (%d)",
		name_.c_str(), pid_ );
	return MachineGuardMessage::s_buf_;
}

// -----------------------------------------------------------------------------
// Section: ListenerMessage
// -----------------------------------------------------------------------------

void ListenerMessage::writeImpl( BinaryOStream &os )
{
	ProcessMessage::writeImpl( os );
	os << preAddr_ << postAddr_;
}

void ListenerMessage::readImpl( BinaryIStream &is )
{
	ProcessMessage::readImpl( is );
	is >> preAddr_ >> postAddr_;
}

const char *ListenerMessage::c_str() const
{
	if (param_ == (this->PARAM_IS_MSGTYPE | this->ADD_BIRTH_LISTENER))
	{
		bw_snprintf( MachineGuardMessage::s_buf_, sizeof(MachineGuardMessage::s_buf_),
			"RegBirthL:%s",
			name_.c_str() );
	}
	else
	{
		bw_snprintf( MachineGuardMessage::s_buf_, sizeof(MachineGuardMessage::s_buf_),
			"RegDeathL:%s",
			name_.c_str() );
	}

	return MachineGuardMessage::s_buf_;
}

// -----------------------------------------------------------------------------
// Section: CreateMessage
// -----------------------------------------------------------------------------

void CreateMessage::writeImpl( BinaryOStream &os )
{
	MachineGuardMessage::writeImpl( os );
	os << name_ << config_ << uid_ << recover_ << fwdIp_ << fwdPort_;
}

void CreateMessage::readImpl( BinaryIStream &is )
{
	MachineGuardMessage::readImpl( is );
	is >> name_ >> config_ >> uid_ >> recover_ >> fwdIp_ >> fwdPort_;
}

const char *CreateMessage::c_str() const
{
	return this->c_str_name( "CreateMessage" );
}

const char *CreateMessage::c_str_name( const char * className ) const
{
	bw_snprintf( MachineGuardMessage::s_buf_, sizeof(MachineGuardMessage::s_buf_),
		"%s: %s/%s (uid:%d)",
		className, config_.c_str(), name_.c_str(), uid_ );
	return MachineGuardMessage::s_buf_;
}

// -----------------------------------------------------------------------------
// Section: CreateWithArgsMessage
// -----------------------------------------------------------------------------

void CreateWithArgsMessage::writeImpl( BinaryOStream &os )
{
	CreateMessage::writeImpl( os );

	os << uint32( args_.size() );
	for ( Args::const_iterator i = args_.begin(); i != args_.end(); ++i )
	{
		os << *i;
	}
}

void CreateWithArgsMessage::readImpl( BinaryIStream &is )
{
	CreateMessage::readImpl( is );

	uint32 numArgs;
	is >> numArgs;
	args_.resize( numArgs );
	for ( uint32 i = 0; i < numArgs; ++i )
	{
		is >> args_[i];
	}
}

const char *CreateWithArgsMessage::c_str() const
{
	std::stringstream ss;
	ss << CreateMessage::c_str_name( "CreateWithArgsMessage" );
	ss << " args=[";
	Args::const_iterator i = args_.begin();
	if (i != args_.end())
	{
		ss << *i;
		for ( ++i; i != args_.end(); ++i )
		{
			ss << ',' << *i;
		}
	}
	ss << ']';

	strncpy( s_buf_, ss.str().c_str(),
			sizeof(MachineGuardMessage::s_buf_) - 1 );
	s_buf_[ sizeof(MachineGuardMessage::s_buf_) - 1 ] = '\0';

	return MachineGuardMessage::s_buf_;
}


// -----------------------------------------------------------------------------
// Section: SignalMessage
// -----------------------------------------------------------------------------

void SignalMessage::writeImpl( BinaryOStream &os )
{
	ProcessMessage::writeImpl( os );
	os << signal_;
}

void SignalMessage::readImpl( BinaryIStream &is )
{
	ProcessMessage::readImpl( is );
	is >> signal_;
}

const char *SignalMessage::c_str() const
{
	bw_snprintf( MachineGuardMessage::s_buf_, sizeof(MachineGuardMessage::s_buf_), "SignalMessage: %d", signal_ );
	return MachineGuardMessage::s_buf_;
}

// -----------------------------------------------------------------------------
// Section: TagsMessage
// -----------------------------------------------------------------------------

void TagsMessage::writeImpl( BinaryOStream &os )
{
	MachineGuardMessage::writeImpl( os );
	os << (uint8)tags_.size() << exists_;
	for (unsigned i=0; i < tags_.size(); i++)
		os << tags_[i];
}

void TagsMessage::readImpl( BinaryIStream &is )
{
	MachineGuardMessage::readImpl( is );
	uint8 tagCount;
	is >> tagCount >> exists_;

	std::string s;
	for (uint8 i=0; i < tagCount; i++)
	{
		is >> s;
		tags_.push_back( s );
	}
}

const char *TagsMessage::c_str() const
{
	bw_snprintf( MachineGuardMessage::s_buf_, sizeof(MachineGuardMessage::s_buf_), "TagsMessage" );
	return MachineGuardMessage::s_buf_;
}

// -----------------------------------------------------------------------------
// Section: UserMessage
// -----------------------------------------------------------------------------

BinaryIStream& operator>>( BinaryIStream &is, UserMessage::CoreDump &cd )
{
	is >> cd.filename_ >> cd.assert_ >> cd.time_;
	return is;
}

BinaryOStream& operator<<( BinaryOStream &os, const UserMessage::CoreDump &cd )
{
	os << cd.filename_ << cd.assert_ << cd.time_;
	return os;
}

void UserMessage::writeImpl( BinaryOStream &os )
{
	MachineGuardMessage::writeImpl( os );
	os << param_ << uid_ << username_ << fullname_ <<
		home_ << mfroot_ << bwrespath_;

	// We only report coredumps when specifically requested
	static std::vector< int > empty;
	if (param_ & PARAM_CHECK_COREDUMPS)
		os << coredumps_;
	else
		os << empty;
}

void UserMessage::readImpl( BinaryIStream &is )
{
	MachineGuardMessage::readImpl( is );
	is >> param_ >> uid_ >> username_ >> fullname_ >>
		home_ >> mfroot_ >> bwrespath_ >> coredumps_;
}

const char *UserMessage::c_str() const
{
	bw_snprintf( MachineGuardMessage::s_buf_, sizeof(MachineGuardMessage::s_buf_), "UserMessage: %s:%d",
		username_.c_str(), uid_ );
	return MachineGuardMessage::s_buf_;
}

#if !defined( _WIN32 ) && !defined( PLAYSTATION3 )
void UserMessage::init( const struct passwd &pwent )
{
	uid_ = pwent.pw_uid;
	gid_ = pwent.pw_gid;
	username_ = pwent.pw_name;
	fullname_ = pwent.pw_gecos;
	home_ = pwent.pw_dir;
}
#endif

bool UserMessage::matches( const UserMessage &query ) const
{
	if (query.param_ & query.PARAM_USE_UID && query.uid_ != uid_)
		return false;
	if (query.param_ & query.PARAM_USE_NAME && query.username_ != username_)
		return false;
	return true;
}

const char * UserMessage::getConfFilename() const
{
	static char buf[ 256 ];
	bw_snprintf( buf, sizeof( buf ), "%s/.bwmachined.conf", home_.c_str() );;
	return buf;
}

// -----------------------------------------------------------------------------
// Section: ResetMessage
// -----------------------------------------------------------------------------

void ResetMessage::writeImpl( BinaryOStream &os )
{
	MachineGuardMessage::writeImpl( os );
}

void ResetMessage::readImpl( BinaryIStream &is )
{
	MachineGuardMessage::readImpl( is );
}

// -----------------------------------------------------------------------------
// Section: ErrorMessage
// -----------------------------------------------------------------------------

void ErrorMessage::writeImpl( BinaryOStream &os )
{
	MachineGuardMessage::writeImpl( os );
	os << severity_ << msg_;
}

void ErrorMessage::readImpl( BinaryIStream &is )
{
	MachineGuardMessage::readImpl( is );
	is >> severity_ >> msg_;
}

const char *ErrorMessage::c_str() const
{
	bw_snprintf( s_buf_, sizeof(s_buf_), "%s: %s", messagePrefix( (DebugMessagePriority)severity_ ),
		msg_.c_str() );

	return s_buf_;
}

// -----------------------------------------------------------------------------
// Section: MachinedAnnounceMessage
// -----------------------------------------------------------------------------

void MachinedAnnounceMessage::writeImpl( BinaryOStream &os )
{
	MachineGuardMessage::writeImpl( os );
	os << type_ << count_;
}

void MachinedAnnounceMessage::readImpl( BinaryIStream &is )
{
	MachineGuardMessage::readImpl( is );
	is >> type_ >> count_;
}

const char *MachinedAnnounceMessage::c_str() const
{
	if (type_ == this->ANNOUNCE_BIRTH)
	{
		bw_snprintf( MachineGuardMessage::s_buf_, sizeof(MachineGuardMessage::s_buf_), "MachinedBirth: %d", count_ );
	}
	else
	{
		bw_snprintf( MachineGuardMessage::s_buf_, sizeof(MachineGuardMessage::s_buf_), "MachinedDeath: %s",
			inet_ntoa( (in_addr&)addr_ ) );
	}

	return MachineGuardMessage::s_buf_;
}

// -----------------------------------------------------------------------------
// Section: PidMessage
// -----------------------------------------------------------------------------

void PidMessage::writeImpl( BinaryOStream &os )
{
	MachineGuardMessage::writeImpl( os );
	os << pid_ << running_;
}

void PidMessage::readImpl( BinaryIStream &is )
{
	MachineGuardMessage::readImpl( is );
	is >> pid_ >> running_;
}

const char *PidMessage::c_str() const
{
	bw_snprintf( MachineGuardMessage::s_buf_, sizeof(MachineGuardMessage::s_buf_), "PidMessage: %d %d", pid_,
		running_ );
	return MachineGuardMessage::s_buf_;
}

// -----------------------------------------------------------------------------
// Section: UnknownMessage
// -----------------------------------------------------------------------------

UnknownMessage::~UnknownMessage()
{
	delete [] data_;
}

void UnknownMessage::writeImpl( BinaryOStream &os )
{
	MachineGuardMessage::writeImpl( os );
	os.appendString( data_, len_ );
}

void UnknownMessage::readImpl( BinaryIStream &is )
{
	MachineGuardMessage::readImpl( is );
	len_ = is.remainingLength();
	delete [] data_;
	data_ = new char[ len_ ];
	strncpy( data_, (char*)is.retrieve( len_ ), len_ );
	flags_ |= MESSAGE_NOT_UNDERSTOOD;
}

const char *UnknownMessage::c_str() const
{
	bw_snprintf( MachineGuardMessage::s_buf_, sizeof(MachineGuardMessage::s_buf_),
		"id %d, %d bytes long", message_, len_ );
	return MachineGuardMessage::s_buf_;
}
