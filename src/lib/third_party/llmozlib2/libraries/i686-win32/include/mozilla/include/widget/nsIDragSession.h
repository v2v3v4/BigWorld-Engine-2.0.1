/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/widget/public/nsIDragSession.idl
 */

#ifndef __gen_nsIDragSession_h__
#define __gen_nsIDragSession_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsISupportsArray_h__
#include "nsISupportsArray.h"
#endif

#ifndef __gen_nsITransferable_h__
#include "nsITransferable.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "nsSize.h"
class nsIDOMDocument; /* forward declaration */

class nsIDOMNode; /* forward declaration */


/* starting interface:    nsIDragSession */
#define NS_IDRAGSESSION_IID_STR "cba22c53-fcce-11d2-96d4-0060b0fb9956"

#define NS_IDRAGSESSION_IID \
  {0xcba22c53, 0xfcce, 0x11d2, \
    { 0x96, 0xd4, 0x00, 0x60, 0xb0, 0xfb, 0x99, 0x56 }}

class NS_NO_VTABLE nsIDragSession : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDRAGSESSION_IID)

  /**
    * Set the current state of the drag whether it can be dropped or not.
    * usually the target "frame" sets this so the native system can render the correct feedback
    */
  /* attribute boolean canDrop; */
  NS_IMETHOD GetCanDrop(PRBool *aCanDrop) = 0;
  NS_IMETHOD SetCanDrop(PRBool aCanDrop) = 0;

  /**
    * Sets the action (copy, move, link, et.c) for the current drag 
    */
  /* attribute unsigned long dragAction; */
  NS_IMETHOD GetDragAction(PRUint32 *aDragAction) = 0;
  NS_IMETHOD SetDragAction(PRUint32 aDragAction) = 0;

  /**
    * Sets the current width and height if the drag target area. 
    * It will contain the current size of the Frame that the drag is currently in
    */
  /* [noscript] attribute nsSize targetSize; */
  NS_IMETHOD GetTargetSize(nsSize *aTargetSize) = 0;
  NS_IMETHOD SetTargetSize(nsSize aTargetSize) = 0;

  /**
    * Get the number items that were dropped
    */
  /* readonly attribute unsigned long numDropItems; */
  NS_IMETHOD GetNumDropItems(PRUint32 *aNumDropItems) = 0;

  /**
    * The document where the drag was started, which will be null if the
    * drag originated outside the application. Useful for determining if a drop
    * originated in the same document.
    */
  /* readonly attribute nsIDOMDocument sourceDocument; */
  NS_IMETHOD GetSourceDocument(nsIDOMDocument * *aSourceDocument) = 0;

  /**
    * The dom node that was originally dragged to start the session, which will be null if the
    * drag originated outside the application.
    */
  /* readonly attribute nsIDOMNode sourceNode; */
  NS_IMETHOD GetSourceNode(nsIDOMNode * *aSourceNode) = 0;

  /**
    * Get data from a Drag&Drop. Can be called while the drag is in process
    * or after the drop has completed.  
    *
    * @param  aTransferable the transferable for the data to be put into
    * @param  aItemIndex which of multiple drag items, zero-based
    */
  /* void getData (in nsITransferable aTransferable, in unsigned long aItemIndex); */
  NS_IMETHOD GetData(nsITransferable *aTransferable, PRUint32 aItemIndex) = 0;

  /**
    * Check to set if ant of the native data on the clipboard matches this data flavor
    *
    * @result NS_OK if if the data flavor is supported and, NS_ERROR_FAILURE is it is not
    */
  /* boolean isDataFlavorSupported (in string aDataFlavor); */
  NS_IMETHOD IsDataFlavorSupported(const char *aDataFlavor, PRBool *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDRAGSESSION \
  NS_IMETHOD GetCanDrop(PRBool *aCanDrop); \
  NS_IMETHOD SetCanDrop(PRBool aCanDrop); \
  NS_IMETHOD GetDragAction(PRUint32 *aDragAction); \
  NS_IMETHOD SetDragAction(PRUint32 aDragAction); \
  NS_IMETHOD GetTargetSize(nsSize *aTargetSize); \
  NS_IMETHOD SetTargetSize(nsSize aTargetSize); \
  NS_IMETHOD GetNumDropItems(PRUint32 *aNumDropItems); \
  NS_IMETHOD GetSourceDocument(nsIDOMDocument * *aSourceDocument); \
  NS_IMETHOD GetSourceNode(nsIDOMNode * *aSourceNode); \
  NS_IMETHOD GetData(nsITransferable *aTransferable, PRUint32 aItemIndex); \
  NS_IMETHOD IsDataFlavorSupported(const char *aDataFlavor, PRBool *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDRAGSESSION(_to) \
  NS_IMETHOD GetCanDrop(PRBool *aCanDrop) { return _to GetCanDrop(aCanDrop); } \
  NS_IMETHOD SetCanDrop(PRBool aCanDrop) { return _to SetCanDrop(aCanDrop); } \
  NS_IMETHOD GetDragAction(PRUint32 *aDragAction) { return _to GetDragAction(aDragAction); } \
  NS_IMETHOD SetDragAction(PRUint32 aDragAction) { return _to SetDragAction(aDragAction); } \
  NS_IMETHOD GetTargetSize(nsSize *aTargetSize) { return _to GetTargetSize(aTargetSize); } \
  NS_IMETHOD SetTargetSize(nsSize aTargetSize) { return _to SetTargetSize(aTargetSize); } \
  NS_IMETHOD GetNumDropItems(PRUint32 *aNumDropItems) { return _to GetNumDropItems(aNumDropItems); } \
  NS_IMETHOD GetSourceDocument(nsIDOMDocument * *aSourceDocument) { return _to GetSourceDocument(aSourceDocument); } \
  NS_IMETHOD GetSourceNode(nsIDOMNode * *aSourceNode) { return _to GetSourceNode(aSourceNode); } \
  NS_IMETHOD GetData(nsITransferable *aTransferable, PRUint32 aItemIndex) { return _to GetData(aTransferable, aItemIndex); } \
  NS_IMETHOD IsDataFlavorSupported(const char *aDataFlavor, PRBool *_retval) { return _to IsDataFlavorSupported(aDataFlavor, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDRAGSESSION(_to) \
  NS_IMETHOD GetCanDrop(PRBool *aCanDrop) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCanDrop(aCanDrop); } \
  NS_IMETHOD SetCanDrop(PRBool aCanDrop) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCanDrop(aCanDrop); } \
  NS_IMETHOD GetDragAction(PRUint32 *aDragAction) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDragAction(aDragAction); } \
  NS_IMETHOD SetDragAction(PRUint32 aDragAction) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDragAction(aDragAction); } \
  NS_IMETHOD GetTargetSize(nsSize *aTargetSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTargetSize(aTargetSize); } \
  NS_IMETHOD SetTargetSize(nsSize aTargetSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTargetSize(aTargetSize); } \
  NS_IMETHOD GetNumDropItems(PRUint32 *aNumDropItems) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNumDropItems(aNumDropItems); } \
  NS_IMETHOD GetSourceDocument(nsIDOMDocument * *aSourceDocument) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSourceDocument(aSourceDocument); } \
  NS_IMETHOD GetSourceNode(nsIDOMNode * *aSourceNode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSourceNode(aSourceNode); } \
  NS_IMETHOD GetData(nsITransferable *aTransferable, PRUint32 aItemIndex) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetData(aTransferable, aItemIndex); } \
  NS_IMETHOD IsDataFlavorSupported(const char *aDataFlavor, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->IsDataFlavorSupported(aDataFlavor, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDragSession : public nsIDragSession
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDRAGSESSION

  nsDragSession();

private:
  ~nsDragSession();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDragSession, nsIDragSession)

nsDragSession::nsDragSession()
{
  /* member initializers and constructor code */
}

nsDragSession::~nsDragSession()
{
  /* destructor code */
}

/* attribute boolean canDrop; */
NS_IMETHODIMP nsDragSession::GetCanDrop(PRBool *aCanDrop)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDragSession::SetCanDrop(PRBool aCanDrop)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute unsigned long dragAction; */
NS_IMETHODIMP nsDragSession::GetDragAction(PRUint32 *aDragAction)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDragSession::SetDragAction(PRUint32 aDragAction)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] attribute nsSize targetSize; */
NS_IMETHODIMP nsDragSession::GetTargetSize(nsSize *aTargetSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDragSession::SetTargetSize(nsSize aTargetSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long numDropItems; */
NS_IMETHODIMP nsDragSession::GetNumDropItems(PRUint32 *aNumDropItems)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMDocument sourceDocument; */
NS_IMETHODIMP nsDragSession::GetSourceDocument(nsIDOMDocument * *aSourceDocument)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMNode sourceNode; */
NS_IMETHODIMP nsDragSession::GetSourceNode(nsIDOMNode * *aSourceNode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getData (in nsITransferable aTransferable, in unsigned long aItemIndex); */
NS_IMETHODIMP nsDragSession::GetData(nsITransferable *aTransferable, PRUint32 aItemIndex)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isDataFlavorSupported (in string aDataFlavor); */
NS_IMETHODIMP nsDragSession::IsDataFlavorSupported(const char *aDataFlavor, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDragSession_h__ */
