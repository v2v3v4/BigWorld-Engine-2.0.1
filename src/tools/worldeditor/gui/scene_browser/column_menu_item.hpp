/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef COLUMN_MENU_ITEM_HPP
#define COLUMN_MENU_ITEM_HPP


/**
 *	This helper class is used for the columns popup menu, to allow sorting
 *	and grouping of column types by the asset type that created it.
 */
class ColumnMenuItem
{
public:
	ColumnMenuItem( const std::string & item, const std::string & prefix,
					const std::string & owner, int colIdx );
	
	//	This method returns the asset type that created this type.
	const std::string & owner() const { return owner_; }

	//	This method returns the type of this column.
	std::string menuItem() const { return prefix_ + item_; }

	//	This method returns the index of this column.
	int colIdx() const { return colIdx_; }

	bool operator<( const ColumnMenuItem & other ) const;

private:
	std::string item_;
	std::string prefix_;
	std::string owner_;
	int colIdx_;
};


#endif // COLUMN_MENU_ITEM_HPP
