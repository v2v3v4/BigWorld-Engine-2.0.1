/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/threads/nsIEventQueueService.idl
 */

#ifndef __gen_nsIEventQueueService_h__
#define __gen_nsIEventQueueService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsIEventQueue_h__
#include "nsIEventQueue.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "prthread.h"
#include "plevent.h"
/* be761f00-a3b0-11d2-996c-0080c7cb1080 */
#define NS_EVENTQUEUESERVICE_CID \
{ 0xbe761f00, 0xa3b0, 0x11d2, \
  {0x99, 0x6c, 0x00, 0x80, 0xc7, 0xcb, 0x10, 0x80} }
#define NS_EVENTQUEUESERVICE_CONTRACTID "@mozilla.org/event-queue-service;1"
#define NS_EVENTQUEUESERVICE_CLASSNAME "Event Queue Service"
#define NS_CURRENT_THREAD    ((PRThread*)0)
#define NS_CURRENT_EVENTQ    ((nsIEventQueue*)0)
#define NS_UI_THREAD         ((PRThread*)1)
#define NS_UI_THREAD_EVENTQ  ((nsIEventQueue*)1)
class nsIThread; /* forward declaration */


/* starting interface:    nsIEventQueueService */
#define NS_IEVENTQUEUESERVICE_IID_STR "a6cf90dc-15b3-11d2-932e-00805f8add32"

#define NS_IEVENTQUEUESERVICE_IID \
  {0xa6cf90dc, 0x15b3, 0x11d2, \
    { 0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32 }}

class NS_NO_VTABLE nsIEventQueueService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IEVENTQUEUESERVICE_IID)

  /**
   * Creates and holds a native event queue for the current thread.
   * "Native" queues have an associated callback mechanism which is
   * automatically triggered when an event is posted. See plevent.c for 
   * details.
   * @return NS_OK on success, or a host of failure indications
   */
  /* void createThreadEventQueue (); */
  NS_IMETHOD CreateThreadEventQueue(void) = 0;

  /**
   * Creates and hold a monitored event queue for the current thread.
   * "Monitored" queues have no callback processing mechanism.
   * @return NS_OK on success, or a host of failure indications
   */
  /* void createMonitoredThreadEventQueue (); */
  NS_IMETHOD CreateMonitoredThreadEventQueue(void) = 0;

  /**
   * Somewhat misnamed, this method releases the service's hold on the event
   * queue(s) for this thread. Subsequent attempts to access this thread's
   * queue (GetThreadEventQueue, for example) may fail, though the queue itself
   * will be destroyed only after all references to it are released and the
   * queue itself is no longer actively processing events.
   * @return nonsense.
   */
  /* void destroyThreadEventQueue (); */
  NS_IMETHOD DestroyThreadEventQueue(void) = 0;

  /* nsIEventQueue createFromIThread (in nsIThread aThread, in boolean aNative); */
  NS_IMETHOD CreateFromIThread(nsIThread *aThread, PRBool aNative, nsIEventQueue **_retval) = 0;

  /* [noscript] nsIEventQueue createFromPLEventQueue (in PLEventQueuePtr aPLEventQueue); */
  NS_IMETHOD CreateFromPLEventQueue(PLEventQueue * aPLEventQueue, nsIEventQueue **_retval) = 0;

  /* nsIEventQueue pushThreadEventQueue (); */
  NS_IMETHOD PushThreadEventQueue(nsIEventQueue **_retval) = 0;

  /* void popThreadEventQueue (in nsIEventQueue aQueue); */
  NS_IMETHOD PopThreadEventQueue(nsIEventQueue *aQueue) = 0;

  /* [noscript] nsIEventQueue getThreadEventQueue (in PRThreadPtr aThread); */
  NS_IMETHOD GetThreadEventQueue(PRThread * aThread, nsIEventQueue **_retval) = 0;

  /**
   * @deprecated in favor of getSpecialEventQueue, since that's
   * scriptable and this isn't.
   *
   * Check for any "magic" event queue constants (NS_CURRENT_EVENTQ,
   * NS_UI_THREAD_EVENTQ) and return the real event queue that they
   * represent, AddRef()ed.  Otherwise, return the event queue passed
   * in, AddRef()ed.  This is not scriptable because the arguments in
   * question may be magic constants rather than real nsIEventQueues.
   *
   * @arg queueOrConstant    either a real event queue or a magic
   *                         constant to be resolved
   *
   * @return                 a real event queue, AddRef()ed
   */
  /* [noscript] nsIEventQueue resolveEventQueue (in nsIEventQueue queueOrConstant); */
  NS_IMETHOD ResolveEventQueue(nsIEventQueue *queueOrConstant, nsIEventQueue **_retval) = 0;

  /**
   * Returns the appropriate special event queue, AddRef()ed.  Really
   * just a scriptable version of ResolveEventQueue.
   *
   * @arg aQueue    Either CURRENT_THREAD_EVENT_QUEUE or
   *                UI_THREAD_EVENT_QUEUE
   * @return        The requested nsIEventQueue, AddRef()ed
   * @exception NS_ERROR_NULL_POINTER   Zero pointer passed in for return value
   * @exception NS_ERROR_ILLEGAL_VALUE  Bogus constant passed in aQueue
   * @exception NS_ERROR_FAILURE        Error while calling 
   *                                    GetThreadEventQueue()
   */
  /* nsIEventQueue getSpecialEventQueue (in long aQueue); */
  NS_IMETHOD GetSpecialEventQueue(PRInt32 aQueue, nsIEventQueue **_retval) = 0;

  enum { CURRENT_THREAD_EVENT_QUEUE = 0 };

  enum { UI_THREAD_EVENT_QUEUE = 1 };

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIEVENTQUEUESERVICE \
  NS_IMETHOD CreateThreadEventQueue(void); \
  NS_IMETHOD CreateMonitoredThreadEventQueue(void); \
  NS_IMETHOD DestroyThreadEventQueue(void); \
  NS_IMETHOD CreateFromIThread(nsIThread *aThread, PRBool aNative, nsIEventQueue **_retval); \
  NS_IMETHOD CreateFromPLEventQueue(PLEventQueue * aPLEventQueue, nsIEventQueue **_retval); \
  NS_IMETHOD PushThreadEventQueue(nsIEventQueue **_retval); \
  NS_IMETHOD PopThreadEventQueue(nsIEventQueue *aQueue); \
  NS_IMETHOD GetThreadEventQueue(PRThread * aThread, nsIEventQueue **_retval); \
  NS_IMETHOD ResolveEventQueue(nsIEventQueue *queueOrConstant, nsIEventQueue **_retval); \
  NS_IMETHOD GetSpecialEventQueue(PRInt32 aQueue, nsIEventQueue **_retval); \

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIEVENTQUEUESERVICE(_to) \
  NS_IMETHOD CreateThreadEventQueue(void) { return _to CreateThreadEventQueue(); } \
  NS_IMETHOD CreateMonitoredThreadEventQueue(void) { return _to CreateMonitoredThreadEventQueue(); } \
  NS_IMETHOD DestroyThreadEventQueue(void) { return _to DestroyThreadEventQueue(); } \
  NS_IMETHOD CreateFromIThread(nsIThread *aThread, PRBool aNative, nsIEventQueue **_retval) { return _to CreateFromIThread(aThread, aNative, _retval); } \
  NS_IMETHOD CreateFromPLEventQueue(PLEventQueue * aPLEventQueue, nsIEventQueue **_retval) { return _to CreateFromPLEventQueue(aPLEventQueue, _retval); } \
  NS_IMETHOD PushThreadEventQueue(nsIEventQueue **_retval) { return _to PushThreadEventQueue(_retval); } \
  NS_IMETHOD PopThreadEventQueue(nsIEventQueue *aQueue) { return _to PopThreadEventQueue(aQueue); } \
  NS_IMETHOD GetThreadEventQueue(PRThread * aThread, nsIEventQueue **_retval) { return _to GetThreadEventQueue(aThread, _retval); } \
  NS_IMETHOD ResolveEventQueue(nsIEventQueue *queueOrConstant, nsIEventQueue **_retval) { return _to ResolveEventQueue(queueOrConstant, _retval); } \
  NS_IMETHOD GetSpecialEventQueue(PRInt32 aQueue, nsIEventQueue **_retval) { return _to GetSpecialEventQueue(aQueue, _retval); } \

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIEVENTQUEUESERVICE(_to) \
  NS_IMETHOD CreateThreadEventQueue(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateThreadEventQueue(); } \
  NS_IMETHOD CreateMonitoredThreadEventQueue(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateMonitoredThreadEventQueue(); } \
  NS_IMETHOD DestroyThreadEventQueue(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->DestroyThreadEventQueue(); } \
  NS_IMETHOD CreateFromIThread(nsIThread *aThread, PRBool aNative, nsIEventQueue **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateFromIThread(aThread, aNative, _retval); } \
  NS_IMETHOD CreateFromPLEventQueue(PLEventQueue * aPLEventQueue, nsIEventQueue **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateFromPLEventQueue(aPLEventQueue, _retval); } \
  NS_IMETHOD PushThreadEventQueue(nsIEventQueue **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->PushThreadEventQueue(_retval); } \
  NS_IMETHOD PopThreadEventQueue(nsIEventQueue *aQueue) { return !_to ? NS_ERROR_NULL_POINTER : _to->PopThreadEventQueue(aQueue); } \
  NS_IMETHOD GetThreadEventQueue(PRThread * aThread, nsIEventQueue **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetThreadEventQueue(aThread, _retval); } \
  NS_IMETHOD ResolveEventQueue(nsIEventQueue *queueOrConstant, nsIEventQueue **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ResolveEventQueue(queueOrConstant, _retval); } \
  NS_IMETHOD GetSpecialEventQueue(PRInt32 aQueue, nsIEventQueue **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSpecialEventQueue(aQueue, _retval); } \

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsEventQueueService : public nsIEventQueueService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIEVENTQUEUESERVICE

  nsEventQueueService();

private:
  ~nsEventQueueService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsEventQueueService, nsIEventQueueService)

nsEventQueueService::nsEventQueueService()
{
  /* member initializers and constructor code */
}

nsEventQueueService::~nsEventQueueService()
{
  /* destructor code */
}

/* void createThreadEventQueue (); */
NS_IMETHODIMP nsEventQueueService::CreateThreadEventQueue()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void createMonitoredThreadEventQueue (); */
NS_IMETHODIMP nsEventQueueService::CreateMonitoredThreadEventQueue()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void destroyThreadEventQueue (); */
NS_IMETHODIMP nsEventQueueService::DestroyThreadEventQueue()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIEventQueue createFromIThread (in nsIThread aThread, in boolean aNative); */
NS_IMETHODIMP nsEventQueueService::CreateFromIThread(nsIThread *aThread, PRBool aNative, nsIEventQueue **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] nsIEventQueue createFromPLEventQueue (in PLEventQueuePtr aPLEventQueue); */
NS_IMETHODIMP nsEventQueueService::CreateFromPLEventQueue(PLEventQueue * aPLEventQueue, nsIEventQueue **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIEventQueue pushThreadEventQueue (); */
NS_IMETHODIMP nsEventQueueService::PushThreadEventQueue(nsIEventQueue **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void popThreadEventQueue (in nsIEventQueue aQueue); */
NS_IMETHODIMP nsEventQueueService::PopThreadEventQueue(nsIEventQueue *aQueue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] nsIEventQueue getThreadEventQueue (in PRThreadPtr aThread); */
NS_IMETHODIMP nsEventQueueService::GetThreadEventQueue(PRThread * aThread, nsIEventQueue **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] nsIEventQueue resolveEventQueue (in nsIEventQueue queueOrConstant); */
NS_IMETHODIMP nsEventQueueService::ResolveEventQueue(nsIEventQueue *queueOrConstant, nsIEventQueue **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIEventQueue getSpecialEventQueue (in long aQueue); */
NS_IMETHODIMP nsEventQueueService::GetSpecialEventQueue(PRInt32 aQueue, nsIEventQueue **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIEventQueueService_h__ */
