/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/threads/nsIThread.idl
 */

#ifndef __gen_nsIThread_h__
#define __gen_nsIThread_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "prthread.h"
#define NS_THREAD_CID                                \
{ /* 85CE5510-7808-11d3-A181-0050041CAF44 */         \
    0x85ce5510,                                      \
    0x7808,                                          \
    0x11d3,                                          \
    {0xa1, 0x81, 0x00, 0x50, 0x04, 0x1c, 0xaf, 0x44} \
}
#define NS_THREAD_CONTRACTID "@mozilla.org/thread;1"
#define NS_THREAD_CLASSNAME "Thread"
#if 0
typedef PRUint32 PRThreadPriority;

typedef PRUint32 PRThreadScope;

typedef PRUint32 PRThreadState;

#endif
class nsIRunnable; /* forward declaration */


/* starting interface:    nsIThread */
#define NS_ITHREAD_IID_STR "6be5e380-6886-11d3-9382-00104ba0fd40"

#define NS_ITHREAD_IID \
  {0x6be5e380, 0x6886, 0x11d3, \
    { 0x93, 0x82, 0x00, 0x10, 0x4b, 0xa0, 0xfd, 0x40 }}

class nsIThread : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ITHREAD_IID)

  enum { PRIORITY_LOW = 0U };

  enum { PRIORITY_NORMAL = 1U };

  enum { PRIORITY_HIGH = 2U };

  enum { PRIORITY_URGENT = 3U };

  enum { SCOPE_LOCAL = 0U };

  enum { SCOPE_GLOBAL = 1U };

  enum { SCOPE_BOUND = 2U };

  enum { STATE_JOINABLE = 0U };

  enum { STATE_UNJOINABLE = 1U };

  /* void join (); */
  NS_IMETHOD Join(void) = 0;

  /* void interrupt (); */
  NS_IMETHOD Interrupt(void) = 0;

  /* attribute PRThreadPriority priority; */
  NS_IMETHOD GetPriority(PRThreadPriority *aPriority) = 0;
  NS_IMETHOD SetPriority(PRThreadPriority aPriority) = 0;

  /* readonly attribute PRThreadScope scope; */
  NS_IMETHOD GetScope(PRThreadScope *aScope) = 0;

  /* readonly attribute PRThreadState state; */
  NS_IMETHOD GetState(PRThreadState *aState) = 0;

  /* [noscript] PRThread GetPRThread (); */
  NS_IMETHOD GetPRThread(PRThread * *_retval) = 0;

  /* void init (in nsIRunnable aRunnable, in PRUint32 aStackSize, in PRThreadPriority aPriority, in PRThreadScope aScope, in PRThreadState aState); */
  NS_IMETHOD Init(nsIRunnable *aRunnable, PRUint32 aStackSize, PRThreadPriority aPriority, PRThreadScope aScope, PRThreadState aState) = 0;

  /* readonly attribute nsIThread currentThread; */
  NS_IMETHOD GetCurrentThread(nsIThread * *aCurrentThread) = 0;

  /* void sleep (in PRUint32 msec); */
  NS_IMETHOD Sleep(PRUint32 msec) = 0;

    // returns the nsIThread for the current thread:
    static NS_COM nsresult GetCurrent(nsIThread* *result);
    // returns the nsIThread for an arbitrary PRThread:
    static NS_COM nsresult GetIThread(PRThread* prthread, nsIThread* *result);
    // initializes the "main" thread (really, just saves the current thread
    // at time of calling. meant to be called once at app startup, in lieu
    // of proper static initializers, to save the primordial thread
    // for later recall.)
    static NS_COM nsresult SetMainThread();
    // return the "main" thread
    static NS_COM nsresult GetMainThread(nsIThread **result);
    static NS_COM PRBool IsMainThread(); 
};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSITHREAD \
  NS_IMETHOD Join(void); \
  NS_IMETHOD Interrupt(void); \
  NS_IMETHOD GetPriority(PRThreadPriority *aPriority); \
  NS_IMETHOD SetPriority(PRThreadPriority aPriority); \
  NS_IMETHOD GetScope(PRThreadScope *aScope); \
  NS_IMETHOD GetState(PRThreadState *aState); \
  NS_IMETHOD GetPRThread(PRThread * *_retval); \
  NS_IMETHOD Init(nsIRunnable *aRunnable, PRUint32 aStackSize, PRThreadPriority aPriority, PRThreadScope aScope, PRThreadState aState); \
  NS_IMETHOD GetCurrentThread(nsIThread * *aCurrentThread); \
  NS_IMETHOD Sleep(PRUint32 msec); \

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSITHREAD(_to) \
  NS_IMETHOD Join(void) { return _to Join(); } \
  NS_IMETHOD Interrupt(void) { return _to Interrupt(); } \
  NS_IMETHOD GetPriority(PRThreadPriority *aPriority) { return _to GetPriority(aPriority); } \
  NS_IMETHOD SetPriority(PRThreadPriority aPriority) { return _to SetPriority(aPriority); } \
  NS_IMETHOD GetScope(PRThreadScope *aScope) { return _to GetScope(aScope); } \
  NS_IMETHOD GetState(PRThreadState *aState) { return _to GetState(aState); } \
  NS_IMETHOD GetPRThread(PRThread * *_retval) { return _to GetPRThread(_retval); } \
  NS_IMETHOD Init(nsIRunnable *aRunnable, PRUint32 aStackSize, PRThreadPriority aPriority, PRThreadScope aScope, PRThreadState aState) { return _to Init(aRunnable, aStackSize, aPriority, aScope, aState); } \
  NS_IMETHOD GetCurrentThread(nsIThread * *aCurrentThread) { return _to GetCurrentThread(aCurrentThread); } \
  NS_IMETHOD Sleep(PRUint32 msec) { return _to Sleep(msec); } \

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSITHREAD(_to) \
  NS_IMETHOD Join(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Join(); } \
  NS_IMETHOD Interrupt(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Interrupt(); } \
  NS_IMETHOD GetPriority(PRThreadPriority *aPriority) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPriority(aPriority); } \
  NS_IMETHOD SetPriority(PRThreadPriority aPriority) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPriority(aPriority); } \
  NS_IMETHOD GetScope(PRThreadScope *aScope) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScope(aScope); } \
  NS_IMETHOD GetState(PRThreadState *aState) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetState(aState); } \
  NS_IMETHOD GetPRThread(PRThread * *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPRThread(_retval); } \
  NS_IMETHOD Init(nsIRunnable *aRunnable, PRUint32 aStackSize, PRThreadPriority aPriority, PRThreadScope aScope, PRThreadState aState) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(aRunnable, aStackSize, aPriority, aScope, aState); } \
  NS_IMETHOD GetCurrentThread(nsIThread * *aCurrentThread) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCurrentThread(aCurrentThread); } \
  NS_IMETHOD Sleep(PRUint32 msec) { return !_to ? NS_ERROR_NULL_POINTER : _to->Sleep(msec); } \

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsThread : public nsIThread
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSITHREAD

  nsThread();

private:
  ~nsThread();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsThread, nsIThread)

nsThread::nsThread()
{
  /* member initializers and constructor code */
}

nsThread::~nsThread()
{
  /* destructor code */
}

/* void join (); */
NS_IMETHODIMP nsThread::Join()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void interrupt (); */
NS_IMETHODIMP nsThread::Interrupt()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute PRThreadPriority priority; */
NS_IMETHODIMP nsThread::GetPriority(PRThreadPriority *aPriority)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsThread::SetPriority(PRThreadPriority aPriority)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRThreadScope scope; */
NS_IMETHODIMP nsThread::GetScope(PRThreadScope *aScope)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRThreadState state; */
NS_IMETHODIMP nsThread::GetState(PRThreadState *aState)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] PRThread GetPRThread (); */
NS_IMETHODIMP nsThread::GetPRThread(PRThread * *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void init (in nsIRunnable aRunnable, in PRUint32 aStackSize, in PRThreadPriority aPriority, in PRThreadScope aScope, in PRThreadState aState); */
NS_IMETHODIMP nsThread::Init(nsIRunnable *aRunnable, PRUint32 aStackSize, PRThreadPriority aPriority, PRThreadScope aScope, PRThreadState aState)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIThread currentThread; */
NS_IMETHODIMP nsThread::GetCurrentThread(nsIThread * *aCurrentThread)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void sleep (in PRUint32 msec); */
NS_IMETHODIMP nsThread::Sleep(PRUint32 msec)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

extern NS_COM nsresult
NS_NewThread(nsIThread* *result, 
             nsIRunnable* runnable,
             PRUint32 stackSize = 0,
             PRThreadState state = PR_UNJOINABLE_THREAD,
             PRThreadPriority priority = PR_PRIORITY_NORMAL,
             PRThreadScope scope = PR_GLOBAL_THREAD);
extern NS_COM nsresult
NS_NewThread(nsIThread* *result, 
             PRUint32 stackSize = 0,
             PRThreadState state = PR_UNJOINABLE_THREAD,
             PRThreadPriority priority = PR_PRIORITY_NORMAL,
             PRThreadScope scope = PR_GLOBAL_THREAD);

#endif /* __gen_nsIThread_h__ */
