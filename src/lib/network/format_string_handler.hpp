/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FORMAT_STRING_HANDLER_HPP
#define FORMAT_STRING_HANDLER_HPP

#include "cstdmf/stdmf.hpp"

/**
 * Interface to be used by format string parsing code to provide callbacks
 * to the implementors as necessary.
 */
class FormatStringHandler
{
public:
	virtual ~FormatStringHandler( void ) {};

	/* Callback for when a full string segment has been identified
	 * in the format string. This is any part of the format string that
	 * does not comprise a token.
	 */
	virtual void onString( int start, int end ) {};

	/* Callback for when a token has been fully parsed.
	 * The implementor of this interface needs to keep track of the
	 * variable argument list to know which argument the next token
	 * is referring to
	 */
	virtual void onToken( char type, int cflags, int min, int max,
		int flags, uint8 base, int vflags ) = 0;
};

#endif /* FORMAT_STRING_HANDLER_HPP */
