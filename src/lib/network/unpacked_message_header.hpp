/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef UNPACKED_MESSAGE_HEADER_HPP
#define UNPACKED_MESSAGE_HEADER_HPP

#include "misc.hpp"

namespace Mercury
{
class Channel;
class NetworkInterface;

/**
 * 	This structure is returned by Mercury when delivering
 * 	messages to a client.
 *
 * 	@ingroup mercury
 */
class UnpackedMessageHeader
{
public:
	static const int NO_REPLY = -1;

	MessageID		identifier;		///< The message identifier.
	char			flags;			///< Message header flags.
	ReplyID			replyID;		///< A unique ID, used for replying.
	int				length;			///< The number of bytes in this message.
	bool * 			pBreakLoop;		///< Used to break bundle processing.
	InterfaceElement *	pInterfaceElement;	///< The interface element for this message.
	Channel *		pChannel;		///< The channel that received this message.
	NetworkInterface * pInterface;  ///< The interface this message came on.

	UnpackedMessageHeader() :
		identifier( 0 ), flags( 0 ),
		replyID( REPLY_ID_NONE ), length( 0 ),
		pBreakLoop( NULL ), pInterfaceElement( NULL ), pChannel( NULL ),
		pInterface( NULL )
	{}

	const char * msgName() const;

	void breakBundleLoop() const
	{
		if (pBreakLoop)
		{
			*pBreakLoop = true;
		}
	}

private:
};

} // namespace Mercury

#endif // UNPACKED_MESSAGE_HEADER_HPP
