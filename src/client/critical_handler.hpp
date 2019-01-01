/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CRITICAL_HANDLER_HPP
#define CRITICAL_HANDLER_HPP


#include "cstdmf/dprintf.hpp"


/**
 *	This functor is called when a critical message is printed.
 */
struct CriticalHandler : public CriticalMessageCallback
{
	void handleCritical( const char * msg );
};


#endif // CRITICAL_HANDLER_HPP


// critical_handler.hpp
