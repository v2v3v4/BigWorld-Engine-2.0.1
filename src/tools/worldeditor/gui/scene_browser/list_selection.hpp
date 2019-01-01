/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LIST_SELECTION_HPP
#define LIST_SELECTION_HPP

#include <hash_set>


/**
 *	This class helps to keeps track of a ItemInfoDB item list selection.
 */
class ListSelection
{
public:
	typedef std::list< ItemInfoDB::ItemPtr > ItemList;

	ListSelection();

	void init( CListCtrl * pList );

	void filter( bool eraseMissingItems = true );

	void restore( const std::vector< ItemInfoDB::ItemPtr > & items );

	void update( const std::vector< ItemInfoDB::ItemPtr > & items );

	int numItems() const { return (int)selection_.size(); }
	int numSelectedTris() const;
	int numSelectedPrimitives() const;

	const ItemList & selection( bool validate );
	void selection( const std::vector< ChunkItemPtr > & selection,
					const std::vector< ItemInfoDB::ItemPtr > & items  );

	void deleteSelectedItems() const;

	bool needsUpdate() const { return needsUpdate_; }
	void needsUpdate( bool val ) { needsUpdate_ = val; }

	bool changed() const { return changed_; }
	void resetChanged() { changed_ = false; }

private:
	typedef stdext::hash_set< ChunkItem * > ChunkItemHash;

	CListCtrl * pList_;

	ItemList selection_;
	ChunkItemHash selectionHash_; // for speed
	bool needsUpdate_;
	bool changed_;
};


#endif // LIST_SELECTION_HPP
