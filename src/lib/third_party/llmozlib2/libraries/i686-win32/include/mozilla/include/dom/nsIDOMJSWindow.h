/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/base/nsIDOMJSWindow.idl
 */

#ifndef __gen_nsIDOMJSWindow_h__
#define __gen_nsIDOMJSWindow_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMJSWindow */
#define NS_IDOMJSWINDOW_IID_STR "c8188620-1dd1-11b2-bc88-df8440498add"

#define NS_IDOMJSWINDOW_IID \
  {0xc8188620, 0x1dd1, 0x11b2, \
    { 0xbc, 0x88, 0xdf, 0x84, 0x40, 0x49, 0x8a, 0xdd }}

class NS_NO_VTABLE nsIDOMJSWindow : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMJSWINDOW_IID)

  /* void dump (in DOMString str); */
  NS_IMETHOD Dump(const nsAString & str) = 0;

  /**
   * These methods take typeless arguments and optional arguments, the
   * first argument is either a function or a string, the second
   * argument must be a number (ms) and the rest of the arguments (2
   * ... n) are passed to the callback function
   */
  /* long setTimeout (); */
  NS_IMETHOD SetTimeout(PRInt32 *_retval) = 0;

  /* long setInterval (); */
  NS_IMETHOD SetInterval(PRInt32 *_retval) = 0;

  /**
   * These methods take one optional argument that's the timer ID to
   * clear. Often in existing code these methods are passed undefined,
   * which is a nop so we need to support that as well.
   */
  /* void clearTimeout (); */
  NS_IMETHOD ClearTimeout(void) = 0;

  /* void clearInterval (); */
  NS_IMETHOD ClearInterval(void) = 0;

  /* void setResizable (in boolean resizable); */
  NS_IMETHOD SetResizable(PRBool resizable) = 0;

  /* void captureEvents (in long eventFlags); */
  NS_IMETHOD CaptureEvents(PRInt32 eventFlags) = 0;

  /* void releaseEvents (in long eventFlags); */
  NS_IMETHOD ReleaseEvents(PRInt32 eventFlags) = 0;

  /* void routeEvent (in nsIDOMEvent evt); */
  NS_IMETHOD RouteEvent(nsIDOMEvent *evt) = 0;

  /* void enableExternalCapture (); */
  NS_IMETHOD EnableExternalCapture(void) = 0;

  /* void disableExternalCapture (); */
  NS_IMETHOD DisableExternalCapture(void) = 0;

  /**
   * The prompt method takes up to four arguments, the arguments are
   * message, initial prompt value, title and a save password flag
   */
  /* DOMString prompt (); */
  NS_IMETHOD Prompt(nsAString & _retval) = 0;

  /**
   * These are the scriptable versions of nsIDOMWindowInternal::open() and
   * nsIDOMWindowInternal::openDialog() that take 3 optional arguments.  Unlike
   * the nsIDOMWindowInternal methods, these methods assume that they are
   * called from JavaScript and hence will look on the JS context stack to
   * determine the caller and hence correct security context for doing their
   * search for an existing named window.  Also, these methods will set the
   * default charset on the newly opened window based on the current document
   * charset in the caller.
   */
  /* nsIDOMWindow open (); */
  NS_IMETHOD Open(nsIDOMWindow **_retval) = 0;

  /* nsIDOMWindow openDialog (); */
  NS_IMETHOD OpenDialog(nsIDOMWindow **_retval) = 0;

  /**
   * window.frames in Netscape 4.x and IE is just a reference to the
   * window itself (i.e. window.frames === window), but this doesn't
   * make sense from a generic API point of view so that's why this is
   * JS specific.
   *
   * This property is "replaceable" in JavaScript.
   */
  /* readonly attribute nsIDOMWindow frames; */
  NS_IMETHOD GetFrames(nsIDOMWindow * *aFrames) = 0;

  /* boolean find (); */
  NS_IMETHOD Find(PRBool *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMJSWINDOW \
  NS_IMETHOD Dump(const nsAString & str); \
  NS_IMETHOD SetTimeout(PRInt32 *_retval); \
  NS_IMETHOD SetInterval(PRInt32 *_retval); \
  NS_IMETHOD ClearTimeout(void); \
  NS_IMETHOD ClearInterval(void); \
  NS_IMETHOD SetResizable(PRBool resizable); \
  NS_IMETHOD CaptureEvents(PRInt32 eventFlags); \
  NS_IMETHOD ReleaseEvents(PRInt32 eventFlags); \
  NS_IMETHOD RouteEvent(nsIDOMEvent *evt); \
  NS_IMETHOD EnableExternalCapture(void); \
  NS_IMETHOD DisableExternalCapture(void); \
  NS_IMETHOD Prompt(nsAString & _retval); \
  NS_IMETHOD Open(nsIDOMWindow **_retval); \
  NS_IMETHOD OpenDialog(nsIDOMWindow **_retval); \
  NS_IMETHOD GetFrames(nsIDOMWindow * *aFrames); \
  NS_IMETHOD Find(PRBool *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMJSWINDOW(_to) \
  NS_IMETHOD Dump(const nsAString & str) { return _to Dump(str); } \
  NS_IMETHOD SetTimeout(PRInt32 *_retval) { return _to SetTimeout(_retval); } \
  NS_IMETHOD SetInterval(PRInt32 *_retval) { return _to SetInterval(_retval); } \
  NS_IMETHOD ClearTimeout(void) { return _to ClearTimeout(); } \
  NS_IMETHOD ClearInterval(void) { return _to ClearInterval(); } \
  NS_IMETHOD SetResizable(PRBool resizable) { return _to SetResizable(resizable); } \
  NS_IMETHOD CaptureEvents(PRInt32 eventFlags) { return _to CaptureEvents(eventFlags); } \
  NS_IMETHOD ReleaseEvents(PRInt32 eventFlags) { return _to ReleaseEvents(eventFlags); } \
  NS_IMETHOD RouteEvent(nsIDOMEvent *evt) { return _to RouteEvent(evt); } \
  NS_IMETHOD EnableExternalCapture(void) { return _to EnableExternalCapture(); } \
  NS_IMETHOD DisableExternalCapture(void) { return _to DisableExternalCapture(); } \
  NS_IMETHOD Prompt(nsAString & _retval) { return _to Prompt(_retval); } \
  NS_IMETHOD Open(nsIDOMWindow **_retval) { return _to Open(_retval); } \
  NS_IMETHOD OpenDialog(nsIDOMWindow **_retval) { return _to OpenDialog(_retval); } \
  NS_IMETHOD GetFrames(nsIDOMWindow * *aFrames) { return _to GetFrames(aFrames); } \
  NS_IMETHOD Find(PRBool *_retval) { return _to Find(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMJSWINDOW(_to) \
  NS_IMETHOD Dump(const nsAString & str) { return !_to ? NS_ERROR_NULL_POINTER : _to->Dump(str); } \
  NS_IMETHOD SetTimeout(PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTimeout(_retval); } \
  NS_IMETHOD SetInterval(PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetInterval(_retval); } \
  NS_IMETHOD ClearTimeout(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ClearTimeout(); } \
  NS_IMETHOD ClearInterval(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ClearInterval(); } \
  NS_IMETHOD SetResizable(PRBool resizable) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetResizable(resizable); } \
  NS_IMETHOD CaptureEvents(PRInt32 eventFlags) { return !_to ? NS_ERROR_NULL_POINTER : _to->CaptureEvents(eventFlags); } \
  NS_IMETHOD ReleaseEvents(PRInt32 eventFlags) { return !_to ? NS_ERROR_NULL_POINTER : _to->ReleaseEvents(eventFlags); } \
  NS_IMETHOD RouteEvent(nsIDOMEvent *evt) { return !_to ? NS_ERROR_NULL_POINTER : _to->RouteEvent(evt); } \
  NS_IMETHOD EnableExternalCapture(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->EnableExternalCapture(); } \
  NS_IMETHOD DisableExternalCapture(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->DisableExternalCapture(); } \
  NS_IMETHOD Prompt(nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Prompt(_retval); } \
  NS_IMETHOD Open(nsIDOMWindow **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Open(_retval); } \
  NS_IMETHOD OpenDialog(nsIDOMWindow **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->OpenDialog(_retval); } \
  NS_IMETHOD GetFrames(nsIDOMWindow * *aFrames) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFrames(aFrames); } \
  NS_IMETHOD Find(PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Find(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMJSWindow : public nsIDOMJSWindow
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMJSWINDOW

  nsDOMJSWindow();

private:
  ~nsDOMJSWindow();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMJSWindow, nsIDOMJSWindow)

nsDOMJSWindow::nsDOMJSWindow()
{
  /* member initializers and constructor code */
}

nsDOMJSWindow::~nsDOMJSWindow()
{
  /* destructor code */
}

/* void dump (in DOMString str); */
NS_IMETHODIMP nsDOMJSWindow::Dump(const nsAString & str)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* long setTimeout (); */
NS_IMETHODIMP nsDOMJSWindow::SetTimeout(PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* long setInterval (); */
NS_IMETHODIMP nsDOMJSWindow::SetInterval(PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void clearTimeout (); */
NS_IMETHODIMP nsDOMJSWindow::ClearTimeout()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void clearInterval (); */
NS_IMETHODIMP nsDOMJSWindow::ClearInterval()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setResizable (in boolean resizable); */
NS_IMETHODIMP nsDOMJSWindow::SetResizable(PRBool resizable)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void captureEvents (in long eventFlags); */
NS_IMETHODIMP nsDOMJSWindow::CaptureEvents(PRInt32 eventFlags)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void releaseEvents (in long eventFlags); */
NS_IMETHODIMP nsDOMJSWindow::ReleaseEvents(PRInt32 eventFlags)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void routeEvent (in nsIDOMEvent evt); */
NS_IMETHODIMP nsDOMJSWindow::RouteEvent(nsIDOMEvent *evt)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void enableExternalCapture (); */
NS_IMETHODIMP nsDOMJSWindow::EnableExternalCapture()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void disableExternalCapture (); */
NS_IMETHODIMP nsDOMJSWindow::DisableExternalCapture()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* DOMString prompt (); */
NS_IMETHODIMP nsDOMJSWindow::Prompt(nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMWindow open (); */
NS_IMETHODIMP nsDOMJSWindow::Open(nsIDOMWindow **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMWindow openDialog (); */
NS_IMETHODIMP nsDOMJSWindow::OpenDialog(nsIDOMWindow **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMWindow frames; */
NS_IMETHODIMP nsDOMJSWindow::GetFrames(nsIDOMWindow * *aFrames)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean find (); */
NS_IMETHODIMP nsDOMJSWindow::Find(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMJSWindow_h__ */
