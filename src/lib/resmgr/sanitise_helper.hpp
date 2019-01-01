/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _SANITISE_HELPER_HPP_
#define _SANITISE_HELPER_HPP_

#include <string>

class SanitiseHelper
{
public:
	// Sanitising token
	static const std::string SANITISING_TOKEN;	// = ".."
	static const std::string SPACE_TOKEN;	// = " "

	static std::string substringReplace
		(
		 	const std::string& val,
		 	const std::string& oldSubstr = SPACE_TOKEN,
			const std::string& newSubstr = SANITISING_TOKEN
		);
};

#endif
