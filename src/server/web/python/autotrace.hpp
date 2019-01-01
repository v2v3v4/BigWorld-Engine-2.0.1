/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __AUTOTRACE_HPP__
#define __AUTOTRACE_HPP__

#include "cstdmf/debug.hpp"

#include <string>

/**
 * Class for debugging. Intended for automatic instantiation, and prints
 * trace debug messages in its constructor and destructor.
 */
class AutoTrace
{
public:
	AutoTrace( const std::string& name ):
		name_(name)
	{
		TRACE_MSG( "%s start\n", name_.c_str() );
	}

	~AutoTrace()
	{
		TRACE_MSG( "%s end\n", name_.c_str() );
	}

private:
	std::string name_;
};

#endif // __AUTO_TRACE_HPP__
