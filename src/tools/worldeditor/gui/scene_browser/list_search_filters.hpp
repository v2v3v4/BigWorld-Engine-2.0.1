/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LIST_SEARCH_FILTERS_HPP
#define LIST_SEARCH_FILTERS_HPP


#include "world/item_info_db.hpp"
#include "list_column.hpp"


/**
 *	This class keeps track of the current filtering and handles the filters
 *	UI and user input.
 */
class ListSearchFilters
{
public:
	typedef std::set< ItemInfoDB::Type > Types;

	ListSearchFilters( const ListColumns & columns );

	void doModal( const CWnd & pParent, const CPoint & pt );
	
	const Types & allowedTypes() const { return allowedTypes_; }

	void updateAllowedTypes();

	std::wstring filterDesc() const;

private:
	const ListColumns & columns_;
	int currentFilter_;
	Types allowedTypes_;

	std::wstring checkedPrefix( int filter ) const;

	std::wstring descFromId( int filter ) const;
};


#endif // LIST_SEARCH_FILTERS_HPP
