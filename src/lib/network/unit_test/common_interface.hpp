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
	#undef TEST_COMMON_INTERFACE_HPP
#endif

#ifndef TEST_COMMON_INTERFACE_HPP
#define TEST_COMMON_INTERFACE_HPP

#include "network/channel.hpp"
#include "network/interface_macros.hpp"

#define BW_COMMON_MSG( NAME )											\
	BEGIN_HANDLED_STRUCT_MESSAGE( NAME,									\
		CommonStructMessageHandler< CommonInterface::NAME##Args >,		\
		&CommonHandler::NAME )											\

// -----------------------------------------------------------------------------
// Section: Interior interface
// -----------------------------------------------------------------------------

#pragma pack(push,1)
BEGIN_MERCURY_INTERFACE( CommonInterface )

	BW_COMMON_MSG( msg1 )
		Mercury::Channel::Traits traits;
		uint32	seq;
		uint32	data;
	END_STRUCT_MESSAGE()

	BW_COMMON_MSG( disconnect )
		uint32 seq;
	END_STRUCT_MESSAGE()

END_MERCURY_INTERFACE()

#pragma pack(pop)

// -----------------------------------------------------------------------------
// Section: CommonHandler
// -----------------------------------------------------------------------------

#ifndef BW_COMMON_HANDLER
#define BW_COMMON_HANDLER

#define BW_MSG_HANDLER( NAME ) 											\
public:																	\
	void NAME( const Mercury::Address & srcAddr, 						\
			const CommonInterface::NAME##Args & args )					\
	{																	\
		this->on_##NAME( srcAddr, args );								\
	}																	\
																		\
protected:																\
	virtual void on_##NAME( const Mercury::Address & srcAddr, 			\
			const CommonInterface::NAME##Args & args ) {}				\


/**
 *	This class is a base class for handlers of CommonInterface. To support this
 *	interface, you need to do the following:
 *
 * class LocalHandler : public CommonHandler
 * {
 * 	protected:
 * 		// Implement any messages that you want to handle
 * 		void on_msg1( const Mercury::Address & srcAddr,
 * 			const CommonInterface::msg1Args & args )
 * 		{
 * 			...
 * 		}
 * };
 *
 *	NetworkInterface networkInterface( &dispatcher );
 *	LocalHandler handler;
 *	networkInterface.pExtensionData( &handler );
 */
class CommonHandler
{
	BW_MSG_HANDLER( msg1 )
	BW_MSG_HANDLER( disconnect )
};

#undef BW_MSG_HANDLER

#endif // BW_COMMON_HANDLER

#undef BW_COMMON_MSG
#undef BW_MSG

#endif // TEST_COMMON_INTERFACE_HPP
