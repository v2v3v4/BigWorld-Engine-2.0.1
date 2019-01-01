/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef GUITABS_DOCK_HPP
#define GUITABS_DOCK_HPP

namespace GUITABS
{

typedef std::list<PanelPtr>::iterator PanelItr;

/**
 *  This class manages all panel layout functionality, keeping track of
 *  the tree structure of the nested splitter windows that contain docked
 *  panels as well as keeping a list of all panels, docked and floating.
 *  There is only one Dock in the current implementation, but it should be
 *  easy to extend Manager in order to get multiple Docks in an application,
 *  with the restriction that there can only be one Dock per app frame window.
 */
class Dock : public ReferenceCount
{
public:

	Dock( CFrameWnd* mainFrame, CWnd* mainView );
	~Dock();

	CFrameWnd* getMainFrame();
	CWnd* getMainView();

	bool empty();

	void dockTab( PanelPtr panel, TabPtr tab, CWnd* destPanel, InsertAt insertAt, int srcX, int srcY, int dstX, int dstY );

	void dockPanel( PanelPtr panel, CWnd* destPanel, InsertAt insertAt, int srcX, int srcY, int dstX, int dstY );

	void floatPanel( PanelPtr panel, int srcX, int srcY, int dstX, int dstY );
	
	void attachAsTab( PanelPtr panel, CWnd* destPanel );

	PanelPtr insertPanel( const std::wstring & contentID, PanelHandle destPanel, InsertAt insertAt );
	const PanelItr removePanel( PanelPtr panel );

	void removePanel( const std::wstring & contentID );

	PanelPtr getPanelByWnd( CWnd* ptr );
	PanelPtr getPanelByHandle( PanelHandle handle );

	bool load( DataSectionPtr section );
	bool save( DataSectionPtr section );

	void setActivePanel( PanelPtr panel );

	void showPanel( PanelPtr panel, bool show );
	void showPanel( const std::wstring & contentID, bool show );
	void showPanel( ContentPtr content, bool show );

	ContentPtr getContent( const std::wstring & contentID, int index = 0 );

	bool isContentVisible( const std::wstring & contentID );

	DockNodePtr getNodeByPoint( int x, int y );

	void togglePanelPos( PanelPtr panel );
	void toggleTabPos( PanelPtr panel, TabPtr tab );

	FloaterPtr getFloaterByWnd( CWnd* ptr );
	
	bool isDockVisible();

	void showDock( bool show );

	void showFloaters( bool show );

	void destroyFloater( FloaterPtr floater );

	void broadcastMessage( UINT msg, WPARAM wParam, LPARAM lParam );

	void sendMessage( const std::wstring & contentID, UINT msg, WPARAM wParam, LPARAM lParam );

	int getContentCount( const std::wstring & contentID );
	PanelPtr detachTabToPanel( PanelPtr panel, TabPtr tab );

	void rollupPanel( PanelPtr panel );

	int getPanelIndex( PanelPtr panel );
	PanelPtr getPanelByIndex( int index );

	DockNodePtr nodeFactory( DataSectionPtr section );

private:
	bool dockVisible_;
	DockNodePtr dockTreeRoot_;
	std::list<PanelPtr> panelList_;
	std::list<FloaterPtr> floaterList_;
	typedef std::list<FloaterPtr>::iterator FloaterItr;
	CFrameWnd* mainFrame_;
	int originalMainViewID_;
	CWnd* mainView_;

	void insertPanelIntoPanel( PanelPtr panel, CWnd* destPanel, InsertAt insertAt );
	bool getNodeByWnd( CWnd* ptr, DockNodePtr& childNode, DockNodePtr& parentNode );
	void removeNodeByWnd( CWnd* ptr );
	void getLeaves( DockNodePtr node, std::vector<DockNodePtr>& leaves );
	bool buildPanelPosList( bool docked, DockNodePtr node, PanelPtr panel );
	void savePanelDockPos( PanelPtr panel );
	void restorePanelDockPos( PanelPtr panel );
	void copyPanelRestorePosToTab( PanelPtr src, PanelPtr dstTab );
};


} // namespace

#endif // GUITABS_DOCK_HPP