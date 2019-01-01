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
	#undef TEST_INTERFACE_HPP
#endif

#ifndef TEST_INTERFACE_HPP
#define TEST_INTERFACE_HPP

#include "network/channel.hpp"
#include "network/interface_macros.hpp"

#define BW_COMMON_MSG( NAME, TYPE )										\
	BEGIN_HANDLED_STRUCT_MESSAGE( NAME,									\
		TYPE##StructMessageHandler< TYPE##Interface::NAME##Args >,		\
		&Channel##TYPE##App::NAME )										\

#define BW_SERVER_MSG( NAME ) BW_COMMON_MSG( NAME, Server )
#define BW_CLIENT_MSG( NAME ) BW_COMMON_MSG( NAME, Client )


// -----------------------------------------------------------------------------
// Section: Interior interface
// -----------------------------------------------------------------------------

#pragma pack(push,1)
BEGIN_MERCURY_INTERFACE( ServerInterface )

	BW_SERVER_MSG( msg1 )
		Mercury::Channel::Traits traits;
		uint32	seq;
		uint32	data;
	END_STRUCT_MESSAGE()

	BW_SERVER_MSG( disconnect )
		uint32 seq;
	END_STRUCT_MESSAGE()

END_MERCURY_INTERFACE()

BEGIN_MERCURY_INTERFACE( ClientInterface )

	BW_CLIENT_MSG( msg1 )
		uint32	seq;
		uint32	data;
	END_STRUCT_MESSAGE()

END_MERCURY_INTERFACE()


#pragma pack(pop)

#undef BW_COMMON_MSG
#undef BW_SERVER_MSG
#undef BW_CLIENT_MSG

#endif // TEST_INTERFACE_HPP
