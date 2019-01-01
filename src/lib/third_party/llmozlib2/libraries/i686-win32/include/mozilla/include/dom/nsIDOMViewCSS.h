/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/css/nsIDOMViewCSS.idl
 */

#ifndef __gen_nsIDOMViewCSS_h__
#define __gen_nsIDOMViewCSS_h__


#ifndef __gen_nsIDOMAbstractView_h__
#include "nsIDOMAbstractView.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMViewCSS */
#define NS_IDOMVIEWCSS_IID_STR "0b9341f3-95d4-4fa4-adcd-e119e0db2889"

#define NS_IDOMVIEWCSS_IID \
  {0x0b9341f3, 0x95d4, 0x4fa4, \
    { 0xad, 0xcd, 0xe1, 0x19, 0xe0, 0xdb, 0x28, 0x89 }}

class NS_NO_VTABLE nsIDOMViewCSS : public nsIDOMAbstractView {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMVIEWCSS_IID)

  /* nsIDOMCSSStyleDeclaration getComputedStyle (in nsIDOMElement elt, in DOMString pseudoElt); */
  NS_IMETHOD GetComputedStyle(nsIDOMElement *elt, const nsAString & pseudoElt, nsIDOMCSSStyleDeclaration **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMVIEWCSS \
  NS_IMETHOD GetComputedStyle(nsIDOMElement *elt, const nsAString & pseudoElt, nsIDOMCSSStyleDeclaration **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMVIEWCSS(_to) \
  NS_IMETHOD GetComputedStyle(nsIDOMElement *elt, const nsAString & pseudoElt, nsIDOMCSSStyleDeclaration **_retval) { return _to GetComputedStyle(elt, pseudoElt, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMVIEWCSS(_to) \
  NS_IMETHOD GetComputedStyle(nsIDOMElement *elt, const nsAString & pseudoElt, nsIDOMCSSStyleDeclaration **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetComputedStyle(elt, pseudoElt, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMViewCSS : public nsIDOMViewCSS
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMVIEWCSS

  nsDOMViewCSS();

private:
  ~nsDOMViewCSS();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMViewCSS, nsIDOMViewCSS)

nsDOMViewCSS::nsDOMViewCSS()
{
  /* member initializers and constructor code */
}

nsDOMViewCSS::~nsDOMViewCSS()
{
  /* destructor code */
}

/* nsIDOMCSSStyleDeclaration getComputedStyle (in nsIDOMElement elt, in DOMString pseudoElt); */
NS_IMETHODIMP nsDOMViewCSS::GetComputedStyle(nsIDOMElement *elt, const nsAString & pseudoElt, nsIDOMCSSStyleDeclaration **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMViewCSS_h__ */
