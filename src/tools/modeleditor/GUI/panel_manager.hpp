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
#include "ual/ual_manager.hpp"
#include "guitabs/manager.hpp"
#include "guimanager/gui_updater_maker.hpp"

class UalDialog;
//class UalItemInfo;

class PanelManager :
	public Singleton<PanelManager>,
	public GUI::ActionMaker<PanelManager>,	 // load default panels
	public GUI::UpdaterMaker<PanelManager>	 // update show/hide panels
{
public:
	~PanelManager();

	static bool init( CFrameWnd* mainFrame, CWnd* mainView );
	static void fini();

	// panel stuff
	bool ready();
	void showPanel( std::string& pyID, int show = 1 );
	int isPanelVisible( std::string& pyID );
	int currentTool();
	bool handelGUIAction( GUI::ItemPtr item );
	bool showSidePanel( GUI::ItemPtr item );
	bool hideSidePanel( GUI::ItemPtr item );
	unsigned int handleGUIUpdate( GUI::ItemPtr item );
	unsigned int updateSidePanel( GUI::ItemPtr item );
	void updateControls();
	void onClose();
	void ualAddItemToHistory( std::string filePath );

	GUITABS::Manager& panels() { return panels_; }

private:
	PanelManager();

	std::map< std::string, std::wstring > contentID_;

	int currentTool_;
	CFrameWnd* mainFrame_;
	CWnd* mainView_;
	bool ready_;
	HCURSOR addCursor_;

	UalManager ualManager_;

	GUITABS::Manager panels_;

	std::string currentLanguageName_; 
	std::string currentCountryName_; 

	// guimanager stuff
	bool recent_models( GUI::ItemPtr item );
	bool recent_lights( GUI::ItemPtr item );
	bool setLanguage( GUI::ItemPtr item );
	unsigned int updateLanguage( GUI::ItemPtr item );
	bool OnAppAbout( GUI::ItemPtr item );
	bool openHelpFile( const std::string& name, const std::string& defaultFile );
	bool OnToolsReferenceGuide( GUI::ItemPtr item );
	bool OnContentCreation( GUI::ItemPtr item );
	bool OnShortcuts( GUI::ItemPtr item );
	

	// panel stuff
	void finishLoad();
	bool initPanels();
	bool loadDefaultPanels( GUI::ItemPtr item );
	bool loadLastPanels( GUI::ItemPtr item );
	std::string getContentID( std::string& pyID );

	// UAL callbacks
	void ualItemDblClick( UalItemInfo* ii );
	void ualStartDrag( UalItemInfo* ii );
	void ualUpdateDrag( UalItemInfo* ii );
	void ualEndDrag( UalItemInfo* ii );
	void ualStartPopupMenu( UalItemInfo* ii, UalPopupMenuItems& menuItems );
	void ualEndPopupMenu( UalItemInfo* ii, int result );

};


#endif // PANEL_MANAGER_HPP
