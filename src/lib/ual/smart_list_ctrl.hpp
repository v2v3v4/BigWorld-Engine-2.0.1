/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef SMART_LIST_CTRL_HPP
#define SMART_LIST_CTRL_HPP

#include "atlimage.h"

#include "cstdmf/smartpointer.hpp"
#include "asset_info.hpp"
#include "xml_item_list.hpp"
#include "filter_holder.hpp"
#include "thumbnail_manager.hpp"


// Forward declarations
namespace controls
{
	class DragImage;
}


/**
 *	This abstract class serves as an interface other classes can inherit in
 *	order to receive notifications from the SmartListCtrl.
 */
class SmartListCtrlEventHandler
{
public:
	virtual void listLoadingUpdate() = 0;
	virtual void listLoadingFinished() = 0;
	virtual void listItemSelect() = 0;
	virtual void listItemDelete() = 0;
	virtual void listDoubleClick( int index ) = 0;
	virtual void listStartDrag( int index ) = 0;
	virtual void listItemRightClick( int index ) = 0;
	virtual void listItemToolTip( int index, std::wstring& info ) = 0;
};


/**
 *	This abstract class is the base class for all list providers, which are
 *	classes capable of providing list items to the SmartListCtrl.
 */
class ListProvider : public ReferenceCount
{
public:
	ListProvider() : filterHolder_( 0 ) {};
	virtual ~ListProvider() {};

	virtual void setFilterHolder( FilterHolder* filterHolder ) { filterHolder_ = filterHolder; };

	virtual void refresh() = 0;

	virtual bool finished() = 0;

	virtual int getNumItems() = 0;

	virtual	const AssetInfo getAssetInfo( int index ) = 0;
	virtual	void getThumbnail( ThumbnailManager& manager,
		int index, CImage& img, int w, int h, ThumbnailUpdater* updater ) = 0;

	virtual void filterItems() = 0;

protected:
	FilterHolder* filterHolder_;
};
typedef SmartPointer<ListProvider> ListProviderPtr;



/**
 *	This class derives from CListCtrl to implement a virtual list optimised
 *	to handle large lists.
 */
class SmartListCtrl : public CListCtrl, public ThumbnailUpdater
{
public:
	// Displaying style of items
	enum ViewStyle
	{
		LIST,
		SMALLICONS,
		BIGICONS
	};

	explicit SmartListCtrl( ThumbnailManagerPtr thumbnailManager );
	virtual ~SmartListCtrl();

	void init( ListProviderPtr provider, XmlItemVec* customItems = 0, bool clearSelection = true );

	void setMaxCache( int maxItems );

	ViewStyle getStyle();
	void setStyle( ViewStyle style );
	bool getListViewIcons();
	void setListViewIcons( bool listViewIcons );

	void refresh();

	ListProviderPtr getProvider();

	bool finished();
	bool isCustomItem( int index );
	const AssetInfo getAssetInfo( int index );
	void updateItem( int index, bool removeFromCache = true );
	void updateItem( const AssetInfo& assetInfo, bool removeFromCache = true );
	bool showItem( const AssetInfo& assetInfo );

	void setEventHandler( SmartListCtrlEventHandler* eventHandler );

	void setDefaultIcon( HICON icon );

	void setSearchText( std::string searchText );

	void updateFilters();

	bool isDragging();
	void updateDrag( int x, int y );
	void endDrag();

	void setDropTarget( int index );
	void clearDropTarget();

	void allowMultiSelect( bool allow );

	// ThumbnailUpdater interface implementation
	void thumbManagerUpdate( const std::wstring& longText );

private:
	ViewStyle style_;
	std::vector<AssetInfo> selItems_;
	typedef std::vector<AssetInfo>::iterator SelectedItemItr;
	XmlItemVec* customItems_;
	static const int SMARTLIST_SELTIMER_ID =		1000;
	static const int SMARTLIST_SELTIMER_MSEC =		  50;
	static const int SMARTLIST_LOADTIMER_ID =		1001;
	static const int SMARTLIST_LOADTIMER_MSEC =		 100;
	static const int SMARTLIST_REDRAWTIMER_ID =		1002;
	static const int SMARTLIST_REDRAWTIMER_MSEC =	 200;
	SmartListCtrlEventHandler* eventHandler_;
	ListProviderPtr provider_;
	ThumbnailManagerPtr thumbnailManager_;
	ListCache* listCache_;
	ListCache listCacheBig_;
	ListCache listCacheSmall_;
	bool ignoreSelMessages_;
	CImageList imgListBig_;
	CImageList imgListSmall_;
	CImageList* dragImgList_;
	controls::DragImage * dragImage_;
	bool dragging_;
	bool generateDragListEndItem_;
	int lastListDropItem_;
	int lastItemChanged_;
	int thumbWidth_;
	int thumbHeight_;
	int thumbWidthSmall_;
	int thumbHeightSmall_;
	int thumbWidthCur_;
	int thumbHeightCur_;
	bool listViewIcons_;
	int maxSelUpdateMsec_;
	bool delayedSelectionPending_;
	bool redrawPending_;
	int maxItems_;

	void initCtrl();

	void PreSubclassWindow();

	int binSearch( int size, int begin, int end, const AssetInfo& assetInfo );
	void updateItemInternal( int index, const AssetInfo& inf, bool removeFromCache );
	void getData( int index, std::wstring& text, int& image, bool textOnly = false );
	XmlItem* getCustomItem( int& index );
	void changeItemCount( int numItems );
	void updateSelection();
	void delayedSelectionNotify();

	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnRightClick( NMHDR * pNotifyStruct, LRESULT* result );
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOdFindItem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer( UINT id );
	afx_msg void OnOdStateChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
	afx_msg void OnToolTipText(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

#endif // SMART_LIST_CTRL_HPP
