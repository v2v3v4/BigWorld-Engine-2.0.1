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
 *	FILTER_SPEC: filters text according to its include/exclude rules
 */


#include <algorithm>
#include <vector>
#include <string>
#include "cstdmf/guard.hpp"
#include "filter_spec.hpp"
#include "common/string_utils.hpp"
#include "cstdmf/string_utils.hpp"


/**
 *	Constructor.  Parses the include and exclude strings into vectors.
 */
FilterSpec::FilterSpec( const std::wstring& name, bool active,
					   const std::wstring& include, const std::wstring& exclude,
					   const std::wstring& group ) :
	name_( name ),
	active_( active ),
	enabled_( true ),
	group_( group )
{
	BW_GUARD;

	std::wstring includeL = include;
	std::replace( includeL.begin(), includeL.end(), L'/', L'\\' );
	bw_tokenise( includeL, L",;", includes_ );

	std::wstring excludeL = exclude;
	std::replace( excludeL.begin(), excludeL.end(), L'/', L'\\' );
	bw_tokenise( excludeL, L",;", excludes_ );
}


/**
 *	Destructor
 */
FilterSpec::~FilterSpec()
{
	BW_GUARD;
}


/**
 *	This method tests the input string agains the filter, and returns true if
 *	there's a match.
 *
 *	@param str	String to match.
 *	@return		True if the string matches the filter, false otherwise.
 */
bool FilterSpec::filter( const std::wstring& str )
{
	BW_GUARD;

	if ( !active_ || !enabled_ )
		return true;

	// pass filter test if it's in the includes and not in the excludes.
	return StringUtils::matchSpecT( str, includes_ ) &&
		( excludes_.empty() || !StringUtils::matchSpecT( str, excludes_ ) );
}


/**
 *	This method enables or disables this filter.
 *
 *	@param enable	True to enable this filter, false to disable it.
 */
void FilterSpec::enable( bool enable )
{
	BW_GUARD;

	enabled_ = enable;
}
