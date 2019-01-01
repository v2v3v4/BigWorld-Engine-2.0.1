/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPEEDTREE_CONFIG_HPP
#define SPEEDTREE_CONFIG_HPP

#include "cstdmf/timestamp.hpp"

#include <string>

#include "speedtree_config_lite.hpp"

// Link speedtreert library
#if SPEEDTREE_SUPPORT 
	#if defined(_MSC_VER) && _MSC_VER >= 1300
		#ifdef _DEBUG
			#pragma comment(lib, "SpeedTreeRT_Static_Eval_Yup_d.lib")
		#else
			#pragma comment(lib, "SpeedTreeRT_Static_Eval_Yup.lib")
		#endif
	#else
		#ifdef _DEBUG
			#pragma comment(lib, "SpeedTreeRT_Eval_Yup_Static_d.lib")
		#else
			#pragma comment(lib, "SpeedTreeRT_Eval_Yup_Static.lib")
		#endif
	#endif
#endif

// set ENABLE_PROFILE_OUTPUT to 1 to 
// enable output of profiling messages
#define ENABLE_PROFILE_OUTPUT 0

#if ENABLE_PROFILE_OUTPUT
	#define START_TIMER(timer)                    \
		uint64 timer##timer = timestamp()
	#define STOP_TIMER(timer, message)            \
		timer##timer = timestamp() - timer##timer;\
		std::stringstream stream##timer;          \
		stream##timer << NiceTime(timer##timer);  \
		INFO_MSG(	                              \
			"%s took %s (%s)\n", #timer,          \
			stream##timer.str().c_str(), message)
#else
	#define START_TIMER(timer)
	#define STOP_TIMER(timer, message)
#endif

namespace speedtree {

// Error callback
typedef void(*ErrorCallbackFunc)(const std::string &, const std::string &);

void setErrorCallback(ErrorCallbackFunc callback);
void speedtreeError(const char * treeFilename, const char * errorMsg);

} // namespace speedtree
#endif // SPEEDTREE_CONFIG_HPP
