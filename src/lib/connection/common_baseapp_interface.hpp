/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "network/common_interface_macros.hpp"

#undef MF_BEGIN_PROXY_MSG
#define MF_BEGIN_PROXY_MSG( NAME )											\
	BEGIN_HANDLED_STRUCT_MESSAGE( NAME,										\
			NoBlockProxyMessageHandler< INTERFACE_NAME::NAME##Args >,	\
			&Proxy::NAME )													\

#define MF_BEGIN_UNBLOCKABLE_PROXY_MSG MF_BEGIN_PROXY_MSG

#undef MF_BEGIN_BLOCKABLE_PROXY_MSG
#define MF_BEGIN_BLOCKABLE_PROXY_MSG( NAME )								\
	BEGIN_HANDLED_STRUCT_MESSAGE( NAME,										\
			ProxyMessageHandler< INTERFACE_NAME::NAME##Args >,			\
			&Proxy::NAME )													\

#define MF_VARLEN_PROXY_MSG( NAME ) 										\
	MERCURY_HANDLED_VARIABLE_MESSAGE( NAME, 2, 								\
			ProxyVarLenMessageHandler<false>, &Proxy::NAME )

#define MF_VARLEN_UNBLOCKABLE_PROXY_MSG MF_VARLEN_PROXY_MSG

#undef MF_VARLEN_BLOCKABLE_PROXY_MSG
#define MF_VARLEN_BLOCKABLE_PROXY_MSG( NAME )								\
	MERCURY_HANDLED_VARIABLE_MESSAGE( NAME, 2,								\
			ProxyVarLenMessageHandler< true >,								\
			&Proxy::NAME )

// common_baseapp_interface.hpp
