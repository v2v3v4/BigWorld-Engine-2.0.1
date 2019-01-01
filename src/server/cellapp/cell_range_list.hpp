/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CELL_RANGE_LIST_HPP
#define CELL_RANGE_LIST_HPP

#include "range_list_node.hpp"

/**
 *	This class implements a range list.
 */
class RangeList
{
public:
	RangeList();

	void add( RangeListNode * pNode );

	// For debugging
	bool isSorted() const;
	void debugDump();

	const RangeListNode * pFirstNode() const	{ return &first_; }
	const RangeListNode * pLastNode() const		{ return &last_; }

	RangeListNode * pFirstNode() { return &first_; }
	RangeListNode * pLastNode()	{ return &last_; }

private:
	RangeListTerminator first_;
	RangeListTerminator last_;
};


#ifdef CODE_INLINE
#include "cell_range_list.ipp"
#endif


#endif // CELL_RANGE_LIST_HPP
