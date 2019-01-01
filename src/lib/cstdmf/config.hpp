/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONFIG_HPP
#define CONFIG_HPP


#if defined( MF_SERVER ) && defined( CONSUMER_CLIENT )
#	error "The CONSUMER_CLIENT macro should not be used when building the server."
#endif

/**
 * This define is used to control the conditional compilation of features 
 * that will be removed from the client builds provided to the public.
 * Examples of these would be the in-game Python console and the Watcher 
 * Nub interface.
 *
 * If CONSUMER_CLIENT_BUILD is equal to zero then the array of development features should be compiled in.
 * If CONSUMER_CLIENT_BUILD is equal to one then development and maintenance only features should be excluded.
 */
#ifdef CONSUMER_CLIENT
#	define CONSUMER_CLIENT_BUILD						1
#else
#	define CONSUMER_CLIENT_BUILD						0
#endif


/**
 * These settings enable have to be manually turned on or off (no matter whether we're
 * compiling the consumer client build or not).
 */
#define ENABLE_RESOURCE_COUNTERS					0
#define ENABLE_CULLING_HUD							0


/**
 * By setting one of the following FORCE_ENABLE_ defines to one then support for the
 * corresponding feature will be compiled in even on a consumer client build.
 */
#define FORCE_ENABLE_DEBUG_KEY_HANDLER				0
#define FORCE_ENABLE_MSG_LOGGING					0
#define FORCE_ENABLE_DPRINTF						0
#define FORCE_ENABLE_PYTHON_TELNET_SERVICE			0
#define FORCE_ENABLE_WATCHERS						0
#define FORCE_ENABLE_DOG_WATCHERS					0
#define FORCE_ENABLE_PROFILER						0
#define FORCE_ENABLE_ACTION_QUEUE_DEBUGGER			0
#define FORCE_ENABLE_DRAW_PORTALS					0
#define FORCE_ENABLE_DRAW_SKELETON					0
#define FORCE_ENABLE_CULLING_HUD					0
#define FORCE_ENABLE_DOC_STRINGS					0
#define FORCE_ENABLE_DIARIES						0
#define FORCE_ENABLE_DDS_GENERATION					0
#define FORCE_ENABLE_FILE_CASE_CHECKING				0
#define FORCE_ENABLE_ENVIRONMENT_SYNC				0
#define FORCE_ENABLE_ENTER_DEBUGGER_MESSAGE			0
#define FORCE_ENABLE_NVIDIA_PERFHUD					0
#define FORCE_ENABLE_PYTHON_LOG						0
#define FORCE_ENABLE_STACK_TRACKER					0

///
#define ENABLE_DEBUG_KEY_HANDLER		(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_DEBUG_KEY_HANDLER)
#define ENABLE_MSG_LOGGING				(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_MSG_LOGGING)
#define ENABLE_DPRINTF					(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_DPRINTF)
#define ENABLE_PYTHON_TELNET_SERVICE	(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_PYTHON_TELNET_SERVICE)
#define ENABLE_WATCHERS					(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_WATCHERS)
#define ENABLE_DOG_WATCHERS				(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_DOG_WATCHERS)
#define ENABLE_PROFILER					(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_PROFILER)
#define ENABLE_ACTION_QUEUE_DEBUGGER	(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_ACTION_QUEUE_DEBUGGER)
#define ENABLE_DRAW_PORTALS				(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_DRAW_PORTALS)
#define ENABLE_DRAW_SKELETON			(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_DRAW_SKELETON)
//#define ENABLE_CULLING_HUD				(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_CULLING_HUD)
#define ENABLE_DOC_STRINGS				(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_DOC_STRINGS)
#define ENABLE_DIARIES					(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_DIARIES)
#define ENABLE_DDS_GENERATION			(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_DDS_GENERATION)
#define ENABLE_FILE_CASE_CHECKING		(!BW_EXPORTER && (!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_FILE_CASE_CHECKING))
#define ENABLE_ENVIRONMENT_SYNC			(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_ENVIRONMENT_SYNC)
#define ENABLE_ENTER_DEBUGGER_MESSAGE	(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_ENTER_DEBUGGER_MESSAGE)
#define ENABLE_NVIDIA_PERFHUD			(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_NVIDIA_PERFHUD)
#define ENABLE_PYTHON_LOG				(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_PYTHON_LOG)
#define ENABLE_STACK_TRACKER			(!BW_EXPORTER && (!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_STACK_TRACKER))

/**
 *	Target specific restrictions.
 */
#if defined( PLAYSTATION3 )
#	undef  ENABLE_WATCHERS
#	define ENABLE_WATCHERS				0
#	undef  ENABLE_STACK_TRACKER
#	define ENABLE_STACK_TRACKER			0
#endif

#if defined( _XBOX360 )
#	undef  ENABLE_STACK_TRACKER
#	define ENABLE_STACK_TRACKER			0
#endif

#if defined( MF_SERVER )
#	undef  ENABLE_RESOURCE_COUNTERS
#	define ENABLE_RESOURCE_COUNTERS		0
#	undef  ENABLE_STACK_TRACKER
#	define ENABLE_STACK_TRACKER			0
#endif

#if defined( BWCLIENT_AS_PYTHON_MODULE )
#	undef  ENABLE_STACK_TRACKER
#	define ENABLE_STACK_TRACKER			0
#endif

#endif // CONFIG_HPP
