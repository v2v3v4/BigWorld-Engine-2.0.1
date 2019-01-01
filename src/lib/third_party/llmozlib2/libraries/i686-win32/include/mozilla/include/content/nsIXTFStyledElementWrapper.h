/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/xtf/public/nsIXTFStyledElementWrapper.idl
 */

#ifndef __gen_nsIXTFStyledElementWrapper_h__
#define __gen_nsIXTFStyledElementWrapper_h__


#ifndef __gen_nsIXTFElementWrapper_h__
#include "nsIXTFElementWrapper.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIXTFStyledElementWrapper */
#define NS_IXTFSTYLEDELEMENTWRAPPER_IID_STR "814dbfdd-32ff-4734-9aea-b84c925bc9c0"

#define NS_IXTFSTYLEDELEMENTWRAPPER_IID \
  {0x814dbfdd, 0x32ff, 0x4734, \
    { 0x9a, 0xea, 0xb8, 0x4c, 0x92, 0x5b, 0xc9, 0xc0 }}

class NS_NO_VTABLE nsIXTFStyledElementWrapper : public nsIXTFElementWrapper {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXTFSTYLEDELEMENTWRAPPER_IID)

  /**
   * This sets the name of the class attribute.
   * Should be called only during ::onCreated.
   * Note! nsIXTFAttributeHandler can't be used to handle class attribute.
   */
  /* void setClassAttributeName (in nsIAtom name); */
  NS_IMETHOD SetClassAttributeName(nsIAtom *name) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXTFSTYLEDELEMENTWRAPPER \
  NS_IMETHOD SetClassAttributeName(nsIAtom *name); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXTFSTYLEDELEMENTWRAPPER(_to) \
  NS_IMETHOD SetClassAttributeName(nsIAtom *name) { return _to SetClassAttributeName(name); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXTFSTYLEDELEMENTWRAPPER(_to) \
  NS_IMETHOD SetClassAttributeName(nsIAtom *name) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetClassAttributeName(name); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXTFStyledElementWrapper : public nsIXTFStyledElementWrapper
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFSTYLEDELEMENTWRAPPER

  nsXTFStyledElementWrapper();

private:
  ~nsXTFStyledElementWrapper();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXTFStyledElementWrapper, nsIXTFStyledElementWrapper)

nsXTFStyledElementWrapper::nsXTFStyledElementWrapper()
{
  /* member initializers and constructor code */
}

nsXTFStyledElementWrapper::~nsXTFStyledElementWrapper()
{
  /* destructor code */
}

/* void setClassAttributeName (in nsIAtom name); */
NS_IMETHODIMP nsXTFStyledElementWrapper::SetClassAttributeName(nsIAtom *name)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXTFStyledElementWrapper_h__ */
