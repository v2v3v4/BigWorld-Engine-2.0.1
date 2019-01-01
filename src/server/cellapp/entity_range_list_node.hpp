/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_RANGE_LIST_NODE_HPP
#define ENTITY_RANGE_LIST_NODE_HPP

#include "range_list_node.hpp"

class Entity;

/**
 *	This class is used as an entity's entry into the range list. The position
 *	of this node is the same as the entity's position. When the entity moves,
 *	this node may also move along the x/z lists.
 */
class EntityRangeListNode: public RangeListNode
{
public:
	EntityRangeListNode( Entity * entity );
	float x() const;
	float z() const;
	std::string debugString() const;
	Entity * getEntity() const;

	void remove();

	static Entity * getEntity( RangeListNode * pNode )
	{
		return static_cast< EntityRangeListNode * >( pNode )->getEntity();
	}

	static const Entity * getEntity( const RangeListNode * pNode )
	{
		return static_cast< const EntityRangeListNode * >( pNode )->getEntity();
	}

protected:
	Entity * pEntity_;
};

#endif // ENTITY_RANGE_LIST_NODE_HPP
