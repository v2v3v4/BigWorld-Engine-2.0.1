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
#include "column_menu_item.hpp"



/**
 *	Constructor.
 */
ColumnMenuItem::ColumnMenuItem( const std::string & item,
		const std::string & prefix, const std::string & owner, int colIdx ) :
	item_( item ),
	prefix_( prefix ),
	owner_( owner ),
	colIdx_( colIdx )
{
	BW_GUARD;
}


/**
 *	This method compares two ColumnMenuItems for sorting.
 */
bool ColumnMenuItem::operator<( const ColumnMenuItem & other ) const
{
	BW_GUARD;

	if (owner_.empty() && !other.owner_.empty())
	{
		return true;
	}
	else if (!owner_.empty() && other.owner_.empty())
	{
		return false;
	}

	int ownerCmp = owner_.compare( other.owner_ );
	if (ownerCmp < 0)
	{
		return true;
	}
	else if (ownerCmp == 0)
	{
		return item_.compare( other.item_ ) < 0;
	}

	return false;
}
