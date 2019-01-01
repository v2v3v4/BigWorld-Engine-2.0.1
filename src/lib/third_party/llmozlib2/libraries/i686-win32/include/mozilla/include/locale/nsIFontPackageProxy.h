/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/intl/locale/idl/nsIFontPackageProxy.idl
 */

#ifndef __gen_nsIFontPackageProxy_h__
#define __gen_nsIFontPackageProxy_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIFontPackageProxy */
#define NS_IFONTPACKAGEPROXY_IID_STR "6712fdd4-f978-11d4-a144-005004832142"

#define NS_IFONTPACKAGEPROXY_IID \
  {0x6712fdd4, 0xf978, 0x11d4, \
    { 0xa1, 0x44, 0x00, 0x50, 0x04, 0x83, 0x21, 0x42 }}

class NS_NO_VTABLE nsIFontPackageProxy : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IFONTPACKAGEPROXY_IID)

  /**
    * Request a font package. The proxy will call font package 
    * handler to download the font package
    * @param aFontPackID a font package ID.
    * The id have the following naming convenation 
    *  name_space:name
    * we currently define one name space
    *    lang - a font package for a particular language group
    * and udnder the lang name space, we use the language code to identify
    * the package, below are the defined packages
    *   ja - Japanese font package ("lang:ja")
    *   ko - Korean font package ("lang:ko")
    *   zh-TW - Traditional Chinese font package ("lang:zh-TW")
    *   zh-CN - Simplified Chinese font package ("lang:zh-CN")
    * In the future, we may want to add
    *    ar - Arabic font packges ("lang:ar")
    *    he - Hebrew font packages ("lang:he")
    *    th - Thai font package ("lang:th")
    * other name space are reserved for now.
    *
    * This interface is implemented by Gecko for internal purpose
    * It is a service and are different from nsIFontPackageHandler although
    * the method signature is the same. 
    * Embedding application should not see this interface.
    */
  /* void NeedFontPackage (in string aFontPackID); */
  NS_IMETHOD NeedFontPackage(const char *aFontPackID) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIFONTPACKAGEPROXY \
  NS_IMETHOD NeedFontPackage(const char *aFontPackID); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIFONTPACKAGEPROXY(_to) \
  NS_IMETHOD NeedFontPackage(const char *aFontPackID) { return _to NeedFontPackage(aFontPackID); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIFONTPACKAGEPROXY(_to) \
  NS_IMETHOD NeedFontPackage(const char *aFontPackID) { return !_to ? NS_ERROR_NULL_POINTER : _to->NeedFontPackage(aFontPackID); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsFontPackageProxy : public nsIFontPackageProxy
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIFONTPACKAGEPROXY

  nsFontPackageProxy();

private:
  ~nsFontPackageProxy();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsFontPackageProxy, nsIFontPackageProxy)

nsFontPackageProxy::nsFontPackageProxy()
{
  /* member initializers and constructor code */
}

nsFontPackageProxy::~nsFontPackageProxy()
{
  /* destructor code */
}

/* void NeedFontPackage (in string aFontPackID); */
NS_IMETHODIMP nsFontPackageProxy::NeedFontPackage(const char *aFontPackID)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIFontPackageProxy_h__ */
