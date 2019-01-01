/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	GUI Tearoff panel framework - Manager class
 */

#ifndef GUITABS_MANAGER_HPP
#define GUITABS_MANAGER_HPP


#include "datatypes.hpp"
#include "cstdmf/singleton.hpp"
#include <list>


namespace GUITABS
{


/**
 *  This singleton class is the only class accesible by the user of the tearoff
 *  panel framework. The programmer must first register it's content factories
 *  and insert a dock into his mainFrame (using registerFactory and insertDock)
 *  in any order, and the the user can load a configuration file or insert
 *  panels manually to actually create and display the desired panels on the
 *  screen (using load or insertPanel). It is required that the programmer
 *  calls removeDock() on exit, before destroying it's main frame and view
 *  windows. It is recommended that the mainView window doesn't have a border.
 *  For more information, please see content.hpp and content_factory.hpp
 */
class Manager : public Singleton<Manager>, public ReferenceCount
{
public:
	Manager();
	~Manager();

	bool registerFactory( ContentFactoryPtr factory );

	bool insertDock( CFrameWnd* mainFrame, CWnd* mainView );

	void removeDock();

	PanelHandle insertPanel( const std::wstring & contentID, InsertAt insertAt, PanelHandle destPanel = 0 );

	bool removePanel( PanelHandle panel );

	bool removePanel( const std::wstring & contentID );

	void removePanels();

	void showPanel( PanelHandle panel, bool show );

	void showPanel( const std::wstring & contentID, bool show );

	bool isContentVisible( const std::wstring & contentID );

	Content* getContent( const std::wstring & contentID, int index = 0 );

	bool isValid( PanelHandle panel );

	bool isDockVisible();

	void showDock( bool show );

	void showFloaters( bool show );

	void broadcastMessage( UINT msg, WPARAM wParam, LPARAM lParam );

	void sendMessage( const std::wstring & contentID, UINT msg, WPARAM wParam, LPARAM lParam );

	bool load( const std::wstring & fname = L"" );

	bool save( const std::wstring & fname = L"" );

	PanelHandle clone( PanelHandle content, int x, int y );

private:
	DockPtr dock_;
	DragManagerPtr dragMgr_;
	std::list<ContentFactoryPtr> factoryList_;
	typedef std::list<ContentFactoryPtr>::iterator ContentFactoryItr;
	std::wstring lastLayoutFile_;

	// Utility methods, for friend classes internal use only.
	// In the case of the dock() method, this was done in order to avoid
	// having a pointer to the Dock inside each of the friend classes.
	friend Tab;
	friend DragManager;
	friend Panel;
	friend SplitterNode;
	friend Floater;
	friend DockedPanelNode;
	friend ContentContainer;

	ContentPtr createContent( const std::wstring & contentID );

	DockPtr dock();

	DragManagerPtr dragManager();
};


} // namespace

#endif // GUITABS_MANAGER_HPP
