/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/xbl/nsIDOMDocumentXBL.idl
 */

#ifndef __gen_nsIDOMDocumentXBL_h__
#define __gen_nsIDOMDocumentXBL_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMDocumentXBL */
#define NS_IDOMDOCUMENTXBL_IID_STR "c7c0ae9b-a0ba-4f4e-9f2c-c18deb62ee8b"

#define NS_IDOMDOCUMENTXBL_IID \
  {0xc7c0ae9b, 0xa0ba, 0x4f4e, \
    { 0x9f, 0x2c, 0xc1, 0x8d, 0xeb, 0x62, 0xee, 0x8b }}

class NS_NO_VTABLE nsIDOMDocumentXBL : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMDOCUMENTXBL_IID)

  /* nsIDOMNodeList getAnonymousNodes (in nsIDOMElement elt); */
  NS_IMETHOD GetAnonymousNodes(nsIDOMElement *elt, nsIDOMNodeList **_retval) = 0;

  /* nsIDOMElement getAnonymousElementByAttribute (in nsIDOMElement elt, in DOMString attrName, in DOMString attrValue); */
  NS_IMETHOD GetAnonymousElementByAttribute(nsIDOMElement *elt, const nsAString & attrName, const nsAString & attrValue, nsIDOMElement **_retval) = 0;

  /* void addBinding (in nsIDOMElement elt, in DOMString bindingURL); */
  NS_IMETHOD AddBinding(nsIDOMElement *elt, const nsAString & bindingURL) = 0;

  /* void removeBinding (in nsIDOMElement elt, in DOMString bindingURL); */
  NS_IMETHOD RemoveBinding(nsIDOMElement *elt, const nsAString & bindingURL) = 0;

  /* nsIDOMElement getBindingParent (in nsIDOMNode node); */
  NS_IMETHOD GetBindingParent(nsIDOMNode *node, nsIDOMElement **_retval) = 0;

  /* nsIDOMDocument loadBindingDocument (in DOMString documentURL); */
  NS_IMETHOD LoadBindingDocument(const nsAString & documentURL, nsIDOMDocument **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMDOCUMENTXBL \
  NS_IMETHOD GetAnonymousNodes(nsIDOMElement *elt, nsIDOMNodeList **_retval); \
  NS_IMETHOD GetAnonymousElementByAttribute(nsIDOMElement *elt, const nsAString & attrName, const nsAString & attrValue, nsIDOMElement **_retval); \
  NS_IMETHOD AddBinding(nsIDOMElement *elt, const nsAString & bindingURL); \
  NS_IMETHOD RemoveBinding(nsIDOMElement *elt, const nsAString & bindingURL); \
  NS_IMETHOD GetBindingParent(nsIDOMNode *node, nsIDOMElement **_retval); \
  NS_IMETHOD LoadBindingDocument(const nsAString & documentURL, nsIDOMDocument **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMDOCUMENTXBL(_to) \
  NS_IMETHOD GetAnonymousNodes(nsIDOMElement *elt, nsIDOMNodeList **_retval) { return _to GetAnonymousNodes(elt, _retval); } \
  NS_IMETHOD GetAnonymousElementByAttribute(nsIDOMElement *elt, const nsAString & attrName, const nsAString & attrValue, nsIDOMElement **_retval) { return _to GetAnonymousElementByAttribute(elt, attrName, attrValue, _retval); } \
  NS_IMETHOD AddBinding(nsIDOMElement *elt, const nsAString & bindingURL) { return _to AddBinding(elt, bindingURL); } \
  NS_IMETHOD RemoveBinding(nsIDOMElement *elt, const nsAString & bindingURL) { return _to RemoveBinding(elt, bindingURL); } \
  NS_IMETHOD GetBindingParent(nsIDOMNode *node, nsIDOMElement **_retval) { return _to GetBindingParent(node, _retval); } \
  NS_IMETHOD LoadBindingDocument(const nsAString & documentURL, nsIDOMDocument **_retval) { return _to LoadBindingDocument(documentURL, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMDOCUMENTXBL(_to) \
  NS_IMETHOD GetAnonymousNodes(nsIDOMElement *elt, nsIDOMNodeList **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAnonymousNodes(elt, _retval); } \
  NS_IMETHOD GetAnonymousElementByAttribute(nsIDOMElement *elt, const nsAString & attrName, const nsAString & attrValue, nsIDOMElement **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAnonymousElementByAttribute(elt, attrName, attrValue, _retval); } \
  NS_IMETHOD AddBinding(nsIDOMElement *elt, const nsAString & bindingURL) { return !_to ? NS_ERROR_NULL_POINTER : _to->AddBinding(elt, bindingURL); } \
  NS_IMETHOD RemoveBinding(nsIDOMElement *elt, const nsAString & bindingURL) { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveBinding(elt, bindingURL); } \
  NS_IMETHOD GetBindingParent(nsIDOMNode *node, nsIDOMElement **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBindingParent(node, _retval); } \
  NS_IMETHOD LoadBindingDocument(const nsAString & documentURL, nsIDOMDocument **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->LoadBindingDocument(documentURL, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMDocumentXBL : public nsIDOMDocumentXBL
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMDOCUMENTXBL

  nsDOMDocumentXBL();

private:
  ~nsDOMDocumentXBL();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMDocumentXBL, nsIDOMDocumentXBL)

nsDOMDocumentXBL::nsDOMDocumentXBL()
{
  /* member initializers and constructor code */
}

nsDOMDocumentXBL::~nsDOMDocumentXBL()
{
  /* destructor code */
}

/* nsIDOMNodeList getAnonymousNodes (in nsIDOMElement elt); */
NS_IMETHODIMP nsDOMDocumentXBL::GetAnonymousNodes(nsIDOMElement *elt, nsIDOMNodeList **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMElement getAnonymousElementByAttribute (in nsIDOMElement elt, in DOMString attrName, in DOMString attrValue); */
NS_IMETHODIMP nsDOMDocumentXBL::GetAnonymousElementByAttribute(nsIDOMElement *elt, const nsAString & attrName, const nsAString & attrValue, nsIDOMElement **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void addBinding (in nsIDOMElement elt, in DOMString bindingURL); */
NS_IMETHODIMP nsDOMDocumentXBL::AddBinding(nsIDOMElement *elt, const nsAString & bindingURL)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeBinding (in nsIDOMElement elt, in DOMString bindingURL); */
NS_IMETHODIMP nsDOMDocumentXBL::RemoveBinding(nsIDOMElement *elt, const nsAString & bindingURL)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMElement getBindingParent (in nsIDOMNode node); */
NS_IMETHODIMP nsDOMDocumentXBL::GetBindingParent(nsIDOMNode *node, nsIDOMElement **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMDocument loadBindingDocument (in DOMString documentURL); */
NS_IMETHODIMP nsDOMDocumentXBL::LoadBindingDocument(const nsAString & documentURL, nsIDOMDocument **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMDocumentXBL_h__ */
