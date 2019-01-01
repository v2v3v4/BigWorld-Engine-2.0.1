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
#include "group_item.hpp"


/**
 *	Constructor
 *
 *	@param group	Group name, matching the type (chunk id, filename, etc).
 *	@param type		Type of the grouping property.
 */
GroupItem::GroupItem(	const std::string & group,
										ItemInfoDB::Type type ) :
	ItemInfoDB::Item( "", NULL, 0, 0, "", "", "",
		ItemInfoDB::ItemProperties() ),
	group_( group ),
	type_( type ),
	numItems_( 0 )
{
	BW_GUARD;
}


/**
 *	This method handles returning group-level property strings depending on the
 *	requested type.
 *
 *	@param type		Type of the desired property.
 *	@return		The property string if it's a group-level property, "" if not.
 */
const std::string & GroupItem::propertyAsString(
									const ItemInfoDB::Type & type ) const
{
	BW_GUARD;

	if (type == type_)
	{
		return group_;
	}
	else if (type == ItemInfoDB::Type::builtinType(
										ItemInfoDB::Type::TYPEID_NUMTRIS ))
	{
		return numTrisStr_;
	}
	else if (type == ItemInfoDB::Type::builtinType(
									ItemInfoDB::Type::TYPEID_NUMPRIMS ))
	{
		return numPrimitivesStr_;
	}
	else if (type == groupNumItemsType())
	{
		return numItemsStr_;
	}
	else if (type == groupNameType())
	{
		return group_;
	}

	static std::string s_emptyStr;
	return s_emptyStr;
}


/**
 *	This method sets the number of items in the group.
 */
void GroupItem::numItems( int numItems )
{
	BW_GUARD;

	numItems_ = numItems;
	numItemsStr_ = bw_format( LocaliseUTF8( L"SCENEBROWSER/NUM_ITEMS" , numItems_ ).c_str() );
	propertyCacheW_.erase( groupNumItemsType() );
}


/**
 *	This method sets the number of triangles in the group.
 */
void GroupItem::numTris( int numTris )
{
	BW_GUARD;

	numTris_ = numTris;
	numTrisStr_ = bw_format( "%d", numTris_ );
	propertyCacheW_.erase( ItemInfoDB::Type::builtinType(
										ItemInfoDB::Type::TYPEID_NUMTRIS ) );
}


/**
 *	This method sets the number of primitives in the group.
 */
void GroupItem::numPrimitives( int numPrims )
{
	BW_GUARD;

	numPrimitives_ = numPrims;
	numPrimitivesStr_ = bw_format( "%d", numPrimitives_ );
	propertyCacheW_.erase( ItemInfoDB::Type::builtinType(
										ItemInfoDB::Type::TYPEID_NUMPRIMS ) );
}


/**
 *	This static member returns the type of the group-only property that stores
 *	the number of items in the group.
 *
 *	@return	Type of the property for the number of items in the group.
 */
/*static*/
ItemInfoDB::Type GroupItem::groupNumItemsType()
{
	BW_GUARD;

	return ItemInfoDB::Type( "builtin_groupNumItems", ValueTypeDesc::STRING );
}



/**
 *	This static member returns the type of the group-only property that stores
 *	the name of the group.
 *
 *	@return	Type of the property for the name of the group.
 */
/*static*/
ItemInfoDB::Type GroupItem::groupNameType()
{
	BW_GUARD;

	return ItemInfoDB::Type( "builtin_groupName", ValueTypeDesc::STRING );
}
