/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/threads/nsIEventTarget.idl
 */

#ifndef __gen_nsIEventTarget_h__
#define __gen_nsIEventTarget_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "plevent.h"

/* starting interface:    nsIEventTarget */
#define NS_IEVENTTARGET_IID_STR "ea99ad5b-cc67-4efb-97c9-2ef620a59f2a"

#define NS_IEVENTTARGET_IID \
  {0xea99ad5b, 0xcc67, 0x4efb, \
    { 0x97, 0xc9, 0x2e, 0xf6, 0x20, 0xa5, 0x9f, 0x2a }}

/**
 * nsIEventTarget
 *
 * This interface is used to dispatch events to a particular thread.  In many
 * cases the event target also supports nsIEventQueue.
 */
class NS_NO_VTABLE nsIEventTarget : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IEVENTTARGET_IID)

  /**
     * Method for posting an asynchronous event to the event target.  If this
     * method succeeds, then the event will be dispatched on the target thread.
     *
     * @param aEvent
     *        The event to dispatched.
     */
  /* [noscript] void postEvent (in PLEventPtr aEvent); */
  NS_IMETHOD PostEvent(PLEvent * aEvent) = 0;

  /**
     * This method returns true if the event target is the current thread.
     */
  /* boolean isOnCurrentThread (); */
  NS_IMETHOD IsOnCurrentThread(PRBool *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIEVENTTARGET \
  NS_IMETHOD PostEvent(PLEvent * aEvent); \
  NS_IMETHOD IsOnCurrentThread(PRBool *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIEVENTTARGET(_to) \
  NS_IMETHOD PostEvent(PLEvent * aEvent) { return _to PostEvent(aEvent); } \
  NS_IMETHOD IsOnCurrentThread(PRBool *_retval) { return _to IsOnCurrentThread(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIEVENTTARGET(_to) \
  NS_IMETHOD PostEvent(PLEvent * aEvent) { return !_to ? NS_ERROR_NULL_POINTER : _to->PostEvent(aEvent); } \
  NS_IMETHOD IsOnCurrentThread(PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->IsOnCurrentThread(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsEventTarget : public nsIEventTarget
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIEVENTTARGET

  nsEventTarget();

private:
  ~nsEventTarget();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsEventTarget, nsIEventTarget)

nsEventTarget::nsEventTarget()
{
  /* member initializers and constructor code */
}

nsEventTarget::~nsEventTarget()
{
  /* destructor code */
}

/* [noscript] void postEvent (in PLEventPtr aEvent); */
NS_IMETHODIMP nsEventTarget::PostEvent(PLEvent * aEvent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isOnCurrentThread (); */
NS_IMETHODIMP nsEventTarget::IsOnCurrentThread(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIEventTarget_h__ */
