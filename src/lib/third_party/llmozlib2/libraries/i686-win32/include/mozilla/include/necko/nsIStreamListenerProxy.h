/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsIStreamListenerProxy.idl
 */

#ifndef __gen_nsIStreamListenerProxy_h__
#define __gen_nsIStreamListenerProxy_h__


#ifndef __gen_nsIStreamListener_h__
#include "nsIStreamListener.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIEventQueue; /* forward declaration */


/* starting interface:    nsIStreamListenerProxy */
#define NS_ISTREAMLISTENERPROXY_IID_STR "e400e688-6b54-4a84-8c4e-56b40281981a"

#define NS_ISTREAMLISTENERPROXY_IID \
  {0xe400e688, 0x6b54, 0x4a84, \
    { 0x8c, 0x4e, 0x56, 0xb4, 0x02, 0x81, 0x98, 0x1a }}

/**
 * A stream listener proxy is used to ship data over to another thread specified
 * by the thread's event queue.  The "true" stream listener's methods are
 * invoked on the other thread.
 *
 * This interface only provides the initialization needed after construction. 
 * Otherwise, these objects are used as nsIStreamListener.
 */
class NS_NO_VTABLE nsIStreamListenerProxy : public nsIStreamListener {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISTREAMLISTENERPROXY_IID)

  /**
     * Initializes an nsIStreamListenerProxy.
     *
     * @param aListener receives listener notifications on the other thread
     * @param aEventQ may be NULL indicating the calling thread's event queue
     * @param aBufferSegmentSize passing zero indicates the default
     * @param aBufferMaxSize passing zero indicates the default
     */
  /* void init (in nsIStreamListener aListener, in nsIEventQueue aEventQ, in unsigned long aBufferSegmentSize, in unsigned long aBufferMaxSize); */
  NS_IMETHOD Init(nsIStreamListener *aListener, nsIEventQueue *aEventQ, PRUint32 aBufferSegmentSize, PRUint32 aBufferMaxSize) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISTREAMLISTENERPROXY \
  NS_IMETHOD Init(nsIStreamListener *aListener, nsIEventQueue *aEventQ, PRUint32 aBufferSegmentSize, PRUint32 aBufferMaxSize); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISTREAMLISTENERPROXY(_to) \
  NS_IMETHOD Init(nsIStreamListener *aListener, nsIEventQueue *aEventQ, PRUint32 aBufferSegmentSize, PRUint32 aBufferMaxSize) { return _to Init(aListener, aEventQ, aBufferSegmentSize, aBufferMaxSize); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISTREAMLISTENERPROXY(_to) \
  NS_IMETHOD Init(nsIStreamListener *aListener, nsIEventQueue *aEventQ, PRUint32 aBufferSegmentSize, PRUint32 aBufferMaxSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(aListener, aEventQ, aBufferSegmentSize, aBufferMaxSize); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsStreamListenerProxy : public nsIStreamListenerProxy
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISTREAMLISTENERPROXY

  nsStreamListenerProxy();

private:
  ~nsStreamListenerProxy();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsStreamListenerProxy, nsIStreamListenerProxy)

nsStreamListenerProxy::nsStreamListenerProxy()
{
  /* member initializers and constructor code */
}

nsStreamListenerProxy::~nsStreamListenerProxy()
{
  /* destructor code */
}

/* void init (in nsIStreamListener aListener, in nsIEventQueue aEventQ, in unsigned long aBufferSegmentSize, in unsigned long aBufferMaxSize); */
NS_IMETHODIMP nsStreamListenerProxy::Init(nsIStreamListener *aListener, nsIEventQueue *aEventQ, PRUint32 aBufferSegmentSize, PRUint32 aBufferMaxSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIAsyncStreamListener */
#define NS_IASYNCSTREAMLISTENER_IID_STR "1b012ade-91bf-11d3-8cd9-0060b0fc14a3"

#define NS_IASYNCSTREAMLISTENER_IID \
  {0x1b012ade, 0x91bf, 0x11d3, \
    { 0x8c, 0xd9, 0x00, 0x60, 0xb0, 0xfc, 0x14, 0xa3 }}

/**
 * THIS INTERFACE IS DEPRECATED
 *
 * An asynchronous stream listener is used to ship data over to another thread specified
 * by the thread's event queue. The receiver stream listener is then used to receive
 * the notifications on the other thread. 
 *
 * This interface only provides the initialization needed after construction. Otherwise,
 * these objects are used simply as nsIStreamListener.
 */
class NS_NO_VTABLE nsIAsyncStreamListener : public nsIStreamListener {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IASYNCSTREAMLISTENER_IID)

  /**
     * Initializes an nsIAsyncStreamListener. 
     *
     * @param aReceiver receives listener notifications on the other thread
     * @param aEventQ may be null indicating the calling thread's event queue
     */
  /* void init (in nsIStreamListener aReceiver, in nsIEventQueue aEventQ); */
  NS_IMETHOD Init(nsIStreamListener *aReceiver, nsIEventQueue *aEventQ) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIASYNCSTREAMLISTENER \
  NS_IMETHOD Init(nsIStreamListener *aReceiver, nsIEventQueue *aEventQ); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIASYNCSTREAMLISTENER(_to) \
  NS_IMETHOD Init(nsIStreamListener *aReceiver, nsIEventQueue *aEventQ) { return _to Init(aReceiver, aEventQ); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIASYNCSTREAMLISTENER(_to) \
  NS_IMETHOD Init(nsIStreamListener *aReceiver, nsIEventQueue *aEventQ) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(aReceiver, aEventQ); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsAsyncStreamListener : public nsIAsyncStreamListener
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIASYNCSTREAMLISTENER

  nsAsyncStreamListener();

private:
  ~nsAsyncStreamListener();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsAsyncStreamListener, nsIAsyncStreamListener)

nsAsyncStreamListener::nsAsyncStreamListener()
{
  /* member initializers and constructor code */
}

nsAsyncStreamListener::~nsAsyncStreamListener()
{
  /* destructor code */
}

/* void init (in nsIStreamListener aReceiver, in nsIEventQueue aEventQ); */
NS_IMETHODIMP nsAsyncStreamListener::Init(nsIStreamListener *aReceiver, nsIEventQueue *aEventQ)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIStreamListenerProxy_h__ */
