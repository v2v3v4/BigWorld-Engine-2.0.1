/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __RATE_LIMIT_MESSAGE_FILTER_HPP__
#define __RATE_LIMIT_MESSAGE_FILTER_HPP__

#include "network/message_filter.hpp"
#include "network/unpacked_message_header.hpp"

#include "cstdmf/memory_stream.hpp"
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/timestamp.hpp"

#include <queue>


class BufferedMessage;
class RateLimitConfig;


/**
 *	All messages from external clients get passed through an instance of this
 *	class. It is responsible for enforcing rate limits on client messages,
 *	buffering and playing back messages when limits are no longer exceeded.
 */
class RateLimitMessageFilter : public Mercury::MessageFilter
{
public:
	typedef RateLimitConfig Config;

	/**
	 *	Callback interface for customising the behaviour of the filter. There
	 *	are hook methods for when messages are buffered, when they are
	 *	dispatched, and when the filter limits are exceeded.
	 */
	class Callback
	{

	public:
		/**
		 *	Destructor.
		 */
		virtual ~Callback()
		{}


		virtual BufferedMessage * createBufferedMessage(
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data,
			Mercury::InputMessageHandler * pHandler );


		/**
		 *	Callback for when a buffered message has been deleted from the
		 *	buffer, and not dispatched.
		 *
		 *	@param pMessage 	The buffered message.
		 */
		virtual void onMessageDeleted( BufferedMessage * pMessage )
		{}


		/**
		 *	Callback for when the buffer limit has been exceeded, such that we
		 *	cannot buffer any more messages.
		 *
		 *	@param pMessage		The offending message that was to be buffered.
		 */
		virtual void onFilterLimitsExceeded( const Mercury::Address & srcAddr,
			BufferedMessage * pMessage )
		{}
	};


public:

	RateLimitMessageFilter( const Mercury::Address & addr );

	~RateLimitMessageFilter();

public: // from Mercury::MessageFilter
	void filterMessage( const Mercury::Address & srcAddr,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data,
		Mercury::InputMessageHandler * pHandler );

public:

	void tick();


	/**
	 *	Get the address associated with this filter.
	 */
	const Mercury::Address & addr() { return addr_; }

	/**
	 *	Set the callback object for this filter.
	 */
	void setCallback( Callback * pCallback ) { pCallback_ = pCallback; }

private:

	void replayAny();

	bool canSendNow( uint dataLen );


	/**
	 *	Return the BufferedMessage at the front of the buffered message queue,
	 *	or NULL if the queue is empty.
	 */
	BufferedMessage * front()
	{ return queue_.empty() ? NULL : queue_.front(); }


	void pop_front();

	void dispatch( Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data, Mercury::InputMessageHandler * pHandler );

	void dispatch( BufferedMessage * pMessage );

	bool buffer( BufferedMessage * pMsg );


private:
	// Warn bit flag consts
	/**
	 *	Warn bit flat constant for when the number of received messages exceeds
	 *	the warning limit.
	 */
	static const uint8 WARN_MESSAGE_COUNT 		= 0x01;
	/**
	 *	Warn bit flat constant for when the size of received messages exceeds
	 *	the warning limit.
	 */
	static const uint8 WARN_MESSAGE_SIZE 		= 0x02;
	/**
	 *	Warn bit flat constant for when the number of buffered messages exceeds
	 *	the warning limit.
	 */
	static const uint8 WARN_MESSAGE_BUFFERED 	= 0x04;
	/**
	 *	Warn bit flat constant for when the size of buffered messages exceeds
	 *	the warning limit.
	 */
	static const uint8 WARN_BYTES_BUFFERED 		= 0x08;

	/**
	 *	Message queue data type.
	 */
	typedef std::queue< BufferedMessage * > MsgQueue;

	// The callback object.
	Callback * 			pCallback_;

	// The address we are associated with.
	Mercury::Address  	addr_;

	// This is a bitflag indicating whether we have warned about a particular
	// condition or not, see FilterForAddress::WARN_* constants.
	uint8				warnFlags_;

	// The number of received messages coming into the filter since we were
	// last ticked.
	uint 				numReceivedSinceLastTick_;

	// The total size of received messages coming into the filter since we were
	// last ticked.
	uint 				receivedBytesSinceLastTick_;

	// The number of dispatches since we were last ticked.
	uint 				numDispatchedSinceLastTick_;

	// The total size of the dispatched messages since we were last ticked.
	uint 				dispatchedBytesSinceLastTick_;

	// The buffer message queue.
	MsgQueue			queue_;

	// This is always the sum of all the messages in queue_.
	uint				sumBufferedSizes_;

};

typedef SmartPointer< RateLimitMessageFilter > RateLimitMessageFilterPtr;

/**
 *	This class is a buffered message instance, and stores the header, data
 *	stream and the destination message handler for deferred playback.
 *
 *	Note that the source address is not stored, as RateLimitMessageFilters are
 *	per-address already, and it supplies that source address to the dispatch
 *	method.
 *
 *	Application code can derive from BufferedMessage for associating their own
 *	state and/or overriding the dispatch method for doing any custom actions.
 *	They can override RateLimitMessageFilter::Callback::createBufferedMessage()
 *	in order to have the filter create instances of different derived classes.
 */
class BufferedMessage
{
public:
	/**
	 *	Constructor.
	 *
	 *	@param header 		The message header.
	 *	@param data 		The message data stream.
	 *	@param pHandler 	The destination message handler.
	 */
	BufferedMessage( const Mercury::UnpackedMessageHeader & header,
				BinaryIStream & data,
				Mercury::InputMessageHandler * pHandler ):
			header_( header ),
			data_( data.remainingLength() ),
			pHandler_( pHandler )
	{
		data_.transfer( data, data.remainingLength() );
	}


	/**
	 *	Destructor.
	 */
	virtual ~BufferedMessage()
	{}


	/**
	 *	Dispatch a buffered message. Subclasses of BufferedMessage can override
	 *	this method to supply custom functionality.
	 *
	 *	@param pCallback 	The filter's callback object.
	 *	@param srcAddr 		The source address of the message. This is passed
	 *						down from the per-address rate limiting filter.
	 */
	virtual void dispatch( RateLimitMessageFilter::Callback * /*pCallback*/,
			const Mercury::Address & srcAddr )
	{
		this->pHandler()->handleMessage( srcAddr, this->header(),
			this->data() );
	}

	/**
	 *	Return the data size of the message. Note that calling this after the
	 *	message has been dispatched will not return the original data size.
	 */
	uint size() const 						{ return data_.remainingLength(); }


	/**
	 *	Return the message header (const version).
	 */
	const Mercury::UnpackedMessageHeader & header() const
												{ return header_; }

	/**
	 *	Return the message header.
	 */
	Mercury::UnpackedMessageHeader & header()	{ return header_; }

protected:
	/**
	 *	Return the message data stream.
	 */
	BinaryIStream & data() 						{ return data_; }

	/**
	 *	Return the message's destination handler.
	 */
	Mercury::InputMessageHandler * pHandler() 	{ return pHandler_; }


private:
	// The message header.
	Mercury::UnpackedMessageHeader 	header_;

	// The message data.
	MemoryOStream 					data_;

	// The destination handler for this message.
	Mercury::InputMessageHandler * 	pHandler_;

};


#ifdef CODE_INLINE
#include "rate_limit_message_filter.ipp"
#endif
#endif // __RATE_LIMIT_MESSAGE_FILTER_HPP__
