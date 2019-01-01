/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BLOCKING_RESPONSE_HANDLER_HPP
#define BLOCKING_RESPONSE_HANDLER_HPP

#include "network/blocking_reply_handler.hpp"

#include "pyscript/script.hpp"

class BinaryIStream;
class MethodDescription;

namespace Mercury
{
	class Address;
	class NetworkInterface;
	class UnpackedMessageHeader;
}

/**
 *	Helper class for blocking return-value reply message handling.
 *	Puts return values into a dictionary object.
 *
 *	@ingroup entity
 */
class BlockingResponseHandler: public Mercury::BlockingReplyHandler
{
public:
	BlockingResponseHandler(
			const MethodDescription & methodDesc,
			Mercury::NetworkInterface & networkInterface );

	virtual ~BlockingResponseHandler();

	PyObjectPtr getDict();

protected: // from BlockingReplyHandler

	virtual void onMessage( const Mercury::Address & source,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data,
		void * arg  );

private:
	const MethodDescription & 		methodDesc_;
	PyObjectPtr 					pReturnValueDict_;
};

#endif // BLOCKING_RESPONSE_HANDLER_HPP
