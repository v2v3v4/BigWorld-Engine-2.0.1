/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/svg/nsIDOMGetSVGDocument.idl
 */

#ifndef __gen_nsIDOMGetSVGDocument_h__
#define __gen_nsIDOMGetSVGDocument_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMSVGDocument; /* forward declaration */


/* starting interface:    nsIDOMGetSVGDocument */
#define NS_IDOMGETSVGDOCUMENT_IID_STR "0401f299-685b-43a1-82b4-ce1a0011598c"

#define NS_IDOMGETSVGDOCUMENT_IID \
  {0x0401f299, 0x685b, 0x43a1, \
    { 0x82, 0xb4, 0xce, 0x1a, 0x00, 0x11, 0x59, 0x8c }}

class NS_NO_VTABLE nsIDOMGetSVGDocument : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMGETSVGDOCUMENT_IID)

  /* nsIDOMSVGDocument getSVGDocument (); */
  NS_IMETHOD GetSVGDocument(nsIDOMSVGDocument **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMGETSVGDOCUMENT \
  NS_IMETHOD GetSVGDocument(nsIDOMSVGDocument **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMGETSVGDOCUMENT(_to) \
  NS_IMETHOD GetSVGDocument(nsIDOMSVGDocument **_retval) { return _to GetSVGDocument(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMGETSVGDOCUMENT(_to) \
  NS_IMETHOD GetSVGDocument(nsIDOMSVGDocument **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSVGDocument(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMGetSVGDocument : public nsIDOMGetSVGDocument
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMGETSVGDOCUMENT

  nsDOMGetSVGDocument();

private:
  ~nsDOMGetSVGDocument();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMGetSVGDocument, nsIDOMGetSVGDocument)

nsDOMGetSVGDocument::nsDOMGetSVGDocument()
{
  /* member initializers and constructor code */
}

nsDOMGetSVGDocument::~nsDOMGetSVGDocument()
{
  /* destructor code */
}

/* nsIDOMSVGDocument getSVGDocument (); */
NS_IMETHODIMP nsDOMGetSVGDocument::GetSVGDocument(nsIDOMSVGDocument **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMGetSVGDocument_h__ */
