/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PARTICLE_EDITOR_H
#define PARTICLE_EDITOR_H

#include "resource.h"
#include "fwd.hpp"
#include "pyscript/script.hpp"
#include "guimanager/gui_functor_option.hpp"


class App;


class ParticleEditorApp : public CWinApp,
	                      public GUI::OptionMap
{
public:
    ParticleEditorApp();

    ~ParticleEditorApp();
    
	bool InitialiseMF( std::string &openFile );
    /*virtual*/ BOOL InitInstance();

    /*virtual*/ int ExitInstance();

	/*virtual*/ int Run();

	bool setLanguage( GUI::ItemPtr item );
	unsigned int updateLanguage( GUI::ItemPtr item );
	void updateLanguageList();

    void openDirectory(std::string const &dir, bool forceRefresh = false);

    void setFrameRate(float rate) { m_desiredFrameRate = rate; }    // No more frame limiting (Bug 4834)

    void OnDirectoryOpen();

    void OnFileSaveParticleSystem();

    enum State
    {
        PE_PLAYING,
        PE_PAUSED,
        PE_STOPPED
    };

    void setState(State state);

    State getState() const;

    void OnAppAbout();

	bool openHelpFile( const std::string& name, const std::string& defaultFile );
		
	void OnToolsReferenceGuide();

	void OnContentCreation();

    void OnAppShortcuts();

	void OnViewShowSide();

    void OnViewShowUAL();

    void OnViewShowMsgs();

    static ParticleEditorApp & instance() { return *s_instance; }

	App* mfApp() { return mfApp_; }

	CWnd* mainWnd() { return m_pMainWnd; }

protected:

    /*virtual*/ BOOL OnIdle(LONG lCount);
	BOOL InternalInitInstance();
	int InternalExitInstance();
	int InternalRun();

    DECLARE_MESSAGE_MAP()

	/*virtual*/ std::string get(std::string const &key) const;
	/*virtual*/ bool exist(std::string const &key) const;
	/*virtual*/ void set(std::string const &key, std::string const &value);

    void update();

    typedef ParticleEditorApp This;

    PY_MODULE_STATIC_METHOD_DECLARE(py_update)
    PY_MODULE_STATIC_METHOD_DECLARE(py_particleSystem)   
    
private:

private:
    PeShell                     *m_appShell;
    PeApp                       *m_peApp;
    App                         *mfApp_;
    float                       m_desiredFrameRate;     // No more frame limiting (Bug 4834)
    State                       m_state;
    static ParticleEditorApp    *s_instance;
};

extern ParticleEditorApp theApp;

#endif // PARTICLE_EDITOR_H
