/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#if defined( DEFINE_INTERFACE_HERE ) || defined( DEFINE_SERVER_HERE )
	#undef LOGIN_INT_INTERFACE_HPP
#endif

#ifndef LOGIN_INT_INTERFACE_HPP
#define LOGIN_INT_INTERFACE_HPP

#include "network/interface_macros.hpp"

#include "server/anonymous_channel_client.hpp"

#ifdef MF_SERVER
#include "server/reviver_subject.hpp"
#else
#define MF_REVIVER_PING_MSG()
#endif

// -----------------------------------------------------------------------------
// Section: Interior interface
// -----------------------------------------------------------------------------

#pragma pack(push,1)
BEGIN_MERCURY_INTERFACE( LoginIntInterface )

	BW_ANONYMOUS_CHANNEL_CLIENT_MSG( DBInterface )

	MERCURY_EMPTY_MESSAGE( controlledShutDown, &gShutDownHandler )

	MF_REVIVER_PING_MSG()

END_MERCURY_INTERFACE()
#pragma pack(pop)

#endif // LOGIN__INT_INTERFACE_HPP
