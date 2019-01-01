/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/xtf/public/nsIXTFSVGVisualWrapper.idl
 */

#ifndef __gen_nsIXTFSVGVisualWrapper_h__
#define __gen_nsIXTFSVGVisualWrapper_h__


#ifndef __gen_nsIXTFStyledElementWrapper_h__
#include "nsIXTFStyledElementWrapper.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIXTFSVGVisualWrapper */
#define NS_IXTFSVGVISUALWRAPPER_IID_STR "78582ad3-db1e-4aa6-a15b-b055a7846352"

#define NS_IXTFSVGVISUALWRAPPER_IID \
  {0x78582ad3, 0xdb1e, 0x4aa6, \
    { 0xa1, 0x5b, 0xb0, 0x55, 0xa7, 0x84, 0x63, 0x52 }}

class NS_NO_VTABLE nsIXTFSVGVisualWrapper : public nsIXTFStyledElementWrapper {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXTFSVGVISUALWRAPPER_IID)

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXTFSVGVISUALWRAPPER \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXTFSVGVISUALWRAPPER(_to) \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXTFSVGVISUALWRAPPER(_to) \
  /* no methods! */

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXTFSVGVisualWrapper : public nsIXTFSVGVisualWrapper
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFSVGVISUALWRAPPER

  nsXTFSVGVisualWrapper();

private:
  ~nsXTFSVGVisualWrapper();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXTFSVGVisualWrapper, nsIXTFSVGVisualWrapper)

nsXTFSVGVisualWrapper::nsXTFSVGVisualWrapper()
{
  /* member initializers and constructor code */
}

nsXTFSVGVisualWrapper::~nsXTFSVGVisualWrapper()
{
  /* destructor code */
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXTFSVGVisualWrapper_h__ */
