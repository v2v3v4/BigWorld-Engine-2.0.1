/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/base/public/nsIXMLHttpRequest.idl
 */

#ifndef __gen_nsIXMLHttpRequest_h__
#define __gen_nsIXMLHttpRequest_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMDocument; /* forward declaration */

class nsIDOMEventListener; /* forward declaration */

class nsIChannel; /* forward declaration */

class nsIVariant; /* forward declaration */


/* starting interface:    nsIXMLHttpRequest */
#define NS_IXMLHTTPREQUEST_IID_STR "7b3b3c62-9d53-4abc-bc89-c33ce78f439f"

#define NS_IXMLHTTPREQUEST_IID \
  {0x7b3b3c62, 0x9d53, 0x4abc, \
    { 0xbc, 0x89, 0xc3, 0x3c, 0xe7, 0x8f, 0x43, 0x9f }}

/**
 * Mozilla's XMLHttpRequest is modelled after Microsoft's IXMLHttpRequest
 * object. The goal has been to make Mozilla's version match Microsoft's
 * version as closely as possible, but there are bound to be some differences.
 *
 * In general, Microsoft's documentation for IXMLHttpRequest can be used.
 * Mozilla's interface definitions provide some additional documentation. The
 * web page to look at is http://www.mozilla.org/xmlextras/
 *
 * Mozilla's XMLHttpRequest object can be created in JavaScript like this:
 *   new XMLHttpRequest()
 * compare to Internet Explorer:
 *   new ActiveXObject("Msxml2.XMLHTTP")
 *
 * From JavaScript, the methods and properties visible in the XMLHttpRequest
 * object are a combination of nsIXMLHttpRequest and nsIJSXMLHttpRequest;
 * there is no need to differentiate between those interfaces.
 *
 * From native code, the way to set up onload and onerror handlers is a bit
 * different. Here is a comment from Johnny Stenback <jst@netscape.com>:
 *
 *   The mozilla implementation of nsIXMLHttpRequest implements the interface
 *   nsIDOMEventTarget and that's how you're supported to add event listeners.
 *   Try something like this:
 *
 *   nsCOMPtr<nsIDOMEventTarget> target(do_QueryInterface(myxmlhttpreq));
 *
 *   target->AddEventListener(NS_LITERAL_STRING("load"), mylistener,
 *                            PR_FALSE)
 *
 *   where mylistener is your event listener object that implements the
 *   interface nsIDOMEventListener.
 *
 *   The 'onload' and 'onerror' attributes moved to nsIJSXMLHttRequest,
 *   but if you're coding in C++ you should avoid using those.
 */
class NS_NO_VTABLE nsIXMLHttpRequest : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXMLHTTPREQUEST_IID)

  /**
   * The request uses a channel in order to perform the
   * request.  This attribute represents the channel used
   * for the request.  NULL if the channel has not yet been
   * created.
   *
   * In a multipart request case, this is the initial channel, not the
   * different parts in the multipart request.
   *
   * Mozilla only. Requires elevated privileges to access.
   */
  /* readonly attribute nsIChannel channel; */
  NS_IMETHOD GetChannel(nsIChannel * *aChannel) = 0;

  /**
   * The response to the request is parsed as if it were a
   * text/xml stream. This attributes represents the response as
   * a DOM Document object. NULL if the request is unsuccessful or
   * has not yet been sent.
   */
  /* readonly attribute nsIDOMDocument responseXML; */
  NS_IMETHOD GetResponseXML(nsIDOMDocument * *aResponseXML) = 0;

  /**
   * The response to the request as text.
   * NULL if the request is unsuccessful or
   * has not yet been sent.
   */
  /* readonly attribute AString responseText; */
  NS_IMETHOD GetResponseText(nsAString & aResponseText) = 0;

  /**
   * The status of the response to the request for HTTP requests.
   */
  /* readonly attribute unsigned long status; */
  NS_IMETHOD GetStatus(PRUint32 *aStatus) = 0;

  /**
   * The string representing the status of the response for
   * HTTP requests.
   */
  /* readonly attribute AUTF8String statusText; */
  NS_IMETHOD GetStatusText(nsACString & aStatusText) = 0;

  /**
   * If the request has been sent already, this method will
   * abort the request.
   */
  /* void abort (); */
  NS_IMETHOD Abort(void) = 0;

  /**
   * Returns all of the response headers as a string for HTTP
   * requests.
   *
   * Note that this will return all the headers from the *current*
   * part of a multipart request, not from the original channel.
   *
   * @returns A string containing all of the response headers.
   *          NULL if the response has not yet been received.
   */
  /* string getAllResponseHeaders (); */
  NS_IMETHOD GetAllResponseHeaders(char **_retval) = 0;

  /**
   * Returns the text of the header with the specified name for
   * HTTP requests.
   *
   * @param header The name of the header to retrieve
   * @returns A string containing the text of the header specified.
   *          NULL if the response has not yet been received or the
   *          header does not exist in the response.
   */
  /* ACString getResponseHeader (in AUTF8String header); */
  NS_IMETHOD GetResponseHeader(const nsACString & header, nsACString & _retval) = 0;

  /**
   * Native (non-script) method to initialize a request. Note that
   * the request is not sent until the <code>send</code> method
   * is invoked.
   *
   * Will abort currently active loads.
   *
   * After the initial response, all event listeners will be cleared.
   * Call open() before setting new event listeners.
   *
   * @param method The HTTP method, for example "POST" or "GET". Ignored
   *               if the URL is not a HTTP(S) URL.
   * @param url The URL to which to send the request.
   * @param async Whether the request is synchronous or asynchronous
   *              i.e. whether send returns only after the response
   *              is received or if it returns immediately after
   *              sending the request. In the latter case, notification
   *              of completion is sent through the event listeners.
   *              This argument must be true if the multipart
   *              attribute has been set to true, or an exception will
   *              be thrown.
   * @param user A username for authentication if necessary.
   * @param password A password for authentication if necessary.
   */
  /* [noscript] void openRequest (in AUTF8String method, in AUTF8String url, in boolean async, in AString user, in AString password); */
  NS_IMETHOD OpenRequest(const nsACString & method, const nsACString & url, PRBool async, const nsAString & user, const nsAString & password) = 0;

  /**
   * Meant to be a script-only method for initializing a request.
   * The parameters are similar to the ones detailed in the
   * description of <code>openRequest</code>, but the last
   * 3 are optional.
   *
   * Will abort currently active loads.
   *
   * After the initial response, all event listeners will be cleared.
   * Call open() before setting new event listeners.
   *
   * @param method The HTTP method - either "POST" or "GET". Ignored
   *               if the URL is not a HTTP URL.
   * @param url The URL to which to send the request.
   * @param async (optional) Whether the request is synchronous or
   *              asynchronous i.e. whether send returns only after
   *              the response is received or if it returns immediately after
   *              sending the request. In the latter case, notification
   *              of completion is sent through the event listeners.
   *              The default value is true.
   *              This argument must be true if the multipart
   *              attribute has been set to true, or an exception will
   *              be thrown.
   * @param user (optional) A username for authentication if necessary.
   *             The default value is the empty string
   * @param password (optional) A password for authentication if necessary.
   *                 The default value is the empty string
   */
  /* void open (in AUTF8String method, in AUTF8String url); */
  NS_IMETHOD Open(const nsACString & method, const nsACString & url) = 0;

  /**
   * Sends the request. If the request is asynchronous, returns
   * immediately after sending the request. If it is synchronous
   * returns only after the response has been received.
   *
   * After the initial response, all event listeners will be cleared.
   * Call open() before setting new event listeners.
   *
   * @param body Either an instance of nsIDOMDocument, nsIInputStream
   *             or a string (nsISupportsString in the native calling
   *             case). This is used to populate the body of the
   *             HTTP request if the HTTP request method is "POST".
   *             If the parameter is a nsIDOMDocument, it is serialized.
   *             If the parameter is a nsIInputStream, then it must be
   *             compatible with nsIUploadChannel.setUploadStream, and a
   *             Content-Length header will be added to the HTTP request
   *             with a value given by nsIInputStream.available.  Any
   *             headers included at the top of the stream will be
   *             treated as part of the message body.  The MIME type of
   *             the stream should be specified by setting the Content-
   *             Type header via the setRequestHeader method before
   *             calling send.
   */
  /* void send (in nsIVariant body); */
  NS_IMETHOD Send(nsIVariant *body) = 0;

  /**
   * Sets a HTTP request header for HTTP requests. You must call open
   * before setting the request headers.
   *
   * @param header The name of the header to set in the request.
   * @param value The body of the header.
   */
  /* void setRequestHeader (in AUTF8String header, in AUTF8String value); */
  NS_IMETHOD SetRequestHeader(const nsACString & header, const nsACString & value) = 0;

  /**
   * The state of the request.
   *
   * Possible values:
   *   0 UNINITIALIZED open() has not been called yet.
   *   1 LOADING       send() has not been called yet.
   *   2 LOADED        send() has been called, headers and status are available.
   *   3 INTERACTIVE   Downloading, responseText holds the partial data.
   *   4 COMPLETED     Finished with all operations.
   */
  /* readonly attribute long readyState; */
  NS_IMETHOD GetReadyState(PRInt32 *aReadyState) = 0;

  /**
   * Override the mime type returned by the server (if any). This may
   * be used, for example, to force a stream to be treated and parsed
   * as text/xml, even if the server does not report it as such. This
   * must be done before the <code>send</code> method is invoked.
   *
   * @param mimetype The type used to override that returned by the server
   *                 (if any).
   */
  /* void overrideMimeType (in AUTF8String mimetype); */
  NS_IMETHOD OverrideMimeType(const nsACString & mimetype) = 0;

  /**
   * Set to true if the response is expected to be a stream of
   * possibly multiple (XML) documents. If set to true, the content
   * type of the initial response must be multipart/x-mixed-replace or
   * an error will be triggerd. All requests must be asynchronous.
   *
   * This enables server push. For each XML document that's written to
   * this request, a new XML DOM document is created and the onload
   * handler is called inbetween documents. Note that when this is
   * set, the onload handler and other event handlers are not reset
   * after the first XML document is loaded, and the onload handler
   * will be called as each part of the response is received.
   */
  /* attribute boolean multipart; */
  NS_IMETHOD GetMultipart(PRBool *aMultipart) = 0;
  NS_IMETHOD SetMultipart(PRBool aMultipart) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXMLHTTPREQUEST \
  NS_IMETHOD GetChannel(nsIChannel * *aChannel); \
  NS_IMETHOD GetResponseXML(nsIDOMDocument * *aResponseXML); \
  NS_IMETHOD GetResponseText(nsAString & aResponseText); \
  NS_IMETHOD GetStatus(PRUint32 *aStatus); \
  NS_IMETHOD GetStatusText(nsACString & aStatusText); \
  NS_IMETHOD Abort(void); \
  NS_IMETHOD GetAllResponseHeaders(char **_retval); \
  NS_IMETHOD GetResponseHeader(const nsACString & header, nsACString & _retval); \
  NS_IMETHOD OpenRequest(const nsACString & method, const nsACString & url, PRBool async, const nsAString & user, const nsAString & password); \
  NS_IMETHOD Open(const nsACString & method, const nsACString & url); \
  NS_IMETHOD Send(nsIVariant *body); \
  NS_IMETHOD SetRequestHeader(const nsACString & header, const nsACString & value); \
  NS_IMETHOD GetReadyState(PRInt32 *aReadyState); \
  NS_IMETHOD OverrideMimeType(const nsACString & mimetype); \
  NS_IMETHOD GetMultipart(PRBool *aMultipart); \
  NS_IMETHOD SetMultipart(PRBool aMultipart); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXMLHTTPREQUEST(_to) \
  NS_IMETHOD GetChannel(nsIChannel * *aChannel) { return _to GetChannel(aChannel); } \
  NS_IMETHOD GetResponseXML(nsIDOMDocument * *aResponseXML) { return _to GetResponseXML(aResponseXML); } \
  NS_IMETHOD GetResponseText(nsAString & aResponseText) { return _to GetResponseText(aResponseText); } \
  NS_IMETHOD GetStatus(PRUint32 *aStatus) { return _to GetStatus(aStatus); } \
  NS_IMETHOD GetStatusText(nsACString & aStatusText) { return _to GetStatusText(aStatusText); } \
  NS_IMETHOD Abort(void) { return _to Abort(); } \
  NS_IMETHOD GetAllResponseHeaders(char **_retval) { return _to GetAllResponseHeaders(_retval); } \
  NS_IMETHOD GetResponseHeader(const nsACString & header, nsACString & _retval) { return _to GetResponseHeader(header, _retval); } \
  NS_IMETHOD OpenRequest(const nsACString & method, const nsACString & url, PRBool async, const nsAString & user, const nsAString & password) { return _to OpenRequest(method, url, async, user, password); } \
  NS_IMETHOD Open(const nsACString & method, const nsACString & url) { return _to Open(method, url); } \
  NS_IMETHOD Send(nsIVariant *body) { return _to Send(body); } \
  NS_IMETHOD SetRequestHeader(const nsACString & header, const nsACString & value) { return _to SetRequestHeader(header, value); } \
  NS_IMETHOD GetReadyState(PRInt32 *aReadyState) { return _to GetReadyState(aReadyState); } \
  NS_IMETHOD OverrideMimeType(const nsACString & mimetype) { return _to OverrideMimeType(mimetype); } \
  NS_IMETHOD GetMultipart(PRBool *aMultipart) { return _to GetMultipart(aMultipart); } \
  NS_IMETHOD SetMultipart(PRBool aMultipart) { return _to SetMultipart(aMultipart); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXMLHTTPREQUEST(_to) \
  NS_IMETHOD GetChannel(nsIChannel * *aChannel) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetChannel(aChannel); } \
  NS_IMETHOD GetResponseXML(nsIDOMDocument * *aResponseXML) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetResponseXML(aResponseXML); } \
  NS_IMETHOD GetResponseText(nsAString & aResponseText) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetResponseText(aResponseText); } \
  NS_IMETHOD GetStatus(PRUint32 *aStatus) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStatus(aStatus); } \
  NS_IMETHOD GetStatusText(nsACString & aStatusText) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStatusText(aStatusText); } \
  NS_IMETHOD Abort(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Abort(); } \
  NS_IMETHOD GetAllResponseHeaders(char **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAllResponseHeaders(_retval); } \
  NS_IMETHOD GetResponseHeader(const nsACString & header, nsACString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetResponseHeader(header, _retval); } \
  NS_IMETHOD OpenRequest(const nsACString & method, const nsACString & url, PRBool async, const nsAString & user, const nsAString & password) { return !_to ? NS_ERROR_NULL_POINTER : _to->OpenRequest(method, url, async, user, password); } \
  NS_IMETHOD Open(const nsACString & method, const nsACString & url) { return !_to ? NS_ERROR_NULL_POINTER : _to->Open(method, url); } \
  NS_IMETHOD Send(nsIVariant *body) { return !_to ? NS_ERROR_NULL_POINTER : _to->Send(body); } \
  NS_IMETHOD SetRequestHeader(const nsACString & header, const nsACString & value) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetRequestHeader(header, value); } \
  NS_IMETHOD GetReadyState(PRInt32 *aReadyState) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetReadyState(aReadyState); } \
  NS_IMETHOD OverrideMimeType(const nsACString & mimetype) { return !_to ? NS_ERROR_NULL_POINTER : _to->OverrideMimeType(mimetype); } \
  NS_IMETHOD GetMultipart(PRBool *aMultipart) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMultipart(aMultipart); } \
  NS_IMETHOD SetMultipart(PRBool aMultipart) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMultipart(aMultipart); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXMLHttpRequest : public nsIXMLHttpRequest
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXMLHTTPREQUEST

  nsXMLHttpRequest();

private:
  ~nsXMLHttpRequest();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXMLHttpRequest, nsIXMLHttpRequest)

nsXMLHttpRequest::nsXMLHttpRequest()
{
  /* member initializers and constructor code */
}

nsXMLHttpRequest::~nsXMLHttpRequest()
{
  /* destructor code */
}

/* readonly attribute nsIChannel channel; */
NS_IMETHODIMP nsXMLHttpRequest::GetChannel(nsIChannel * *aChannel)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMDocument responseXML; */
NS_IMETHODIMP nsXMLHttpRequest::GetResponseXML(nsIDOMDocument * *aResponseXML)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute AString responseText; */
NS_IMETHODIMP nsXMLHttpRequest::GetResponseText(nsAString & aResponseText)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long status; */
NS_IMETHODIMP nsXMLHttpRequest::GetStatus(PRUint32 *aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute AUTF8String statusText; */
NS_IMETHODIMP nsXMLHttpRequest::GetStatusText(nsACString & aStatusText)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void abort (); */
NS_IMETHODIMP nsXMLHttpRequest::Abort()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* string getAllResponseHeaders (); */
NS_IMETHODIMP nsXMLHttpRequest::GetAllResponseHeaders(char **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* ACString getResponseHeader (in AUTF8String header); */
NS_IMETHODIMP nsXMLHttpRequest::GetResponseHeader(const nsACString & header, nsACString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void openRequest (in AUTF8String method, in AUTF8String url, in boolean async, in AString user, in AString password); */
NS_IMETHODIMP nsXMLHttpRequest::OpenRequest(const nsACString & method, const nsACString & url, PRBool async, const nsAString & user, const nsAString & password)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void open (in AUTF8String method, in AUTF8String url); */
NS_IMETHODIMP nsXMLHttpRequest::Open(const nsACString & method, const nsACString & url)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void send (in nsIVariant body); */
NS_IMETHODIMP nsXMLHttpRequest::Send(nsIVariant *body)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setRequestHeader (in AUTF8String header, in AUTF8String value); */
NS_IMETHODIMP nsXMLHttpRequest::SetRequestHeader(const nsACString & header, const nsACString & value)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long readyState; */
NS_IMETHODIMP nsXMLHttpRequest::GetReadyState(PRInt32 *aReadyState)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void overrideMimeType (in AUTF8String mimetype); */
NS_IMETHODIMP nsXMLHttpRequest::OverrideMimeType(const nsACString & mimetype)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean multipart; */
NS_IMETHODIMP nsXMLHttpRequest::GetMultipart(PRBool *aMultipart)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsXMLHttpRequest::SetMultipart(PRBool aMultipart)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIOnReadyStateChangeHandler */
#define NS_IONREADYSTATECHANGEHANDLER_IID_STR "6459b7ce-6b57-4934-a0af-0133ba6f9085"

#define NS_IONREADYSTATECHANGEHANDLER_IID \
  {0x6459b7ce, 0x6b57, 0x4934, \
    { 0xa0, 0xaf, 0x01, 0x33, 0xba, 0x6f, 0x90, 0x85 }}

class NS_NO_VTABLE nsIOnReadyStateChangeHandler : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IONREADYSTATECHANGEHANDLER_IID)

  /**
   * Helper to implement the onreadystatechange callback member.
   * You should not need to use this.
   */
  /* void handleEvent (); */
  NS_IMETHOD HandleEvent(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIONREADYSTATECHANGEHANDLER \
  NS_IMETHOD HandleEvent(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIONREADYSTATECHANGEHANDLER(_to) \
  NS_IMETHOD HandleEvent(void) { return _to HandleEvent(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIONREADYSTATECHANGEHANDLER(_to) \
  NS_IMETHOD HandleEvent(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->HandleEvent(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsOnReadyStateChangeHandler : public nsIOnReadyStateChangeHandler
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIONREADYSTATECHANGEHANDLER

  nsOnReadyStateChangeHandler();

private:
  ~nsOnReadyStateChangeHandler();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsOnReadyStateChangeHandler, nsIOnReadyStateChangeHandler)

nsOnReadyStateChangeHandler::nsOnReadyStateChangeHandler()
{
  /* member initializers and constructor code */
}

nsOnReadyStateChangeHandler::~nsOnReadyStateChangeHandler()
{
  /* destructor code */
}

/* void handleEvent (); */
NS_IMETHODIMP nsOnReadyStateChangeHandler::HandleEvent()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIJSXMLHttpRequest */
#define NS_IJSXMLHTTPREQUEST_IID_STR "9deabc90-28d5-41d3-a660-474f2254f4ba"

#define NS_IJSXMLHTTPREQUEST_IID \
  {0x9deabc90, 0x28d5, 0x41d3, \
    { 0xa6, 0x60, 0x47, 0x4f, 0x22, 0x54, 0xf4, 0xba }}

class NS_NO_VTABLE nsIJSXMLHttpRequest : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IJSXMLHTTPREQUEST_IID)

  /**
   * Meant to be a script-only mechanism for setting a load event listener.
   * The attribute is expected to be JavaScript function object. When
   * the load event occurs, the function is invoked.
   * This attribute should not be used from native code!!
   *
   * After the initial response, all event listeners will be cleared.
   * Call open() before setting new event listeners.
   *
   * Mozilla only.
   */
  /* attribute nsIDOMEventListener onload; */
  NS_IMETHOD GetOnload(nsIDOMEventListener * *aOnload) = 0;
  NS_IMETHOD SetOnload(nsIDOMEventListener * aOnload) = 0;

  /**
   * Meant to be a script-only mechanism for setting an error event listener.
   * The attribute is expected to be JavaScript function object. When
   * the error event occurs, the function is invoked.
   * This attribute should not be used from native code!!
   *
   * After the initial response, all event listeners will be cleared.
   * Call open() before setting new event listeners.
   *
   * Mozilla only.
   */
  /* attribute nsIDOMEventListener onerror; */
  NS_IMETHOD GetOnerror(nsIDOMEventListener * *aOnerror) = 0;
  NS_IMETHOD SetOnerror(nsIDOMEventListener * aOnerror) = 0;

  /**
   * Meant to be a script-only mechanism for setting a progress event listener.
   * The attribute is expected to be JavaScript function object. When
   * the error event occurs, the function is invoked.
   * This attribute should not be used from native code!!
   * This event listener may be called multiple times during the open request.
   *
   * After the initial response, all event listeners will be cleared.
   * Call open() before setting new event listeners.
   *
   * Mozilla only.
   */
  /* attribute nsIDOMEventListener onprogress; */
  NS_IMETHOD GetOnprogress(nsIDOMEventListener * *aOnprogress) = 0;
  NS_IMETHOD SetOnprogress(nsIDOMEventListener * aOnprogress) = 0;

  /**
   * Meant to be a script-only mechanism for setting a callback function.
   * The attribute is expected to be JavaScript function object. When the
   * readyState changes, the callback function will be called.
   * This attribute should not be used from native code!!
   *
   * After the initial response, all event listeners will be cleared.
   * Call open() before setting new event listeners.
   */
  /* attribute nsIOnReadyStateChangeHandler onreadystatechange; */
  NS_IMETHOD GetOnreadystatechange(nsIOnReadyStateChangeHandler * *aOnreadystatechange) = 0;
  NS_IMETHOD SetOnreadystatechange(nsIOnReadyStateChangeHandler * aOnreadystatechange) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIJSXMLHTTPREQUEST \
  NS_IMETHOD GetOnload(nsIDOMEventListener * *aOnload); \
  NS_IMETHOD SetOnload(nsIDOMEventListener * aOnload); \
  NS_IMETHOD GetOnerror(nsIDOMEventListener * *aOnerror); \
  NS_IMETHOD SetOnerror(nsIDOMEventListener * aOnerror); \
  NS_IMETHOD GetOnprogress(nsIDOMEventListener * *aOnprogress); \
  NS_IMETHOD SetOnprogress(nsIDOMEventListener * aOnprogress); \
  NS_IMETHOD GetOnreadystatechange(nsIOnReadyStateChangeHandler * *aOnreadystatechange); \
  NS_IMETHOD SetOnreadystatechange(nsIOnReadyStateChangeHandler * aOnreadystatechange); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIJSXMLHTTPREQUEST(_to) \
  NS_IMETHOD GetOnload(nsIDOMEventListener * *aOnload) { return _to GetOnload(aOnload); } \
  NS_IMETHOD SetOnload(nsIDOMEventListener * aOnload) { return _to SetOnload(aOnload); } \
  NS_IMETHOD GetOnerror(nsIDOMEventListener * *aOnerror) { return _to GetOnerror(aOnerror); } \
  NS_IMETHOD SetOnerror(nsIDOMEventListener * aOnerror) { return _to SetOnerror(aOnerror); } \
  NS_IMETHOD GetOnprogress(nsIDOMEventListener * *aOnprogress) { return _to GetOnprogress(aOnprogress); } \
  NS_IMETHOD SetOnprogress(nsIDOMEventListener * aOnprogress) { return _to SetOnprogress(aOnprogress); } \
  NS_IMETHOD GetOnreadystatechange(nsIOnReadyStateChangeHandler * *aOnreadystatechange) { return _to GetOnreadystatechange(aOnreadystatechange); } \
  NS_IMETHOD SetOnreadystatechange(nsIOnReadyStateChangeHandler * aOnreadystatechange) { return _to SetOnreadystatechange(aOnreadystatechange); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIJSXMLHTTPREQUEST(_to) \
  NS_IMETHOD GetOnload(nsIDOMEventListener * *aOnload) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOnload(aOnload); } \
  NS_IMETHOD SetOnload(nsIDOMEventListener * aOnload) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOnload(aOnload); } \
  NS_IMETHOD GetOnerror(nsIDOMEventListener * *aOnerror) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOnerror(aOnerror); } \
  NS_IMETHOD SetOnerror(nsIDOMEventListener * aOnerror) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOnerror(aOnerror); } \
  NS_IMETHOD GetOnprogress(nsIDOMEventListener * *aOnprogress) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOnprogress(aOnprogress); } \
  NS_IMETHOD SetOnprogress(nsIDOMEventListener * aOnprogress) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOnprogress(aOnprogress); } \
  NS_IMETHOD GetOnreadystatechange(nsIOnReadyStateChangeHandler * *aOnreadystatechange) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOnreadystatechange(aOnreadystatechange); } \
  NS_IMETHOD SetOnreadystatechange(nsIOnReadyStateChangeHandler * aOnreadystatechange) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOnreadystatechange(aOnreadystatechange); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsJSXMLHttpRequest : public nsIJSXMLHttpRequest
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIJSXMLHTTPREQUEST

  nsJSXMLHttpRequest();

private:
  ~nsJSXMLHttpRequest();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsJSXMLHttpRequest, nsIJSXMLHttpRequest)

nsJSXMLHttpRequest::nsJSXMLHttpRequest()
{
  /* member initializers and constructor code */
}

nsJSXMLHttpRequest::~nsJSXMLHttpRequest()
{
  /* destructor code */
}

/* attribute nsIDOMEventListener onload; */
NS_IMETHODIMP nsJSXMLHttpRequest::GetOnload(nsIDOMEventListener * *aOnload)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsJSXMLHttpRequest::SetOnload(nsIDOMEventListener * aOnload)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIDOMEventListener onerror; */
NS_IMETHODIMP nsJSXMLHttpRequest::GetOnerror(nsIDOMEventListener * *aOnerror)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsJSXMLHttpRequest::SetOnerror(nsIDOMEventListener * aOnerror)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIDOMEventListener onprogress; */
NS_IMETHODIMP nsJSXMLHttpRequest::GetOnprogress(nsIDOMEventListener * *aOnprogress)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsJSXMLHttpRequest::SetOnprogress(nsIDOMEventListener * aOnprogress)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIOnReadyStateChangeHandler onreadystatechange; */
NS_IMETHODIMP nsJSXMLHttpRequest::GetOnreadystatechange(nsIOnReadyStateChangeHandler * *aOnreadystatechange)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsJSXMLHttpRequest::SetOnreadystatechange(nsIOnReadyStateChangeHandler * aOnreadystatechange)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

#define NS_XMLHTTPREQUEST_CID                       \
 { /* d164e770-4157-11d4-9a42-000064657374 */       \
  0xd164e770, 0x4157, 0x11d4,                       \
 {0x9a, 0x42, 0x00, 0x00, 0x64, 0x65, 0x73, 0x74} }
#define NS_XMLHTTPREQUEST_CONTRACTID \
"@mozilla.org/xmlextras/xmlhttprequest;1"

#endif /* __gen_nsIXMLHttpRequest_h__ */
