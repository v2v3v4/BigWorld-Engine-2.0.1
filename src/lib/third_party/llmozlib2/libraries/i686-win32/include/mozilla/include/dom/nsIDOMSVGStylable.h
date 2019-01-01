/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/svg/nsIDOMSVGStylable.idl
 */

#ifndef __gen_nsIDOMSVGStylable_h__
#define __gen_nsIDOMSVGStylable_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMSVGAnimatedString; /* forward declaration */

class nsIDOMCSSStyleDeclaration; /* forward declaration */

class nsIDOMCSSValue; /* forward declaration */


/* starting interface:    nsIDOMSVGStylable */
#define NS_IDOMSVGSTYLABLE_IID_STR "ea8a6cb1-9176-45db-989d-d0e89f563d7e"

#define NS_IDOMSVGSTYLABLE_IID \
  {0xea8a6cb1, 0x9176, 0x45db, \
    { 0x98, 0x9d, 0xd0, 0xe8, 0x9f, 0x56, 0x3d, 0x7e }}

class NS_NO_VTABLE nsIDOMSVGStylable : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGSTYLABLE_IID)

  /* readonly attribute nsIDOMSVGAnimatedString className; */
  NS_IMETHOD GetClassName(nsIDOMSVGAnimatedString * *aClassName) = 0;

  /* readonly attribute nsIDOMCSSStyleDeclaration style; */
  NS_IMETHOD GetStyle(nsIDOMCSSStyleDeclaration * *aStyle) = 0;

  /* nsIDOMCSSValue getPresentationAttribute (in DOMString name); */
  NS_IMETHOD GetPresentationAttribute(const nsAString & name, nsIDOMCSSValue **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGSTYLABLE \
  NS_IMETHOD GetClassName(nsIDOMSVGAnimatedString * *aClassName); \
  NS_IMETHOD GetStyle(nsIDOMCSSStyleDeclaration * *aStyle); \
  NS_IMETHOD GetPresentationAttribute(const nsAString & name, nsIDOMCSSValue **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGSTYLABLE(_to) \
  NS_IMETHOD GetClassName(nsIDOMSVGAnimatedString * *aClassName) { return _to GetClassName(aClassName); } \
  NS_IMETHOD GetStyle(nsIDOMCSSStyleDeclaration * *aStyle) { return _to GetStyle(aStyle); } \
  NS_IMETHOD GetPresentationAttribute(const nsAString & name, nsIDOMCSSValue **_retval) { return _to GetPresentationAttribute(name, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGSTYLABLE(_to) \
  NS_IMETHOD GetClassName(nsIDOMSVGAnimatedString * *aClassName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetClassName(aClassName); } \
  NS_IMETHOD GetStyle(nsIDOMCSSStyleDeclaration * *aStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStyle(aStyle); } \
  NS_IMETHOD GetPresentationAttribute(const nsAString & name, nsIDOMCSSValue **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPresentationAttribute(name, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGStylable : public nsIDOMSVGStylable
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGSTYLABLE

  nsDOMSVGStylable();

private:
  ~nsDOMSVGStylable();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGStylable, nsIDOMSVGStylable)

nsDOMSVGStylable::nsDOMSVGStylable()
{
  /* member initializers and constructor code */
}

nsDOMSVGStylable::~nsDOMSVGStylable()
{
  /* destructor code */
}

/* readonly attribute nsIDOMSVGAnimatedString className; */
NS_IMETHODIMP nsDOMSVGStylable::GetClassName(nsIDOMSVGAnimatedString * *aClassName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMCSSStyleDeclaration style; */
NS_IMETHODIMP nsDOMSVGStylable::GetStyle(nsIDOMCSSStyleDeclaration * *aStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMCSSValue getPresentationAttribute (in DOMString name); */
NS_IMETHODIMP nsDOMSVGStylable::GetPresentationAttribute(const nsAString & name, nsIDOMCSSValue **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMSVGStylable_h__ */
