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

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include <windows.h>
#include <windowsx.h>
#include <winnls.h>

#include <objbase.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "input.hpp"
#include "vk_map.hpp"
#include "ime.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/dogwatch.hpp"
#include "cstdmf/string_utils.hpp"

#include "scaleform/config.hpp"

DECLARE_DEBUG_COMPONENT2( "UI", 0 )

#ifndef CODE_INLINE
#include "input.ipp"
#endif

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif
#ifndef HID_USAGE_GENERIC_KEYBOARD
#define HID_USAGE_GENERIC_KEYBOARD     ((USHORT) 0x06)
#endif

#if !defined(EDITOR_ENABLED) && (!defined(SCALEFORM_IME) || (!SCALEFORM_IME))
#define ENABLE_BW_IME 1
#else
#define ENABLE_BW_IME 0
#endif

namespace {

	inline Vector2 clientToClip( HWND hWnd, const POINT& pt )
	{
		RECT r;
		GetClientRect( hWnd, &r );

		return Vector2(  (float(pt.x)/float(r.right)) * 2.0f - 1.0f, 
						-((float(pt.y)/float(r.bottom)) * 2.0f - 1.0f) );
	}

	inline Vector2 currentCursorPos( HWND hWnd )
	{
		POINT cursorPos;
		GetCursorPos( &cursorPos );
		ScreenToClient( hWnd, &cursorPos );

		return clientToClip( hWnd, cursorPos );
	}

	inline Vector2 lastMessageCursorPos( HWND hWnd )
	{
		DWORD dwCursorPos = GetMessagePos();
		POINT msgCursorPos = { GET_X_LPARAM( dwCursorPos ), GET_Y_LPARAM( dwCursorPos ) };
		ScreenToClient( hWnd, &msgCursorPos );

		return clientToClip( hWnd, msgCursorPos );
	}
}

// -----------------------------------------------------------------------------
// Section: Data declaration
// -----------------------------------------------------------------------------

/// device input library Singleton
BW_SINGLETON_STORAGE( InputDevices )


const int DIRECT_INPUT_AXIS_MAX = 1000;
const int DIRECT_INPUT_AXIS_DEAD_ZONE = 150;

bool InputDevices::focus_;

/// This static creates an ascending ID for events.
/*static*/ uint32 InputEvent::s_seqId_ = 0;


// -----------------------------------------------------------------------------
// Section: InputDevices
// -----------------------------------------------------------------------------

static const int JOYSTICK_BUFFER_SIZE = 32;

/**
 *	InputDevices::InputDevices:
 */
InputDevices::InputDevices() : 
	hWnd_( NULL ),
	pDirectInput_( NULL ),
	languageChanged_( false ),
	processingEvents_( false ),
	consumeUpToSeqId_( 0 ),
	lostData_( NO_DATA_LOST )
{
	IME::init();
}


/**
 *	Destructor
 */
InputDevices::~InputDevices()
{
	BW_GUARD;

	IME::fini();

	// Release our DirectInput object.
	if (pDirectInput_ != NULL)
	{
		pDirectInput_->Release();
		pDirectInput_ = NULL;
	}
}


/**
 * InputDevices::privateInit:
 */
bool InputDevices::privateInit( void * _hInst, void * _hWnd )
{
	BW_GUARD;
	HINSTANCE hInst = static_cast< HINSTANCE >( _hInst );
	hWnd_ = static_cast< HWND >( _hWnd );

	HRESULT hr;

	// hrm, _hInst is being passed in as null from borland, and dinput doesn't
	// like that
#ifdef EDITOR_ENABLED
	hInst = (HINSTANCE) GetModuleHandle( NULL );
#endif

	// Register with the DirectInput subsystem and get a pointer to a
	// IDirectInput interface we can use.
    hr = DirectInput8Create( hInst,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(LPVOID*)&pDirectInput_,
		NULL );
	if (SUCCEEDED(hr)) 
	{
		// ****** Joystick initialisation. ******
		if (this->joystick_.init( pDirectInput_, _hWnd ))
		{
			INFO_MSG( "InputDevices::InputDevices: Joystick initialised\n" );
		}
		else
		{
			INFO_MSG( "InputDevices::InputDevices: Joystick failed to initialise\n" );
		}
	}
	else
	{
		ERROR_MSG( "Failed to create DirectInput device (hr=0x%x). There will be no joystick support.\n", hr );
	}

	// Initialise raw input for mouse and keyboard.
	// We do this in two calls to work around a problem with PerfHUD.
	RAWINPUTDEVICE rawDevice;
	memset( &rawDevice, 0, sizeof(rawDevice) );

	rawDevice.usUsagePage = HID_USAGE_PAGE_GENERIC;
	rawDevice.usUsage = HID_USAGE_GENERIC_KEYBOARD;
	rawDevice.dwFlags = 0;
	rawDevice.hwndTarget = hWnd_;

	if ( !RegisterRawInputDevices(&rawDevice, 1, sizeof(rawDevice)) )
	{
		ERROR_MSG( "Failed to register keyboard raw input device (GetLastError=%d).\n", GetLastError() );
		return false;
	}

	rawDevice.usUsagePage = HID_USAGE_PAGE_GENERIC;
	rawDevice.usUsage = HID_USAGE_GENERIC_MOUSE;
	rawDevice.dwFlags = 0;
	rawDevice.hwndTarget = hWnd_;

	if ( !RegisterRawInputDevices(&rawDevice, 1, sizeof(rawDevice)) )
	{
		ERROR_MSG( "Failed to register mouse raw input device (GetLastError=%d).\n", GetLastError() );
		return false;
	}

#if ENABLE_BW_IME
	if ( !IME::instance().initIME( hWnd_ ) )	
	{
		WARNING_MSG( "Failed to initialise IME.\n" );
	}
#endif

	return true;
}


/**
 *	This method processes all device events since it was last called. It asks
 *	the input handler to handle each of these events.
 */
bool InputDevices::privateProcessEvents( InputHandler & handler )
{
	BW_GUARD;

	if (!focus_) return true;

	// Prevent re-entry
	if (processingEvents_)
	{
		ERROR_MSG( "InputDevices::privateProcessEvents: "
					"Tried to re-enter the method.\n" );
		return true;
	}
	processingEvents_ = true;

	// Update the Joystick state when this is called.
	this->joystick_.update();

	bool jbLostData = false;
	this->joystick().processEvents( handler, keyStates_, &jbLostData );
	if ( jbLostData )
		lostData_ |= JOY_DATA_LOST;

	static DogWatch watchHandle( "Event Handlers" );

	{ // DogWatch scope
	ScopedDogWatch watcher( watchHandle );

	// Sometimes new events can be added while handling events in the queue, so
	// we need to iterate by index, and avoid iterators.
	for (size_t i = 0; i < eventQueue_.size(); ++i)
	{
		// Use a copy of the event, in case the vector changes.
		InputEvent event = eventQueue_[i];

		if (event.seqId_ <= consumeUpToSeqId_)
		{
			// skip this event, we've been told to consume it.
			continue;
		}

		switch( event.type_ )
		{
		case InputEvent::KEY:
			{
				keyStates_.setKeyState( event.key_.key(), event.key_.repeatCount() );
				handler.handleKeyEvent( event.key_ );
				break;
			}

		case InputEvent::MOUSE:
			handler.handleMouseEvent( event.mouse_ );
			break;

		case InputEvent::AXIS:
			handler.handleAxisEvent( event.axis_ );
			break;

		default:
			break;
		}
	}

	eventQueue_.resize(0);
	consumeUpToSeqId_ = 0;

#if ENABLE_BW_IME
	IME::instance().update();
	IME::instance().processEvents( handler );
#endif

	if (languageChanged_)
	{
		handler.handleInputLangChangeEvent();
		languageChanged_ = false;

		IME::instance().onInputLangChange();
	}

	} // DogWatch scope

	//handle lost data
	if (lostData_ != NO_DATA_LOST)
		handleLostData( handler, lostData_ );

	for (uint i = 0; i < gVirtualKeyboards.size(); i++)
	{
		KeyboardDevice * pKB = gVirtualKeyboards[i];
		pKB->update();

		KeyEvent event;
		while (pKB->next( event ))
		{
			keyStates_.updateKey( event.key(), event.isKeyDown() );
			handler.handleKeyEvent( event );
		}
	}

	processingEvents_ = false;
	return true;
}

/**
 *	This is called by the application's window proc in order to allow the input system to
 *	to handle input related messages.
 */
bool InputDevices::handleWindowsMessagePrivate( HWND hWnd, UINT msg, WPARAM& wParam, 
											    LPARAM& lParam, LRESULT& result )
{
	BW_GUARD;

	result = 0;
	bool handled = false;

	switch (msg)
	{
	case WM_INPUTLANGCHANGE:
		languageChanged_ = true;
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			// Ignore alt+tabs/escape
			if ( msg == WM_SYSKEYDOWN &&
				 (wParam == VK_TAB || wParam == VK_ESCAPE) )
			{
				break;
			}

			if ( wParam != VK_SHIFT && wParam != VK_PROCESSKEY) // VK_PROCESSKEY == IME sunk key event.
			{
				bool extendedBit = lParam & (1<<24) ? true : false;
				KeyCode::Key key = VKMap::fromVKey( (USHORT)wParam, extendedBit );

				pushKeyEventWin32( key, msg, modifiers(), lastMessageCursorPos( hWnd ) );
			}
			break;
		}
	case WM_CHAR:
	case WM_SYSCHAR:
		{
			wchar_t utf16chr[2] = { LOWORD(wParam), HIWORD(wParam) };

			// Attach to the most recent event, which works since WM_CHAR is posted immediately after
			// the key event that caused it
			if (!eventQueue_.empty())
			{
				eventQueue_.back().key_.appendChar( utf16chr );
			}
			else
			{
				// Hopefully this never happens, but shove the character in anyway using KEY_NONE.
				WARNING_MSG( "InputDevices: Received CHAR message 0x%x with no corresponding key press (char=0x%x).\n",
					msg, wParam );
				pushKeyEvent( KeyCode::KEY_NONE, 0, utf16chr, modifiers(), lastMessageCursorPos( hWnd ) );				
			}

			handled = true;
			break;
		}

	case WM_INPUT:
		{
			UINT dwSize = 0;
			RAWINPUT rid;

			if( GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER)) != 0)
			{
				ERROR_MSG("InputDevices::WM_INPUT: GetRawInputData failed to get buffer size.\n");
				break;
			}

			if (dwSize > sizeof(rid))
			{
				// If this error occurs, the client needs to be rebuilt using a newer version of the Platform SDK.
				ERROR_MSG("GetRawInputData needs size %d which is greater than sizeof(RAWINPUT)==%d\n.", dwSize, sizeof(RAWINPUT));
				MF_ASSERT(!"GetRawInputData buffer size mismatch.\n");
				break;
			}

			UINT res = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &rid, &dwSize, sizeof(RAWINPUTHEADER));
			if (res != dwSize)
			{
				ERROR_MSG("InputDevices::WM_INPUT: GetRawInputData failed to get data (GetLastError: %d)\n", GetLastError());
				break;
			}

			if (rid.header.dwType == RIM_TYPEKEYBOARD)
			{
				RAWKEYBOARD* rkb = &rid.data.keyboard;

				// Use raw input for shift so we can differentiate between the two shifts.
				if ( rkb->VKey == VK_SHIFT || rkb->VKey == VK_SNAPSHOT )
				{				
					pushKeyEventWin32( VKMap::fromRawKey( rkb ), rkb->Message, modifiers(), lastMessageCursorPos( hWnd ) );
				}
			}
			else if (rid.header.dwType == RIM_TYPEMOUSE)
			{
				RAWMOUSE* rm = &rid.data.mouse;

				/*
				DWORD dwCursorPos = GetMessagePos();
				POINT msgCursorPos = { GET_X_LPARAM( dwCursorPos ), GET_Y_LPARAM( dwCursorPos ) };
				ScreenToClient(hWnd, &msgCursorPos);

				POINT cursorPos;
				GetCursorPos( &cursorPos );
				ScreenToClient( hWnd, &cursorPos );

				INFO_MSG("RIM_TYPEMOUSE: usFlags: %d, usButtonFlags: %d, usButtonData: %d, ulRawButtons: %d, lLastX: %d, lLastY: %d, ulExtraInformation: %d, GetCursorPos: (%d,%d), GetMessagePos: (%d,%d)\n", 
							rm->usFlags,
							rm->usButtonFlags,
							rm->usButtonData,
							rm->ulRawButtons,
							rm->lLastX,
							rm->lLastY,
							rm->ulExtraInformation,
							cursorPos.x, cursorPos.y,
							msgCursorPos.x, msgCursorPos.y );
				*/
				// Mouse movement
				long dz = rm->usButtonFlags & RI_MOUSE_WHEEL 
							? long(SHORT(rm->usButtonData)) : 0;

				if (rm->usFlags == MOUSE_MOVE_RELATIVE)
				{
					if ( rm->lLastX != 0 || rm->lLastY != 0 || dz != 0 )
					{
						pushMouseEvent( rm->lLastX, rm->lLastY, dz, currentCursorPos( hWnd ) );
					}
				}
				else
				{
					static bool warningPrinted = false;
					if (!warningPrinted)
					{
						ERROR_MSG( "Non-relative mouse devices are not fully "
							"supported (e.g. pen tablets, touch screens). "
							"Some functionality may be missing while using this device.\n" );
						warningPrinted = true;
					}
				}
			}

			result = 1;
			handled = true;
			break;
		}

	case WM_LBUTTONDOWN:
		{
			pushMouseButtonEvent( KeyCode::KEY_LEFTMOUSE, true, modifiers(), lastMessageCursorPos( hWnd ) );			
			break;
		}

	case WM_LBUTTONUP:
		{
			pushMouseButtonEvent( KeyCode::KEY_LEFTMOUSE, false, modifiers(), lastMessageCursorPos( hWnd ) );
			break;
		}

	case WM_RBUTTONDOWN:
		{
			pushMouseButtonEvent( KeyCode::KEY_RIGHTMOUSE, true, modifiers(), lastMessageCursorPos( hWnd ) );
			break;
		}
	case WM_RBUTTONUP:
		{
			pushMouseButtonEvent( KeyCode::KEY_RIGHTMOUSE, false, modifiers(), lastMessageCursorPos( hWnd ) );
			break;
		}

	case WM_MBUTTONDOWN:
		{
			pushMouseButtonEvent( KeyCode::KEY_MIDDLEMOUSE, true, modifiers(), lastMessageCursorPos( hWnd ) );
			break;
		}
	case WM_MBUTTONUP:
		{
			pushMouseButtonEvent( KeyCode::KEY_MIDDLEMOUSE, false, modifiers(), lastMessageCursorPos( hWnd ) );
			break;
		}

	case WM_XBUTTONDOWN:
		{
			KeyCode::Key key = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 
									KeyCode::KEY_MOUSE4 : KeyCode::KEY_MOUSE5;

			pushMouseButtonEvent( key, true, modifiers(), lastMessageCursorPos( hWnd ) );
			break;
		}

	case WM_XBUTTONUP:
		{
			KeyCode::Key key = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 
									KeyCode::KEY_MOUSE4 : KeyCode::KEY_MOUSE5;

			pushMouseButtonEvent( key, false, modifiers(), lastMessageCursorPos( hWnd ) );
			break;
		}

	case WM_MOUSEMOVE:
		{
			// Find the most recent mouse move event and attach our position to it.
			for ( EventQueue::reverse_iterator it = eventQueue_.rbegin(); 
				it != eventQueue_.rend(); it++ )
			{
				InputEvent& event = *it;
				if ( event.type_ == InputEvent::MOUSE )
				{
					POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
					Vector2 cursorPos = clientToClip( hWnd, pt );

					//DEBUG_MSG( "WM_MOUSEMOVE attached: x=%d, y=%d\n", pt.x, pt.y );

					MouseEvent& mevent = event.mouse_;
					mevent.fill( mevent.dx(), mevent.dy(), mevent.dz(), cursorPos );
				}
			}

			break;
		}
	default:
		break;
	}

#if ENABLE_BW_IME
	if(!handled)
	{
		handled = IME::instance().handleWindowsMessage( hWnd, msg, wParam, lParam, result );
	}
#endif

	return handled;
}

/**
 *	Captures the Windows cursor if any of the mouse buttons are
 *	currently pressed, and relases it if none are pressed.
 */
void InputDevices::updateCursorCapture( HWND hWnd )
{
	bool mouseDown = false;

	for( int i = KeyCode::KEY_MINIMUM_MOUSE; i < KeyCode::KEY_MAXIMUM_MOUSE; i++ )
	{
		if ( keyStatesInternal_.isKeyDown( (KeyCode::Key)i ) )
		{
			mouseDown = true;
			break;
		}
	}

	if (mouseDown)
	{
		SetCapture( hWnd );
	}
	else
	{
		ReleaseCapture();
	}
}

/**
 *	Notifies the input system when the input focus changes to or from
 *	the input window.
 */
void InputDevices::setFocus( bool state, InputHandler * handler )
{
	bool losingFocus = (state == false && focus_ == true);
	focus_ = state;
	if (losingFocus)
	{
		//DEBUG_MSG( "InputDevices losing focus.\n" );

		//We are losing focus, so we don't want the currently
		//held down keys to remain down.  Especially if the key
		//up event is going to be lost to another application.

		//Send all down keys an up event
		if (handler != NULL)
		{
			for (int32 i=0; i < KeyCode::NUM_KEYS; i++)
			{
				KeyCode::Key k = (KeyCode::Key)i;
				if (pInstance())
				{
					if (keyStates().isKeyDown( k ))
					{
						KeyEvent ke;
						ke.fill( k, -1, 0, currentCursorPos( instance().hWnd_ ) );
						keyStates().updateKey( k, false );
						handler->handleKeyEvent( ke );
					}
				}
			}
		}

		if (pInstance())
		{
			//Clear anything in the event queue, because if it dodn't get pumped
			//out last frame, we don't want to know about it when we regain focus.
			instance().eventQueue_.resize(0);

			//Now make sure the keyboard state has been reset.
			instance().resetKeyStates();
		}

		ReleaseCapture();
	}	
}


/**
 *	This method is called if DirectInput encountered buffer
 *	overflow or lost data, and button events were lost.
 *
 *	We get the current state of all buttons, and compare them to
 *	our presumed state.  If there is any difference, then create
 *	imaginary events.
 *
 *	Note that while these events will be delivered out of order,
 *	vital key up events that were missed will be delivered, saving
 *	the game from untenable positions
 */
void InputDevices::handleLostData( InputHandler & handler, int mask )
{
	BW_GUARD;
	HRESULT hr;


	//process any lost joystick button state
	if ( (mask & JOY_DATA_LOST) && joystick_.pDIJoystick() )
	{
		DIJOYSTATE joyState;
		ZeroMemory( &joyState, sizeof( joyState ) );

		hr = joystick_.pDIJoystick()->GetDeviceState( sizeof( joyState ), &joyState );

		if ( SUCCEEDED( hr ) )
		{
			//success.  iterate through valid joystick codes and check the state
			for ( int k = KeyCode::KEY_MINIMUM_JOY;
					k != KeyCode::KEY_MAXIMUM_JOY;
					k++ )
			{
				//success.  iterate through valid key codes and check the state
				for ( int k = KeyCode::KEY_MINIMUM_JOY;
						k != KeyCode::KEY_MAXIMUM_JOY;
						k++ )
				{
					KeyEvent event;
					event.fill( static_cast<KeyCode::Key>(k), 
								(joyState.rgbButtons[ k - KeyCode::KEY_MINIMUM_JOY ] & 0x80) ? true : false,
								this->modifiers(), currentCursorPos( hWnd_ ) );

					//pass event to handler if there is a mismatch between
					//immediate device state and our recorded state
					if ( event.isKeyDown() != (keyStates_.isKeyDown( event.key() ) ? true : false ) )
					{
						keyStates_.setKeyStateNoRepeat( event.key(), event.isKeyDown() );
						handler.handleKeyEvent( event );
					}
				}
			}
			lostData_ &= ~JOY_DATA_LOST;
		}
		else
		{
			DEBUG_MSG( "InputDevices::handleLostData::GetDeviceState[joystick] failed  %lx\n", hr );
		}
	}
}

/**
 *	Insert a key event into the queue, based on the given windows key event.
 */
void InputDevices::pushKeyEventWin32( KeyCode::Key key, USHORT msg, 
									  uint32 mods, const Vector2& cursorPos )
{
	bool isDown = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);

	if (!isDown && !keyStatesInternal_.isKeyDown(key))
		return;

	int32 state = keyStatesInternal_.updateKey( key, isDown );	
	pushKeyEvent( key, state, mods, cursorPos );	
}

/**
 *	Insert a key event into the queue.
 */
void InputDevices::pushKeyEvent( KeyCode::Key key, int32 state, 
								 uint32 mods, const Vector2& cursorPos )
{
	InputEvent event;
	event.type_ = InputEvent::KEY;
	event.key_.fill( key, state, mods, cursorPos );
	eventQueue_.push_back( event );
}

/**
 *	Insert a key event into the queue, with an associated character string.
 */
void InputDevices::pushKeyEvent( KeyCode::Key key, int32 state, 
								 const wchar_t *ch, uint32 mods, 
								 const Vector2& cursorPos )
{
	InputEvent event;
	event.type_ = InputEvent::KEY;
	event.key_.fill( key, state, ch, mods, cursorPos );
	eventQueue_.push_back( event );
}

/**
 *	Insert a mouse button event into the queue.
 */
void InputDevices::pushMouseButtonEvent( KeyCode::Key key, bool isDown, uint32 mods,
										 const Vector2& cursorPos)
{
	if (!isDown && !keyStatesInternal_.isKeyDown(key))
		return;

	int32 state = keyStatesInternal_.updateKey( key, isDown );	
	pushKeyEvent( key, state, mods, cursorPos );
	updateCursorCapture( hWnd_ );
}

/**
 *	Insert a mouse event into the queue. If accumulate is true and the previous event
 *	was also a mouse event, then it accumulates the new mouse movement onto the previous
 *	mouse movement.
 */
void InputDevices::pushMouseEvent( long dx, long dy, long dz, 
								const Vector2& cursorPos, bool accumulate )
{
	if ( !accumulate || 
		  eventQueue_.empty() || 
		  eventQueue_.back().type_ != InputEvent::MOUSE )
	{
		InputEvent event;
		event.type_ = InputEvent::MOUSE;
		event.mouse_.fill( dx, dy, dz, cursorPos );
		eventQueue_.push_back( event );
	}
	else
	{
		MouseEvent& mevent = eventQueue_.back().mouse_;
		mevent.fill( mevent.dx() + dx, mevent.dy() + dy, mevent.dz() + dz, cursorPos );
	}
}

/**
 *	Insert an IME character event into the event queue. This gets posted off to
 *	the input handler as a KeyEvent with the key code set to KEY_IME_CHAR.
 */
/*static*/ void InputDevices::pushIMECharEvent( wchar_t* chs )
{
	if (InputDevices::pInstance())
	{
		HWND hWnd = IME::instance().hWnd();
		InputDevices::instance().pushKeyEvent( KeyCode::KEY_IME_CHAR, 0, chs, 0, lastMessageCursorPos(hWnd) );
	}
}


// -----------------------------------------------------------------------------
// Section: Joystick
// -----------------------------------------------------------------------------

/**
 *	Structure to hold the Direct Input callback objects.
 */
struct EnumJoysticksCallbackData
{
	IDirectInputDevice8 ** ppDIJoystick;
	IDirectInput8 * pDirectInput;
};


/*
 * Called once for each enumerated joystick. If we find one, create a device
 * interface on it so we can play with it.
 */
BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE * pdidInstance,
								void * pData )
{
	BW_GUARD;
	EnumJoysticksCallbackData * pCallbackData =
		reinterpret_cast<EnumJoysticksCallbackData *>( pData );

	// Obtain an interface to the enumerated joystick.
	HRESULT hr = pCallbackData->pDirectInput->CreateDevice(
			GUID_Joystick,
			pCallbackData->ppDIJoystick,
			NULL );

	// If it failed, then we can't use this joystick. (Maybe the user unplugged
	// it while we were in the middle of enumerating it.)

	if( FAILED(hr) )
		return DIENUM_CONTINUE;


	// Stop enumeration. Note: we're just taking the first joystick we get. You
	// could store all the enumerated joysticks and let the user pick.
	return DIENUM_STOP;
}




/*
 *	Callback function for enumerating the axes on a joystick.
 */
BOOL CALLBACK EnumAxesCallback( const DIDEVICEOBJECTINSTANCE * pdidoi,
							void * pJoystickAsVoid )
{
	BW_GUARD;
	Joystick * pJoystick = reinterpret_cast<Joystick *>( pJoystickAsVoid );

	DIPROPRANGE diprg;
	diprg.diph.dwSize       = sizeof(DIPROPRANGE);
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprg.diph.dwHow        = DIPH_BYOFFSET;
	diprg.diph.dwObj        = pdidoi->dwOfs; // Specify the enumerated axis
	diprg.lMin              = -DIRECT_INPUT_AXIS_MAX;
	diprg.lMax              = +DIRECT_INPUT_AXIS_MAX;

	// Set the range for the axis
	if( FAILED( pJoystick->pDIJoystick()->
		SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
	{
		return DIENUM_STOP;
	}

	// Set the UI to reflect what axes the joystick supports
	AxisEvent::Axis amap = AxisEvent::NUM_AXES;
	switch( pdidoi->dwOfs )
	{
		// these are PlayStation mappings
		case DIJOFS_X:			amap = AxisEvent::AXIS_LX;		break;
		case DIJOFS_Y:			amap = AxisEvent::AXIS_LY;		break;
		case DIJOFS_Z:			amap = AxisEvent::AXIS_RX;		break;
//		case DIJOFS_RX:			amap = AxisEvent::AXIS_;		break;
//		case DIJOFS_RY:			amap = AxisEvent::AXIS_;		break;
		case DIJOFS_RZ:			amap = AxisEvent::AXIS_RY;		break;
//		case DIJOFS_SLIDER(0):	amap = AxisEvent::AXIS_;		break;
//		case DIJOFS_SLIDER(1):	amap = AxisEvent::AXIS_;		break;
	}

	if (amap != AxisEvent::NUM_AXES)
		pJoystick->getAxis( amap ).enabled( true );

	return DIENUM_CONTINUE;
}



/**
 *	This method initialises the Joystick.
 */
bool Joystick::init( IDirectInput8 * pDirectInput,
					void * hWnd )
{
	BW_GUARD;
	EnumJoysticksCallbackData callbackData =
	{
		&pDIJoystick_,
		pDirectInput
	};

	hWnd_ = (HWND)hWnd;

	// Look for a simple joystick we can use for this sample program.
	if ( FAILED( pDirectInput->EnumDevices( DI8DEVCLASS_GAMECTRL,
		EnumJoysticksCallback,
		&callbackData,
		DIEDFL_ATTACHEDONLY ) ) )
	{
		return false;
	}

	// Make sure we got a joystick
	if( NULL == pDIJoystick_ )
	{
		DEBUG_MSG( "Joystick::init: Joystick not found\n" );

		return false;
	}

	// Set the data format to "simple joystick" - a predefined data format
	//
	// A data format specifies which controls on a device we are interested in,
	// and how they should be reported. This tells DInput that we will be
	// passing a DIJOYSTATE structure to IDirectInputDevice::GetDeviceState().

	if ( FAILED( pDIJoystick_->SetDataFormat( &c_dfDIJoystick ) ) )
	{
		return false;
	}

	// Set the cooperative level to let DInput know how this device should
	// interact with the system and with other DInput applications.

	if ( FAILED( pDIJoystick_->SetCooperativeLevel( hWnd_,
						DISCL_EXCLUSIVE|DISCL_FOREGROUND ) ) )
	{
		return false;
	}

	// IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
	//
	// DirectInput uses unbuffered I/O (buffer size = 0) by default.
	// If you want to read buffered data, you need to set a nonzero
	// buffer size.
	//
	// Set the buffer size to KEYBOARD_BUFFER_SIZE elements.
	//
	// The buffer size is a DWORD property associated with the device.
	DIPROPDWORD dipdw;

	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = JOYSTICK_BUFFER_SIZE;

	if (FAILED( pDIJoystick_->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
	{
		return false;
	}

	// Determine the capabilities of the device.

	DIDEVCAPS diDevCaps;
	diDevCaps.dwSize = sizeof( DIDEVCAPS );
	if ( SUCCEEDED( pDIJoystick_->GetCapabilities( &diDevCaps ) ) )
	{
		if ( diDevCaps.dwFlags & DIDC_POLLEDDATAFORMAT )
		{
			DEBUG_MSG( "Joystick::init: Polled data format\n" );
		}
		else
		{
			DEBUG_MSG( "Joystick::init: Not Polled data format\n" );
		}

		if ( diDevCaps.dwFlags & DIDC_POLLEDDEVICE )
		{
			DEBUG_MSG( "Joystick::init: Polled device\n" );
		}
		else
		{
			DEBUG_MSG( "Joystick::init: Not Polled device\n" );
		}
	}
	else
	{
		DEBUG_MSG( "Joystick::init: Did not get capabilities\n" );
	}

	// Enumerate the axes of the joyctick and set the range of each axis. Note:
	// we could just use the defaults, but we're just trying to show an example
	// of enumerating device objects (axes, buttons, etc.).

	pDIJoystick_->EnumObjects( EnumAxesCallback, (VOID*)this, DIDFT_AXIS );

	return true;
}


/*
 *	Simple helper method to convert from the joystick axis coordinates that
 *	DirectInput returns to a float in the range [-1, 1].
 */
static inline float scaleFromDIToUnit( int value )
{
	BW_GUARD;
	// We want to do the following, where the range mappings are linear.
	//
	// [-DIRECT_INPUT_AXIS_MAX,			DIRECT_INPUT_AXIS_DEAD_ZONE	] -> [-1, 0]
	// [-DIRECT_INPUT_AXIS_DEAD_ZONE,	DIRECT_INPUT_AXIS_DEAD_ZONE	] -> [ 0, 0]
	// [DIRECT_INPUT_AXIS_DEAD_ZONE,	DIRECT_INPUT_AXIS_MAX		] -> [ 0, 1]

	bool isNegative = false;

	if (value < 0)
	{
		value = -value;
		isNegative = true;
	}

	value -= DIRECT_INPUT_AXIS_DEAD_ZONE;


	if (value < 0)
	{
		value = 0;
	}

	float floatValue = (float)value/
		(float)(DIRECT_INPUT_AXIS_MAX - DIRECT_INPUT_AXIS_DEAD_ZONE);

	return isNegative ? -floatValue : floatValue;
}



/**
 *	This method updates this object from a joystick device.
 */
bool Joystick::updateFromJoystickDevice()
{
	BW_GUARD;
	HRESULT     hr;
	DIJOYSTATE  js;           // DInput joystick state

	if ( pDIJoystick_ )
	{
		const int MAX_ATTEMPTS = 10;
		int attempts = 0;

		do
		{
			// Poll the device to read the current state
			hr = pDIJoystick_->Poll();

			if ( SUCCEEDED( hr ) )
			{
				// Get the input's device state
				hr = pDIJoystick_->GetDeviceState( sizeof(DIJOYSTATE), &js );
			}

			if (hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST)
			{
				// DInput is telling us that the input stream has been
				// interrupted. We aren't tracking any state between polls, so
				// we don't have any special reset that needs to be done. We
				// just re-acquire and try again.

				HRESULT localHR = pDIJoystick_->Acquire();

				if (FAILED( localHR ))
				{
					// DEBUG_MSG( "Joystick::updateFromJoystickDevice: Acquire failed\n" );
					return false;
				}
			}
		}
		while ((hr != DI_OK) && (++attempts < MAX_ATTEMPTS));

		if( FAILED(hr) )
			return false;

		// PlayStation Pelican adapter settings
		// We use a math-like not screen-like coordinate system here
		this->getAxis( AxisEvent::AXIS_LX ).value( scaleFromDIToUnit( js.lX ) );
		this->getAxis( AxisEvent::AXIS_LY ).value(-scaleFromDIToUnit( js.lY ) );

		this->getAxis( AxisEvent::AXIS_RX ).value( scaleFromDIToUnit( js.lZ ) );
		this->getAxis( AxisEvent::AXIS_RY ).value(-scaleFromDIToUnit( js.lRz) );

/*
		// Point of view
		if( g_diDevCaps.dwPOVs >= 1 )
		{
			bw_snwprintf( strText, sizeof(strText)/sizeof(wchar_t), "%ld", js.rgdwPOV[0] );
			SetWindowText( GetDlgItem( hDlg, IDC_POV ), strText );
		}

		// Fill up text with which buttons are pressed
		str = strText;
		for( int i = 0; i < 32; i++ )
		{
			if ( js.rgbButtons[i] & 0x80 )
				str += bw_snprintf( str, sizeof(str), "%02d ", i );
		}
		*str = 0;   // Terminate the string

		SetWindowText( GetDlgItem( hDlg, IDC_BUTTONS ), strText );
*/
	}

	return true;
}


/**
 *	Mapping between direct input joystick button number and
 *	our joystick key events.
 */
static const KeyCode::Key s_joyKeys_PlayStation[32] =
{
	KeyCode::KEY_JOYTRIANGLE,
	KeyCode::KEY_JOYCIRCLE,
	KeyCode::KEY_JOYCROSS,
	KeyCode::KEY_JOYSQUARE,
	KeyCode::KEY_JOYL2,
	KeyCode::KEY_JOYR2,
	KeyCode::KEY_JOYL1,
	KeyCode::KEY_JOYR1,
	KeyCode::KEY_JOYSELECT,
	KeyCode::KEY_JOYSTART,
	KeyCode::KEY_JOYARPUSH,
	KeyCode::KEY_JOYALPUSH,
	KeyCode::KEY_JOYDUP,
	KeyCode::KEY_JOYDRIGHT,
	KeyCode::KEY_JOYDDOWN,
	KeyCode::KEY_JOYDLEFT,

	KeyCode::KEY_JOY16,
	KeyCode::KEY_JOY17,
	KeyCode::KEY_JOY18,
	KeyCode::KEY_JOY19,
	KeyCode::KEY_JOY20,
	KeyCode::KEY_JOY21,
	KeyCode::KEY_JOY22,
	KeyCode::KEY_JOY23,
	KeyCode::KEY_JOY24,
	KeyCode::KEY_JOY25,
	KeyCode::KEY_JOY26,
	KeyCode::KEY_JOY27,
	KeyCode::KEY_JOY28,
	KeyCode::KEY_JOY29,
	KeyCode::KEY_JOY30,
	KeyCode::KEY_JOY31
};


/**
 *	This methods processes the pending joystick events.
 */
bool Joystick::processEvents( InputHandler& handler,
							 KeyStates& keyStates,
							 bool * pLostDataFlag )
{
	BW_GUARD;
	if (!pDIJoystick_)
		return true;

	Vector2 cursorPosition = currentCursorPos( hWnd_ );


	DIDEVICEOBJECTDATA didod[ JOYSTICK_BUFFER_SIZE ];
	DWORD dwElements = 0;
	HRESULT hr;

	dwElements = JOYSTICK_BUFFER_SIZE;
	hr = pDIJoystick_->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
										didod, &dwElements, 0 );
	switch (hr) {

	case DI_OK:
		break;

	case DI_BUFFEROVERFLOW:
//		DEBUG_MSG( "Joystick::processEvents: joystick buffer overflow\n" );

		if ( pLostDataFlag )
			*pLostDataFlag = true;

		break;

	case DIERR_INPUTLOST:
	case DIERR_NOTACQUIRED:
//		DEBUG_MSG( "Joystick::processEvents: input not acquired, acquiring\n" );
		hr = pDIJoystick_->Acquire();
		if (FAILED(hr)) {
			DEBUG_MSG( "Joystick::processEvents: acquire failed\n" );
			return false;
		}
		dwElements = JOYSTICK_BUFFER_SIZE;
		hr = pDIJoystick_->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
										didod, &dwElements, 0 );
		if ( pLostDataFlag )
			*pLostDataFlag = true;

		break;

	default:
		DEBUG_MSG( "Joystick::processEvents: unhandled joystick error\n" );
		return false;
	}


	for (DWORD i = 0; i < dwElements; i++)
	{
		DWORD offset = didod[ i ].dwOfs;

		if (DIJOFS_BUTTON0 <= offset && offset <= DIJOFS_BUTTON31)
		{
			this->generateKeyEvent(
				!!(didod[ i ].dwData & 0x80),
				s_joyKeys_PlayStation[ offset - DIJOFS_BUTTON0 ],
				handler,
				keyStates,
				cursorPosition );
		}
		else
		{
			// TODO:PM Not handling joystick events currently.
			// It is all state based.
		}
	}

	this->generateCommonEvents( handler, keyStates, cursorPosition );

	return true;
}

//input.cpp
