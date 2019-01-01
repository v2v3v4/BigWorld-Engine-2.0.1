/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/cookie/public/nsICookieManager2.idl
 */

#ifndef __gen_nsICookieManager2_h__
#define __gen_nsICookieManager2_h__


#ifndef __gen_nsICookieManager_h__
#include "nsICookieManager.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsICookie2; /* forward declaration */


/* starting interface:    nsICookieManager2 */
#define NS_ICOOKIEMANAGER2_IID_STR "3e73ff5f-154e-494f-b640-3c654ba2cc2b"

#define NS_ICOOKIEMANAGER2_IID \
  {0x3e73ff5f, 0x154e, 0x494f, \
    { 0xb6, 0x40, 0x3c, 0x65, 0x4b, 0xa2, 0xcc, 0x2b }}

/** 
 * Additions to the frozen nsICookieManager
 */
class NS_NO_VTABLE nsICookieManager2 : public nsICookieManager {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICOOKIEMANAGER2_IID)

  /**
   * Add a cookie. nsICookieService is the normal way to do this. This
   * method is something of a backdoor.
   *
   * @param aDomain
   *        the host or domain for which the cookie is set. presence of a
   *        leading dot indicates a domain cookie; otherwise, the cookie
   *        is treated as a non-domain cookie. see RFC2109.
   * @param aPath
   *        path within the domain for which the cookie is valid
   * @param aName
   *        cookie name
   * @param aValue
   *        cookie data
   * @param aSecure
   *        true if the cookie should only be sent over a secure connection.
   * @param aIsSession
   *        true if the cookie should exist for the current session only.
   * @param aExpiry
   *        expiration date, in seconds since the epoch. only relevant if
   *        aIsSession is false.
   */
  /* void add (in AUTF8String aDomain, in AUTF8String aPath, in ACString aName, in ACString aValue, in boolean aSecure, in boolean aIsSession, in PRInt64 aExpiry); */
  NS_IMETHOD Add(const nsACString & aDomain, const nsACString & aPath, const nsACString & aName, const nsACString & aValue, PRBool aSecure, PRBool aIsSession, PRInt64 aExpiry) = 0;

  /**
   * Find whether a matching cookie already exists, and how many cookies
   * a given host has already set. This is useful when e.g. prompting the
   * user whether to accept a given cookie.
   *
   * @param aCookie
   *        the cookie to look for
   * @param aCountFromHost
   *        the number of cookies found whose hosts are the same as, or
   *        subdomains of, the host field of aCookie
   *
   * @return true if a cookie was found which matches the host, path, and name
   *         fields of aCookie
   */
  /* boolean findMatchingCookie (in nsICookie2 aCookie, out unsigned long aCountFromHost); */
  NS_IMETHOD FindMatchingCookie(nsICookie2 *aCookie, PRUint32 *aCountFromHost, PRBool *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICOOKIEMANAGER2 \
  NS_IMETHOD Add(const nsACString & aDomain, const nsACString & aPath, const nsACString & aName, const nsACString & aValue, PRBool aSecure, PRBool aIsSession, PRInt64 aExpiry); \
  NS_IMETHOD FindMatchingCookie(nsICookie2 *aCookie, PRUint32 *aCountFromHost, PRBool *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICOOKIEMANAGER2(_to) \
  NS_IMETHOD Add(const nsACString & aDomain, const nsACString & aPath, const nsACString & aName, const nsACString & aValue, PRBool aSecure, PRBool aIsSession, PRInt64 aExpiry) { return _to Add(aDomain, aPath, aName, aValue, aSecure, aIsSession, aExpiry); } \
  NS_IMETHOD FindMatchingCookie(nsICookie2 *aCookie, PRUint32 *aCountFromHost, PRBool *_retval) { return _to FindMatchingCookie(aCookie, aCountFromHost, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICOOKIEMANAGER2(_to) \
  NS_IMETHOD Add(const nsACString & aDomain, const nsACString & aPath, const nsACString & aName, const nsACString & aValue, PRBool aSecure, PRBool aIsSession, PRInt64 aExpiry) { return !_to ? NS_ERROR_NULL_POINTER : _to->Add(aDomain, aPath, aName, aValue, aSecure, aIsSession, aExpiry); } \
  NS_IMETHOD FindMatchingCookie(nsICookie2 *aCookie, PRUint32 *aCountFromHost, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->FindMatchingCookie(aCookie, aCountFromHost, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsCookieManager2 : public nsICookieManager2
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICOOKIEMANAGER2

  nsCookieManager2();

private:
  ~nsCookieManager2();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsCookieManager2, nsICookieManager2)

nsCookieManager2::nsCookieManager2()
{
  /* member initializers and constructor code */
}

nsCookieManager2::~nsCookieManager2()
{
  /* destructor code */
}

/* void add (in AUTF8String aDomain, in AUTF8String aPath, in ACString aName, in ACString aValue, in boolean aSecure, in boolean aIsSession, in PRInt64 aExpiry); */
NS_IMETHODIMP nsCookieManager2::Add(const nsACString & aDomain, const nsACString & aPath, const nsACString & aName, const nsACString & aValue, PRBool aSecure, PRBool aIsSession, PRInt64 aExpiry)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean findMatchingCookie (in nsICookie2 aCookie, out unsigned long aCountFromHost); */
NS_IMETHODIMP nsCookieManager2::FindMatchingCookie(nsICookie2 *aCookie, PRUint32 *aCountFromHost, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsICookieManager2_h__ */
