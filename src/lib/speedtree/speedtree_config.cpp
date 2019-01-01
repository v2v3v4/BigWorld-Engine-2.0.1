/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// Module Interface
#include "speedtree_config.hpp"

// BW Tech Hearders
#include "cstdmf/debug.hpp"
#include "cstdmf/guard.hpp"

DECLARE_DEBUG_COMPONENT2("SpeedTree", 0)

namespace speedtree {

namespace { // anonymous
ErrorCallbackFunc f_errorCallback = NULL;
} // namespace anonymous

// -------------------------------------------------------------- Error Callback

void setErrorCallback(ErrorCallbackFunc callback)
{
	BW_GUARD;
	f_errorCallback = callback;
}


void speedtreeError(const char * treeFilename, const char * errorMsg)
{
	BW_GUARD;
	if (f_errorCallback != NULL)
	{
		f_errorCallback(treeFilename, errorMsg);
	}
	else
	{
		ERROR_MSG("%s: %s\n", treeFilename, errorMsg);
	}
}

} // namespace speedtree
