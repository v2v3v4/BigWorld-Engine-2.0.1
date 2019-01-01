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


namespace Mercury
{

// -----------------------------------------------------------------------------
// Section: Bundle
// -----------------------------------------------------------------------------

/**
 * Sets the default reliability policy of the current message.
 */
INLINE void Bundle::reliable( ReliableType currentMsgReliable )
{
	msgIsReliable_ = currentMsgReliable.isReliable();
	reliableDriver_ |= currentMsgReliable.isDriver();
}


/**
 *  Returns true if this bundle has any reliable driver messages on it.
 */
INLINE bool Bundle::isReliable() const
{
	return reliableDriver_;
}


/**
 * Make using simple messages easier - returns a pointer the
 * size of the message (note: not all fixed length msgs will
 * be simple structs, so startMessage doesn't do it
 * automatically)
 */
INLINE void * Bundle::startStructMessage( const InterfaceElement & ie,
	ReliableType reliable )
{
	//MF_ASSERT( ie.lengthStyle == kFixedLengthMessage );
	startMessage( ie, reliable );
	return qreserve( ie.lengthParam() );
}

/**
 * Make using simple requests easier - returns a pointer the
 * size of the request message.
 */
INLINE void * Bundle::startStructRequest( const InterfaceElement & ie,
	ReplyMessageHandler * handler, void * arg,
	int timeout, ReliableType reliable)
{
	//MF_ASSERT( ie.lengthStyle == kFixedLengthMessage );
	startRequest( ie, handler, arg, timeout, reliable );
	return qreserve( ie.lengthParam() );
}

/**
 * Return the number of free bytes remaining in the current packet.
 */
INLINE int Bundle::freeBytesInPacket()
{
	return currentPacket_->freeSpace();
}

/**
 *	This method returns whether or not this bundle has interesting footers to
 *	send.
 */
INLINE bool Bundle::hasDataFooters() const
{
	for (const Packet * p = firstPacket_.getObject(); p; p = p->next())
	{
		if (p->hasFlags( Packet::FLAG_HAS_ACKS ) ||
			p->hasFlags( Packet::FLAG_HAS_PIGGYBACKS ))
		{
			return true;
		}
	}

	return false;
}

/**
 * 	This method reserves the given number of bytes in this bundle.
 */
INLINE void * Bundle::reserve( int nBytes )
{
	return qreserve( nBytes );
}

/**
 * 	This method gets a pointer to this many bytes quickly
 * 	(non-virtual function)
 */
INLINE void * Bundle::qreserve( int nBytes )
{
	if (nBytes <= currentPacket_->freeSpace())
	{
		void * writePosition = currentPacket_->back();
		currentPacket_->grow( nBytes );
		return writePosition;
	}
	else
	{
		return this->sreserve( nBytes );
	}
}


/**
 *	This convenience method is used to add a block of memory to this stream.
 */
INLINE
void Bundle::addBlob( const void * pBlob, int size )
{
	const char * pCurr = (const char *)pBlob;

	while (size > 0)
	{
		// If there isn't any more space on this packet, force a new one to be
		// allocated to this bundle.
		if (this->freeBytesInPacket() == 0)
		{
			this->sreserve( 0 );
		}

		int currSize = std::min( size, int( this->freeBytesInPacket() ) );
		MF_ASSERT( currSize > 0 );

		memcpy( this->qreserve( currSize ), pCurr, currSize );
		size -= currSize;
		pCurr += currSize;
	}
}

} // namespace Mercury

// bundle.ipp
