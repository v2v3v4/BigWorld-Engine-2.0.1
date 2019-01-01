/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/threads/nsIEventQueueListener.idl
 */

#ifndef __gen_nsIEventQueueListener_h__
#define __gen_nsIEventQueueListener_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIEventQueue; /* forward declaration */


/* starting interface:    nsIEventQueueListener */
#define NS_IEVENTQUEUELISTENER_IID_STR "7579c049-8f16-4981-a6fb-a2c4126799ca"

#define NS_IEVENTQUEUELISTENER_IID \
  {0x7579c049, 0x8f16, 0x4981, \
    { 0xa6, 0xfb, 0xa2, 0xc4, 0x12, 0x67, 0x99, 0xca }}

/**
 * This interface represents a listener who wants to be notified when
 * an event queue is about to process events or when it has finished
 * processing events.
 */
class NS_NO_VTABLE nsIEventQueueListener : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IEVENTQUEUELISTENER_IID)

  /**
   * Call before processing events.
   *
   * @param aQueue the queue that will process events
   *
   * If this method throws, events should NOT be processed.
   */
  /* void willProcessEvents (in nsIEventQueue aQueue); */
  NS_IMETHOD WillProcessEvents(nsIEventQueue *aQueue) = 0;

  /**
   * Call after processing events.
   *
   * @param aQueue the queue that has processed events
   */
  /* void didProcessEvents (in nsIEventQueue aQueue); */
  NS_IMETHOD DidProcessEvents(nsIEventQueue *aQueue) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIEVENTQUEUELISTENER \
  NS_IMETHOD WillProcessEvents(nsIEventQueue *aQueue); \
  NS_IMETHOD DidProcessEvents(nsIEventQueue *aQueue); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIEVENTQUEUELISTENER(_to) \
  NS_IMETHOD WillProcessEvents(nsIEventQueue *aQueue) { return _to WillProcessEvents(aQueue); } \
  NS_IMETHOD DidProcessEvents(nsIEventQueue *aQueue) { return _to DidProcessEvents(aQueue); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIEVENTQUEUELISTENER(_to) \
  NS_IMETHOD WillProcessEvents(nsIEventQueue *aQueue) { return !_to ? NS_ERROR_NULL_POINTER : _to->WillProcessEvents(aQueue); } \
  NS_IMETHOD DidProcessEvents(nsIEventQueue *aQueue) { return !_to ? NS_ERROR_NULL_POINTER : _to->DidProcessEvents(aQueue); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsEventQueueListener : public nsIEventQueueListener
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIEVENTQUEUELISTENER

  nsEventQueueListener();

private:
  ~nsEventQueueListener();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsEventQueueListener, nsIEventQueueListener)

nsEventQueueListener::nsEventQueueListener()
{
  /* member initializers and constructor code */
}

nsEventQueueListener::~nsEventQueueListener()
{
  /* destructor code */
}

/* void willProcessEvents (in nsIEventQueue aQueue); */
NS_IMETHODIMP nsEventQueueListener::WillProcessEvents(nsIEventQueue *aQueue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void didProcessEvents (in nsIEventQueue aQueue); */
NS_IMETHODIMP nsEventQueueListener::DidProcessEvents(nsIEventQueue *aQueue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIEventQueueListener_h__ */
