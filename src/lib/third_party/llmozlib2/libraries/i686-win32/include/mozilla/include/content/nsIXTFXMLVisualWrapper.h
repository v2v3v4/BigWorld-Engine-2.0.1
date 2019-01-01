/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/xtf/public/nsIXTFXMLVisualWrapper.idl
 */

#ifndef __gen_nsIXTFXMLVisualWrapper_h__
#define __gen_nsIXTFXMLVisualWrapper_h__


#ifndef __gen_nsIXTFStyledElementWrapper_h__
#include "nsIXTFStyledElementWrapper.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIXTFXMLVisualWrapper */
#define NS_IXTFXMLVISUALWRAPPER_IID_STR "656c9417-744e-4fa3-8e2b-8218185efe21"

#define NS_IXTFXMLVISUALWRAPPER_IID \
  {0x656c9417, 0x744e, 0x4fa3, \
    { 0x8e, 0x2b, 0x82, 0x18, 0x18, 0x5e, 0xfe, 0x21 }}

class NS_NO_VTABLE nsIXTFXMLVisualWrapper : public nsIXTFStyledElementWrapper {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXTFXMLVISUALWRAPPER_IID)

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXTFXMLVISUALWRAPPER \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXTFXMLVISUALWRAPPER(_to) \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXTFXMLVISUALWRAPPER(_to) \
  /* no methods! */

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXTFXMLVisualWrapper : public nsIXTFXMLVisualWrapper
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFXMLVISUALWRAPPER

  nsXTFXMLVisualWrapper();

private:
  ~nsXTFXMLVisualWrapper();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXTFXMLVisualWrapper, nsIXTFXMLVisualWrapper)

nsXTFXMLVisualWrapper::nsXTFXMLVisualWrapper()
{
  /* member initializers and constructor code */
}

nsXTFXMLVisualWrapper::~nsXTFXMLVisualWrapper()
{
  /* destructor code */
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXTFXMLVisualWrapper_h__ */
