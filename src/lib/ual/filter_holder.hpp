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
 *	FilterHolder: a class that manages a series of filters and searchtext
 */


#ifndef FILTER_HOLDER_HPP
#define FILTER_HOLDER_HPP

#include <string>
#include <map>
#include <vector>
#include "filter_spec.hpp"


/**
 *	This class keeps track of search text and multiple filters used for
 *	narrowing a list of strings.
 */
class FilterHolder
{
public:
	FilterHolder();

	bool hasActiveFilters();
	bool isFiltering();
	void addFilter( FilterSpecPtr filter );
	FilterSpecPtr getFilter( int index );

	void setSearchText( const std::wstring& searchText );
	void enableSearchText( bool enable );

	bool filter( const std::wstring& shortText, const std::wstring& text );
	void enableAll( bool enable );
	void enable( const std::wstring& name, bool enable );
private:
	std::wstring searchText_;
	bool searchTextEnabled_;
	std::vector<FilterSpecPtr> filters_;
	typedef std::vector<FilterSpecPtr>::iterator FilterSpecItr;
};


#endif // FILTER_HOLDER_HPP
