/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/protocol/http/public/nsIHttpChannelInternal.idl
 */

#ifndef __gen_nsIHttpChannelInternal_h__
#define __gen_nsIHttpChannelInternal_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIURI; /* forward declaration */

class nsIProxyInfo; /* forward declaration */


/* starting interface:    nsIHttpChannelInternal */
#define NS_IHTTPCHANNELINTERNAL_IID_STR "f3764874-ed7e-4873-883c-11d67a4e3638"

#define NS_IHTTPCHANNELINTERNAL_IID \
  {0xf3764874, 0xed7e, 0x4873, \
    { 0x88, 0x3c, 0x11, 0xd6, 0x7a, 0x4e, 0x36, 0x38 }}

/** 
 * Dumping ground for http.  This interface will never be frozen.  If you are 
 * using any feature exposed by this interface, be aware that this interface 
 * will change and you will be broken.  You have been warned.
 */
class NS_NO_VTABLE nsIHttpChannelInternal : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IHTTPCHANNELINTERNAL_IID)

  /**
     * An http channel can own a reference to the document URI
     */
  /* attribute nsIURI documentURI; */
  NS_IMETHOD GetDocumentURI(nsIURI * *aDocumentURI) = 0;
  NS_IMETHOD SetDocumentURI(nsIURI * aDocumentURI) = 0;

  /**
     * Get the major/minor version numbers for the request
     */
  /* void getRequestVersion (out unsigned long major, out unsigned long minor); */
  NS_IMETHOD GetRequestVersion(PRUint32 *major, PRUint32 *minor) = 0;

  /**
     * Get the major/minor version numbers for the response
     */
  /* void getResponseVersion (out unsigned long major, out unsigned long minor); */
  NS_IMETHOD GetResponseVersion(PRUint32 *major, PRUint32 *minor) = 0;

  /**
     * Helper method to set a cookie with a consumer-provided
     * cookie header, _but_ using the channel's other information
     * (URI's, prompters, date headers etc).
     *
     * @param aCookieHeader
     *        The cookie header to be parsed.
     */
  /* void setCookie (in string aCookieHeader); */
  NS_IMETHOD SetCookie(const char *aCookieHeader) = 0;

  /**
     * Get the proxy info in use by the channel.
     */
  /* readonly attribute nsIProxyInfo proxyInfo; */
  NS_IMETHOD GetProxyInfo(nsIProxyInfo * *aProxyInfo) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIHTTPCHANNELINTERNAL \
  NS_IMETHOD GetDocumentURI(nsIURI * *aDocumentURI); \
  NS_IMETHOD SetDocumentURI(nsIURI * aDocumentURI); \
  NS_IMETHOD GetRequestVersion(PRUint32 *major, PRUint32 *minor); \
  NS_IMETHOD GetResponseVersion(PRUint32 *major, PRUint32 *minor); \
  NS_IMETHOD SetCookie(const char *aCookieHeader); \
  NS_IMETHOD GetProxyInfo(nsIProxyInfo * *aProxyInfo); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIHTTPCHANNELINTERNAL(_to) \
  NS_IMETHOD GetDocumentURI(nsIURI * *aDocumentURI) { return _to GetDocumentURI(aDocumentURI); } \
  NS_IMETHOD SetDocumentURI(nsIURI * aDocumentURI) { return _to SetDocumentURI(aDocumentURI); } \
  NS_IMETHOD GetRequestVersion(PRUint32 *major, PRUint32 *minor) { return _to GetRequestVersion(major, minor); } \
  NS_IMETHOD GetResponseVersion(PRUint32 *major, PRUint32 *minor) { return _to GetResponseVersion(major, minor); } \
  NS_IMETHOD SetCookie(const char *aCookieHeader) { return _to SetCookie(aCookieHeader); } \
  NS_IMETHOD GetProxyInfo(nsIProxyInfo * *aProxyInfo) { return _to GetProxyInfo(aProxyInfo); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIHTTPCHANNELINTERNAL(_to) \
  NS_IMETHOD GetDocumentURI(nsIURI * *aDocumentURI) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDocumentURI(aDocumentURI); } \
  NS_IMETHOD SetDocumentURI(nsIURI * aDocumentURI) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDocumentURI(aDocumentURI); } \
  NS_IMETHOD GetRequestVersion(PRUint32 *major, PRUint32 *minor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRequestVersion(major, minor); } \
  NS_IMETHOD GetResponseVersion(PRUint32 *major, PRUint32 *minor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetResponseVersion(major, minor); } \
  NS_IMETHOD SetCookie(const char *aCookieHeader) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCookie(aCookieHeader); } \
  NS_IMETHOD GetProxyInfo(nsIProxyInfo * *aProxyInfo) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetProxyInfo(aProxyInfo); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsHttpChannelInternal : public nsIHttpChannelInternal
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIHTTPCHANNELINTERNAL

  nsHttpChannelInternal();

private:
  ~nsHttpChannelInternal();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsHttpChannelInternal, nsIHttpChannelInternal)

nsHttpChannelInternal::nsHttpChannelInternal()
{
  /* member initializers and constructor code */
}

nsHttpChannelInternal::~nsHttpChannelInternal()
{
  /* destructor code */
}

/* attribute nsIURI documentURI; */
NS_IMETHODIMP nsHttpChannelInternal::GetDocumentURI(nsIURI * *aDocumentURI)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsHttpChannelInternal::SetDocumentURI(nsIURI * aDocumentURI)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getRequestVersion (out unsigned long major, out unsigned long minor); */
NS_IMETHODIMP nsHttpChannelInternal::GetRequestVersion(PRUint32 *major, PRUint32 *minor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getResponseVersion (out unsigned long major, out unsigned long minor); */
NS_IMETHODIMP nsHttpChannelInternal::GetResponseVersion(PRUint32 *major, PRUint32 *minor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setCookie (in string aCookieHeader); */
NS_IMETHODIMP nsHttpChannelInternal::SetCookie(const char *aCookieHeader)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIProxyInfo proxyInfo; */
NS_IMETHODIMP nsHttpChannelInternal::GetProxyInfo(nsIProxyInfo * *aProxyInfo)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIHttpChannelInternal_h__ */
