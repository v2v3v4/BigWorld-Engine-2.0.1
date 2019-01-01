/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include "stdmf.hpp"

#include <string>
#include <vector>

/**
 * String utility functions
 */
std::string bw_format( const char * format, ... );

#ifdef _WIN32
/**
 * Converts from ACP (active code page) to UTF8, which is good for converting
 *  results from ANSI versions of win32 API to ResMgr stuff.
 */
bool bw_acptoutf8( const char * s, std::string& output );
inline bool bw_acptoutf8( const std::string & s, std::string& output )
{
	return bw_acptoutf8( s.c_str(), output );
}
inline std::string bw_acptoutf8( const std::string & s )
{
	std::string ret;
	bw_acptoutf8( s, ret );
	return ret;
}


/**
* Converts from UTF8 to ACP (active code page), which is good for converting
*  results from ResMgr to ANSI versions of win32 API.
*/
bool bw_utf8toacp( const char * s, std::string& output );
inline bool bw_utf8toacp( const std::string & s, std::string& output )
{
	return bw_utf8toacp( s.c_str(), output );
}
inline std::string bw_utf8toacp( const std::string & s )
{
	std::string ret;
	bw_utf8toacp( s, ret );
	return ret;
}


/**
 *
 *	Converts the given narrow string to the wide representation, using the 
 *  active code page on this system. Returns true if it succeeded, otherwise
 *	false if there was a decoding error.
 *
 */
bool bw_acptow( const char * s, std::wstring& output );
inline bool bw_acptow( const std::string & s, std::wstring& output )
{
	return bw_acptow( s.c_str(), output );
}
inline std::wstring bw_acptow( const std::string & s )
{
	std::wstring ret;
	bw_acptow( s, ret );
	return ret;
}


/**
 *
 *	Converts the given wide string to the narrow representation, using the 
 *  active code page on this system. Returns true if it succeeded, otherwise
 *	false if there was a decoding error.
 *
 */
bool bw_wtoacp( const wchar_t * s, std::string& output );
inline bool bw_wtoacp( const std::wstring & s, std::string& output )
{
	return bw_wtoacp( s.c_str(), output );
}
inline std::string bw_wtoacp( const std::wstring & s )
{
	std::string ret;
	bw_wtoacp( s, ret );
	return ret;
}


#endif

/**
 *
 *	Converts the given utf-8 string to the wide representation. Returns true 
 *  if it succeeded, otherwise false if there was a decoding error.
 *
 */
bool bw_utf8tow( const char * s, std::wstring& output );
inline bool bw_utf8tow( const std::string & s, std::wstring& output )
{
	return bw_utf8tow( s.c_str(), output );
}
inline bool bw_utf8towSW( const std::string & s, std::wstring& output )
{
	return bw_utf8tow( s.c_str(), output );
}
inline std::wstring bw_utf8tow( const std::string & s )
{
	std::wstring ret;
	bw_utf8tow( s, ret );
	return ret;
}


/**
 *
 *	Converts the given wide string to the utf-8 representation. Returns true 
 *  if it succeeded, otherwise false if there was a decoding error.
 *
 */
bool bw_wtoutf8( const wchar_t * s, std::string& output );
inline bool bw_wtoutf8( const std::wstring & s, std::string& output )
{
	return bw_wtoutf8( s.c_str(), output );
}
inline bool bw_wtoutf8WS( const std::wstring & s, std::string& output )
{
	return bw_wtoutf8( s.c_str(), output );
}
inline std::string bw_wtoutf8( const std::wstring & s )
{
	std::string ret;
	bw_wtoutf8( s, ret );
	return ret;
}


/**
 * This function will convert from UTF-8 to wide char in-place, it won't 
 * allocate any new buffers. At most, it will write dSize-1 chars plus a
 * trailing \0. If total conversion is not possible, it will return false
 * 
 */
bool bw_utf8tow( const char * src, size_t srcSize, wchar_t * dst, size_t dSize );
inline bool bw_utf8tow( const std::string & src, wchar_t * dst, size_t dSize )
{
	return bw_utf8tow( src.c_str(), src.length()+1, dst, dSize );
}

/**
 * This function will convert from wide char to UTF-8 in-place, it won't 
 * allocate any new buffers. At most, it will write dSize-1 chars plus a
 * trailing \0. If total conversion is not possible, it will return false
 * 
 */
bool bw_wtoutf8( const wchar_t * src, size_t srcSize, char * dst, size_t dstSize );
inline bool bw_wtoutf8( const std::wstring & src, char * dst, size_t dstSize )
{
	return bw_wtoutf8( src.c_str(), src.length()+1, dst, dstSize );
}


/**
 * This function converts a null-terminated ansi c-string into a wide char 
 *  string. It does this just by promoting char to wchar_t. Always succeeds.
 */
void bw_ansitow( const char * src, std::wstring & dst );
inline void bw_ansitow( const std::string & src, std::wstring & dst )
{
	return bw_ansitow( src.c_str(), dst );
}

/**
 * These functions do naive comparison between narrow & wide strings. They 
 * assume that the wide string will only really contain ASCII characters,
 * but if it doesn't, the comparison will still work.
 */
int bw_MW_strcmp ( const wchar_t * lhs, const char * rhs );
int bw_MW_strcmp ( const char * lhs, const wchar_t * rhs );
int bw_MW_stricmp ( const wchar_t * lhs, const char * rhs );
int bw_MW_stricmp ( const char * lhs, const wchar_t * rhs );

/**
 * This function takes a string source (or wstring or whatever), and a string of
 *  delimiters. It will tokenise the string according to the list of delimiters
 *  and put each found token in the vector. It will tokenise at most a_MaxTokens
 *  tokens, unless it is 0, then it will consume all the input string.
 * Note that this function does NOT clear the container, making it possible to
 *	append to a container that already contains elements.
 */
template< typename T1, typename T2, typename T3 >
void bw_tokenise( const T1 & a_Str,
				  const T2 & a_Delimiters, 
				  T3 & o_Tokens,
				  size_t a_MaxTokens = 0 )
{
	typename T1::size_type lastPos = a_Str.find_first_not_of( a_Delimiters, 0 );
	typename T1::size_type pos = a_Str.find_first_of( a_Delimiters, lastPos );
	while ( T1::npos != pos || T1::npos != lastPos )
	{
		if ( a_MaxTokens > 0 && o_Tokens.size() == ( a_MaxTokens - 1 ) )
		{
			o_Tokens.push_back( a_Str.substr( lastPos ) );
			return;
		}
		typename T1::size_type sublen = ( pos == T1::npos ? a_Str.length() : pos ) - lastPos;
		o_Tokens.push_back( a_Str.substr( lastPos, sublen ) );
		lastPos = a_Str.find_first_not_of( a_Delimiters, pos );
		pos = a_Str.find_first_of( a_Delimiters, lastPos );
	}
}


/**
 * This function joins together a container of strings using the specified
 * separator.
 */
template< typename T1, typename T2, typename T3 >
void bw_stringify( const T1 & a_Tokens, 
				   const T2 & a_Sep,
				   T3 & o_Str )
{
	for ( typename T1::const_iterator it = a_Tokens.begin(); it != a_Tokens.end(); ++it )
	{
		if ( o_Str.length() > 0 )
			o_Str += a_Sep;
		o_Str += (*it);
	}
}


template< typename T1, typename T2, class F >
void bw_containerConversion( const T1 & src, T2 & dst, F functor )
{
	for ( typename T1::const_iterator it = src.begin() ; it != src.end() ; ++it )
	{
		const typename T1::value_type & srcItem = *it;
		typename T2::value_type dstItem;
		functor( srcItem, dstItem );
		dst.push_back( dstItem );
	}
}


/**
 * This generates names in the form root 0, root 1...
 */

class IncrementalNameGenerator
{
public:
	IncrementalNameGenerator( const std::string& rootName );
	~IncrementalNameGenerator() {};

	std::string nextName();

private:
	std::string rootName_;
	int			nameCount_;
};

inline IncrementalNameGenerator::IncrementalNameGenerator( const std::string& rootName )
:	rootName_( rootName ),
	nameCount_( 0 )
{
}

inline std::string IncrementalNameGenerator::nextName()
{
	char name[80];
	bw_snprintf( name, sizeof(name), "%s %d", rootName_.c_str(), nameCount_++ );
	return std::string( name );
}


#endif // STRING_UTILS_HPP
