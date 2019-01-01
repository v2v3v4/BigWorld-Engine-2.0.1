/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/xtf/public/nsIXTFElementWrapper.idl
 */

#ifndef __gen_nsIXTFElementWrapper_h__
#define __gen_nsIXTFElementWrapper_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIAtom; /* forward declaration */

class nsIDOMElement; /* forward declaration */

class nsIDOMDocument; /* forward declaration */


/* starting interface:    nsIXTFElementWrapper */
#define NS_IXTFELEMENTWRAPPER_IID_STR "444d0276-3302-4d35-a74e-25c4e9c483c9"

#define NS_IXTFELEMENTWRAPPER_IID \
  {0x444d0276, 0x3302, 0x4d35, \
    { 0xa7, 0x4e, 0x25, 0xc4, 0xe9, 0xc4, 0x83, 0xc9 }}

class NS_NO_VTABLE nsIXTFElementWrapper : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXTFELEMENTWRAPPER_IID)

  /* readonly attribute nsIDOMElement elementNode; */
  NS_IMETHOD GetElementNode(nsIDOMElement * *aElementNode) = 0;

  /* readonly attribute nsIDOMElement documentFrameElement; */
  NS_IMETHOD GetDocumentFrameElement(nsIDOMElement * *aDocumentFrameElement) = 0;

  /**
   * Events can be unmasked by setting the corresponding bit as given
   * by the NOTIFY_* constants in nsIXTFElement and nsIXTFVisual:
   */
  /* attribute unsigned long notificationMask; */
  NS_IMETHOD GetNotificationMask(PRUint32 *aNotificationMask) = 0;
  NS_IMETHOD SetNotificationMask(PRUint32 aNotificationMask) = 0;

  /**
   * Sets the intrinsic state for the element.
   * @see nsIContent::IntrinsicState().
   */
  /* void setIntrinsicState (in long newState); */
  NS_IMETHOD SetIntrinsicState(PRInt32 newState) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXTFELEMENTWRAPPER \
  NS_IMETHOD GetElementNode(nsIDOMElement * *aElementNode); \
  NS_IMETHOD GetDocumentFrameElement(nsIDOMElement * *aDocumentFrameElement); \
  NS_IMETHOD GetNotificationMask(PRUint32 *aNotificationMask); \
  NS_IMETHOD SetNotificationMask(PRUint32 aNotificationMask); \
  NS_IMETHOD SetIntrinsicState(PRInt32 newState); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXTFELEMENTWRAPPER(_to) \
  NS_IMETHOD GetElementNode(nsIDOMElement * *aElementNode) { return _to GetElementNode(aElementNode); } \
  NS_IMETHOD GetDocumentFrameElement(nsIDOMElement * *aDocumentFrameElement) { return _to GetDocumentFrameElement(aDocumentFrameElement); } \
  NS_IMETHOD GetNotificationMask(PRUint32 *aNotificationMask) { return _to GetNotificationMask(aNotificationMask); } \
  NS_IMETHOD SetNotificationMask(PRUint32 aNotificationMask) { return _to SetNotificationMask(aNotificationMask); } \
  NS_IMETHOD SetIntrinsicState(PRInt32 newState) { return _to SetIntrinsicState(newState); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXTFELEMENTWRAPPER(_to) \
  NS_IMETHOD GetElementNode(nsIDOMElement * *aElementNode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetElementNode(aElementNode); } \
  NS_IMETHOD GetDocumentFrameElement(nsIDOMElement * *aDocumentFrameElement) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDocumentFrameElement(aDocumentFrameElement); } \
  NS_IMETHOD GetNotificationMask(PRUint32 *aNotificationMask) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNotificationMask(aNotificationMask); } \
  NS_IMETHOD SetNotificationMask(PRUint32 aNotificationMask) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetNotificationMask(aNotificationMask); } \
  NS_IMETHOD SetIntrinsicState(PRInt32 newState) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetIntrinsicState(newState); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXTFElementWrapper : public nsIXTFElementWrapper
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFELEMENTWRAPPER

  nsXTFElementWrapper();

private:
  ~nsXTFElementWrapper();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXTFElementWrapper, nsIXTFElementWrapper)

nsXTFElementWrapper::nsXTFElementWrapper()
{
  /* member initializers and constructor code */
}

nsXTFElementWrapper::~nsXTFElementWrapper()
{
  /* destructor code */
}

/* readonly attribute nsIDOMElement elementNode; */
NS_IMETHODIMP nsXTFElementWrapper::GetElementNode(nsIDOMElement * *aElementNode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMElement documentFrameElement; */
NS_IMETHODIMP nsXTFElementWrapper::GetDocumentFrameElement(nsIDOMElement * *aDocumentFrameElement)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute unsigned long notificationMask; */
NS_IMETHODIMP nsXTFElementWrapper::GetNotificationMask(PRUint32 *aNotificationMask)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsXTFElementWrapper::SetNotificationMask(PRUint32 aNotificationMask)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setIntrinsicState (in long newState); */
NS_IMETHODIMP nsXTFElementWrapper::SetIntrinsicState(PRInt32 newState)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXTFElementWrapper_h__ */
