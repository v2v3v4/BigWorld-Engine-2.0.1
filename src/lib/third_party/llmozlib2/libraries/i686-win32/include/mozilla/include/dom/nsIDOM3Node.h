/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/core/nsIDOM3Node.idl
 */

#ifndef __gen_nsIDOM3Node_h__
#define __gen_nsIDOM3Node_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIVariant; /* forward declaration */

class nsIDOMUserDataHandler; /* forward declaration */


/* starting interface:    nsIDOM3Node */
#define NS_IDOM3NODE_IID_STR "29fb2a18-1dd2-11b2-8dd9-a6fd5d5ad12f"

#define NS_IDOM3NODE_IID \
  {0x29fb2a18, 0x1dd2, 0x11b2, \
    { 0x8d, 0xd9, 0xa6, 0xfd, 0x5d, 0x5a, 0xd1, 0x2f }}

class NS_NO_VTABLE nsIDOM3Node : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOM3NODE_IID)

  /* readonly attribute DOMString baseURI; */
  NS_IMETHOD GetBaseURI(nsAString & aBaseURI) = 0;

  enum { DOCUMENT_POSITION_DISCONNECTED = 1U };

  enum { DOCUMENT_POSITION_PRECEDING = 2U };

  enum { DOCUMENT_POSITION_FOLLOWING = 4U };

  enum { DOCUMENT_POSITION_CONTAINS = 8U };

  enum { DOCUMENT_POSITION_CONTAINED_BY = 16U };

  enum { DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC = 32U };

  /* unsigned short compareDocumentPosition (in nsIDOMNode other)  raises (DOMException); */
  NS_IMETHOD CompareDocumentPosition(nsIDOMNode *other, PRUint16 *_retval) = 0;

  /* attribute DOMString textContent; */
  NS_IMETHOD GetTextContent(nsAString & aTextContent) = 0;
  NS_IMETHOD SetTextContent(const nsAString & aTextContent) = 0;

  /* boolean isSameNode (in nsIDOMNode other); */
  NS_IMETHOD IsSameNode(nsIDOMNode *other, PRBool *_retval) = 0;

  /* DOMString lookupPrefix (in DOMString namespaceURI); */
  NS_IMETHOD LookupPrefix(const nsAString & namespaceURI, nsAString & _retval) = 0;

  /* boolean isDefaultNamespace (in DOMString namespaceURI); */
  NS_IMETHOD IsDefaultNamespace(const nsAString & namespaceURI, PRBool *_retval) = 0;

  /* DOMString lookupNamespaceURI (in DOMString prefix); */
  NS_IMETHOD LookupNamespaceURI(const nsAString & prefix, nsAString & _retval) = 0;

  /* boolean isEqualNode (in nsIDOMNode arg); */
  NS_IMETHOD IsEqualNode(nsIDOMNode *arg, PRBool *_retval) = 0;

  /* nsISupports getFeature (in DOMString feature, in DOMString version); */
  NS_IMETHOD GetFeature(const nsAString & feature, const nsAString & version, nsISupports **_retval) = 0;

  /* nsIVariant setUserData (in DOMString key, in nsIVariant data, in nsIDOMUserDataHandler handler); */
  NS_IMETHOD SetUserData(const nsAString & key, nsIVariant *data, nsIDOMUserDataHandler *handler, nsIVariant **_retval) = 0;

  /* nsIVariant getUserData (in DOMString key); */
  NS_IMETHOD GetUserData(const nsAString & key, nsIVariant **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOM3NODE \
  NS_IMETHOD GetBaseURI(nsAString & aBaseURI); \
  NS_IMETHOD CompareDocumentPosition(nsIDOMNode *other, PRUint16 *_retval); \
  NS_IMETHOD GetTextContent(nsAString & aTextContent); \
  NS_IMETHOD SetTextContent(const nsAString & aTextContent); \
  NS_IMETHOD IsSameNode(nsIDOMNode *other, PRBool *_retval); \
  NS_IMETHOD LookupPrefix(const nsAString & namespaceURI, nsAString & _retval); \
  NS_IMETHOD IsDefaultNamespace(const nsAString & namespaceURI, PRBool *_retval); \
  NS_IMETHOD LookupNamespaceURI(const nsAString & prefix, nsAString & _retval); \
  NS_IMETHOD IsEqualNode(nsIDOMNode *arg, PRBool *_retval); \
  NS_IMETHOD GetFeature(const nsAString & feature, const nsAString & version, nsISupports **_retval); \
  NS_IMETHOD SetUserData(const nsAString & key, nsIVariant *data, nsIDOMUserDataHandler *handler, nsIVariant **_retval); \
  NS_IMETHOD GetUserData(const nsAString & key, nsIVariant **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOM3NODE(_to) \
  NS_IMETHOD GetBaseURI(nsAString & aBaseURI) { return _to GetBaseURI(aBaseURI); } \
  NS_IMETHOD CompareDocumentPosition(nsIDOMNode *other, PRUint16 *_retval) { return _to CompareDocumentPosition(other, _retval); } \
  NS_IMETHOD GetTextContent(nsAString & aTextContent) { return _to GetTextContent(aTextContent); } \
  NS_IMETHOD SetTextContent(const nsAString & aTextContent) { return _to SetTextContent(aTextContent); } \
  NS_IMETHOD IsSameNode(nsIDOMNode *other, PRBool *_retval) { return _to IsSameNode(other, _retval); } \
  NS_IMETHOD LookupPrefix(const nsAString & namespaceURI, nsAString & _retval) { return _to LookupPrefix(namespaceURI, _retval); } \
  NS_IMETHOD IsDefaultNamespace(const nsAString & namespaceURI, PRBool *_retval) { return _to IsDefaultNamespace(namespaceURI, _retval); } \
  NS_IMETHOD LookupNamespaceURI(const nsAString & prefix, nsAString & _retval) { return _to LookupNamespaceURI(prefix, _retval); } \
  NS_IMETHOD IsEqualNode(nsIDOMNode *arg, PRBool *_retval) { return _to IsEqualNode(arg, _retval); } \
  NS_IMETHOD GetFeature(const nsAString & feature, const nsAString & version, nsISupports **_retval) { return _to GetFeature(feature, version, _retval); } \
  NS_IMETHOD SetUserData(const nsAString & key, nsIVariant *data, nsIDOMUserDataHandler *handler, nsIVariant **_retval) { return _to SetUserData(key, data, handler, _retval); } \
  NS_IMETHOD GetUserData(const nsAString & key, nsIVariant **_retval) { return _to GetUserData(key, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOM3NODE(_to) \
  NS_IMETHOD GetBaseURI(nsAString & aBaseURI) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBaseURI(aBaseURI); } \
  NS_IMETHOD CompareDocumentPosition(nsIDOMNode *other, PRUint16 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CompareDocumentPosition(other, _retval); } \
  NS_IMETHOD GetTextContent(nsAString & aTextContent) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTextContent(aTextContent); } \
  NS_IMETHOD SetTextContent(const nsAString & aTextContent) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTextContent(aTextContent); } \
  NS_IMETHOD IsSameNode(nsIDOMNode *other, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->IsSameNode(other, _retval); } \
  NS_IMETHOD LookupPrefix(const nsAString & namespaceURI, nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->LookupPrefix(namespaceURI, _retval); } \
  NS_IMETHOD IsDefaultNamespace(const nsAString & namespaceURI, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->IsDefaultNamespace(namespaceURI, _retval); } \
  NS_IMETHOD LookupNamespaceURI(const nsAString & prefix, nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->LookupNamespaceURI(prefix, _retval); } \
  NS_IMETHOD IsEqualNode(nsIDOMNode *arg, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->IsEqualNode(arg, _retval); } \
  NS_IMETHOD GetFeature(const nsAString & feature, const nsAString & version, nsISupports **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFeature(feature, version, _retval); } \
  NS_IMETHOD SetUserData(const nsAString & key, nsIVariant *data, nsIDOMUserDataHandler *handler, nsIVariant **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetUserData(key, data, handler, _retval); } \
  NS_IMETHOD GetUserData(const nsAString & key, nsIVariant **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetUserData(key, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOM3Node : public nsIDOM3Node
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOM3NODE

  nsDOM3Node();

private:
  ~nsDOM3Node();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOM3Node, nsIDOM3Node)

nsDOM3Node::nsDOM3Node()
{
  /* member initializers and constructor code */
}

nsDOM3Node::~nsDOM3Node()
{
  /* destructor code */
}

/* readonly attribute DOMString baseURI; */
NS_IMETHODIMP nsDOM3Node::GetBaseURI(nsAString & aBaseURI)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* unsigned short compareDocumentPosition (in nsIDOMNode other)  raises (DOMException); */
NS_IMETHODIMP nsDOM3Node::CompareDocumentPosition(nsIDOMNode *other, PRUint16 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString textContent; */
NS_IMETHODIMP nsDOM3Node::GetTextContent(nsAString & aTextContent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOM3Node::SetTextContent(const nsAString & aTextContent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isSameNode (in nsIDOMNode other); */
NS_IMETHODIMP nsDOM3Node::IsSameNode(nsIDOMNode *other, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* DOMString lookupPrefix (in DOMString namespaceURI); */
NS_IMETHODIMP nsDOM3Node::LookupPrefix(const nsAString & namespaceURI, nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isDefaultNamespace (in DOMString namespaceURI); */
NS_IMETHODIMP nsDOM3Node::IsDefaultNamespace(const nsAString & namespaceURI, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* DOMString lookupNamespaceURI (in DOMString prefix); */
NS_IMETHODIMP nsDOM3Node::LookupNamespaceURI(const nsAString & prefix, nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isEqualNode (in nsIDOMNode arg); */
NS_IMETHODIMP nsDOM3Node::IsEqualNode(nsIDOMNode *arg, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsISupports getFeature (in DOMString feature, in DOMString version); */
NS_IMETHODIMP nsDOM3Node::GetFeature(const nsAString & feature, const nsAString & version, nsISupports **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIVariant setUserData (in DOMString key, in nsIVariant data, in nsIDOMUserDataHandler handler); */
NS_IMETHODIMP nsDOM3Node::SetUserData(const nsAString & key, nsIVariant *data, nsIDOMUserDataHandler *handler, nsIVariant **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIVariant getUserData (in DOMString key); */
NS_IMETHODIMP nsDOM3Node::GetUserData(const nsAString & key, nsIVariant **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOM3Node_h__ */
