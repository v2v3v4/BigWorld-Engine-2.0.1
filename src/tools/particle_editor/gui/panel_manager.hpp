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
#include "guimanager/gui_action_maker.hpp"
#include "guimanager/gui_updater_maker.hpp"


class UalDialog;
class UalItemInfo;


class PanelManager :
	public Singleton<PanelManager>,
	public GUI::ActionMaker<PanelManager>  ,	// show panels
	public GUI::ActionMaker<PanelManager,1>,	// hide panels
	public GUI::ActionMaker<PanelManager, 2>,		// Language selection
	public GUI::UpdaterMaker<PanelManager>,		// update show/hide side panel
	public GUI::UpdaterMaker<PanelManager, 1>,	// update show/hide ual panel
	public GUI::UpdaterMaker<PanelManager, 2>,	// update show/hide messages panel
	public GUI::UpdaterMaker<PanelManager, 3>		// Language selection
{
public:
    ~PanelManager();

	static bool init( CFrameWnd* mainFrame, CWnd* mainView );
	static void fini();

	// panel stuff
    bool ready();
    void showPanel(std::wstring const &pyID, int show);
    bool isPanelVisible(std::wstring const &pyID);
    bool showSidePanel(GUI::ItemPtr item);
    bool hideSidePanel(GUI::ItemPtr item);
    bool setLanguage(GUI::ItemPtr item);
    unsigned int updateSidePanel(GUI::ItemPtr item);
	unsigned int updateUalPanel(GUI::ItemPtr item);
	unsigned int updateMsgsPanel(GUI::ItemPtr item);
	unsigned int updateLanguage(GUI::ItemPtr item);
    void updateControls();
    void onClose();
    void ualAddItemToHistory(std::string filePath);
	bool loadDefaultPanels(GUI::ItemPtr item);
	bool loadLastPanels(GUI::ItemPtr item);

	GUITABS::Manager& panels() { return panels_; }

private:
    PanelManager();

    void finishLoad();
    bool initPanels();

	// UAL callbacks
	void ualItemDblClick(UalItemInfo* ii);
	void ualStartDrag(UalItemInfo* ii);
	void ualUpdateDrag(UalItemInfo* ii);
	void ualEndDrag(UalItemInfo* ii);

private:
	UalManager ualManager_;

	GUITABS::Manager panels_;

    std::map<std::wstring, std::wstring>  m_contentID;
    int                                 m_currentTool;
    CFrameWnd                           *m_mainFrame;
    CWnd                                *m_mainView;
    bool                                m_ready;
    bool                                m_dropable;
	std::string							currentLanguageName_;
	std::string							currentCountryName_;
};

#endif // PANEL_MANAGER_HPP
