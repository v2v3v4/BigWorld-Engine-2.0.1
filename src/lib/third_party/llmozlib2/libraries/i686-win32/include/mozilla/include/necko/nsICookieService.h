/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/cookie/public/nsICookieService.idl
 */

#ifndef __gen_nsICookieService_h__
#define __gen_nsICookieService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIURI; /* forward declaration */

class nsIPrompt; /* forward declaration */

class nsIChannel; /* forward declaration */


/* starting interface:    nsICookieService */
#define NS_ICOOKIESERVICE_IID_STR "011c3190-1434-11d6-a618-0010a401eb10"

#define NS_ICOOKIESERVICE_IID \
  {0x011c3190, 0x1434, 0x11d6, \
    { 0xa6, 0x18, 0x00, 0x10, 0xa4, 0x01, 0xeb, 0x10 }}

/**
 * nsICookieService
 *
 * Provides methods for setting and getting cookies in the context of a
 * page load.  See nsICookieManager for methods to manipulate the cookie
 * database directly.  This separation of interface is mainly historical.
 *
 * This service broadcasts the following notifications when the cookie
 * list is changed, or a cookie is rejected:
 *
 * topic  : "cookie-changed"
 *          broadcast whenever the cookie list changes in some way. there
 *          are four possible data strings for this notification; one
 *          notification will be broadcast for each change, and will involve
 *          a single cookie.
 * subject: an nsICookie2 interface pointer representing the cookie object
 *          that changed.
 * data   : "deleted"
 *          a cookie was deleted. the subject is the deleted cookie.
 *          "added"
 *          a cookie was added. the subject is the added cookie.
 *          "changed"
 *          a cookie was changed. the subject is the new cookie.
 *          "cleared"
 *          the entire cookie list was cleared. the subject is null.
 *
 * topic  : "cookie-rejected"
 *          broadcast whenever a cookie was rejected from being set as a
 *          result of user prefs.
 * subject: an nsIURI interface pointer representing the URI that attempted
 *          to set the cookie.
 * data   : none.
 */
class NS_NO_VTABLE nsICookieService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICOOKIESERVICE_IID)

  /* string getCookieString (in nsIURI aURI, in nsIChannel aChannel); */
  NS_IMETHOD GetCookieString(nsIURI *aURI, nsIChannel *aChannel, char **_retval) = 0;

  /* string getCookieStringFromHttp (in nsIURI aURI, in nsIURI aFirstURI, in nsIChannel aChannel); */
  NS_IMETHOD GetCookieStringFromHttp(nsIURI *aURI, nsIURI *aFirstURI, nsIChannel *aChannel, char **_retval) = 0;

  /* void setCookieString (in nsIURI aURI, in nsIPrompt aPrompt, in string aCookie, in nsIChannel aChannel); */
  NS_IMETHOD SetCookieString(nsIURI *aURI, nsIPrompt *aPrompt, const char *aCookie, nsIChannel *aChannel) = 0;

  /* void setCookieStringFromHttp (in nsIURI aURI, in nsIURI aFirstURI, in nsIPrompt aPrompt, in string aCookie, in string aServerTime, in nsIChannel aChannel); */
  NS_IMETHOD SetCookieStringFromHttp(nsIURI *aURI, nsIURI *aFirstURI, nsIPrompt *aPrompt, const char *aCookie, const char *aServerTime, nsIChannel *aChannel) = 0;

  /**
   * This attribute really doesn't belong on this interface.  CVS blame will
   * tell you why it is here.  It remains until we can find a better home for
   * it.  Read the source if you want to know what it does :-(
   */
  /* readonly attribute boolean cookieIconIsVisible; */
  NS_IMETHOD GetCookieIconIsVisible(PRBool *aCookieIconIsVisible) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICOOKIESERVICE \
  NS_IMETHOD GetCookieString(nsIURI *aURI, nsIChannel *aChannel, char **_retval); \
  NS_IMETHOD GetCookieStringFromHttp(nsIURI *aURI, nsIURI *aFirstURI, nsIChannel *aChannel, char **_retval); \
  NS_IMETHOD SetCookieString(nsIURI *aURI, nsIPrompt *aPrompt, const char *aCookie, nsIChannel *aChannel); \
  NS_IMETHOD SetCookieStringFromHttp(nsIURI *aURI, nsIURI *aFirstURI, nsIPrompt *aPrompt, const char *aCookie, const char *aServerTime, nsIChannel *aChannel); \
  NS_IMETHOD GetCookieIconIsVisible(PRBool *aCookieIconIsVisible); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICOOKIESERVICE(_to) \
  NS_IMETHOD GetCookieString(nsIURI *aURI, nsIChannel *aChannel, char **_retval) { return _to GetCookieString(aURI, aChannel, _retval); } \
  NS_IMETHOD GetCookieStringFromHttp(nsIURI *aURI, nsIURI *aFirstURI, nsIChannel *aChannel, char **_retval) { return _to GetCookieStringFromHttp(aURI, aFirstURI, aChannel, _retval); } \
  NS_IMETHOD SetCookieString(nsIURI *aURI, nsIPrompt *aPrompt, const char *aCookie, nsIChannel *aChannel) { return _to SetCookieString(aURI, aPrompt, aCookie, aChannel); } \
  NS_IMETHOD SetCookieStringFromHttp(nsIURI *aURI, nsIURI *aFirstURI, nsIPrompt *aPrompt, const char *aCookie, const char *aServerTime, nsIChannel *aChannel) { return _to SetCookieStringFromHttp(aURI, aFirstURI, aPrompt, aCookie, aServerTime, aChannel); } \
  NS_IMETHOD GetCookieIconIsVisible(PRBool *aCookieIconIsVisible) { return _to GetCookieIconIsVisible(aCookieIconIsVisible); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICOOKIESERVICE(_to) \
  NS_IMETHOD GetCookieString(nsIURI *aURI, nsIChannel *aChannel, char **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCookieString(aURI, aChannel, _retval); } \
  NS_IMETHOD GetCookieStringFromHttp(nsIURI *aURI, nsIURI *aFirstURI, nsIChannel *aChannel, char **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCookieStringFromHttp(aURI, aFirstURI, aChannel, _retval); } \
  NS_IMETHOD SetCookieString(nsIURI *aURI, nsIPrompt *aPrompt, const char *aCookie, nsIChannel *aChannel) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCookieString(aURI, aPrompt, aCookie, aChannel); } \
  NS_IMETHOD SetCookieStringFromHttp(nsIURI *aURI, nsIURI *aFirstURI, nsIPrompt *aPrompt, const char *aCookie, const char *aServerTime, nsIChannel *aChannel) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCookieStringFromHttp(aURI, aFirstURI, aPrompt, aCookie, aServerTime, aChannel); } \
  NS_IMETHOD GetCookieIconIsVisible(PRBool *aCookieIconIsVisible) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCookieIconIsVisible(aCookieIconIsVisible); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsCookieService : public nsICookieService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICOOKIESERVICE

  nsCookieService();

private:
  ~nsCookieService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsCookieService, nsICookieService)

nsCookieService::nsCookieService()
{
  /* member initializers and constructor code */
}

nsCookieService::~nsCookieService()
{
  /* destructor code */
}

/* string getCookieString (in nsIURI aURI, in nsIChannel aChannel); */
NS_IMETHODIMP nsCookieService::GetCookieString(nsIURI *aURI, nsIChannel *aChannel, char **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* string getCookieStringFromHttp (in nsIURI aURI, in nsIURI aFirstURI, in nsIChannel aChannel); */
NS_IMETHODIMP nsCookieService::GetCookieStringFromHttp(nsIURI *aURI, nsIURI *aFirstURI, nsIChannel *aChannel, char **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setCookieString (in nsIURI aURI, in nsIPrompt aPrompt, in string aCookie, in nsIChannel aChannel); */
NS_IMETHODIMP nsCookieService::SetCookieString(nsIURI *aURI, nsIPrompt *aPrompt, const char *aCookie, nsIChannel *aChannel)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setCookieStringFromHttp (in nsIURI aURI, in nsIURI aFirstURI, in nsIPrompt aPrompt, in string aCookie, in string aServerTime, in nsIChannel aChannel); */
NS_IMETHODIMP nsCookieService::SetCookieStringFromHttp(nsIURI *aURI, nsIURI *aFirstURI, nsIPrompt *aPrompt, const char *aCookie, const char *aServerTime, nsIChannel *aChannel)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean cookieIconIsVisible; */
NS_IMETHODIMP nsCookieService::GetCookieIconIsVisible(PRBool *aCookieIconIsVisible)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsICookieService_h__ */
