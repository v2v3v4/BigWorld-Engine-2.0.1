/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/base/nsIDOMNavigator.idl
 */

#ifndef __gen_nsIDOMNavigator_h__
#define __gen_nsIDOMNavigator_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMNavigator */
#define NS_IDOMNAVIGATOR_IID_STR "8758b72b-63d4-4685-b908-4275126410fb"

#define NS_IDOMNAVIGATOR_IID \
  {0x8758b72b, 0x63d4, 0x4685, \
    { 0xb9, 0x08, 0x42, 0x75, 0x12, 0x64, 0x10, 0xfb }}

class NS_NO_VTABLE nsIDOMNavigator : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMNAVIGATOR_IID)

  /* readonly attribute DOMString appCodeName; */
  NS_IMETHOD GetAppCodeName(nsAString & aAppCodeName) = 0;

  /* readonly attribute DOMString appName; */
  NS_IMETHOD GetAppName(nsAString & aAppName) = 0;

  /* readonly attribute DOMString appVersion; */
  NS_IMETHOD GetAppVersion(nsAString & aAppVersion) = 0;

  /* readonly attribute DOMString language; */
  NS_IMETHOD GetLanguage(nsAString & aLanguage) = 0;

  /* readonly attribute nsIDOMMimeTypeArray mimeTypes; */
  NS_IMETHOD GetMimeTypes(nsIDOMMimeTypeArray * *aMimeTypes) = 0;

  /* readonly attribute DOMString platform; */
  NS_IMETHOD GetPlatform(nsAString & aPlatform) = 0;

  /* readonly attribute DOMString oscpu; */
  NS_IMETHOD GetOscpu(nsAString & aOscpu) = 0;

  /* readonly attribute DOMString vendor; */
  NS_IMETHOD GetVendor(nsAString & aVendor) = 0;

  /* readonly attribute DOMString vendorSub; */
  NS_IMETHOD GetVendorSub(nsAString & aVendorSub) = 0;

  /* readonly attribute DOMString product; */
  NS_IMETHOD GetProduct(nsAString & aProduct) = 0;

  /* readonly attribute DOMString productSub; */
  NS_IMETHOD GetProductSub(nsAString & aProductSub) = 0;

  /* readonly attribute nsIDOMPluginArray plugins; */
  NS_IMETHOD GetPlugins(nsIDOMPluginArray * *aPlugins) = 0;

  /* readonly attribute DOMString securityPolicy; */
  NS_IMETHOD GetSecurityPolicy(nsAString & aSecurityPolicy) = 0;

  /* readonly attribute DOMString userAgent; */
  NS_IMETHOD GetUserAgent(nsAString & aUserAgent) = 0;

  /* readonly attribute boolean cookieEnabled; */
  NS_IMETHOD GetCookieEnabled(PRBool *aCookieEnabled) = 0;

  /* readonly attribute boolean onLine; */
  NS_IMETHOD GetOnLine(PRBool *aOnLine) = 0;

  /* boolean javaEnabled (); */
  NS_IMETHOD JavaEnabled(PRBool *_retval) = 0;

  /* boolean taintEnabled (); */
  NS_IMETHOD TaintEnabled(PRBool *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMNAVIGATOR \
  NS_IMETHOD GetAppCodeName(nsAString & aAppCodeName); \
  NS_IMETHOD GetAppName(nsAString & aAppName); \
  NS_IMETHOD GetAppVersion(nsAString & aAppVersion); \
  NS_IMETHOD GetLanguage(nsAString & aLanguage); \
  NS_IMETHOD GetMimeTypes(nsIDOMMimeTypeArray * *aMimeTypes); \
  NS_IMETHOD GetPlatform(nsAString & aPlatform); \
  NS_IMETHOD GetOscpu(nsAString & aOscpu); \
  NS_IMETHOD GetVendor(nsAString & aVendor); \
  NS_IMETHOD GetVendorSub(nsAString & aVendorSub); \
  NS_IMETHOD GetProduct(nsAString & aProduct); \
  NS_IMETHOD GetProductSub(nsAString & aProductSub); \
  NS_IMETHOD GetPlugins(nsIDOMPluginArray * *aPlugins); \
  NS_IMETHOD GetSecurityPolicy(nsAString & aSecurityPolicy); \
  NS_IMETHOD GetUserAgent(nsAString & aUserAgent); \
  NS_IMETHOD GetCookieEnabled(PRBool *aCookieEnabled); \
  NS_IMETHOD GetOnLine(PRBool *aOnLine); \
  NS_IMETHOD JavaEnabled(PRBool *_retval); \
  NS_IMETHOD TaintEnabled(PRBool *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMNAVIGATOR(_to) \
  NS_IMETHOD GetAppCodeName(nsAString & aAppCodeName) { return _to GetAppCodeName(aAppCodeName); } \
  NS_IMETHOD GetAppName(nsAString & aAppName) { return _to GetAppName(aAppName); } \
  NS_IMETHOD GetAppVersion(nsAString & aAppVersion) { return _to GetAppVersion(aAppVersion); } \
  NS_IMETHOD GetLanguage(nsAString & aLanguage) { return _to GetLanguage(aLanguage); } \
  NS_IMETHOD GetMimeTypes(nsIDOMMimeTypeArray * *aMimeTypes) { return _to GetMimeTypes(aMimeTypes); } \
  NS_IMETHOD GetPlatform(nsAString & aPlatform) { return _to GetPlatform(aPlatform); } \
  NS_IMETHOD GetOscpu(nsAString & aOscpu) { return _to GetOscpu(aOscpu); } \
  NS_IMETHOD GetVendor(nsAString & aVendor) { return _to GetVendor(aVendor); } \
  NS_IMETHOD GetVendorSub(nsAString & aVendorSub) { return _to GetVendorSub(aVendorSub); } \
  NS_IMETHOD GetProduct(nsAString & aProduct) { return _to GetProduct(aProduct); } \
  NS_IMETHOD GetProductSub(nsAString & aProductSub) { return _to GetProductSub(aProductSub); } \
  NS_IMETHOD GetPlugins(nsIDOMPluginArray * *aPlugins) { return _to GetPlugins(aPlugins); } \
  NS_IMETHOD GetSecurityPolicy(nsAString & aSecurityPolicy) { return _to GetSecurityPolicy(aSecurityPolicy); } \
  NS_IMETHOD GetUserAgent(nsAString & aUserAgent) { return _to GetUserAgent(aUserAgent); } \
  NS_IMETHOD GetCookieEnabled(PRBool *aCookieEnabled) { return _to GetCookieEnabled(aCookieEnabled); } \
  NS_IMETHOD GetOnLine(PRBool *aOnLine) { return _to GetOnLine(aOnLine); } \
  NS_IMETHOD JavaEnabled(PRBool *_retval) { return _to JavaEnabled(_retval); } \
  NS_IMETHOD TaintEnabled(PRBool *_retval) { return _to TaintEnabled(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMNAVIGATOR(_to) \
  NS_IMETHOD GetAppCodeName(nsAString & aAppCodeName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAppCodeName(aAppCodeName); } \
  NS_IMETHOD GetAppName(nsAString & aAppName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAppName(aAppName); } \
  NS_IMETHOD GetAppVersion(nsAString & aAppVersion) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAppVersion(aAppVersion); } \
  NS_IMETHOD GetLanguage(nsAString & aLanguage) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLanguage(aLanguage); } \
  NS_IMETHOD GetMimeTypes(nsIDOMMimeTypeArray * *aMimeTypes) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMimeTypes(aMimeTypes); } \
  NS_IMETHOD GetPlatform(nsAString & aPlatform) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPlatform(aPlatform); } \
  NS_IMETHOD GetOscpu(nsAString & aOscpu) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOscpu(aOscpu); } \
  NS_IMETHOD GetVendor(nsAString & aVendor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetVendor(aVendor); } \
  NS_IMETHOD GetVendorSub(nsAString & aVendorSub) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetVendorSub(aVendorSub); } \
  NS_IMETHOD GetProduct(nsAString & aProduct) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetProduct(aProduct); } \
  NS_IMETHOD GetProductSub(nsAString & aProductSub) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetProductSub(aProductSub); } \
  NS_IMETHOD GetPlugins(nsIDOMPluginArray * *aPlugins) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPlugins(aPlugins); } \
  NS_IMETHOD GetSecurityPolicy(nsAString & aSecurityPolicy) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSecurityPolicy(aSecurityPolicy); } \
  NS_IMETHOD GetUserAgent(nsAString & aUserAgent) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetUserAgent(aUserAgent); } \
  NS_IMETHOD GetCookieEnabled(PRBool *aCookieEnabled) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCookieEnabled(aCookieEnabled); } \
  NS_IMETHOD GetOnLine(PRBool *aOnLine) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOnLine(aOnLine); } \
  NS_IMETHOD JavaEnabled(PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->JavaEnabled(_retval); } \
  NS_IMETHOD TaintEnabled(PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->TaintEnabled(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMNavigator : public nsIDOMNavigator
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMNAVIGATOR

  nsDOMNavigator();

private:
  ~nsDOMNavigator();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMNavigator, nsIDOMNavigator)

nsDOMNavigator::nsDOMNavigator()
{
  /* member initializers and constructor code */
}

nsDOMNavigator::~nsDOMNavigator()
{
  /* destructor code */
}

/* readonly attribute DOMString appCodeName; */
NS_IMETHODIMP nsDOMNavigator::GetAppCodeName(nsAString & aAppCodeName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString appName; */
NS_IMETHODIMP nsDOMNavigator::GetAppName(nsAString & aAppName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString appVersion; */
NS_IMETHODIMP nsDOMNavigator::GetAppVersion(nsAString & aAppVersion)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString language; */
NS_IMETHODIMP nsDOMNavigator::GetLanguage(nsAString & aLanguage)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMMimeTypeArray mimeTypes; */
NS_IMETHODIMP nsDOMNavigator::GetMimeTypes(nsIDOMMimeTypeArray * *aMimeTypes)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString platform; */
NS_IMETHODIMP nsDOMNavigator::GetPlatform(nsAString & aPlatform)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString oscpu; */
NS_IMETHODIMP nsDOMNavigator::GetOscpu(nsAString & aOscpu)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString vendor; */
NS_IMETHODIMP nsDOMNavigator::GetVendor(nsAString & aVendor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString vendorSub; */
NS_IMETHODIMP nsDOMNavigator::GetVendorSub(nsAString & aVendorSub)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString product; */
NS_IMETHODIMP nsDOMNavigator::GetProduct(nsAString & aProduct)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString productSub; */
NS_IMETHODIMP nsDOMNavigator::GetProductSub(nsAString & aProductSub)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMPluginArray plugins; */
NS_IMETHODIMP nsDOMNavigator::GetPlugins(nsIDOMPluginArray * *aPlugins)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString securityPolicy; */
NS_IMETHODIMP nsDOMNavigator::GetSecurityPolicy(nsAString & aSecurityPolicy)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString userAgent; */
NS_IMETHODIMP nsDOMNavigator::GetUserAgent(nsAString & aUserAgent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean cookieEnabled; */
NS_IMETHODIMP nsDOMNavigator::GetCookieEnabled(PRBool *aCookieEnabled)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean onLine; */
NS_IMETHODIMP nsDOMNavigator::GetOnLine(PRBool *aOnLine)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean javaEnabled (); */
NS_IMETHODIMP nsDOMNavigator::JavaEnabled(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean taintEnabled (); */
NS_IMETHODIMP nsDOMNavigator::TaintEnabled(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMJSNavigator */
#define NS_IDOMJSNAVIGATOR_IID_STR "4b4f8316-1dd2-11b2-b265-9a857376d159"

#define NS_IDOMJSNAVIGATOR_IID \
  {0x4b4f8316, 0x1dd2, 0x11b2, \
    { 0xb2, 0x65, 0x9a, 0x85, 0x73, 0x76, 0xd1, 0x59 }}

class NS_NO_VTABLE nsIDOMJSNavigator : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMJSNAVIGATOR_IID)

  /* void preference (); */
  NS_IMETHOD Preference(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMJSNAVIGATOR \
  NS_IMETHOD Preference(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMJSNAVIGATOR(_to) \
  NS_IMETHOD Preference(void) { return _to Preference(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMJSNAVIGATOR(_to) \
  NS_IMETHOD Preference(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Preference(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMJSNavigator : public nsIDOMJSNavigator
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMJSNAVIGATOR

  nsDOMJSNavigator();

private:
  ~nsDOMJSNavigator();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMJSNavigator, nsIDOMJSNavigator)

nsDOMJSNavigator::nsDOMJSNavigator()
{
  /* member initializers and constructor code */
}

nsDOMJSNavigator::~nsDOMJSNavigator()
{
  /* destructor code */
}

/* void preference (); */
NS_IMETHODIMP nsDOMJSNavigator::Preference()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMNavigator_MOZILLA_1_8_BRANCH */
#define NS_IDOMNAVIGATOR_MOZILLA_1_8_BRANCH_IID_STR "b3abb5c6-a0c4-47e9-84f6-edb873042769"

#define NS_IDOMNAVIGATOR_MOZILLA_1_8_BRANCH_IID \
  {0xb3abb5c6, 0xa0c4, 0x47e9, \
    { 0x84, 0xf6, 0xed, 0xb8, 0x73, 0x04, 0x27, 0x69 }}

class NS_NO_VTABLE nsIDOMNavigator_MOZILLA_1_8_BRANCH : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMNAVIGATOR_MOZILLA_1_8_BRANCH_IID)

  /* readonly attribute DOMString buildID; */
  NS_IMETHOD GetBuildID(nsAString & aBuildID) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMNAVIGATOR_MOZILLA_1_8_BRANCH \
  NS_IMETHOD GetBuildID(nsAString & aBuildID); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMNAVIGATOR_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD GetBuildID(nsAString & aBuildID) { return _to GetBuildID(aBuildID); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMNAVIGATOR_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD GetBuildID(nsAString & aBuildID) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBuildID(aBuildID); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMNavigator_MOZILLA_1_8_BRANCH : public nsIDOMNavigator_MOZILLA_1_8_BRANCH
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMNAVIGATOR_MOZILLA_1_8_BRANCH

  nsDOMNavigator_MOZILLA_1_8_BRANCH();

private:
  ~nsDOMNavigator_MOZILLA_1_8_BRANCH();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMNavigator_MOZILLA_1_8_BRANCH, nsIDOMNavigator_MOZILLA_1_8_BRANCH)

nsDOMNavigator_MOZILLA_1_8_BRANCH::nsDOMNavigator_MOZILLA_1_8_BRANCH()
{
  /* member initializers and constructor code */
}

nsDOMNavigator_MOZILLA_1_8_BRANCH::~nsDOMNavigator_MOZILLA_1_8_BRANCH()
{
  /* destructor code */
}

/* readonly attribute DOMString buildID; */
NS_IMETHODIMP nsDOMNavigator_MOZILLA_1_8_BRANCH::GetBuildID(nsAString & aBuildID)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMNavigator_h__ */
