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
	#undef LOGIN_INTERFACE_HPP
#endif

#ifndef LOGIN_INTERFACE_HPP
#define LOGIN_INTERFACE_HPP

// -----------------------------------------------------------------------------
// Section: Includes
// -----------------------------------------------------------------------------

// Everything in this section is only parsed once.

#ifndef LOGIN_INTERFACE_HPP_ONCE_ONLY
#define LOGIN_INTERFACE_HPP_ONCE_ONLY

#include "cstdmf/stdmf.hpp"

#include "network/interface_macros.hpp"

// Version 11: Pitch and roll are sent down in createEntity.
// Version 12: Added voiceData to the client interface.
// Version 13: Entity to update sent up in avatarUpdate.
// Version 14: EntityTypeID changed to a uint16.
// Version 15: Implemented spaces, including space viewports and space data.
// Version 16: setGameTime has only (server) game time. Renamed from setTime.
// Version 17: Implemented vehicles and split enterAoI into 3.
// Version 18: Upstream avatarUpdate does vehicles. Removed requestBandwidth.
// Version 19: Added cell fault tolerance.
// Version 20: Resource versioning and basic update messages.
// Version 21: Added changeProxy to client interface.
// Version 22: Add base app fault tolerance.
// Version 23: Messages for co-ordinated (live) resource updates.
// Version 24: Client (and server) authentication with a session key.
// Version 25: Player entity data from createPlayer instead of login reply.
// Version 26: Separate createBasePlayer and createCellPlayer messages.
// Version 27: Explicit pose corrections and control toggle. Removed cell ids.
// Version 28: Replaced LogOnReplyStatus with LogOnStatus.
// Version 29: Client type indices collapsed. Signed/unsigned data MD5s differ.
// Version 30: Changes to how Mercury handles once-off reliable data.
// Version 31: Changed packed float y-value format.
// Version 32: Add baseAppLogin message to fix NAT issues.
// Version 33: Added configuration option for ordering client-server channel
// Version 34: Changed login to use once-off reliability to LoginApp.
// Version 35: Added setGameTime message to BaseApp to fix restore from DB.
// Version 36: Reverted to 1472 for MTU. Added disconnectClient and loggedOff.
// Version 37: Implemented piggybacking for ordered channels.
// Version 38: Xbox 360 (i.e. big-endian) support.
// Version 39: Logging in no longer uses once-off reliability.
// Version 40: LOGIN_VERSION is now 4 bytes.
// Version 41: Piggyback length changed to ones complement.
// Version 42: Added support for fully encrypted sessions.
// Version 43: Added FLAG_HAS_CHECKSUM, packet headers are now 2 bytes.
// Version 44: No longer using RelPosRef. Removal of updater and viewport code.
// Version 45: All logins RSA encrypted and Blowfish encrypted channels
//             optional.
// Version 46: Blowfish encryption is now mandatory.
// Version 47: FLAG_FIRST_PACKET is invalid on external nubs/channels.
// Version 48: Public keys are no longer fetchable from the server.
// Version 49: Blowfish encryption now has XOR stage to prevent replay attacks.
// Version 50: Roll is now expressed with 2pi radians of freedom.
// Version 51: Preventing replay attacks with unreliable packets.
// Version 52: Added support for cumulative ACKs.
// Version 53: Login replies are no longer sent once-off reliably 
//             and client no longer accepts once off reliable packets.
// Version 54: LogOnParams no longer streams nonce twice.
// Version 55: Optimised slice changes to arrays. Bug: 15846
// Version 56: createEntity can now be compressed
const uint32 LOGIN_VERSION = 56;
const uint32 OLDEST_SUPPORTED_CLIENT_LOGIN_VERSION = 56;

// Probe reply is a list of pairs of strings
// Some strings can be interpreted as integers
#define PROBE_KEY_HOST_NAME			"hostName"
#define PROBE_KEY_OWNER_NAME		"ownerName"
#define PROBE_KEY_USERS_COUNT		"usersCount"
#define PROBE_KEY_UNIVERSE_NAME		"universeName"
#define PROBE_KEY_SPACE_NAME		"spaceName"
#define PROBE_KEY_BINARY_ID			"binaryID"

#endif // LOGIN_INTERFACE_HPP_ONCE_ONLY


// -----------------------------------------------------------------------------
// Section: Login Interface
// -----------------------------------------------------------------------------


// gLoginHandler and gProbeHandler below are only defined in 
// loginapp/messsage_handlers.cpp:

// class LoginAppRawMessageHandler;
// extern LoginAppRawMessageHandler gLoginHandler;
// extern LoginAppRawMessageHandler gProbeHandler;

#pragma pack(push,1)
BEGIN_MERCURY_INTERFACE( LoginInterface )

	// uint32 version
	// bool encrypted
	// LogOnParams
	MERCURY_VARIABLE_MESSAGE( login, 2, &gLoginHandler )

	MERCURY_FIXED_MESSAGE( probe, 0, &gProbeHandler )

END_MERCURY_INTERFACE()

#pragma pack(pop)

#endif // LOGIN_INTERFACE_HPP
