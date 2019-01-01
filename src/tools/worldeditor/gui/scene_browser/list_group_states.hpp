/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LIST_GROUP_STATES_HPP
#define LIST_GROUP_STATES_HPP


#include "world/item_info_db.hpp"


typedef std::pair< std::string, ItemInfoDB::Type > ListGroup;
typedef std::vector< ListGroup > ListGroups;


/**
 *	This helper class deals with list groups and group-related operations.
 */
class ListGroupStates
{
public:
	typedef std::vector< ItemInfoDB::ItemPtr > ItemIndex;

	ListGroupStates();
	~ListGroupStates();

	void groupBy( ListGroup * group );
	const ListGroup * groupBy() const { return pGroupBy_; }
	void groups( ListGroups & retGroups ) const;

	ItemInfoDB::Type groupByType() const
				{ return pGroupBy_ ? pGroupBy_->second : ItemInfoDB::Type(); }

	bool groupExpanded( const std::string & group ) const;

	bool isGroupStart( const ItemIndex & items, int index ) const;

	void expandCollapse( const std::string & group, bool doExpand );

	void expandCollapseAll( const ItemIndex & items, bool doExpand );
	
	void handleGroupClick( const ItemIndex & items,
												const std::string & group );

private:
	typedef std::pair< std::string, bool > GroupStatePair;
	typedef std::map< std::string, bool > GroupStateMap;

	ListGroup * pGroupBy_;
	GroupStateMap groupStates_;
};



#endif // LIST_GROUP_STATES_HPP
