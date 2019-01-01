/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/xtf/public/nsIXTFGenericElementWrapper.idl
 */

#ifndef __gen_nsIXTFGenericElementWrapper_h__
#define __gen_nsIXTFGenericElementWrapper_h__


#ifndef __gen_nsIXTFElementWrapper_h__
#include "nsIXTFElementWrapper.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIXTFGenericElementWrapper */
#define NS_IXTFGENERICELEMENTWRAPPER_IID_STR "5e0cf513-5b87-4da2-a5ce-d9ba3a30d540"

#define NS_IXTFGENERICELEMENTWRAPPER_IID \
  {0x5e0cf513, 0x5b87, 0x4da2, \
    { 0xa5, 0xce, 0xd9, 0xba, 0x3a, 0x30, 0xd5, 0x40 }}

class NS_NO_VTABLE nsIXTFGenericElementWrapper : public nsIXTFElementWrapper {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXTFGENERICELEMENTWRAPPER_IID)

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXTFGENERICELEMENTWRAPPER \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXTFGENERICELEMENTWRAPPER(_to) \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXTFGENERICELEMENTWRAPPER(_to) \
  /* no methods! */

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXTFGenericElementWrapper : public nsIXTFGenericElementWrapper
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFGENERICELEMENTWRAPPER

  nsXTFGenericElementWrapper();

private:
  ~nsXTFGenericElementWrapper();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXTFGenericElementWrapper, nsIXTFGenericElementWrapper)

nsXTFGenericElementWrapper::nsXTFGenericElementWrapper()
{
  /* member initializers and constructor code */
}

nsXTFGenericElementWrapper::~nsXTFGenericElementWrapper()
{
  /* destructor code */
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXTFGenericElementWrapper_h__ */
