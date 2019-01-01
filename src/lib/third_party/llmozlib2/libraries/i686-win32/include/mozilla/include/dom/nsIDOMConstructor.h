/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/base/nsIDOMConstructor.idl
 */

#ifndef __gen_nsIDOMConstructor_h__
#define __gen_nsIDOMConstructor_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMConstructor */
#define NS_IDOMCONSTRUCTOR_IID_STR "0ccbcf19-d1b4-489e-984c-cd8c43672bb9"

#define NS_IDOMCONSTRUCTOR_IID \
  {0x0ccbcf19, 0xd1b4, 0x489e, \
    { 0x98, 0x4c, 0xcd, 0x8c, 0x43, 0x67, 0x2b, 0xb9 }}

class NS_NO_VTABLE nsIDOMConstructor : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMCONSTRUCTOR_IID)

  /* AString toString (); */
  NS_IMETHOD ToString(nsAString & _retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMCONSTRUCTOR \
  NS_IMETHOD ToString(nsAString & _retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMCONSTRUCTOR(_to) \
  NS_IMETHOD ToString(nsAString & _retval) { return _to ToString(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMCONSTRUCTOR(_to) \
  NS_IMETHOD ToString(nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ToString(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMConstructor : public nsIDOMConstructor
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMCONSTRUCTOR

  nsDOMConstructor();

private:
  ~nsDOMConstructor();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMConstructor, nsIDOMConstructor)

nsDOMConstructor::nsDOMConstructor()
{
  /* member initializers and constructor code */
}

nsDOMConstructor::~nsDOMConstructor()
{
  /* destructor code */
}

/* AString toString (); */
NS_IMETHODIMP nsDOMConstructor::ToString(nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMConstructor_h__ */
