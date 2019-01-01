/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/intl/locale/idl/nsIFontPackageService.idl
 */

#ifndef __gen_nsIFontPackageService_h__
#define __gen_nsIFontPackageService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIFontPackageHandler; /* forward declaration */

// {6712FDD6-F978-11d4-A144-005004832142}
#define NS_FONTPACKAGESERVICE_CID \
{ 0x6712fdd6, 0xf978, 0x11d4, { 0xa1, 0x44, 0x0, 0x50, 0x4, 0x83, 0x21, 0x42 } }
#define NS_FONTPACKAGESERVICE_CONTRACTID \
  "@mozilla.org/intl/fontpackageservice;1"

/* starting interface:    nsIFontPackageService */
#define NS_IFONTPACKAGESERVICE_IID_STR "6712fdd2-f978-11d4-a144-005004832142"

#define NS_IFONTPACKAGESERVICE_IID \
  {0x6712fdd2, 0xf978, 0x11d4, \
    { 0xa1, 0x44, 0x00, 0x50, 0x04, 0x83, 0x21, 0x42 }}

class NS_NO_VTABLE nsIFontPackageService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IFONTPACKAGESERVICE_IID)

  /* void SetHandler (in nsIFontPackageHandler aHandler); */
  NS_IMETHOD SetHandler(nsIFontPackageHandler *aHandler) = 0;

  /* void FontPackageHandled (in boolean aSuccess, in boolean aRedrawPages, in string aFontPackID); */
  NS_IMETHOD FontPackageHandled(PRBool aSuccess, PRBool aRedrawPages, const char *aFontPackID) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIFONTPACKAGESERVICE \
  NS_IMETHOD SetHandler(nsIFontPackageHandler *aHandler); \
  NS_IMETHOD FontPackageHandled(PRBool aSuccess, PRBool aRedrawPages, const char *aFontPackID); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIFONTPACKAGESERVICE(_to) \
  NS_IMETHOD SetHandler(nsIFontPackageHandler *aHandler) { return _to SetHandler(aHandler); } \
  NS_IMETHOD FontPackageHandled(PRBool aSuccess, PRBool aRedrawPages, const char *aFontPackID) { return _to FontPackageHandled(aSuccess, aRedrawPages, aFontPackID); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIFONTPACKAGESERVICE(_to) \
  NS_IMETHOD SetHandler(nsIFontPackageHandler *aHandler) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetHandler(aHandler); } \
  NS_IMETHOD FontPackageHandled(PRBool aSuccess, PRBool aRedrawPages, const char *aFontPackID) { return !_to ? NS_ERROR_NULL_POINTER : _to->FontPackageHandled(aSuccess, aRedrawPages, aFontPackID); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsFontPackageService : public nsIFontPackageService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIFONTPACKAGESERVICE

  nsFontPackageService();

private:
  ~nsFontPackageService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsFontPackageService, nsIFontPackageService)

nsFontPackageService::nsFontPackageService()
{
  /* member initializers and constructor code */
}

nsFontPackageService::~nsFontPackageService()
{
  /* destructor code */
}

/* void SetHandler (in nsIFontPackageHandler aHandler); */
NS_IMETHODIMP nsFontPackageService::SetHandler(nsIFontPackageHandler *aHandler)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void FontPackageHandled (in boolean aSuccess, in boolean aRedrawPages, in string aFontPackID); */
NS_IMETHODIMP nsFontPackageService::FontPackageHandled(PRBool aSuccess, PRBool aRedrawPages, const char *aFontPackID)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIFontPackageService_h__ */
