/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FORWARDING_STRING_HANDLER_H
#define FORWARDING_STRING_HANDLER_H

#include <stdarg.h>
#include <vector>

#include "format_string_handler.hpp"
#include "cstdmf/memory_stream.hpp"
#include "bsd_snprintf.h"

void TestStrings();

/**
 *  This object handles log message forwarding for a single format string.
 */
class ForwardingStringHandler : public FormatStringHandler
{
public:
	ForwardingStringHandler( const char *fmt, bool isSuppressible = false );

	virtual void onToken( char type, int cflags, int min, int max,
		int flags, uint8 base, int vflags );

	void parseArgs( va_list argPtr, MemoryOStream &os );
	const std::string & fmt() const { return fmt_; }

	unsigned numRecentCalls() const { return numRecentCalls_; }
	void addRecentCall() { ++numRecentCalls_; }
	void clearRecentCalls() { numRecentCalls_ = 0; }

	bool isSuppressible() const { return isSuppressible_; }
	void isSuppressible( bool b ) { isSuppressible_ = b; }

protected:
	/**
	 *  This helper class holds the various numbers needed by the bsd_snprintf
	 *  family to correctly format this message.  We cache this stuff to avoid
	 *  re-parsing the format string each time a message of this type is sent.
	 */
	class FormatData
	{
	public:
		FormatData( char type, uint8 cflags, uint8 vflags ) :
			type_( type ), cflags_( cflags ), vflags_( vflags ) {}

		char type_;
		unsigned cflags_:4;
		unsigned vflags_:4;
	};

	void processToken( char type, int cflags );

	/// The format string associated with this object
	std::string fmt_;

	/// Cached printf data structures, allowing us to avoid re-parsing the
	/// format string each time we call parseArgs()
	std::vector< FormatData > fmtData_;

	/// The number of calls since the last time clearRecentCalls() was called.
	unsigned numRecentCalls_;

	/// Whether or not this format string should be suppressed if it is
	/// spamming.
	bool isSuppressible_;
};

#endif
