/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "helper_types.hpp"

#include "wrapper.hpp"

/**
 *	Constructor.
 */
MySqlEscapedString::MySqlEscapedString( MySql & connection,
		const std::string& string ) :
	escapedString_( new char[ string.length() * 2 + 1 ] )
{
	mysql_real_escape_string( connection.get(), escapedString_,
			string.data(), string.length() );
}


/**
 *	Destructor.
 */
MySqlEscapedString::~MySqlEscapedString()
{
	delete [] escapedString_;
}

// helper_types.cpp
