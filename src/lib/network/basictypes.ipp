/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// This file contains type definitions shared by the client and the server.

/**
 * This is the default constructor for Address. It does not
 * initialise the members.
 */
inline Mercury::Address::Address()
{
}

/**
 * This constructor initialises the address and port.
 * @param ipArg		The IP address.
 * @param portArg	The port.
 */
inline Mercury::Address::Address(uint32 ipArg, uint16 portArg) :
	ip(ipArg),
	port(portArg),
	salt(0)
{
}

/**
 * 	This operator compares two addresses. It returns true
 * 	if the IP addresses and ports match.
 *
 * 	@return true if the addresses are equal, false otherwise.
 */
inline bool Mercury::operator==(const Mercury::Address & a, const Mercury::Address & b)
{
	return (a.ip == b.ip) && (a.port == b.port);
}

/**
 * 	This operator compares two addresses. It returns true
 * 	if the IP addresses and ports do not match.
 *
 * 	@return true if the addresses are non-equal, false otherwise.
 */
inline bool Mercury::operator!=(const Mercury::Address & a, const Mercury::Address & b)
{
	return (a.ip != b.ip) || (a.port != b.port);
}

/**
 * 	This operator compares two addresses. It is needed
 * 	for using an address as a key in an STL map.
 *
 * 	@return true if a is less than b, false otherwise.
 */
inline bool Mercury::operator<(const Mercury::Address & a, const Mercury::Address & b)
{
	return (a.ip < b.ip) || (a.ip == b.ip && (a.port < b.port));
}


/**
 * This is the default constructor for Capabilities.
 */
inline Capabilities::Capabilities() :
	caps_( 0 )
{
}

/**
 *	This method sets the capability with the given index
 *	
 *	@param cap	Index of the capability to add.
 *
 *	@note	Caps lager than s_maxCap_ will be ignored.
 */
inline void Capabilities::add( uint cap )
{
	caps_ |= 1 << cap;
}

/**
 * This method returns true if the given capability is present.
 *
 * @param cap	Index of the capability to check.
 * @return		True if the capability is present, false otherwise.
 */
inline bool Capabilities::has( uint cap ) const
{
	return !!(caps_ & (1 << cap));
}

/**
 *	Returns true if no caps have been set.
 *
 *	@return	true if no caps are set.
 */
inline bool Capabilities::empty() const
{
	return caps_ == 0;
}

/**
 * This method checks whether the given capabilities are on/off.
 *
 * @param on	Capabilities that should be on.
 * @param off	Capabilities that should be off.
 * @return		True if the given capabilities match those that are set.
 */
inline bool Capabilities::match( const Capabilities& on,
		const Capabilities& off ) const
{
	return (on.caps_ & caps_ ) == on.caps_ &&
		(off.caps_ & ~caps_) == off.caps_;
}


/**
 * This method checks whether any of the given capabilities are on/off.
 *
 * @param on	Capabilities, one of which should be on.
 * @param off	Capabilities, one of which should be off.
 * @return		True if the given capabilities match any of those set.
 */
inline bool Capabilities::matchAny( const Capabilities& on,
		const Capabilities& off ) const
{
	return !!(on.caps_ & caps_ ) && (off.caps_ & ~caps_) == off.caps_;
}



/**
 * 	This operator compares two space data entry ids.
 */
inline bool operator==(const SpaceEntryID & a, const SpaceEntryID & b)
{
	return Mercury::operator==( a, b ) && a.salt == b.salt;
}

/**
 * 	This operator compares two space data entry ids.
 */
inline bool operator!=(const SpaceEntryID & a, const SpaceEntryID & b)
{
	return Mercury::operator!=( a, b ) || a.salt != b.salt;
}

/**
 * 	This operator compares two space data entry ids.
 */
inline bool operator<(const SpaceEntryID & a, const SpaceEntryID & b)
{
	return Mercury::operator<( a, b ) ||
		(Mercury::operator==( a, b ) && (a.salt < b.salt));
}

// basictypes.ipp
