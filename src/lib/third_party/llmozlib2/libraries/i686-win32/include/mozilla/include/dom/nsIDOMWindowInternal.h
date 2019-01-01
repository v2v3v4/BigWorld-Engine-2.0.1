/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/base/nsIDOMWindowInternal.idl
 */

#ifndef __gen_nsIDOMWindowInternal_h__
#define __gen_nsIDOMWindowInternal_h__


#ifndef __gen_nsIDOMWindow2_h__
#include "nsIDOMWindow2.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIPrompt; /* forward declaration */

class nsIControllers; /* forward declaration */

class nsIDOMLocation; /* forward declaration */


/* starting interface:    nsIDOMWindowInternal */
#define NS_IDOMWINDOWINTERNAL_IID_STR "f914492c-0138-4123-a634-6ef8e3f126f8"

#define NS_IDOMWINDOWINTERNAL_IID \
  {0xf914492c, 0x0138, 0x4123, \
    { 0xa6, 0x34, 0x6e, 0xf8, 0xe3, 0xf1, 0x26, 0xf8 }}

class NS_NO_VTABLE nsIDOMWindowInternal : public nsIDOMWindow2 {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMWINDOWINTERNAL_IID)

  /* readonly attribute nsIDOMWindowInternal window; */
  NS_IMETHOD GetWindow(nsIDOMWindowInternal * *aWindow) = 0;

  /* readonly attribute nsIDOMWindowInternal self; */
  NS_IMETHOD GetSelf(nsIDOMWindowInternal * *aSelf) = 0;

  /* readonly attribute nsIDOMNavigator navigator; */
  NS_IMETHOD GetNavigator(nsIDOMNavigator * *aNavigator) = 0;

  /* readonly attribute nsIDOMScreen screen; */
  NS_IMETHOD GetScreen(nsIDOMScreen * *aScreen) = 0;

  /* readonly attribute nsIDOMHistory history; */
  NS_IMETHOD GetHistory(nsIDOMHistory * *aHistory) = 0;

  /* readonly attribute nsIDOMWindow content; */
  NS_IMETHOD GetContent(nsIDOMWindow * *aContent) = 0;

  /* [noscript] readonly attribute nsIPrompt prompter; */
  NS_IMETHOD GetPrompter(nsIPrompt * *aPrompter) = 0;

  /* readonly attribute nsIDOMBarProp menubar; */
  NS_IMETHOD GetMenubar(nsIDOMBarProp * *aMenubar) = 0;

  /* readonly attribute nsIDOMBarProp toolbar; */
  NS_IMETHOD GetToolbar(nsIDOMBarProp * *aToolbar) = 0;

  /* readonly attribute nsIDOMBarProp locationbar; */
  NS_IMETHOD GetLocationbar(nsIDOMBarProp * *aLocationbar) = 0;

  /* readonly attribute nsIDOMBarProp personalbar; */
  NS_IMETHOD GetPersonalbar(nsIDOMBarProp * *aPersonalbar) = 0;

  /* readonly attribute nsIDOMBarProp statusbar; */
  NS_IMETHOD GetStatusbar(nsIDOMBarProp * *aStatusbar) = 0;

  /* readonly attribute nsIDOMBarProp directories; */
  NS_IMETHOD GetDirectories(nsIDOMBarProp * *aDirectories) = 0;

  /* readonly attribute boolean closed; */
  NS_IMETHOD GetClosed(PRBool *aClosed) = 0;

  /* readonly attribute nsIDOMCrypto crypto; */
  NS_IMETHOD GetCrypto(nsIDOMCrypto * *aCrypto) = 0;

  /* readonly attribute nsIDOMPkcs11 pkcs11; */
  NS_IMETHOD GetPkcs11(nsIDOMPkcs11 * *aPkcs11) = 0;

  /* readonly attribute nsIControllers controllers; */
  NS_IMETHOD GetControllers(nsIControllers * *aControllers) = 0;

  /* attribute nsIDOMWindowInternal opener; */
  NS_IMETHOD GetOpener(nsIDOMWindowInternal * *aOpener) = 0;
  NS_IMETHOD SetOpener(nsIDOMWindowInternal * aOpener) = 0;

  /* attribute DOMString status; */
  NS_IMETHOD GetStatus(nsAString & aStatus) = 0;
  NS_IMETHOD SetStatus(const nsAString & aStatus) = 0;

  /* attribute DOMString defaultStatus; */
  NS_IMETHOD GetDefaultStatus(nsAString & aDefaultStatus) = 0;
  NS_IMETHOD SetDefaultStatus(const nsAString & aDefaultStatus) = 0;

  /* readonly attribute nsIDOMLocation location; */
  NS_IMETHOD GetLocation(nsIDOMLocation * *aLocation) = 0;

  /* attribute long innerWidth; */
  NS_IMETHOD GetInnerWidth(PRInt32 *aInnerWidth) = 0;
  NS_IMETHOD SetInnerWidth(PRInt32 aInnerWidth) = 0;

  /* attribute long innerHeight; */
  NS_IMETHOD GetInnerHeight(PRInt32 *aInnerHeight) = 0;
  NS_IMETHOD SetInnerHeight(PRInt32 aInnerHeight) = 0;

  /* attribute long outerWidth; */
  NS_IMETHOD GetOuterWidth(PRInt32 *aOuterWidth) = 0;
  NS_IMETHOD SetOuterWidth(PRInt32 aOuterWidth) = 0;

  /* attribute long outerHeight; */
  NS_IMETHOD GetOuterHeight(PRInt32 *aOuterHeight) = 0;
  NS_IMETHOD SetOuterHeight(PRInt32 aOuterHeight) = 0;

  /* attribute long screenX; */
  NS_IMETHOD GetScreenX(PRInt32 *aScreenX) = 0;
  NS_IMETHOD SetScreenX(PRInt32 aScreenX) = 0;

  /* attribute long screenY; */
  NS_IMETHOD GetScreenY(PRInt32 *aScreenY) = 0;
  NS_IMETHOD SetScreenY(PRInt32 aScreenY) = 0;

  /* readonly attribute long pageXOffset; */
  NS_IMETHOD GetPageXOffset(PRInt32 *aPageXOffset) = 0;

  /* readonly attribute long pageYOffset; */
  NS_IMETHOD GetPageYOffset(PRInt32 *aPageYOffset) = 0;

  /* readonly attribute long scrollMaxX; */
  NS_IMETHOD GetScrollMaxX(PRInt32 *aScrollMaxX) = 0;

  /* readonly attribute long scrollMaxY; */
  NS_IMETHOD GetScrollMaxY(PRInt32 *aScrollMaxY) = 0;

  /* readonly attribute unsigned long length; */
  NS_IMETHOD GetLength(PRUint32 *aLength) = 0;

  /* attribute boolean fullScreen; */
  NS_IMETHOD GetFullScreen(PRBool *aFullScreen) = 0;
  NS_IMETHOD SetFullScreen(PRBool aFullScreen) = 0;

  /* void alert (in DOMString text); */
  NS_IMETHOD Alert(const nsAString & text) = 0;

  /* boolean confirm (in DOMString text); */
  NS_IMETHOD Confirm(const nsAString & text, PRBool *_retval) = 0;

  /* DOMString prompt (in DOMString aMessage, in DOMString aInitial, in DOMString aTitle, in unsigned long aSavePassword); */
  NS_IMETHOD Prompt(const nsAString & aMessage, const nsAString & aInitial, const nsAString & aTitle, PRUint32 aSavePassword, nsAString & _retval) = 0;

  /* void focus (); */
  NS_IMETHOD Focus(void) = 0;

  /* void blur (); */
  NS_IMETHOD Blur(void) = 0;

  /* void back (); */
  NS_IMETHOD Back(void) = 0;

  /* void forward (); */
  NS_IMETHOD Forward(void) = 0;

  /* void home (); */
  NS_IMETHOD Home(void) = 0;

  /* void stop (); */
  NS_IMETHOD Stop(void) = 0;

  /* void print (); */
  NS_IMETHOD Print(void) = 0;

  /* void moveTo (in long xPos, in long yPos); */
  NS_IMETHOD MoveTo(PRInt32 xPos, PRInt32 yPos) = 0;

  /* void moveBy (in long xDif, in long yDif); */
  NS_IMETHOD MoveBy(PRInt32 xDif, PRInt32 yDif) = 0;

  /* void resizeTo (in long width, in long height); */
  NS_IMETHOD ResizeTo(PRInt32 width, PRInt32 height) = 0;

  /* void resizeBy (in long widthDif, in long heightDif); */
  NS_IMETHOD ResizeBy(PRInt32 widthDif, PRInt32 heightDif) = 0;

  /* void scroll (in long xScroll, in long yScroll); */
  NS_IMETHOD Scroll(PRInt32 xScroll, PRInt32 yScroll) = 0;

  /**
   * Open a new window with this one as the parent.  This method will
   * NOT examine the JS stack for purposes of determining a caller.
   * This window will be used for security checks during the search by
   * name and the default character set on the newly opened window
   * will just be the default character set of this window.
   */
  /* [noscript] nsIDOMWindow open (in DOMString url, in DOMString name, in DOMString options); */
  NS_IMETHOD Open(const nsAString & url, const nsAString & name, const nsAString & options, nsIDOMWindow **_retval) = 0;

  /* [noscript] nsIDOMWindow openDialog (in DOMString url, in DOMString name, in DOMString options, in nsISupports aExtraArgument); */
  NS_IMETHOD OpenDialog(const nsAString & url, const nsAString & name, const nsAString & options, nsISupports *aExtraArgument, nsIDOMWindow **_retval) = 0;

  /* void close (); */
  NS_IMETHOD Close(void) = 0;

  /* void updateCommands (in DOMString action); */
  NS_IMETHOD UpdateCommands(const nsAString & action) = 0;

  /* [noscript] boolean find (in DOMString str, in boolean caseSensitive, in boolean backwards, in boolean wrapAround, in boolean wholeWord, in boolean searchInFrames, in boolean showDialog); */
  NS_IMETHOD Find(const nsAString & str, PRBool caseSensitive, PRBool backwards, PRBool wrapAround, PRBool wholeWord, PRBool searchInFrames, PRBool showDialog, PRBool *_retval) = 0;

  /* DOMString atob (in DOMString aAsciiString); */
  NS_IMETHOD Atob(const nsAString & aAsciiString, nsAString & _retval) = 0;

  /* DOMString btoa (in DOMString aBase64Data); */
  NS_IMETHOD Btoa(const nsAString & aBase64Data, nsAString & _retval) = 0;

  /* readonly attribute nsIDOMElement frameElement; */
  NS_IMETHOD GetFrameElement(nsIDOMElement * *aFrameElement) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMWINDOWINTERNAL \
  NS_IMETHOD GetWindow(nsIDOMWindowInternal * *aWindow); \
  NS_IMETHOD GetSelf(nsIDOMWindowInternal * *aSelf); \
  NS_IMETHOD GetNavigator(nsIDOMNavigator * *aNavigator); \
  NS_IMETHOD GetScreen(nsIDOMScreen * *aScreen); \
  NS_IMETHOD GetHistory(nsIDOMHistory * *aHistory); \
  NS_IMETHOD GetContent(nsIDOMWindow * *aContent); \
  NS_IMETHOD GetPrompter(nsIPrompt * *aPrompter); \
  NS_IMETHOD GetMenubar(nsIDOMBarProp * *aMenubar); \
  NS_IMETHOD GetToolbar(nsIDOMBarProp * *aToolbar); \
  NS_IMETHOD GetLocationbar(nsIDOMBarProp * *aLocationbar); \
  NS_IMETHOD GetPersonalbar(nsIDOMBarProp * *aPersonalbar); \
  NS_IMETHOD GetStatusbar(nsIDOMBarProp * *aStatusbar); \
  NS_IMETHOD GetDirectories(nsIDOMBarProp * *aDirectories); \
  NS_IMETHOD GetClosed(PRBool *aClosed); \
  NS_IMETHOD GetCrypto(nsIDOMCrypto * *aCrypto); \
  NS_IMETHOD GetPkcs11(nsIDOMPkcs11 * *aPkcs11); \
  NS_IMETHOD GetControllers(nsIControllers * *aControllers); \
  NS_IMETHOD GetOpener(nsIDOMWindowInternal * *aOpener); \
  NS_IMETHOD SetOpener(nsIDOMWindowInternal * aOpener); \
  NS_IMETHOD GetStatus(nsAString & aStatus); \
  NS_IMETHOD SetStatus(const nsAString & aStatus); \
  NS_IMETHOD GetDefaultStatus(nsAString & aDefaultStatus); \
  NS_IMETHOD SetDefaultStatus(const nsAString & aDefaultStatus); \
  NS_IMETHOD GetLocation(nsIDOMLocation * *aLocation); \
  NS_IMETHOD GetInnerWidth(PRInt32 *aInnerWidth); \
  NS_IMETHOD SetInnerWidth(PRInt32 aInnerWidth); \
  NS_IMETHOD GetInnerHeight(PRInt32 *aInnerHeight); \
  NS_IMETHOD SetInnerHeight(PRInt32 aInnerHeight); \
  NS_IMETHOD GetOuterWidth(PRInt32 *aOuterWidth); \
  NS_IMETHOD SetOuterWidth(PRInt32 aOuterWidth); \
  NS_IMETHOD GetOuterHeight(PRInt32 *aOuterHeight); \
  NS_IMETHOD SetOuterHeight(PRInt32 aOuterHeight); \
  NS_IMETHOD GetScreenX(PRInt32 *aScreenX); \
  NS_IMETHOD SetScreenX(PRInt32 aScreenX); \
  NS_IMETHOD GetScreenY(PRInt32 *aScreenY); \
  NS_IMETHOD SetScreenY(PRInt32 aScreenY); \
  NS_IMETHOD GetPageXOffset(PRInt32 *aPageXOffset); \
  NS_IMETHOD GetPageYOffset(PRInt32 *aPageYOffset); \
  NS_IMETHOD GetScrollMaxX(PRInt32 *aScrollMaxX); \
  NS_IMETHOD GetScrollMaxY(PRInt32 *aScrollMaxY); \
  NS_IMETHOD GetLength(PRUint32 *aLength); \
  NS_IMETHOD GetFullScreen(PRBool *aFullScreen); \
  NS_IMETHOD SetFullScreen(PRBool aFullScreen); \
  NS_IMETHOD Alert(const nsAString & text); \
  NS_IMETHOD Confirm(const nsAString & text, PRBool *_retval); \
  NS_IMETHOD Prompt(const nsAString & aMessage, const nsAString & aInitial, const nsAString & aTitle, PRUint32 aSavePassword, nsAString & _retval); \
  NS_IMETHOD Focus(void); \
  NS_IMETHOD Blur(void); \
  NS_IMETHOD Back(void); \
  NS_IMETHOD Forward(void); \
  NS_IMETHOD Home(void); \
  NS_IMETHOD Stop(void); \
  NS_IMETHOD Print(void); \
  NS_IMETHOD MoveTo(PRInt32 xPos, PRInt32 yPos); \
  NS_IMETHOD MoveBy(PRInt32 xDif, PRInt32 yDif); \
  NS_IMETHOD ResizeTo(PRInt32 width, PRInt32 height); \
  NS_IMETHOD ResizeBy(PRInt32 widthDif, PRInt32 heightDif); \
  NS_IMETHOD Scroll(PRInt32 xScroll, PRInt32 yScroll); \
  NS_IMETHOD Open(const nsAString & url, const nsAString & name, const nsAString & options, nsIDOMWindow **_retval); \
  NS_IMETHOD OpenDialog(const nsAString & url, const nsAString & name, const nsAString & options, nsISupports *aExtraArgument, nsIDOMWindow **_retval); \
  NS_IMETHOD Close(void); \
  NS_IMETHOD UpdateCommands(const nsAString & action); \
  NS_IMETHOD Find(const nsAString & str, PRBool caseSensitive, PRBool backwards, PRBool wrapAround, PRBool wholeWord, PRBool searchInFrames, PRBool showDialog, PRBool *_retval); \
  NS_IMETHOD Atob(const nsAString & aAsciiString, nsAString & _retval); \
  NS_IMETHOD Btoa(const nsAString & aBase64Data, nsAString & _retval); \
  NS_IMETHOD GetFrameElement(nsIDOMElement * *aFrameElement); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMWINDOWINTERNAL(_to) \
  NS_IMETHOD GetWindow(nsIDOMWindowInternal * *aWindow) { return _to GetWindow(aWindow); } \
  NS_IMETHOD GetSelf(nsIDOMWindowInternal * *aSelf) { return _to GetSelf(aSelf); } \
  NS_IMETHOD GetNavigator(nsIDOMNavigator * *aNavigator) { return _to GetNavigator(aNavigator); } \
  NS_IMETHOD GetScreen(nsIDOMScreen * *aScreen) { return _to GetScreen(aScreen); } \
  NS_IMETHOD GetHistory(nsIDOMHistory * *aHistory) { return _to GetHistory(aHistory); } \
  NS_IMETHOD GetContent(nsIDOMWindow * *aContent) { return _to GetContent(aContent); } \
  NS_IMETHOD GetPrompter(nsIPrompt * *aPrompter) { return _to GetPrompter(aPrompter); } \
  NS_IMETHOD GetMenubar(nsIDOMBarProp * *aMenubar) { return _to GetMenubar(aMenubar); } \
  NS_IMETHOD GetToolbar(nsIDOMBarProp * *aToolbar) { return _to GetToolbar(aToolbar); } \
  NS_IMETHOD GetLocationbar(nsIDOMBarProp * *aLocationbar) { return _to GetLocationbar(aLocationbar); } \
  NS_IMETHOD GetPersonalbar(nsIDOMBarProp * *aPersonalbar) { return _to GetPersonalbar(aPersonalbar); } \
  NS_IMETHOD GetStatusbar(nsIDOMBarProp * *aStatusbar) { return _to GetStatusbar(aStatusbar); } \
  NS_IMETHOD GetDirectories(nsIDOMBarProp * *aDirectories) { return _to GetDirectories(aDirectories); } \
  NS_IMETHOD GetClosed(PRBool *aClosed) { return _to GetClosed(aClosed); } \
  NS_IMETHOD GetCrypto(nsIDOMCrypto * *aCrypto) { return _to GetCrypto(aCrypto); } \
  NS_IMETHOD GetPkcs11(nsIDOMPkcs11 * *aPkcs11) { return _to GetPkcs11(aPkcs11); } \
  NS_IMETHOD GetControllers(nsIControllers * *aControllers) { return _to GetControllers(aControllers); } \
  NS_IMETHOD GetOpener(nsIDOMWindowInternal * *aOpener) { return _to GetOpener(aOpener); } \
  NS_IMETHOD SetOpener(nsIDOMWindowInternal * aOpener) { return _to SetOpener(aOpener); } \
  NS_IMETHOD GetStatus(nsAString & aStatus) { return _to GetStatus(aStatus); } \
  NS_IMETHOD SetStatus(const nsAString & aStatus) { return _to SetStatus(aStatus); } \
  NS_IMETHOD GetDefaultStatus(nsAString & aDefaultStatus) { return _to GetDefaultStatus(aDefaultStatus); } \
  NS_IMETHOD SetDefaultStatus(const nsAString & aDefaultStatus) { return _to SetDefaultStatus(aDefaultStatus); } \
  NS_IMETHOD GetLocation(nsIDOMLocation * *aLocation) { return _to GetLocation(aLocation); } \
  NS_IMETHOD GetInnerWidth(PRInt32 *aInnerWidth) { return _to GetInnerWidth(aInnerWidth); } \
  NS_IMETHOD SetInnerWidth(PRInt32 aInnerWidth) { return _to SetInnerWidth(aInnerWidth); } \
  NS_IMETHOD GetInnerHeight(PRInt32 *aInnerHeight) { return _to GetInnerHeight(aInnerHeight); } \
  NS_IMETHOD SetInnerHeight(PRInt32 aInnerHeight) { return _to SetInnerHeight(aInnerHeight); } \
  NS_IMETHOD GetOuterWidth(PRInt32 *aOuterWidth) { return _to GetOuterWidth(aOuterWidth); } \
  NS_IMETHOD SetOuterWidth(PRInt32 aOuterWidth) { return _to SetOuterWidth(aOuterWidth); } \
  NS_IMETHOD GetOuterHeight(PRInt32 *aOuterHeight) { return _to GetOuterHeight(aOuterHeight); } \
  NS_IMETHOD SetOuterHeight(PRInt32 aOuterHeight) { return _to SetOuterHeight(aOuterHeight); } \
  NS_IMETHOD GetScreenX(PRInt32 *aScreenX) { return _to GetScreenX(aScreenX); } \
  NS_IMETHOD SetScreenX(PRInt32 aScreenX) { return _to SetScreenX(aScreenX); } \
  NS_IMETHOD GetScreenY(PRInt32 *aScreenY) { return _to GetScreenY(aScreenY); } \
  NS_IMETHOD SetScreenY(PRInt32 aScreenY) { return _to SetScreenY(aScreenY); } \
  NS_IMETHOD GetPageXOffset(PRInt32 *aPageXOffset) { return _to GetPageXOffset(aPageXOffset); } \
  NS_IMETHOD GetPageYOffset(PRInt32 *aPageYOffset) { return _to GetPageYOffset(aPageYOffset); } \
  NS_IMETHOD GetScrollMaxX(PRInt32 *aScrollMaxX) { return _to GetScrollMaxX(aScrollMaxX); } \
  NS_IMETHOD GetScrollMaxY(PRInt32 *aScrollMaxY) { return _to GetScrollMaxY(aScrollMaxY); } \
  NS_IMETHOD GetLength(PRUint32 *aLength) { return _to GetLength(aLength); } \
  NS_IMETHOD GetFullScreen(PRBool *aFullScreen) { return _to GetFullScreen(aFullScreen); } \
  NS_IMETHOD SetFullScreen(PRBool aFullScreen) { return _to SetFullScreen(aFullScreen); } \
  NS_IMETHOD Alert(const nsAString & text) { return _to Alert(text); } \
  NS_IMETHOD Confirm(const nsAString & text, PRBool *_retval) { return _to Confirm(text, _retval); } \
  NS_IMETHOD Prompt(const nsAString & aMessage, const nsAString & aInitial, const nsAString & aTitle, PRUint32 aSavePassword, nsAString & _retval) { return _to Prompt(aMessage, aInitial, aTitle, aSavePassword, _retval); } \
  NS_IMETHOD Focus(void) { return _to Focus(); } \
  NS_IMETHOD Blur(void) { return _to Blur(); } \
  NS_IMETHOD Back(void) { return _to Back(); } \
  NS_IMETHOD Forward(void) { return _to Forward(); } \
  NS_IMETHOD Home(void) { return _to Home(); } \
  NS_IMETHOD Stop(void) { return _to Stop(); } \
  NS_IMETHOD Print(void) { return _to Print(); } \
  NS_IMETHOD MoveTo(PRInt32 xPos, PRInt32 yPos) { return _to MoveTo(xPos, yPos); } \
  NS_IMETHOD MoveBy(PRInt32 xDif, PRInt32 yDif) { return _to MoveBy(xDif, yDif); } \
  NS_IMETHOD ResizeTo(PRInt32 width, PRInt32 height) { return _to ResizeTo(width, height); } \
  NS_IMETHOD ResizeBy(PRInt32 widthDif, PRInt32 heightDif) { return _to ResizeBy(widthDif, heightDif); } \
  NS_IMETHOD Scroll(PRInt32 xScroll, PRInt32 yScroll) { return _to Scroll(xScroll, yScroll); } \
  NS_IMETHOD Open(const nsAString & url, const nsAString & name, const nsAString & options, nsIDOMWindow **_retval) { return _to Open(url, name, options, _retval); } \
  NS_IMETHOD OpenDialog(const nsAString & url, const nsAString & name, const nsAString & options, nsISupports *aExtraArgument, nsIDOMWindow **_retval) { return _to OpenDialog(url, name, options, aExtraArgument, _retval); } \
  NS_IMETHOD Close(void) { return _to Close(); } \
  NS_IMETHOD UpdateCommands(const nsAString & action) { return _to UpdateCommands(action); } \
  NS_IMETHOD Find(const nsAString & str, PRBool caseSensitive, PRBool backwards, PRBool wrapAround, PRBool wholeWord, PRBool searchInFrames, PRBool showDialog, PRBool *_retval) { return _to Find(str, caseSensitive, backwards, wrapAround, wholeWord, searchInFrames, showDialog, _retval); } \
  NS_IMETHOD Atob(const nsAString & aAsciiString, nsAString & _retval) { return _to Atob(aAsciiString, _retval); } \
  NS_IMETHOD Btoa(const nsAString & aBase64Data, nsAString & _retval) { return _to Btoa(aBase64Data, _retval); } \
  NS_IMETHOD GetFrameElement(nsIDOMElement * *aFrameElement) { return _to GetFrameElement(aFrameElement); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMWINDOWINTERNAL(_to) \
  NS_IMETHOD GetWindow(nsIDOMWindowInternal * *aWindow) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetWindow(aWindow); } \
  NS_IMETHOD GetSelf(nsIDOMWindowInternal * *aSelf) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSelf(aSelf); } \
  NS_IMETHOD GetNavigator(nsIDOMNavigator * *aNavigator) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNavigator(aNavigator); } \
  NS_IMETHOD GetScreen(nsIDOMScreen * *aScreen) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScreen(aScreen); } \
  NS_IMETHOD GetHistory(nsIDOMHistory * *aHistory) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHistory(aHistory); } \
  NS_IMETHOD GetContent(nsIDOMWindow * *aContent) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetContent(aContent); } \
  NS_IMETHOD GetPrompter(nsIPrompt * *aPrompter) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrompter(aPrompter); } \
  NS_IMETHOD GetMenubar(nsIDOMBarProp * *aMenubar) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMenubar(aMenubar); } \
  NS_IMETHOD GetToolbar(nsIDOMBarProp * *aToolbar) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetToolbar(aToolbar); } \
  NS_IMETHOD GetLocationbar(nsIDOMBarProp * *aLocationbar) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLocationbar(aLocationbar); } \
  NS_IMETHOD GetPersonalbar(nsIDOMBarProp * *aPersonalbar) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPersonalbar(aPersonalbar); } \
  NS_IMETHOD GetStatusbar(nsIDOMBarProp * *aStatusbar) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStatusbar(aStatusbar); } \
  NS_IMETHOD GetDirectories(nsIDOMBarProp * *aDirectories) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDirectories(aDirectories); } \
  NS_IMETHOD GetClosed(PRBool *aClosed) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetClosed(aClosed); } \
  NS_IMETHOD GetCrypto(nsIDOMCrypto * *aCrypto) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCrypto(aCrypto); } \
  NS_IMETHOD GetPkcs11(nsIDOMPkcs11 * *aPkcs11) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPkcs11(aPkcs11); } \
  NS_IMETHOD GetControllers(nsIControllers * *aControllers) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetControllers(aControllers); } \
  NS_IMETHOD GetOpener(nsIDOMWindowInternal * *aOpener) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOpener(aOpener); } \
  NS_IMETHOD SetOpener(nsIDOMWindowInternal * aOpener) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOpener(aOpener); } \
  NS_IMETHOD GetStatus(nsAString & aStatus) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStatus(aStatus); } \
  NS_IMETHOD SetStatus(const nsAString & aStatus) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetStatus(aStatus); } \
  NS_IMETHOD GetDefaultStatus(nsAString & aDefaultStatus) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDefaultStatus(aDefaultStatus); } \
  NS_IMETHOD SetDefaultStatus(const nsAString & aDefaultStatus) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDefaultStatus(aDefaultStatus); } \
  NS_IMETHOD GetLocation(nsIDOMLocation * *aLocation) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLocation(aLocation); } \
  NS_IMETHOD GetInnerWidth(PRInt32 *aInnerWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetInnerWidth(aInnerWidth); } \
  NS_IMETHOD SetInnerWidth(PRInt32 aInnerWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetInnerWidth(aInnerWidth); } \
  NS_IMETHOD GetInnerHeight(PRInt32 *aInnerHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetInnerHeight(aInnerHeight); } \
  NS_IMETHOD SetInnerHeight(PRInt32 aInnerHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetInnerHeight(aInnerHeight); } \
  NS_IMETHOD GetOuterWidth(PRInt32 *aOuterWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOuterWidth(aOuterWidth); } \
  NS_IMETHOD SetOuterWidth(PRInt32 aOuterWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOuterWidth(aOuterWidth); } \
  NS_IMETHOD GetOuterHeight(PRInt32 *aOuterHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOuterHeight(aOuterHeight); } \
  NS_IMETHOD SetOuterHeight(PRInt32 aOuterHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOuterHeight(aOuterHeight); } \
  NS_IMETHOD GetScreenX(PRInt32 *aScreenX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScreenX(aScreenX); } \
  NS_IMETHOD SetScreenX(PRInt32 aScreenX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetScreenX(aScreenX); } \
  NS_IMETHOD GetScreenY(PRInt32 *aScreenY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScreenY(aScreenY); } \
  NS_IMETHOD SetScreenY(PRInt32 aScreenY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetScreenY(aScreenY); } \
  NS_IMETHOD GetPageXOffset(PRInt32 *aPageXOffset) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPageXOffset(aPageXOffset); } \
  NS_IMETHOD GetPageYOffset(PRInt32 *aPageYOffset) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPageYOffset(aPageYOffset); } \
  NS_IMETHOD GetScrollMaxX(PRInt32 *aScrollMaxX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScrollMaxX(aScrollMaxX); } \
  NS_IMETHOD GetScrollMaxY(PRInt32 *aScrollMaxY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScrollMaxY(aScrollMaxY); } \
  NS_IMETHOD GetLength(PRUint32 *aLength) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLength(aLength); } \
  NS_IMETHOD GetFullScreen(PRBool *aFullScreen) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFullScreen(aFullScreen); } \
  NS_IMETHOD SetFullScreen(PRBool aFullScreen) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFullScreen(aFullScreen); } \
  NS_IMETHOD Alert(const nsAString & text) { return !_to ? NS_ERROR_NULL_POINTER : _to->Alert(text); } \
  NS_IMETHOD Confirm(const nsAString & text, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Confirm(text, _retval); } \
  NS_IMETHOD Prompt(const nsAString & aMessage, const nsAString & aInitial, const nsAString & aTitle, PRUint32 aSavePassword, nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Prompt(aMessage, aInitial, aTitle, aSavePassword, _retval); } \
  NS_IMETHOD Focus(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Focus(); } \
  NS_IMETHOD Blur(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Blur(); } \
  NS_IMETHOD Back(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Back(); } \
  NS_IMETHOD Forward(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Forward(); } \
  NS_IMETHOD Home(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Home(); } \
  NS_IMETHOD Stop(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Stop(); } \
  NS_IMETHOD Print(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Print(); } \
  NS_IMETHOD MoveTo(PRInt32 xPos, PRInt32 yPos) { return !_to ? NS_ERROR_NULL_POINTER : _to->MoveTo(xPos, yPos); } \
  NS_IMETHOD MoveBy(PRInt32 xDif, PRInt32 yDif) { return !_to ? NS_ERROR_NULL_POINTER : _to->MoveBy(xDif, yDif); } \
  NS_IMETHOD ResizeTo(PRInt32 width, PRInt32 height) { return !_to ? NS_ERROR_NULL_POINTER : _to->ResizeTo(width, height); } \
  NS_IMETHOD ResizeBy(PRInt32 widthDif, PRInt32 heightDif) { return !_to ? NS_ERROR_NULL_POINTER : _to->ResizeBy(widthDif, heightDif); } \
  NS_IMETHOD Scroll(PRInt32 xScroll, PRInt32 yScroll) { return !_to ? NS_ERROR_NULL_POINTER : _to->Scroll(xScroll, yScroll); } \
  NS_IMETHOD Open(const nsAString & url, const nsAString & name, const nsAString & options, nsIDOMWindow **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Open(url, name, options, _retval); } \
  NS_IMETHOD OpenDialog(const nsAString & url, const nsAString & name, const nsAString & options, nsISupports *aExtraArgument, nsIDOMWindow **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->OpenDialog(url, name, options, aExtraArgument, _retval); } \
  NS_IMETHOD Close(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Close(); } \
  NS_IMETHOD UpdateCommands(const nsAString & action) { return !_to ? NS_ERROR_NULL_POINTER : _to->UpdateCommands(action); } \
  NS_IMETHOD Find(const nsAString & str, PRBool caseSensitive, PRBool backwards, PRBool wrapAround, PRBool wholeWord, PRBool searchInFrames, PRBool showDialog, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Find(str, caseSensitive, backwards, wrapAround, wholeWord, searchInFrames, showDialog, _retval); } \
  NS_IMETHOD Atob(const nsAString & aAsciiString, nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Atob(aAsciiString, _retval); } \
  NS_IMETHOD Btoa(const nsAString & aBase64Data, nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Btoa(aBase64Data, _retval); } \
  NS_IMETHOD GetFrameElement(nsIDOMElement * *aFrameElement) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFrameElement(aFrameElement); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMWindowInternal : public nsIDOMWindowInternal
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMWINDOWINTERNAL

  nsDOMWindowInternal();

private:
  ~nsDOMWindowInternal();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMWindowInternal, nsIDOMWindowInternal)

nsDOMWindowInternal::nsDOMWindowInternal()
{
  /* member initializers and constructor code */
}

nsDOMWindowInternal::~nsDOMWindowInternal()
{
  /* destructor code */
}

/* readonly attribute nsIDOMWindowInternal window; */
NS_IMETHODIMP nsDOMWindowInternal::GetWindow(nsIDOMWindowInternal * *aWindow)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMWindowInternal self; */
NS_IMETHODIMP nsDOMWindowInternal::GetSelf(nsIDOMWindowInternal * *aSelf)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMNavigator navigator; */
NS_IMETHODIMP nsDOMWindowInternal::GetNavigator(nsIDOMNavigator * *aNavigator)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMScreen screen; */
NS_IMETHODIMP nsDOMWindowInternal::GetScreen(nsIDOMScreen * *aScreen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMHistory history; */
NS_IMETHODIMP nsDOMWindowInternal::GetHistory(nsIDOMHistory * *aHistory)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMWindow content; */
NS_IMETHODIMP nsDOMWindowInternal::GetContent(nsIDOMWindow * *aContent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] readonly attribute nsIPrompt prompter; */
NS_IMETHODIMP nsDOMWindowInternal::GetPrompter(nsIPrompt * *aPrompter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMBarProp menubar; */
NS_IMETHODIMP nsDOMWindowInternal::GetMenubar(nsIDOMBarProp * *aMenubar)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMBarProp toolbar; */
NS_IMETHODIMP nsDOMWindowInternal::GetToolbar(nsIDOMBarProp * *aToolbar)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMBarProp locationbar; */
NS_IMETHODIMP nsDOMWindowInternal::GetLocationbar(nsIDOMBarProp * *aLocationbar)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMBarProp personalbar; */
NS_IMETHODIMP nsDOMWindowInternal::GetPersonalbar(nsIDOMBarProp * *aPersonalbar)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMBarProp statusbar; */
NS_IMETHODIMP nsDOMWindowInternal::GetStatusbar(nsIDOMBarProp * *aStatusbar)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMBarProp directories; */
NS_IMETHODIMP nsDOMWindowInternal::GetDirectories(nsIDOMBarProp * *aDirectories)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean closed; */
NS_IMETHODIMP nsDOMWindowInternal::GetClosed(PRBool *aClosed)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMCrypto crypto; */
NS_IMETHODIMP nsDOMWindowInternal::GetCrypto(nsIDOMCrypto * *aCrypto)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMPkcs11 pkcs11; */
NS_IMETHODIMP nsDOMWindowInternal::GetPkcs11(nsIDOMPkcs11 * *aPkcs11)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIControllers controllers; */
NS_IMETHODIMP nsDOMWindowInternal::GetControllers(nsIControllers * *aControllers)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIDOMWindowInternal opener; */
NS_IMETHODIMP nsDOMWindowInternal::GetOpener(nsIDOMWindowInternal * *aOpener)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMWindowInternal::SetOpener(nsIDOMWindowInternal * aOpener)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString status; */
NS_IMETHODIMP nsDOMWindowInternal::GetStatus(nsAString & aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMWindowInternal::SetStatus(const nsAString & aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString defaultStatus; */
NS_IMETHODIMP nsDOMWindowInternal::GetDefaultStatus(nsAString & aDefaultStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMWindowInternal::SetDefaultStatus(const nsAString & aDefaultStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMLocation location; */
NS_IMETHODIMP nsDOMWindowInternal::GetLocation(nsIDOMLocation * *aLocation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long innerWidth; */
NS_IMETHODIMP nsDOMWindowInternal::GetInnerWidth(PRInt32 *aInnerWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMWindowInternal::SetInnerWidth(PRInt32 aInnerWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long innerHeight; */
NS_IMETHODIMP nsDOMWindowInternal::GetInnerHeight(PRInt32 *aInnerHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMWindowInternal::SetInnerHeight(PRInt32 aInnerHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long outerWidth; */
NS_IMETHODIMP nsDOMWindowInternal::GetOuterWidth(PRInt32 *aOuterWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMWindowInternal::SetOuterWidth(PRInt32 aOuterWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long outerHeight; */
NS_IMETHODIMP nsDOMWindowInternal::GetOuterHeight(PRInt32 *aOuterHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMWindowInternal::SetOuterHeight(PRInt32 aOuterHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long screenX; */
NS_IMETHODIMP nsDOMWindowInternal::GetScreenX(PRInt32 *aScreenX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMWindowInternal::SetScreenX(PRInt32 aScreenX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long screenY; */
NS_IMETHODIMP nsDOMWindowInternal::GetScreenY(PRInt32 *aScreenY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMWindowInternal::SetScreenY(PRInt32 aScreenY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long pageXOffset; */
NS_IMETHODIMP nsDOMWindowInternal::GetPageXOffset(PRInt32 *aPageXOffset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long pageYOffset; */
NS_IMETHODIMP nsDOMWindowInternal::GetPageYOffset(PRInt32 *aPageYOffset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long scrollMaxX; */
NS_IMETHODIMP nsDOMWindowInternal::GetScrollMaxX(PRInt32 *aScrollMaxX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long scrollMaxY; */
NS_IMETHODIMP nsDOMWindowInternal::GetScrollMaxY(PRInt32 *aScrollMaxY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long length; */
NS_IMETHODIMP nsDOMWindowInternal::GetLength(PRUint32 *aLength)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean fullScreen; */
NS_IMETHODIMP nsDOMWindowInternal::GetFullScreen(PRBool *aFullScreen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMWindowInternal::SetFullScreen(PRBool aFullScreen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void alert (in DOMString text); */
NS_IMETHODIMP nsDOMWindowInternal::Alert(const nsAString & text)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean confirm (in DOMString text); */
NS_IMETHODIMP nsDOMWindowInternal::Confirm(const nsAString & text, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* DOMString prompt (in DOMString aMessage, in DOMString aInitial, in DOMString aTitle, in unsigned long aSavePassword); */
NS_IMETHODIMP nsDOMWindowInternal::Prompt(const nsAString & aMessage, const nsAString & aInitial, const nsAString & aTitle, PRUint32 aSavePassword, nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void focus (); */
NS_IMETHODIMP nsDOMWindowInternal::Focus()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void blur (); */
NS_IMETHODIMP nsDOMWindowInternal::Blur()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void back (); */
NS_IMETHODIMP nsDOMWindowInternal::Back()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void forward (); */
NS_IMETHODIMP nsDOMWindowInternal::Forward()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void home (); */
NS_IMETHODIMP nsDOMWindowInternal::Home()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void stop (); */
NS_IMETHODIMP nsDOMWindowInternal::Stop()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void print (); */
NS_IMETHODIMP nsDOMWindowInternal::Print()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void moveTo (in long xPos, in long yPos); */
NS_IMETHODIMP nsDOMWindowInternal::MoveTo(PRInt32 xPos, PRInt32 yPos)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void moveBy (in long xDif, in long yDif); */
NS_IMETHODIMP nsDOMWindowInternal::MoveBy(PRInt32 xDif, PRInt32 yDif)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void resizeTo (in long width, in long height); */
NS_IMETHODIMP nsDOMWindowInternal::ResizeTo(PRInt32 width, PRInt32 height)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void resizeBy (in long widthDif, in long heightDif); */
NS_IMETHODIMP nsDOMWindowInternal::ResizeBy(PRInt32 widthDif, PRInt32 heightDif)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void scroll (in long xScroll, in long yScroll); */
NS_IMETHODIMP nsDOMWindowInternal::Scroll(PRInt32 xScroll, PRInt32 yScroll)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] nsIDOMWindow open (in DOMString url, in DOMString name, in DOMString options); */
NS_IMETHODIMP nsDOMWindowInternal::Open(const nsAString & url, const nsAString & name, const nsAString & options, nsIDOMWindow **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] nsIDOMWindow openDialog (in DOMString url, in DOMString name, in DOMString options, in nsISupports aExtraArgument); */
NS_IMETHODIMP nsDOMWindowInternal::OpenDialog(const nsAString & url, const nsAString & name, const nsAString & options, nsISupports *aExtraArgument, nsIDOMWindow **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void close (); */
NS_IMETHODIMP nsDOMWindowInternal::Close()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void updateCommands (in DOMString action); */
NS_IMETHODIMP nsDOMWindowInternal::UpdateCommands(const nsAString & action)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] boolean find (in DOMString str, in boolean caseSensitive, in boolean backwards, in boolean wrapAround, in boolean wholeWord, in boolean searchInFrames, in boolean showDialog); */
NS_IMETHODIMP nsDOMWindowInternal::Find(const nsAString & str, PRBool caseSensitive, PRBool backwards, PRBool wrapAround, PRBool wholeWord, PRBool searchInFrames, PRBool showDialog, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* DOMString atob (in DOMString aAsciiString); */
NS_IMETHODIMP nsDOMWindowInternal::Atob(const nsAString & aAsciiString, nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* DOMString btoa (in DOMString aBase64Data); */
NS_IMETHODIMP nsDOMWindowInternal::Btoa(const nsAString & aBase64Data, nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMElement frameElement; */
NS_IMETHODIMP nsDOMWindowInternal::GetFrameElement(nsIDOMElement * *aFrameElement)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMWindowInternal_h__ */
