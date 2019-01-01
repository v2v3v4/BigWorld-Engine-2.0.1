/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/html/nsIDOMHTMLCanvasElement.idl
 */

#ifndef __gen_nsIDOMHTMLCanvasElement_h__
#define __gen_nsIDOMHTMLCanvasElement_h__


#ifndef __gen_nsIDOMHTMLElement_h__
#include "nsIDOMHTMLElement.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMHTMLCanvasElement */
#define NS_IDOMHTMLCANVASELEMENT_IID_STR "0583a2ea-ab19-40e1-8be4-5e9b2f275560"

#define NS_IDOMHTMLCANVASELEMENT_IID \
  {0x0583a2ea, 0xab19, 0x40e1, \
    { 0x8b, 0xe4, 0x5e, 0x9b, 0x2f, 0x27, 0x55, 0x60 }}

/**
 * The nsIDOMHTMLCanvasElement interface is the interface to a HTML
 * <canvas> element.
 *
 * For more information on this interface, please see
 * http://www.whatwg.org/specs/web-apps/current-work/#graphics
 *
 * @status UNDER_DEVELOPMENT
 */
class NS_NO_VTABLE nsIDOMHTMLCanvasElement : public nsIDOMHTMLElement {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMHTMLCANVASELEMENT_IID)

  /* attribute long width; */
  NS_IMETHOD GetWidth(PRInt32 *aWidth) = 0;
  NS_IMETHOD SetWidth(PRInt32 aWidth) = 0;

  /* attribute long height; */
  NS_IMETHOD GetHeight(PRInt32 *aHeight) = 0;
  NS_IMETHOD SetHeight(PRInt32 aHeight) = 0;

  /* nsISupports getContext (in DOMString contextId); */
  NS_IMETHOD GetContext(const nsAString & contextId, nsISupports **_retval) = 0;

  /* DOMString toDataURL (); */
  NS_IMETHOD ToDataURL(nsAString & _retval) = 0;

  /* [noscript] DOMString toDataURLAs (in DOMString mimeType, in DOMString encoderOptions); */
  NS_IMETHOD ToDataURLAs(const nsAString & mimeType, const nsAString & encoderOptions, nsAString & _retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMHTMLCANVASELEMENT \
  NS_IMETHOD GetWidth(PRInt32 *aWidth); \
  NS_IMETHOD SetWidth(PRInt32 aWidth); \
  NS_IMETHOD GetHeight(PRInt32 *aHeight); \
  NS_IMETHOD SetHeight(PRInt32 aHeight); \
  NS_IMETHOD GetContext(const nsAString & contextId, nsISupports **_retval); \
  NS_IMETHOD ToDataURL(nsAString & _retval); \
  NS_IMETHOD ToDataURLAs(const nsAString & mimeType, const nsAString & encoderOptions, nsAString & _retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMHTMLCANVASELEMENT(_to) \
  NS_IMETHOD GetWidth(PRInt32 *aWidth) { return _to GetWidth(aWidth); } \
  NS_IMETHOD SetWidth(PRInt32 aWidth) { return _to SetWidth(aWidth); } \
  NS_IMETHOD GetHeight(PRInt32 *aHeight) { return _to GetHeight(aHeight); } \
  NS_IMETHOD SetHeight(PRInt32 aHeight) { return _to SetHeight(aHeight); } \
  NS_IMETHOD GetContext(const nsAString & contextId, nsISupports **_retval) { return _to GetContext(contextId, _retval); } \
  NS_IMETHOD ToDataURL(nsAString & _retval) { return _to ToDataURL(_retval); } \
  NS_IMETHOD ToDataURLAs(const nsAString & mimeType, const nsAString & encoderOptions, nsAString & _retval) { return _to ToDataURLAs(mimeType, encoderOptions, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMHTMLCANVASELEMENT(_to) \
  NS_IMETHOD GetWidth(PRInt32 *aWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetWidth(aWidth); } \
  NS_IMETHOD SetWidth(PRInt32 aWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetWidth(aWidth); } \
  NS_IMETHOD GetHeight(PRInt32 *aHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHeight(aHeight); } \
  NS_IMETHOD SetHeight(PRInt32 aHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetHeight(aHeight); } \
  NS_IMETHOD GetContext(const nsAString & contextId, nsISupports **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetContext(contextId, _retval); } \
  NS_IMETHOD ToDataURL(nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ToDataURL(_retval); } \
  NS_IMETHOD ToDataURLAs(const nsAString & mimeType, const nsAString & encoderOptions, nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ToDataURLAs(mimeType, encoderOptions, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMHTMLCanvasElement : public nsIDOMHTMLCanvasElement
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMHTMLCANVASELEMENT

  nsDOMHTMLCanvasElement();

private:
  ~nsDOMHTMLCanvasElement();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMHTMLCanvasElement, nsIDOMHTMLCanvasElement)

nsDOMHTMLCanvasElement::nsDOMHTMLCanvasElement()
{
  /* member initializers and constructor code */
}

nsDOMHTMLCanvasElement::~nsDOMHTMLCanvasElement()
{
  /* destructor code */
}

/* attribute long width; */
NS_IMETHODIMP nsDOMHTMLCanvasElement::GetWidth(PRInt32 *aWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMHTMLCanvasElement::SetWidth(PRInt32 aWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long height; */
NS_IMETHODIMP nsDOMHTMLCanvasElement::GetHeight(PRInt32 *aHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMHTMLCanvasElement::SetHeight(PRInt32 aHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsISupports getContext (in DOMString contextId); */
NS_IMETHODIMP nsDOMHTMLCanvasElement::GetContext(const nsAString & contextId, nsISupports **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* DOMString toDataURL (); */
NS_IMETHODIMP nsDOMHTMLCanvasElement::ToDataURL(nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] DOMString toDataURLAs (in DOMString mimeType, in DOMString encoderOptions); */
NS_IMETHODIMP nsDOMHTMLCanvasElement::ToDataURLAs(const nsAString & mimeType, const nsAString & encoderOptions, nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMHTMLCanvasElement_h__ */
