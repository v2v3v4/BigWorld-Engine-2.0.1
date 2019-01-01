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
#include "sanitise_helper.hpp"


/**
 *	The default tokens used to sanitise data sections.
 */
const std::string SanitiseHelper::SANITISING_TOKEN = "..";
const std::string SanitiseHelper::SPACE_TOKEN = " ";


/**
 *	Static helper method for replacing tokens in a string.
 *
 * @param val		The string being processed
 * @param oldSubstr	The substring being searched for
 * @param newSubstr	The replacing substring
 * @return			The string after processing
 */
/*static*/ std::string SanitiseHelper::substringReplace
	(
		const std::string& val,
		const std::string& oldSubstr /* = SPACE_TOKEN */,
		const std::string& newSubstr /* = SANITISING_TOKEN */
	)
{
	// Check if we can early out
	size_t valSize = val.size();
	size_t oldSubstrSize = oldSubstr.size();
	if ( valSize < oldSubstrSize )
		return val;

	std::string result = val;

	for
		(
			std::string::size_type pos = result.find(oldSubstr);
			pos != std::string::npos;
			pos = result.find(oldSubstr)
		)
	{
		result.replace(pos, oldSubstrSize, newSubstr);
	}

	return result;
}
