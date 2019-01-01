/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/widget/public/nsIMenuRollup.idl
 */

#ifndef __gen_nsIMenuRollup_h__
#define __gen_nsIMenuRollup_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsISupportsArray_h__
#include "nsISupportsArray.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIMenuRollup */
#define NS_IMENUROLLUP_IID_STR "05c48880-0fcf-11d4-bb6f-d9f289fe803c"

#define NS_IMENUROLLUP_IID \
  {0x05c48880, 0x0fcf, 0x11d4, \
    { 0xbb, 0x6f, 0xd9, 0xf2, 0x89, 0xfe, 0x80, 0x3c }}

class NS_NO_VTABLE nsIMenuRollup : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IMENUROLLUP_IID)

  /* nsISupportsArray GetSubmenuWidgetChain (); */
  NS_IMETHOD GetSubmenuWidgetChain(nsISupportsArray **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIMENUROLLUP \
  NS_IMETHOD GetSubmenuWidgetChain(nsISupportsArray **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIMENUROLLUP(_to) \
  NS_IMETHOD GetSubmenuWidgetChain(nsISupportsArray **_retval) { return _to GetSubmenuWidgetChain(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIMENUROLLUP(_to) \
  NS_IMETHOD GetSubmenuWidgetChain(nsISupportsArray **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSubmenuWidgetChain(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsMenuRollup : public nsIMenuRollup
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIMENUROLLUP

  nsMenuRollup();

private:
  ~nsMenuRollup();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsMenuRollup, nsIMenuRollup)

nsMenuRollup::nsMenuRollup()
{
  /* member initializers and constructor code */
}

nsMenuRollup::~nsMenuRollup()
{
  /* destructor code */
}

/* nsISupportsArray GetSubmenuWidgetChain (); */
NS_IMETHODIMP nsMenuRollup::GetSubmenuWidgetChain(nsISupportsArray **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIMenuRollup_h__ */
