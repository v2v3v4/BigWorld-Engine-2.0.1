/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GROUP_ITEM_HPP
#define GROUP_ITEM_HPP


#include "world/item_info_db.hpp"


/**
 *	This subclass stores info for a group of items, and returns the
 *	appropriate group-level info for the list row it occupies.
 */
class GroupItem : public ItemInfoDB::Item
{
public:
	GroupItem( const std::string & group, ItemInfoDB::Type type );

	const std::string & propertyAsString(
									const ItemInfoDB::Type & type ) const;

	void numItems( int numItems );

	void numTris( int numTris );

	void numPrimitives( int numPrims );

	static ItemInfoDB::Type groupNumItemsType();

	static ItemInfoDB::Type groupNameType();

private:
	std::string group_;
	ItemInfoDB::Type type_;
	int numItems_;
	std::string numItemsStr_;
};


#endif //GROUP_ITEM_HPP