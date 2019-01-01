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
#include "bw_winmain.hpp"


//#include <crtdbg.h>
#include <mmsystem.h>
#include <windowsx.h>


#include "moo/init.hpp"
#include "pyscript/script.hpp"
#include "cstdmf/string_utils.hpp"
#include "cstdmf/message_box.hpp"
#include "ashes/simple_gui.hpp"
#include "app.hpp"
#include "open_automate/open_automate_wrapper.hpp"
#include "scaleform/config.hpp"
#include "web_render/mozilla_web_page.hpp"
#if SCALEFORM_SUPPORT
	#include "scaleform/ime.hpp"
#endif
#include "app_config.hpp"

static const int MIN_WINDOW_WIDTH = 100;
static const int MIN_WINDOW_HEIGHT = 100;

int parseCommandLine( LPCTSTR str );

bool g_bActive = false;
bool g_bAppQuit = false;
static std::string configFilename = "";


/**
 *	This function is the BigWorld implementation of WinMain and contains the
 *	message pump. Before this function is called the application should already
 *	have registered a WNDCLASS with class name lpClassName. The registered
 *	WndProc function should also call BWWndProc.
 *
 *	@param hInstance	The instance handle passed to WinMain.
 *	@param lpCmdLine	The command line string passed to WinMain.
 *	@param nCmdShow		The initial window state passed to WinMain.
 *	@param lpClassName	The name of the WNDCLASS previously registered.
 *	@param lpWindowName	The window title for the program.
 *
 *	@return	The wParam value of the WM_QUIT that should be returned by WinMain.	
 */
int BWWinMain(	HINSTANCE hInstance,
				LPCTSTR lpCmdLine,
				int nCmdShow,
				LPCTSTR lpClassName,
				LPCTSTR lpWindowName )
{
	BW_GUARD;

	// Initiating here means that it is no longer destroyed at static
	// destruction time.
	BWResource bwresource;
	HWND parent = NULL;
	std::wstring commandLine = lpCmdLine;
	static const wchar_t paramSwitch[] = L"parentWindow=";

	if (commandLine.find( paramSwitch ) != commandLine.npos)
	{
		commandLine = commandLine.substr(
			commandLine.find( paramSwitch ) + wcslen( paramSwitch ) );
		commandLine = commandLine.substr( 0, commandLine.find( ' ' ) );
	
		if (!commandLine.empty())
		{
			std::wstringstream ss( commandLine );
			ULONG_PTR i;

			ss >> i;
			parent = (HWND)i;

			if (!IsWindow( parent ))
			{
				parent = NULL;
			}
		}
	}

	if (!parseCommandLine( lpCmdLine ))
	{
		return FALSE;
	}

	// Initialise Moo
	if ( !Moo::init() )
	{
		return FALSE;
	}

	//int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	//flags |= _CRTDBG_LEAK_CHECK_DF | _CRTDBG_DELAY_FREE_MEM_DF;
	//_CrtSetDbgFlag(flags);

	const int borderWidth = GetSystemMetrics(SM_CXFRAME);
	const int borderHeight = GetSystemMetrics(SM_CYFRAME);
	const int titleHeight = GetSystemMetrics(SM_CYCAPTION);

    // Create the main window
	timeBeginPeriod(1);

    HWND hWnd = NULL;
	MSG msg;
	ZeroMemory( &msg, sizeof(msg) );

	// App scope
	try
	{
		// Initialise the application.
		App app( configFilename, compileTimeString );

		//now get the application window title
		DataSectionPtr pSection = AppConfig::instance().pRoot()->openSection("appTitle", false);
		std::wstring windowName = lpWindowName;
		if(pSection)
		{
			windowName = pSection->asWideString(L"BigWorld Client");
			extern wchar_t* BUILD_CONFIGURATION;
			windowName+= std::wstring(L" ") + BUILD_CONFIGURATION;
		}

		int width = 640 + borderWidth*2;
		int height = 480 + titleHeight;

		if (parent)
		{
			RECT rect;

			GetWindowRect( parent, &rect );
			width = rect.right - rect.left;
			height = rect.bottom - rect.top;
		}

		hWnd = CreateWindow(		lpClassName,
									windowName.c_str(),
									parent ? WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS : WS_OVERLAPPEDWINDOW ,
									parent ? 0 : CW_USEDEFAULT,
									parent ? 0 : CW_USEDEFAULT,
									width,
									height,
									parent,
									NULL,
									hInstance,
									0 );
		if (!hWnd)
		{
			CRITICAL_MSG( "Failed to create window." );
			return -1;
		}

		// Initialise and start a new game
		ShowWindow( hWnd, nCmdShow );
		UpdateWindow( hWnd );

		if (parent)
		{
			SetFocus( hWnd );
		}


		if( !app.init( hInstance, hWnd ) )
		{
			DestroyWindow( hWnd );
			return FALSE;
		}

		// Standard game loop.
		while( !g_bAppQuit )
		{
			if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
			{
				// Check for a quit message
				if( msg.message == WM_QUIT )
					break;

#if SCALEFORM_IME
				if ( (msg.message == WM_KEYDOWN) || 
					(msg.message == WM_KEYUP) || 
					ImmIsUIMessage(NULL, msg.message, msg.wParam, msg.lParam) )
				{
					Scaleform::IME::handleIMMMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
				}
#endif

				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			else
			{
				// Play the game (check user input 
				// and update the window)
				if (!app.updateFrame(g_bActive))
				{
					::PostQuitMessage(0);
				}
			}
		}
	}
	catch (const App::InitError &)
	{
		if (hWnd)
		{
			DestroyWindow( hWnd );
		}
		return FALSE;
	}

	timeEndPeriod(1);

	Moo::fini();

#if defined(_DEBUG) && ENABLE_STACK_TRACKER
	DEBUG_MSG( "StackTracker: maximum stack depth achieved: %d.\n", StackTracker::getMaxStackPos() );
#endif
	//if exiting while running open automate we return 0 (even if singeltons issues cause us to return something else)
	if (BWOpenAutomate::s_doingExit)
	{
		DebugMsgHelper::logToFile( "Exiting automated test normally" );
		if (msg.wParam != 0) 
		{
			WARNING_MSG("Returning with 0 from main as open automate is calling exit");
			return 0;
		}
	}
    return msg.wParam;
}

static bool shuttingDown = false;


/**
 *	This function is called when WM_CLOSE or alt+F4 are received in order to close the 
 *	application.
 *
 *	@param hWnd	The window handle associated with the current windows message.
 *
*/
 void handleCloseRequest(HWND hWnd)
{
	shuttingDown = true;
	//Early call for mozilla fini as mozilla requires the window to exist in order to function correctly
	MozillaWebPageManager::s_fini(true);
	PostQuitMessage(0);
}


/**
 *	This function is the BigWorld implementation of WndProc which should be
 *	called by the WndProc registered when the application was started.
 *
 *	@param hWnd	The window handle associated with the current windows message.
 *	@param msg		The message passed to WndProc
 *	@param wParam	Additional message parameter passed to WndProc
 *	@param lParam	Additional message parameter passed to WndProc
 *
 *	@return	DefWindowProc will be called if the message is not otherwise
 *			handled.
 */
LRESULT BWWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	BW_GUARD;

	static bool s_inSizeMove = false;
	static RECT s_enterSizeMoveRect = { 0,0,0,0 };

	switch (msg)
	{
	case WM_CREATE:
		MsgBox::setDefaultParent( hWnd );
		break;

	// This is to ensure that the main app window will get
	// focus when embedded
	case WM_LBUTTONDOWN:
		SetFocus( hWnd );
		break;

	case WM_SETCURSOR:
		// From DirectX "ShowCursor" documentation, one should prevent Windows
		// from setting a Windows cursor for this window by doing this:
		if (Moo::rc().device() && !DebugMsgHelper::criticalMsgOccurs())
		{
			if (SimpleGUI::pInstance() != NULL && 
				SimpleGUI::instance().mouseCursor().automaticCursorManagement())
			{
				if (SimpleGUI::instance().mouseCursor().visible())
				{
					Moo::rc().device()->ShowCursor( TRUE );
				}
				else
				{
					Moo::rc().device()->ShowCursor( FALSE );
					SetCursor( NULL );
				}
			}

			return TRUE;
		}

		break;

	case WM_MOUSEMOVE:
		// If we're using a software cursor, then we need to move it.
		if (Moo::rc().device() && !Moo::rc().hardwareCursor())
		{
			POINT pt = { GET_X_LPARAM(lParam),
						 GET_Y_LPARAM(lParam) };

			ClientToScreen( hWnd, &pt );
			Moo::rc().device()->SetCursorPosition( pt.x, pt.y, 0 );
		}
		break;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = MIN_WINDOW_WIDTH;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = MIN_WINDOW_HEIGHT;
		break;

	case WM_PAINT:
		if (!g_bAppStarted)
		{
			RECT rect;
			GetClientRect( hWnd, &rect );
			HDC hDeviceContext = GetDC( hWnd );
			FillRect( hDeviceContext, &rect, (HBRUSH)( COLOR_WINDOW+1) );
			DrawText( hDeviceContext, L"Launching ...", -1, &rect,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE );
			ReleaseDC( hWnd, hDeviceContext );
		}
		break;

	case WM_MOVE:
		if (g_bActive && g_bAppStarted)
		{
			App::instance().moveWindow((int16)LOWORD(lParam), (int16)HIWORD(lParam));
		}
		break;

	case WM_SIZE:
		g_bActive = !( (SIZE_MAXHIDE == wParam) || (SIZE_MINIMIZED == wParam) || ( 0 == lParam ));

		if (g_bActive && g_bAppStarted && !s_inSizeMove)
		{
			App::instance().resizeWindow();
		}
		break;

	case WM_ENTERSIZEMOVE:
		{
			s_inSizeMove = true;
			GetClientRect(hWnd, &s_enterSizeMoveRect);
			break;
		}

	case WM_EXITSIZEMOVE:
		{
			s_inSizeMove = false;
			if (g_bActive && g_bAppStarted)
			{
				RECT r;
				GetClientRect(hWnd, &r);
				if (r.right != s_enterSizeMoveRect.right || 
					r.bottom != s_enterSizeMoveRect.bottom)
				{
					App::instance().resizeWindow();
				}
			}
			break;
		}

	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_CLOSE)
		{
			handleCloseRequest(hWnd);
		}
		break;

	case WM_CLOSE:
		{
			handleCloseRequest(hWnd);
			break;
		}		
	case WM_DESTROY:
		MsgBox::setDefaultParent( NULL );
		PostQuitMessage(0);
		break;

#if SCALEFORM_IME
	// Pass windows IME messages to GFx. 
	case WM_IME_SETCONTEXT:
		return DefWindowProc(hWnd, msg, wParam, 0); // In order to hide the system IME windows
	case WM_IME_STARTCOMPOSITION:
	case WM_IME_KEYDOWN:
	case WM_IME_COMPOSITION:
	case WM_IME_ENDCOMPOSITION:
	case WM_IME_COMPOSITIONFULL:
	case WM_IME_SELECT:
	case WM_IME_CONTROL:
	case WM_IME_NOTIFY:
	case WM_IME_CHAR:
	case WM_IME_KEYUP:
	case WM_INPUTLANGCHANGE:
		if (Scaleform::IME::handleIMEMessage( hWnd, msg, wParam, lParam ))
			return 0;
	case WM_CHAR:
		Scaleform::IME::handleIMEMessage( hWnd, msg, wParam, lParam );
#endif
	}

	// These messages need to be handled specifically based on whether we are embedded.
	// This is to allow mozilla integration to work properlly which needs to use WM_ACTIVATE, 
	// while when we're running inside a browser we need to use WM_SETFOCUS/WM_KILLFOCUS. This
	// is okay since we don't support mozilla integration when running inside a browser.
	if (GetParent(hWnd) == NULL)
	{
		if (msg == WM_ACTIVATE && !shuttingDown)
		{
			// LOWORD == active state, HIWORD == minimised state
			if ( LOWORD(wParam) != WA_INACTIVE && !HIWORD(wParam) )
			{
				App::handleSetFocus( true );
			}
			else
			{
				App::handleSetFocus( false );
			}
		}
	}
	else
	{
		if (msg == WM_SETFOCUS && !shuttingDown)
		{
			App::handleSetFocus( true );
		}
		else if (msg == WM_KILLFOCUS && !shuttingDown)
		{
			App::handleSetFocus( false );
		}
	}

	// Let the input system handle stuff
	LRESULT inputResult = 0;
	if ( InputDevices::handleWindowsMessage( hWnd, msg, wParam, lParam, inputResult ) )
	{
		return inputResult;
	}

	if ( msg == WM_SYSKEYUP && wParam == VK_MENU)
	{
		// Override these to stop some default windows behaviour (e.g. pressing alt to bring
		// up the system menu at the top left of the game window). Since this is specific to
		// the client it is here and not in InputDevices.
		return TRUE;
	}

	if (msg == WM_CONTEXTMENU)
	{
		return TRUE;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}


/**
 *	The function processes all outstanding windows messages and returns when
 *	there are non-remaining or a WM_QUIT is received.
 *
 *	@return Returns false if a WM_QUIT was encountered, otherwise true.
 */
bool BWProcessOutstandingMessages()
{
	BW_GUARD;
	MSG msg;

	while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
	{
		if( msg.message == WM_QUIT )
		{
			g_bAppQuit = true;
			return false;
		}

		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	return true;
}


/**
 * Splits the given string in the style of Windows command line parsing (i.e. the
 * same style which is passed into a C main function under win32).
 *
 * - The string is split by delimeters (default: whitespace/newlines), which are 
 *   ignored when between matching binding characters (default: ' or "). 
 * - \" escapes the quote.
 * - \\" escapes the slash only when inside a quoted block.
 *
 *	@param str Raw string to be parsed
 *  @param out Output array of strings, where the result will be placed.
 *             Note that the array will be appended to, and not cleared.
 *  @param delim Possible argument delimeters. Defaults to whitespace characters.
 *  @param bind Binding characters used to quote arguments. Defaults to ".
 *	@param escape Character used to escape binding characters. Defaults to \.
 *
 *	@return Returns the number of arguments appended to the output list
 */
size_t splitCommandArgs (const std::wstring& str, std::vector< std::string >& out, 
                         const std::wstring& delim = L" \t\r\n", 
						 const std::wstring& bind = L"\"",
						 wchar_t escape = L'\\')
{
    const size_t prevSize = out.size();

    std::wstring buf;
    wchar_t binding = 0;

    for(size_t i = 0; i < str.size(); ++i)
    {
		// Process \" (escaped binding char)
		if(str[i] == escape && i < str.size()-1)
		{
			size_t pos = bind.find_first_of( str[i+1] );
			if (pos != std::wstring::npos)
			{
				buf.push_back( bind[pos] );
				++i;
				continue;
			}
		}

		// Two modes: bound, or unbound.
		// bound = we are currently inside a quote, so process until we hit a non-quote
		if(binding)
        {
			// Process \\" (escaped slash-with-a-quote)
			if(str[i] == escape && i < str.size()-2 && str[i+1] == escape)
			{
				size_t pos = bind.find_first_of( str[i+2] );
				if (pos != std::wstring::npos)
				{
					buf.push_back( escape );
					++i;
					continue;
				}
			}

            if(str[i] == binding)
            {
                // Hit matching binding char, add buffer to output (always - empty strings included)
				out.push_back( std::string() );
				bw_wtoutf8( buf, out.back() );
                buf.clear();

                // Set unbound
                binding = 0;
            }
            else
			{
                // Not special, add char to buffer
                buf.push_back(str[i]);
			}
        }
        else
        {
            if(delim.find_first_of(str[i]) != std::wstring::npos)
            {
                // Hit a delimeter
                if(!buf.empty())
                {
                    // Add buffer to output (if nonempty - ignore empty strings when unbound)
					out.push_back( std::string() );
					bw_wtoutf8( buf, out.back() );
                    buf.clear();
                }
            }
            else if(bind.find_first_of(str[i]) != std::wstring::npos)
            {
                // Hit binding character
                if(!buf.empty())
                {
                    // Add buffer to output (if nonempty - ignore empty strings when unbound)
					out.push_back( std::string() );
					bw_wtoutf8( buf, out.back() );
                    buf.clear();
                }

                // Set bounding character
                binding = str[i];
            }
            else
			{
                // Not special, add char to buffer
                buf.push_back(str[i]);
			}
        }
    }

	if(!buf.empty())
	{
		out.push_back( std::string() );
		bw_wtoutf8( buf, out.back() );
	}

    return out.size() - prevSize;
}

static std::vector< std::string > g_commandLine;

/**
 *	This function processes the command line.
 *
 *	@return True if successful, otherwise false.
 */
int parseCommandLine( LPCTSTR str )
{
	BW_GUARD;
	MF_ASSERT( g_commandLine.empty() && "parseCommandLine called twice!" );

#if !BWCLIENT_AS_PYTHON_MODULE
	// __argv is a macro exposed by the MS compiler (together with __argc).
	g_commandLine.push_back( BWResource::appPath() );
#endif

	splitCommandArgs(str, g_commandLine);

	if (g_commandLine.empty())
	{
		ERROR_MSG( "winmain::parseCommandLine: No path given\n" );
		return FALSE;
	}

	// Always copy argv[0] into python args
	Script::g_scriptArgv[ Script::g_scriptArgc++ ] = const_cast<char*>(g_commandLine[0].c_str());

	// Build a C style array of characters for functions that use this signature
	const size_t MAX_ARGS = 20;
	const char * argv[ MAX_ARGS ];
	size_t argc = 0;

	if (g_commandLine.size() >= MAX_ARGS)
	{
		ERROR_MSG( "winmain::parseCommandLine: Too many arguments!!\n" );
		return FALSE;
	}

	while(argc < g_commandLine.size())
	{
		std::string& arg = g_commandLine[argc];
		
		argv[argc] = arg.c_str();

		if( arg == "--res" || arg == "-r" ||
			arg == "--options" || 
			arg == "--config" || arg == "-c" ||
			arg == "--script-arg" || arg == "-sa" &&
			argc < g_commandLine.size() - 1)
		{
			std::string& arg2 = g_commandLine[++argc];
			argv[argc] = arg2.c_str();
			
			if(arg == "--config" || arg == "-c")
			{
				configFilename = arg2;
			}
			else if(arg == "--script-arg" || arg == "-sa")
			{
				Script::g_scriptArgv[ Script::g_scriptArgc++ ] = const_cast<char*>(arg2.c_str());
			}
		}
		else if(arg == BWOpenAutomate::OPEN_AUTOMATE_ARGUMENT &&
			argc < g_commandLine.size() - 1)
		{
			DebugMsgHelper::automatedTest( true );

			char logLine[256];
			extern const wchar_t * APP_TITLE;
			bw_snprintf( logLine, sizeof( logLine ), "Automated test initiated, %ls, built %s", APP_TITLE, compileTimeString );
			DebugMsgHelper::logToFile( logLine );

			std::string& arg2 = g_commandLine[++argc];
			argv[argc] = arg2.c_str();
			Script::g_scriptArgv[ Script::g_scriptArgc++ ] = const_cast<char*>(arg.c_str());
			Script::g_scriptArgv[ Script::g_scriptArgc++ ] = const_cast<char*>(arg2.c_str());
			//support internal testing with the path of the testing file
			if (arg2 == BWOpenAutomate::OPEN_AUTOMATE_TEST_ARGUMENT) 
			{
				if (argc < g_commandLine.size()-1)
				{
					std::string& arg3 = g_commandLine[++argc];
					argv[argc] = arg3.c_str();
					DebugMsgHelper::logToFile( argv[argc] );
					Script::g_scriptArgv[ Script::g_scriptArgc++ ] = const_cast<char*>(arg3.c_str());
				}
			}
		}

		++argc;
	}

#if BWCLIENT_AS_PYTHON_MODULE
	BWResource::overrideAppDirectory(Script::getMainScriptPath());
#endif // BWCLIENT_AS_PYTHON_MODULE

	return BWResource::init( (int)argc, (const char **)argv )	? TRUE : FALSE;
}



// bw_winmain.cpp
