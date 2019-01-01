/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MESSAGE_TIME_PREFIX_HPP
#define MESSAGE_TIME_PREFIX_HPP


#include "cstdmf/dprintf.hpp"


/**
 *	This functor is called when any message is printed
 */
struct MessageTimePrefix : public DebugMessageCallback
{
	bool handleMessage( int componentPriority,
		int messagePriority, const char * format, va_list /*argPtr*/ );
};


#endif // MESSAGE_TIME_PREFIX_HPP


// message_time_prefix.hpp
