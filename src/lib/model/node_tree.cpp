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

#include "node_tree.hpp"


#include "moo/node.hpp"


DECLARE_DEBUG_COMPONENT2( "Model", 0 )


/**
 * Constructs a NodeTreeIterator that will traverse the given node tree.
 *
 * @param cur the current element in the node tree.
 * @param end   on past the last element in the node tree.
 * @param root  the world transformation matrix of the root node.
 */
NodeTreeIterator::NodeTreeIterator(
		const NodeTreeData * cur,
		const NodeTreeData * end,
		const Matrix * root ) :
	curNodeData_( cur ),
	endNodeData_( end ),
	curNodeItem_(),
	sat_( 0 )
{
	BW_GUARD;
	curNodeItem_.pData = curNodeData_;
	curNodeItem_.pParentTransform = root;

	stack_[sat_].trans = root;
	stack_[sat_].pop = end - cur;
	//while (stack_[sat_].pop-- <= 0) --sat_;
}

/**
 * Returns current item.
 */
const NodeTreeDataItem & NodeTreeIterator::operator * () const
{
	BW_GUARD;
	return curNodeItem_;
}

/**
 * Returns current item.
 */
const NodeTreeDataItem * NodeTreeIterator::operator -> () const
{
	BW_GUARD;
	return &curNodeItem_;
}

/**
 * Advance to next node in tree. Call eot to check if
 * traversal has ended before accessing any node data.
 */
void NodeTreeIterator::operator ++ ( int )
{
	BW_GUARD;
	const NodeTreeData & ntd = *(curNodeItem_.pData);
	if (ntd.nChildren > 0)
	{
		++sat_;
		if ( sat_ >= NODE_STACK_SIZE )
		{
			--sat_;
			ERROR_MSG( "Node Hierarchy too deep, unable to complete traversal\n" );
			/* Set this equal to the end to prevent infinite loops 
			*/
			curNodeData_ = endNodeData_;
			curNodeItem_.pData = curNodeData_;
			return;
		}
		stack_[sat_].trans = &ntd.pNode->worldTransform();
		stack_[sat_].pop = ntd.nChildren;
	}

	++curNodeData_;
	curNodeItem_.pData = curNodeData_;

	while (stack_[sat_].pop-- <= 0) --sat_;
	curNodeItem_.pParentTransform = stack_[sat_].trans;
}

/**
 * operator equals to
 */
bool NodeTreeIterator::operator == ( const NodeTreeIterator & other ) const
{
	BW_GUARD;
	return other.curNodeData_ == this->curNodeData_;
}

/**
 * operator not equals to
 */
bool NodeTreeIterator::operator != ( const NodeTreeIterator & other ) const
{
	BW_GUARD;
	return !(other == *this);
}




// node_tree.cpp
