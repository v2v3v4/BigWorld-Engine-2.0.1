#ifdef _WINDOWS
#include <windows.h>
#endif

#include <iostream>
#include "llmozlib2.h"
#include <list>

using std::cerr;
using std::endl;
using std::list;

const LPTSTR MOZILLA_WINDOW_CLASS_NAME = L"TempMozilla";

HWND createExtraWindow()
{
	WNDCLASS wndClass = { sizeof( WNDCLASS ) };
	wndClass.hInstance = ::GetModuleHandle( NULL );
	wndClass.lpfnWndProc        = DefWindowProc;
	wndClass.lpszClassName      = MOZILLA_WINDOW_CLASS_NAME;
	wndClass.style              = CS_HREDRAW | CS_VREDRAW;
	HWND mozillaWindow = 0;


	if (!::RegisterClass(&wndClass) )
	{
		//might fail if unregister didn't happen.
		std::cerr << "MozillaWebPage Failed to register the window class ";
	}
	mozillaWindow = ::CreateWindow(MOZILLA_WINDOW_CLASS_NAME,     /* class name */
		L"Mozilla",      /* title to window */
		WS_OVERLAPPEDWINDOW, /* style */
		CW_USEDEFAULT,   /* start pos x */
		CW_USEDEFAULT,   /* start pos y */
		/*Doesn't change based on the documentations*/800,        /* width */
		/*Doesn't change based on the documentations*/600,       /* height */
		NULL,            /* parent HWND */
		NULL,            /* menu HANDLE */
		::GetModuleHandle( NULL ),       /* */
		NULL);           /* creatstruct param */
	if ( !mozillaWindow )
	{
		cerr << "MozillaWebPage Failed to create the mozilla window error " << endl;
		return NULL;
	}
	UpdateWindow(mozillaWindow);
	return mozillaWindow;
}

int browserWindowWidth = 1024; 
int browserWindowHeight = 768;
typedef list<int> WindowList;

////////////////////////////////////////////////////////////////////////////////
//
int main( int argc, char* argv[] )
{
	// create a single browser window and set things up.
	std::string applicationDir = std::string( argv[0] ).substr( 0, std::string( argv[0] ).find_last_of("\\/") );
	std::string componentDir = applicationDir;
	std::string profileDir = applicationDir + "\\" + "profile";
	HWND usedHwnd = createExtraWindow();
	LLMozLib::getInstance()->init( applicationDir, componentDir, profileDir, usedHwnd);


	WindowList windowList;

	for (int i = 0; i <20; i++) 
	{
		int mBrowserWindowId = LLMozLib::getInstance()->createBrowserWindow( browserWindowWidth, browserWindowHeight );
		std::cout << "mBrowserWindowId " << mBrowserWindowId << std::endl;
		// tell LLMozLib about the size of the browser window
		LLMozLib::getInstance()->setSize( mBrowserWindowId, browserWindowWidth, browserWindowHeight );

		// append details to agent string
		LLMozLib::getInstance()->setBrowserAgentId( "TestAgent");

		// don't flip bitmap
		LLMozLib::getInstance()->flipWindow( mBrowserWindowId, false );

		// go to the "home page"
		for (int j = 0; j < 10; j++) 
		{
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.google.com" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.cnn.com" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "http://www.youtube.com/watch?v=j-dg6cjZ2z8&feature=bz302" );
			const unsigned char* pixels = LLMozLib::getInstance()->grabBrowserWindow(mBrowserWindowId );
		}
		windowList.push_back(mBrowserWindowId);
	}

	WindowList::iterator iter = windowList.begin();
	while (iter != windowList.end())
	{
		std::cout << "list content " << *iter << std::endl;
		iter++;
	}

	iter = windowList.begin();
	int i = 0;
	while (iter != windowList.end())
	{
		std::cout << "Releasing " << *iter << " loop " << i++ << std::endl;
		LLMozLib::getInstance()->destroyBrowserWindow(*iter);
		windowList.erase(iter);
		iter = windowList.begin();
	}
	
	

	for (int k = 0; k < 20; k++)
	{
		int mBrowserWindowId = LLMozLib::getInstance()->createBrowserWindow( browserWindowWidth, browserWindowHeight );
		std::cout << "k" << k << "mBrowserWindowId " << mBrowserWindowId << std::endl;
		// tell LLMozLib about the size of the browser window
		LLMozLib::getInstance()->setSize( mBrowserWindowId, browserWindowWidth, browserWindowHeight );

		// append details to agent string
		LLMozLib::getInstance()->setBrowserAgentId( "TestAgent");

		// don't flip bitmap
		LLMozLib::getInstance()->flipWindow( mBrowserWindowId, false );

		for (int j = 0; j < 100; j++) 
		{
			// go to the "home page"
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.google.com" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.cnn.com" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "http://www.youtube.com/watch?v=j-dg6cjZ2z8&feature=bz302" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.yahoo.com" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.youtube.com" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.walla.co.il" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.msn.com" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.vvv.com" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.37.com" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.mozilla.com" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.tier3.com" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.hotbot.com" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.ubrowser.com" );
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, "www.aaa.com" );
		}
		//	const unsigned char* pixels = LLMozLib::getInstance()->grabBrowserWindow(mBrowserWindowId );
		LLMozLib::getInstance()->destroyBrowserWindow(mBrowserWindowId);

	}
	int a = 0;
	std::cin >> a;
	
	LLMozLib::getInstance()->reset();
	std::cin >> a;
}

