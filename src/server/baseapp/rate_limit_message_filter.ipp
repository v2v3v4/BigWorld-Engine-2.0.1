/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifdef CODE_INLINE
    #define INLINE    inline
#else
	/// INLINE macro.
    #define INLINE
#endif

/**
 *	Creates a new BufferedMessage instance when buffering.
 *	Override this method to supply derived instances of
 *	BufferedMessage.
 *
 *	@param header 		the message header
 *	@param data 		the message data stream
 *	@param pHandler 	the destination message handler
 */
INLINE
BufferedMessage * RateLimitMessageFilter::Callback::createBufferedMessage(
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data,
		Mercury::InputMessageHandler * pHandler )
{
	return new BufferedMessage( header, data, pHandler );
}

// rate_limit_message_filter.ipp
