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
#include "message_time_prefix.hpp"


#include "app.hpp"


bool MessageTimePrefix::handleMessage( int componentPriority,
	int messagePriority, const char * format, va_list /*argPtr*/ )
{
	static THREADLOCAL( bool ) newLine = true;

	if (DebugFilter::shouldAccept( componentPriority, messagePriority ))
	{
		if (newLine && (&App::instance()) != NULL)
		{
			wchar_t wbuf[ 256 ];
			bw_snwprintf( wbuf, ARRAY_SIZE(wbuf), L"%.3f ", float(App::instance().getTime()) );
			OutputDebugString( wbuf );
		}

		int len = strlen(format);
		newLine = (len && format[len - 1] == '\n');
	}

	return false;
}


// message_time_prefix.cpp
