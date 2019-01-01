/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/xtf/public/nsIXTFBindableElementWrapper.idl
 */

#ifndef __gen_nsIXTFBindableElementWrapper_h__
#define __gen_nsIXTFBindableElementWrapper_h__


#ifndef __gen_nsIXTFStyledElementWrapper_h__
#include "nsIXTFStyledElementWrapper.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIXTFBindableElementWrapper */
#define NS_IXTFBINDABLEELEMENTWRAPPER_IID_STR "7914d5f2-14b4-47d0-88f8-70715cfbdfb7"

#define NS_IXTFBINDABLEELEMENTWRAPPER_IID \
  {0x7914d5f2, 0x14b4, 0x47d0, \
    { 0x88, 0xf8, 0x70, 0x71, 0x5c, 0xfb, 0xdf, 0xb7 }}

class NS_NO_VTABLE nsIXTFBindableElementWrapper : public nsIXTFStyledElementWrapper {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXTFBINDABLEELEMENTWRAPPER_IID)

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXTFBINDABLEELEMENTWRAPPER \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXTFBINDABLEELEMENTWRAPPER(_to) \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXTFBINDABLEELEMENTWRAPPER(_to) \
  /* no methods! */

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXTFBindableElementWrapper : public nsIXTFBindableElementWrapper
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFBINDABLEELEMENTWRAPPER

  nsXTFBindableElementWrapper();

private:
  ~nsXTFBindableElementWrapper();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXTFBindableElementWrapper, nsIXTFBindableElementWrapper)

nsXTFBindableElementWrapper::nsXTFBindableElementWrapper()
{
  /* member initializers and constructor code */
}

nsXTFBindableElementWrapper::~nsXTFBindableElementWrapper()
{
  /* destructor code */
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXTFBindableElementWrapper_h__ */
