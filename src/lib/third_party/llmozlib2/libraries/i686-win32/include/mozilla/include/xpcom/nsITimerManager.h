/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/threads/nsITimerManager.idl
 */

#ifndef __gen_nsITimerManager_h__
#define __gen_nsITimerManager_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsITimerManager */
#define NS_ITIMERMANAGER_IID_STR "8fce8c6a-1dd2-11b2-8352-8cdd2b965efc"

#define NS_ITIMERMANAGER_IID \
  {0x8fce8c6a, 0x1dd2, 0x11b2, \
    { 0x83, 0x52, 0x8c, 0xdd, 0x2b, 0x96, 0x5e, 0xfc }}

class NS_NO_VTABLE nsITimerManager : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ITIMERMANAGER_IID)

  /**
   * A flag that turns on the use of idle timers on the main thread.
   * this should only be called once.
   *
   * By default, idle timers are off.
   *
   * One this is set to TRUE, you are expected to call hasIdleTimers/fireNextIdleTimer
   * when you have time in your main loop.
   */
  /* attribute boolean useIdleTimers; */
  NS_IMETHOD GetUseIdleTimers(PRBool *aUseIdleTimers) = 0;
  NS_IMETHOD SetUseIdleTimers(PRBool aUseIdleTimers) = 0;

  /* boolean hasIdleTimers (); */
  NS_IMETHOD HasIdleTimers(PRBool *_retval) = 0;

  /* void fireNextIdleTimer (); */
  NS_IMETHOD FireNextIdleTimer(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSITIMERMANAGER \
  NS_IMETHOD GetUseIdleTimers(PRBool *aUseIdleTimers); \
  NS_IMETHOD SetUseIdleTimers(PRBool aUseIdleTimers); \
  NS_IMETHOD HasIdleTimers(PRBool *_retval); \
  NS_IMETHOD FireNextIdleTimer(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSITIMERMANAGER(_to) \
  NS_IMETHOD GetUseIdleTimers(PRBool *aUseIdleTimers) { return _to GetUseIdleTimers(aUseIdleTimers); } \
  NS_IMETHOD SetUseIdleTimers(PRBool aUseIdleTimers) { return _to SetUseIdleTimers(aUseIdleTimers); } \
  NS_IMETHOD HasIdleTimers(PRBool *_retval) { return _to HasIdleTimers(_retval); } \
  NS_IMETHOD FireNextIdleTimer(void) { return _to FireNextIdleTimer(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSITIMERMANAGER(_to) \
  NS_IMETHOD GetUseIdleTimers(PRBool *aUseIdleTimers) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetUseIdleTimers(aUseIdleTimers); } \
  NS_IMETHOD SetUseIdleTimers(PRBool aUseIdleTimers) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetUseIdleTimers(aUseIdleTimers); } \
  NS_IMETHOD HasIdleTimers(PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->HasIdleTimers(_retval); } \
  NS_IMETHOD FireNextIdleTimer(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->FireNextIdleTimer(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsTimerManager : public nsITimerManager
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSITIMERMANAGER

  nsTimerManager();

private:
  ~nsTimerManager();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsTimerManager, nsITimerManager)

nsTimerManager::nsTimerManager()
{
  /* member initializers and constructor code */
}

nsTimerManager::~nsTimerManager()
{
  /* destructor code */
}

/* attribute boolean useIdleTimers; */
NS_IMETHODIMP nsTimerManager::GetUseIdleTimers(PRBool *aUseIdleTimers)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsTimerManager::SetUseIdleTimers(PRBool aUseIdleTimers)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean hasIdleTimers (); */
NS_IMETHODIMP nsTimerManager::HasIdleTimers(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void fireNextIdleTimer (); */
NS_IMETHODIMP nsTimerManager::FireNextIdleTimer()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsITimerManager_h__ */
