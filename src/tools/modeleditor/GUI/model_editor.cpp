/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// ModelEditor.cpp : Defines the class behaviors for the application.
//
#include "pch.hpp"

#include "main_frm.h"

#include "model_editor_doc.h"
#include "model_editor_view.h"
#include "prefs_dialog.hpp"

#include "tools_camera.hpp"
#include "utilities.hpp"

#include "me_module.hpp"
#include "me_app.hpp"
#include "me_shell.hpp"
#include "python_adapter.hpp"

#include "pyscript/py_output_writer.hpp"

#include "appmgr/app.hpp"
#include "appmgr/options.hpp"
#include "common/compile_time.hpp"
#include "common/tools_common.hpp"
#include "common/directory_check.hpp"
#include "common/cooperative_moo.hpp"
#include "common/command_line.hpp"
#include "common/file_dialog.hpp"
#include "common/string_utils.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/string_provider.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_loader.hpp"
#if UMBRA_ENABLE
#include "chunk/chunk_umbra.hpp"
#endif
#include "cstdmf/bgtask_manager.hpp"

#include "gizmo/gizmo_manager.hpp"
#include "gizmo/tool_manager.hpp"

#include "guimanager/gui_manager.hpp"
#include "guimanager/gui_menu.hpp"
#include "guimanager/gui_toolbar.hpp"
#include "guimanager/gui_functor.hpp"
#include "guimanager/gui_functor_cpp.hpp"
#include "guimanager/gui_functor_option.hpp"

#include "ual/ual_manager.hpp"

#include "page_lights.hpp"
#include "page_messages.hpp"

#include "panel_manager.hpp"

#include "loading_dialog.hpp"

#include "material_preview.hpp"

#include "resmgr/string_provider.hpp"
#include "resmgr/auto_config.hpp"

#include "chunk/chunk_space.hpp"
#include "terrain/terrain_settings.hpp"

DECLARE_DEBUG_COMPONENT( 0 )
#include "me_error_macros.hpp"

#include "model_editor.h"
#include "cstdmf/restart.hpp"
#include "cstdmf/bwversion.hpp"

#include <string>
#include <fstream>

DogWatch CModelEditorApp::s_updateWatch( "Page Updates" );
static AutoConfigString s_LanguageFile( "system/language" );



/*static*/ CMainFrame* CMainFrame::s_instance = NULL;

// The one and only CModelEditorApp object

CModelEditorApp theApp;

// CModelEditorApp

BEGIN_MESSAGE_MAP(CModelEditorApp, CWinApp)
END_MESSAGE_MAP()


CModelEditorApp * CModelEditorApp::s_instance_ = NULL;


/**
 * TODO: to be documented.
 */
template< class C >
class AnimLoadFunctor: public AnimLoadCallback
{
public:
	typedef void (C::*Method)();

	AnimLoadFunctor( C* instance, Method method ):
		instance_(instance),
		method_(method)
	{}

	void execute()
	{
		BW_GUARD;

		if ((instance_) && (method_))
			(instance_->*method_)();
	}
private:
	C* instance_;
	Method method_;
};


CModelEditorApp::CModelEditorApp():
	mfApp_(NULL),
	initDone_( false ),
	pPythonAdapter_(NULL)
{
	BW_GUARD;

	ASSERT(s_instance_ == NULL);
	s_instance_ = this;

	EnableHtmlHelp();

	char dateStr [9];
    char timeStr [9];
    _strdate( dateStr );
    _strtime( timeStr );

	static const int MAX_LOG_NAME = 8192;
	static wchar_t logName[ MAX_LOG_NAME + 1 ] = L"";
	if( logName[0] == 0 )
	{
		DWORD len = GetModuleFileName( NULL, logName, MAX_LOG_NAME );
		while( len && logName[ len - 1 ] != L'.' )
			--len;
		wcscpy( logName + len, L"log" );
	}

	std::ostream* logFile = NULL;
		
	if( logName[0] != 0 )
	{
		logFile = new std::ofstream( logName, std::ios::app );

		*logFile << "\n/-----------------------------------------------"
			"-------------------------------------------\\\n";

		*logFile << "BigWorld Model Editor " << BWVersion::versionString()
			<< " (compiled at " << aboutCompileTimeString << ") starting on "
			<< dateStr << " " << timeStr << "\n\n";
		
		logFile->flush();
	}

	//Instanciate the Message handler to catch BigWorld messages
	MsgHandler::instance().logFile( logFile );
}


CModelEditorApp::~CModelEditorApp()
{
	BW_GUARD;

    ASSERT(s_instance_ != NULL);
	s_instance_ = NULL;
}

//A method to allow the load error dialog to not interfere with loading
/*static*/ UINT CModelEditorApp::loadErrorMsg( LPVOID lpvParam )
{
	BW_GUARD;

	const char* modelName = (char*)lpvParam;

	::MessageBox( AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
		Localise(L"MODELEDITOR/GUI/MODEL_EDITOR/ME_CRASHED", modelName), 
		Localise(L"MODELEDITOR/GUI/MODEL_EDITOR/MODEL_LOAD_ERROR"), MB_OK | MB_ICONERROR);
	return 0;
}


// CModelEditorApp initialisation
namespace
{
	wchar_t * s_pCmdLine = NULL;

}


BOOL CModelEditorApp::InitInstance()
{
	BOOL result = CallWithExceptionFilter( this, &CModelEditorApp::InternalInitInstance );

	if (!result)
	{
		MessageBox(
			NULL,
			L"ModelEditor failed to initailise itself correctly, please check the debug log for detailed information.",
			L"ModelEditor",
			MB_OK );
	}

	return result;
}

int CModelEditorApp::ExitInstance()
{
	return CallWithExceptionFilter( this, &CModelEditorApp::InternalExitInstance );
}

int CModelEditorApp::Run()
{
	return CallWithExceptionFilter( this, &CModelEditorApp::InternalRun );
}


BOOL CModelEditorApp::InternalInitInstance()
{
	BW_GUARD;

	waitForRestarting();

	//Let the user know something is going on
	SetCursor( ::LoadCursor( NULL, IDC_WAIT ));
	
	CWinApp::InitInstance();

	size_t cmdLineSize = wcslen( this->m_lpCmdLine );
	s_pCmdLine = new wchar_t[ cmdLineSize + 1 ];
	// copy the command line for parsing
	wcscpy( s_pCmdLine, this->m_lpCmdLine );

	// Initialise OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();

	// Need to disable custom allocation so MFC doesn't get confused on exit
	// trying to delete the doc template created classes below.
	int oldMallocEnabled = bw_malloc_enabled( 0 );

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CModelEditorDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CModelEditorView));
	AddDocTemplate(pDocTemplate);

	// Re-enabling custom allocation
	bw_malloc_enabled( oldMallocEnabled );

	//Assume there will be nothing to load initially,
	//Do it now since "parseCommandLineMF" may set it
	modelToLoad_ = "";
	modelToAdd_ = "";
	
	// Parse command line for standard shell commands, DDE, file open
	MFCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// initialise the MF file services (read in the command line arguments too)
	if (!parseCommandLineMF())
		return FALSE;

	if( !s_LanguageFile.value().empty() )
		StringProvider::instance().load( BWResource::openSection( s_LanguageFile ) );
	std::vector<DataSectionPtr> languages;

	Options::pRoot()->openSections( "language", languages );
	if (!languages.empty())
	{
		for( std::vector<DataSectionPtr>::iterator iter = languages.begin(); iter != languages.end(); ++iter )
			if( !(*iter)->asString().empty() )
				StringProvider::instance().load( BWResource::openSection( (*iter)->asString() ) );
	}
	else
	{
		// Force English:
		StringProvider::instance().load( BWResource::openSection("helpers/languages/modeleditor_gui_en.xml"));
		StringProvider::instance().load( BWResource::openSection("helpers/languages/modeleditor_rc_en.xml" ));
		StringProvider::instance().load( BWResource::openSection("helpers/languages/files_en.xml"          ));
	}

	std::string currentLanguage = Options::getOptionString( "currentLanguage", "" );
	std::string currentCountry  = Options::getOptionString( "currentCountry", "" );
	if ( currentLanguage != "" )
		StringProvider::instance().setLanguages( bw_utf8tow( currentLanguage ), bw_utf8tow( currentCountry ) );
	else
		StringProvider::instance().setLanguage();

	// Check the use-by date
	if (!ToolsCommon::canRun())
	{
		ToolsCommon::outOfDateMessage( "ModelEditor" );
		return FALSE;
	}

	WindowTextNotifier::instance();

	CooperativeMoo::init();

	GUI::Manager::init();

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	// This also creates all the GUI windows
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialised, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_pMainWnd->UpdateWindow();

	// initialise the MF App components
	ASSERT( !mfApp_ );
	mfApp_ = new App;

	ASSERT( !meShell_ );
	meShell_ = new MeShell;

#ifdef INDIE
	if (! LicenseManagement::validateLicense(true, m_pMainWnd->GetSafeHwnd()))
	{
		ERROR_MSG("License validation failed\n");
		//doing exit as cleaning the exit code is too complicated currently.
		_exit(1);
	}
#endif 

	HINSTANCE hInst = AfxGetInstanceHandle();
	CMainFrame * mainFrame = (CMainFrame *)(m_pMainWnd);
	CWnd* mainView = mainFrame->GetActiveView();

	if (!mfApp_->init( hInst, m_pMainWnd->GetSafeHwnd(), mainView->GetSafeHwnd(), MeShell::initApp ))
	{
		ERROR_MSG( "CModelEditorApp::InitInstance - init failed\n" );
		return FALSE;
	}

	//give a warning if there is no terrain info or space
	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (!pSpace || (pSpace && !pSpace->terrainSettings().exists()))
	{
		ERROR_MSG( "Could not open the default space. Terrain and Game Lighting preview will be disabled.\n");
	}

	ASSERT( !meApp_ );
	meApp_ = new MeApp;

	// need to load the adapter before the load thread begins, but after the modules
	pPythonAdapter_ = new MEPythonAdapter();

	// Prepare the GUI
	GUI::Manager::instance().optionFunctor().setOption( this );

	DataSectionPtr section = BWResource::openSection( "resources/data/gui.xml" );
	for( int i = 0; i < section->countChildren(); ++i )
		GUI::Manager::instance().add( new GUI::Item( section->openChild( i ) ) );

#ifdef INDIE
	GUI::Manager::instance()( "MainMenu/Help" )->remove( "RequestFeature" );
	GUI::Manager::instance()( "MainMenu/Help" )->remove( "SEPARATOR" );
#endif 


	//Setup the main menu
	GUI::Manager::instance().add(
		new GUI::Menu( "MainMenu", AfxGetMainWnd()->GetSafeHwnd() ));

	updateLanguageList();

	AfxGetMainWnd()->DrawMenuBar();

	// Add the toolbar(s) through the BaseMainFrame base class
	( (CMainFrame*) AfxGetMainWnd() )->createToolbars( "AppToolbars" );

	// GUITABS Tearoff tabs system and AssetBrowser init and setup
	PanelManager::init( mainFrame, mainView );

	//Add some drop acceptance functors to the Asset Browser
	UalManager::instance().dropManager().add(
		new UalDropFunctor< CModelEditorApp >( mainView, "model", this, &CModelEditorApp::loadFile, true ),
		false /*can't highlight the DX view*/ );

	UalManager::instance().dropManager().add(
		new UalDropFunctor< CModelEditorApp >( mainView, "mvl", this, &CModelEditorApp::loadFile ),
		false /*can't highlight the DX view*/  );

	UalManager::instance().dropManager().add(
		new UalDropFunctor< CModelEditorApp >( mainView, "", this, &CModelEditorApp::loadFile ),
		false /*can't highlight the DX view*/  );

	BgTaskManager::instance().startThreads( 1 );
	
	if (modelToLoad_ != "")
	{
		modelToLoad_ = BWResource::dissolveFilename( modelToLoad_ );
	}
	else if (Options::getOptionInt( "startup/loadLastModel", 1 ))
	{
		modelToLoad_ = Options::getOptionString( "models/file0", "" );
		
		if (!Options::getOptionBool( "startup/lastLoadOK", true ))
		{
			static char modelName[256];
			strcpy( modelName, modelToLoad_.c_str() );
			AfxBeginThread( &CModelEditorApp::loadErrorMsg, modelName );

			//Remove this model from the MRU models list
			MRU::instance().update( "models", modelToLoad_, false );

			Options::setOptionBool( "startup/lastLoadOK", true );
			Options::save();

			modelToLoad_ = "";
		}
	}

	//If there is no model to load, restore the cursor
	if (modelToLoad_ == "")
	{
		updateRecentList("models");
		SetCursor( ::LoadCursor( NULL, IDC_ARROW ));
	}

	updateRecentList("lights");

	initDone_ = true;

	if (!Options::optionsFileExisted())
	{
		Options::setOptionInt("messages/errorMsgs", 1); // turn on showing of error messages
		ERROR_MSG("options.xml is missing\n");
	}
	//oldView->SetFocus();

	//mainFrame->SetForegroundWindow();

	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand

	/* Disable Umbra if it is enabled
	 * This fixes mouse lag issues caused by the present thread allowing the cpu to 
	 * get a few frames ahead of the GPU and then stalling for it to catch up
	 * Note identical code is set in the particle editor InitInstance code, please
	 * update it if you update the code below.
	 */
	#if UMBRA_ENABLE
	if (Options::getOptionInt( "render/useUmbra", 1) == 1 )
	{
		WARNING_MSG( "Umbra is enabled in ModelEditor, It will now be disabled\n" );
	}
	Options::setOptionInt( "render/useUmbra", 0 );
	UmbraHelper::instance().umbraEnabled( false );
	#endif
	return TRUE;
}

BOOL CModelEditorApp::parseCommandLineMF()
{
	BW_GUARD;

	DirectoryCheck( L"ModelEditor" );
	
	// parse command line
	const int MAX_ARGS = 20;
	char * argv[ MAX_ARGS ];
	int argc = 0;

	char cmdline [32768];
	bw_wtoutf8( s_pCmdLine, wcslen( s_pCmdLine ), cmdline, ARRAY_SIZE( cmdline ) );
	char * str = cmdline;
	while (char * token = StringUtils::retrieveCmdTokenT( str ))
	{
		if (argc >= MAX_ARGS)
		{
			ERROR_MSG( "ModelEditor::parseCommandLineMF: Too many arguments!!\n" );
			return FALSE;
		}
		if (argc && (!strcmp( argv[ argc-1 ], "-o" ) || !strcmp( argv[ argc-1 ], "-O" )))
		{
			modelToLoad_ = token;
		}
		argv[argc++] = token;
	}
	
	return Options::init( argc, argv ) && BWResource::init( argc, (const char **)argv );
}

void CModelEditorApp::updateRecentList( const std::string& kind )
{
	BW_GUARD;

	GUI::ItemPtr recentFiles = GUI::Manager::instance()( "/MainMenu/File/Recent_" + kind );
	if( recentFiles )
	{
		while( recentFiles->num() )
			recentFiles->remove( 0 );

		std::vector<std::string> files;
		MRU::instance().read(kind, files);

		for( unsigned i=0; i < files.size(); i++ )
		{
			std::stringstream name, displayName;
			name << kind << i;
			if (i <= 9)
				displayName << '&' << i << "  " << files[i];
			else
				displayName << "    " << files[i];
			GUI::ItemPtr item = new GUI::Item( "ACTION", name.str(), displayName.str(),
				"",	"", "", "recent_"+kind, "", "" );
			item->set( "fileName", files[i] );
			recentFiles->add( item );
		}
	}
}

void CModelEditorApp::updateLanguageList()
{
	BW_GUARD;

	GUI::ItemPtr languageList = GUI::Manager::instance()( "/MainMenu/Languages/LanguageList" );
	if( languageList )
	{
		while( languageList->num() )
			languageList->remove( 0 );
		for( unsigned int i = 0; i < StringProvider::instance().languageNum(); ++i )
		{
			LanguagePtr l = StringProvider::instance().getLanguage( i );
			std::wstringstream wname, wdisplayName;
			wname << L"language" << i;
			wdisplayName << L'&' << l->getLanguageName();
			std::string name, displayName;
			bw_wtoutf8( wname.str(), name );
			bw_wtoutf8( wdisplayName.str(), displayName );
			GUI::ItemPtr item = new GUI::Item( "CHILD", name, displayName,
				"",	"", "", "setLanguage", "updateLanguage", "" );
			item->set( "LanguageName", l->getIsoLangNameUTF8() );
			item->set( "CountryName", l->getIsoCountryNameUTF8() );
			languageList->add( item );
		}
	}
}

MEPythonAdapter * CModelEditorApp::pythonAdapter() const
{
	BW_GUARD;

	if (!pPythonAdapter_->hasScriptObject())
	{
		return NULL;
	}
	return pPythonAdapter_;
}

BOOL CModelEditorApp::OnIdle(LONG lCount)
{
	BW_GUARD;

	static bool s_justLoaded = false;
	
	// The following two lines need to be uncommented for toolbar docking to
	// work properly, and the application's toolbars have to inherit from
	// GUI::CGUIToolBar
	if ( CWinApp::OnIdle( lCount ) )
		return TRUE; // give priority to windows GUI as MS says it should be.

	if (CMainFrame::instance().cursorOverGraphicsWnd())
	{
		::SetFocus( MeShell::instance().hWndGraphics() );
	}

	if (s_justLoaded)
	{
		Options::setOptionBool( "startup/lastLoadOK", true );
		Options::save();
		s_justLoaded = false;
	}

	if (modelToLoad_ != "")
	{
		SetCursor( ::LoadCursor( NULL, IDC_WAIT ));

		std::string::size_type first = modelToLoad_.rfind("/") + 1;
		std::string::size_type last = modelToLoad_.rfind(".");
		std::string modelName = modelToLoad_.substr( first, last-first );

		int numAnim = MeApp::instance().mutant()->animCount( modelToLoad_ );
		bool needsBBCalc = Options::getOptionInt( "settings/regenBBOnLoad", 1 ) &&
			(!MeApp::instance().mutant()->hasVisibilityBox( modelToLoad_ ));
		
		CLoadingDialog* load = NULL;
		if (numAnim > 4)
		{
			load = new CLoadingDialog( Localise(L"MODELEDITOR/GUI/MODEL_EDITOR/LOADING", modelName ) );
			load->setRange( needsBBCalc ? 2*numAnim + 1 : numAnim + 1 );
			Model::setAnimLoadCallback(
				new AnimLoadFunctor< CLoadingDialog >(load, &CLoadingDialog::step) );
		}
			
		ME_INFO_MSGW( Localise(L"MODELEDITOR/GUI/MODEL_EDITOR/LOADING_MODEL", modelToLoad_ ) );

		Options::setOptionBool( "startup/lastLoadOK", false );
		Options::save();

		if (MeApp::instance().mutant()->loadModel( modelToLoad_ ))
		{
			s_justLoaded = true;

			if (needsBBCalc)
			{
				MeApp::instance().mutant()->recreateModelVisibilityBox(
					new AnimLoadFunctor< CLoadingDialog >(load, &CLoadingDialog::step), false );

				//Let the user know
				ME_WARNING_MSGW( Localise(L"MODELEDITOR/GUI/MODEL_EDITOR/VIS_BOX_AUTO_CALC") );

				UndoRedo::instance().forceSave();
			}
			
			MRU::instance().update( "models", modelToLoad_, true );
			updateRecentList( "models" );

			MeModule::instance().materialPreviewMode( false );
			
			MeApp::instance().camera()->boundingBox(
				MeApp::instance().mutant()->zoomBoundingBox() );

			if (!!Options::getOptionInt( "settings/zoomOnLoad", 1 ))
			{
				MeApp::instance().camera()->zoomToExtents( false );
			}

			MeApp::instance().camera()->render( 0.f );

			PanelManager::instance().ualAddItemToHistory( modelToLoad_ );

			// Forcefully update any gui stuff
			CMainFrame::instance().updateGUI( true );
		}
		else
		{
			ERROR_MSG( "Unable to load \"%s\" since an error occured.\n", modelToLoad_.c_str() );
			
			//Remove this model from the MRU models list
			MRU::instance().update( "models", modelToLoad_, false );
			updateRecentList( "models" );
		}
		SetCursor( ::LoadCursor( NULL, IDC_ARROW ));
		modelToLoad_ = "";
		if (load)
		{
			Model::setAnimLoadCallback( NULL );
			delete load;
		}
	}
	else if (modelToAdd_ != "")
	{
		if (MeApp::instance().mutant()->addModel( modelToAdd_ ))
		{
			MRU::instance().update( "models", modelToAdd_, true );
			updateRecentList( "models" );
				
			MeApp::instance().camera()->boundingBox(
					MeApp::instance().mutant()->zoomBoundingBox() );

			if (!!Options::getOptionInt( "settings/zoomOnLoad", 1 ))
			{
				MeApp::instance().camera()->zoomToExtents( false );
			}

			MeApp::instance().camera()->render( 0.f );

			ME_INFO_MSGW( Localise(L"MODELEDITOR/GUI/MODEL_EDITOR/ADDED_MODEL", modelToAdd_ ) );

			PanelManager::instance().ualAddItemToHistory( modelToAdd_ );
		}
		else
		{
			ME_WARNING_MSGW( Localise(L"MODELEDITOR/GUI/MODEL_EDITOR/UNABLE_ADD_MODEL", modelToAdd_ ) );
		}
		modelToAdd_ = "";
	}

	CMainFrame * mainFrame = (CMainFrame *)(m_pMainWnd);
	HWND fgWin = GetForegroundWindow();

	bool isWindowActive =
		fgWin == mainFrame->GetSafeHwnd() || GetParent( fgWin ) == mainFrame->GetSafeHwnd();

	if (!CooperativeMoo::canUseMoo( isWindowActive ) || !isWindowActive)
	{
		// If activate failed, because the app is minimised, there's not enough
		// videomem to restore, or the app is in the background and other apps
		// we need to cooperate with are running, we just try again later.
		mfApp_->calculateFrameTime();
	} 
	else
	{
		/*
		// measure the update time
		uint64 beforeTime = timestamp();
		//*/

		if ( MeApp::instance().mutant()->texMemUpdate() )
		{
			CMainFrame::instance().setStatusText( ID_INDICATOR_TETXURE_MEM, 
				Localise(L"MODELEDITOR/GUI/MODEL_EDITOR/TEXTURE_MEM", Utilities::memorySizeToStr( MeApp::instance().mutant()->texMem() )) );
		}
	
		mfApp_->updateFrame();

		MaterialPreview::instance().update();

		// update any gui stuff
		CModelEditorApp::s_updateWatch.start();
		CMainFrame::instance().updateGUI();
		CModelEditorApp::s_updateWatch.stop();
	
		/*
		uint64 afterTime = timestamp();
		float lastUpdateMilliseconds = (float) (((int64)(afterTime - beforeTime)) / stampsPerSecondD()) * 1000.f;

		float desiredFrameRate_ = 60.f;
		const float desiredFrameTime = 1000.f / desiredFrameRate_;	// milliseconds

		if (desiredFrameTime > lastUpdateMilliseconds)
		{
			float compensation = desiredFrameTime - lastUpdateMilliseconds;
			Sleep((int)compensation);
		}
		//*/
	}

	return TRUE;
}

int CModelEditorApp::InternalExitInstance()
{
	BW_GUARD;

	if ( mfApp_ )
	{
		BgTaskManager::instance().stopAll();

		GizmoManager::instance().removeAllGizmo();
		while ( ToolManager::instance().tool() )
		{
			ToolManager::instance().popTool();
		}

		delete pPythonAdapter_;
		pPythonAdapter_ = NULL;

		mfApp_->fini();
		delete mfApp_;
		mfApp_ = NULL;

		meShell_->fini();
		delete meShell_;
		meShell_ = NULL;

		MsgHandler::fini();
		
		PanelManager::fini();

		GUI::Manager::fini();

		DogWatchManager::fini();

		WindowTextNotifier::fini();
		Options::fini();
	}

	initDone_ = false;
	
	delete [] s_pCmdLine;

	return CWinApp::ExitInstance();
}

int CModelEditorApp::InternalRun()
{
	return CWinApp::Run();
}

std::string CModelEditorApp::get( const std::string& key ) const
{
	BW_GUARD;

	return Options::getOptionString( key );
}

bool CModelEditorApp::exist( const std::string& key ) const
{
	BW_GUARD;

	return Options::optionExists( key );
}

void CModelEditorApp::set( const std::string& key, const std::string& value )
{
	BW_GUARD;

	Options::setOptionString( key, value );
}

bool CModelEditorApp::loadFile( UalItemInfo* ii )
{
	BW_GUARD;

	return pPythonAdapter_->callString( "openFile", 
		BWResource::dissolveFilename( bw_wtoutf8( ii->longText() ) ) );
}

void CModelEditorApp::loadModel( const char* modelName )
{
	modelToLoad_ = modelName;
}

/*~ function ModelEditor.loadModel
 *	@components{ modeleditor }
 *
 *	Loads the specified model into the editor.
 *
 *	@param modelName The name of the model to load.
 */
static PyObject * py_loadModel( PyObject * args )
{
	BW_GUARD;

	char* modelName;

	if (!PyArg_ParseTuple( args, "s", &modelName ))
	{
		PyErr_SetString( PyExc_TypeError, "py_openModel: Expected a string argument." );
		return NULL;
	}
	
	//Either Ctrl or Alt will result in the model being added
	if ((GetAsyncKeyState(VK_LCONTROL) < 0) ||
		(GetAsyncKeyState(VK_RCONTROL) < 0) ||
		(GetAsyncKeyState(VK_LMENU) < 0) ||
		(GetAsyncKeyState(VK_RMENU) < 0))
	{
		CModelEditorApp::instance().addModel( modelName );
	}
	else if (MeApp::instance().canExit( false ))
	{
		CModelEditorApp::instance().loadModel( modelName );
	}

	Py_Return;
}
PY_MODULE_FUNCTION( loadModel, ModelEditor )

void CModelEditorApp::addModel( const char* modelName )
{
	BW_GUARD;

	//If there is no model loaded then load this one...
	if (MeApp::instance().mutant()->modelName() == "")
	{
		loadModel( modelName );
	}
	else if (!MeApp::instance().mutant()->nodefull())
	{
		ERROR_MSG( "Models can only be added to nodefull models.\n\n"
			"  \"%s\"\n\n"
			"is not a nodefull model.\n",
			MeApp::instance().mutant()->modelName().c_str() );
	}
	else if (!MeApp::instance().mutant()->nodefull( modelName ))
	{
		ERROR_MSG( "Only nodefull models can be added to other models.\n\n"
			"  \"%s\"\n\n"
			"is not a nodefull model.\n",
			modelName );
	}
	else if ( !MeApp::instance().mutant()->canAddModel( modelName ))
	{
		ERROR_MSG( "The model cannot be added since it shares no nodes in common with the loaded model.\n" );
	}
	else
	{
		modelToAdd_ = modelName;
	}
}
/*~ function ModelEditor.addModel
 *	@components{ modeleditor }
 *
 *	Adds the specified model to the currently loaded model. 
 *	Only Nodefull models can be added to other Nodefull models.
 *
 *	@param modelName The name of the model to be added.
 */
static PyObject * py_addModel( PyObject * args )
{
	BW_GUARD;

	char* modelName;

	if (!PyArg_ParseTuple( args, "s", &modelName ))
	{
		PyErr_SetString( PyExc_TypeError, "py_addModel: Expected a string argument." );
		return NULL;
	}

	if ( BWResource::openSection( modelName ) == NULL )
	{
		PyErr_SetString( PyExc_IOError, "py_addModel: The model was not found." );
		return NULL;
	}

	CModelEditorApp::instance().addModel( modelName );

	Py_Return;
}
PY_MODULE_FUNCTION( addModel, ModelEditor )

/*~ function ModelEditor.removeAddedModels
 *	@components{ modeleditor }
 *
 *	This function removes any added models from the loaded model. 
 */
static PyObject * py_removeAddedModels( PyObject * args )
{
	BW_GUARD;

	if (MeApp::instance().mutant())
		MeApp::instance().mutant()->removeAddedModels();

	Py_Return;
}
PY_MODULE_FUNCTION( removeAddedModels, ModelEditor )

/*~ function ModelEditor.hasAddedModels
 *	@components{ modeleditor }
 *	
 *	This function checks whether the loaded model currently has any added models.
 *
 *	@return Returns True (1) if there are any added models, False (0) otherwise.
 */
static PyObject * py_hasAddedModels( PyObject * args )
{
	BW_GUARD;

	if (MeApp::instance().mutant())
		return PyInt_FromLong( MeApp::instance().mutant()->hasAddedModels() );

	return PyInt_FromLong( 0 );
}
PY_MODULE_FUNCTION( hasAddedModels, ModelEditor )

void CModelEditorApp::loadLights( const char* lightsName )
{
	BW_GUARD;

	PageLights* lightPage = (PageLights*)PanelManager::instance().panels().getContent( PageLights::contentID );
	lightPage->openLightFile( bw_utf8tow( lightsName ) );
	updateRecentList( "lights" );
}
/*~ function ModelEditor.loadLights
 *	@components{ modeleditor }
 *
 *	This function loads the specified light setup. 
 *
 *	@param lightsName The name of the light to be loaded.
 */
static PyObject * py_loadLights( PyObject * args )
{
	BW_GUARD;

	char* lightsName;

	if (!PyArg_ParseTuple( args, "s", &lightsName ))
	{
		PyErr_SetString( PyExc_TypeError, "py_openModel: Expected a string argument." );
		return NULL;
	}
	
	CModelEditorApp::instance().loadLights( lightsName );

	Py_Return;
}
PY_MODULE_FUNCTION( loadLights, ModelEditor )

// File Menu Open
void CModelEditorApp::OnFileOpen()
{
	BW_GUARD;

	static wchar_t BASED_CODE szFilter[] =	L"Model (*.model)|*.model||";
	BWFileDialog fileDlg (TRUE, L"", L"", OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilter);

	std::string modelsDir;
	MRU::instance().getDir( "models" ,modelsDir );
	std::wstring wmodelsDir = bw_utf8tow( modelsDir );
	fileDlg.m_ofn.lpstrInitialDir = wmodelsDir.c_str();

	if ( fileDlg.DoModal() == IDOK )
	{
		modelToLoad_ = BWResource::dissolveFilename( bw_wtoutf8( fileDlg.GetPathName().GetString() ));
		if (!BWResource::validPath( modelToLoad_ ))
		{
			::MessageBox( AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
				Localise(L"MODELEDITOR/GUI/MODEL_EDITOR/BAD_DIR_MODEL_LOAD"),
				Localise(L"MODELEDITOR/GUI/MODEL_EDITOR/UNABLE_RESOLVE_MODEL"),
				MB_OK | MB_ICONWARNING );
			modelToLoad_ = "";
		}
	}
}
/*~ function ModelEditor.openFile
 *	@components{ modeleditor }
 * 
 *	This function enables the Open File dialog, which allows a model to be loaded.
 */
static PyObject * py_openFile( PyObject * args )
{
	BW_GUARD;

	if (MeApp::instance().canExit( false ))
		CModelEditorApp::instance().OnFileOpen();

	Py_Return;
}
PY_MODULE_FUNCTION( openFile, ModelEditor )

// File Menu Add
void CModelEditorApp::OnFileAdd()
{
	BW_GUARD;

	static wchar_t BASED_CODE szFilter[] = L"Model (*.model)|*.model||";
	BWFileDialog fileDlg (TRUE, L"", L"", OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilter);

	std::string modelsDir;
	MRU::instance().getDir( "models" ,modelsDir );
	std::wstring wmodelsDir = bw_utf8tow( modelsDir );
	fileDlg.m_ofn.lpstrInitialDir = wmodelsDir.c_str();

	if ( fileDlg.DoModal() == IDOK )
	{
		std::string modelPath = BWResource::dissolveFilename( bw_wtoutf8( fileDlg.GetPathName().GetString() ));

		if (BWResource::validPath( modelPath ))
		{
			addModel( modelPath.c_str() );
		}
		else
		{
			::MessageBox( AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
				Localise(L"MODELEDITOR/GUI/MODEL_EDITOR/BAD_DIR_MODEL_ADD"),
				Localise(L"MODELEDITOR/GUI/MODEL_EDITOR/UNABLE_RESOLVE_MODEL"),
				MB_OK | MB_ICONWARNING );
		}
		
	}
}
/*~ function ModelEditor.addFile
 *	@components{ modeleditor }
 *
 *	This function enables the Open File dialog, which allows a model to be 
 *	added to the currently loaded model.
 */
static PyObject * py_addFile( PyObject * args )
{
	BW_GUARD;

	CModelEditorApp::instance().OnFileAdd();
	Py_Return;
}
PY_MODULE_FUNCTION( addFile, ModelEditor )

void CModelEditorApp::OnFileReloadTextures()
{
	BW_GUARD;

	CWaitCursor wait;
	
	Moo::ManagedTexture::accErrs( true );
	
	Moo::TextureManager::instance()->reloadAllTextures();

	std::string errStr = Moo::ManagedTexture::accErrStr();
	if (errStr != "")
	{
		ERROR_MSG( "Moo:ManagedTexture::load, unable to load the following textures:\n"
				"%s\n\nPlease ensure these textures exist.\n", errStr.c_str() );
	}

	Moo::ManagedTexture::accErrs( false );

}
/*~	function ModelEditor.reloadTextures
 *
 *	This function forces all textures to be reloaded.
 */
static PyObject * py_reloadTextures( PyObject * args )
{
	BW_GUARD;

	CModelEditorApp::instance().OnFileReloadTextures();
	Py_Return;
}
PY_MODULE_FUNCTION( reloadTextures, ModelEditor )

void CModelEditorApp::OnFileRegenBoundingBox()
{
	BW_GUARD;

	CWaitCursor wait;

	int animCount = MeApp::instance().mutant()->animCount();
	CLoadingDialog* load = NULL;
	if (animCount > 4)
	{
		load = new CLoadingDialog( Localise(L"MODELEDITOR/GUI/MODEL_EDITOR/REGENERATING_VIS_BOX") );
		load->setRange( animCount );
	}
		
	MeApp::instance().mutant()->recreateModelVisibilityBox( new AnimLoadFunctor< CLoadingDialog >(load, &CLoadingDialog::step), true );

	ME_INFO_MSGW( Localise(L"MODELEDITOR/GUI/MODEL_EDITOR/REGENERATED_VIS_BOX") );

	MeApp::instance().camera()->boundingBox(
		MeApp::instance().mutant()->zoomBoundingBox() );

	if (load) delete load;
}
/*~ function ModelEditor.regenBoundingBox
 *	@components{ modeleditor }
 * 
 *	This function forces the model's Visibility Bounding Box to be recalculated.
 */
static PyObject * py_regenBoundingBox( PyObject * args )
{
	BW_GUARD;

	CModelEditorApp::instance().OnFileRegenBoundingBox();
	Py_Return;
}
PY_MODULE_FUNCTION( regenBoundingBox, ModelEditor )

//The preferences dialog
void CModelEditorApp::OnAppPrefs()
{
	BW_GUARD;

	CPrefsDlg prefsDlg;
	prefsDlg.DoModal();
}
/*~ function ModelEditor.appPrefs
 *	@components{ modeleditor }
 *
 *	This function opens the ModelEditor's Preferences dialog.
 */
static PyObject * py_appPrefs( PyObject * args )
{
	BW_GUARD;

	CModelEditorApp::instance().OnAppPrefs();
	Py_Return;
}
PY_MODULE_FUNCTION( appPrefs, ModelEditor )
