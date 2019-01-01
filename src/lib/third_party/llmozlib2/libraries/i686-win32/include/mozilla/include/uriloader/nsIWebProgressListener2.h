/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/uriloader/base/nsIWebProgressListener2.idl
 */

#ifndef __gen_nsIWebProgressListener2_h__
#define __gen_nsIWebProgressListener2_h__


#ifndef __gen_nsIWebProgressListener_h__
#include "nsIWebProgressListener.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIWebProgressListener2 */
#define NS_IWEBPROGRESSLISTENER2_IID_STR "3f24610d-1e1f-4151-9d2e-239884742324"

#define NS_IWEBPROGRESSLISTENER2_IID \
  {0x3f24610d, 0x1e1f, 0x4151, \
    { 0x9d, 0x2e, 0x23, 0x98, 0x84, 0x74, 0x23, 0x24 }}

/**
 * This interface is an extension to nsIWebProgressListener to support 64-bit
 * progress values.
 */
class NS_NO_VTABLE nsIWebProgressListener2 : public nsIWebProgressListener {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IWEBPROGRESSLISTENER2_IID)

  /**
   * Notification that the progress has changed for one of the requests
   * associated with aWebProgress.  Progress totals are reset to zero when all
   * requests in aWebProgress complete (corresponding to onStateChange being
   * called with aStateFlags including the STATE_STOP and STATE_IS_WINDOW
   * flags).
   *
   * This function is identical to nsIWebProgressListener::onProgressChange,
   * except that this function supports 64-bit values.
   *
   * @param aWebProgress
   *        The nsIWebProgress instance that fired the notification.
   * @param aRequest
   *        The nsIRequest that has new progress.
   * @param aCurSelfProgress
   *        The current progress for aRequest.
   * @param aMaxSelfProgress
   *        The maximum progress for aRequest.
   * @param aCurTotalProgress
   *        The current progress for all requests associated with aWebProgress.
   * @param aMaxTotalProgress
   *        The total progress for all requests associated with aWebProgress.
   *
   * NOTE: If any progress value is unknown, then its value is replaced with -1.
   *
   * @see nsIWebProgressListener2::onProgressChange64
   */
  /* void onProgressChange64 (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in long long aCurSelfProgress, in long long aMaxSelfProgress, in long long aCurTotalProgress, in long long aMaxTotalProgress); */
  NS_IMETHOD OnProgressChange64(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRInt64 aCurSelfProgress, PRInt64 aMaxSelfProgress, PRInt64 aCurTotalProgress, PRInt64 aMaxTotalProgress) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIWEBPROGRESSLISTENER2 \
  NS_IMETHOD OnProgressChange64(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRInt64 aCurSelfProgress, PRInt64 aMaxSelfProgress, PRInt64 aCurTotalProgress, PRInt64 aMaxTotalProgress); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIWEBPROGRESSLISTENER2(_to) \
  NS_IMETHOD OnProgressChange64(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRInt64 aCurSelfProgress, PRInt64 aMaxSelfProgress, PRInt64 aCurTotalProgress, PRInt64 aMaxTotalProgress) { return _to OnProgressChange64(aWebProgress, aRequest, aCurSelfProgress, aMaxSelfProgress, aCurTotalProgress, aMaxTotalProgress); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIWEBPROGRESSLISTENER2(_to) \
  NS_IMETHOD OnProgressChange64(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRInt64 aCurSelfProgress, PRInt64 aMaxSelfProgress, PRInt64 aCurTotalProgress, PRInt64 aMaxTotalProgress) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnProgressChange64(aWebProgress, aRequest, aCurSelfProgress, aMaxSelfProgress, aCurTotalProgress, aMaxTotalProgress); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsWebProgressListener2 : public nsIWebProgressListener2
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIWEBPROGRESSLISTENER2

  nsWebProgressListener2();

private:
  ~nsWebProgressListener2();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsWebProgressListener2, nsIWebProgressListener2)

nsWebProgressListener2::nsWebProgressListener2()
{
  /* member initializers and constructor code */
}

nsWebProgressListener2::~nsWebProgressListener2()
{
  /* destructor code */
}

/* void onProgressChange64 (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in long long aCurSelfProgress, in long long aMaxSelfProgress, in long long aCurTotalProgress, in long long aMaxTotalProgress); */
NS_IMETHODIMP nsWebProgressListener2::OnProgressChange64(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRInt64 aCurSelfProgress, PRInt64 aMaxSelfProgress, PRInt64 aCurTotalProgress, PRInt64 aMaxTotalProgress)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIWebProgressListener2_h__ */
