/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "common_interface.hpp"

#include "network/network_interface.hpp"

/**
 *	Class for struct-style Mercury message handler objects.
 */
template <class ARGS> class CommonStructMessageHandler :
	public Mercury::InputMessageHandler
{
public:
	typedef void (CommonHandler::*Handler)(
			const Mercury::Address & srcAddr,
			const ARGS & args );

	CommonStructMessageHandler( Handler handler ) :
		handler_( handler )
	{}

private:
	virtual void handleMessage( const Mercury::Address & srcAddr,
		Mercury::UnpackedMessageHeader & header, BinaryIStream & data )
	{
		ARGS * pArgs = (ARGS*)data.retrieve( sizeof(ARGS) );
		void * pData = header.pInterface->pExtensionData();

		// Make sure the pExtensionData has been set on the NetworkInterface
		// to the local handler.
		MF_ASSERT( pData );

		CommonHandler * pHandler = static_cast<CommonHandler *>( pData );
		(pHandler->*handler_)( srcAddr, *pArgs );
	}

	Handler handler_;
};

#define DEFINE_SERVER_HERE
#include "common_interface.hpp"

// common_interface.cpp
