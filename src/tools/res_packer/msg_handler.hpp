/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __MSG_HANDLER_HPP__
#define __MSG_HANDLER_HPP__

#include "cstdmf/dprintf.hpp"

/**
 *	This class filters normal and critical messages, printing only warning, 
 *	errors and critical messages. If an error or critical message is received
 *	the internal errorsOccurred_ flag is set.
 */
class MsgHandler: public DebugMessageCallback, public CriticalMessageCallback
{
public:
	MsgHandler();
	virtual ~MsgHandler();

	// normal DebugMessageCallback message handler
	bool handleMessage( int componentPriority, int messagePriority, 
		const char * format, va_list argPtr );

	// critical CriticalMessageCallback handler
	void handleCritical( const char * msg );

	bool errorsOccurred() const;

private:
	bool errorsOccurred_;
};


#endif // __MSG_HANDLER_HPP__
