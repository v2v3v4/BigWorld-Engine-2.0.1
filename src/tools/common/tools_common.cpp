/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "tools_common.hpp"
#include "compile_time.hpp"

#include "cstdmf/message_box.hpp"
#include "resmgr/string_provider.hpp"


//Uncomment the following line to build the evaluation version...
//#define BW_EVALUATION 1
static const int DEADDATE = 20100901;
/**
 *	This function returns whether or not the tool can run.
 */
/*static*/ bool ToolsCommon::canRun()
{
	return true;
}

/*static*/ bool ToolsCommon::isEval()
{
	return false;
}

#include <windows.h>

/*static*/ void ToolsCommon::outOfDateMessage( const std::string& toolName )
{
	BW_GUARD;

	MessageBox( NULL, 
		Localise(L"COMMON/TOOLS_COMMON/EXPIRED", toolName, aboutCompileTimeString ), 
		Localise(L"COMMON/TOOLS_COMMON/TOOL_EXPIRED"),
		MB_OK | MB_ICONERROR );
}
