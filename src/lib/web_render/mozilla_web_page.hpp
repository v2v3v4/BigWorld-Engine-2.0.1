/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOZILLA_WEB_PAGE_HPP
#define MOZILLA_WEB_PAGE_HPP

#include "moo/forward_declarations.hpp"
#include "moo/device_callback.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "romp/texture_feeds.hpp"
#include "web_page.hpp"
#include "third_party/LLMozlib2/llmozlib2.h"
#include "cstdmf/safe_fifo.hpp"
#include "mozilla_web_page_command.hpp"

class MozillaWebPageInterface;
typedef SmartPointer<MozillaWebPageInterface> MozillaWebPageInterfacePtr;

/**
*  This class implements a background thread which services commands for updating
*  Web pages. Each web page is identified by a key and is implemented by a MozillaWebPage instance.
*  Commands are send by the MozillaWebPageInterface
*  In addition to that, the tick method (being ticked by the main thread) is 
*  In charge of sending callbacks back from the MozillaWebPage into the MozillaWebPageInterface
*/
class MozillaWebPageManager : public Singleton< MozillaWebPageManager >, public SimpleThread
{
public:

	MozillaWebPageManager();
	~MozillaWebPageManager();
	static void s_run(const DataSectionPtr& configSection);
	static void s_mainLoop(void* );
	void mainLoopInternal();
	void pushCommand(MozillaWebPageCommandPtr& command);
	void pushCallback(MozillaWebPageCommandPtr& command);
	void updateTextureDirect(int key, ComObjectWrap<DX::Texture>& destTexture, uint textureWidth, uint textureHeight, bool mipmap, MD5::Digest& textureChkSum);
	uint getSequenceNum(int key);
	void addInterface(int key, MozillaWebPageInterfacePtr mozillaInterface);
	void removeInterface(int key);
	uint getNextKey();

	static bool s_fini(bool keepInstance);
	bool fini();
	static bool s_tick(float dTime);
	bool tick(float dTime);
	static bool bruteFini();

	//static initialisation methods
	static void setApplicationWindow(HWND newApplicationWindow);

	static uint32 s_textureMemoryLimitLow;
	static uint32 s_textureMemoryLimitMedium;
	static uint32 s_textureMemoryLimitHigh;
	static uint32 s_cleanMemoryConstant;
	static volatile bool s_isValid;
	//the default sequence num to start with
	static const uint s_defaultSequenceNum;
	static D3DFORMAT s_format;
	void updateTexturesBasedOnGraphicSettings();
	bool drawWeb() const;

	static const char* ON_NAVIGATION_BEGIN;
	static const char* ON_NAVIGATION_COMPLETE;
	static const char* ON_STATUS_TEXT_CHANGED;

private:
	void processCommand(MozillaWebPageCommandPtr& command);
	void processCommandInternal(MozillaWebPageCommandPtr& command);
	void internalProcessCallbacks();	
	void internalCleanUnusedMemory();
	static bool init(const DataSectionPtr& configSection);
	static void s_chooseFormat();

	static HWND createExtraWindow();
	static void deleteExtraWindow();
	static HWND s_applicationWindow;
	static HWND s_usedWindow;

	class MozillaWebPage;
	typedef SmartPointer<MozillaWebPage> MozillaWebPagePtr;
	//to protect internal data
	mutable SimpleMutex managerMutex_;
	//commands fifo
	SafeFifo<MozillaWebPageCommandPtr > commandFifo_;
	//callbacks fifo
	SafeFifo<MozillaWebPageCommandPtr > callbackFifo_;

	volatile int runningState_;
	typedef std::map<int, MozillaWebPagePtr> MapIntMozillaWebPage;
	typedef std::map<int, MozillaWebPageInterfacePtr> MapIntMozillaWebPageInterface;
	typedef std::map<int, unsigned char*> MapSizeAllocatedMemory;
	//map between the key and the MozillaWebPage
	MapIntMozillaWebPage mapIntMozillaWebPage_;
	//map between the key and the MozillaWebPageInterface
	MapIntMozillaWebPageInterface mapIntMozillaWebPageInterface_;
	//the used key
	int key_;
	float timeSinceLastMemClean_;
	bool drawWeb_;
	bool bruteExit_;

private:
	/**
	*	This class implements WebPage as a class using LLMozlib to provide 
	*  basic web functionality. basically it supports a method to provide the web
	*  page texture. In addition to that it provides input functionality 
	*  and dispatches callbacks for web page navigation events.
	*  This class is used by the MozillaWebPageManager to provide the web functionality 
	*  using a background thread.
	*/
	class MozillaWebPage : public LLEmbeddedBrowserWindowObserver, public SafeReferenceCount
	{
	public:
		MozillaWebPage(uint32 width, uint32 height, bool mipmap, const std::wstring& url, WebPageProvider::eGraphicsSettingsBehaviour graphicsSettingsBehaviour, int key);
		~MozillaWebPage();

		virtual void navigate( const std::wstring& url );
		virtual void updateBrowser(uint sequenceNum);
		virtual void updateTextureDirect(ComObjectWrap<DX::Texture>& destTexture, uint textureWidth, uint textureHeight, bool mipmap, MD5::Digest& textureChkSum);
		uint getSequenceNum();
		virtual void handleMouseButtonEvent(const Vector2& pos, bool down);
		virtual void scrollByLines(int32 lines);
		virtual void handleMouseMove(const Vector2& pos);
		virtual void handleKeyboardEvent(uint32 keyCodeIn);
		virtual void handleUnicodeInput(uint32 keyCodeIn, uint32 codePage );
		virtual bool enableProxy( bool proxyEnabledIn, const std::string& proxyHostNameIn, uint32 proxyPortIn );
		virtual bool enableCookies( bool value);
		virtual bool navigateBack();
		virtual bool navigateForward();
		virtual bool navigateReload();
		virtual bool setSize (uint32 width, uint32 height, uint32 textureWidth, uint32 textureHeight, uint32 exactWidth, uint32 exactHeight);
		virtual void focusBrowser(bool focus, bool enable);
		virtual void allowCursorInteraction(bool allow);
		virtual void set404Redirect( const std::wstring& url  ); 
		virtual std::string evaluateJavaScript( const std::string & script );
		virtual bool MozillaWebPage::cleanUnusedMemory();

		//observer methods
		virtual void onPageChanged( const EventType& eventIn );
		virtual void onNavigateBegin( const EventType& eventIn );
		virtual void onNavigateComplete( const EventType& eventIn ) ;
		virtual void onUpdateProgress( const EventType& eventIn );
		virtual void onStatusTextChange( const EventType& eventIn );
		virtual void onLocationChange( const EventType& eventIn );
		virtual void onClickLinkHref( const EventType& eventIn );
		virtual void onClickLinkNoFollow( const EventType& eventIn );

		bool isValid(const char* file, int line);

		uint webPageWidth_;
		uint webPageHeight_;
	private:
		MozillaWebPage( const MozillaWebPage & );
		MozillaWebPage & operator = ( const MozillaWebPage & );
		void copyPixelsIntoCopy(const unsigned char* pixels);

		//to protect the internal buffer
		mutable SimpleMutex mozillaWebPageMutex_;
		unsigned char* pixelsCopy_;
		uint32 pixelsSize_;
		volatile uint sequenceNum_;
		void clearPixels();
		void createBrowserWindow();
		void destroyBrowserWindow();
		int mWindowId_;
		uint browserRowSpan_;
		bool currentFocus_;
		std::wstring	initialUrl_;
		int key_;
		MD5::Digest chkSum_;
		uint textureWidth_;
		uint textureHeight_;
		uint exactWidth_;
		uint exactHeight_;
		//store the real sizes used to create the pixels to prevent race condition with a set size request.
		uint realPixelsWidth_;
		uint realPixelsHeight_;
	};
};
#endif // MOZILLA_WEB_PAGE_HPP
