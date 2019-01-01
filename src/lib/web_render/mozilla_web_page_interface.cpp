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
#include "mozilla_web_page_interface.hpp"
#include "mozilla_web_page.hpp"
#include "cstdmf/bgtask_manager.hpp"
#include "ashes/simple_gui.hpp"
#include "moo/render_context.hpp"
#include "third_party/LLMozlib2/llmozlib_virtual_wrapper.h"
#include <mmsystem.h>
#include "math/math_extra.hpp"
#include "cstdmf/string_utils.hpp"
#include "resmgr\bwresource.hpp"

//minimal size for setting a mozilla window
const int MOZILLA_EPSILON = 25;
//Time to continue doing navigation (in seconds).
const int CONTINUE_NAVIGATION_TIME = 5;

/*~ class PyMozillaEvent
 *	@components{ client }
 *
 *	This class contains the event state that is passed to the listener
 *	for a BigWorld.WebPageProvider
 *
 *  @see WebPageProvider.listener for usage
 */
class PyMozillaEvent : public PyObjectPlus
{
	Py_Header( PyMozillaEvent, PyObjectPlus )

public:
	typedef LLEmbeddedBrowserWindowObserver::EventType EventType;

	PyMozillaEvent( const EventType & eventIn ) :
	PyObjectPlus( &s_type_ ),
		eventIn_( eventIn )
	{
	}


	/**
	*	Standard get attribute method.
	*/
	PyObject * pyGetAttribute( const char * attr )
	{
		PY_GETATTR_STD();

		return PyObjectPlus::pyGetAttribute( attr );
	}

private:
	PY_RO_ATTRIBUTE_DECLARE( eventIn_.getEventUri(), uri )
	PY_RO_ATTRIBUTE_DECLARE( eventIn_.getIntValue(), intValue )
	PY_RO_ATTRIBUTE_DECLARE( eventIn_.getStringValue(), stringValue )
	PY_RO_ATTRIBUTE_DECLARE( eventIn_.getStringValue2(), stringValue2 )

	PyObject * pyGet_rect()
	{
		int x, y, w, h;
		eventIn_.getRectValue( x, y, w, h );
		return Py_BuildValue( "iiii", x, y, w, h );

	}
	PY_RO_ATTRIBUTE_SET( rect )
	
	EventType eventIn_;
};

PY_TYPEOBJECT( PyMozillaEvent )

PY_BEGIN_METHODS( PyMozillaEvent )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyMozillaEvent )
/*~ attribute PyMozillaEvent.uri
 *	The url that this event is for
 * @type string
 */
PY_ATTRIBUTE( uri )
/*~ attribute PyMozillaEvent.intValue
 * int value for this event
 * @type int
 */
PY_ATTRIBUTE( intValue )
/*~ attribute PyMozillaEvent.stringValue
 * string value for this event
 * @type string
 */
PY_ATTRIBUTE( stringValue )
/*~ attribute PyMozillaEvent.stringValue2
 * second string value for this event
 * @type string
 */
PY_ATTRIBUTE( stringValue2 )
/*~ attribute PyMozillaEvent.rect
 * rectangle for this event
 * @type tuple of 4 ints
 */
PY_ATTRIBUTE( rect )
PY_END_ATTRIBUTES()

/**
*	Constructor for the MozillaWebPage object
*	@param width the width of the web page texture
*	@param height the height of the web page texture
*	@param dynamicPage - dynamice pages are updated in a higher rate
*	@param mipmap whether this page is using mipmapping
*	@param url the url to use for the web page texture
*	@param graphicsSettingsBehaviour how this web page responds to graphics settings
*/
MozillaWebPageInterface::MozillaWebPageInterface(uint32 width, 
											 uint32 height, 
											 bool dynamicPage, 
											 bool mipmap, 
											 const std::wstring& url,
											 WebPageProvider::eGraphicsSettingsBehaviour graphicsSettingsBehaviour) :
	url_(url), 
	dynamicPage_(dynamicPage), 
	mipmap_(mipmap),
	graphicsSettingsBehaviour_(graphicsSettingsBehaviour),
	pageChanged_(true),
	sequenceNum_(MozillaWebPageManager::s_defaultSequenceNum),
	textureSequenceNum_(0),
	pageInUse_(true),
	allowTextureCreation_(true),
	exactWidth_(0),
	exactHeight_(0),
	doingNavigation_(true),
	stopDoingNavigation_(false),
	stopDoingNavigationStartTime_(0),
	constantRefreshDuringNavigation_(true),
	ratePerSecond_(1),
	forceNextTextureUpdate_(false)
{
	if (MozillaWebPageManager::pInstance())
	{
		key_ = 	MozillaWebPageManager::instance().getNextKey();
		CommandMozillaWebPage* command = new CommandMozillaWebPage(key_);
		command->width = width;
		command->height = height;
		command->mipmap = mipmap;
		command->url = url;
		command->graphicsSettingsBehaviour = graphicsSettingsBehaviour;
		MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
		prevTime_ = timeGetTime();
		requestedWidth_ = width;
		requestedHeight_ = height;
	}
}


/**
 * This method initialises this Web Page
 */
void MozillaWebPageInterface::initPage()
{
	if (MozillaWebPageManager::pInstance())
	{
		//add this interface
		CommandMozillaAddInterface* commandAddInterface = new CommandMozillaAddInterface(key_);
		commandAddInterface->mozillaInterface = this;
		MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(commandAddInterface));
		//sizeInfo.setSize - (size calculation is dependent on the format we choose as it effects the mem usage
		sizeInfo.setSize(requestedWidth_, requestedHeight_, MozillaWebPageManager::s_format, graphicsSettingsBehaviour_, bw_wtoacp(url_));
		//set the size of this window based on the choosen size
		setWebSize(getWidth(), getHeight());
	}
}

MozillaWebPageInterface::~MozillaWebPageInterface()
{
	//MozillaWebPageManager might have been already removed.
	if (MozillaWebPageManager::pInstance() && MozillaWebPageManager::s_isValid)
	{
		MozillaWebPageCommandPtr command = new CommandMozillaWebPageDTOR(key_);
		MozillaWebPageManager::instance().pushCommand(command);
	}
	deleteUnmanagedObjects();
}


/**
*	This method causes the object to navigate to a url
*	@param url the url to navigate to
*/
void MozillaWebPageInterface::navigate( const std::wstring& url )
{
	CommandMozillaWebPageNavigate* command = new CommandMozillaWebPageNavigate(key_);
	command->url = url;
	MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
	pageChanged_ = true;
	url_ = url;
	doingNavigation_ = true;
}


void MozillaWebPageInterface::setUrl(std::wstring url)
{ 
	url_ = url; 
}

/**
*	This method causes the browser to update itself
*/
void MozillaWebPageInterface::update()
{
	updateBrowser();
}


/**
*	This method causes the browser to update itself
*/
void MozillaWebPageInterface::updateBrowser()
{
	//don't send update unless previous update was finished.
	if (MozillaWebPageManager::instance().drawWeb()) 
	{
		if (MozillaWebPageManager::instance().getSequenceNum(key_) == sequenceNum_) 
		{
			CommandMozillaWebPageUpdateBrowser* command = new CommandMozillaWebPageUpdateBrowser(key_);
			sequenceNum_++;
			command->sequenceNum = sequenceNum_;
			MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
		}
	}
}


/**
*	This method updates the texture to the current browser state
*/
void MozillaWebPageInterface::updateTexture()
{
	//nothing required (done automatically by the pTexture)
	return;
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
void MozillaWebPageInterface::handleMouseButtonEvent(const Vector2& pos, bool down)
{
	CommandMozillaWebPageHandleMouseButtonEvent* command = new CommandMozillaWebPageHandleMouseButtonEvent(key_);
	command->pos = pos;
	command->down = down;
	MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
}


/**
*	This method handles scrolling the web page
*  
*	@param	Lines to scroll the web page
*
*/
void MozillaWebPageInterface::scrollByLines(int32 lines)
{
	CommandMozillaWebPageScrollByLines* command = new CommandMozillaWebPageScrollByLines(key_);
	command->lines = lines;
	MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
}


/**
*	This method handles mouse move events sent to the web page. These should be 
*  In pixel space of the WebPage
*
*	@param	pos		The position of the button event
*
*	@return None
*/
void MozillaWebPageInterface::handleMouseMove(const Vector2& pos)
{
	CommandMozillaWebPageHandleMouseMove* command = new CommandMozillaWebPageHandleMouseMove(key_);
	command->pos = pos;
	MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
}


/**
*	This method handles special int keyboard events sent to the web page. 
*
*	@param	keyCodeIn		The special key clicked
*
*	@return			None 
*/
void MozillaWebPageInterface::handleKeyboardEvent(uint32 keyCodeIn)
{
	CommandMozillaWebPageHandleKeyboardEvent* command = new CommandMozillaWebPageHandleKeyboardEvent(key_);
	command->keyCodeIn = keyCodeIn;
	MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
}


/**
*	This method handles unicode keyboard events sent to the web page. 
*
*	@param	keyCodeIn		The unicode char clicked
*
*	@return			None 
*/
void MozillaWebPageInterface::handleUnicodeInput(uint32 keyCodeIn )
{
	CommandMozillaWebPageHandleUnicodeInput* command = new CommandMozillaWebPageHandleUnicodeInput(key_);
	command->keyCodeIn = keyCodeIn;

	HKL tempHKL = GetKeyboardLayout(GetCurrentThreadId());
	DWORD lcid = MAKELCID( tempHKL, SORT_DEFAULT );
	char codePageString[200];
	DWORD ret = GetLocaleInfoA( lcid, LOCALE_IDEFAULTANSICODEPAGE, codePageString, ARRAY_SIZE(codePageString));
	command->codePage = atoi(codePageString);
	MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
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
bool MozillaWebPageInterface::enableProxy( bool proxyEnabledIn, const std::string& proxyHostNameIn, uint32 proxyPortIn )
{
	CommandMozillaWebPageEnableProxy* command = new CommandMozillaWebPageEnableProxy(key_);
	command->proxyEnabledIn = proxyEnabledIn;
	command->proxyHostNameIn = proxyHostNameIn;
	command->proxyPortIn = proxyPortIn;
	MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
	return true;
}


/**
*	This method enables or disables cookies
*
*	@param	value		The Cookies setting
*
*	@return			bool		True for success
*/
bool MozillaWebPageInterface::enableCookies( bool value)
{
	CommandMozillaWebPageEnableCookies* command = new CommandMozillaWebPageEnableCookies(key_);
	command->value = value;
	MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
	return true;
}


/**
 *	This method sets the refresh update rate for the web page
 *  The web page will be refreshed at most ratePerSecond times per second
 *  This is important as refreshing the web page might cause cpu load (especially in single core machines)
 *
 *	@param	ratePerSecond		An int representing the max number of refreshes per second
 *
 *	@return			None
 */
void MozillaWebPageInterface::setRatePerSecond( float ratePerSecond ) 
{
	BW_GUARD;
	if (ratePerSecond == 0)
	{
		WARNING_MSG("MozillaWebPag Invalid ratePerSecond, it cannot be 0\n");
		return;
	}
	ratePerSecond_ = ratePerSecond;
}


/**
*	This method gets the refresh update rate for the web page
*  The web page will be refreshed at most ratePerSecond times per second
*
*	@return			the refresh rate per second
*/
float MozillaWebPageInterface::getRatePerSecond() 
{
	return ratePerSecond_;
}


/**
 *	This method sets whether the web page refreshes in a constant rate during navigation
 *
 *	@param	constantRefreshDuringNavigation		A bool representing whether the web page will refresh in a constant rate
 *
 *	@return			None
 */
void MozillaWebPageInterface::setConstantRefreshDuringNavigation( bool constantRefreshDuringNavigation ) 
{
	constantRefreshDuringNavigation_ = constantRefreshDuringNavigation;
}


/**
*	This method returns whether the web page refreshes in a constant rate during navigation
*
*	@return			A bool representing whether the web page will refresh in a constant rate
*/
bool MozillaWebPageInterface::getConstantRefreshDuringNavigation() 
{
	return constantRefreshDuringNavigation_;
}


/**
* Get the delta time between refreshes
*/
DWORD MozillaWebPageInterface::getDeltaRefreshInMili(DWORD currentTime) 
{ 
	DWORD ret = /*Just very low refresh rate*/2147483648UL;
	//stop navigation after CONTINUE_NAVIGATION_TIME seconds.
	if (stopDoingNavigation_ && stopDoingNavigationStartTime_ + CONTINUE_NAVIGATION_TIME * 1000 < currentTime)
	{
		doingNavigation_ = false;
		stopDoingNavigation_ = false;
		stopDoingNavigationStartTime_ = 0;
	}
	if (doingNavigation_ && constantRefreshDuringNavigation_)
	{
		ret = 1000;
	}
	if (ratePerSecond_ <= 0)
	{
		WARNING_MSG("MozillaWebPag Invalid ratePerSecond, it cannot be <= 0\n");
		return 1000;
	}
	//we return the minimal time between the ret and the calculated value
	return min((DWORD)(1.0/ratePerSecond_ * 1000.0), ret);
}


/**
 *	This method sets the listener for this web page
 *  This listener will get callbacks (see the WebPage class) when specific 
 *  Web page navigation events happen.
 *
 *	@param	pListener a python object
 */
void MozillaWebPageInterface::listener(PyObjectPtr pListener)
{
	pListener_ = pListener;
}


/**
 *	This method gets the listener for this web page
 *
 *	@return	 the current listener object
 */
PyObjectPtr MozillaWebPageInterface::listener()
{
	return pListener_;
}


/**
*	This method navigates one page back
*
*	@return			bool  True if succsess
*/
bool MozillaWebPageInterface::navigateBack()
{
	MozillaWebPageCommandPtr command = new CommandMozillaWebPageNavigateBack(key_);
	MozillaWebPageManager::instance().pushCommand(command);
	doingNavigation_ = true;
	return true;
}


/**
*	This method navigates one page forward.
*
*	@return			bool  True if succsess
*/
bool MozillaWebPageInterface::navigateForward()
{
	MozillaWebPageCommandPtr command = new CommandMozillaWebPageNavigateForward(key_);
	MozillaWebPageManager::instance().pushCommand(command);
	doingNavigation_ = true;
	return true;
}


/**
 *	This method reloads the web page
 *
 *	@return			bool  True if succsess
*/
bool MozillaWebPageInterface::navigateReload()
{
	MozillaWebPageCommandPtr command = new CommandMozillaWebPageNavigateReload(key_);
	MozillaWebPageManager::instance().pushCommand(command);
	doingNavigation_ = true;
	return true;
}


/**
*	This method sets the size of the mozila component
*
*	@param width the width of the web page texture
*	@param height the height of the web page texture
*
*	@return			bool  True if succsess
*/
bool MozillaWebPageInterface::setSize (uint32 width, uint32 height, uint32 exactWidth, uint32 exactHeight)
{
	requestedWidth_ = width;
	requestedHeight_ = height;
	exactWidth_ = exactWidth;
	exactHeight_ = exactHeight;
	//recreate the textures
	destroyTexture(/*we allow creating of texture if already allowed*/allowTextureCreation_);
	//sizeInfo.setSize - (size calculation is dependent on the format we choose as it effects the mem usage
	sizeInfo.setSize(requestedWidth_, requestedHeight_, MozillaWebPageManager::s_format, graphicsSettingsBehaviour_, bw_wtoacp(url_));
	//set the size of this window based on the choosen size
	setWebSize(getWidth(), getHeight());
	return true;
}

/**
*	This method sets the size of the mozila internal component
*
*	@param width the width of the web page 
*	@param height the height of the web page 
*
*	@return			bool  True if succsess
*/
bool MozillaWebPageInterface::setWebSize (uint32 width, uint32 height)
{
	//send command to recreate the browser window
	CommandMozillaWebPageSetSize* command = new CommandMozillaWebPageSetSize(key_);
	command->width = width;
	command->height = height;
	command->textureWidth = getTextureWidth();
	command->textureHeight = getTextureHeight();
	command->exactWidth = exactWidth_;
	command->exactHeight = exactHeight_;
	MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
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
void MozillaWebPageInterface::focusBrowser (bool focus, bool enable)
{
	CommandMozillaWebPageFocusBrowser* command = new CommandMozillaWebPageFocusBrowser(key_);
	command->focus = focus;
	command->enable = enable;
	MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
}


/**
*
*	This method is used to allow the browser to interact with our window (mouse ...)
*
*	@param	allow   The new state
*
*/
void MozillaWebPageInterface::allowCursorInteraction (bool allow)
{
	CommandMozillaWebPageAllowCursorInteraction* command = new CommandMozillaWebPageAllowCursorInteraction(key_);
	command->allow = allow;
	MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
}

/**
*
*	This method is used to set the default page when the web page
*  isn't found.
*
*	@param	url   The redirect location
*/
void MozillaWebPageInterface::set404Redirect( const std::wstring& url  )
{
	CommandMozillaWebPageSet404Redirect* command = new CommandMozillaWebPageSet404Redirect(key_);
	command->url = url;
	MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
}


/**
*	This method is used to set whether this page is dynamic
*  meaning refreshes without someone clicking a link so should get
*  higher refresh rate.
*/
void MozillaWebPageInterface::setDynamic( bool dynamicPage )
{
	dynamicPage_ = dynamicPage;
}


/**
*	This method is used to evaluate JavaScript code on this web page.
*/
std::string  MozillaWebPageInterface::evaluateJavaScript( const std::string & script )
{
	CommandMozillaWebPageEvaluateJavascript* command = new CommandMozillaWebPageEvaluateJavascript(key_);
	command->script = script;
	MozillaWebPageManager::instance().pushCommand(MozillaWebPageCommandPtr(command));
	return "";
}


/**
*
* This method is used to unregister this interface from its manager
*
*/
void MozillaWebPageInterface::unregister()
{
	MozillaWebPageManager::instance().removeInterface(key_);
}


/**
*	This is the overridden width method from the BaseTexture interface,
*/	
uint32 MozillaWebPageInterface::width( ) const 
{ 
	BW_GUARD;
	return getWidth(); 
}


/**
*	This is the overridden height method from the BaseTexture interface,
*/
uint32 MozillaWebPageInterface::height( ) const 
{ 
	BW_GUARD;
	return getHeight(); 
}


/**
 *	This is the overridden pTexture method from the BaseTexture interface,
 *	it return the d3d texture for the web page
 *	It also makes sure the texture is updated.
 */
DX::BaseTexture* MozillaWebPageInterface::pTexture( ) 
{
	BW_GUARD;
	MF_ASSERT_DEV(MainThreadTracker::isCurrentThreadMain());
	if (MozillaWebPageManager::instance().drawWeb()) 
	{
		//mark that this page is in use
		pageInUse_ = true;
		if (!pTexture_)
		{
			if (allowTextureCreation_ && Moo::rc().device())
			{
				createTexture(true);
				forceNextTextureUpdate_ = true;
			}
		}
		if (pTexture_)
		{
			// Create the update callback as we are using the texture this frame
			DWORD currentTime = timeGetTime();
			if (currentTime - prevTime_ > getDeltaRefreshInMili(currentTime) || currentTime < prevTime_ || forceNextTextureUpdate_) 
			{
				if (dynamicPage_ || pageChanged_)
				{
					prevTime_ = currentTime;
					updateBrowser();
				}
			}
			if (MozillaWebPageManager::instance().getSequenceNum(key_) != textureSequenceNum_ || forceNextTextureUpdate_) 
			{
				textureSequenceNum_ = MozillaWebPageManager::instance().getSequenceNum(key_);
				MozillaWebPageManager::instance().updateTextureDirect(key_, pTexture_, getTextureWidth(), getTextureHeight(), mipmap_, textureChkSum_);
			}
			forceNextTextureUpdate_ = false;
		}
	}
	return pTexture_.pComObject(); 
}

/**
*	Callback method for when the device is lost or changed
*/
void MozillaWebPageInterface::createUnmanagedObjects()
{
	BW_GUARD;
	//create the texture (texture size and format choosen above)	
	createTexture(true);
	//set the pageChanged_ to make sure next refresh updates our texture
	pageChanged_ = true;
}

/**
*	Callback method for when the device is lost or changed
*/
void MozillaWebPageInterface::deleteUnmanagedObjects()
{
	BW_GUARD;
	destroyTexture(false);
}

/**
*	Helper method to create the texture for the web page
*/
void MozillaWebPageInterface::createTexture(bool allowTextureCreation)
{
	BW_GUARD;

	DX::newFrame();

	allowTextureCreation_ = allowTextureCreation;
	UINT nLevels = 1;	
	DWORD usage = D3DUSAGE_DYNAMIC;
	if (mipmap_)
	{
		nLevels = 0;
		usage |= D3DUSAGE_AUTOGENMIPMAP;
	}
	pTexture_= Moo::rc().createTexture( getTextureWidth(), getTextureHeight(), nLevels, 
		usage, MozillaWebPageManager::s_format, D3DPOOL_DEFAULT, "WebPage/MozillaWebTexture/" );
	textureSequenceNum_ = -1;
	if (!pTexture_)
	{
		ERROR_MSG ("failed to create the web page texture\n");
		deleteUnmanagedObjects();
		return;
	}
	pTexture_->SetAutoGenFilterType(D3DTEXF_LINEAR);
	textureChkSum_.clear();

	D3DLOCKED_RECT rect;
	if (D3D_OK == pTexture_->LockRect( 0, &rect, NULL, D3DLOCK_DISCARD ))
	{
		uint w = getTextureWidth();
		uint h = getTextureHeight();

		int paddingPixels = ( rect.Pitch >> 2 ) - w;
		if ( paddingPixels )
		{
			uint32* dst = (uint32*)rect.pBits;
			for ( uint y = 0; y < h; y++ )
			{
				memset( dst, 0, w << 2 );
				dst += w + paddingPixels;
			}
		}
		else
		{
			memset( rect.pBits, 0, rect.Pitch * h );
		}

		pTexture_->UnlockRect( 0 );
	}
	else
	{
		ERROR_MSG("MozillaWebPageInterface failed to fill the texture");
	}

	if (mipmap_)
	{
		pTexture_->AddDirtyRect( NULL );
		pTexture_->GenerateMipSubLevels();
	}

	DX::newFrame();
}


/**
*	Helper method to destroy the texture for the web page
*/
void MozillaWebPageInterface::destroyTexture(bool allowTextureCreation)
{
	BW_GUARD;
	pTexture_ = NULL;
	//When destroying texture we only allow 
	allowTextureCreation_ = allowTextureCreation;
}


/**
*	This is the overridden textureMemoryUsed method from the BaseTexture interface,
*/
uint32 MozillaWebPageInterface::textureMemoryUsed( ) 
{ 
	BW_GUARD;
	MF_ASSERT(MozillaWebPageManager::s_format == D3DFMT_A8R8G8B8);
	return getTextureWidth() * getTextureHeight() * 4;
}

/**
*	This method gets the width of this page
*/
int MozillaWebPageInterface::getWidth() const
{
	BW_GUARD;
	return sizeInfo.getWebPageWidth();
}


/**
*	This method gets the height of this page
*/
int MozillaWebPageInterface::getHeight() const
{
	BW_GUARD;
	return sizeInfo.getWebPageHeight();
}

/**
*	This method gets the width of the texture
*/
int MozillaWebPageInterface::getTextureWidth() const
{
	BW_GUARD;
	return sizeInfo.getTextureWidth();
}


/**
*	This method gets the height of this page
*/
int MozillaWebPageInterface::getTextureHeight() const
{
	BW_GUARD;
	return sizeInfo.getTextureHeight();
}

/**
*	This is the overridden format method from the BaseTexture interface,
*/
D3DFORMAT MozillaWebPageInterface::format( ) const 
{ 
	BW_GUARD;
	return MozillaWebPageManager::s_format; 
}


/**
*	This method calls an internal python callback
*
*	@param methodName the name of the python script method to call
*	@param eventIn an eventType struct containing the utl and more details on this event.
*/
bool MozillaWebPageInterface::callMethod (const char * methodName, const LLEmbeddedBrowserWindowObserver::EventType& eventIn )
{
	BW_GUARD;
	MF_ASSERT_DEV(MainThreadTracker::isCurrentThreadMain());
	if (strcmp(methodName, MozillaWebPageManager::ON_NAVIGATION_BEGIN) == 0)
	{
		doingNavigation_ = true;
	}
	else if (strcmp(methodName, MozillaWebPageManager::ON_STATUS_TEXT_CHANGED) == 0 && bw_stricmp(eventIn.getStringValue().c_str(),"Done"))
	{
		url_ = bw_acptow(eventIn.getEventUri());
		stopDoingNavigation_ = true;
		stopDoingNavigationStartTime_ = timeGetTime();
	}
	bool handled = false;
	if (pListener_.getObject() != NULL)
	{
		PyObject * pFn = PyObject_GetAttrString( pListener_.getObject(),
			const_cast< char* >( methodName ));

		if (pFn)
		{
			PyObject * pEvent = new PyMozillaEvent( eventIn );
			PyObject * ret = Script::ask( pFn,
				PyTuple_Pack( 1, pEvent ),
				"MozillaWebPageInterface::MozillaWebPage::callMethod: ", true );
			Py_DECREF( pEvent );

			Script::setAnswer( ret, handled, "MozillaWebPageInterface callMethod retval");
		}
		else
		{
			PyErr_Clear();
		}
	}
	return handled;
}


/**
*	Clean memory if this interface is not being used
*   called by the main thread, therefore no risk of race condition on the texture.
*	@return true if memory cleaning was done
*/
bool MozillaWebPageInterface::cleanUnusedMemory()
{
	bool cleanDone = false;
	if (!pageInUse_)
	{
		if (pTexture_)
		{
			destroyTexture(true);
			cleanDone = true;
		}
	}
	//memory is now set as unused
	pageInUse_ = false;
	return cleanDone;
}

/**
*	Construction for SizeInfo used to detect our texture capabilities
*   to support fixed fucntion cards pow2 textures
*/
MozillaWebPageInterface::SizeInfo::SizeInfo() :
texturePow2_(false),
textureSquared_(false)
{
	BW_GUARD;
	const Moo::DeviceInfo& di = Moo::rc().deviceInfo( Moo::rc().deviceIndex() );

	if (di.caps_.TextureCaps & D3DPTEXTURECAPS_POW2 && !(di.caps_.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL))
	{
		texturePow2_ = true;
	}
	if 	(di.caps_.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
	{
		textureSquared_ = true;
	}
}


/**
*	Set width of the internal SizeInfo class
*   deal with width too small and with power of two width on specific FF cards
*/
void MozillaWebPageInterface::SizeInfo::setSize(int width, int height, D3DFORMAT format, WebPageProvider::eGraphicsSettingsBehaviour graphicsSettingsBehaviour, const std::string& url)
{
	BW_GUARD;
	if (width < MOZILLA_EPSILON)
	{
		ERROR_MSG("MozillaWebPageInterface::SizeInfo::setWidth width too small, overriding\n");
		webPageWidth_ = 800;
	}
	else 
	{ 
		webPageWidth_ = width;
	}
	if (height < MOZILLA_EPSILON)
	{
		ERROR_MSG("MozillaWebPageInterface::SizeInfo::setHeight height too small, overriding\n");
		webPageHeight_ = 600;
	}
	else 
	{ 
		webPageHeight_ = height;
	}

	//number of bytes used by texture
	int textureFormatBytesUsage = format == D3DFMT_R5G6B5 ? 2 : 4;
	//now calculate the texture size
	int textureMemoryLimit = 0;
	switch (WebPage::graphicsOption_)
	{
	case WebPage::GRAPHICS_OPTION_LOW:
		{
			textureMemoryLimit = MozillaWebPageManager::s_textureMemoryLimitLow;
			break;
		}
	case WebPage::GRAPHICS_OPTION_MEDIUM:
		{
			textureMemoryLimit = MozillaWebPageManager::s_textureMemoryLimitMedium;
			break;
		}
	case WebPage::GRAPHICS_OPTION_HIGH:
		{
			textureMemoryLimit = MozillaWebPageManager::s_textureMemoryLimitHigh;
			break;
		}
	}

	textureWidth_ = webPageWidth_;
	textureHeight_ = webPageHeight_;
	//adjust the texture memory to be less than textureMemoryLimit
	while(textureWidth_ * textureHeight_ * textureFormatBytesUsage > textureMemoryLimit)
	{
		textureWidth_ /= 2;
		textureHeight_ /= 2;
	}
	if (textureSquared_)
	{
		textureWidth_ = textureHeight_;
	}
	if (texturePow2_)
	{
		textureWidth_ = BW::smallerPow2(textureWidth_);
	}
	if (texturePow2_)
	{
		textureHeight_ = BW::smallerPow2(textureHeight_);
	}

	if (graphicsSettingsBehaviour == WebPageProvider::GS_EFFECT_QUALITY)
	{
		//do nothing - texture will be scaled automatically therefore losing quality
	}
	else if (graphicsSettingsBehaviour == WebPageProvider::GS_EFFECT_SCROLLING)
	{
		//make the width and height same as the texture width and height (causing the webpage
		//to probably be smaller - scrolling
		webPageWidth_ = textureWidth_;
		webPageHeight_ = textureHeight_;
	}
}


/**
*	This method gets the width of this page
*/
int MozillaWebPageInterface::SizeInfo::getWebPageWidth() const
{
	BW_GUARD;
	return webPageWidth_;
}


/**
*	This method gets the height of this page
*/
int MozillaWebPageInterface::SizeInfo::getWebPageHeight() const
{
	BW_GUARD;
	return webPageHeight_;
}




/**
*	This method gets the width of this page
*/
int MozillaWebPageInterface::SizeInfo::getTextureWidth() const
{
	BW_GUARD;
	return textureWidth_;
}


/**
*	This method gets the height of this page
*/
int MozillaWebPageInterface::SizeInfo::getTextureHeight() const
{
	BW_GUARD;
	return textureHeight_;
}
