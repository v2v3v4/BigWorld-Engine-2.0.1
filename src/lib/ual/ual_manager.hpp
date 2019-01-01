/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef UAL_MANAGER_HPP
#define UAL_MANAGER_HPP

#include "cstdmf/singleton.hpp"
#include "ual_callback.hpp"
#include "guimanager/gui_action_maker.hpp"
#include "ual_history.hpp"
#include "ual_favourites.hpp"
#include "ual_drop_manager.hpp"
#include "thumbnail_manager.hpp"


// Forward declarations
class UalDialog;
class UalDialogFactory;


/**
 *	This class is a singleton that anages interaction between the app and the
 *	Asset Browser dialog(s).
 */
class UalManager :
	public Singleton<UalManager>,
	public GUI::ActionMaker<UalManager>
{
public:
	UalManager();
	~UalManager();

	// Used by the app to add default search paths
	void addPath( const std::wstring& path );

	// Used by the app to set the config file path
	// (should be called before creating any ual dialogs)
	void setConfigFile( std::wstring config );

	// Get the config file path
	const std::wstring getConfigFile();

	// History and Favourites accesors
	UalHistory& history() { return history_; }
	UalFavourites& favourites() { return favourites_; }

	// ThumbnailManager accessor
	ThumbnailManager& thumbnailManager() { return *thumbnailManager_; }

	// DropManager accessor
	UalDropManager& dropManager() { return dropManager_; }

	// Used by the app to set the corresponding callbacks
	// Note: Memory is disposed by the UalManager, so set callbacks using new
	// see comments for each one. Also callbacks that receive UalItemInfo pointers
	// must handle NULL UalItemInfo pointer, and may also handle multiple items via
	// the getNext method of the UalItemInfo. UalItemInfo objects are temporary, valid
	// only inside the callback.
	//  - UalFunctor1<YourClass, UalItemInfo*>
	void setItemClickCallback( UalItemCallback* callback ) { itemClickCallback_ = callback; };
	void setItemDblClickCallback( UalItemCallback* callback ) { itemDblClickCallback_ = callback; };
	void setPopupMenuCallbacks( UalStartPopupMenuCallback* callback1, UalEndPopupMenuCallback* callback2 )
		{ startPopupMenuCallback_ = callback1; endPopupMenuCallback_ = callback2; };
	void setStartDragCallback( UalItemCallback* callback ) { startDragCallback_ = callback; };
	void setUpdateDragCallback( UalItemCallback* callback ) { updateDragCallback_ = callback; };
	void setEndDragCallback( UalItemCallback* callback ) { endDragCallback_ = callback; };
	//  - UalFunctor2<YourClass, UalDialog*, bool>
	void setFocusCallback( UalFocusCallback* callback ) { focusCallback_ = callback; };
	//  - UalFunctor1<YourClass, const std::string&>
	void setErrorCallback( UalErrorCallback* callback ) { errorCallback_ = callback; };

	// methods used internally, but can also be used by the app if needed
	const std::wstring getPath( int i );
	int getNumPaths();

	// Returns the currently active UalDialog
	UalDialog* getActiveDialog();

	// Forces a refresh a single item in all dialogs
	void updateItem( const std::wstring& longText );

	void refreshAllDialogs();

	// Selects the vfolder and item specified
	void showItem( const std::wstring& vfolder, const std::wstring& longText );

	// call this method before exiting in order to clean up
	void fini();

private:
	std::vector<std::wstring> paths_;
	std::vector<UalDialog*> dialogs_;
	typedef std::vector<UalDialog*>::iterator DialogsItr;
	std::wstring configFile_;
	UalHistory history_;
	UalFavourites favourites_;
	ThumbnailManagerPtr thumbnailManager_;
	UalDropManager dropManager_;
	UINT_PTR timerID_;

	SmartPointer<UalItemCallback> itemClickCallback_;
	SmartPointer<UalItemCallback> itemDblClickCallback_;
	SmartPointer<UalStartPopupMenuCallback> startPopupMenuCallback_;
	SmartPointer<UalEndPopupMenuCallback> endPopupMenuCallback_;
	SmartPointer<UalItemCallback> startDragCallback_;
	SmartPointer<UalItemCallback> updateDragCallback_;
	SmartPointer<UalItemCallback> endDragCallback_;
	SmartPointer<UalFocusCallback> focusCallback_;
	SmartPointer<UalErrorCallback> errorCallback_;

	friend UalDialog;
	friend UalDialogFactory;

	void favouritesCallback();
	void historyCallback();

	static void CALLBACK onTimer(HWND hwnd, UINT nMsg, UINT_PTR nIDEvent, DWORD dwTime );

	UalItemCallback* itemClickCallback() { return itemClickCallback_.getObject(); };
	UalItemCallback* itemDblClickCallback() { return itemDblClickCallback_.getObject(); };
	UalStartPopupMenuCallback* startPopupMenuCallback() { return startPopupMenuCallback_.getObject(); };
	UalEndPopupMenuCallback* endPopupMenuCallback() { return endPopupMenuCallback_.getObject(); };
	UalItemCallback* startDragCallback() { return startDragCallback_.getObject(); };
	UalItemCallback* updateDragCallback() { return updateDragCallback_.getObject(); };
	UalItemCallback* endDragCallback() { return endDragCallback_.getObject(); };
	UalFocusCallback* focusCallback() { return focusCallback_.getObject(); };
	UalErrorCallback* errorCallback() { return errorCallback_.getObject(); };

	void registerDialog( UalDialog* dialog );
	void unregisterDialog( UalDialog* dialog );

	bool handleGUIAction( GUI::ItemPtr item );
	bool guiActionRefresh( GUI::ItemPtr item );
	bool guiActionLayout( GUI::ItemPtr item );

	void cancelDrag();
	UalDialog* updateDrag( const UalItemInfo& itemInfo, bool endDrag );
	void copyVFolder( UalDialog* srcUal, UalDialog* dstUal, const UalItemInfo& ii );
};



#endif UAL_MANAGER_HPP
