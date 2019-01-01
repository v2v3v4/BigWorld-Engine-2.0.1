#ifndef LLMOZLIB_VIRTUAL_WRAPPER
#define LLMOZLIB_VIRTUAL_WRAPPER
#include "llmozlib2.h"

//This is a class wrapping llmozlib and containing virtual methods for all llmozlib methods
// We need a virtual wrapper because:
// 1. We want to create a dll from llmozlib and can't usually call methods using dll (and don't want c wrappers for everything)
// 2. We can't just change llmozlib as changing its code will require extra work based on the llmozlib mpl license (distributing source code)

class LLMozlibVirtualWrapper
{
	public:
		LLMozlibVirtualWrapper();
		virtual ~LLMozlibVirtualWrapper();

		virtual bool init( std::string applicationDir, std::string componentDir, std::string profileDir, void* nativeWindowHandleIn );
		virtual bool reset();
		virtual bool clearCache();
		virtual int getLastError();
		virtual const std::string getVersion();
		virtual void setBrowserAgentId( std::string idIn );
		virtual bool enableProxy( bool proxyEnabledIn, std::string proxyHostNameIn, int proxyPortIn );
		virtual bool enableCookies( bool enabledIn );
		virtual bool clearAllCookies();
		virtual bool enablePlugins( bool enabledIn );
		virtual bool allowCursorInteraction ( bool allow );
		virtual int createBrowserWindow( int browserWindowWidthIn, int browserWindowHeightIn );
		virtual bool destroyBrowserWindow( int browserWindowIdIn );
		virtual bool setSize( int browserWindowIdIn, int widthIn, int heightIn );
		virtual bool scrollByLines( int browserWindowIdIn, int linesIn );
		virtual bool setBackgroundColor( int browserWindowIdIn, const int redIn, const int greenIn, const int blueIn );
		virtual bool setCaretColor( int browserWindowIdIn, const int redIn, const int greenIn, const int blueIn );
		virtual bool setEnabled( int browserWindowIdIn, bool enabledIn );
		virtual bool addObserver( int browserWindowIdIn, LLEmbeddedBrowserWindowObserver* subjectIn );
		virtual bool remObserver( int browserWindowIdIn, LLEmbeddedBrowserWindowObserver* subjectIn );
		virtual bool navigateTo( int browserWindowIdIn, const std::string uriIn );
		virtual bool navigateStop( int browserWindowIdIn );
		virtual bool canNavigateBack( int browserWindowIdIn );
		virtual bool navigateBack( int browserWindowIdIn );
		virtual bool canNavigateForward( int browserWindowIdIn );
		virtual bool navigateForward( int browserWindowIdIn );
		virtual bool navigateReload( int browserWindowIdIn );
		virtual std::string evaluateJavascript( int browserWindowIdIn, const std::string scriptIn );
		virtual void cleanMemory( int browserWindowIdIn );
		virtual bool set404RedirectUrl( int browser_window_in, std::string redirect_url );
		virtual bool clr404RedirectUrl( int browser_window_in );
		virtual const unsigned char* grabBrowserWindow( int browserWindowIdIn );		
		virtual const unsigned char* getBrowserWindowPixels( int browserWindowIdIn );	
		virtual const bool flipWindow( int browserWindowIdIn, bool flipIn );			
		virtual const int getBrowserWidth( int browserWindowIdIn );						
		virtual const int getBrowserHeight( int browserWindowIdIn );					
		virtual const int getBrowserDepth( int browserWindowIdIn );						
		virtual const int getBrowserRowSpan( int browserWindowIdIn );					
		virtual bool mouseDown( int browserWindowIdIn, int xPosIn, int yPosIn, int exactWidth, int exactHeight );		
		virtual bool mouseUp( int browserWindowIdIn, int xPosIn, int yPosIn, int exactWidth, int exactHeight );				
		virtual bool mouseMove( int browserWindowIdIn, int xPosIn, int yPosIn, int exactWidth, int exactHeight );			
		virtual bool mouseLeftDoubleClick( int browserWindowIdIn, int xPosIn, int yPosIn, int exactWidth, int exactHeight );	
		virtual bool keyPress( int browserWindowIdIn, int keyCodeIn );						
		virtual bool unicodeInput ( int browserWindowIdIn, unsigned long uni_char, unsigned long codePage );		
		virtual bool focusBrowser( int browserWindowIdIn, bool focusBrowserIn );			
		virtual void setNoFollowScheme( int browserWindowIdIn, std::string schemeIn );
		virtual std::string getNoFollowScheme( int browserWindowIdIn );

private:
	LLMozLib* llmozlib;
};

#endif
