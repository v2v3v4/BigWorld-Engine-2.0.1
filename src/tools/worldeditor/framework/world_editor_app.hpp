/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WORLD_EDITOR_APP_HPP
#define WORLD_EDITOR_APP_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/resource.h"


class AppFunctor;
typedef SmartPointer<AppFunctor> AppFunctorPtr;


class WorldEditorApp : public CWinApp
{
public:
	static WorldEditorApp & instance() { return *s_instance_; }

	WorldEditorApp();
	~WorldEditorApp();

	CWnd* mainWnd() { return m_pMainWnd; }
	App* mfApp() { return s_mfApp; }

	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run();

	virtual BOOL OnIdle(LONG lCount);

	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	WEPythonAdapter * pythonAdapter() const;

protected:
	DECLARE_MESSAGE_MAP()

	BOOL parseCommandLineMF();

private:
	BOOL InternalInitInstance();
	int InternalExitInstance();
	int InternalRun();

	static WorldEditorApp * s_instance_;
	App					  * s_mfApp;
	WEPythonAdapter		  * pPythonAdapter_;
	HANDLE					updateMailSlot_;
	AppFunctorPtr			pAppFunctor_;
	LPCTSTR					oldAppName_;
};


extern WorldEditorApp theApp;


#endif // WORLD_EDITOR_APP_HPP
