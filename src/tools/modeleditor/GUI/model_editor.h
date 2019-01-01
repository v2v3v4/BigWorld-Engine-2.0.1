// ModelEditor.h : main header file for the ModelEditor application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include "me_shell.hpp"
#include "me_app.hpp"
#include "mru.hpp"
#include "me_python_adapter.hpp"

#include "appmgr/app.hpp"

#include "guimanager/gui_manager.hpp"
#include "guimanager/gui_functor_option.hpp"

class UalItemInfo;
class PageLights;

class CModelEditorApp :
	public CWinApp,
	GUI::OptionMap
{
public:
	static DogWatch s_updateWatch;
	
	CModelEditorApp();
	~CModelEditorApp();

	static UINT loadErrorMsg( LPVOID lpvParam );

	static CModelEditorApp & instance() { return *s_instance_; }
	CWnd* mainWnd() { return m_pMainWnd; }
	App* mfApp() { return mfApp_; }
	bool initDone() { return initDone_; };

	bool loadFile( UalItemInfo* item );

	const std::string& modelToLoad() { return modelToLoad_; }
	void modelToLoad( const std::string& modelName ) { modelToLoad_ = modelName; }
		
	//Interface for python calls
	void loadModel( const char* modelName );
	void addModel( const char* modelName );
	void loadLights( const char* lightName );
		
	//The menu commands
	void OnFileOpen();
	void OnFileAdd();
	void OnFileReloadTextures();
	void OnFileRegenBoundingBox();
	void OnAppPrefs();
		
	void updateRecentList( const std::string& kind );

	void updateLanguageList();

	MEPythonAdapter* pythonAdapter() const;

// Overrides
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	virtual int ExitInstance(); 
	virtual int Run();

private:
	static CModelEditorApp * s_instance_;

	App* mfApp_;
	bool initDone_;

	MeShell*	meShell_;
	MeApp*		meApp_;

	PageLights* lightPage_;

	MEPythonAdapter* pPythonAdapter_;

	std::string modelToLoad_;
	std::string modelToAdd_;

	BOOL parseCommandLineMF();
		
	virtual std::string get( const std::string& key ) const;
	virtual bool exist( const std::string& key ) const;
	virtual void set( const std::string& key, const std::string& value );

	BOOL InternalInitInstance();
	int InternalExitInstance();
	int InternalRun();

	DECLARE_MESSAGE_MAP()
};

extern CModelEditorApp theApp;