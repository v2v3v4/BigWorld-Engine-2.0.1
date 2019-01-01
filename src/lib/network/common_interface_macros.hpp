/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#undef CREATE_TYPEDEF_WORKAROUND
#undef CREATE_TYPEDEF_WORKAROUND_EX
#undef USE_TYPEDEF_WORKAROUND

#ifdef DEFINE_SERVER_HERE

// The following macros get around the problem with putting an argument in
// a macro that has a comma in it. In this case, the template class.
#define CREATE_TYPEDEF_WORKAROUND( TYPE, NAME )								\
	typedef StructMessageHandler< TYPE, INTERFACE_NAME::NAME##Args >		\
													TYPE##_##NAME##_Handler;

#define CREATE_TYPEDEF_WORKAROUND_EX( TYPE, NAME )							\
	typedef StructMessageHandlerEx< TYPE, INTERFACE_NAME::NAME##Args >		\
													TYPE##_##NAME##_Handler;
#define USE_TYPEDEF_WORKAROUND( TYPE, NAME )								\
		TYPE##_##NAME##_Handler
#else

#define CREATE_TYPEDEF_WORKAROUND( TYPE, NAME )
#define CREATE_TYPEDEF_WORKAROUND_EX( TYPE, NAME )
#define USE_TYPEDEF_WORKAROUND( TYPE, NAME )

#endif

#undef BW_BEGIN_STRUCT_MSG
#define BW_BEGIN_STRUCT_MSG( TYPE, NAME )									\
	CREATE_TYPEDEF_WORKAROUND( TYPE, NAME )									\
	BEGIN_HANDLED_STRUCT_MESSAGE( NAME,										\
		USE_TYPEDEF_WORKAROUND( TYPE, NAME ),								\
		&TYPE::NAME )

#undef BW_BEGIN_STRUCT_MSG_EX
#define BW_BEGIN_STRUCT_MSG_EX( TYPE, NAME )								\
	CREATE_TYPEDEF_WORKAROUND_EX( TYPE, NAME )								\
	BEGIN_HANDLED_STRUCT_MESSAGE( NAME,										\
		USE_TYPEDEF_WORKAROUND( TYPE, NAME ),								\
		&TYPE::NAME )

#define BW_STREAM_MSG( TYPE, NAME ) 										\
	MERCURY_HANDLED_VARIABLE_MESSAGE( NAME, 2,								\
			StreamMessageHandler< TYPE >,									\
			&TYPE::NAME )

#define BW_STREAM_MSG_EX( TYPE, NAME ) 										\
	MERCURY_HANDLED_VARIABLE_MESSAGE( NAME, 2,								\
			StreamMessageHandlerEx< TYPE >,									\
			&TYPE::NAME )


#define BW_BIG_STREAM_MSG( TYPE, NAME ) 									\
	MERCURY_HANDLED_VARIABLE_MESSAGE( NAME, 4,								\
			StreamMessageHandler< TYPE >,									\
			&TYPE::NAME )

#define BW_BIG_STREAM_MSG_EX( TYPE, NAME ) 									\
	MERCURY_HANDLED_VARIABLE_MESSAGE( NAME, 4,								\
			StreamMessageHandlerEx< TYPE >,									\
			&TYPE::NAME )


#include "network/interface_macros.hpp"

// common_interface_macros.hpp
