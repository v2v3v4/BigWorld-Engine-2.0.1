/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOZILLA_WEB_PAGE_COMMAND_HPP
#define MOZILLA_WEB_PAGE_COMMAND_HPP

#include "moo/forward_declarations.hpp"
#include "moo/device_callback.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "romp/texture_feeds.hpp"
#include "web_page.hpp"
#include "third_party/LLMozlib2/llmozlib2.h"
#include "cstdmf/safe_fifo.hpp"

/**
*  This class implements a command sent in one of the following scenarios:
*  1. A MozillaWebPageInterface sending commands to its MozillaWebPage
*  2. A MozillaWebPage sending callbacks to its MozillaWebPageInterface
*/
class MozillaWebPageCommand : public SafeReferenceCount
{
public:
	MozillaWebPageCommand(int command, int key) :
		commandType_(command),
		key_(key)
	{

	}

	virtual ~MozillaWebPageCommand() 
	{
	}

	int commandType_;
	int key_;
};
typedef SmartPointer<MozillaWebPageCommand> MozillaWebPageCommandPtr;

//webPageCommands
const int COMMAND_MOZILLAWEBPAGE_COMMAND_TYPE = 1;
const int COMMAND_MOZILLAWEBPAGE_DTOR_COMMAND_TYPE = 2;
const int COMMAND_MOZILLAWEBPAGE_NAVIGATE_COMMAND_TYPE = 3;
const int COMMAND_MOZILLAWEBPAGE_ADD_INTERFACE_COMMAND_TYPE = 4;
const int COMMAND_MOZILLAWEBPAGE_UPDATE_BROWSER_COMMAND_TYPE = 5;
const int COMMAND_MOZILLAWEBPAGE_HANDLE_MOUSE_BUTTON_EVENT_COMMAND_TYPE = 7;
const int COMMAND_MOZILLAWEBPAGE_SCROLL_BY_LINES_COMMAND_TYPE = 8;
const int COMMAND_MOZILLAWEBPAGE_HANDLE_MOUSE_MOVE_COMMAND_TYPE = 9;
const int COMMAND_MOZILLAWEBPAGE_HANDLE_KEYBOARD_EVENT_COMMAND_TYPE = 10;
const int COMMAND_MOZILLAWEBPAGE_HANDLE_UNICODE_INPUT_COMMAND_TYPE = 11;
const int COMMAND_MOZILLAWEBPAGE_ENABLE_PROXY_COMMAND_TYPE = 12;
const int COMMAND_MOZILLAWEBPAGE_ENABLE_COOKIES_COMMAND_TYPE = 13;
const int COMMAND_MOZILLAWEBPAGE_NAVIGATE_BACK_COMMAND_TYPE = 14;
const int COMMAND_MOZILLAWEBPAGE_NAVIGATE_FORWARD_COMMAND_TYPE = 15;
const int COMMAND_MOZILLAWEBPAGE_NAVIGATE_RELOAD_COMMAND_TYPE = 16;
const int COMMAND_MOZILLAWEBPAGE_SET_SIZE_COMMAND_TYPE = 17;
const int COMMAND_MOZILLAWEBPAGE_FOCUS_BROWSER_COMMAND_TYPE = 18;
const int COMMAND_MOZILLAWEBPAGE_ALLOW_CURSOR_INTERACTION_COMMAND_TYPE = 19;
const int COMMAND_MOZILLAWEBPAGE_SET_404_REDIRECT_COMMAND_TYPE = 20;
const int COMMAND_MOZILLAWEBPAGE_EVALUATE_JAVASCRIPT_COMMAND_TYPE = 21;
const int COMMAND_MOZILLAWEBPAGE_CLEAN_MEMORY = 22;

//Callback commands
const int COMMAND_MOZILLAWEBPAGE_ON_PAGE_CHANGED = 100;
const int COMMAND_MOZILLAWEBPAGE_ON_NAVIGATION_BEGIN = 101;
const int COMMAND_MOZILLAWEBPAGE_ON_NAVIGATION_COMPLETE = 102;
const int COMMAND_MOZILLAWEBPAGE_ON_UPDATE_PROGRESS = 103;
const int COMMAND_MOZILLAWEBPAGE_ON_STATUS_TEXT_CHANGE = 104;
const int COMMAND_MOZILLAWEBPAGE_ON_LOCATION_CHANGE = 105;
const int COMMAND_MOZILLAWEBPAGE_ON_CLICK_LINK_HREF = 106;
const int COMMAND_MOZILLAWEBPAGE_ON_CLICK_LINK_NO_FOLLOW = 107;

class CommandMozillaWebPage : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPage (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_COMMAND_TYPE, key)
	  {
	  }

	  uint32 width;
	  uint32 height;
	  bool mipmap;
	  std::wstring url;
	  WebPageProvider::eGraphicsSettingsBehaviour graphicsSettingsBehaviour;
};

class CommandMozillaWebPageDTOR : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageDTOR (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_DTOR_COMMAND_TYPE, key)
	  {

	  }
};

//CommandMozillaAddInterface is defined at the end of the MozillaWebPageInterface due
//to hpp dependencies.

class CommandMozillaWebPageNavigate : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageNavigate (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_NAVIGATE_COMMAND_TYPE, key)
	  {

	  }
	  std::wstring url;
};

class CommandMozillaWebPageUpdateBrowser : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageUpdateBrowser (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_UPDATE_BROWSER_COMMAND_TYPE, key)
	  {

	  }
	  uint sequenceNum;
};

class CommandMozillaWebPageHandleMouseButtonEvent : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageHandleMouseButtonEvent (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_HANDLE_MOUSE_BUTTON_EVENT_COMMAND_TYPE, key)
	  {

	  }

	  Vector2 pos;
	  bool down;
};

class CommandMozillaWebPageScrollByLines : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageScrollByLines (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_SCROLL_BY_LINES_COMMAND_TYPE, key)
	  {

	  }

	  int32 lines;
};

class CommandMozillaWebPageHandleMouseMove : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageHandleMouseMove (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_HANDLE_MOUSE_MOVE_COMMAND_TYPE, key)
	  {

	  }
	  Vector2 pos;
	  bool down;
};

class CommandMozillaWebPageHandleKeyboardEvent : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageHandleKeyboardEvent (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_HANDLE_KEYBOARD_EVENT_COMMAND_TYPE, key)
	  {

	  }
	  uint32 keyCodeIn;
};

class CommandMozillaWebPageHandleUnicodeInput : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageHandleUnicodeInput (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_HANDLE_UNICODE_INPUT_COMMAND_TYPE, key)
	  {

	  }
	  uint32 keyCodeIn;
	  uint32 codePage;
};

class CommandMozillaWebPageEnableProxy : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageEnableProxy (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_ENABLE_PROXY_COMMAND_TYPE, key)
	  {

	  }
	  bool proxyEnabledIn;
	  std::string proxyHostNameIn;
	  uint32 proxyPortIn;
};

class CommandMozillaWebPageEnableCookies : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageEnableCookies (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_ENABLE_COOKIES_COMMAND_TYPE, key)
	  {

	  }
	  bool value;
};

class CommandMozillaWebPageNavigateBack : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageNavigateBack (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_NAVIGATE_BACK_COMMAND_TYPE, key)
	  {

	  }
};

class CommandMozillaWebPageNavigateForward : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageNavigateForward (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_NAVIGATE_FORWARD_COMMAND_TYPE, key)
	  {

	  }
};

class CommandMozillaWebPageNavigateReload : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageNavigateReload(int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_NAVIGATE_RELOAD_COMMAND_TYPE, key)
	  {

	  }
};

class CommandMozillaWebPageSetSize : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageSetSize (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_SET_SIZE_COMMAND_TYPE, key)
	  {

	  }
	  uint32 width; 
	  uint32 height;
	  uint32 textureWidth; 
	  uint32 textureHeight;
	  uint32 exactWidth; 
	  uint32 exactHeight;
};

class CommandMozillaWebPageFocusBrowser : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageFocusBrowser (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_FOCUS_BROWSER_COMMAND_TYPE, key)
	  {

	  }
	  bool focus;
	  bool enable;
};

class CommandMozillaWebPageAllowCursorInteraction : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageAllowCursorInteraction (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_ALLOW_CURSOR_INTERACTION_COMMAND_TYPE, key)
	  {

	  }
	  bool allow;
};

class CommandMozillaWebPageSet404Redirect : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageSet404Redirect (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_SET_404_REDIRECT_COMMAND_TYPE, key)
	  {

	  }
	  std::wstring url;
};

class CommandMozillaWebPageEvaluateJavascript : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageEvaluateJavascript (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_EVALUATE_JAVASCRIPT_COMMAND_TYPE, key)
	  {

	  }
	  std::string script;
};


class CommandMozillaWebPageCleanMemory : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageCleanMemory (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_CLEAN_MEMORY, key)
	  {

	  }
};

//callback commands
class CommandMozillaWebPageCallbackBase : public MozillaWebPageCommand
{
public:
	CommandMozillaWebPageCallbackBase (int command, int key) :
	  MozillaWebPageCommand(command, key),
	  event(-1, "")
	  {

	  }
	  LLEmbeddedBrowserWindowObserver::EventType event;
};

//callback commands
class CommandMozillaWebPageCallbackOnPageChanged : public CommandMozillaWebPageCallbackBase
{
public:
	CommandMozillaWebPageCallbackOnPageChanged (int key) :
	  CommandMozillaWebPageCallbackBase(COMMAND_MOZILLAWEBPAGE_ON_PAGE_CHANGED, key)
	  {
	  }
};

class CommandMozillaWebPageCallbackOnNavigationBegin : public CommandMozillaWebPageCallbackBase
{
public:
	CommandMozillaWebPageCallbackOnNavigationBegin (int key) :
	  CommandMozillaWebPageCallbackBase(COMMAND_MOZILLAWEBPAGE_ON_NAVIGATION_BEGIN, key)
	  {
	  }
};

class CommandMozillaWebPageCallbackOnNavigationComplete : public CommandMozillaWebPageCallbackBase
{
public:
	CommandMozillaWebPageCallbackOnNavigationComplete (int key) :
	  CommandMozillaWebPageCallbackBase(COMMAND_MOZILLAWEBPAGE_ON_NAVIGATION_COMPLETE, key)
	  {
	  }
};

class CommandMozillaWebPageCallbackOnUpdateProgress : public CommandMozillaWebPageCallbackBase
{
public:
	CommandMozillaWebPageCallbackOnUpdateProgress (int key) :
	  CommandMozillaWebPageCallbackBase(COMMAND_MOZILLAWEBPAGE_ON_UPDATE_PROGRESS, key)
	  {
	  }
};

class CommandMozillaWebPageCallbackOnStatusTextChange : public CommandMozillaWebPageCallbackBase
{
public:
	CommandMozillaWebPageCallbackOnStatusTextChange (int key) :
	  CommandMozillaWebPageCallbackBase(COMMAND_MOZILLAWEBPAGE_ON_STATUS_TEXT_CHANGE, key)
	  {
	  }
};

class CommandMozillaWebPageCallbackOnLocationChange : public CommandMozillaWebPageCallbackBase
{
public:
	CommandMozillaWebPageCallbackOnLocationChange (int key) :
	  CommandMozillaWebPageCallbackBase(COMMAND_MOZILLAWEBPAGE_ON_LOCATION_CHANGE, key)
	  {
	  }
};

class CommandMozillaWebPageCallbackOnClickLinkHref : public CommandMozillaWebPageCallbackBase
{
public:
	CommandMozillaWebPageCallbackOnClickLinkHref (int key) :
	  CommandMozillaWebPageCallbackBase(COMMAND_MOZILLAWEBPAGE_ON_CLICK_LINK_HREF, key)
	  {
	  }
};

class CommandMozillaWebPageCallbackOnClickLinkNoFollow : public CommandMozillaWebPageCallbackBase
{
public:
	CommandMozillaWebPageCallbackOnClickLinkNoFollow (int key) :
	  CommandMozillaWebPageCallbackBase(COMMAND_MOZILLAWEBPAGE_ON_CLICK_LINK_NO_FOLLOW, key)
	  {
	  }
};

#endif // MOZILLA_WEB_PAGE_COMMAND_HPP
