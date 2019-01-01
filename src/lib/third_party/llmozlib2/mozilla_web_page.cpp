#include "pch.hpp"
#include "mozilla_web_page.hpp"
#include "cstdmf/bgtask_manager.hpp"
#include "ashes/simple_gui.hpp"
#include "moo/render_context.hpp"
#include "third_party/LLMozlib2/llmozlib_virtual_wrapper.h"

#undef BW_GUARD
#define BW_GUARD

#ifdef _DEBUG
static const wchar_t MOZILLA_DIRECTORY[]=L"mozilla\\debug";
static const char MOZILLA_DIRECTORY_A[]="mozilla\\debug";
#else
static const wchar_t MOZILLA_DIRECTORY[]=L"mozilla\\release";
static const char MOZILLA_DIRECTORY_A[]="mozilla\\release";
#endif

DECLARE_DEBUG_COMPONENT2( "Web ", 0 )

#define DEBUG_WEB
#ifdef DEBUG_WEB
#define PRINT_DEBUG INFO_MSG
#else
#define PRINT_DEBUG(param) 
#endif

#define USE_EXTRA_WINDOW
const LPTSTR MOZILLA_WINDOW_CLASS_NAME = L"TempMozilla";

typedef LLMozLib_Virtual_Wrapper* (* GET_INSTANCE)();
LLMozLib_Virtual_Wrapper* llmozlibInstance;

int getNextTexture(int currentTexture)
{
	return (currentTexture + 1) % NUMBER_OF_TEXTURES;
}

#define CHECK_LLMOZLIB_CALL(call) if ( ! (call)) { ERROR_MSG ("%s failed %d", #call, llmozlibInstance->getLastError());}
#define CHECK_LLMOZLIB_CALL_RET(call) do { \
	bool ret = call; \
	if ( ! (ret)) \
	{ \
		ERROR_MSG ("%s failed %d", #call, llmozlibInstance->getLastError()); \
		return ret; \
	} \
} \
while (false) 

//The mutex to protect llmozlib as either llmozlib or gecko are NOT thread safe.
SimpleMutex smMozlibAccess; 

namespace
{
	/*
	*	This background task helps us manage the browser updates
	*/
	class MozillaWebPageBGTask : public BackgroundTask
	{
	public:
		MozillaWebPageBGTask( MozillaWebPage* mozillaWebPage ) :
		  mozillaWebPage_( mozillaWebPage  )
		  {
		  }
		  void doBackgroundTask( BgTaskManager & mgr )
		  {
			  BW_GUARD;
			  static DogWatch dwBrowser("MozillaBrowser");
			  ScopedDogWatch sdw( dwBrowser );
			  doTask(mgr);
		  }
		  void doMainThreadTask( BgTaskManager & mgr ) 
		  {
			  BW_GUARD;
			  static DogWatch dwBrowser("MozillaBrowser");
			  ScopedDogWatch sdw( dwBrowser );
			  doTask(mgr);
		  }
		  void doTask( BgTaskManager & mgr )
		  {
			  BW_GUARD;
			  mozillaWebPage_->updateBrowser();
			  mozillaWebPage_->updateTexture();
			  mozillaWebPage_ = NULL;
		  }

		  bool finished() const { return !mozillaWebPage_.hasObject(); }
	private:
		SmartPointer<MozillaWebPage> mozillaWebPage_;
	};
}

#ifdef NOT_DEF
//NOT used - might be used if moving llmozlig to a different thread. which might be not supported
namespace
{
	/*
	*	This background task helps us manage the browser updates
	*/
	class MozillaWebPageInitTask : public BackgroundTask
	{
	public:
		MozillaWebPageInitTask( ) 
		{
		}
		void doBackgroundTask( BgTaskManager & mgr )
		{
			BW_GUARD;
			MozillaWebPage::mozillaInit();
		}
		void doMainThreadTask( BgTaskManager & mgr ) 
		{
			BW_GUARD;
		}
		bool finished() const { return !mozillaWebPage_.hasObject(); }
	private:
		SmartPointer<MozillaWebPage> mozillaWebPage_;
	};
}
#endif

//static variables
//Whether mizilla was initialised
bool MozillaWebPage::mozillaInit_ = false;
//The main application window (given as a parameter to mozilla)
HWND MozillaWebPage::applicationWindow_ = 0;

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

/**
*	Constructor for the MozillaWebPage object
*	@param width the width of the web page texture
*	@param height the height of the web page texture
*	@param url the url to use for the web page texture
*/
MozillaWebPage::MozillaWebPage(uint32 width, uint32 height, const std::wstring& url) :
width_(width), height_(height), currentTextureUsed_(0), inNavigate_(false)
{
	BW_GUARD;
	SimpleMutexHolder smh(smMozlibAccess);
	PRINT_DEBUG("hERE1");
	//Init the mozilla interface (done once)
	if (! mozillaInit_) 
	{
		HWND usedWindow = applicationWindow_;
#ifdef USE_EXTRA_WINDOW
		usedWindow = MozillaWebPage::createExtraWindow();
#endif
		if (! mozillaInit(usedWindow) )
		{
			return;
		}
	}

	mWindowId_ = llmozlibInstance->createBrowserWindow( width, height );

	CHECK_LLMOZLIB_CALL(llmozlibInstance->setSize( mWindowId_, width, height ));

	CHECK_LLMOZLIB_CALL(llmozlibInstance->addObserver( mWindowId_, this ));

	// this is the default color - just here to show that it can be done
	CHECK_LLMOZLIB_CALL(llmozlibInstance->setBackgroundColor( mWindowId_, 0xff, 0xff, 0xff ));

	// this is the default color - just here to show that it can be done
	CHECK_LLMOZLIB_CALL(llmozlibInstance->setCaretColor( mWindowId_, 0x00, 0x00, 0x00 ));

	// don't flip bitmap
	CHECK_LLMOZLIB_CALL(llmozlibInstance->flipWindow( mWindowId_, false ));

	CHECK_LLMOZLIB_CALL(llmozlibInstance->navigateTo( mWindowId_, url));

	if (llmozlibInstance->getBrowserDepth( mWindowId_ ) == 3) 
	{
		format_ = D3DFMT_R8G8B8;
	}
	else if (llmozlibInstance->getBrowserDepth( mWindowId_ ) == 4) 
	{	
		format_ = D3DFMT_A8R8G8B8;
	}
	else 
	{
		ERROR_MSG("Invalid browser depth");
	}

	if (Moo::rc().device())
	{
		createUnmanagedObjects();
	}
}

/**
* Destructor
*/
MozillaWebPage::~MozillaWebPage()
{
	BW_GUARD;
	SimpleMutexHolder smh(smMozlibAccess);
	llmozlibInstance->destroyBrowserWindow(mWindowId_);
	deleteUnmanagedObjects();
#ifdef USE_EXTRA_WINDOW
	UnregisterClass( MOZILLA_WINDOW_CLASS_NAME, GetModuleHandle( NULL ) );
#endif
	//delete pBrowser_;
}

/**
*	This method causes the object to navigate to a url
*	@url the url to navigate to
*/
void MozillaWebPage::navigate( const std::wstring& url )
{
	BW_GUARD;
	SimpleMutexHolder smh(smMozlibAccess);
	if (! mozillaInit_) return;
	url_ = url;
	llmozlibInstance->navigateTo( mWindowId_, url);
}

/**
*	This method causes the browser to update itself
*/
void MozillaWebPage::updateBrowser()
{
	BW_GUARD;
	//SimpleMutexHolder smh(smMozlibAccess);
	if (! mozillaInit_) return;
	//Looks like this is done automatically (updated in a separate thread by gecko)
}

/**
*	This method updates the texture to the current browser state
*/
void MozillaWebPage::updateTexture()
{
	BW_GUARD;
	//SimpleMutexHolder smh(smMozlibAccess);
	if (! mozillaInit_) return;
	//we do not want to update the texture during navigation - this causes crashes while doing alt + enter.
	if (inNavigate_) 
	{
		return;
	}
	int usedTexture = 0;
	PRINT_DEBUG("windowid %d updateTexture %d ", mWindowId_, usedTexture);
	int nextTexture = getNextTexture(currentTextureUsed_);
	if (pTexture_[nextTexture].hasComObject())
	{
		IDirect3DSurface9* surface = NULL;
		if (pTexture_[nextTexture]->GetSurfaceLevel( 0, &surface ) == D3D_OK) 
		{

		RECT rect = { 0, 0, width_, height_ };
		const unsigned char* pixels = llmozlibInstance->grabBrowserWindow( mWindowId_ );

#ifdef NOT_DEFINED
		char buffer1[2048];
		bw_snprintf(buffer1, sizeof(buffer1), "Here, browser surface %x \
											  pixes %x \
											  format %d \
											  rowspan %d \
											  depth %d \
											  width %d", 
											  (int)surface, 
											  (int)pixels,
											  format_,
											  llmozlibInstance->getBrowserRowSpan( mWindowId_ ), 
											  llmozlibInstance->getBrowserDepth( mWindowId_), 
											  width_);
		PRINT_DEBUG(buffer1);
#endif NOT_DEFINED
		HRESULT tempResult;
		//		D3DLOCKED_RECT temp;
		//surface->LockRect(&temp, &rect, 0);
		if ((tempResult = D3DXLoadSurfaceFromMemory( surface, NULL, NULL,
			pixels, format_, 
			llmozlibInstance->getBrowserRowSpan( mWindowId_ ), NULL, &rect, D3DX_DEFAULT, 0 )) != D3D_OK) 
		{
			ERROR_MSG ("Failed loading the texture D3DXLoadSurfaceFromMemory %d - %s .", tempResult, DX::errorAsString( tempResult ).c_str());
		} 
		//surface->UnlockRect();
		surface->Release();
		}
		else 
		{
			ERROR_MSG ("Failed Getting the surface");
		}
	}
	currentTextureUsed_ = nextTexture;
	PRINT_DEBUG("windowid %d updateTexture ending %d ", mWindowId_, usedTexture);
}

void MozillaWebPage::handleMouseButtonEvent(const Vector2& pos, bool down)
{
	SimpleMutexHolder smh(smMozlibAccess);
	if (! mozillaInit_) return;
	if (down) 
	{
		llmozlibInstance->mouseDown( mWindowId_, static_cast<int>(pos.x), static_cast<int>(pos.y));
	}
	else 
	{
		llmozlibInstance->mouseUp (mWindowId_, static_cast<int>(pos.x), static_cast<int>(pos.y));
	}
}

void MozillaWebPage::handleKeyboardEvent(int keyCodeIn)
{
	SimpleMutexHolder smh(smMozlibAccess);
	if (! mozillaInit_) return;
	PRINT_DEBUG("sending key %d", (int)keyCodeIn);

	llmozlibInstance->keyPress (mWindowId_, keyCodeIn);
}

void MozillaWebPage::handleUnicodeInput(unsigned long uni_char )
{
	SimpleMutexHolder smh(smMozlibAccess);
	if (! mozillaInit_) return;
	PRINT_DEBUG("sending key %d", (int)uni_char);

	llmozlibInstance->unicodeInput(mWindowId_, uni_char);
}

bool MozillaWebPage::enableProxy( bool proxyEnabledIn, const std::string& proxyHostNameIn, int proxyPortIn )
{
	CHECK_LLMOZLIB_CALL_RET(llmozlibInstance->enableProxy( proxyEnabledIn, proxyHostNameIn, proxyPortIn ));
	return true;
}

/**
*	This method updates the browser and texture
*/
void MozillaWebPage::update()
{
	BW_GUARD;
	SimpleMutexHolder smh(smMozlibAccess);
	if (! mozillaInit_) return;
	//	this->updateBrowser();
	//	this->updateTexture();

}

void MozillaWebPage::setApplicationWindow(HWND newApplicationWindow)
{
	applicationWindow_ = newApplicationWindow;
}

//init the mozilla application
class WorkingDirectorySetter
{
	public: 
	WorkingDirectorySetter(const wchar_t* directory)
	{
		if (GetCurrentDirectory(sizeof(currentDirectory_)/sizeof(currentDirectory_[0]), currentDirectory_) == 0)
		{
			ERROR_MSG("Failed to get the current directory");
		}
		if (SetCurrentDirectory(directory) == 0) 
		{
			ERROR_MSG("Failed to set the current directory");
		}
		wchar_t dirAfterChange[2048];
		wchar_t path[10000];
		if (GetCurrentDirectory(sizeof(dirAfterChange)/sizeof(dirAfterChange[0]), dirAfterChange) == 0)
		{
			ERROR_MSG("Failed to get the current directory");
		}
		//also add this to the path 
		GetEnvironmentVariable(L"PATH", path, sizeof(path)/sizeof(path[0]));
		//TBD - change sizes of the paths (maybe used strings)
		//check ret codes of the SetEn and GetEn functions
		std::wstring temp = std::wstring(path) + std::wstring(L";") + std::wstring(dirAfterChange);
		SetEnvironmentVariable(L"PATH", temp.c_str());
	}

	~WorkingDirectorySetter()
	{
		if (SetCurrentDirectory(currentDirectory_) == 0) 
		{
			ERROR_MSG("Failed to set the current directory back");
		}
	}
	wchar_t currentDirectory_[2048];
};

#ifdef USE_EXTRA_WINDOW
HWND MozillaWebPage::createExtraWindow()
{
   WNDCLASS wndClass = { sizeof( WNDCLASS ) };
   wndClass.hInstance = GetModuleHandle( NULL );
   wndClass.lpfnWndProc        = DefWindowProc;
	wndClass.lpszClassName      = MOZILLA_WINDOW_CLASS_NAME;
   wndClass.style              = CS_HREDRAW | CS_VREDRAW;


    if ( !::RegisterClass(&wndClass) )
    {
        WARNING_MSG("Failed to register the window class %S", get_formatted_message(::GetLastError()).c_str());
    }
	PRINT_DEBUG("Mozilla window registered");
    HWND mozillaWindow = ::CreateWindow(MOZILLA_WINDOW_CLASS_NAME,     /* class name */
                               L"Mozilla",      /* title to window */
                               WS_OVERLAPPEDWINDOW, /* style */
                               CW_USEDEFAULT,   /* start pos x */
                               CW_USEDEFAULT,   /* start pos y */
                               width_,        /* width */
                               height_,       /* height */
                               NULL,            /* parent HWND */
                               NULL,            /* menu HANDLE */
                               GetModuleHandle( NULL ),       /* */
                               NULL);           /* creatstruct param */
    if ( !mozillaWindow )
    {
		ERROR_MSG("Failed to create the mozilla window error %S", get_formatted_message(::GetLastError()).c_str());
        return NULL;
    }
//    ShowWindow(mozillaWindow, SW_SHOW);
    UpdateWindow(mozillaWindow);
	return mozillaWindow;
}
#endif

bool MozillaWebPage::mozillaInit(HWND usedWindow)
{
	//Load the mozilla shared library
	HMODULE module = 0;
	{
		WorkingDirectorySetter ws(MOZILLA_DIRECTORY);
		module = LoadLibrary(L"llmozlib_dll");
		if (! module) 
		{
			ERROR_MSG("Failed loading llmozlib");
			return false;
		}

		GET_INSTANCE get_instance_func = (GET_INSTANCE)GetProcAddress(module,"getInstance");
		if (! get_instance_func) 
		{
			ERROR_MSG("Failed getting proc");
			return false;
		}
		llmozlibInstance = get_instance_func();
		INFO_MSG("LLMozLib version %s\n", llmozlibInstance->getVersion().c_str());

		if (usedWindow == 0)
		{
			ERROR_MSG("Please make sure you set the application window before using the mozilla interface");
			return false;
		}

		char currentDirectory[2048];
		if (GetCurrentDirectoryA(sizeof(currentDirectory)/sizeof(currentDirectory[0]), currentDirectory) == 0)
		{
			ERROR_MSG("Failed to get the current directory");
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
		if (! profile_dir_exists) 
		{
			if (! CreateDirectoryA(profileDir.c_str(), NULL) )
			{
				ERROR_MSG("Failed to create directory");
			}
		}
		char temp[2004];
		bw_snprintf (temp, sizeof(temp), "Application dir %s componentDir %s profileDir %s", applicationDir.c_str(), componentDir.c_str(), profileDir.c_str());
		PRINT_DEBUG(temp);

		mozillaInit_ = true;
		
		//init the mozilla api	
		CHECK_LLMOZLIB_CALL(llmozlibInstance->init( applicationDir, componentDir, profileDir, usedWindow));

		// enable cookies
		CHECK_LLMOZLIB_CALL(llmozlibInstance->enableCookies( true ));

		// turn proxy off
		CHECK_LLMOZLIB_CALL(llmozlibInstance->enableProxy( false, "", 0 ));

		// append details to agent string
		std::ostringstream codec;
		codec << "(BigWorld)";
		llmozlibInstance->setBrowserAgentId( codec.str() );
	}
	return true;
}

void MozillaWebPage::onNavigateBegin( const EventType& eventIn )
{
	inNavigate_ = true;
};

////////////////////////////////////////////////////////////////////////////////
// virtual
void MozillaWebPage::onNavigateComplete( const EventType& eventIn )
{
	inNavigate_ = false;
};

/**
*	Callback method for when the device is lost or changed
*/
void MozillaWebPage::createUnmanagedObjects()
{
	BW_GUARD;
	createTexture();
}

/**
*	Callback method for when the device is lost or changed
*/
void MozillaWebPage::deleteUnmanagedObjects()
{
	BW_GUARD;
	destroyTexture();
}

/**
*	Helper method to create the texture for the web page
*/
void MozillaWebPage::createTexture()
{
	BW_GUARD;
	for (int i = 0; i < NUMBER_OF_TEXTURES; i++) 
	{
		pTexture_[i] = Moo::rc().createTexture( width_, height_, 1, 
			D3DUSAGE_DYNAMIC, format_, D3DPOOL_DEFAULT, "WebPage/MozillaWebTexture/" );
	}
}

/**
*	Helper method to destroy the texture for the web page
*/
void MozillaWebPage::destroyTexture()
{
	BW_GUARD;
	for (int i = 0; i < NUMBER_OF_TEXTURES; i++) 
	{
		pTexture_[i] = NULL;
	}
}

/**
*	This is the overridden pTexture method from the BaseTexture interface,
*	it return the d3d texture for the web page
*	It also makes sure the texture is updated.
*/
DX::BaseTexture* MozillaWebPage::pTexture( ) 
{ 
	BW_GUARD;
	// Create the update callback as we are using the texture this frame
	SimpleMutexHolder smh(smMozlibAccess);
	if (! mozillaInit_) return NULL;

	//if (!webPageTask_.exists() || webPageTask_->finished())
	//{
	//	PRINT_DEBUG("creatign new bg task");
	//	webPageTask_ = new MozillaWebPageBGTask( this );
		//TBD Maybe in the future we can do this in a background thread
		//BgTaskManager::instance().addBackgroundTask( webPageTask_ );
	//	BgTaskManager::instance().addMainThreadTask( webPageTask_ );
	//}
	
    updateBrowser();
	updateTexture();

	return pTexture_[currentTextureUsed_].pComObject(); 
}

/**
*	This is the overridden width method from the BaseTexture interface,
*/
uint32 MozillaWebPage::width( ) const 
{ 
	return width_; 
}

/**
*	This is the overridden height method from the BaseTexture interface,
*/
uint32 MozillaWebPage::height( ) const 
{ 
	return height_; 
}

/**
*	This is the overridden format method from the BaseTexture interface,
*/
D3DFORMAT MozillaWebPage::format( ) const 
{ 
	return format_; 
}

/**
*	This is the overridden textureMemoryUsed method from the BaseTexture interface,
*/
uint32 MozillaWebPage::textureMemoryUsed( ) 
{ 
	if (format_ == D3DFMT_R8G8B8) 
	{
		return width_ * height_ * 3;
	}
	else 
	{
		return width_ * height_ * 4;
	}
}

