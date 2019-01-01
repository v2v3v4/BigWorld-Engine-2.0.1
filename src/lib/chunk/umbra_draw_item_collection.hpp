/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef UMBRA_DRAW_ITEM_COLLECTION_HPP
#define UMBRA_DRAW_ITEM_COLLECTION_HPP

#include "umbra_config.hpp"

#if UMBRA_ENABLE

#include "umbra_draw_item.hpp"

/**
 *	This class implements the UmbraDrawItem interface for a collection of draw items.
 *	It's intended use is to combine multiple draw items for one umbra object to reduce
 *	the number of umbra objects created.
 */
class UmbraDrawItemCollection : public UmbraDrawItem
{
public:
	UmbraDrawItemCollection();
	~UmbraDrawItemCollection();
	virtual Chunk* draw(Chunk* pChunkContext);
	virtual Chunk* drawDepth(Chunk* pChunkContext);

	void createObject( Umbra::Cell* pCell );
	void addItem( UmbraDrawItem* pItem );

private:
	typedef std::vector<UmbraDrawItem*> ItemVector;
	UmbraModelProxyPtr		pModel_;
	ItemVector				items_;
};

#endif // UMBRA_ENABLE

#endif // UMBRA_DRAW_ITEM_COLLECTION_HPP
