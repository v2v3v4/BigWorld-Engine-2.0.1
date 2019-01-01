/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/cookie/public/nsICookieConsent.idl
 */

#ifndef __gen_nsICookieConsent_h__
#define __gen_nsICookieConsent_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsICookie_h__
#include "nsICookie.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIURI; /* forward declaration */

class nsIHttpChannel; /* forward declaration */


/* starting interface:    nsICookieConsent */
#define NS_ICOOKIECONSENT_IID_STR "f5a34f50-1f39-11d6-a627-0010a401eb10"

#define NS_ICOOKIECONSENT_IID \
  {0xf5a34f50, 0x1f39, 0x11d6, \
    { 0xa6, 0x27, 0x00, 0x10, 0xa4, 0x01, 0xeb, 0x10 }}

class NS_NO_VTABLE nsICookieConsent : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICOOKIECONSENT_IID)

  /**
   * getConsent
   *
   * gives a decision on what should be done with a cookie, based on a site's
   * p3p policy and the user's preferences. the policy for the given URI and
   * channel is also returned.
   *
   * @param uri
   *        the URI to find the policy for
   * @param httpChannel
   *        the channel to extract the p3p headers from
   * @param isForeign
   *        true if the cookie originates from a third-party site. this is used
   *        to decide the cookie status based on user preferences.
   * @param policy
   *        the policy for the given URI, or nsICookie::POLICY_UNKNOWN if one
   *        cannot be found. valid values are defined in nsICookie.idl.
   *
   * @return nsCookieStatus value. valid values are defined in nsICookie.idl.
   */
  /* nsCookieStatus getConsent (in nsIURI uri, in nsIHttpChannel httpChannel, in boolean isForeign, out nsCookiePolicy policy); */
  NS_IMETHOD GetConsent(nsIURI *uri, nsIHttpChannel *httpChannel, PRBool isForeign, nsCookiePolicy *policy, nsCookieStatus *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICOOKIECONSENT \
  NS_IMETHOD GetConsent(nsIURI *uri, nsIHttpChannel *httpChannel, PRBool isForeign, nsCookiePolicy *policy, nsCookieStatus *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICOOKIECONSENT(_to) \
  NS_IMETHOD GetConsent(nsIURI *uri, nsIHttpChannel *httpChannel, PRBool isForeign, nsCookiePolicy *policy, nsCookieStatus *_retval) { return _to GetConsent(uri, httpChannel, isForeign, policy, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICOOKIECONSENT(_to) \
  NS_IMETHOD GetConsent(nsIURI *uri, nsIHttpChannel *httpChannel, PRBool isForeign, nsCookiePolicy *policy, nsCookieStatus *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetConsent(uri, httpChannel, isForeign, policy, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsCookieConsent : public nsICookieConsent
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICOOKIECONSENT

  nsCookieConsent();

private:
  ~nsCookieConsent();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsCookieConsent, nsICookieConsent)

nsCookieConsent::nsCookieConsent()
{
  /* member initializers and constructor code */
}

nsCookieConsent::~nsCookieConsent()
{
  /* destructor code */
}

/* nsCookieStatus getConsent (in nsIURI uri, in nsIHttpChannel httpChannel, in boolean isForeign, out nsCookiePolicy policy); */
NS_IMETHODIMP nsCookieConsent::GetConsent(nsIURI *uri, nsIHttpChannel *httpChannel, PRBool isForeign, nsCookiePolicy *policy, nsCookieStatus *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

#define NS_COOKIECONSENT_CONTRACTID "@mozilla.org/cookie-consent;1"

#endif /* __gen_nsICookieConsent_h__ */
