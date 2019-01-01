/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	StringUtils: String utility methods.
 */

#include "pch.hpp"
#include <windows.h>
#include "Shlwapi.h"
#include <algorithm>
#include <string>
#include <vector>
#include "string_utils.hpp"
#include "cstdmf/debug.hpp"
#include "resmgr/string_provider.hpp"

DECLARE_DEBUG_COMPONENT( 0 );




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------------
bool StringUtils::PathMatchSpecT( const char * pszFileParam, const char * pszSpec )
{
	BW_GUARD;

	return PathMatchSpecA( pszFileParam, pszSpec ) == TRUE;
}

//------------------------------------------------------------------------------
bool StringUtils::PathMatchSpecT( const wchar_t * pszFileParam, const wchar_t * pszSpec )
{
	BW_GUARD;

	return PathMatchSpecW( pszFileParam, pszSpec ) == TRUE;
}


//------------------------------------------------------------------------------
void StringUtils::incrementT( std::string & str, StringUtils::IncrementStyle incrementStyle )
{
	BW_GUARD;

	//
	// For IS_EXPLORER the incrementation goes like:
	//
	//      original string
	//      Copy of original string
	//      Copy (2) of original string
	//      Copy (3) of original string
	//      ...
	//
	if (incrementStyle == IS_EXPLORER)
	{
		// Handle the degenerate case:
		if (str.empty())
			return;

		// See if the string starts with "Copy of ".  If it does then the result
		// will be "Copy (2) of" remainder.
		std::string prefix = LocaliseUTF8(L"COMMON/STRING_UTILS/COPY_OF" );
		if (str.substr(0, prefix.size()) == prefix)
		{
			std::string remainder = str.substr(prefix.size(), std::string::npos);
			str = LocaliseUTF8(L"COMMON/STRING_UTILS/COPY_OF_N", 2, remainder );
			return;
		}
		// See if the string starts with "Copy (n) of " where n is a number.  If it
		// does then the result will be "Copy (n + 1) of " remainder.
		prefix = LocaliseUTF8(L"COMMON/STRING_UTILS/COPY");
		if (str.substr(0, prefix.size()) == prefix)
		{
			size_t       pos = 6;
			unsigned int num = 0;
			while (pos < str.length() && ::isdigit(str[pos]))
			{
				num *= 10;
				num += str[pos] - '0';
				++pos;
			}
			++num;
			if (pos < str.length())
			{
				std::string suffix = LocaliseUTF8(L"COMMON/STRING_UTILS/OF");
				if (str.substr(pos, suffix.size()) == suffix)
				{
					std::string remainder = str.substr(pos + suffix.size(), std::string::npos);
					str = LocaliseUTF8(L"COMMON/STRING_UTILS/COPY_OF_N", num, remainder );
					return;
				}
			}
		}

		// The result must be "Copy of " str.
		str = LocaliseUTF8(L"COMMON/STRING_UTILS/COPY_OF") + ' ' + str;
		return;
	}
	//
	// For IS_END the incrementation goes like:
	//
	//      original string
	//      original string 2
	//      original string 3
	//      original string 4
	//      ...
	//
	// or, if the orignal string is "original string(0)":
	//
	//      original string(0)
	//      original string(1)
	//      original string(2)
	//      ...
	//
	else if (incrementStyle == IS_END)
	{
		if (str.empty())
			return;

		char lastChar    = str[str.length() - 1];
		bool hasLastChar = ::isdigit(lastChar) == 0;

		// Find the position of the ending number and where it begins:
		int pos = (int)str.length() - 1;
		if (hasLastChar)
			--pos;
		unsigned int count = 0;
		unsigned int power = 1;
		bool hasDigits = false;
		for (;pos >= 0 && ::isdigit(str[pos]) != 0; --pos)
		{
			count += power*(str[pos] - '0'); 
			power *= 10;
			hasDigits = true;
		}

		// Case where there was no number:
		if (!hasDigits)
		{
			count = 1;
			++pos;
			hasLastChar = false;
		}

		// Increment the count:
		++count;    

		// Construct the result:
		std::stringstream stream;
		std::string prefix = str.substr(0, pos + 1);
		stream << prefix.c_str();
		if (!hasDigits)
			stream << ' ';
		stream << count;
		if (hasLastChar)
			stream << lastChar;
		str = stream.str();
		return;
	}
	else
	{
		return;
	}
}

//------------------------------------------------------------------------------
void StringUtils::incrementT( std::wstring & str, StringUtils::IncrementStyle incrementStyle )
{
	BW_GUARD;

	//
	// For IS_EXPLORER the incrementation goes like:
	//
	//      original string
	//      Copy of original string
	//      Copy (2) of original string
	//      Copy (3) of original string
	//      ...
	//
	if (incrementStyle == IS_EXPLORER)
	{
		// Handle the degenerate case:
		if (str.empty())
			return;

		// See if the string starts with "Copy of ".  If it does then the result
		// will be "Copy (2) of" remainder.
		std::wstring prefix = Localise(L"COMMON/STRING_UTILS/COPY_OF" );
		if (str.substr(0, prefix.size()) == prefix)
		{
			std::wstring remainder = str.substr(prefix.size(), std::wstring::npos);
			str = Localise(L"COMMON/STRING_UTILS/COPY_OF_N", 2, remainder );
			return;
		}
		// See if the string starts with "Copy (n) of " where n is a number.  If it
		// does then the result will be "Copy (n + 1) of " remainder.
		prefix = Localise(L"COMMON/STRING_UTILS/COPY");
		if (str.substr(0, prefix.size()) == prefix)
		{
			size_t       pos = 6;
			unsigned int num = 0;
			while (pos < str.length() && ::isdigit(str[pos]))
			{
				num *= 10;
				num += str[pos] - '0';
				++pos;
			}
			++num;
			if (pos < str.length())
			{
				std::wstring suffix = Localise(L"COMMON/STRING_UTILS/OF");
				if (str.substr(pos, suffix.size()) == suffix)
				{
					std::wstring remainder = str.substr(pos + suffix.size(), std::wstring::npos);
					str = Localise(L"COMMON/STRING_UTILS/COPY_OF_N", num, remainder );
					return;
				}
			}
		}

		// The result must be "Copy of " str.
		str = Localise(L"COMMON/STRING_UTILS/COPY_OF") + L' ' + str;
		return;
	}
	//
	// For IS_END the incrementation goes like:
	//
	//      original string
	//      original string 2
	//      original string 3
	//      original string 4
	//      ...
	//
	// or, if the orignal string is "original string(0)":
	//
	//      original string(0)
	//      original string(1)
	//      original string(2)
	//      ...
	//
	else if (incrementStyle == IS_END)
	{
		if (str.empty())
			return;

		wchar_t lastChar    = str[str.length() - 1];
		bool hasLastChar = ::isdigit(lastChar) == 0;

		// Find the position of the ending number and where it begins:
		int pos = (int)str.length() - 1;
		if (hasLastChar)
			--pos;
		unsigned int count = 0;
		unsigned int power = 1;
		bool hasDigits = false;
		for (;pos >= 0 && ::isdigit(str[pos]) != 0; --pos)
		{
			count += power*(str[pos] - '0'); 
			power *= 10;
			hasDigits = true;
		}

		// Case where there was no number:
		if (!hasDigits)
		{
			count = 1;
			++pos;
			hasLastChar = false;
		}

		// Increment the count:
		++count;    

		// Construct the result:
		std::wstringstream stream;
		std::wstring prefix = str.substr(0, pos + 1);
		stream << prefix.c_str();
		if (!hasDigits)
			stream << ' ';
		stream << count;
		if (hasLastChar)
			stream << lastChar;
		str = stream.str();
		return;
	}
	else
	{
		return;
	}
}

//------------------------------------------------------------------------------
bool StringUtils::makeValidFilenameT( std::string & str, char replaceChar, bool allowSpaces )
{
	BW_GUARD;

	static char const *NOT_ALLOWED         = "/<>?\\|*:";
	static char const *NOT_ALLOWED_NOSPACE = "/<>?\\|*: ";

	bool changed = false; // Were any changes made?

	// Remove leading whitespace:
	while (!str.empty() && ::isspace(str[0]))
	{
		changed = true;
		str.erase(str.begin() + 0);
	}

	// Remove trailing whitespace:
	while (!str.empty() && ::isspace(str[str.length() - 1]))
	{
		changed = true;
		str.erase(str.begin() + str.length() - 1);
	}

	// Handle the degenerate case:
	if (str.empty())
	{
		str = replaceChar;
		return false;
	}
	else
	{
		// Look for and replace characters that are not allowed:        
		size_t pos = std::string::npos;
		do
		{
			if (allowSpaces)
				pos = str.find_first_of(NOT_ALLOWED);
			else
				pos = str.find_first_of(NOT_ALLOWED_NOSPACE);
			if (pos != std::string::npos)
			{
				str[pos] = replaceChar;
				changed = true;
			}
		}
		while (pos != std::string::npos);
		return !changed;
	}
}


//------------------------------------------------------------------------------
bool StringUtils::makeValidFilenameT( std::wstring & str, wchar_t replaceChar, bool allowSpaces )
{
	BW_GUARD;

	static wchar_t const *NOT_ALLOWED         = L"/<>?\\|*:";
	static wchar_t const *NOT_ALLOWED_NOSPACE = L"/<>?\\|*: ";

	bool changed = false; // Were any changes made?

	// Remove leading whitespace:
	while (!str.empty() && ::isspace(str[0]))
	{
		changed = true;
		str.erase(str.begin() + 0);
	}

	// Remove trailing whitespace:
	while (!str.empty() && ::isspace(str[str.length() - 1]))
	{
		changed = true;
		str.erase(str.begin() + str.length() - 1);
	}

	// Handle the degenerate case:
	if (str.empty())
	{
		str = replaceChar;
		return false;
	}
	else
	{
		// Look for and replace characters that are not allowed:        
		size_t pos = std::wstring::npos;
		do
		{
			if (allowSpaces)
				pos = str.find_first_of(NOT_ALLOWED);
			else
				pos = str.find_first_of(NOT_ALLOWED_NOSPACE);
			if (pos != std::wstring::npos)
			{
				str[pos] = replaceChar;
				changed = true;
			}
		}
		while (pos != std::wstring::npos);
		return !changed;
	}
}