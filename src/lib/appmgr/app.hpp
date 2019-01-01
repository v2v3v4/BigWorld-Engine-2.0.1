/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef APP_HPP
#define APP_HPP

#define WIN32_LEAN_AND_MEAN

#include <list>
#include <windows.h>
#include <string>
#include "input_manager.hpp"
#include "scripted_module.hpp"
#include "cstdmf/concurrency.hpp"


/**
 *	This is a class that is used to represent the entire application.
 */
class App
{
public:
	App();
	~App();

	bool init( HINSTANCE hInstance, HWND hWndApp, HWND hwndGraphics,
		 bool ( *userInit )( HINSTANCE, HWND, HWND ) );
	void fini();
	void updateFrame( bool tick = true );
	// The default implementation waits to avoid a possible D3D call while the
	// present thread is still presenting. The wait loop has to call Sleep(0)
	// so it gives control to other threads (in this case, the present thread)
	virtual void onPresent()	{	while( presenting() ) Sleep(0); }
	void consumeInput();

	HWND hwndApp();

	// Windows messages
	void resizeWindow( );
	void handleSetFocus( bool focusState );

	// Application Tasks
	float	calculateFrameTime();
    void    pause(bool paused);
    bool    isPaused() const;

	bool	presenting() const	{	return presentThread_.isPresenting();	}
	float	fps() const	{	return presentThread_.fps();	}

private:
	// Properties
	HWND				hWndApp_;

	// Timing
	uint64				lastTime_;
    bool                paused_;

	// Input
	InputManager		inputHandler_;
	bool				bInputFocusLastFrame_;

	// General
	float				maxTimeDelta_;
	float				timeScale_;

	struct PresentThread : SimpleSemaphore, SimpleThread
	{
		App* app_;
		volatile bool presenting_;
		volatile float fps_;
		static void ThreadFunc( LPVOID param );
		void presentLoop();
		void present();
		bool isPresenting() const;
		float fps() const;
		void stop();
		void init( App* app );
		
		PresentThread();
	}
	presentThread_;
};

#ifdef CODE_INLINE
	#include "app.ipp"
#endif


#endif // APP_HPP
