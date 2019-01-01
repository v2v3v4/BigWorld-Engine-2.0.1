/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/xtf/public/nsIXTFGenericElement.idl
 */

#ifndef __gen_nsIXTFGenericElement_h__
#define __gen_nsIXTFGenericElement_h__


#ifndef __gen_nsIXTFElement_h__
#include "nsIXTFElement.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIXTFGenericElementWrapper; /* forward declaration */


/* starting interface:    nsIXTFGenericElement */
#define NS_IXTFGENERICELEMENT_IID_STR "e339eb1d-3ea8-4c85-87ce-644eb7a19034"

#define NS_IXTFGENERICELEMENT_IID \
  {0xe339eb1d, 0x3ea8, 0x4c85, \
    { 0x87, 0xce, 0x64, 0x4e, 0xb7, 0xa1, 0x90, 0x34 }}

class NS_NO_VTABLE nsIXTFGenericElement : public nsIXTFElement {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXTFGENERICELEMENT_IID)

  /* void onCreated (in nsIXTFGenericElementWrapper wrapper); */
  NS_IMETHOD OnCreated(nsIXTFGenericElementWrapper *wrapper) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXTFGENERICELEMENT \
  NS_IMETHOD OnCreated(nsIXTFGenericElementWrapper *wrapper); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXTFGENERICELEMENT(_to) \
  NS_IMETHOD OnCreated(nsIXTFGenericElementWrapper *wrapper) { return _to OnCreated(wrapper); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXTFGENERICELEMENT(_to) \
  NS_IMETHOD OnCreated(nsIXTFGenericElementWrapper *wrapper) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnCreated(wrapper); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXTFGenericElement : public nsIXTFGenericElement
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFGENERICELEMENT

  nsXTFGenericElement();

private:
  ~nsXTFGenericElement();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXTFGenericElement, nsIXTFGenericElement)

nsXTFGenericElement::nsXTFGenericElement()
{
  /* member initializers and constructor code */
}

nsXTFGenericElement::~nsXTFGenericElement()
{
  /* destructor code */
}

/* void onCreated (in nsIXTFGenericElementWrapper wrapper); */
NS_IMETHODIMP nsXTFGenericElement::OnCreated(nsIXTFGenericElementWrapper *wrapper)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXTFGenericElement_h__ */
