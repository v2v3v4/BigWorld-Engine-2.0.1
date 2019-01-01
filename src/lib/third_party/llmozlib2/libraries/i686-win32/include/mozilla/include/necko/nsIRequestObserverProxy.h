/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsIRequestObserverProxy.idl
 */

#ifndef __gen_nsIRequestObserverProxy_h__
#define __gen_nsIRequestObserverProxy_h__


#ifndef __gen_nsIRequestObserver_h__
#include "nsIRequestObserver.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIEventQueue; /* forward declaration */


/* starting interface:    nsIRequestObserverProxy */
#define NS_IREQUESTOBSERVERPROXY_IID_STR "3c9b532e-db84-4ecf-aa6a-4d38a9c4c5f0"

#define NS_IREQUESTOBSERVERPROXY_IID \
  {0x3c9b532e, 0xdb84, 0x4ecf, \
    { 0xaa, 0x6a, 0x4d, 0x38, 0xa9, 0xc4, 0xc5, 0xf0 }}

/**
 * A request observer proxy is used to ship data over to another thread specified
 * by the thread's event queue. The "true" request observer's methods are 
 * invoked on the other thread.
 *
 * This interface only provides the initialization needed after construction. Otherwise,
 * these objects are used simply as nsIRequestObserver's.
 */
class NS_NO_VTABLE nsIRequestObserverProxy : public nsIRequestObserver {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IREQUESTOBSERVERPROXY_IID)

  /**
     * Initializes an nsIRequestObserverProxy.
     *
     * @param observer - receives observer notifications on the other thread
     * @param eventQ - may be NULL indicating the calling thread's event queue
     */
  /* void init (in nsIRequestObserver observer, in nsIEventQueue eventQ); */
  NS_IMETHOD Init(nsIRequestObserver *observer, nsIEventQueue *eventQ) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIREQUESTOBSERVERPROXY \
  NS_IMETHOD Init(nsIRequestObserver *observer, nsIEventQueue *eventQ); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIREQUESTOBSERVERPROXY(_to) \
  NS_IMETHOD Init(nsIRequestObserver *observer, nsIEventQueue *eventQ) { return _to Init(observer, eventQ); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIREQUESTOBSERVERPROXY(_to) \
  NS_IMETHOD Init(nsIRequestObserver *observer, nsIEventQueue *eventQ) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(observer, eventQ); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsRequestObserverProxy : public nsIRequestObserverProxy
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIREQUESTOBSERVERPROXY

  nsRequestObserverProxy();

private:
  ~nsRequestObserverProxy();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsRequestObserverProxy, nsIRequestObserverProxy)

nsRequestObserverProxy::nsRequestObserverProxy()
{
  /* member initializers and constructor code */
}

nsRequestObserverProxy::~nsRequestObserverProxy()
{
  /* destructor code */
}

/* void init (in nsIRequestObserver observer, in nsIEventQueue eventQ); */
NS_IMETHODIMP nsRequestObserverProxy::Init(nsIRequestObserver *observer, nsIEventQueue *eventQ)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIRequestObserverProxy_h__ */
