/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// history_event.ipp

#ifdef CODE_INLINE
#define INLINE	inline
#else
#define INLINE
#endif


// -----------------------------------------------------------------------------
// Section: HistoryEvent
// -----------------------------------------------------------------------------

/**
 *	Destructor.
 */
INLINE
HistoryEvent::~HistoryEvent()
{
	delete [] msg_;
}


/**
 *	This method returns the number associated with this event.
 */
INLINE
EventNumber HistoryEvent::number() const
{
	return number_;
}


/**
 *	This method adds this event to the input bundle.
 *
 *	@param bundle	The bundle to add the event to.
 */
INLINE
void HistoryEvent::addToBundle( Mercury::Bundle & bundle )
{
	if (pChangedDescription_)
	{
		pChangedDescription_->countSentToOtherClients( msgLen_ );
	}
	bundle.startMessage( msgIE_ );
	bundle.addBlob( msg_, msgLen_ );
}


/**
 *	This method returns whether or not this event represents a property state
 *	change (as opposed to a message).
 */
INLINE bool HistoryEvent::isStateChange() const
{
	return (msgIE_.id() & 0xc0) == 0xc0;
}


/**
 *	This method decides whether to send this event to a client based on the
 *	input priority threshold and detail level.
 */
INLINE bool HistoryEvent::shouldSend( float threshold, int detailLevel ) const
{
	return this->isStateChange() ?
		(detailLevel <= level_.detail) :
		(threshold < level_.priority);
}

// -----------------------------------------------------------------------------
// Section: EventHistory
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
INLINE
EventHistory::EventHistory() : trimSize_( 0 )
{
}


/**
 *	This method adds an event to the event history.
 *
 *	@param pEvent	The event to add.
 */
INLINE
void EventHistory::add( HistoryEvent * pEvent )
{
	container_.push_back( pEvent );
}

// history_event.ipp
