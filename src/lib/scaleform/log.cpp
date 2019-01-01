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
#if SCALEFORM_SUPPORT
#include "log.hpp"

DECLARE_DEBUG_COMPONENT2( "Scaleform", 0 )

namespace Scaleform
{
	Log::Log():
		logCallback_( NULL )
	{
		BW_GUARD;
	}


	Log::~Log()
	{

	}


	void Log::SetLogHandler(LOGFN callback)
	{
		BW_GUARD;
		logCallback_ = callback;
	}


	void Log::LogMessageVarg(GFxLog::LogMessageType messageType, const char* pfmt, va_list argList)
	{
		BW_GUARD;
		if ( messageType & GFxLogConstants::Log_MessageType_Warning )
			WARNING_MSG( pfmt, argList );
		else if ( messageType & GFxLogConstants::Log_MessageType_Message )
			INFO_MSG( pfmt, argList );
		else
			ERROR_MSG( pfmt, argList );
	}
} // namespace Scaleform
#endif //#if SCALEFORM_SUPPORT