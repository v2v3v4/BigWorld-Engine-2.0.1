#include "llmozlib_virtual_wrapper.h"
#include <string>
#include <windows.h>

LLMozlibVirtualWrapper::LLMozlibVirtualWrapper()
{
	llmozlib = LLMozLib::getInstance();
}

LLMozlibVirtualWrapper::~LLMozlibVirtualWrapper()
{

}

bool LLMozlibVirtualWrapper::init( std::string applicationDir, std::string componentDir, std::string profileDir, void* nativeWindowHandleIn )
{
  return llmozlib->init( applicationDir, componentDir, profileDir, nativeWindowHandleIn );
}

bool LLMozlibVirtualWrapper::reset()
{
  return llmozlib->reset();
}

bool LLMozlibVirtualWrapper::clearCache()
{
  return llmozlib->clearCache();
}

int LLMozlibVirtualWrapper::getLastError()
{
  return llmozlib->getLastError();
}

const std::string LLMozlibVirtualWrapper::getVersion()
{
	return llmozlib->getVersion();
}

void LLMozlibVirtualWrapper::setBrowserAgentId( std::string idIn )
{
  llmozlib->setBrowserAgentId( idIn );
}

bool LLMozlibVirtualWrapper::enableProxy( bool proxyEnabledIn, std::string proxyHostNameIn, int proxyPortIn )
{
  return llmozlib->enableProxy( proxyEnabledIn, proxyHostNameIn, proxyPortIn );
}

bool LLMozlibVirtualWrapper::enableCookies( bool enabledIn )
{
  return llmozlib->enableCookies( enabledIn );
}

bool LLMozlibVirtualWrapper::clearAllCookies()
{
  return llmozlib->clearAllCookies();
}

bool LLMozlibVirtualWrapper::enablePlugins( bool enabledIn )
{
  return llmozlib->enablePlugins( enabledIn );
}

bool LLMozlibVirtualWrapper::allowCursorInteraction( bool allow )
{
  return llmozlib->allowCursorInteraction( allow );
}

int LLMozlibVirtualWrapper::createBrowserWindow( int browserWindowWidthIn, int browserWindowHeightIn )
{
  return llmozlib->createBrowserWindow( browserWindowWidthIn,  browserWindowHeightIn );
}

bool LLMozlibVirtualWrapper::destroyBrowserWindow( int browserWindowIdIn )
{
  return llmozlib->destroyBrowserWindow( browserWindowIdIn );
}

bool LLMozlibVirtualWrapper::setSize( int browserWindowIdIn, int widthIn, int heightIn )
{
  return llmozlib->setSize( browserWindowIdIn, widthIn, heightIn );
}

bool LLMozlibVirtualWrapper::scrollByLines( int browserWindowIdIn, int linesIn )
{
  return llmozlib->scrollByLines( browserWindowIdIn, linesIn );
}

bool LLMozlibVirtualWrapper::setBackgroundColor( int browserWindowIdIn, const int redIn, const int greenIn, const int blueIn )
{
  return llmozlib->setBackgroundColor( browserWindowIdIn, redIn,  greenIn, blueIn );
}

bool LLMozlibVirtualWrapper::setCaretColor( int browserWindowIdIn, const int redIn, const int greenIn, const int blueIn )
{
  return llmozlib->setCaretColor( browserWindowIdIn, redIn, greenIn, blueIn );
}

bool LLMozlibVirtualWrapper::setEnabled( int browserWindowIdIn, bool enabledIn )
{
  return llmozlib->setEnabled( browserWindowIdIn, enabledIn );
}

bool LLMozlibVirtualWrapper::addObserver( int browserWindowIdIn, LLEmbeddedBrowserWindowObserver* subjectIn )
{
  return llmozlib->addObserver( browserWindowIdIn, subjectIn );
}

bool LLMozlibVirtualWrapper::remObserver( int browserWindowIdIn, LLEmbeddedBrowserWindowObserver* subjectIn )
{
  return llmozlib->remObserver( browserWindowIdIn, subjectIn );
}

bool LLMozlibVirtualWrapper::navigateTo( int browserWindowIdIn, const std::string uriIn )
{
  return llmozlib->navigateTo( browserWindowIdIn, uriIn );
}

bool LLMozlibVirtualWrapper::navigateStop( int browserWindowIdIn )
{
  return llmozlib->navigateStop( browserWindowIdIn );
}

bool LLMozlibVirtualWrapper::canNavigateBack( int browserWindowIdIn )
{
  return llmozlib->canNavigateBack(browserWindowIdIn );
}

bool LLMozlibVirtualWrapper::navigateBack( int browserWindowIdIn )
{
  return llmozlib->navigateBack( browserWindowIdIn );
}

bool LLMozlibVirtualWrapper::canNavigateForward( int browserWindowIdIn )
{
  return llmozlib->canNavigateForward( browserWindowIdIn );
}

bool LLMozlibVirtualWrapper::navigateForward( int browserWindowIdIn )
{
  return llmozlib->navigateForward(browserWindowIdIn );
}

bool LLMozlibVirtualWrapper::navigateReload( int browserWindowIdIn )
{
  return llmozlib->navigateReload(browserWindowIdIn );
}

std::string LLMozlibVirtualWrapper:: evaluateJavascript( int browserWindowIdIn, const std::string scriptIn )
{
  return llmozlib->evaluateJavascript(browserWindowIdIn, scriptIn );
}

void LLMozlibVirtualWrapper::cleanMemory( int browserWindowIdIn )
{
  return llmozlib->cleanMemory(browserWindowIdIn );
}

bool LLMozlibVirtualWrapper::set404RedirectUrl( int browser_window_in, std::string redirect_url )
{
  return llmozlib->set404RedirectUrl( browser_window_in, redirect_url );
}

bool LLMozlibVirtualWrapper::clr404RedirectUrl( int browser_window_in )
{
  return llmozlib->clr404RedirectUrl(browser_window_in );
}

const unsigned char* LLMozlibVirtualWrapper::grabBrowserWindow( int browserWindowIdIn )
{
  return llmozlib->grabBrowserWindow(browserWindowIdIn );	      
}

const unsigned char* LLMozlibVirtualWrapper::getBrowserWindowPixels( int browserWindowIdIn )
{
  return llmozlib->getBrowserWindowPixels(browserWindowIdIn );;
}

const bool LLMozlibVirtualWrapper::flipWindow( int browserWindowIdIn, bool flipIn )		       
{
  return llmozlib->flipWindow( browserWindowIdIn, flipIn );		      
}

const int LLMozlibVirtualWrapper::getBrowserWidth( int browserWindowIdIn )					       
{
  return llmozlib->getBrowserWidth( browserWindowIdIn );					      
}

const int LLMozlibVirtualWrapper::getBrowserHeight( int browserWindowIdIn )				       
{
  return llmozlib->getBrowserHeight(browserWindowIdIn );				      
}

const int LLMozlibVirtualWrapper::getBrowserDepth( int browserWindowIdIn )					       
{
  return llmozlib->getBrowserDepth( browserWindowIdIn );					      
}

const int LLMozlibVirtualWrapper::getBrowserRowSpan( int browserWindowIdIn )				       
{
  return llmozlib->getBrowserRowSpan(browserWindowIdIn );				      
}

bool LLMozlibVirtualWrapper::mouseDown( int browserWindowIdIn, int xPosIn, int yPosIn, int exactWidth, int exactHeight )	       
{
  return llmozlib->mouseDown(browserWindowIdIn, xPosIn, yPosIn, exactWidth, exactHeight );	      
}

bool LLMozlibVirtualWrapper::mouseUp( int browserWindowIdIn, int xPosIn, int yPosIn, int exactWidth, int exactHeight )			       
{
  return llmozlib->mouseUp( browserWindowIdIn, xPosIn, yPosIn, exactWidth, exactHeight );			      
}

bool LLMozlibVirtualWrapper::mouseMove( int browserWindowIdIn, int xPosIn, int yPosIn, int exactWidth, int exactHeight )		       
{
  return llmozlib->mouseMove( browserWindowIdIn, xPosIn, yPosIn, exactWidth, exactHeight );		      
}

bool LLMozlibVirtualWrapper::mouseLeftDoubleClick( int browserWindowIdIn, int xPosIn, int yPosIn, int exactWidth, int exactHeight  )  
{
  return llmozlib->mouseLeftDoubleClick( browserWindowIdIn, xPosIn, yPosIn, exactWidth, exactHeight ); 
}

bool LLMozlibVirtualWrapper::keyPress( int browserWindowIdIn, int keyCodeIn )					       
{
  return llmozlib->keyPress( browserWindowIdIn, keyCodeIn );					      
}

bool LLMozlibVirtualWrapper::unicodeInput ( int browserWindowIdIn, unsigned long uni_char, unsigned long codePage )	       
{
  return llmozlib->unicodeInput ( browserWindowIdIn, uni_char, codePage );	      
}

bool LLMozlibVirtualWrapper::focusBrowser( int browserWindowIdIn, bool focusBrowserIn )		       
{
  return llmozlib->focusBrowser( browserWindowIdIn, focusBrowserIn );		      
}

void LLMozlibVirtualWrapper::setNoFollowScheme( int browserWindowIdIn, std::string schemeIn )
{
  llmozlib->setNoFollowScheme( browserWindowIdIn, schemeIn );
}

std::string LLMozlibVirtualWrapper::getNoFollowScheme( int browserWindowIdIn )
{
  return llmozlib->getNoFollowScheme( browserWindowIdIn );
}

