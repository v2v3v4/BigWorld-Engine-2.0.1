/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef UNIQUE_ID_HPP
#define UNIQUE_ID_HPP

#include <string>
#include "stdmf.hpp"

/**
 * A Wrapper around GUIDs, used for graph and node names
 *
 * Custom conversion to/from strings is defined, the format is:
 * 81A9D1BF.4B8B622E.6F7081B3.0698330E
 * ie, 4 uints represented in hex separated by periods.
 */
class UniqueID
{
private:
	// NOTE: Structure must be compatible with Microsoft's GUID structure since
	// we cast to GUID in one of functions.
	uint a_;
	uint b_;
	uint c_;
	uint d_;
	
	static UniqueID s_zero_;

public:
	UniqueID();
	UniqueID( const std::string& s );
	UniqueID( uint a, uint b, uint c, uint d ) : 
		a_( a ), b_( b ), c_( c ), d_( d )
	{}
	
	uint getA() const	{ return a_; }
	uint getB() const	{ return b_; }
	uint getC() const	{ return c_; }
	uint getD() const	{ return d_; }

	std::string toString() const {return *this;}
	operator std::string() const;

	bool operator==( const UniqueID& rhs ) const;
	bool operator!=( const UniqueID& rhs ) const;
	bool operator<( const UniqueID& rhs ) const;

	/**
	 * Ask windows to generade a GUID for us
	 */
#ifdef _WIN32
	static UniqueID generate();
#endif

	static const UniqueID& zero() { return s_zero_; }

    static bool isUniqueID( const std::string& s );

private:
    static bool fromString( const std::string& s, uint* data );
};

#endif	//UNIQUE_ID_HPP
