/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOZILLA_WEB_PAGE_INTERFACE_HPP
#define MOZILLA_WEB_PAGE_INTERFACE_HPP

#include "moo/forward_declarations.hpp"
#include "moo/device_callback.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "romp/texture_feeds.hpp"
#include "web_page.hpp"
#include "third_party/LLMozlib2/llmozlib2.h"
#include "cstdmf/safe_fifo.hpp"
#include "mozilla_web_page_command.hpp"

/**
*	This class implements WebPage as a class using LLMozlib to provide 
*  basic web functionality. basically it supports a method to provide the web
*  page texture. In addition to that it provides input functionality 
*  and dispatches callbacks for web page navigation events.
*  This class is NOT thread safe, do not currently use it in multiple threads
*  when using in multiple threads you should be extra careful as I think flash
*  is using directx directly
*/
class MozillaWebPageInterface : public WebPage
{
public:
	MozillaWebPageInterface(uint32 width, uint32 height, bool dynamicPage, bool mipmap, const std::wstring& url, WebPageProvider::eGraphicsSettingsBehaviour graphicsSettingsBehaviour);

	~MozillaWebPageInterface();

	virtual void initPage();
	virtual void navigate( const std::wstring& url );
	virtual const std::wstring url() { return url_; }
	//used by the manager to update this page about url change (might happen in multiple ways)
	virtual void setUrl(std::wstring url);
	virtual DX::BaseTexture* pTexture();
	virtual void update();
	virtual void updateBrowser();
	virtual void updateTexture();
	virtual void handleMouseButtonEvent(const Vector2& pos, bool down);
	virtual void scrollByLines(int32 lines);
	virtual void handleMouseMove(const Vector2& pos);
	virtual void handleKeyboardEvent(uint32 keyCodeIn);
	virtual void handleUnicodeInput(uint32 keyCodeIn );
	virtual bool enableProxy( bool proxyEnabledIn, const std::string& proxyHostNameIn, uint32 proxyPortIn );
	virtual bool enableCookies( bool value);
	virtual void setRatePerSecond( float ratePerSecond );
	virtual float getRatePerSecond();
	virtual void setConstantRefreshDuringNavigation( bool constantRefreshDuringNavigation );
	virtual bool getConstantRefreshDuringNavigation();
	virtual bool navigateBack();
	virtual bool navigateForward();
	virtual bool navigateReload();
	virtual bool setSize (uint32 width, uint32 height, uint32 exactWidth, uint32 exactHeight);
	virtual void focusBrowser(bool focus, bool enable);
	virtual void allowCursorInteraction(bool allow);
	virtual void set404Redirect( const std::wstring& url  ); 
	virtual void setDynamic( bool dynamicPage );
	virtual std::string evaluateJavaScript( const std::string & script );

	virtual void listener( PyObjectPtr pListener );
	virtual PyObjectPtr listener();


	virtual void unregister();
	virtual uint32 width() const;
	virtual uint32 height() const;

	//call an observer method in the listener
	bool callMethod (const char * methodName, const LLEmbeddedBrowserWindowObserver::EventType& eventIn );
	void createUnmanagedObjects();
	void deleteUnmanagedObjects();
	bool cleanUnusedMemory();

protected:
	int getWidth() const;
	int getHeight() const;

	bool setWebSize (uint32 width, uint32 height);

private:
	void createTexture(bool allowTextureCreation);
	void destroyTexture(bool allowTextureCreation);
	int getTextureWidth() const;
	int getTextureHeight() const;
	DWORD getDeltaRefreshInMili(DWORD currentTime);
	
	D3DFORMAT			format() const;

	uint32 textureMemoryUsed();

	int key_;
	ComObjectWrap<DX::Texture> pTexture_;
	MD5::Digest textureChkSum_;
	//mark that the page has changed
	bool pageChanged_;

	uint32 requestedWidth_;
	uint32 requestedHeight_;
	DWORD prevTime_;

	std::wstring	url_;
	WebPageProvider::eGraphicsSettingsBehaviour graphicsSettingsBehaviour_;
	//mark if the page uses mipmapping
	bool mipmap_;
	bool dynamicPage_;
	uint sequenceNum_;
	uint textureSequenceNum_;
	PyObjectPtr	pListener_;
	bool pageInUse_;
	bool allowTextureCreation_;
	int exactWidth_;
	int exactHeight_;
	float ratePerSecond_;
	bool doingNavigation_;
	bool constantRefreshDuringNavigation_;
	bool forceNextTextureUpdate_;
	//Flags to mark that we finished the navigation, meaning doing navigation will be automatically set 
	//to off soon and the refresh rate will drop
	bool stopDoingNavigation_;
	DWORD stopDoingNavigationStartTime_;

	/**
	* Internal class used to make sure width and height are protected.
	*/
	class SizeInfo
	{
	public:
		SizeInfo();
		int getWebPageWidth() const;
		int getWebPageHeight() const;
		int getTextureWidth() const;
		int getTextureHeight() const;
		void setSize(int width, int height, D3DFORMAT format, WebPageProvider::eGraphicsSettingsBehaviour graphicsSettingsBehaviour, const std::string& url) ;
		void setHeight(int height);

	private:
		int webPageWidth_;
		int webPageHeight_;
		int textureWidth_;
		int textureHeight_;
		bool texturePow2_;
		bool textureSquared_;
	};
	SizeInfo sizeInfo;
};

class MozillaWebPageInterface;
typedef SmartPointer<MozillaWebPageInterface> MozillaWebPageInterfacePtr;

class CommandMozillaAddInterface : public MozillaWebPageCommand
{
public:
	CommandMozillaAddInterface (int key) :
	  MozillaWebPageCommand(COMMAND_MOZILLAWEBPAGE_ADD_INTERFACE_COMMAND_TYPE, key)
	  {

	  }
	  MozillaWebPageInterfacePtr mozillaInterface;
};
#endif // MOZILLA_WEB_PAGE_HPP
