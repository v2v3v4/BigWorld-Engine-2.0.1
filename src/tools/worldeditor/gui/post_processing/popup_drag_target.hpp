/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef POPUP_DRAG_TARGET_HPP
#define POPUP_DRAG_TARGET_HPP


/**
 *	This class draws a translucent popup menu that is suitable for use when
 *	performing a drag & drop operation.
 */
class PopupDragTarget : public CWnd
{
public:
	typedef std::vector< std::string > ItemList;

	PopupDragTarget();
	virtual ~PopupDragTarget();

	CSize calcSize( const ItemList & itemList ) const;

	bool open( const CPoint & pt, const ItemList & itemList, bool arrowUp );

	std::string update( int alpha );

	void close();

private:
	CFont font_;
	ItemList items_;
	bool arrowUp_;
};


#endif // POPUP_DRAG_TARGET_HPP
