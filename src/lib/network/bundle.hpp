/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BUNDLE_HPP
#define BUNDLE_HPP

#include "basictypes.hpp"
#include "bundle_piggyback.hpp"
#include "misc.hpp"
#include "packet.hpp"
#include "reliable_order.hpp"
#include "reply_order.hpp"
#include "unpacked_message_header.hpp"
#include "interface_element.hpp"

#include "cstdmf/binary_stream.hpp"

#include <map>

class Endpoint;

namespace Mercury
{

class Bundle;
class Channel;
class InterfaceElement;
class InterfaceTable;
class NetworkInterface;
class ReplyMessageHandler;
class ProcessSocketStatsHelper;

/**
 *	@internal
 *	This is the default request timeout in microseconds.
 */
const int DEFAULT_REQUEST_TIMEOUT = 5000000;



/**
 *	There are three types of reliability. RELIABLE_PASSENGER messages will
 *	only be sent so long as there is at least one RELIABLE_DRIVER in the
 *	same Bundle.  RELIABLE_CRITICAL means the same as RELIABLE_DRIVER, however,
 *	starting a message of this type also marks the Bundle as critical.
 */
enum ReliableTypeEnum
{
	RELIABLE_NO = 0,
	RELIABLE_DRIVER = 1,
	RELIABLE_PASSENGER = 2,
	RELIABLE_CRITICAL = 3
};


/**
 *	This struct wraps a @see ReliableTypeEnum value.
 */
class ReliableType
{
public:
	ReliableType( ReliableTypeEnum e ) : e_( e ) { }

	bool isReliable() const	{ return e_ != RELIABLE_NO; }

	// Leveraging the fact that only RELIABLE_DRIVER and RELIABLE_CRITICAL share
	// the 0x1 bit.
	bool isDriver() const	{ return e_ & RELIABLE_DRIVER; }

	bool operator== (const ReliableTypeEnum e) { return e == e_; }

private:
	ReliableTypeEnum e_;
};


/**
 *	A bundle is a sequence of messages. You stream or otherwise
 *	add your messages onto the bundle. When you want to send
 *	a group of messages (possibly just one), you tell a nub
 *	to send the bundle. Bundles can be sent multiple times
 *	to different hosts, but beware that any requests inside
 *	will also be made multiple times.
 *
 *	@ingroup mercury
 */
class Bundle : public BinaryOStream
{
public:
	Bundle( uint8 spareSize = 0, Channel * pChannel = NULL );

	Bundle( Packet * packetChain );
	virtual ~Bundle();

	void clear( bool newBundle = false );
	void cancelRequests();

	bool isEmpty() const;

	int size() const;
	int sizeInPackets() const;
	bool isMultiPacket() const { return firstPacket_->next() != NULL; }
	int freeBytesInPacket();
	int numMessages() const		{ return numMessages_; }

	bool hasDataFooters() const;

	void send( const Address & address, NetworkInterface & networkInterface,
		Channel * pChannel = NULL );

	void startMessage( const InterfaceElement & ie,
		ReliableType reliable = RELIABLE_DRIVER );

	void startRequest( const InterfaceElement & ie,
		ReplyMessageHandler * handler,
		void * arg = NULL,
		int timeout = DEFAULT_REQUEST_TIMEOUT,
		ReliableType reliable = RELIABLE_DRIVER );

	void startReply( ReplyID id, ReliableType reliable = RELIABLE_DRIVER );

	void * startStructMessage( const InterfaceElement & ie,
		ReliableType reliable = RELIABLE_DRIVER );

	void * startStructRequest( const InterfaceElement & ie,
		ReplyMessageHandler * handler,
		void * arg = NULL,
		int timeout = DEFAULT_REQUEST_TIMEOUT,
		ReliableType reliable = RELIABLE_DRIVER );

	void reliable( ReliableType currentMsgReliabile );
	bool isReliable() const;

	bool isCritical() const { return isCritical_; }

	bool isOnExternalChannel() const;

	Channel * pChannel() { return pChannel_; }

	virtual void * reserve( int nBytes );
	virtual void addBlob( const void * pBlob, int size );
	INLINE void * qreserve( int nBytes );

	void reliableOrders( Packet * p,
		const ReliableOrder *& roBeg, const ReliableOrder *& roEnd );

	void writeFlags( Packet * p ) const;

#ifdef USE_PIGGIES
	bool piggyback( SeqNum seq, const ReliableVector & reliableOrders,
		Packet * p );
#endif

	Reason dispatchMessages( InterfaceTable & interfaceTable,
			const Address & addr, Channel * pChannel,
			NetworkInterface & networkInterface,
			ProcessSocketStatsHelper * pStatsHelper );

	/**
	 *	@internal
	 *	This class is used to iterate over the messages in a bundle.
	 *	Mercury uses this internally when unpacking a bundle and
	 *	delivering messages to the client.
	 */
	class iterator
	{
	public:
		iterator(Packet * first);
		iterator(const iterator & i);
		~iterator();

		const iterator & operator=( const iterator & i );

		MessageID msgID() const;

		// Note: You have to unpack before you can call
		// 'data' or 'operator++'

		UnpackedMessageHeader & unpack( const InterfaceElement & ie );
		const char * data();

		void operator++(int);
		bool operator==(const iterator & x) const;
		bool operator!=(const iterator & x) const;

	private:
		void nextPacket();

		Packet		* cursor_;
		uint16		bodyEndOffset_;
		uint16		offset_;
		uint16		dataOffset_;
		int			dataLength_;
		char		* dataBuffer_;

		uint16	nextRequestOffset_;

		UnpackedMessageHeader	curHeader_;
	};

	/**
	 * Get some iterators
	 */
	iterator begin();
	iterator end();

	/**
	 * Clean up any loose ends (called only by nub)
	 */
	void finalise();

	void dumpMessages( InterfaceTable & interfaceTable );

public:
	int addAck( SeqNum seq ) 
	{ 
		MF_ASSERT( ack_ == SEQ_NULL );
		ack_ = seq;
		return true;
	}

	SeqNum getAck() const
	{
		MF_ASSERT( ack_ != SEQ_NULL );
		return ack_;
	}

private:
	void * sreserve( int nBytes );
#ifdef USE_PIGGIES
	void reserveFooter( int nBytes, Packet::Flags flag );
#endif
	void dispose();
	void startPacket( Packet * p );
	void endPacket(bool isExtending);
	void endMessage();
	char * newMessage( int extra = 0 );
	void addReliableOrder();

	Packet::Flags packetFlags() const { return currentPacket_->flags(); }

	Bundle( const Bundle & );
	Bundle & operator=( const Bundle & );

private:
public:	// public so streaming operators work (they can't
		// be in-class due to VC's problem with template
		// member functions)

	// per bundle stuff
	PacketPtr firstPacket_;		///< The first packet in the bundle
	Packet	* currentPacket_;	///< The current packet in the bundle
	bool	finalised_;			///< True if the bundle has been finalised
	bool	reliableDriver_;	///< True if any driving reliable messages added
	uint8	extraSize_;			///< Size of extra bytes needed for e.g. filter

	ReplyOrders	replyOrders_;

	/// This vector stores all the reliable messages for this bundle.
	ReliableVector	reliableOrders_;
	int				reliableOrdersExtracted_;

	/// If true, this Bundle's packets will be considered to be 'critical' by
	/// the Channel.
	bool			isCritical_;

#ifdef USE_PIGGIES
	BundlePiggybacks piggybacks_;
#endif

private:
	/// This is the Channel that owns this Bundle, or NULL if not on a Channel.
	Channel * pChannel_;
	// Off channel acking, we only store one but we only need one
	SeqNum	ack_;

	// per message stuff
	InterfaceElement const * curIE_;
	int					msgLen_;
	int					msgExtra_;
	uint8 * 			msgBeg_;
	uint16				msgChunkOffset_;
	bool				msgIsReliable_;
	bool				msgIsRequest_;

	// Statistics
	int		numMessages_;
	int		numReliableMessages_;

};

class NetworkInterface;

/**
 *  This class is useful when you have a lot of data you want to send to a
 *  collection of other apps, but want to group the sends to each app together.
 */
class BundleSendingMap
{
public:
	BundleSendingMap( NetworkInterface & networkInterface ) :
		networkInterface_( networkInterface ) {}
	Bundle & operator[]( const Address & addr );
	void sendAll();

private:
	NetworkInterface & networkInterface_;

	typedef std::map< Address, Channel* > Channels;
	Channels channels_;
};



} // namespace Mercury

#ifdef CODE_INLINE
#include "bundle.ipp"
#endif

#endif // BUNDLE_HPP
