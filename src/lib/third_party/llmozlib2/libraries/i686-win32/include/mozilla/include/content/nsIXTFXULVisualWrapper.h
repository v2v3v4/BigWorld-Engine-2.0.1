/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/xtf/public/nsIXTFXULVisualWrapper.idl
 */

#ifndef __gen_nsIXTFXULVisualWrapper_h__
#define __gen_nsIXTFXULVisualWrapper_h__


#ifndef __gen_nsIXTFStyledElementWrapper_h__
#include "nsIXTFStyledElementWrapper.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIXTFXULVisualWrapper */
#define NS_IXTFXULVISUALWRAPPER_IID_STR "1ab4b724-fd7e-41af-bc19-80bb7299f3c0"

#define NS_IXTFXULVISUALWRAPPER_IID \
  {0x1ab4b724, 0xfd7e, 0x41af, \
    { 0xbc, 0x19, 0x80, 0xbb, 0x72, 0x99, 0xf3, 0xc0 }}

class NS_NO_VTABLE nsIXTFXULVisualWrapper : public nsIXTFStyledElementWrapper {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXTFXULVISUALWRAPPER_IID)

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXTFXULVISUALWRAPPER \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXTFXULVISUALWRAPPER(_to) \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXTFXULVISUALWRAPPER(_to) \
  /* no methods! */

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXTFXULVisualWrapper : public nsIXTFXULVisualWrapper
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFXULVISUALWRAPPER

  nsXTFXULVisualWrapper();

private:
  ~nsXTFXULVisualWrapper();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXTFXULVisualWrapper, nsIXTFXULVisualWrapper)

nsXTFXULVisualWrapper::nsXTFXULVisualWrapper()
{
  /* member initializers and constructor code */
}

nsXTFXULVisualWrapper::~nsXTFXULVisualWrapper()
{
  /* destructor code */
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXTFXULVisualWrapper_h__ */
