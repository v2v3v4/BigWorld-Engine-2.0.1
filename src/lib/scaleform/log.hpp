/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCALEFORM_LOG_HPP
#define SCALEFORM_LOG_HPP
#if SCALEFORM_SUPPORT

#include <GFxLog.h>

namespace Scaleform
{
	typedef void (*LOGFN)(const char*);
	const uint32 MAXLINE = 256;

	/**
	 *	This class subclasses the GFxLog class, and provides integration
	 *	between GFx logging and BW logging services.
	 */
	class Log : public GFxLog
	{
	public:
		Log();
		~Log();

		typedef Log This;

		void SetLogHandler(LOGFN callback);

		//GFXLog virtual fns
		void LogMessageVarg(LogMessageType messageType, const char* pfmt, va_list argList);

	private:
		LOGFN logCallback_;
	};
} //namespace Scaleform

#endif // #if SCALEFORM_SUPPORT
#endif // SCALEFORM_LOG_HPP