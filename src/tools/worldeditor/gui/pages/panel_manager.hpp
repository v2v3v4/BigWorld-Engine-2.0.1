/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PANEL_MANAGER_HPP
#define PANEL_MANAGER_HPP


#include "cstdmf/singleton.hpp"
#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "ual/ual_manager.hpp"
#include "guitabs/manager.hpp"
#include "guimanager/gui_functor_cpp.hpp"
#include <set>


class PanelManager :
	public Singleton<PanelManager>,
	public GUI::ActionMaker<PanelManager>,	// load default panels
	public GUI::ActionMaker<PanelManager,1>,// show panels
	public GUI::ActionMaker<PanelManager,2>,// hide panels
	public GUI::ActionMaker<PanelManager,3>,// load last panels
	public GUI::UpdaterMaker<PanelManager>,	// update show/hide panels
	public GUI::UpdaterMaker<PanelManager,1>// disable/enable show/hide panels
{
public:
	~PanelManager() {};

	static bool init( CFrameWnd* mainFrame, CWnd* mainView );
	static void fini();

	bool ready();
	void updateUIToolMode( const std::wstring& pyID );
	void setToolMode( const std::wstring pyID );
	void setDefaultToolMode();
	void showPanel( const std::wstring pyID, int show );
	int isPanelVisible( const std::wstring pyID );
	const std::wstring currentTool();
	bool isCurrentTool( const std::wstring& id );
	bool showSidePanel( GUI::ItemPtr item );
	bool hideSidePanel( GUI::ItemPtr item );
	unsigned int updateSidePanel( GUI::ItemPtr item );
	unsigned int disableEnablePanels( GUI::ItemPtr item );
	void updateControls();
	void onClose();
    void onNewSpace(unsigned int width, unsigned int height);
	void onChangedChunkState(int x, int z);
	void onNewWorkingChunk();
    void onBeginSave();
    void onEndSave();
	void showItemInUal( const std::wstring& vfolder, const std::wstring& longText );

	// UAL callbacks
	void ualAddItemToHistory( const std::wstring & str, const std::wstring & type );

	GUITABS::Manager& panels() { return panels_; }

private:
	PanelManager();

	std::map<std::wstring,std::wstring> panelNames_; // pair( python ID, GUITABS contentID )
	typedef std::pair<std::wstring,std::wstring> StrPair;
	std::wstring currentTool_;
	CFrameWnd* mainFrame_;
	CWnd* mainView_;
	bool ready_;
	std::set<std::wstring> ignoredObjectTypes_;

	UalManager ualManager_;

	GUITABS::Manager panels_;

	// panel stuff
	void finishLoad();
	bool initPanels();
	bool allPanelsLoaded();
	bool loadDefaultPanels( GUI::ItemPtr item );
	bool loadLastPanels( GUI::ItemPtr item );
	const std::wstring getContentID( const std::wstring pyID );
	const std::wstring getPythonID( const std::wstring contentID );

	// UAL callbacks
	void ualItemClick( UalItemInfo* ii );
	void ualDblItemClick( UalItemInfo* ii );
	void ualStartPopupMenu( UalItemInfo* ii, UalPopupMenuItems& menuItems );
	void ualEndPopupMenu( UalItemInfo* ii, int result );

	void ualStartDrag( UalItemInfo* ii );
	void ualUpdateDrag( UalItemInfo* ii );
	void ualEndDrag( UalItemInfo* ii );

	// other
	void addSimpleError( const std::string& msg );
};


#endif // PANEL_MANAGER_HPP
