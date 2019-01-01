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
#include "web_page.hpp"
#include "cstdmf/bgtask_manager.hpp"
#include "ashes/simple_gui.hpp"
#include "moo/render_context.hpp"
#include "mozilla_web_page_interface.hpp"
#include "cstdmf/string_utils.hpp"
#include "resmgr\bwresource.hpp"
#include "mozilla_web_page.hpp"

DECLARE_DEBUG_COMPONENT2( "Web", 0 )

#define DEBUG_WEB
#ifdef DEBUG_WEB
#define PRINT_DEBUG INFO_MSG
#else
#define PRINT_DEBUG(param) 
#endif

static const std::string FILE_PREFIX = "file:///";

PY_ENUM_MAP( WebPageProvider::eGraphicsSettingsBehaviour );
PY_ENUM_CONVERTERS_SCATTERED( WebPageProvider::eGraphicsSettingsBehaviour );

/*
 *	WebPageProvider implementation
 */
PY_TYPEOBJECT( WebPageProvider )

PY_BEGIN_METHODS( WebPageProvider )
	PY_METHOD( navigate )
	PY_METHOD( update )
	PY_METHOD( handleMouseButtonEvent )
	PY_METHOD( scrollByLines )
	PY_METHOD( handleMouseMove )
	PY_METHOD( handleKeyboardEvent )
	PY_METHOD( handleUnicodeInput )
	PY_METHOD( enableProxy )
	PY_METHOD( enableCookies )
	PY_METHOD( navigateBack )
	PY_METHOD( navigateForward )
	PY_METHOD( navigateReload )
	PY_METHOD( setSize )
	PY_METHOD( focusBrowser )
	PY_METHOD( allowCursorInteraction )
	PY_METHOD( set404Redirect )
	PY_METHOD( setDynamic )
	PY_METHOD( evaluateJavaScript )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( WebPageProvider )
	/*~ attribute WebPageProvider.ratePerSecond
	*   The refresh update rate for the web page.
	*   The web page will be refreshed n times per second where n is smaller or equals to this value. 
	*   This is important as refreshing the web page is currently done in the main thread
	*   Creating performance spikes.
	*
	*	@type	float
	*/
	PY_ATTRIBUTE( ratePerSecond )
	/*~ attribute WebPageProvider.constantRefreshDuringNavigation
	*   A value specifiying whether this web page refreshes in a constant rate during navigation
	*   The web page will be refreshed one time per second during navigation if this value is true
	*
	*	@type	bool
	*/
	PY_ATTRIBUTE( constantRefreshDuringNavigation )
	/*~ attribute WebPageProvider.texture
	*   The texture of the current web page
	*
	*	@type	PyTextureProvider
	*/
	PY_ATTRIBUTE( texture )
	/*~ attribute WebPageProvider.url
	*	The current url
	*
	*	@type	unicode string
	*/
	PY_ATTRIBUTE( url )
	/*~ attribute WebPageProvider.listener
	*	The listener to receive callbacks for this webpage
	*
	*	The listener is a python object that exposes the callbacks from BigWorld.WebPageProvider
	*	All the callbacks receive a PyMozillaEvent, although not all properties 
	*	of the event is used in all callbacks, all callbacks need to return a boolean value
	*	to say whether they handled the event
	*	
	*	The callbacks that can be defined and their event properties are:
	*
	*	onStatusTextChange
	*	- PyMozillaEvent.uri - url for this event
	*	- PyMozillaEvent.stringValue - status text
	*
	*	onUpdateProgress
	*	- PyMozillaEvent.uri - url for this event
	*	- PyMozillaEvent.intValue - progress percentage
	*	
	*	onNavigateBegin
	*	- PyMozillaEvent.uri - url for this event
	*	- PyMozillaEvent.stringValue - status text
	*
	*	onNavigateComplete
	*	- PyMozillaEvent.uri - url for this event
	*	- PyMozillaEvent.stringValue - status text
	*	- PyMozillaEvent.intValue - the http response code
	*
	*	onLocationChange
	*	- PyMozillaEvent.uri - url for this event
	*
	*	onClickLinkHref
	*	- PyMozillaEvent.uri - url for this event
	*	- PyMozillaEvent.stringValue - href for the clicked link
	*	- PyMozillaEvent.stringValue2 - target for the clicked link
	*	
	*	onClickLinkNoFollow
	*	- PyMozillaEvent.uri - url for this event
	*	- PyMozillaEvent.stringValue - same as uri
	*
	*	onPageChanged
	*	- PyMozillaEvent.uri - url for this event
	*	- PyMozillaEvent.rect - rectangle that has changed
	*	
	*	@NOTE not all callbacks have to be supplied, only callbacks that are present will be called
	*
	*	@type python object
	*/
	PY_ATTRIBUTE( listener )
	/*~ attribute WebPageProvider.width
	* The width of the web page texture
	* 
	* @type uint32
	*/
	PY_ATTRIBUTE( width )
	/*~ attribute WebPageProvider.height
	* The height of the web page texture
	* 
	* @type uint32
	*/
	PY_ATTRIBUTE( height )
PY_END_ATTRIBUTES()

PY_FACTORY( WebPageProvider, BigWorld )

int PyWebPageProvider_token = 1;


/**
 *	Constructor for the web page provider
 */
WebPageProvider::WebPageProvider(uint32 width, uint32 height, bool dynamicPage, bool mipmap,
									const std::wstring& url, WebPageProvider::eGraphicsSettingsBehaviour graphicsSettingBehaviour, PyTypePlus * pType) :
	PyObjectPlusWithWeakReference( pType )
{
	webPage_ = new MozillaWebPageInterface( width, height, dynamicPage, mipmap, url, graphicsSettingBehaviour );

	webPage_->initPage();
}


WebPageProvider::~WebPageProvider()
{
	webPage_->unregister();
}


/**
 *	Standard get attribute method.
 */
PyObject * WebPageProvider::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	Standard set attribute method.
 */
int WebPageProvider::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}


/**
 *	Python construction method for WebPageProvider
 */
PyObject * WebPageProvider::pyNew( PyObject * args )
{
	BW_GUARD;
	int w;
	int h;
	char* graphicsSettingBehaviourString;
	int dynamicPage;
	int mipmap;
	eGraphicsSettingsBehaviour graphicsSettingsBehaviour;
	Py_UNICODE* url = NULL;
 
	if (!PyArg_ParseTuple( args, "iiiius", &w, &h, &dynamicPage, &mipmap, &url, &graphicsSettingBehaviourString ))
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.WebPageProvider() "
			"expects width, height, dynamicPage, mipmap, url and graphicsSettingsBehaviour" );
		return NULL;
	}
	
	if (w <= 0 || w > 4096 || h <= 0 || h > 4096)
	{
		PyErr_SetString( PyExc_ValueError, "BigWorld.WebPageProvider() "
			"width and heigth must be > 0 and <= 4096" );
		return NULL;
	}

	PyObject* tempObj = Script::getData(graphicsSettingBehaviourString);
	int ret = Script::setData( tempObj, graphicsSettingsBehaviour, "EFFECT_QUALITY" );
	Py_DecRef(tempObj);
	if (ret != 0) 
	{
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.WebPageProvider() graphicsSettingsBehaviour "
						"is incorrect, it should be either EFFECT_QUALITY or EFFECT_SCROLLING");
		return NULL;
	}
	}
	return new WebPageProvider( w, h, dynamicPage != 0 ? true : false, mipmap != 0 ? true : false, url ? reinterpret_cast<wchar_t*>(url) : L"", graphicsSettingsBehaviour );
}


/*~ function WebPageProvider.navigate
 *
 *	This method navigates the web page to a different url
 *
 *	@param	url		A unicode string specifying the new url
 *
 *	@return			None 
 */
/**
 *	This method causes the webpage to navigate to a url
 *	@param url the url to navigate to
*/
void WebPageProvider::navigate( const std::wstring& url )
{
	webPage_->navigate( url );
}


/*~ function WebPageProvider.url
 *
 *	This method returns the current web page url
 *
 *	@return			The webpage url
 */
/**
 *	This method returns the current url
 *	@return current url
 */
const std::wstring WebPageProvider::url()
{
	return webPage_->url();
}

PyObjectPtr WebPageProvider::listener()
{
	return webPage_->listener();
}

void WebPageProvider::listener( PyObjectPtr pListener )
{
	webPage_->listener( pListener );
}


/*~ function WebPageProvider.texture
 *
 *	This method returns the web page texture
 *
 *	@return			A texture provider for the webpage
 */
/**
 *	Get a texture provider for the webpage
 *	@return a unique texture provider for the webpage
 */
PyTextureProviderPtr WebPageProvider::pTexture()
{
	return PyTextureProviderPtr( 
		new PyTextureProvider( this, webPage_.get() ), 
		PyObjectPtr::STEAL_REFERENCE );
}


/*~ function WebPageProvider.update
 *
 *	This method update the web page and the texture
 *
 *	@return			None
 */
/**
 *	update the web page and the texture
 *	@return		None
 */
void WebPageProvider::update()
{
	webPage_->update();
}


/*~ function WebPageProvider.handleMouseButtonEvent
 *
 *	This method handles mouse events sent to the web page. These should be 
 *  in pixel space of the WebPage.
 *
 *	@param	pos		A Vector2 specifying the position of the button event
 *	@param	down	a bool representing whether the button is down or up
 *
 *	@return			None 
 */
/**
 *	This method handles mouse events sent to the web page. These should be 
 *  in pixel space of the WebPage.
 *
 *	@param	pos		The position of the button event
 *	@param	down	Whether the button is down or up
 *
 *	@return None
 */
void WebPageProvider::handleMouseButtonEvent(const Vector2& pos, bool down)
{
	webPage_->handleMouseButtonEvent(pos, down);
}


/*~ function WebPageProvider.scrollByLines
 *
 *	This method handles mouse scrolling events sent to the web page. 
 *  lines mark how much to scroll
 *
 *	@param	lines	A int32 specifying how much to scroll
 *
 *	@return			None 
 */
/**
 *	This method handles scrolling the web page
 *  
 *	@param	Lines to scroll the web page
 *
 */
void WebPageProvider::scrollByLines(int32 lines)
{
	webPage_->scrollByLines(lines);
}


/*~ function WebPageProvider.handleMouseMove
 *
 *	This method handles mouse move events sent to the web page. These should be 
 *  in pixel space of the WebPage.
 *
 *	@param	pos		A Vector2 specifying the position of the mouse event
 *
 *	@return			None 
 */
/**
 *	This method handles mouse move events sent to the web page. These should be 
 *  in pixel space of the WebPage.
 *
 *	@param	pos		The position of the mouse event
 *
 *	@return None
 */
void WebPageProvider::handleMouseMove(const Vector2& pos)
{
	webPage_->handleMouseMove(pos);
}


/*~ function WebPageProvider.handleKeyboardEvent
 *
 *	This method handles special int keyboard events sent to the web page. 
 *
 *	@param	keyCodeIn		An int specifying the special key clicked
 *
 *	@return			None 
 */
/**
 *	This method handles special int keyboard events sent to the web page. 
 *
 *	@param	keyCodeIn		The special key clicked
 *
 *	@return			None 
 */
void WebPageProvider::handleKeyboardEvent(uint32 keyCodeIn)
{
	webPage_->handleKeyboardEvent(keyCodeIn);
}


/*~ function WebPageProvider.handleUnicodeInput
 *
 *	This method handles unicode keyboard events sent to the web page. 
 *
 *	@param	keyCodeIn		A wide string whose first char represents the key in
 *
 *	@return			None 
 */
/**
 *	This method handles unicode keyboard events sent to the web page. 
 *
 *	@param	keyCodeIn		A wide string whose first char represents the key in
 *
 *	@return			None 
*/
void WebPageProvider::handleUnicodeInput(const std::wstring& keyCodeIn)
{
	if (keyCodeIn.size() < 1)
	{
		WARNING_MSG("Empty string sent to provider");
	}
	webPage_->handleUnicodeInput(keyCodeIn[0]);
}


/*~ function WebPageProvider.enableProxy
 *
 *	This method enables proxy usage by this provider.
 *
 *	@param	proxyEnabledIn		A bool marking whether to enable the proxy usage
 *	@param	proxyHostNameIn		A string representing the proxy host name
 *	@param	proxyPortIn			An int representing the proxy port
 *
 *	@return			bool		True for success
 */
/**
 *	This method enables proxy usage by this provider
 *
 *	@param	proxyEnabledIn		Whether to enable the proxy usage
 *	@param	proxyHostNameIn		The proxy host name
 *	@param	proxyPortIn			The proxy port
 *
 *	@return			bool		True for success
 */
bool WebPageProvider::enableProxy( bool proxyEnabledIn, const std::string& proxyHostNameIn, uint32 proxyPortIn )
{
	return webPage_->enableProxy( proxyEnabledIn, proxyHostNameIn, proxyPortIn );
}


/*~ function WebPageProvider.enableCookies
 *
 *	This method enables or disables cookies.
 *
 *	@param	value		The Cookies setting
 *
 *	@return			bool		True for success
 */
/**
 *	This method enables or disables cookies
 *
 *	@param	value		The Cookies setting
 *
 *	@return			bool		True for success
 */
bool WebPageProvider::enableCookies( bool value )
{
	return webPage_->enableCookies( value );
}


/*~ function WebPageProvider.navigateBack
 *
 *	This method navigates one page back.
 *
 *	@return			bool  True if succsess
 */
/**
 *	This method navigates one page back.
 *
 *	@return			bool  True if succsess
 */
bool WebPageProvider::navigateBack() 
{ 
	return webPage_->navigateBack();
}


/*~ function WebPageProvider.navigateForward
 *
 *	This method navigates one page forward.
 *
 *	@return			bool  True if succsess
 */
/**
 *	This method navigates one page forward.
 *
 *	@return			bool  True if succsess
 */
bool WebPageProvider::navigateForward() 
{ 
	return webPage_->navigateForward();
}


/*~ function WebPageProvider.navigateReload
 *
 *	This method reloads the web page.
 *
 *	@return			bool  True if succsess
 */
/**
 *	This method reloads the web page
 *
 *	@return			bool  True if succsess
 */
bool WebPageProvider::navigateReload() 
{ 
	return webPage_->navigateReload();
}


/*~ function WebPageProvider.setSize
 *
 *	This method is used to set the size of the web page
 *
 *	@param	width	The width of the web page
 *	@param	height	The height of the web page
 *	@param	exactWidth	Used for flash GUIs, contains their exact width
 *	@param	exactHeight	Used for flash GUIs, contains their exact height
 *
 *	@return			bool  True if succsess
 */
/**
 *	This method is used to set the size of the web page
 *
 *	@param	width	The width of the web page
 *	@param	height	The height of the web page
 *
 *	@return			bool  True if succsess
 */
bool WebPageProvider::setSize (uint32 width, uint32 height, uint32 exactWidth, uint32 exactHeight)
{
	return webPage_->setSize (width, height, exactWidth, exactHeight);
}


/*~ function WebPageProvider.focusBrowser
 *
 *	This method is used to set the browser focus mode
 *
 *	@param	focus   The new focus state
 *	@param	enable  Whether to enable the window (currently not used)
 *
 */
/**
 *
 *	This method is used to set the browser focus mode
 *
 *	@param	focus   The new focus state
 *	@param	enable  Whether to enable the window (currently not used)
 *
 */
void WebPageProvider::focusBrowser (bool focus, bool enable)
{
	webPage_->focusBrowser (focus, enable);
}


/*~ function WebPageProvider.allowCursorInteraction
 *
 *	This method is used to set the browser interaction mode 
 *  It effects whether it can alter the mouse or not.
 *
 *	@param	allow   The new interaction state
 *
 */
/**
 *
 *	This method is used to set the browser interaction mode.
 *  It effects whether it can alter the mouse or not.
 *
 *	@param	allow   The new interaction state
 *
 */
void WebPageProvider::allowCursorInteraction (bool allow)
{
	webPage_->allowCursorInteraction (allow);
}


/*~ function WebPageProvider.set404Redirect
 *
 *	This method is used to set the default page when the web page
 *  isn't found.
 *
 *	@param	url   The redirect location
 *
 */
/**
 *
 *	This method is used to set the default page when the web page
 *  isn't found.
 *
 *	@param	url   The redirect location
 *
 */
void WebPageProvider::set404Redirect( const std::wstring& url  )
{
	webPage_->set404Redirect(url);
}


/*~ function WebPageProvider.setDynamic
 *
 *	This method is used to set whether this page is dynamic
 *  meaning refreshes without someone clicking a link so should get
 *  higher refresh rate.
 *
 *	@param	dynamicPage whether this page is static or updates all the time (dynamic)
 *
 */
/**
 *
 *	This method is used to set whether this page is dynamic
 *  meaning refreshes without someone clicking a link so should get
 *  higher refresh rate.
 *
 *	@param	dynamicPage whether this page is static or updates all the time (dynamic)
 *
 */
void WebPageProvider::setDynamic( bool dynamicPage )
{
	webPage_->setDynamic (dynamicPage);
}


/*~ function WebPageProvider.evaluateJavaScript
 *
 *	This method is used to evaluate JavaScript code on this web page.
 *
 *	@param	script The JavaScript code to execute
 *
 *	@return The result of this command as a string.
 */
/**
 *	This method is used to evaluate JavaScript code on this web page.
 *
 *	@param	dynamicPage whether this page is static or updates all the time (dynamic)
 *
 */
std::string WebPageProvider::evaluateJavaScript( const std::string & script )
{
	return webPage_->evaluateJavaScript( script );
}


/*~ function WebPageProvider.width
 *
 *	This method is used to get the width of the web page
 *
 *	@return			The width of the webpage
 */
/**
 *	This method returns the width of the webpage.
 *
 *	@return			The width of the webpage
 */
uint32 WebPageProvider::width() const
{
	return webPage_->width();
}


/*~ function WebPageProvider.height
 *
 *	This method is used to get the height of the web page
 *
 *	@return			The height of the webpage
 */
/**
 *	This method returns the height of the webpage.
 *
 *	@return			The height of the webpage
 */
uint32 WebPageProvider::height() const
{
	return webPage_->height();
}


/**
 *	Get method for the refreshRatePerSecond attribute
 */
float WebPageProvider::ratePerSecond()
{
	BW_GUARD;
	return webPage_->getRatePerSecond();
}

/**
 *	Specialised set method for the ratePerSecond attribute
 */
void WebPageProvider::ratePerSecond( float value )
{
	BW_GUARD;
	webPage_->setRatePerSecond(value);
}


/**
 *	Get method for the constantRefreshDuringNavigation attribute
 */
bool WebPageProvider::constantRefreshDuringNavigation()
{
	BW_GUARD;
	return webPage_->getConstantRefreshDuringNavigation();
}

/**
 *	Specialised set method for the constantRefreshDuringNavigation attribute
 */
void WebPageProvider::constantRefreshDuringNavigation( bool value )
{
	BW_GUARD;
	webPage_->setConstantRefreshDuringNavigation(value);
}


//static vars
WebPage::GraphicsSettingPtr WebPage::s_graphicsSettingsPtr = NULL;
WebPage::eGraphicsOption WebPage::graphicsOption_ = GRAPHICS_OPTION_HIGH; 

WebPage::WebPage() 
{
}


/**
* Init the webpage 
*/
void WebPage::init ()
{
  // Mozilla graphics settings
  if (! s_graphicsSettingsPtr.exists())
  {
	  s_graphicsSettingsPtr =
		  new Moo::StaticCallbackGraphicsSetting(
		  "WEB_SETTINGS", "Web Settings", &WebPage::setGraphicSettings,
		  0, false, false);
	  s_graphicsSettingsPtr->addOption("HIGH", "High", true);
	  s_graphicsSettingsPtr->addOption("MEDIUM", "Medium", true);
	  s_graphicsSettingsPtr->addOption("LOW", "Low", true);
	  Moo::GraphicsSetting::add(s_graphicsSettingsPtr);
	  setGraphicSettings(s_graphicsSettingsPtr->activeOption());
  }
}

void WebPage::fini ()
{
  // Mozilla graphics settings
  s_graphicsSettingsPtr = NULL;
}


/**
* This method is used to set the graphics settings of the web page
*/
void WebPage::setGraphicSettings (int option)
{
	BW_GUARD;
	switch (option)
	{
	case 0:
		{
			INFO_MSG("MozillaWebPage uses GRAPHICS_OPTION_HIGH\n");
			graphicsOption_ = GRAPHICS_OPTION_HIGH;	
			break;
		}
	case 1:
		{
			INFO_MSG("MozillaWebPage uses GRAPHICS_OPTION_MEDIUM\n");
			graphicsOption_ = GRAPHICS_OPTION_MEDIUM;	
			break;
		}
	case 2:
		{
			INFO_MSG("MozillaWebPage uses GRAPHICS_OPTION_LOW\n");
			graphicsOption_ = GRAPHICS_OPTION_LOW;	
			break;
		}
	}
	if (MozillaWebPageManager::pInstance())
	{
		MozillaWebPageManager::instance().updateTexturesBasedOnGraphicSettings();
	}
}


/**
 *	This method is used to resolve a url from a file://<relative> to an
 *  absolute path.
*/
std::wstring WebPage::getResolvedUrl(const std::wstring& url)
{
	std::wstring retUrl = url;
	//support relative paths in our res tree (when the path starts with the file:///
	std::string checkedUrl = bw_wtoacp(url);
	if (bw_strnicmp(checkedUrl.c_str(), FILE_PREFIX.c_str(), FILE_PREFIX.size()) == 0) 
	{
		std::string relativePath = checkedUrl.substr(FILE_PREFIX.size(), std::string::npos);
		BWResource::instance().resolveToAbsolutePath( relativePath );
		if (IFileSystem::FT_NOT_FOUND)
		{
			WARNING_MSG("Could not find file %S", url.c_str());
		}
		else 
		{
			retUrl = bw_acptow(FILE_PREFIX + relativePath);
		}
	}
	return retUrl;
}
