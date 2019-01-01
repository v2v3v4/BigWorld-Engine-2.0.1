/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/widget/public/nsIDragService.idl
 */

#ifndef __gen_nsIDragService_h__
#define __gen_nsIDragService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsISupportsArray_h__
#include "nsISupportsArray.h"
#endif

#ifndef __gen_nsIDragSession_h__
#include "nsIDragSession.h"
#endif

#ifndef __gen_nsIScriptableRegion_h__
#include "nsIScriptableRegion.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMNode; /* forward declaration */


/* starting interface:    nsIDragService */
#define NS_IDRAGSERVICE_IID_STR "8b5314bb-db01-11d2-96ce-0060b0fb9956"

#define NS_IDRAGSERVICE_IID \
  {0x8b5314bb, 0xdb01, 0x11d2, \
    { 0x96, 0xce, 0x00, 0x60, 0xb0, 0xfb, 0x99, 0x56 }}

class NS_NO_VTABLE nsIDragService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDRAGSERVICE_IID)

  enum { DRAGDROP_ACTION_NONE = 0 };

  enum { DRAGDROP_ACTION_COPY = 1 };

  enum { DRAGDROP_ACTION_MOVE = 2 };

  enum { DRAGDROP_ACTION_LINK = 4 };

  /**
    * Starts a modal drag session with an array of transaferables 
    *
    * @param  aTransferables - an array of transferables to be dragged
    * @param  aRegion - a region containing rectangles for cursor feedback, 
    *            in window coordinates.
    * @param  aActionType - specified which of copy/move/link are allowed
    */
  /* void invokeDragSession (in nsIDOMNode aDOMNode, in nsISupportsArray aTransferables, in nsIScriptableRegion aRegion, in unsigned long aActionType); */
  NS_IMETHOD InvokeDragSession(nsIDOMNode *aDOMNode, nsISupportsArray *aTransferables, nsIScriptableRegion *aRegion, PRUint32 aActionType) = 0;

  /**
    * Returns the current Drag Session  
    */
  /* nsIDragSession getCurrentSession (); */
  NS_IMETHOD GetCurrentSession(nsIDragSession **_retval) = 0;

  /**
    * Tells the Drag Service to start a drag session. This is called when
    * an external drag occurs
    */
  /* void startDragSession (); */
  NS_IMETHOD StartDragSession(void) = 0;

  /**
    * Tells the Drag Service to end a drag session. This is called when
    * an external drag occurs
    */
  /* void endDragSession (); */
  NS_IMETHOD EndDragSession(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDRAGSERVICE \
  NS_IMETHOD InvokeDragSession(nsIDOMNode *aDOMNode, nsISupportsArray *aTransferables, nsIScriptableRegion *aRegion, PRUint32 aActionType); \
  NS_IMETHOD GetCurrentSession(nsIDragSession **_retval); \
  NS_IMETHOD StartDragSession(void); \
  NS_IMETHOD EndDragSession(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDRAGSERVICE(_to) \
  NS_IMETHOD InvokeDragSession(nsIDOMNode *aDOMNode, nsISupportsArray *aTransferables, nsIScriptableRegion *aRegion, PRUint32 aActionType) { return _to InvokeDragSession(aDOMNode, aTransferables, aRegion, aActionType); } \
  NS_IMETHOD GetCurrentSession(nsIDragSession **_retval) { return _to GetCurrentSession(_retval); } \
  NS_IMETHOD StartDragSession(void) { return _to StartDragSession(); } \
  NS_IMETHOD EndDragSession(void) { return _to EndDragSession(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDRAGSERVICE(_to) \
  NS_IMETHOD InvokeDragSession(nsIDOMNode *aDOMNode, nsISupportsArray *aTransferables, nsIScriptableRegion *aRegion, PRUint32 aActionType) { return !_to ? NS_ERROR_NULL_POINTER : _to->InvokeDragSession(aDOMNode, aTransferables, aRegion, aActionType); } \
  NS_IMETHOD GetCurrentSession(nsIDragSession **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCurrentSession(_retval); } \
  NS_IMETHOD StartDragSession(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->StartDragSession(); } \
  NS_IMETHOD EndDragSession(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->EndDragSession(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDragService : public nsIDragService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDRAGSERVICE

  nsDragService();

private:
  ~nsDragService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDragService, nsIDragService)

nsDragService::nsDragService()
{
  /* member initializers and constructor code */
}

nsDragService::~nsDragService()
{
  /* destructor code */
}

/* void invokeDragSession (in nsIDOMNode aDOMNode, in nsISupportsArray aTransferables, in nsIScriptableRegion aRegion, in unsigned long aActionType); */
NS_IMETHODIMP nsDragService::InvokeDragSession(nsIDOMNode *aDOMNode, nsISupportsArray *aTransferables, nsIScriptableRegion *aRegion, PRUint32 aActionType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDragSession getCurrentSession (); */
NS_IMETHODIMP nsDragService::GetCurrentSession(nsIDragSession **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void startDragSession (); */
NS_IMETHODIMP nsDragService::StartDragSession()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void endDragSession (); */
NS_IMETHODIMP nsDragService::EndDragSession()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDragService_1_8_BRANCH */
#define NS_IDRAGSERVICE_1_8_BRANCH_IID_STR "b6ba2c09-4a12-40d6-ad7e-aa4190e78403"

#define NS_IDRAGSERVICE_1_8_BRANCH_IID \
  {0xb6ba2c09, 0x4a12, 0x40d6, \
    { 0xad, 0x7e, 0xaa, 0x41, 0x90, 0xe7, 0x84, 0x03 }}

class NS_NO_VTABLE nsIDragService_1_8_BRANCH : public nsIDragService {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDRAGSERVICE_1_8_BRANCH_IID)

  /**
   * Increase/decrease dragging suppress level by one.
   * If level is greater than one, dragging is disabled.
   */
  /* void suppress (); */
  NS_IMETHOD Suppress(void) = 0;

  /* void unsuppress (); */
  NS_IMETHOD Unsuppress(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDRAGSERVICE_1_8_BRANCH \
  NS_IMETHOD Suppress(void); \
  NS_IMETHOD Unsuppress(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDRAGSERVICE_1_8_BRANCH(_to) \
  NS_IMETHOD Suppress(void) { return _to Suppress(); } \
  NS_IMETHOD Unsuppress(void) { return _to Unsuppress(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDRAGSERVICE_1_8_BRANCH(_to) \
  NS_IMETHOD Suppress(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Suppress(); } \
  NS_IMETHOD Unsuppress(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Unsuppress(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDragService_1_8_BRANCH : public nsIDragService_1_8_BRANCH
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDRAGSERVICE_1_8_BRANCH

  nsDragService_1_8_BRANCH();

private:
  ~nsDragService_1_8_BRANCH();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDragService_1_8_BRANCH, nsIDragService_1_8_BRANCH)

nsDragService_1_8_BRANCH::nsDragService_1_8_BRANCH()
{
  /* member initializers and constructor code */
}

nsDragService_1_8_BRANCH::~nsDragService_1_8_BRANCH()
{
  /* destructor code */
}

/* void suppress (); */
NS_IMETHODIMP nsDragService_1_8_BRANCH::Suppress()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unsuppress (); */
NS_IMETHODIMP nsDragService_1_8_BRANCH::Unsuppress()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDragService_h__ */
