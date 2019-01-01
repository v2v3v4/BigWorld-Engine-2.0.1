/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef XML_SPECIAL_CHARS_HPP
#define XML_SPECIAL_CHARS_HPP

#include <string>

/**
 *  This class is used to convert between special XML characters and plain
 *  text. An example of this is converting the "less-than" character between
 *  the plain text character "<" and the XML escape sequence "&lt;".
 */
class XmlSpecialChars
{
public:
	// Converts ampersand-based sequences into the actual characters.
	static void reduce( char* buf );

	// Converts the invalid characters to ampersand-based sequences.
	// buf must have enough room for the expanded characters to fit.
	static std::string expand( const char* buf );
};

#endif // XML_SPECIAL_CHARS_HPP
