/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WEB_PAGE_HPP
#define WEB_PAGE_HPP

#include "moo/forward_declarations.hpp"
#include "moo/device_callback.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "romp/texture_feeds.hpp"
#include "math/vector2.hpp"
#include "moo/graphics_settings.hpp"

class WebPage;

/*~ class BigWorld.WebPageProvider
 *
 *	This class is the wrapper class for one web page. 
 *  This class can return a PyTextureProvider for the web page and 
 *  can also be used to send event to the web page and to navigate
 *  to different web pages. These will get callbacks upon specific web page navigation events.
 *  The implementation is using the MPL LLMozlib library
 *
 *	The constructor takes 6 parameters that all have to be provided
 *
 *	The parameters are:
 *	width (uint32) - the width of the web page texture
 *	height (uint32) - the height of the web page texture
 *	dynamicPage (bool) - dynamic pages are refreshed at a higher rate
 *	mipmap (bool) - whether we want to create mipmaps for the texture or not
 *  url (unicode string) - the url to open (can be u"" to start with a blank page)
 *	graphicsSettingsBehaviour (string) - can be either "EFFECT_QUALITY" or "EFFECT_SCROLLING", this is how the 
 *		web page responds to the WEB_SETTINGS graphics setting. If EFFECT_QUALITY is used, the web page is scaled
 *		to fit the WebPageProvider, if EFFECT_SCROLLING is used, the WebPageProvider shows a smaller view of the
 *		web page.
 *
 *	For example:
 *	@{
 *
 *      webPage = BigWorld.WebPageProvider(800,600, True, True, u"", "EFFECT_QUALITY")
 *      webPage.navigate("http://www.google.com")
 *      component.texture = webPage.texture()
 *	@}
 *	This example creates a WebPageProvider which provides the texture of www.google.com to 
 *  A specific component for rendering.
 */
/**
 *	This class is the wrapper class for one web page. 
 *  This class can return a PyTextureProvider for the web page and 
 *  Can also be used to send event to the web page and to navigate
 *  To different web pages. Python observers can be given as paramaeters 
 *  To this class. These will get callbacks upon specific web page navigation events.
 */
class WebPageProvider : public PyObjectPlusWithWeakReference
{
	Py_Header( WebPageProvider, PyObjectPlus )

public:

	typedef enum
	{
		GS_EFFECT_QUALITY,
		GS_EFFECT_SCROLLING,
	} eGraphicsSettingsBehaviour;
	PY_BEGIN_ENUM_MAP( eGraphicsSettingsBehaviour, GS_ )
		PY_ENUM_VALUE( GS_EFFECT_QUALITY )
		PY_ENUM_VALUE( GS_EFFECT_SCROLLING )
	PY_END_ENUM_MAP()

	WebPageProvider(uint32 width, uint32 height, bool dynamicPage, bool mipmap, 
					const std::wstring& url, eGraphicsSettingsBehaviour graphicsSettingBehaviour, PyTypePlus * pType = &s_type_);
	~WebPageProvider();


	PY_FACTORY_DECLARE()

	void navigate( const std::wstring& url );
	const std::wstring url();

	PyTextureProviderPtr pTexture();
	void update();
	void handleMouseButtonEvent(const Vector2& pos, bool down);
	void scrollByLines(int32 lines);
	void handleMouseMove(const Vector2& pos);
	void handleKeyboardEvent(uint32 keyCodeIn);
	void handleUnicodeInput(const std::wstring& keyCodeIn);
	bool enableProxy( bool proxyEnabledIn, const std::string& proxyHostNameIn, uint32 proxyPortIn );
	bool enableCookies( bool value);
	bool navigateBack();
	bool navigateForward();
	bool navigateReload();
	bool setSize (uint32 width, uint32 height, uint32 exactWidth, uint32 exactHeight);
	void focusBrowser(bool focus, bool enable);
	void allowCursorInteraction(bool allow);
	void set404Redirect( const std::wstring& url ); 
	void setDynamic( bool dynamicPage ); 

	void listener( PyObjectPtr pListener );
	PyObjectPtr listener();

	void ratePerSecond( float value );
	float ratePerSecond();

	bool constantRefreshDuringNavigation();
	void constantRefreshDuringNavigation( bool value );


	std::string evaluateJavaScript( const std::string & script );

	uint32 width() const;
	uint32 height() const;

	PY_AUTO_METHOD_DECLARE( RETVOID, navigate, ARG( std::wstring, END ) )
	PY_AUTO_METHOD_DECLARE( RETVOID, update, END ) 
	PY_AUTO_METHOD_DECLARE( RETVOID, handleMouseButtonEvent, ARG( Vector2, ARG(bool, END ) ) )
	PY_AUTO_METHOD_DECLARE( RETVOID, scrollByLines, ARG( int32, END ) )
	PY_AUTO_METHOD_DECLARE( RETVOID, handleMouseMove, ARG( Vector2, END ) )
	PY_AUTO_METHOD_DECLARE( RETVOID, handleKeyboardEvent, ARG( uint32, END ) )
	PY_AUTO_METHOD_DECLARE( RETVOID, handleUnicodeInput, ARG( std::wstring, END ) )
	PY_AUTO_METHOD_DECLARE( RETDATA, enableProxy, ARG(bool, ARG(std::string, ARG(int, END ) ) ) ) 
	PY_AUTO_METHOD_DECLARE( RETDATA, enableCookies, ARG(bool, END ) )
	PY_AUTO_METHOD_DECLARE( RETDATA, navigateBack, END )
	PY_AUTO_METHOD_DECLARE( RETDATA, navigateForward, END )
	PY_AUTO_METHOD_DECLARE( RETDATA, navigateReload, END )
	PY_AUTO_METHOD_DECLARE( RETDATA, setSize, ARG( uint32, ARG(uint32, ARG( uint32, ARG(uint32, END ) ) ) ) )
	PY_AUTO_METHOD_DECLARE( RETVOID, focusBrowser, ARG( bool, ARG( bool, END ) ) )
	PY_AUTO_METHOD_DECLARE( RETVOID, allowCursorInteraction, ARG( bool, END ) )
	PY_AUTO_METHOD_DECLARE( RETVOID, set404Redirect, ARG( std::wstring, END ) )
	PY_AUTO_METHOD_DECLARE( RETVOID, setDynamic, ARG( bool, END ) )

	PY_RO_ATTRIBUTE_DECLARE( pTexture(), texture )
	PY_RO_ATTRIBUTE_DECLARE( url(), url )

	PY_RO_ATTRIBUTE_DECLARE( width(), width )
	PY_RO_ATTRIBUTE_DECLARE( height(), height )


	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( PyObjectPtr, listener, listener )

	PY_AUTO_METHOD_DECLARE( RETDATA, evaluateJavaScript, ARG( std::string, END ) )

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, ratePerSecond, ratePerSecond )

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, constantRefreshDuringNavigation, constantRefreshDuringNavigation )


	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );
private:
	SmartPointer<WebPage> webPage_;
};
PY_ENUM_CONVERTERS_DECLARE( WebPageProvider::eGraphicsSettingsBehaviour )

typedef SmartPointer<WebPageProvider> WebPageProviderPtr;	

/**
 *	This class is the base class for classes which provide basic web page 
 *  functionality. basically these should provide a texture method
 *  input functionality and dispatches callbacks for web page navigation events
 */
class WebPage : public Moo::DeviceCallback, public Moo::BaseTexture
{
public:
	WebPage();
	virtual ~WebPage() {};
	static void init();
	static void fini();

	virtual void navigate( const std::wstring& url ) = 0;
	virtual const std::wstring url() = 0; 

	virtual void initPage() = 0;
	virtual void update() = 0;
	virtual void updateBrowser() = 0;
	virtual void handleMouseButtonEvent(const Vector2& pos, bool down) = 0;
	virtual void scrollByLines(int32 lines) = 0;
	virtual void handleMouseMove(const Vector2& pos) = 0;
	virtual void handleKeyboardEvent(uint32 keyCodeIn) = 0;
	virtual void handleUnicodeInput(uint32 keyCodeIn) = 0;
	virtual bool enableProxy( bool proxyEnabledIn, const std::string& proxyHostNameIn, uint32 proxyPortIn ) = 0;
	virtual bool enableCookies( bool value) = 0;
	virtual bool navigateBack() = 0;
	virtual bool navigateForward() = 0;
	virtual bool navigateReload() = 0;
	virtual bool setSize(uint32 width, uint32 height, uint32 exactWidth, uint32 exactHeight) = 0;
	virtual void focusBrowser(bool focus, bool enable) = 0;
	virtual void allowCursorInteraction(bool allow) = 0;
	virtual void set404Redirect( const std::wstring& url  ) = 0;
	virtual void setDynamic( bool dynamicPage ) = 0;
	virtual std::string evaluateJavaScript( const std::string & script ) = 0;
	virtual void updateTexture() = 0;
	virtual void setRatePerSecond( float ratePerSecond ) = 0;
	virtual float getRatePerSecond() = 0;
	virtual void setConstantRefreshDuringNavigation( bool constantRefreshDuringNavigation ) = 0;
	virtual bool getConstantRefreshDuringNavigation() = 0;
	virtual void listener( PyObjectPtr pListener ) = 0;
	virtual PyObjectPtr listener() = 0;

	static std::wstring getResolvedUrl(const std::wstring& url);

	virtual void unregister() = 0;
	virtual uint32 width() const = 0;
	virtual uint32 height() const = 0;
	typedef enum
	{
		GRAPHICS_OPTION_LOW,
		GRAPHICS_OPTION_MEDIUM,
		GRAPHICS_OPTION_HIGH,
	} eGraphicsOption;
	static eGraphicsOption graphicsOption_;
	
protected:
	static void setGraphicSettings (int option);

	typedef SmartPointer<Moo::GraphicsSetting> GraphicsSettingPtr;
	static GraphicsSettingPtr s_graphicsSettingsPtr;
};

#endif // WEB_PAGE
