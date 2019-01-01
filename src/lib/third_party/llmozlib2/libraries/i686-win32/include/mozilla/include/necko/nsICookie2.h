/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/cookie/public/nsICookie2.idl
 */

#ifndef __gen_nsICookie2_h__
#define __gen_nsICookie2_h__


#ifndef __gen_nsICookie_h__
#include "nsICookie.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsICookie2 */
#define NS_ICOOKIE2_IID_STR "d3493503-7854-46ed-8284-8af54a847efb"

#define NS_ICOOKIE2_IID \
  {0xd3493503, 0x7854, 0x46ed, \
    { 0x82, 0x84, 0x8a, 0xf5, 0x4a, 0x84, 0x7e, 0xfb }}

/** 
 * Main cookie object interface for use by consumers:
 * extends nsICookie, a frozen interface for external
 * access of cookie objects
 */
class NS_NO_VTABLE nsICookie2 : public nsICookie {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICOOKIE2_IID)

  /**
     * the host (possibly fully qualified) of the cookie,
     * without a leading dot to represent if it is a
     * domain cookie.
     */
  /* readonly attribute AUTF8String rawHost; */
  NS_IMETHOD GetRawHost(nsACString & aRawHost) = 0;

  /**
     * true if the cookie is a session cookie
     * (note: if true, the expiry time is undefined).
     */
  /* readonly attribute boolean isSession; */
  NS_IMETHOD GetIsSession(PRBool *aIsSession) = 0;

  /**
     * the actual expiry time of the cookie
     * (where 0 does not represent a session cookie).
     *
     * not to be confused with |expires|, an
     * attribute on nsICookie.
     */
  /* readonly attribute PRInt64 expiry; */
  NS_IMETHOD GetExpiry(PRInt64 *aExpiry) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICOOKIE2 \
  NS_IMETHOD GetRawHost(nsACString & aRawHost); \
  NS_IMETHOD GetIsSession(PRBool *aIsSession); \
  NS_IMETHOD GetExpiry(PRInt64 *aExpiry); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICOOKIE2(_to) \
  NS_IMETHOD GetRawHost(nsACString & aRawHost) { return _to GetRawHost(aRawHost); } \
  NS_IMETHOD GetIsSession(PRBool *aIsSession) { return _to GetIsSession(aIsSession); } \
  NS_IMETHOD GetExpiry(PRInt64 *aExpiry) { return _to GetExpiry(aExpiry); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICOOKIE2(_to) \
  NS_IMETHOD GetRawHost(nsACString & aRawHost) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRawHost(aRawHost); } \
  NS_IMETHOD GetIsSession(PRBool *aIsSession) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIsSession(aIsSession); } \
  NS_IMETHOD GetExpiry(PRInt64 *aExpiry) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetExpiry(aExpiry); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsCookie2 : public nsICookie2
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICOOKIE2

  nsCookie2();

private:
  ~nsCookie2();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsCookie2, nsICookie2)

nsCookie2::nsCookie2()
{
  /* member initializers and constructor code */
}

nsCookie2::~nsCookie2()
{
  /* destructor code */
}

/* readonly attribute AUTF8String rawHost; */
NS_IMETHODIMP nsCookie2::GetRawHost(nsACString & aRawHost)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean isSession; */
NS_IMETHODIMP nsCookie2::GetIsSession(PRBool *aIsSession)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRInt64 expiry; */
NS_IMETHODIMP nsCookie2::GetExpiry(PRInt64 *aExpiry)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsICookie2_MOZILLA_1_8_BRANCH */
#define NS_ICOOKIE2_MOZILLA_1_8_BRANCH_IID_STR "40712890-6c9e-45fc-b77c-c8ea344f690e"

#define NS_ICOOKIE2_MOZILLA_1_8_BRANCH_IID \
  {0x40712890, 0x6c9e, 0x45fc, \
    { 0xb7, 0x7c, 0xc8, 0xea, 0x34, 0x4f, 0x69, 0x0e }}

class NS_NO_VTABLE nsICookie2_MOZILLA_1_8_BRANCH : public nsICookie2 {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICOOKIE2_MOZILLA_1_8_BRANCH_IID)

  /**
     * true if the cookie is an http only cookie
     */
  /* readonly attribute boolean isHttpOnly; */
  NS_IMETHOD GetIsHttpOnly(PRBool *aIsHttpOnly) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICOOKIE2_MOZILLA_1_8_BRANCH \
  NS_IMETHOD GetIsHttpOnly(PRBool *aIsHttpOnly); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICOOKIE2_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD GetIsHttpOnly(PRBool *aIsHttpOnly) { return _to GetIsHttpOnly(aIsHttpOnly); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICOOKIE2_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD GetIsHttpOnly(PRBool *aIsHttpOnly) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIsHttpOnly(aIsHttpOnly); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsCookie2_MOZILLA_1_8_BRANCH : public nsICookie2_MOZILLA_1_8_BRANCH
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICOOKIE2_MOZILLA_1_8_BRANCH

  nsCookie2_MOZILLA_1_8_BRANCH();

private:
  ~nsCookie2_MOZILLA_1_8_BRANCH();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsCookie2_MOZILLA_1_8_BRANCH, nsICookie2_MOZILLA_1_8_BRANCH)

nsCookie2_MOZILLA_1_8_BRANCH::nsCookie2_MOZILLA_1_8_BRANCH()
{
  /* member initializers and constructor code */
}

nsCookie2_MOZILLA_1_8_BRANCH::~nsCookie2_MOZILLA_1_8_BRANCH()
{
  /* destructor code */
}

/* readonly attribute boolean isHttpOnly; */
NS_IMETHODIMP nsCookie2_MOZILLA_1_8_BRANCH::GetIsHttpOnly(PRBool *aIsHttpOnly)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsICookie2_h__ */
