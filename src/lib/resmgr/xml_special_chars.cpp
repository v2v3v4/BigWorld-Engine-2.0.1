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
#include "cstdmf/debug.hpp"
#include "xml_special_chars.hpp"

#include <string>

#include <string.h>

DECLARE_DEBUG_COMPONENT2( "ResMgr", 0 )


void XmlSpecialChars::reduce( char* buf )
{
	int len = strlen( buf );
	// loop and find any ampersand
	for( int i = 0; i < len; ++i )
	{
		if ( buf[i] == '&' )
		{
			// found a escape sequence, so handle it
			// first, get the escape sequence
			char special[32];
			memset( special, 0, 32 );
			int specialLen = 1;
			for( int j = 0, ii = i + 1; ii < len && j < 31; ++j, ++ii )
			{
				specialLen++;
				if ( buf[ii] == ';' )
					break;
				special[j] = buf[ii];
			}
			if ( specialLen <= 2 )
			{
				WARNING_MSG( "XML escape sequence too short in \'%s\'.\n", buf );
				continue;
			}
			if ( specialLen >= 31 )
			{
				WARNING_MSG( "XML escape sequence too long (no trailing semicolon found) in \'%s\'\n", buf );
				continue;
			}

			// now, find the character that will replace the escape sequence
			char replace = ' ';
			if ( strcmp( special, "amp" ) == 0 )
				replace = '&';
			else if ( strcmp( special, "lt" ) == 0 )
				replace = '<';
			else if ( strcmp( special, "gt" ) == 0 )
				replace = '>';
			else if ( strcmp( special, "quot" ) == 0 )
				replace = '\"';
			else if ( strcmp( special, "apos" ) == 0 )
				replace = '\'';
			else if ( special[0] == '#' )
			{
				// Right now it only supports character codes from 0 to 255.
				int val = 0;
				if ( special[1] == 'x' )
				{
					char hexbuf[40];
					bw_snprintf( hexbuf, sizeof(hexbuf), "0x%s", special + 2 );
					sscanf( hexbuf, "%x", &val );
				}
				else
					val = atoi( special + 1 );
				if ( val < 1 || val > 255 )
					WARNING_MSG( "Invalid char code \'&%s;\' in \'%s\'. Only char codes between 1 and 255 are currently supported in \'&#000;\' and \'&#x00;\' style escape sequences.\n", special, buf );
				replace = (char)val;
			}
			else
			{
				WARNING_MSG( "Invalid XML escape sequence \'&%s;\' in \'%s\'.\n", special, buf );
				continue;
			}
			// replace the ampersand
			buf[i] = replace;
			// and shift the remaining of the string, removing the escape sequence
			specialLen--;
			for( int c = i + 1; c + specialLen < len; ++c )
				buf[c] = buf[c + specialLen];
			len -= specialLen;
		}
	}
	buf[len] = 0; // len can never grow, so this should be safe.
}

std::string XmlSpecialChars::expand( const char* buf )
{
	int len = strlen( buf );
	std::string res;
	for( int i = 0; i < len; ++i )
	{
		std::string replace;
		if ( buf[i] == '&' )
			replace = "&amp;";
		else if ( buf[i] == '<' )
			replace = "&lt;";
		else if ( buf[i] == '>' )
			replace = "&gt;";
		else if ( buf[i] == '\"' )
			replace = "&quot;";
		else if ( buf[i] == '\'' )
			replace = "&apos;";
		else if ( buf[i] < 0 )
		{
			// encode chars outside the standard ascii range
			char str[20];
			bw_snprintf( str, sizeof(str), "&#%d;", (int)((unsigned char)buf[i]) );
			replace = str;
		}

		if ( replace.empty() )
			res += buf[i];
		else
			res += replace;
	}

	return res;
}
