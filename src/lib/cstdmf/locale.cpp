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

#include "locale.hpp"

namespace // anonymous
{

std::locale s_standardC( "C" );

} // end namespace (anonymous)

namespace Locale
{

/**
 *	Return the standard "C" locale.
 */
const std::locale & standardC()
{
	return s_standardC;
}

} // end namespace Locale


// locale.cpp
