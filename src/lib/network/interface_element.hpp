/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef INTERFACE_ELEMENT_HPP
#define INTERFACE_ELEMENT_HPP

#include "cstdmf/debug.hpp"
#include "cstdmf/profile.hpp"
#include "cstdmf/smartpointer.hpp"

#include "misc.hpp"
#include "math/ema.hpp"

class Watcher;
typedef SmartPointer<Watcher> WatcherPtr;

namespace Mercury
{

class Bundle;
class Packet;
class InputMessageHandler;

/**
 * 	@internal
 * 	This constant indicates a fixed length message.
 * 	It is used as the lengthStyle in an InterfaceElement.
 * 	For this type of message, lengthParam is the number
 * 	of bytes in the message. This is constant for all
 * 	messages of this type.
 */
const char FIXED_LENGTH_MESSAGE = 0;

/**
 * 	@internal
 * 	This constant indicates a variable length message.
 * 	It is used as the lengthStyle in an InterfaceElement.
 * 	For this type of message, lengthParam is the number
 * 	of bytes used to store the message size. This is
 * 	constant for all messages of this type.
 */
const char VARIABLE_LENGTH_MESSAGE = 1;

/**
 * 	@internal
 * 	This constant indicates the InterfaceElement has not been initialised.
 */
const char INVALID_MESSAGE = 2;


/**
 * 	@internal
 *	This structure describes a single message within an
 *	interface. It describes how to encode and decode the
 *	header of the message, including whether the message
 *	is fixed length or variable.  For fixed length messages, it
 *	defines the length of the message. For variable length
 *	messages, it defines the number of bytes needed to
 *	express the length.
 */
class InterfaceElement
{
public:
	InterfaceElement( const char * name = "", MessageID id = 0,
			int8 lengthStyle = INVALID_MESSAGE, int lengthParam = 0,
			InputMessageHandler * pHandler = NULL );

   	void set( const char * name, MessageID id, int8 lengthStyle,
		int lengthParam )
	{
		id_ = id;
		lengthStyle_ = lengthStyle;
		lengthParam_ = lengthParam;
		name_ = name;
	}

	int headerSize() const;
	int nominalBodySize() const;
	int compressLength( void * header, int length, Bundle & bundle, 
		bool isRequest ) const;
	int expandLength( void * header, Packet * pPacket, bool isRequest ) const;
	bool canHandleLength( int length ) const
	{
		return (lengthStyle_ != VARIABLE_LENGTH_MESSAGE) ||
			(lengthParam_ >= int(sizeof( int32 ))) ||
			(length < (1<<(8*lengthParam_)) - 1);
	}

	MessageID id() const				{ return id_; }
	void id( MessageID id )				{ id_ = id; }

	int8 lengthStyle() const			{ return lengthStyle_; }
	int lengthParam() const				{ return lengthParam_; }

	const char * name() const			{ return name_; }

	const char * c_str() const;

	InputMessageHandler * pHandler() const { return pHandler_; }
	void pHandler( InputMessageHandler * pHandler ) { pHandler_ = pHandler; }

	static const InterfaceElement REPLY;

private:
	int specialCompressLength( void * header, int length,
			Bundle & bundle, bool isRequest ) const;
	int specialExpandLength( void * header, Packet * pPacket, 
		bool isRequest ) const;

protected:
	MessageID			id_; 			///< Unique message ID
	int8				lengthStyle_;	///< Fixed or variable length
	int					lengthParam_;	///< This depends on lengthStyle
	const char *		name_;			///< The name of the interface method
	InputMessageHandler * pHandler_;
};


/**
 *	This class adds statistics to InterfaceElement.
 */
class InterfaceElementWithStats : public InterfaceElement
{
public:
	/**
	 *	Constructor.
	 */
	InterfaceElementWithStats():
		maxBytesReceived_( 0 ),
		numBytesReceived_( 0 ),
		numMessagesReceived_( 0 ),
		avgBytesReceivedPerSecond_( AVERAGE_BIAS ),
		avgMessagesReceivedPerSecond_( AVERAGE_BIAS )
	{
	}

	/**
	 *	Update the averages. Should be called every second.
	 */
	void tick()
	{
		avgBytesReceivedPerSecond_.sample();
		avgMessagesReceivedPerSecond_.sample();
	}

	/**
	 *	Return the maximum number of bytes ever received for a single message
	 *	for this interface element.
	 */
	uint maxBytesReceived() const 		{ return maxBytesReceived_; }

	/**
	 *	Return the number of bytes ever received for this interface element.
	 */
	uint numBytesReceived() const		{ return numBytesReceived_; }

	/**
	 *	Return the number of messages received by this interface element.
	 */
	uint numMessagesReceived() const	{ return numMessagesReceived_; }

	/**
	 *	Return the per-second average messages received for this interface
	 *	element.
	 */
	float avgMessagesReceivedPerSecond() const
	{ return avgMessagesReceivedPerSecond_.average(); }

	/**
	 *	Return the per-second average bytes received for this interface
	 *	element.
	 */
	float avgBytesReceivedPerSecond() const
	{ return avgBytesReceivedPerSecond_.average(); }

	/**
	 *	Return the average message length for this interface element.
	 */
	float avgMessageLength() const
	{
		return numMessagesReceived_ ?
			float( numBytesReceived_ ) / float( numMessagesReceived_ ):
			0.0f;
	}

	void startProfile()
	{
#if ENABLE_WATCHERS
		profile_.start();
#endif
	}

	void stopProfile( uint32 msgLen )
	{
#if ENABLE_WATCHERS
		this->messageReceived( msgLen );
		profile_.stop( msgLen );
#endif
	}

#if ENABLE_WATCHERS
	/**
	 *	Return a watcher for monitoring the counts and averages for interface
	 *	elements.
	 */
	static WatcherPtr		pWatcher();
#endif

private:
	// Note: Needs to be int for watcher
	int idAsInt() const				{ return this->id(); }

	/**
	 *	Notification method when a message has been received for this interface element.
	 *
	 *	@param msgLen 	the message length
	 */
	void messageReceived( uint msgLen )
	{
		++numMessagesReceived_;
		++avgMessagesReceivedPerSecond_.value();
		numBytesReceived_ += msgLen;
		avgBytesReceivedPerSecond_.value() += msgLen;
		maxBytesReceived_ = std::max( msgLen, maxBytesReceived_ );
	}

	/**
	 *	The maximum bytes received for a single message for this interface
	 *	element.
	 */
	uint 		maxBytesReceived_;
	/**
	 *	The number of bytes received over all messages for this interface
	 *	element.
	 */
	uint		numBytesReceived_;

	/**
	 *	The number of messages received for this interface element.
	 */
	uint		numMessagesReceived_;

	/**
	 *	The per-second exponentially weighted moving average for bytes received
	 *	for this interface element.
	 */
	AccumulatingEMA< uint > avgBytesReceivedPerSecond_;

	/**
	 *	The per-second exponentially weighted moving average for messages
	 *	received for this interface element.
	 */
	AccumulatingEMA< uint > avgMessagesReceivedPerSecond_;

#if ENABLE_WATCHERS
	ProfileVal profile_;
#endif

	static const float AVERAGE_BIAS;
};


typedef std::vector< InterfaceElement > InterfaceElements;

/**
 * 	@internal
 * 	This message ID is reserved for reply messages. These messages
 * 	are handled by the internal Mercury NUB.
 */
const unsigned char REPLY_MESSAGE_IDENTIFIER = 0xFF;


/**
 *  __glenc__ 19/01/07: There used to be a REPLY_PIGGY_BACK_IDENTIFIER here,
 *  however since we changed to implementing piggybacking with footers instead
 *  of messages, we don't need it anymore.  A lot of the entitydef code works on
 *  the assumption that there are 2 message IDs reserved for Mercury, i.e. you
 *  can only have 62 unique properties/methods on an entity before 2 byte
 *  identifiers kick in.  We didn't alter entitydef to use
 *  REPLY_PIGGY_BACK_IDENTIFIER when we took it out as there is a lot of
 *  hardcoding in there, and it was a small optimisation for a relatively high
 *  chance of breaking something just prior to 1.8.1 being released.
 */

}

#endif // INTERFACE_ELEMENT_HPP
