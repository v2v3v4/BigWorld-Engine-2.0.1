/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef GUITABS_PANEL_HPP
#define GUITABS_PANEL_HPP

#if defined(_MSC_VER) && _MSC_VER >= 1400
#define HITTESTRESULT HRESULT
#else
#define HITTESTRESULT UINT
#endif

namespace GUITABS
{


/*
 *  This simple struct is used to save a panel's relative position to another
 *  destination panel ( destPanel ). A list of relative positions is kept for
 *	the last docked positions as well as for the last floating positions. When
 *  the user double-clicks on a panel's title bar, the panel will toggle
 *  between the last floating position and the last docked position using this
 *  struct.
 *	Each PanelPos entry contains info about this panels position relative to
 *	others, like "Left to the Asset Browser", "Top of the Properties", "As a
 *	tab in the Object panel" and so on.
 *
 *  @see Panel, Panel::insertPos
 */
struct PanelPos
{
	PanelPos() : insertAt( UNDEFINED_INSERTAT ), destPanel( 0 ) { }
	PanelPos( InsertAt ins , CWnd* pan ) : insertAt( ins ), destPanel( pan ) { }
	InsertAt insertAt;
	CWnd* destPanel;  // not a smart pointer, it's only a weak reference;
};


/**
 *  This class encapsulates the functionality of a panel, and manages one
 *  or more tabs that can be contained inside it. Also, this class is
 *  responsible for drawing all the panel controls, including the tabs, close
 *  button, clone button, titlebar, etc.
 */
class Panel : public CWnd, public TabCtrlEventHandler, public ReferenceCount
{
public:

	Panel( CWnd* parent );
	~Panel();

	CWnd* getCWnd();

	void addTab( const std::wstring contentID );
	void addTab( TabPtr tab );
	void detachTab( TabPtr tab );
	void detachTab( const std::wstring contentID );
	TabPtr detachFirstTab();

	bool load( DataSectionPtr section );
	bool save( DataSectionPtr section );

	void activate();
	void deactivate();

	bool isExpanded();
	void setExpanded( bool expanded );

	bool isFloating();
	void setFloating( bool floating );

	void getPreferredSize( int& width, int& height );

	int getCaptionSize();
	int getTabCtrlSize();
	bool isTabCtrlAtTop();

	void clearPosList( bool docked );
	void resetPosList( bool docked );
	void insertPos( bool docked, PanelPos pos );
	bool getNextPos( bool docked, PanelPos& pos );
	void getLastPos( int& x, int& y );
	void setLastPos( int x, int y );

	bool contains( ContentPtr content );
	int contains( const std::wstring contentID );
	ContentPtr getContent( const std::wstring contentID );
	ContentPtr getContent( const std::wstring contentID, int& index );

	void broadcastMessage( UINT msg, WPARAM wParam, LPARAM lParam );

	// TabCtrl event handler implementation
	void clickedTab( void* itemData, int x, int y );
	void doubleClickedTab( void* itemData, int x, int y );
	void rightClickedTab( void* itemData, int x, int y );

	void showTab( TabPtr tab, bool show );
	void showTab( const std::wstring contentID, bool show );
	void showTab( ContentPtr content, bool show );
	bool isTabVisible( const std::wstring contentID );
	ContentPtr cloneTab( ContentPtr content, int x, int y );

	int tabCount();
	int visibleTabCount();

	void updateTabPosition( int x, int y );
	int getTabInsertionIndex( int x, int y );
	void setActiveTabIndex( int index );
	int getActiveTabIndex();

	bool onClose();

	int getIndex();

	void recalcSize();

	void insertTempTab( TabPtr tab );
	void updateTempTab( int x, int y );
	void removeTempTab();

	TabPtr getActiveTab();

private:
	std::list<TabPtr> tabList_;
	typedef std::list<TabPtr>::iterator TabItr;
	TabPtr activeTab_;
	TabCtrlPtr tabBar_;
	bool isFloating_;
	bool isExpanded_;
	int expandedSize_;
	// Lists of preferred saved docking locations, both for when in a floating
	// window and when docked in the mainframe.
	/// @see PanelPos, Panel::insertPos
	typedef std::vector<PanelPos>::iterator PanelPosItr;
	std::vector<PanelPos> dockedPosList_;
	PanelPosItr dockedPosItr_;
	std::vector<PanelPos> floatingPosList_;
	PanelPosItr floatingPosItr_;
	// Last floating pos
	int lastX_;
	int lastY_;
	TabPtr tempTab_; // used to give drag&drop feedback

	void recalcSize( int w, int h );
	void setActiveTab( TabPtr tab );
	void updateTabBar();

	bool tabContains( TabPtr t, ContentPtr content );
	int tabContains( TabPtr t, const std::wstring contentID );

// MFC specific stuff
	static const int CAPTION_HEIGHT = 16;
	static const int CAPTION_FONT_SIZE = 13;
	static const int CAPTION_LEFTMARGIN = 4;
	static const int CAPTION_TOPMARGIN = 1;
	static const int PANEL_ROLLUP_SIZE = 16;

	static const int HOVER_TIMERID = 1;
	static const int HOVER_TIMERMILLIS = 100;

	static const int BUT_CLOSE = 100;
	static const int BUT_ROLLUP = 101;
	static const int BUT_CLONE = 102;


	CFont captionFont_;
	bool isActive_;
	UINT buttonDown_;

	void paintCaptionBar();
	void paintCaptionBarOnly();
	void paintCaptionButtons( UINT hitButton = 0 );
	UINT hitTest( const CPoint point );
	bool onTabClose( TabPtr tab );

	afx_msg void OnNcCalcSize( BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp );
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg HITTESTRESULT OnNcHitTest( CPoint point );
	afx_msg void OnNcLButtonDown( UINT nHitTest, CPoint point );
	afx_msg void OnNcLButtonDblClk( UINT nHitTest, CPoint point );
	afx_msg void OnNcLButtonUp( UINT nHitTest, CPoint point );
	afx_msg void OnNcRButtonDown( UINT nHitTest, CPoint point );
	afx_msg void OnNcMouseMove( UINT nHitTest, CPoint point );
	afx_msg int OnMouseActivate( CWnd* pDesktopWnd, UINT nHitTest, UINT message );
	afx_msg void OnSize( UINT nType, int cx, int cy );
	DECLARE_MESSAGE_MAP()

};


} // namespace

#endif // GUITABS_PANEL_HPP