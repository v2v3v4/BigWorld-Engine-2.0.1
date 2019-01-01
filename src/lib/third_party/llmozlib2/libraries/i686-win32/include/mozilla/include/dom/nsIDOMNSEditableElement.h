/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/core/nsIDOMNSEditableElement.idl
 */

#ifndef __gen_nsIDOMNSEditableElement_h__
#define __gen_nsIDOMNSEditableElement_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIEditor; /* forward declaration */


/* starting interface:    nsIDOMNSEditableElement */
#define NS_IDOMNSEDITABLEELEMENT_IID_STR "c4a71f8e-82ba-49d7-94f9-beb359361072"

#define NS_IDOMNSEDITABLEELEMENT_IID \
  {0xc4a71f8e, 0x82ba, 0x49d7, \
    { 0x94, 0xf9, 0xbe, 0xb3, 0x59, 0x36, 0x10, 0x72 }}

/**
 * This interface is implemented by elements which have inner editable content,
 * such as HTML input and textarea. 
*/
class NS_NO_VTABLE nsIDOMNSEditableElement : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMNSEDITABLEELEMENT_IID)

  /* readonly attribute nsIEditor editor; */
  NS_IMETHOD GetEditor(nsIEditor * *aEditor) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMNSEDITABLEELEMENT \
  NS_IMETHOD GetEditor(nsIEditor * *aEditor); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMNSEDITABLEELEMENT(_to) \
  NS_IMETHOD GetEditor(nsIEditor * *aEditor) { return _to GetEditor(aEditor); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMNSEDITABLEELEMENT(_to) \
  NS_IMETHOD GetEditor(nsIEditor * *aEditor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetEditor(aEditor); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMNSEditableElement : public nsIDOMNSEditableElement
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMNSEDITABLEELEMENT

  nsDOMNSEditableElement();

private:
  ~nsDOMNSEditableElement();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMNSEditableElement, nsIDOMNSEditableElement)

nsDOMNSEditableElement::nsDOMNSEditableElement()
{
  /* member initializers and constructor code */
}

nsDOMNSEditableElement::~nsDOMNSEditableElement()
{
  /* destructor code */
}

/* readonly attribute nsIEditor editor; */
NS_IMETHODIMP nsDOMNSEditableElement::GetEditor(nsIEditor * *aEditor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMNSEditableElement_h__ */
