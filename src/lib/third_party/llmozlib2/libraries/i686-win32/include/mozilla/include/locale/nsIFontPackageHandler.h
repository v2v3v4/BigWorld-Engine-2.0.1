/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/intl/locale/idl/nsIFontPackageHandler.idl
 */

#ifndef __gen_nsIFontPackageHandler_h__
#define __gen_nsIFontPackageHandler_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIFontPackageHandler */
#define NS_IFONTPACKAGEHANDLER_IID_STR "6712fdd1-f978-11d4-a144-005004832142"

#define NS_IFONTPACKAGEHANDLER_IID \
  {0x6712fdd1, 0xf978, 0x11d4, \
    { 0xa1, 0x44, 0x00, 0x50, 0x04, 0x83, 0x21, 0x42 }}

class NS_NO_VTABLE nsIFontPackageHandler : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IFONTPACKAGEHANDLER_IID)

  /**
    * Set the font package handler for Gecko
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
    * After the installation, the font package handler should call
    * nsIFontPackageService::FontPackageHandled and pass back the aFontPackID
    * 
    * This interface sould be implemented by the embedding application
    * In the other hand the nsIFontPackageProxy is internal to Gecko
    */
  /* void NeedFontPackage (in string aFontPackID); */
  NS_IMETHOD NeedFontPackage(const char *aFontPackID) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIFONTPACKAGEHANDLER \
  NS_IMETHOD NeedFontPackage(const char *aFontPackID); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIFONTPACKAGEHANDLER(_to) \
  NS_IMETHOD NeedFontPackage(const char *aFontPackID) { return _to NeedFontPackage(aFontPackID); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIFONTPACKAGEHANDLER(_to) \
  NS_IMETHOD NeedFontPackage(const char *aFontPackID) { return !_to ? NS_ERROR_NULL_POINTER : _to->NeedFontPackage(aFontPackID); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsFontPackageHandler : public nsIFontPackageHandler
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIFONTPACKAGEHANDLER

  nsFontPackageHandler();

private:
  ~nsFontPackageHandler();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsFontPackageHandler, nsIFontPackageHandler)

nsFontPackageHandler::nsFontPackageHandler()
{
  /* member initializers and constructor code */
}

nsFontPackageHandler::~nsFontPackageHandler()
{
  /* destructor code */
}

/* void NeedFontPackage (in string aFontPackID); */
NS_IMETHODIMP nsFontPackageHandler::NeedFontPackage(const char *aFontPackID)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIFontPackageHandler_h__ */
