/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/xtf/public/nsIXTFVisual.idl
 */

#ifndef __gen_nsIXTFVisual_h__
#define __gen_nsIXTFVisual_h__


#ifndef __gen_nsIXTFElement_h__
#include "nsIXTFElement.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMElement; /* forward declaration */


/* starting interface:    nsIXTFVisual */
#define NS_IXTFVISUAL_IID_STR "2ee5520b-6593-43c1-b660-4885939a6b68"

#define NS_IXTFVISUAL_IID \
  {0x2ee5520b, 0x6593, 0x43c1, \
    { 0xb6, 0x60, 0x48, 0x85, 0x93, 0x9a, 0x6b, 0x68 }}

class NS_NO_VTABLE nsIXTFVisual : public nsIXTFElement {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXTFVISUAL_IID)

  /* readonly attribute nsIDOMElement visualContent; */
  NS_IMETHOD GetVisualContent(nsIDOMElement * *aVisualContent) = 0;

  /* readonly attribute nsIDOMElement insertionPoint; */
  NS_IMETHOD GetInsertionPoint(nsIDOMElement * *aInsertionPoint) = 0;

  /* readonly attribute boolean applyDocumentStyleSheets; */
  NS_IMETHOD GetApplyDocumentStyleSheets(PRBool *aApplyDocumentStyleSheets) = 0;

  enum { NOTIFY_DID_LAYOUT = 131072U };

  /* void didLayout (); */
  NS_IMETHOD DidLayout(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXTFVISUAL \
  NS_IMETHOD GetVisualContent(nsIDOMElement * *aVisualContent); \
  NS_IMETHOD GetInsertionPoint(nsIDOMElement * *aInsertionPoint); \
  NS_IMETHOD GetApplyDocumentStyleSheets(PRBool *aApplyDocumentStyleSheets); \
  NS_IMETHOD DidLayout(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXTFVISUAL(_to) \
  NS_IMETHOD GetVisualContent(nsIDOMElement * *aVisualContent) { return _to GetVisualContent(aVisualContent); } \
  NS_IMETHOD GetInsertionPoint(nsIDOMElement * *aInsertionPoint) { return _to GetInsertionPoint(aInsertionPoint); } \
  NS_IMETHOD GetApplyDocumentStyleSheets(PRBool *aApplyDocumentStyleSheets) { return _to GetApplyDocumentStyleSheets(aApplyDocumentStyleSheets); } \
  NS_IMETHOD DidLayout(void) { return _to DidLayout(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXTFVISUAL(_to) \
  NS_IMETHOD GetVisualContent(nsIDOMElement * *aVisualContent) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetVisualContent(aVisualContent); } \
  NS_IMETHOD GetInsertionPoint(nsIDOMElement * *aInsertionPoint) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetInsertionPoint(aInsertionPoint); } \
  NS_IMETHOD GetApplyDocumentStyleSheets(PRBool *aApplyDocumentStyleSheets) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetApplyDocumentStyleSheets(aApplyDocumentStyleSheets); } \
  NS_IMETHOD DidLayout(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->DidLayout(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXTFVisual : public nsIXTFVisual
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFVISUAL

  nsXTFVisual();

private:
  ~nsXTFVisual();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXTFVisual, nsIXTFVisual)

nsXTFVisual::nsXTFVisual()
{
  /* member initializers and constructor code */
}

nsXTFVisual::~nsXTFVisual()
{
  /* destructor code */
}

/* readonly attribute nsIDOMElement visualContent; */
NS_IMETHODIMP nsXTFVisual::GetVisualContent(nsIDOMElement * *aVisualContent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMElement insertionPoint; */
NS_IMETHODIMP nsXTFVisual::GetInsertionPoint(nsIDOMElement * *aInsertionPoint)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean applyDocumentStyleSheets; */
NS_IMETHODIMP nsXTFVisual::GetApplyDocumentStyleSheets(PRBool *aApplyDocumentStyleSheets)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void didLayout (); */
NS_IMETHODIMP nsXTFVisual::DidLayout()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXTFVisual_h__ */
