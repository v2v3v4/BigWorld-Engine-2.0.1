/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/core/nsIDOMNSDocument.idl
 */

#ifndef __gen_nsIDOMNSDocument_h__
#define __gen_nsIDOMNSDocument_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIBoxObject; /* forward declaration */

class nsIDOMLocation; /* forward declaration */


/* starting interface:    nsIDOMNSDocument */
#define NS_IDOMNSDOCUMENT_IID_STR "a6cf90cd-15b3-11d2-932e-00805f8add32"

#define NS_IDOMNSDOCUMENT_IID \
  {0xa6cf90cd, 0x15b3, 0x11d2, \
    { 0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32 }}

class NS_NO_VTABLE nsIDOMNSDocument : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMNSDOCUMENT_IID)

  /* readonly attribute DOMString characterSet; */
  NS_IMETHOD GetCharacterSet(nsAString & aCharacterSet) = 0;

  /* attribute DOMString dir; */
  NS_IMETHOD GetDir(nsAString & aDir) = 0;
  NS_IMETHOD SetDir(const nsAString & aDir) = 0;

  /* readonly attribute nsIDOMLocation location; */
  NS_IMETHOD GetLocation(nsIDOMLocation * *aLocation) = 0;

  /* attribute DOMString title; */
  NS_IMETHOD GetTitle(nsAString & aTitle) = 0;
  NS_IMETHOD SetTitle(const nsAString & aTitle) = 0;

  /* readonly attribute DOMString contentType; */
  NS_IMETHOD GetContentType(nsAString & aContentType) = 0;

  /* readonly attribute DOMString lastModified; */
  NS_IMETHOD GetLastModified(nsAString & aLastModified) = 0;

  /* readonly attribute DOMString referrer; */
  NS_IMETHOD GetReferrer(nsAString & aReferrer) = 0;

  /* nsIBoxObject getBoxObjectFor (in nsIDOMElement elt); */
  NS_IMETHOD GetBoxObjectFor(nsIDOMElement *elt, nsIBoxObject **_retval) = 0;

  /* void setBoxObjectFor (in nsIDOMElement elt, in nsIBoxObject boxObject); */
  NS_IMETHOD SetBoxObjectFor(nsIDOMElement *elt, nsIBoxObject *boxObject) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMNSDOCUMENT \
  NS_IMETHOD GetCharacterSet(nsAString & aCharacterSet); \
  NS_IMETHOD GetDir(nsAString & aDir); \
  NS_IMETHOD SetDir(const nsAString & aDir); \
  NS_IMETHOD GetLocation(nsIDOMLocation * *aLocation); \
  NS_IMETHOD GetTitle(nsAString & aTitle); \
  NS_IMETHOD SetTitle(const nsAString & aTitle); \
  NS_IMETHOD GetContentType(nsAString & aContentType); \
  NS_IMETHOD GetLastModified(nsAString & aLastModified); \
  NS_IMETHOD GetReferrer(nsAString & aReferrer); \
  NS_IMETHOD GetBoxObjectFor(nsIDOMElement *elt, nsIBoxObject **_retval); \
  NS_IMETHOD SetBoxObjectFor(nsIDOMElement *elt, nsIBoxObject *boxObject); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMNSDOCUMENT(_to) \
  NS_IMETHOD GetCharacterSet(nsAString & aCharacterSet) { return _to GetCharacterSet(aCharacterSet); } \
  NS_IMETHOD GetDir(nsAString & aDir) { return _to GetDir(aDir); } \
  NS_IMETHOD SetDir(const nsAString & aDir) { return _to SetDir(aDir); } \
  NS_IMETHOD GetLocation(nsIDOMLocation * *aLocation) { return _to GetLocation(aLocation); } \
  NS_IMETHOD GetTitle(nsAString & aTitle) { return _to GetTitle(aTitle); } \
  NS_IMETHOD SetTitle(const nsAString & aTitle) { return _to SetTitle(aTitle); } \
  NS_IMETHOD GetContentType(nsAString & aContentType) { return _to GetContentType(aContentType); } \
  NS_IMETHOD GetLastModified(nsAString & aLastModified) { return _to GetLastModified(aLastModified); } \
  NS_IMETHOD GetReferrer(nsAString & aReferrer) { return _to GetReferrer(aReferrer); } \
  NS_IMETHOD GetBoxObjectFor(nsIDOMElement *elt, nsIBoxObject **_retval) { return _to GetBoxObjectFor(elt, _retval); } \
  NS_IMETHOD SetBoxObjectFor(nsIDOMElement *elt, nsIBoxObject *boxObject) { return _to SetBoxObjectFor(elt, boxObject); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMNSDOCUMENT(_to) \
  NS_IMETHOD GetCharacterSet(nsAString & aCharacterSet) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCharacterSet(aCharacterSet); } \
  NS_IMETHOD GetDir(nsAString & aDir) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDir(aDir); } \
  NS_IMETHOD SetDir(const nsAString & aDir) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDir(aDir); } \
  NS_IMETHOD GetLocation(nsIDOMLocation * *aLocation) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLocation(aLocation); } \
  NS_IMETHOD GetTitle(nsAString & aTitle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTitle(aTitle); } \
  NS_IMETHOD SetTitle(const nsAString & aTitle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTitle(aTitle); } \
  NS_IMETHOD GetContentType(nsAString & aContentType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetContentType(aContentType); } \
  NS_IMETHOD GetLastModified(nsAString & aLastModified) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLastModified(aLastModified); } \
  NS_IMETHOD GetReferrer(nsAString & aReferrer) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetReferrer(aReferrer); } \
  NS_IMETHOD GetBoxObjectFor(nsIDOMElement *elt, nsIBoxObject **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBoxObjectFor(elt, _retval); } \
  NS_IMETHOD SetBoxObjectFor(nsIDOMElement *elt, nsIBoxObject *boxObject) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBoxObjectFor(elt, boxObject); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMNSDocument : public nsIDOMNSDocument
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMNSDOCUMENT

  nsDOMNSDocument();

private:
  ~nsDOMNSDocument();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMNSDocument, nsIDOMNSDocument)

nsDOMNSDocument::nsDOMNSDocument()
{
  /* member initializers and constructor code */
}

nsDOMNSDocument::~nsDOMNSDocument()
{
  /* destructor code */
}

/* readonly attribute DOMString characterSet; */
NS_IMETHODIMP nsDOMNSDocument::GetCharacterSet(nsAString & aCharacterSet)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString dir; */
NS_IMETHODIMP nsDOMNSDocument::GetDir(nsAString & aDir)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMNSDocument::SetDir(const nsAString & aDir)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMLocation location; */
NS_IMETHODIMP nsDOMNSDocument::GetLocation(nsIDOMLocation * *aLocation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString title; */
NS_IMETHODIMP nsDOMNSDocument::GetTitle(nsAString & aTitle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMNSDocument::SetTitle(const nsAString & aTitle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString contentType; */
NS_IMETHODIMP nsDOMNSDocument::GetContentType(nsAString & aContentType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString lastModified; */
NS_IMETHODIMP nsDOMNSDocument::GetLastModified(nsAString & aLastModified)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString referrer; */
NS_IMETHODIMP nsDOMNSDocument::GetReferrer(nsAString & aReferrer)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIBoxObject getBoxObjectFor (in nsIDOMElement elt); */
NS_IMETHODIMP nsDOMNSDocument::GetBoxObjectFor(nsIDOMElement *elt, nsIBoxObject **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setBoxObjectFor (in nsIDOMElement elt, in nsIBoxObject boxObject); */
NS_IMETHODIMP nsDOMNSDocument::SetBoxObjectFor(nsIDOMElement *elt, nsIBoxObject *boxObject)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMNSDocument_h__ */
