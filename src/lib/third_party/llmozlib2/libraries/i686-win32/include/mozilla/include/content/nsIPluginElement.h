/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/html/content/public/nsIPluginElement.idl
 */

#ifndef __gen_nsIPluginElement_h__
#define __gen_nsIPluginElement_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIPluginElement */
#define NS_IPLUGINELEMENT_IID_STR "41be252d-c47b-40f3-94bc-bffe51762d68"

#define NS_IPLUGINELEMENT_IID \
  {0x41be252d, 0xc47b, 0x40f3, \
    { 0x94, 0xbc, 0xbf, 0xfe, 0x51, 0x76, 0x2d, 0x68 }}

/**
 * Interface which plugin elements (e.g. embed and object) implement
 * (but don't automatically expose to JS) in addition to their dom
 * specific interface.
 */
class NS_NO_VTABLE nsIPluginElement : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPLUGINELEMENT_IID)

  /**
   * The actual mime type (the one we got back from the network
   * request) for the plugin element.
   */
  /* readonly attribute ACString actualType; */
  NS_IMETHOD GetActualType(nsACString & aActualType) = 0;

  /**
   * Non-scriptable setter for the actual mime type (the one we got
   * back from the network request).
   */
  /* [noscript] void setActualType (in ACString actualType); */
  NS_IMETHOD SetActualType(const nsACString & actualType) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPLUGINELEMENT \
  NS_IMETHOD GetActualType(nsACString & aActualType); \
  NS_IMETHOD SetActualType(const nsACString & actualType); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPLUGINELEMENT(_to) \
  NS_IMETHOD GetActualType(nsACString & aActualType) { return _to GetActualType(aActualType); } \
  NS_IMETHOD SetActualType(const nsACString & actualType) { return _to SetActualType(actualType); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPLUGINELEMENT(_to) \
  NS_IMETHOD GetActualType(nsACString & aActualType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetActualType(aActualType); } \
  NS_IMETHOD SetActualType(const nsACString & actualType) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetActualType(actualType); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsPluginElement : public nsIPluginElement
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPLUGINELEMENT

  nsPluginElement();

private:
  ~nsPluginElement();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsPluginElement, nsIPluginElement)

nsPluginElement::nsPluginElement()
{
  /* member initializers and constructor code */
}

nsPluginElement::~nsPluginElement()
{
  /* destructor code */
}

/* readonly attribute ACString actualType; */
NS_IMETHODIMP nsPluginElement::GetActualType(nsACString & aActualType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void setActualType (in ACString actualType); */
NS_IMETHODIMP nsPluginElement::SetActualType(const nsACString & actualType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIPluginElement_h__ */
