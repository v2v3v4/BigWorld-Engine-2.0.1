/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "mozilla_web_page.hpp"
#include "mozilla_web_page_interface.hpp"
#include "cstdmf/bgtask_manager.hpp"
#include "ashes/simple_gui.hpp"
#include "moo/render_context.hpp"
#include "third_party/LLMozlib2/llmozlib_virtual_wrapper.h"
#include <mmsystem.h>
#include "math/math_extra.hpp"
#include "cstdmf/string_utils.hpp"
#include "resmgr\bwresource.hpp"
//define the following to disable LLMOZLIB
//#define DISABLE_LLMOZLIB

#ifndef _DLL
#define DISABLE_LLMOZLIB
#endif

// TODO: get mozilla working in PyModule_Hybrid mode.
#if BWCLIENT_AS_PYTHON_MODULE && !defined(DISABLE_LLMOZLIB)
#define DISABLE_LLMOZLIB
#endif

uint32 MozillaWebPageManager::s_textureMemoryLimitLow = 0;
uint32 MozillaWebPageManager::s_textureMemoryLimitMedium = 0;
uint32 MozillaWebPageManager::s_textureMemoryLimitHigh = 0;
uint32 MozillaWebPageManager::s_cleanMemoryConstant = 5;
const char* MozillaWebPageManager::ON_NAVIGATION_BEGIN = "onNavigateBegin";
const char* MozillaWebPageManager::ON_NAVIGATION_COMPLETE = "onNavigateComplete";
const char* MozillaWebPageManager::ON_STATUS_TEXT_CHANGED = "onStatusTextChange";

D3DFORMAT MozillaWebPageManager::s_format = D3DFMT_A8R8G8B8;
volatile bool MozillaWebPageManager::s_isValid = true;
const uint MozillaWebPageManager::s_defaultSequenceNum = 1;

//running states to mark our thread status
static const int RUNNING_RUNNINGSTATE = 1;
static const int SHOULD_FINISH_RUNNINGSTATE = 2;
static const int FINISHED_RUNNINGSTATE = 3;

static const wchar_t MOZILLA_DLL[] = L"llmozlib_dll";
#if _MSC_VER >= 1500
#ifdef _DEBUG
static const wchar_t MOZILLA_DIRECTORY[]=L"mozilla/debug_vs2008";
#else
static const wchar_t MOZILLA_DIRECTORY[]=L"mozilla/release_vs2008";
#endif
#else
#ifdef _DEBUG
static const wchar_t MOZILLA_DIRECTORY[]=L"mozilla/debug";
#else
static const wchar_t MOZILLA_DIRECTORY[]=L"mozilla/release";
#endif 
#endif
static const wchar_t MPL_DOCUMENTATION[] = L"mozilla/patch/readme.txt";

DECLARE_DEBUG_COMPONENT2( "Web ", 0 )

//Using an extra window causes focus issues when using non full screen mode (we lose focus)
//define the following to create an additional window for the web integration.
//#define USE_EXTRA_WINDOW
//class name for the extra window
const LPTSTR MOZILLA_WINDOW_CLASS_NAME = L"TempMozilla";
typedef LLMozlibVirtualWrapper* (* GET_INSTANCE)();
typedef void (* DELETE_INSTANCE)();
static DELETE_INSTANCE deleteInstanceFunc = NULL;
LLMozlibVirtualWrapper* llmozlibInstance;
//Whether mozilla was initialised
bool mozillaInit = false;

#define CHECK_LLMOZLIB_CALL(call) if ( !(call)) { ERROR_MSG ("MozillaWebPage %s failed %d\n", #call, llmozlibInstance->getLastError());}
#define CHECK_LLMOZLIB_CALL_RET(call) do {								\
	bool ret = (call);												\
	if ( !(ret))													\
{																\
	ERROR_MSG ("MozillaWebPage %s failed %d\n", #call, llmozlibInstance->getLastError()); \
	return ret;													\
}																\
}																	\
	while (false) 

static DataSectionPtr storedConfigSection;

//static variables
static std::wstring EMPTY_PAGE = L"about:blank";

bool isValidMozilla(const char* file, int line)
{
	BW_GUARD;
	if (! MozillaWebPageManager::s_isValid)
	{
		return false;
	}
	if (!mozillaInit || llmozlibInstance == NULL) 
	{
		ERROR_MSG("MozillaWebPage::isValidMozilla failed due to bad mozilla init file %s line %d\n",
			file, line);
		return false;
	}
	return true;
}

//Helper method to format a system message
std::wstring get_formatted_message(DWORD dwMessageId)
{
	wchar_t buffer[2048];
	if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwMessageId, 0, buffer, ARRAY_SIZE(buffer), NULL) == 0) 
	{
		//format message failed, format the value as string.
		bw_snwprintf (buffer, ARRAY_SIZE(buffer), L"Error value: %d", dwMessageId);
	}
	return buffer;
}

bool isKeyEvent( UINT msg )
{
	return	msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN || 
			msg == WM_KEYUP || msg == WM_SYSKEYUP;
}

/**
* This class is used when using the init of the mozilla application
* by setting the current directory. and setting it back once done.
*/
class WorkingDirectorySetter
{
public: 
	WorkingDirectorySetter(const std::wstring& directory)
	{
		if (::GetCurrentDirectory(sizeof(currentDirectory_)/sizeof(currentDirectory_[0]), currentDirectory_) == 0)
		{
			ERROR_MSG("MozillaWebPage Failed to get the current directory at %d\n", __LINE__);
		}
		if (::SetCurrentDirectory(directory.c_str()) == 0) 
		{
			ERROR_MSG("MozillaWebPage Failed to set the current directory\n");
		}
		wchar_t dirAfterChange[2048];
		wchar_t* path;
		if (::GetCurrentDirectory(sizeof(dirAfterChange)/sizeof(dirAfterChange[0]), dirAfterChange) == 0)
		{
			ERROR_MSG("MozillaWebPage Failed to get the current directory at %d\n", __LINE__);
		}
		//also add this to the path 
		DWORD pathSize = ::GetEnvironmentVariable(L"PATH", NULL, 0);
		//just in case someone is editing the path
		pathSize += 200;
		path = new wchar_t[pathSize];
		DWORD ret = ::GetEnvironmentVariable(L"PATH", path, pathSize);
		if (ret > pathSize || ret == 0) 
		{
			ERROR_MSG("MozillaWebPage Failed to get the current directory at %d\n", __LINE__);
		}
		else
		{
			std::wstring temp = std::wstring(path) + std::wstring(L";") + std::wstring(dirAfterChange);
			if (::SetEnvironmentVariable(L"PATH", temp.c_str()) == 0)
			{
				ERROR_MSG("MozillaWebPage Failed to set the current directory\n");
			}
		}
		delete[] path;
	}

	~WorkingDirectorySetter()
	{
		if (::SetCurrentDirectory(currentDirectory_) == 0) 
		{
			ERROR_MSG("MozillaWebPage Failed to set the current directory back\n");
		}
	}
	wchar_t currentDirectory_[2048];
};

//The main application window (given as a parameter to mozilla)
HWND MozillaWebPageManager::s_applicationWindow = 0;
HWND MozillaWebPageManager::s_usedWindow = 0;
BW_SINGLETON_STORAGE( MozillaWebPageManager )

#ifdef USE_EXTRA_WINDOW
/**
*  Create the additional window used for llmozlib init.
*/
HWND MozillaWebPageManager::createExtraWindow()
{
	BW_GUARD;
	WNDCLASS wndClass = { sizeof( WNDCLASS ) };
	wndClass.hInstance = ::GetModuleHandle( NULL );
	wndClass.lpfnWndProc        = DefWindowProc;
	wndClass.lpszClassName      = MOZILLA_WINDOW_CLASS_NAME;
	wndClass.style              = CS_HREDRAW | CS_VREDRAW;
	HWND mozillaWindow = 0;


	if (!::RegisterClass(&wndClass) )
	{
		//might fail if unregister didn't happen.
		WARNING_MSG("MozillaWebPage Failed to register the window class %S\n", get_formatted_message(::GetLastError()).c_str());
	}
	mozillaWindow = ::CreateWindow(MOZILLA_WINDOW_CLASS_NAME,     /* class name */
		L"Mozilla",      /* title to window */
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_DISABLED, /* style */ //WS_DISABLED | 
		CW_USEDEFAULT,   /* start pos x */
		CW_USEDEFAULT,   /* start pos y */
		/*Doesn't change based on the documentations*/800,        /* width */
		/*Doesn't change based on the documentations*/600,       /* height */
		s_applicationWindow ,            /* parent HWND */
		NULL,            /* menu HANDLE */
		::GetModuleHandle( NULL ),       /* */
		NULL);           /* creatstruct param */
	if ( !mozillaWindow )
	{
		ERROR_MSG("MozillaWebPage Failed to create the mozilla window error %S\n", get_formatted_message(::GetLastError()).c_str());
		return NULL;
	}
	UpdateWindow(mozillaWindow);
	return mozillaWindow;
}


void MozillaWebPageManager::deleteExtraWindow()
{
	if (!UnregisterClass(      
		MOZILLA_WINDOW_CLASS_NAME,
		::GetModuleHandle( NULL )))
	{
		ERROR_MSG("MozillaWebPage UnregisterClass failed\n");
	}
}
#endif

/**
*	Constructor for the MozillaWebPageManager object
*/
MozillaWebPageManager::MozillaWebPageManager() :
runningState_(FINISHED_RUNNINGSTATE),
key_(0),
timeSinceLastMemClean_(0),
drawWeb_(true),
bruteExit_(false)
{
	MF_WATCH( "Client Settings/Web/Draw Web",
			drawWeb_,
			Watcher::WT_READ_WRITE,
			"Toggle use of the web pages" );
}

MozillaWebPageManager::~MozillaWebPageManager()
{
	{
		SimpleMutexHolder smh(managerMutex_);
		//mark as as invalid - don't allow more requests to the manager
		MozillaWebPageManager::s_isValid = false;
		//clear all maps here before starting to destruct everything as objects might call back to the manager.
		mapIntMozillaWebPage_.clear();
		mapIntMozillaWebPageInterface_.clear();
	}
}

void MozillaWebPageManager::s_chooseFormat()
{
	BW_GUARD;
	typedef std::list<D3DFORMAT> FormatList;
	FormatList formats;
	//we lose some quality by trying to use 16 bit format rather than 32 bit one
	//on low graphics options
	formats.push_back(D3DFMT_A8R8G8B8);
	formats.push_back(D3DFMT_R5G6B5);
	bool foundFormat = false;
	FormatList::iterator iter = formats.begin();
	while (! foundFormat && iter != formats.end())
	{
		//choose the correct format
		if (Moo::rc().supportsTextureFormat( *iter ))
		{
			INFO_MSG("MozillaWebPage found format %d\n", *iter);
			foundFormat = true;
			s_format = *iter;
		}
		iter++;
	}
	if (! foundFormat)
	{
		ERROR_MSG("Could not find the correct format for the texture\n");
		//just fail to the folqlowing format:
		s_format = D3DFMT_A8R8G8B8;
	}
	if (s_format != D3DFMT_A8R8G8B8)
	{
		ERROR_MSG("MozillaWebPage D3DFMT_A8R8G8B8 not working\n");
	}
}

/**
*	Run the web thread
*	@param configSection the engine_config section
*/
void MozillaWebPageManager::s_run(const DataSectionPtr& configSection)
{
	if (!MozillaWebPageManager::pInstance())
	{
		new MozillaWebPageManager();
	}
	storedConfigSection = configSection;
	//start the thread
	MozillaWebPageManager::instance().SimpleThread::init( s_mainLoop, NULL);
	Profiler::instance().addThread( MozillaWebPageManager::instance().handle(), "Web" );
}

/**
*	Process the next command
*	@param the command to be processed
*/
void MozillaWebPageManager::processCommand(MozillaWebPageCommandPtr& command)
{
	if (! isValidMozilla(__FILE__,__LINE__))
	{
		return;
	}
	__try {
		processCommandInternal(command);
	}
	__except(EXCEPTION_EXECUTE_HANDLER) \
	{ 
		ERROR_MSG("MozillaWebPage crashed\n"); 
		MozillaWebPageManager::bruteFini(); 
	} 
}

/**
*	Process the next command
*	@param the command to be processed
*/
void MozillaWebPageManager::processCommandInternal(MozillaWebPageCommandPtr& command)
{
	MapIntMozillaWebPage::iterator iter;
	if (command->commandType_ != COMMAND_MOZILLAWEBPAGE_COMMAND_TYPE)
	{
		SimpleMutexHolder smh(managerMutex_);
		iter = mapIntMozillaWebPage_.find(command->key_);
		if (iter == mapIntMozillaWebPage_.end())
		{
			ERROR_MSG("MozillaWebPageManager processCommandInternal Key doesn't exist %d\n", command->key_);
			return;
		}
	}
	switch (command->commandType_)
	{
	case(COMMAND_MOZILLAWEBPAGE_COMMAND_TYPE) :
		{

			{
				SimpleMutexHolder smh(managerMutex_);
				if (mapIntMozillaWebPage_.find(command->key_) != mapIntMozillaWebPage_.end())
				{
					ERROR_MSG("MozillaWebPageManager Key already exist %d\n", command->key_);
				}
			}
			//shouldn't be protected by mutex as might send callback 
			MozillaWebPage* temp = new MozillaWebPage(((CommandMozillaWebPage*)(command.get()))->width, 
				((CommandMozillaWebPage*)(command.get()))->height, 
				((CommandMozillaWebPage*)(command.get()))->mipmap, 
				((CommandMozillaWebPage*)(command.get()))->url, 
				((CommandMozillaWebPage*)(command.get()))->graphicsSettingsBehaviour, 
				command->key_);
			{
				SimpleMutexHolder smh(managerMutex_);
				mapIntMozillaWebPage_[command->key_] = temp;
			}
			break;
		}
	case(COMMAND_MOZILLAWEBPAGE_DTOR_COMMAND_TYPE) :
		{
			SimpleMutexHolder smh(managerMutex_);
			mapIntMozillaWebPage_.erase(iter);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_ADD_INTERFACE_COMMAND_TYPE):
		{
			addInterface(command->key_, 
				((CommandMozillaAddInterface*)(command.get()))->mozillaInterface);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_NAVIGATE_COMMAND_TYPE):
		{
			iter->second->navigate(((CommandMozillaWebPageNavigate*)(command.get()))->url);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_UPDATE_BROWSER_COMMAND_TYPE):
		{
			iter->second->updateBrowser(((CommandMozillaWebPageUpdateBrowser*)(command.get()))->sequenceNum);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_HANDLE_MOUSE_BUTTON_EVENT_COMMAND_TYPE):
		{
			iter->second->handleMouseButtonEvent(((CommandMozillaWebPageHandleMouseMove*)(command.get()))->pos,
				((CommandMozillaWebPageHandleMouseMove*)(command.get()))->down);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_SCROLL_BY_LINES_COMMAND_TYPE):
		{
			iter->second->scrollByLines(((CommandMozillaWebPageScrollByLines*)(command.get()))->lines);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_HANDLE_MOUSE_MOVE_COMMAND_TYPE):
		{
			iter->second->handleMouseMove(((CommandMozillaWebPageHandleMouseMove*)(command.get()))->pos);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_HANDLE_KEYBOARD_EVENT_COMMAND_TYPE):
		{
			iter->second->handleKeyboardEvent(((CommandMozillaWebPageHandleKeyboardEvent*)(command.get()))->keyCodeIn);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_HANDLE_UNICODE_INPUT_COMMAND_TYPE):
		{
			iter->second->handleUnicodeInput(((CommandMozillaWebPageHandleUnicodeInput*)(command.get()))->keyCodeIn, ((CommandMozillaWebPageHandleUnicodeInput*)(command.get()))->codePage);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_ENABLE_PROXY_COMMAND_TYPE):
		{
			iter->second->enableProxy(((CommandMozillaWebPageEnableProxy*)(command.get()))->proxyEnabledIn,
				((CommandMozillaWebPageEnableProxy*)(command.get()))->proxyHostNameIn,
				((CommandMozillaWebPageEnableProxy*)(command.get()))->proxyPortIn
				);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_ENABLE_COOKIES_COMMAND_TYPE):
		{
			iter->second->enableCookies(((CommandMozillaWebPageEnableCookies*)(command.get()))->value);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_NAVIGATE_BACK_COMMAND_TYPE):
		{
			iter->second->navigateBack();
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_NAVIGATE_FORWARD_COMMAND_TYPE):
		{
			iter->second->navigateForward();
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_NAVIGATE_RELOAD_COMMAND_TYPE):
		{
			iter->second->navigateReload();
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_SET_SIZE_COMMAND_TYPE):
		{
			iter->second->setSize(((CommandMozillaWebPageSetSize*)(command.get()))->width, 
				((CommandMozillaWebPageSetSize*)(command.get()))->height,
				((CommandMozillaWebPageSetSize*)(command.get()))->textureWidth,
				((CommandMozillaWebPageSetSize*)(command.get()))->textureHeight,
				((CommandMozillaWebPageSetSize*)(command.get()))->exactWidth,
				((CommandMozillaWebPageSetSize*)(command.get()))->exactHeight);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_FOCUS_BROWSER_COMMAND_TYPE):
		{
			iter->second->focusBrowser(((CommandMozillaWebPageFocusBrowser*)(command.get()))->focus,
				((CommandMozillaWebPageFocusBrowser*)(command.get()))->enable);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_ALLOW_CURSOR_INTERACTION_COMMAND_TYPE):
		{
			iter->second->allowCursorInteraction (((CommandMozillaWebPageAllowCursorInteraction*)(command.get()))->allow);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_SET_404_REDIRECT_COMMAND_TYPE):
		{
			iter->second->set404Redirect(((CommandMozillaWebPageSet404Redirect*)(command.get()))->url);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_EVALUATE_JAVASCRIPT_COMMAND_TYPE):
		{
			iter->second->evaluateJavaScript(((CommandMozillaWebPageEvaluateJavascript*)(command.get()))->script);
			break;
		}
	case (COMMAND_MOZILLAWEBPAGE_CLEAN_MEMORY):
		{
			iter->second->cleanUnusedMemory();
			break;
		}
	default:
		{
			ERROR_MSG("Invalid command %d\n", command->commandType_);
		}
	}
}

void MozillaWebPageManager::s_mainLoop(void * arg)
{	
	if (! init(storedConfigSection))
	{
		ERROR_MSG("MozillaWebPageManager init failed, bailing out.\n");
		return;
	}
	storedConfigSection = NULL;
	MozillaWebPageManager::instance().mainLoopInternal();
}

/**
*	Main background thread loop
*	This loop checks for new commands and handles them
*   It also check for new windows messages for this thread and dispatches them.
*/
void MozillaWebPageManager::mainLoopInternal()
{
	INFO_MSG( "MozillaWebPageManager::mainLoopInternal running\n" );
	runningState_ = RUNNING_RUNNINGSTATE;
	while (runningState_ == RUNNING_RUNNINGSTATE && isValidMozilla(__FILE__,__LINE__))
	{
		MSG msg;
		bool processed = false;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			processed = true;
			// Check for a quit message
			if( msg.message == WM_QUIT ) 
			{
				break;
			}
			TranslateMessage( &msg );
			DispatchMessage( &msg );

			if (::GetFocus() != s_applicationWindow && isKeyEvent(msg.message))
			{
				PostMessage( s_applicationWindow, msg.message, msg.wParam, msg.lParam );
			}
		}
		else
		{
			if (commandFifo_.count() > 0) 
			{
				processed = true;
				MozillaWebPageCommandPtr command = commandFifo_.pop();
				processCommand(command);
			}

		}
		if (! processed)
		{	
			//TBD - future do a wait on the message and the fifo
			Sleep(5);
		}
	}
	runningState_ = FINISHED_RUNNINGSTATE;
}

/**
*	Push command to be processed by the background thread
*	@param The command to proces
*/
void MozillaWebPageManager::pushCommand(MozillaWebPageCommandPtr& command)
{	
	if (! isValidMozilla(__FILE__, __LINE__))
	{
		return;
	}
	if (! drawWeb_) 
	{
		return;
	}
	commandFifo_.push(command);
}

/**
*	Push command to be processed by the next tick as a callback 
*	@param The command to proces
*/
void MozillaWebPageManager::pushCallback(MozillaWebPageCommandPtr& command)
{
	callbackFifo_.push(command);
}


/**
*	Update a texture based on the data collected by the background thread.
*	@param The key of the MozillaWebPageInterface to use
*	@param The texture
*	whether this texture is mipmapped.
*/
void MozillaWebPageManager::updateTextureDirect(int key, ComObjectWrap<DX::Texture>& destTexture, uint textureWidth, uint textureHeight, bool mipmap, MD5::Digest& textureChkSum)
{
	if (! drawWeb_) 
	{
		return;
	}
	SimpleMutexHolder smh(managerMutex_);
	MapIntMozillaWebPage::iterator iter = mapIntMozillaWebPage_.find(key);
	if (iter == mapIntMozillaWebPage_.end() || iter->second->webPageWidth_ == 0 ||
		iter->second->webPageHeight_ == 0)
	{
		INFO_MSG("updateTextureDirect but the web page isn't initialised yet %d\n", key);
		return;
	}
	iter->second->updateTextureDirect(destTexture, textureWidth, textureHeight, mipmap, textureChkSum);
}

/**
*	Get the sequence number of the specific MozillaWebPage
*	important in order to know if the web page has done update or the update hasn't been done yet
*	@param The mozillaWebPage key
*/
uint MozillaWebPageManager::getSequenceNum(int key)
{
	SimpleMutexHolder smh(managerMutex_);
	if (! drawWeb_) 
	{
		return 0;
	}
	MapIntMozillaWebPage::iterator iter = mapIntMozillaWebPage_.find(key);
	if (iter == mapIntMozillaWebPage_.end())
	{
		//can happen during the creation.
		return 0;
	}
	return iter->second->getSequenceNum();
}

/**
*	register a new MozillaWebPageInterface as the owner of a specific key
*	@param The mozillaWebPage key
*	@param The MozillaWebPageInterface 
*/
void MozillaWebPageManager::addInterface(int key, MozillaWebPageInterfacePtr mozillaInterface)
{
	SimpleMutexHolder smh(managerMutex_);
	MapIntMozillaWebPageInterface::iterator iter = mapIntMozillaWebPageInterface_.find(key);
	if (iter != mapIntMozillaWebPageInterface_.end())
	{
		ERROR_MSG("MozillaWebPageManager addInterface Key already exist %d\n", key);
		return;
	}
	mapIntMozillaWebPageInterface_[key] = mozillaInterface;
}

/**
*	remove the owner MozillaWebPageInterface of a specific key
*	@param The mozillaWebPage key
*/
void MozillaWebPageManager::removeInterface(int key)
{
	SimpleMutexHolder smh(managerMutex_);
	MapIntMozillaWebPageInterface::iterator iter = mapIntMozillaWebPageInterface_.find(key);
	if (iter == mapIntMozillaWebPageInterface_.end())
	{
		ERROR_MSG("MozillaWebPageManager removeInterface Key doesn't exists %d\n", key);
		return;
	}
	mapIntMozillaWebPageInterface_.erase(iter);
}


/**
*	Get the next unique key
*	@return The next unique key
*/
uint MozillaWebPageManager::getNextKey()
{
	SimpleMutexHolder smh(managerMutex_);
	key_ = key_ + 1;
	return key_;
}

/**
*  Init the llmozlib interface (static method)
*/
bool MozillaWebPageManager::init(const DataSectionPtr& configSection)
{
	BW_GUARD;
	//Init the mozilla interface (done once)
	if (mozillaInit)
	{
		WARNING_MSG("MozillaWebPage Mozilla init called twice\n");
		return true;
	}

#ifdef DISABLE_LLMOZLIB
	return true;
#endif

	s_usedWindow = s_applicationWindow;
#ifdef USE_EXTRA_WINDOW
	s_usedWindow = createExtraWindow();
#endif
	if (s_usedWindow == 0)
	{
		ERROR_MSG("MozillaWebPage Please make sure you set the application window before using the mozilla interface\n");
		return false;
	}

	//Load the mozilla shared library
	HMODULE module = 0;
	//make sure the licensing required documentation is shipped correctly 
	//with the mozilla dlls by checking that the directory patch/readme.txt exists
	std::wstring appDirectory = bw_acptow(BWResource::appDirectory());
	std::wstring checkedFile = appDirectory + MPL_DOCUMENTATION;
	WIN32_FILE_ATTRIBUTE_DATA attrData;
	BOOL ok = ::GetFileAttributesEx(
		checkedFile.c_str(),
		GetFileExInfoStandard,
		&attrData );
	if (!ok || attrData.dwFileAttributes == DWORD(-1)) 
	{
		ERROR_MSG("MozillaWebPage Cannot load mozilla without the required documentation due to licensing issues. Please make sure the patch\\readme.txt file exists\n");
		return false;
	}
	//switch directory to make sure the load library works
	WorkingDirectorySetter wds(appDirectory + MOZILLA_DIRECTORY);
	module = LoadLibrary(MOZILLA_DLL);
	if (!module) 
	{
		ERROR_MSG("MozillaWebPage Failed loading llmozlib\n");
		return false;
	}

	GET_INSTANCE getInstanceFunc = (GET_INSTANCE)::GetProcAddress(module,"getInstance");
	deleteInstanceFunc = (DELETE_INSTANCE)::GetProcAddress(module,"deleteInstance");
	if (!getInstanceFunc || !deleteInstanceFunc) 
	{
		ERROR_MSG("MozillaWebPage Failed getting proc\n");
		return false;
	}
	llmozlibInstance = getInstanceFunc();
	INFO_MSG("MozillaWebPage LLMozLib version %s\n", llmozlibInstance->getVersion().c_str());

	char currentDirectory[2048];
	//using the ascii version as currently llmozlib uses ascii initialisation.
	if (::GetCurrentDirectoryA(sizeof(currentDirectory)/sizeof(currentDirectory[0]), currentDirectory) == 0)
	{
		ERROR_MSG("MozillaWebPage Failed to get the current directory\n");
	}
	std::string applicationDir = currentDirectory;
	std::string componentDir = applicationDir;
	//TBD - check if works with read only file system
	std::string profileDir = applicationDir + "\\" + "profiling";
	bool profile_dir_exists = false;
	if (access( profileDir.c_str(), 0 ) == 0 ) 
	{
		struct stat status;
		stat( profileDir.c_str(), &status );
		if ( status.st_mode & S_IFDIR )
		{
			profile_dir_exists = true;
		}
	}
	if (!profile_dir_exists) 
	{
		if (!CreateDirectoryA(profileDir.c_str(), NULL) )
		{
			ERROR_MSG("MozillaWebPage Failed to create directory\n");
		}
	}

	//init the mozilla api	
	CHECK_LLMOZLIB_CALL_RET(llmozlibInstance->init( applicationDir, componentDir, profileDir, s_usedWindow));

	// enable cookies
	CHECK_LLMOZLIB_CALL_RET(llmozlibInstance->enableCookies( true ));

	// turn proxy off
	CHECK_LLMOZLIB_CALL_RET(llmozlibInstance->enableProxy( false, "", 0 ));

	// turn plugin on
	CHECK_LLMOZLIB_CALL_RET(llmozlibInstance->enablePlugins(true));

	// append details to agent string
	llmozlibInstance->setBrowserAgentId("Firefox/2.0.1");
	s_chooseFormat();

	//read the engine config for the sizes of textures for different resolutions
	s_textureMemoryLimitLow = configSection->readInt("Web/textureMemoryLimitLow", (uint32)(0.5 * 1024 * 1024));
	s_textureMemoryLimitMedium = configSection->readInt("Web/textureMemoryLimitMedium", (uint32)(1 * 1024 * 1024));
	s_textureMemoryLimitHigh = configSection->readInt("Web/textureMemoryLimitHigh", (uint32)(4 * 1024 * 1024));
	s_cleanMemoryConstant = configSection->readInt("Web/cleanMemoryTime", 5);

	//set mozilla as initialised
	mozillaInit = true;
	return true;
}

/**
* Update the interfaces texture based on the chooosen graphics settings
*/
void MozillaWebPageManager::updateTexturesBasedOnGraphicSettings()
{
	SimpleMutexHolder smh(managerMutex_);
	//recreate interfaces textures
	MapIntMozillaWebPageInterface::iterator iter = mapIntMozillaWebPageInterface_.begin();
	for (; iter != mapIntMozillaWebPageInterface_.end() ; iter++)
	{
		MozillaWebPageInterfacePtr temp = iter->second;
		//give the mutex as the following methods might call the MozillaWebPageManager
		managerMutex_.give();
		temp->deleteUnmanagedObjects();
		temp->createUnmanagedObjects();
		managerMutex_.grab();
	}
}

/**
*  release the llmozlib interface (static method)
*/
bool MozillaWebPageManager::s_fini(bool keepInstance)
{
	//make sure the internal thread finishes
	if (MozillaWebPageManager::pInstance() == NULL)
	{
		//deal with calling twice.
		return true;
	}
	MozillaWebPageManager::instance().fini(); 
	__try 
	{
		if (! isValidMozilla(__FILE__,__LINE__))
		{
			//already done or not needed
			return false;
		}
		mozillaInit = false;
#ifdef TBD 
		//reset for llmozlib still doesn't work.
		CHECK_LLMOZLIB_CALL_RET(llmozlibInstance->reset());
#endif
#ifdef USE_EXTRA_WINDOW
		deleteExtraWindow();
#endif
	}
	__except (EXCEPTION_EXECUTE_HANDLER) \
	{ 
		ERROR_MSG("MozillaWebPage crashed on release\n");
	}
	//mark mozilla as not initialised to prevent anyone touching it as the window for it was already
	//destroyed.
	mozillaInit = false;
	deleteInstanceFunc();
	llmozlibInstance = NULL;
	//mark us as invalid - don't allow more requests to the manager
	MozillaWebPageManager::s_isValid = false;

	//We keep the instance if fini called early to allow for the script to 
	//deallocate using the instance. as the thread is stopped, no calls will get to mozilla.
	if (MozillaWebPageManager::pInstance() && ! MozillaWebPageManager::pInstance()->bruteExit_ && ! keepInstance)
	{
		delete MozillaWebPageManager::pInstance();
		MozillaWebPageManager::s_pInstance = NULL;
	}
	return true;
}


/**
*  End the MozillaWebPageManager thread.
*/
bool MozillaWebPageManager::fini()
{
	if (MozillaWebPageManager::instance().runningState_ == RUNNING_RUNNINGSTATE)
	{
		MozillaWebPageManager::instance().runningState_ = SHOULD_FINISH_RUNNINGSTATE;
		uint totalSlept = 0;
		while (MozillaWebPageManager::instance().runningState_ != FINISHED_RUNNINGSTATE &&
			totalSlept < 3000)
		{
			Sleep(50);
			totalSlept += 50;
		}
		if (MozillaWebPageManager::instance().runningState_ != FINISHED_RUNNINGSTATE)
		{
			ERROR_MSG("MozillaWebPageManager::instance() didn't exit. Current running state %d\n", 
				MozillaWebPageManager::instance().runningState_);
			//make exit fast so the client will not get stuck
			bruteExit_ = true;
		}
	}
	return true;
}


/**
*  search for callbacks to be done on the main thread
*  When a callback arrives we will need to call the MozillaWebPageInterface.
*/
bool MozillaWebPageManager::s_tick(float dTime)
{
	//must have a try except here as fini sometimes crashes
	MozillaWebPageManager::instance().tick(dTime); 
	return true;
}


/**
*  Process callbacks from mozilla and send them to the correct MozillaWebPageInterface.
*  Note that the managerMutex_ is assumed to be taken
*/
void MozillaWebPageManager::internalProcessCallbacks()
{
	while (callbackFifo_.count() > 0)
	{
		MozillaWebPageCommandPtr command = callbackFifo_.pop();
		CommandMozillaWebPageCallbackBase* inCommand = (CommandMozillaWebPageCallbackBase*)command.get();
		MapIntMozillaWebPageInterface::iterator iter = mapIntMozillaWebPageInterface_.find(inCommand->key_);
		if (iter == mapIntMozillaWebPageInterface_.end())
		{
			//can happen mainly during creation when the constructor calls a callback.
			continue;
		}
		switch (inCommand->commandType_)
		{

		case(COMMAND_MOZILLAWEBPAGE_ON_PAGE_CHANGED) :
			{
				managerMutex_.give();
				iter->second->callMethod ("onPageChanged", inCommand->event );
				managerMutex_.grab();
				break;
			}
		case(COMMAND_MOZILLAWEBPAGE_ON_NAVIGATION_BEGIN) :
			{
				managerMutex_.give();
				iter->second->callMethod (ON_NAVIGATION_BEGIN, inCommand->event  );
				managerMutex_.grab();
				break;
			}
		case(COMMAND_MOZILLAWEBPAGE_ON_NAVIGATION_COMPLETE) :
			{
				managerMutex_.give();
				iter->second->callMethod (ON_NAVIGATION_COMPLETE, inCommand->event  );
				iter->second->setUrl(bw_acptow (inCommand->event.getEventUri()));
				managerMutex_.grab();
				break;
			}
		case(COMMAND_MOZILLAWEBPAGE_ON_UPDATE_PROGRESS) :
			{
				managerMutex_.give();
				iter->second->callMethod ("onUpdateProgress", inCommand->event  );
				managerMutex_.grab();
				break;
			}
		case(COMMAND_MOZILLAWEBPAGE_ON_STATUS_TEXT_CHANGE) :
			{
				managerMutex_.give();
				iter->second->callMethod (ON_STATUS_TEXT_CHANGED, inCommand->event  );
				managerMutex_.grab();
				break;
			}
		case(COMMAND_MOZILLAWEBPAGE_ON_LOCATION_CHANGE) :
			{
				managerMutex_.give();
				iter->second->callMethod ("onLocationChange", inCommand->event  );
				iter->second->setUrl(bw_acptow (inCommand->event.getEventUri()));
				managerMutex_.grab();
				break;
			}
		case(COMMAND_MOZILLAWEBPAGE_ON_CLICK_LINK_HREF) :
			{
				managerMutex_.give();
				iter->second->callMethod ("onClickHref", inCommand->event  );
				managerMutex_.grab();
				break;
			}
		case(COMMAND_MOZILLAWEBPAGE_ON_CLICK_LINK_NO_FOLLOW) :
			{
				managerMutex_.give();
				iter->second->callMethod ("onClickLinkNoFollow", inCommand->event  );
				managerMutex_.grab();
				break;
			}
		default:
			{
				ERROR_MSG("Invalid command %d\n", inCommand->commandType_);
			}
		}
	}
}


/**
*  Clean all unused memory from the interfaces and the MozillaWebPages
*/
void MozillaWebPageManager::internalCleanUnusedMemory()
{
	MapIntMozillaWebPageInterface::iterator iterInterface = mapIntMozillaWebPageInterface_.begin();
	for (; iterInterface != mapIntMozillaWebPageInterface_.end() ; iterInterface++)
	{
		if (iterInterface->second->cleanUnusedMemory())
		{
			CommandMozillaWebPageCleanMemory* command = new CommandMozillaWebPageCleanMemory(iterInterface->first);
			pushCommand(MozillaWebPageCommandPtr(command));
		}
	}
}

/**
*  search for callbacks to be done on the main thread
*  When a callback arrives we will need to call the MozillaWebPageInterface.
*/
bool MozillaWebPageManager::tick(float dTime)
{
	SimpleMutexHolder smh(managerMutex_);
	static DogWatch dwTick("Mozilla_tick");
	ScopedDogWatch sdw( dwTick );
	internalProcessCallbacks();

	timeSinceLastMemClean_ += dTime;
	//clean memory every 10 secs
	if (timeSinceLastMemClean_ > s_cleanMemoryConstant)
	{	
		internalCleanUnusedMemory();
		timeSinceLastMemClean_ = 0;
	}

	return true;
}

/**
*  Stop using mozilla (no memory is released)
*/
bool MozillaWebPageManager::bruteFini()
{
	BW_GUARD;
	mozillaInit = false;
	return true;
}

/**
*	This method updates the application window. depending on the USE_EXTRA_WINDOW define
*   the application window might be used as input to llmozlib.
*/
void MozillaWebPageManager::setApplicationWindow(HWND newApplicationWindow)
{
	BW_GUARD;
	s_applicationWindow = newApplicationWindow;
}

/**
*
* This method is used to check wheter web is drawn or not
*
*/ 
bool MozillaWebPageManager::drawWeb() const
{
	return drawWeb_;
}


/*
MozillaWebPage methods
*/
/**
*	Constructor for the MozillaWebPage object
*	@param width the width of the web page texture
*	@param height the height of the web page texture
*	@param mipmap whether this page is using mipmapping
*	@param url the url to use for the web page texture
*/
MozillaWebPageManager::MozillaWebPage::MozillaWebPage(uint32 width, 
													  uint32 height, 
													  bool mipmap, 
													  const std::wstring& url,
													  WebPageProvider::eGraphicsSettingsBehaviour graphicsSettingsBehaviour,
													  int key) :
mWindowId_(INVALID_MOZILLA_WINDOW),	
currentFocus_(false),
pixelsCopy_(NULL),
pixelsSize_(0),
sequenceNum_(MozillaWebPageManager::s_defaultSequenceNum),
key_(key),
chkSum_()
{
	BW_GUARD;
	//mark that we are alive
	initialUrl_ = WebPage::getResolvedUrl(url);
	if (!mozillaInit || llmozlibInstance == NULL) 
	{																	
		ERROR_MSG("MozillaWebPage Mozilla not initialised\n");			
		return;															
	}
	setSize(800, 600, 800, 600, 0, 0);
}


/**
*	create the browser window (unmanaged object as flash uses Directx)
*/
void MozillaWebPageManager::MozillaWebPage::createBrowserWindow()
{
	BW_GUARD;
	if (! isValidMozilla(__FILE__,__LINE__))
	{
		return;
	}

	if (mWindowId_ == INVALID_MOZILLA_WINDOW)
	{
		mWindowId_ = llmozlibInstance->createBrowserWindow( /*will be set later by calling set size directly.*/800, 600);

		CHECK_LLMOZLIB_CALL(llmozlibInstance->addObserver( mWindowId_, this ));

		// this is the default color - just here to show that it can be done
		CHECK_LLMOZLIB_CALL(llmozlibInstance->setBackgroundColor( mWindowId_, 0xff, 0xff, 0xff ));

		// this is the default color - just here to show that it can be done
		CHECK_LLMOZLIB_CALL(llmozlibInstance->setCaretColor( mWindowId_, 0x00, 0x00, 0x00 ));

		// don't flip bitmap
		CHECK_LLMOZLIB_CALL(llmozlibInstance->flipWindow( mWindowId_, false ));

		CHECK_LLMOZLIB_CALL(llmozlibInstance->navigateTo( mWindowId_, bw_wtoacp(initialUrl_)));
		
		// Only used for the initial url so clear it after navigation
		initialUrl_.clear();
	}
}


/**
*	destroy the browser window (unmanaged object as flash uses Directx)
*/
void MozillaWebPageManager::MozillaWebPage::destroyBrowserWindow()
{
	if (! isValidMozilla(__FILE__,__LINE__))
	{
		return;
	}
	if (mWindowId_ != INVALID_MOZILLA_WINDOW)
	{
		llmozlibInstance->remObserver( mWindowId_, this );
		llmozlibInstance->destroyBrowserWindow(mWindowId_);
	}
	mWindowId_ = INVALID_MOZILLA_WINDOW;

}


/**
* Destructor
*/
MozillaWebPageManager::MozillaWebPage::~MozillaWebPage()
{
	BW_GUARD;
	if (isValidMozilla(__FILE__,__LINE__))
	{
		destroyBrowserWindow();
	}
	clearPixels();
}

void MozillaWebPageManager::MozillaWebPage::clearPixels()
{
	if (pixelsCopy_)
	{
		delete[] pixelsCopy_;
		pixelsCopy_ = NULL;
	}
	pixelsSize_ = 0;
	realPixelsWidth_ = 0;
	realPixelsHeight_ = 0;
	chkSum_.clear();
}

/**
*	This method causes the object to navigate to a url
*	@param url the url to navigate to
*/
void MozillaWebPageManager::MozillaWebPage::navigate( const std::wstring& url )
{
	BW_GUARD;
	if (! isValid(__FILE__,__LINE__))
	{
		return;
	}
	std::wstring resolvedUrl = WebPage::getResolvedUrl(url);
	llmozlibInstance->navigateTo( mWindowId_, bw_wtoacp(resolvedUrl));
}


/**
*	This method causes the browser to update itself
*/
void MozillaWebPageManager::MozillaWebPage::updateBrowser(uint sequenceNum)
{
	BW_GUARD;
	if (! isValid(__FILE__,__LINE__))
	{
		sequenceNum_ = sequenceNum;
		return;
	}
	const unsigned char* pixels = llmozlibInstance->grabBrowserWindow( mWindowId_ );
	if (pixels == NULL) 
	{	
		ERROR_MSG("MozillaWebPage Invalid pixels received\n");
		sequenceNum_ = sequenceNum;
		return;
	}
	//guard the pixels memory usage
	{
		browserRowSpan_ = llmozlibInstance->getBrowserRowSpan( mWindowId_ );
		copyPixelsIntoCopy(pixels);
		sequenceNum_ = sequenceNum;
	}
}


void MozillaWebPageManager::MozillaWebPage::copyPixelsIntoCopy(const unsigned char* pixels)
{
	uint srcWidth = webPageWidth_;
	uint srcHeight = webPageHeight_;
	realPixelsWidth_ = textureWidth_;
	realPixelsHeight_ = textureHeight_;


	const uint32* src = (const uint32*)pixels;
	uint32 allocatedSize = 4 * realPixelsWidth_ * realPixelsHeight_;
	uint32* dst = NULL;
	{
		SimpleMutexHolder smh(mozillaWebPageMutex_);
		if (pixelsCopy_)
		{
			if 	(pixelsSize_ == allocatedSize)
			{
				dst = (uint32*)pixelsCopy_;
				//temporary set to null until copy finished (texture will remain the same)
				pixelsCopy_ = NULL;
				pixelsSize_ = 0;
			}
			else 
			{
				delete[] pixelsCopy_;
				pixelsCopy_ = NULL;
				pixelsSize_ = 0;
				dst = (uint32*)new unsigned char[allocatedSize];
			}
		}
		else 
		{
			dst = (uint32*)new unsigned char[allocatedSize];
		}
	}
	int sm = ( browserRowSpan_ >> 2 ) - webPageWidth_;
	uint32 filterWidth = 3;
	uint32 widthRatio = srcWidth/realPixelsWidth_;
	uint32 filterHeight = 3;
	uint32 heightRatio = srcHeight/realPixelsHeight_;
	//no filtering when same width and height
	if (widthRatio == 1 && heightRatio == 1)
	{
		filterWidth = filterHeight = 1;
	}

	uint filterSize = filterWidth * filterHeight;
	uint yDest = 0;
	uint xDest = 0;
	uint32 filterValueRed = 0;
	uint32 filterValueGreen = 0;
	uint32 filterValueBlue = 0;
	chkSum_.clear();
	for ( uint y = 0; y < srcHeight && yDest < realPixelsHeight_; y+= heightRatio)
	{
		for ( uint x = 0; x < srcWidth; x+= widthRatio)
		{
			int startY1 = y > filterHeight ? -1/2 * filterHeight : 0;
			for (int y1 = startY1; y1 < (int)filterHeight + startY1 && y + y1 < srcHeight; y1++)
			{
				int startX1 = x > filterWidth ?  -1/2 * filterWidth : 0;
				for ( int x1 = startX1 ; x1 < (int)filterWidth + startX1 && x + x1 < srcWidth; x1++)
				{
					uint32 rgb = src[(y+y1) * (srcWidth + sm)+ x+x1];
					uint32 red = ((rgb & 0xff0000)>>16);
					uint32 green = ((rgb & 0xff00)>>8);
					uint32 blue = (rgb & 0xff);
					filterValueRed += red / filterSize;
					filterValueGreen += green / filterSize;
					filterValueBlue += blue / filterSize;
				}
			}
			if (filterValueRed > 0xff) filterValueRed = 0xff;
			else if (filterValueRed < 0) filterValueRed = 0;
			if (filterValueGreen > 0xff) filterValueGreen = 0xff;
			else if (filterValueGreen < 0) filterValueGreen = 0;
			if (filterValueBlue > 0xff) filterValueBlue = 0xff;
			else if (filterValueBlue < 0) filterValueBlue = 0;
			if (xDest < realPixelsWidth_) 
			{
				dst[yDest * (realPixelsWidth_)+ xDest] = 0xff000000 | 
					((filterValueRed << 16) & 0xff0000) | 
					((filterValueGreen << 8) & 0xff00) |
					(filterValueBlue & 0xff);
				xDest++;
			}
			//in any case clear the filter.
			filterValueRed = 0;
			filterValueGreen = 0;
			filterValueBlue = 0;
		}
		yDest++;
		xDest=0;
	}
	MD5 md5;
	md5.append( dst, allocatedSize );
	md5.getDigest( chkSum_ );
	{
		//now set the pixelsCopy back
		SimpleMutexHolder smh(mozillaWebPageMutex_);
		pixelsSize_ = allocatedSize;
		pixelsCopy_ = (unsigned char*)dst;
	}
	return;
}


/**
*	This method updates the texture to the current browser state
*/
PROFILER_DECLARE( updateTextureDirect, "updateTextureDirect" );
void MozillaWebPageManager::MozillaWebPage::updateTextureDirect(ComObjectWrap<DX::Texture>& destTexture, uint textureWidth, uint textureHeight, bool mipmap, MD5::Digest& textureChkSum)
{
	BW_GUARD;
	PROFILER_SCOPED( updateTextureDirect );
	SimpleMutexHolder smh(mozillaWebPageMutex_);

	DX::newFrame();

	static DogWatch dwBrowser("UpdateTexture");
	ScopedDogWatch sdw( dwBrowser );
	if (textureWidth != realPixelsWidth_ || textureHeight != realPixelsHeight_)
	{
		return;
	}
	//we do not want to update the texture during navigation - this causes crashes while doing alt + enter.
	if (pixelsCopy_ && destTexture.hasComObject() && chkSum_ != textureChkSum)
	{
		D3DLOCKED_RECT rect;
		if (D3D_OK == destTexture->LockRect( 0, &rect, NULL, D3DLOCK_DISCARD ))
		{
			uint32* src = (uint32*)pixelsCopy_;
			uint32* dst = (uint32*)rect.pBits;
			uint srcWidth;
			uint srcHeight;
			srcWidth = textureWidth;
			srcHeight = textureHeight;
			int paddingPixels = ( rect.Pitch >> 2 ) - textureWidth;
			if ( paddingPixels )
			{
				uint yDest = 0;
				uint xDest = 0;

				for ( uint y = 0; y < srcHeight; y++)
				{
					memcpy(dst, src, srcWidth * 4);
					src += srcWidth;
					dst += srcWidth + paddingPixels;
				}
			}
			else
			{
				memcpy(dst, src, srcWidth * srcHeight * 4);
			}
			destTexture->UnlockRect( 0 );
		}

		if (mipmap)
		{
			destTexture->AddDirtyRect( NULL );
			destTexture->GenerateMipSubLevels();
		}
		textureChkSum = chkSum_;
	}

	DX::newFrame();
}

/**
*	Get the sequence number of the specific MozillaWebPage
*	important in order to know if the web page has done update or the update hasn't been done yet
*/
uint MozillaWebPageManager::MozillaWebPage::getSequenceNum()
{
	return sequenceNum_;
}


/**
*	This method handles mouse events sent to the web page. These should be 
*  In pixel space of the WebPage
*
*	@param	pos		The position of the button event
*	@param	down	Whether the button is down or up
*
*	@return None
*/
void MozillaWebPageManager::MozillaWebPage::handleMouseButtonEvent(const Vector2& pos, bool down)
{
	BW_GUARD;
	if (! isValid(__FILE__,__LINE__))
	{
		return;
	}
	if (down) 
	{
		llmozlibInstance->mouseDown( mWindowId_, static_cast<int>(pos.x), static_cast<int>(pos.y), exactWidth_, exactHeight_);
	}
	else 
	{
		llmozlibInstance->mouseUp (mWindowId_, static_cast<int>(pos.x), static_cast<int>(pos.y), exactWidth_, exactHeight_);
	}
}


/**
*	This method handles scrolling the web page
*  
*	@param	Lines to scroll the web page
*
*/
void MozillaWebPageManager::MozillaWebPage::scrollByLines(int32 lines)
{
	llmozlibInstance->scrollByLines( mWindowId_, lines);
}


/**
*	This method handles mouse move events sent to the web page. These should be 
*  In pixel space of the WebPage
*
*	@param	pos		The position of the button event
*
*	@return None
*/
void MozillaWebPageManager::MozillaWebPage::handleMouseMove(const Vector2& pos)
{
	if (! isValid(__FILE__,__LINE__))
	{
		return;
	}
	llmozlibInstance->mouseMove( mWindowId_, static_cast<int>(pos.x), static_cast<int>(pos.y), exactWidth_, exactHeight_);
}


/**
*	This method handles special int keyboard events sent to the web page. 
*
*	@param	keyCodeIn		The special key clicked
*
*	@return			None 
*/
void MozillaWebPageManager::MozillaWebPage::handleKeyboardEvent(uint32 keyCodeIn)
{
	if (! isValid(__FILE__,__LINE__))
	{
		return;
	}
	if (keyCodeIn == /*tab*/ 9) 
	{
		focusBrowser(true, true);
	}
	llmozlibInstance->keyPress (mWindowId_, keyCodeIn);
	if (keyCodeIn == 9) 
	{
		focusBrowser(false, false);
	}
}


/**
*	This method handles unicode keyboard events sent to the web page. 
*
*	@param	keyCodeIn		The unicode char clicked
*
*	@return			None 
*/
void MozillaWebPageManager::MozillaWebPage::handleUnicodeInput(uint32 keyCodeIn, uint32 codePage )
{
	BW_GUARD;
	if (! isValid(__FILE__,__LINE__))
	{
		return;
	}
	llmozlibInstance->unicodeInput(mWindowId_, keyCodeIn, codePage);
}


/**
*	This method enables proxy usage by this provider
*
*	@param	proxyEnabledIn		Whether to enable the proxy usage
*	@param	proxyHostNameIn		The proxy host name
*	@param	proxyPortIn			The proxy port
*
*	@return			bool		True for success
*/
bool MozillaWebPageManager::MozillaWebPage::enableProxy( bool proxyEnabledIn, const std::string& proxyHostNameIn, uint32 proxyPortIn )
{
	BW_GUARD;
	if (! isValid(__FILE__,__LINE__))
	{
		return false;
	}
	CHECK_LLMOZLIB_CALL_RET(llmozlibInstance->enableProxy( proxyEnabledIn, proxyHostNameIn, proxyPortIn ));
	return true;
}


/**
*	This method enables or disables cookies
*
*	@param	value		The Cookies setting
*
*	@return			bool		True for success
*/
bool MozillaWebPageManager::MozillaWebPage::enableCookies( bool value )
{
	BW_GUARD;
	if (! isValid(__FILE__,__LINE__))
	{
		return false;
	}
	CHECK_LLMOZLIB_CALL_RET(llmozlibInstance->enableCookies( value ));
	return true;
}

/**
*	This method navigates one page back
*
*	@return			bool  True if succsess
*/
bool MozillaWebPageManager::MozillaWebPage::navigateBack()
{
	BW_GUARD;
	if (! isValid(__FILE__,__LINE__))
	{
		return false;
	}
	return llmozlibInstance->navigateBack(mWindowId_);
}


/**
*	This method navigates one page forward.
*
*	@return			bool  True if succsess
*/
bool MozillaWebPageManager::MozillaWebPage::navigateForward()
{
	BW_GUARD;
	if (! isValid(__FILE__,__LINE__))
	{
		return false;
	}
	return llmozlibInstance->navigateForward(mWindowId_);
}


/**
 *	This method reload the web page
 *
 *	@return			bool  True if succsess
*/
bool MozillaWebPageManager::MozillaWebPage::navigateReload()
{
	BW_GUARD;
	if (! isValid(__FILE__,__LINE__))
	{
		return false;
	}
	return llmozlibInstance->navigateReload(mWindowId_);
}


/**
*	This method sets the size of the mozila component
*
*	@param width the width of the web page texture
*	@param height the height of the web page texture
*
*	@return			bool  True if succsess
*/
bool MozillaWebPageManager::MozillaWebPage::setSize (uint32 width, 
													 uint32 height, 
													 uint32 textureWidth, 
													 uint32 textureHeight,
													 uint32 exactWidth, 
													 uint32 exactHeight)
{
	BW_GUARD;
	if (! isValidMozilla(__FILE__,__LINE__))
	{
		return false;
	}
	if (mWindowId_ == INVALID_MOZILLA_WINDOW)
	{
		destroyBrowserWindow();
		createBrowserWindow();
	}
	CHECK_LLMOZLIB_CALL(llmozlibInstance->setSize( mWindowId_, width, height));
	webPageWidth_ = width;
	webPageHeight_ = height;
	textureWidth_ = textureWidth;
	textureHeight_ = textureHeight;
	exactWidth_ = exactWidth;
	exactHeight_ = exactHeight;
	SimpleMutexHolder smh(mozillaWebPageMutex_);
	clearPixels();
	if (llmozlibInstance->getBrowserDepth( mWindowId_ ) != 4) 
	{	
		ERROR_MSG("MozillaWebPage Invalid browser depth\n");
		return false;
	}
	browserRowSpan_ = llmozlibInstance->getBrowserRowSpan( mWindowId_ );
	return true;
}


/**
*
*	This method is used to set the browser focus mode
*
*	@param	focus   The new focus state
*	@param	enable  Whether to enable the window (currently not used)
*
*/
void MozillaWebPageManager::MozillaWebPage::focusBrowser (bool focus, bool enable)
{
	BW_GUARD;
	if (! isValid(__FILE__,__LINE__))
	{
		return;
	}
	//EnableWindow(MozillaWebPageManager::s_usedWindow, enable);
	llmozlibInstance->focusBrowser(mWindowId_, focus);
	//if changing the focus (was focused and now isn't
	//set our FD window focus back to be focused
	if (currentFocus_ && focus == false) 
	{
		::SetFocus(s_applicationWindow);
	}
	currentFocus_ = focus;
}


void MozillaWebPageManager::MozillaWebPage::allowCursorInteraction (bool allow)
{
	BW_GUARD;
	llmozlibInstance->allowCursorInteraction(allow);
}


/**
*
*	This method is used to set the default page when the web page
*  isn't found.
*
*	@param	url   The redirect location
*/
void MozillaWebPageManager::MozillaWebPage::set404Redirect( const std::wstring& url  )
{ 
	BW_GUARD;
	if (! isValid(__FILE__,__LINE__))
	{
		return;
	}
	std::wstring usedUrl = WebPage::getResolvedUrl(url);

	CHECK_LLMOZLIB_CALL(llmozlibInstance->set404RedirectUrl( mWindowId_, 
		bw_wtoacp(usedUrl)));
}


/**
*	This method is used to evaluate JavaScript code on this web page.
*/
std::string MozillaWebPageManager::MozillaWebPage::evaluateJavaScript( const std::string & script )
{
	BW_GUARD;
	if (! isValid(__FILE__,__LINE__))
	{
		return "";
	}
	return llmozlibInstance->evaluateJavascript( mWindowId_, script );
}

/**
* Callback method on Web page event
*/
void MozillaWebPageManager::MozillaWebPage::onPageChanged( const EventType& eventIn )
{
	BW_GUARD;
	CommandMozillaWebPageCallbackBase* command = new CommandMozillaWebPageCallbackOnPageChanged(key_);
	command->event = eventIn;
	MozillaWebPageManager::instance().pushCallback(MozillaWebPageCommandPtr(command));
}


/**
* Callback method on Web page event
*/
void MozillaWebPageManager::MozillaWebPage::onNavigateBegin( const EventType& eventIn )
{
	BW_GUARD;
	CommandMozillaWebPageCallbackBase* command = new CommandMozillaWebPageCallbackOnNavigationBegin(key_);
	command->event = eventIn;
	MozillaWebPageManager::instance().pushCallback(MozillaWebPageCommandPtr(command));
}


/**
* Callback method on Web page event
*/
void MozillaWebPageManager::MozillaWebPage::onNavigateComplete( const EventType& eventIn )
{
	BW_GUARD;
	CommandMozillaWebPageCallbackBase* command = new CommandMozillaWebPageCallbackOnNavigationComplete(key_);
	command->event = eventIn;
	MozillaWebPageManager::instance().pushCallback(MozillaWebPageCommandPtr(command));
}


/**
* Callback method on Web page event
*/
void MozillaWebPageManager::MozillaWebPage::onUpdateProgress( const EventType& eventIn )
{
	BW_GUARD;

	CommandMozillaWebPageCallbackBase* command = new CommandMozillaWebPageCallbackOnUpdateProgress(key_);
	command->event = eventIn;
	MozillaWebPageManager::instance().pushCallback(MozillaWebPageCommandPtr(command));
}


/**
* Callback method on Web page event
*/
void MozillaWebPageManager::MozillaWebPage::onStatusTextChange( const EventType& eventIn )
{
	BW_GUARD;
	CommandMozillaWebPageCallbackBase* command = new CommandMozillaWebPageCallbackOnStatusTextChange(key_);
	command->event = eventIn;
	MozillaWebPageManager::instance().pushCallback(MozillaWebPageCommandPtr(command));
}


/**
* Callback method on Web page event
*/
void MozillaWebPageManager::MozillaWebPage::onLocationChange( const EventType& eventIn )
{
	BW_GUARD;

	CommandMozillaWebPageCallbackBase* command = new CommandMozillaWebPageCallbackOnLocationChange(key_);
	command->event = eventIn;
	MozillaWebPageManager::instance().pushCallback(MozillaWebPageCommandPtr(command));
}


/**
* Callback method on Web page event
*/
void MozillaWebPageManager::MozillaWebPage::onClickLinkHref( const EventType& eventIn )
{
	BW_GUARD;
	CommandMozillaWebPageCallbackBase* command = new CommandMozillaWebPageCallbackOnClickLinkHref(key_);
	command->event = eventIn;
	MozillaWebPageManager::instance().pushCallback(MozillaWebPageCommandPtr(command));
}


/**
* Callback method on Web page event
*/
void MozillaWebPageManager::MozillaWebPage::onClickLinkNoFollow( const EventType& eventIn )
{
	BW_GUARD;
	CommandMozillaWebPageCallbackBase* command = new CommandMozillaWebPageCallbackOnClickLinkNoFollow(key_);
	command->event = eventIn;
	MozillaWebPageManager::instance().pushCallback(MozillaWebPageCommandPtr(command));
}


/**
*	Check if the object is valid
*/
bool MozillaWebPageManager::MozillaWebPage::isValid(const char* file, int line)
{
	BW_GUARD;
	if (! isValidMozilla(file,line)) 
	{ 
		return false; 
	}	
	if (mWindowId_ == INVALID_MOZILLA_WINDOW) 
	{
		ERROR_MSG("MozillaWebPageManager::MozillaWebPage::isValid failed due to bad window id. Called from %s line %d\n",
			file, line);
		return false;
	}
	return true;
}


/**
*	Clean memory for this web page
*/
bool MozillaWebPageManager::MozillaWebPage::cleanUnusedMemory()
{
	SimpleMutexHolder smh(mozillaWebPageMutex_);
	clearPixels();
	llmozlibInstance->cleanMemory( mWindowId_ );
	return true;
}
