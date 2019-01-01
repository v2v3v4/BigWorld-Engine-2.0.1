/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SINGLE_PROPERTY_EDITOR_HPP
#define SINGLE_PROPERTY_EDITOR_HPP


#include "world/item_info_db.hpp"


/**
 *	This class is used to change a property in a chunk item.
 */
class SinglePropertyEditor : public GeneralEditor
{
public:

	SinglePropertyEditor( ChunkItemPtr pItem,
				const ItemInfoDB::Type & type, const std::string & newVal );

	virtual ~SinglePropertyEditor();

	virtual void addProperty( GeneralProperty * pProp );

private:
	ChunkItemPtr pItem_;
	ItemInfoDB::Type type_;
	std::string newVal_;
	bool editHidden_;
	bool editFrozen_;
	GeneralProperty * pProperty_;
};


#endif // SINGLE_PROPERTY_EDITOR_HPP
