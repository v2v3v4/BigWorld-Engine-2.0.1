/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/base/nsIDOMNSLocation.idl
 */

#ifndef __gen_nsIDOMNSLocation_h__
#define __gen_nsIDOMNSLocation_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMNSLocation */
#define NS_IDOMNSLOCATION_IID_STR "a6cf9108-15b3-11d2-932e-00805f8add32"

#define NS_IDOMNSLOCATION_IID \
  {0xa6cf9108, 0x15b3, 0x11d2, \
    { 0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32 }}

class NS_NO_VTABLE nsIDOMNSLocation : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMNSLOCATION_IID)

  /* void reload (); */
  NS_IMETHOD Reload(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMNSLOCATION \
  NS_IMETHOD Reload(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMNSLOCATION(_to) \
  NS_IMETHOD Reload(void) { return _to Reload(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMNSLOCATION(_to) \
  NS_IMETHOD Reload(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Reload(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMNSLocation : public nsIDOMNSLocation
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMNSLOCATION

  nsDOMNSLocation();

private:
  ~nsDOMNSLocation();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMNSLocation, nsIDOMNSLocation)

nsDOMNSLocation::nsDOMNSLocation()
{
  /* member initializers and constructor code */
}

nsDOMNSLocation::~nsDOMNSLocation()
{
  /* destructor code */
}

/* void reload (); */
NS_IMETHODIMP nsDOMNSLocation::Reload()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMNSLocation_h__ */
