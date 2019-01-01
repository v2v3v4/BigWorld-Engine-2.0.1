/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PE_SHELL_HPP
#define PE_SHELL_HPP

#include "fwd.hpp"
#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "common/tools_camera.hpp"

class ChunkSpace;
typedef SmartPointer<ChunkSpace> ChunkSpacePtr;

/*
 *  Interface to the BigWorld message handler
 */
struct PeShellDebugMessageCallback : public DebugMessageCallback
{
    PeShellDebugMessageCallback()
    {
    }

    ~PeShellDebugMessageCallback()
    {
    }

    /*virtual*/ bool 
    handleMessage
    ( 
        int             componentPriority,
        int             messagePriority, 
        char            const *format, 
        va_list         argPtr
    );
};

/**
 *  This class adds solution specific routines to what already exists in
 *  appmgr/app.cpp
 */
class PeShell
{
public:
    PeShell();

    ~PeShell();

    static PeShell & instance();

    static bool hasInstance();

    static bool 
    initApp
    ( 
        HINSTANCE       hInstance, 
        HWND            hWndApp, 
        HWND            hWndGraphics 
    ); 
    
    bool init
    (
        HINSTANCE       hInstance, 
        HWND            hWndApp, 
        HWND            hWndGraphics 
    );

    void fini();

    HINSTANCE & hInstance();

    HWND &hWndApp();

    HWND &hWndGraphics();

    RompHarness &romp();

    ToolsCamera &camera();

	Floor &floor();

    POINT currentCursorPosition() const;

private:
    friend std::ostream& operator<<(std::ostream&, const PeShell&);

    friend struct PeShellDebugMessageCallback;

    static bool 
    messageHandler
    (
        int             componentPriority, 
        int             messagePriority,
        char            const *format, 
        va_list         argPtr 
    );

    bool initGraphics();

    void finiGraphics();

    bool initScripts();

    void finiScripts();

    bool initConsoles();

    bool initErrorHandling();

    bool initRomp();

    bool initCamera();

	bool initSound();

    PeShellDebugMessageCallback debugMessageCallback_;

private:
    PeShell(PeShell const&);                // not permitted
    PeShell& operator=(PeShell const &);    // not permitted

private:    
    static PeShell      *s_instance_;       // the instance of this class (there should be only one!)
	bool				inited_;
    HINSTANCE           hInstance_;         // the current instance of the application
    HWND                hWndApp_;           // application window
    HWND                hWndGraphics_;      // 3D window
    RompHarness         *romp_;
    ToolsCameraPtr		camera_;
	Floor*				floor_;
	ChunkSpacePtr		space_;
};

#endif // PE_SHELL_HPP
