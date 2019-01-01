/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/threads/nsIEventQueue.idl
 */

#ifndef __gen_nsIEventQueue_h__
#define __gen_nsIEventQueue_h__


#ifndef __gen_nsIEventTarget_h__
#include "nsIEventTarget.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "prthread.h"
// {13D86C61-00A9-11d3-9F2A-00400553EEF0}
#define NS_EVENTQUEUE_CID \
{ 0x13d86c61, 0xa9, 0x11d3, { 0x9f, 0x2a, 0x0, 0x40, 0x5, 0x53, 0xee, 0xf0 } }
#define NS_EVENTQUEUE_CONTRACTID "@mozilla.org/event-queue;1"
#define NS_EVENTQUEUE_CLASSNAME "Event Queue"

/* starting interface:    nsIEventQueue */
#define NS_IEVENTQUEUE_IID_STR "176afb41-00a4-11d3-9f2a-00400553eef0"

#define NS_IEVENTQUEUE_IID \
  {0x176afb41, 0x00a4, 0x11d3, \
    { 0x9f, 0x2a, 0x00, 0x40, 0x05, 0x53, 0xee, 0xf0 }}

class NS_NO_VTABLE nsIEventQueue : public nsIEventTarget {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IEVENTQUEUE_IID)

  /* [noscript] void initEvent (in PLEventPtr aEvent, in voidPtr owner, in PLHandleEventProc handler, in PLDestroyEventProc destructor); */
  NS_IMETHOD InitEvent(PLEvent * aEvent, void * owner, PLHandleEventProc handler, PLDestroyEventProc destructor) = 0;

  /* [noscript] void postSynchronousEvent (in PLEventPtr aEvent, out voidPtr aResult); */
  NS_IMETHOD PostSynchronousEvent(PLEvent * aEvent, void * *aResult) = 0;

  /* boolean pendingEvents (); */
  NS_IMETHOD PendingEvents(PRBool *_retval) = 0;

  /* void processPendingEvents (); */
  NS_IMETHOD ProcessPendingEvents(void) = 0;

  /* void eventLoop (); */
  NS_IMETHOD EventLoop(void) = 0;

  /* [noscript] void eventAvailable (in PRBoolRef aResult); */
  NS_IMETHOD EventAvailable(PRBool & aResult) = 0;

  /* [noscript] PLEventPtr getEvent (); */
  NS_IMETHOD GetEvent(PLEvent * *_retval) = 0;

  /* [noscript] void handleEvent (in PLEventPtr aEvent); */
  NS_IMETHOD HandleEvent(PLEvent * aEvent) = 0;

  /* [noscript] PLEventPtr waitForEvent (); */
  NS_IMETHOD WaitForEvent(PLEvent * *_retval) = 0;

  /* [notxpcom] PRInt32 getEventQueueSelectFD (); */
  NS_IMETHOD_(PRInt32) GetEventQueueSelectFD(void) = 0;

  /* void init (in boolean aNative); */
  NS_IMETHOD Init(PRBool aNative) = 0;

  /* [noscript] void initFromPRThread (in PRThreadPtr thread, in boolean aNative); */
  NS_IMETHOD InitFromPRThread(PRThread * thread, PRBool aNative) = 0;

  /* [noscript] void initFromPLQueue (in PLEventQueuePtr aQueue); */
  NS_IMETHOD InitFromPLQueue(PLEventQueue * aQueue) = 0;

  /* void enterMonitor (); */
  NS_IMETHOD EnterMonitor(void) = 0;

  /* void exitMonitor (); */
  NS_IMETHOD ExitMonitor(void) = 0;

  /**
     * Revoke events in this event queue and all other event queues
     * for this thread that have |owner| as the event owner.
     */
  /* [noscript] void revokeEvents (in voidPtr owner); */
  NS_IMETHOD RevokeEvents(void * owner) = 0;

  /* [noscript] PLEventQueuePtr getPLEventQueue (); */
  NS_IMETHOD GetPLEventQueue(PLEventQueue * *_retval) = 0;

  /* boolean isQueueNative (); */
  NS_IMETHOD IsQueueNative(PRBool *_retval) = 0;

  /* void stopAcceptingEvents (); */
  NS_IMETHOD StopAcceptingEvents(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIEVENTQUEUE \
  NS_IMETHOD InitEvent(PLEvent * aEvent, void * owner, PLHandleEventProc handler, PLDestroyEventProc destructor); \
  NS_IMETHOD PostSynchronousEvent(PLEvent * aEvent, void * *aResult); \
  NS_IMETHOD PendingEvents(PRBool *_retval); \
  NS_IMETHOD ProcessPendingEvents(void); \
  NS_IMETHOD EventLoop(void); \
  NS_IMETHOD EventAvailable(PRBool & aResult); \
  NS_IMETHOD GetEvent(PLEvent * *_retval); \
  NS_IMETHOD HandleEvent(PLEvent * aEvent); \
  NS_IMETHOD WaitForEvent(PLEvent * *_retval); \
  NS_IMETHOD_(PRInt32) GetEventQueueSelectFD(void); \
  NS_IMETHOD Init(PRBool aNative); \
  NS_IMETHOD InitFromPRThread(PRThread * thread, PRBool aNative); \
  NS_IMETHOD InitFromPLQueue(PLEventQueue * aQueue); \
  NS_IMETHOD EnterMonitor(void); \
  NS_IMETHOD ExitMonitor(void); \
  NS_IMETHOD RevokeEvents(void * owner); \
  NS_IMETHOD GetPLEventQueue(PLEventQueue * *_retval); \
  NS_IMETHOD IsQueueNative(PRBool *_retval); \
  NS_IMETHOD StopAcceptingEvents(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIEVENTQUEUE(_to) \
  NS_IMETHOD InitEvent(PLEvent * aEvent, void * owner, PLHandleEventProc handler, PLDestroyEventProc destructor) { return _to InitEvent(aEvent, owner, handler, destructor); } \
  NS_IMETHOD PostSynchronousEvent(PLEvent * aEvent, void * *aResult) { return _to PostSynchronousEvent(aEvent, aResult); } \
  NS_IMETHOD PendingEvents(PRBool *_retval) { return _to PendingEvents(_retval); } \
  NS_IMETHOD ProcessPendingEvents(void) { return _to ProcessPendingEvents(); } \
  NS_IMETHOD EventLoop(void) { return _to EventLoop(); } \
  NS_IMETHOD EventAvailable(PRBool & aResult) { return _to EventAvailable(aResult); } \
  NS_IMETHOD GetEvent(PLEvent * *_retval) { return _to GetEvent(_retval); } \
  NS_IMETHOD HandleEvent(PLEvent * aEvent) { return _to HandleEvent(aEvent); } \
  NS_IMETHOD WaitForEvent(PLEvent * *_retval) { return _to WaitForEvent(_retval); } \
  NS_IMETHOD_(PRInt32) GetEventQueueSelectFD(void) { return _to GetEventQueueSelectFD(); } \
  NS_IMETHOD Init(PRBool aNative) { return _to Init(aNative); } \
  NS_IMETHOD InitFromPRThread(PRThread * thread, PRBool aNative) { return _to InitFromPRThread(thread, aNative); } \
  NS_IMETHOD InitFromPLQueue(PLEventQueue * aQueue) { return _to InitFromPLQueue(aQueue); } \
  NS_IMETHOD EnterMonitor(void) { return _to EnterMonitor(); } \
  NS_IMETHOD ExitMonitor(void) { return _to ExitMonitor(); } \
  NS_IMETHOD RevokeEvents(void * owner) { return _to RevokeEvents(owner); } \
  NS_IMETHOD GetPLEventQueue(PLEventQueue * *_retval) { return _to GetPLEventQueue(_retval); } \
  NS_IMETHOD IsQueueNative(PRBool *_retval) { return _to IsQueueNative(_retval); } \
  NS_IMETHOD StopAcceptingEvents(void) { return _to StopAcceptingEvents(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIEVENTQUEUE(_to) \
  NS_IMETHOD InitEvent(PLEvent * aEvent, void * owner, PLHandleEventProc handler, PLDestroyEventProc destructor) { return !_to ? NS_ERROR_NULL_POINTER : _to->InitEvent(aEvent, owner, handler, destructor); } \
  NS_IMETHOD PostSynchronousEvent(PLEvent * aEvent, void * *aResult) { return !_to ? NS_ERROR_NULL_POINTER : _to->PostSynchronousEvent(aEvent, aResult); } \
  NS_IMETHOD PendingEvents(PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->PendingEvents(_retval); } \
  NS_IMETHOD ProcessPendingEvents(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ProcessPendingEvents(); } \
  NS_IMETHOD EventLoop(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->EventLoop(); } \
  NS_IMETHOD EventAvailable(PRBool & aResult) { return !_to ? NS_ERROR_NULL_POINTER : _to->EventAvailable(aResult); } \
  NS_IMETHOD GetEvent(PLEvent * *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetEvent(_retval); } \
  NS_IMETHOD HandleEvent(PLEvent * aEvent) { return !_to ? NS_ERROR_NULL_POINTER : _to->HandleEvent(aEvent); } \
  NS_IMETHOD WaitForEvent(PLEvent * *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->WaitForEvent(_retval); } \
  NS_IMETHOD_(PRInt32) GetEventQueueSelectFD(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetEventQueueSelectFD(); } \
  NS_IMETHOD Init(PRBool aNative) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(aNative); } \
  NS_IMETHOD InitFromPRThread(PRThread * thread, PRBool aNative) { return !_to ? NS_ERROR_NULL_POINTER : _to->InitFromPRThread(thread, aNative); } \
  NS_IMETHOD InitFromPLQueue(PLEventQueue * aQueue) { return !_to ? NS_ERROR_NULL_POINTER : _to->InitFromPLQueue(aQueue); } \
  NS_IMETHOD EnterMonitor(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->EnterMonitor(); } \
  NS_IMETHOD ExitMonitor(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ExitMonitor(); } \
  NS_IMETHOD RevokeEvents(void * owner) { return !_to ? NS_ERROR_NULL_POINTER : _to->RevokeEvents(owner); } \
  NS_IMETHOD GetPLEventQueue(PLEventQueue * *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPLEventQueue(_retval); } \
  NS_IMETHOD IsQueueNative(PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->IsQueueNative(_retval); } \
  NS_IMETHOD StopAcceptingEvents(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->StopAcceptingEvents(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsEventQueue : public nsIEventQueue
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIEVENTQUEUE

  nsEventQueue();

private:
  ~nsEventQueue();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsEventQueue, nsIEventQueue)

nsEventQueue::nsEventQueue()
{
  /* member initializers and constructor code */
}

nsEventQueue::~nsEventQueue()
{
  /* destructor code */
}

/* [noscript] void initEvent (in PLEventPtr aEvent, in voidPtr owner, in PLHandleEventProc handler, in PLDestroyEventProc destructor); */
NS_IMETHODIMP nsEventQueue::InitEvent(PLEvent * aEvent, void * owner, PLHandleEventProc handler, PLDestroyEventProc destructor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void postSynchronousEvent (in PLEventPtr aEvent, out voidPtr aResult); */
NS_IMETHODIMP nsEventQueue::PostSynchronousEvent(PLEvent * aEvent, void * *aResult)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean pendingEvents (); */
NS_IMETHODIMP nsEventQueue::PendingEvents(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void processPendingEvents (); */
NS_IMETHODIMP nsEventQueue::ProcessPendingEvents()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void eventLoop (); */
NS_IMETHODIMP nsEventQueue::EventLoop()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void eventAvailable (in PRBoolRef aResult); */
NS_IMETHODIMP nsEventQueue::EventAvailable(PRBool & aResult)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] PLEventPtr getEvent (); */
NS_IMETHODIMP nsEventQueue::GetEvent(PLEvent * *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void handleEvent (in PLEventPtr aEvent); */
NS_IMETHODIMP nsEventQueue::HandleEvent(PLEvent * aEvent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] PLEventPtr waitForEvent (); */
NS_IMETHODIMP nsEventQueue::WaitForEvent(PLEvent * *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [notxpcom] PRInt32 getEventQueueSelectFD (); */
NS_IMETHODIMP_(PRInt32) nsEventQueue::GetEventQueueSelectFD()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void init (in boolean aNative); */
NS_IMETHODIMP nsEventQueue::Init(PRBool aNative)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void initFromPRThread (in PRThreadPtr thread, in boolean aNative); */
NS_IMETHODIMP nsEventQueue::InitFromPRThread(PRThread * thread, PRBool aNative)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void initFromPLQueue (in PLEventQueuePtr aQueue); */
NS_IMETHODIMP nsEventQueue::InitFromPLQueue(PLEventQueue * aQueue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void enterMonitor (); */
NS_IMETHODIMP nsEventQueue::EnterMonitor()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void exitMonitor (); */
NS_IMETHODIMP nsEventQueue::ExitMonitor()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void revokeEvents (in voidPtr owner); */
NS_IMETHODIMP nsEventQueue::RevokeEvents(void * owner)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] PLEventQueuePtr getPLEventQueue (); */
NS_IMETHODIMP nsEventQueue::GetPLEventQueue(PLEventQueue * *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isQueueNative (); */
NS_IMETHODIMP nsEventQueue::IsQueueNative(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void stopAcceptingEvents (); */
NS_IMETHODIMP nsEventQueue::StopAcceptingEvents()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIEventQueue_h__ */
