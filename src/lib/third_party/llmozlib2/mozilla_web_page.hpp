#ifndef MOZILLA_WEB_PAGE_HPP
#define MOZILLA_WEB_PAGE_HPP

#include "moo/forward_declarations.hpp"
#include "moo/device_callback.hpp"
#include "web_browser_snap.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "romp/texture_feeds.hpp"
#include "web_page.hpp"
#include "third_party/LLMozlib2/llmozlib2.h"

namespace
{
	class MozillaWebPageBGTask;
}

static const int NUMBER_OF_TEXTURES = 2;

class MozillaWebPage : public WebPage, public LLEmbeddedBrowserWindowObserver
{
public:
	MozillaWebPage(uint32 width, uint32 height, const std::wstring& url);
	~MozillaWebPage();

	virtual void navigate( const std::wstring& url );
	virtual void update();
	virtual void updateBrowser();
	virtual void updateTexture();
	virtual void handleMouseButtonEvent(const Vector2& pos, bool down);
	virtual void handleKeyboardEvent(int keyCodeIn);
	virtual void handleUnicodeInput(unsigned long uni_char );
	virtual bool enableProxy( bool proxyEnabledIn, const std::string& proxyHostNameIn, int proxyPortIn );

	static void setApplicationWindow(HWND newApplicationWindow);
	HWND createExtraWindow();
	bool mozillaInit(HWND usedWindow);

	virtual void onNavigateBegin( const EventType& eventIn );
	virtual void onNavigateComplete( const EventType& eventIn );

private:

	void createUnmanagedObjects();
	void deleteUnmanagedObjects();

	void createTexture();
	void destroyTexture();

		/*
	*  It is assumed only one object/element is using the pTexture call
	*  as for concurrency reasons we are working on one texture while the other one
	*  is being refreshed.
	*/
	DX::BaseTexture*	pTexture();
	uint32				width() const;
	uint32				height() const;
	D3DFORMAT			format() const;
	uint32				textureMemoryUsed();

	void				finishInit( WebBrowserSnap* pBrowser );

//	WebBrowserSnap*					pBrowser_;
	int mWindowId_;
	uint32				width_;
	uint32				height_;
	D3DFORMAT  format_;
	
	static HWND applicationWindow_;
	static bool mozillaInit_;

	bool inNavigate_;
	ComObjectWrap<DX::Texture> pTexture_[NUMBER_OF_TEXTURES];
	int currentTextureUsed_;
	//The webpage task
	SmartPointer<MozillaWebPageBGTask> webPageTask_;
};

#endif // MOZILLA_WEB_PAGE_HPP
