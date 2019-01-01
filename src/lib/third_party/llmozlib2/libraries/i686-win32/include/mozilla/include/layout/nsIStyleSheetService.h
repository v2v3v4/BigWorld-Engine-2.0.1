/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/base/nsIStyleSheetService.idl
 */

#ifndef __gen_nsIStyleSheetService_h__
#define __gen_nsIStyleSheetService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIURI; /* forward declaration */


/* starting interface:    nsIStyleSheetService */
#define NS_ISTYLESHEETSERVICE_IID_STR "41d979dc-ea03-4235-86ff-1e3c090c5630"

#define NS_ISTYLESHEETSERVICE_IID \
  {0x41d979dc, 0xea03, 0x4235, \
    { 0x86, 0xff, 0x1e, 0x3c, 0x09, 0x0c, 0x56, 0x30 }}

class NS_NO_VTABLE nsIStyleSheetService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISTYLESHEETSERVICE_IID)

  enum { AGENT_SHEET = 0U };

  enum { USER_SHEET = 1U };

  /**
   * Synchronously loads a style sheet from |sheetURI| and adds it to the list
   * of user or agent style sheets.
   *
   * A user sheet loaded via this API will come before userContent.css and
   * userChrome.css in the cascade (so the rules in it will have lower
   * precedence than rules in those sheets).
   *
   * An agent sheet loaded via this API will come after ua.css in the cascade
   * (so the rules in it will have higher precedence than rules in ua.css).
   *
   * The relative ordering of two user or two agent sheets loaded via
   * this API is undefined.
   */
  /* void loadAndRegisterSheet (in nsIURI sheetURI, in unsigned long type); */
  NS_IMETHOD LoadAndRegisterSheet(nsIURI *sheetURI, PRUint32 type) = 0;

  /**
   * Returns true if a style sheet at |sheetURI| has previously been
   * added to the list of style sheets specified by |type|.
   */
  /* boolean sheetRegistered (in nsIURI sheetURI, in unsigned long type); */
  NS_IMETHOD SheetRegistered(nsIURI *sheetURI, PRUint32 type, PRBool *_retval) = 0;

  /**
   * Remove the style sheet at |sheetURI| from the list of style
   * sheets specified by |type|.  All documents loaded after
   * this call will no longer use the style sheet.
   */
  /* void unregisterSheet (in nsIURI sheetURI, in unsigned long type); */
  NS_IMETHOD UnregisterSheet(nsIURI *sheetURI, PRUint32 type) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISTYLESHEETSERVICE \
  NS_IMETHOD LoadAndRegisterSheet(nsIURI *sheetURI, PRUint32 type); \
  NS_IMETHOD SheetRegistered(nsIURI *sheetURI, PRUint32 type, PRBool *_retval); \
  NS_IMETHOD UnregisterSheet(nsIURI *sheetURI, PRUint32 type); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISTYLESHEETSERVICE(_to) \
  NS_IMETHOD LoadAndRegisterSheet(nsIURI *sheetURI, PRUint32 type) { return _to LoadAndRegisterSheet(sheetURI, type); } \
  NS_IMETHOD SheetRegistered(nsIURI *sheetURI, PRUint32 type, PRBool *_retval) { return _to SheetRegistered(sheetURI, type, _retval); } \
  NS_IMETHOD UnregisterSheet(nsIURI *sheetURI, PRUint32 type) { return _to UnregisterSheet(sheetURI, type); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISTYLESHEETSERVICE(_to) \
  NS_IMETHOD LoadAndRegisterSheet(nsIURI *sheetURI, PRUint32 type) { return !_to ? NS_ERROR_NULL_POINTER : _to->LoadAndRegisterSheet(sheetURI, type); } \
  NS_IMETHOD SheetRegistered(nsIURI *sheetURI, PRUint32 type, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SheetRegistered(sheetURI, type, _retval); } \
  NS_IMETHOD UnregisterSheet(nsIURI *sheetURI, PRUint32 type) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnregisterSheet(sheetURI, type); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsStyleSheetService : public nsIStyleSheetService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISTYLESHEETSERVICE

  nsStyleSheetService();

private:
  ~nsStyleSheetService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsStyleSheetService, nsIStyleSheetService)

nsStyleSheetService::nsStyleSheetService()
{
  /* member initializers and constructor code */
}

nsStyleSheetService::~nsStyleSheetService()
{
  /* destructor code */
}

/* void loadAndRegisterSheet (in nsIURI sheetURI, in unsigned long type); */
NS_IMETHODIMP nsStyleSheetService::LoadAndRegisterSheet(nsIURI *sheetURI, PRUint32 type)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean sheetRegistered (in nsIURI sheetURI, in unsigned long type); */
NS_IMETHODIMP nsStyleSheetService::SheetRegistered(nsIURI *sheetURI, PRUint32 type, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unregisterSheet (in nsIURI sheetURI, in unsigned long type); */
NS_IMETHODIMP nsStyleSheetService::UnregisterSheet(nsIURI *sheetURI, PRUint32 type)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIStyleSheetService_h__ */
