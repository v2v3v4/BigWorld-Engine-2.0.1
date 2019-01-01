/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/threads/nsITimerInternal.idl
 */

#ifndef __gen_nsITimerInternal_h__
#define __gen_nsITimerInternal_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsITimerInternal */
#define NS_ITIMERINTERNAL_IID_STR "6dd8f185-ceb8-4878-8e38-2d13edc2d079"

#define NS_ITIMERINTERNAL_IID \
  {0x6dd8f185, 0xceb8, 0x4878, \
    { 0x8e, 0x38, 0x2d, 0x13, 0xed, 0xc2, 0xd0, 0x79 }}

class NS_NO_VTABLE nsITimerInternal : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ITIMERINTERNAL_IID)

  /* attribute boolean idle; */
  NS_IMETHOD GetIdle(PRBool *aIdle) = 0;
  NS_IMETHOD SetIdle(PRBool aIdle) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSITIMERINTERNAL \
  NS_IMETHOD GetIdle(PRBool *aIdle); \
  NS_IMETHOD SetIdle(PRBool aIdle); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSITIMERINTERNAL(_to) \
  NS_IMETHOD GetIdle(PRBool *aIdle) { return _to GetIdle(aIdle); } \
  NS_IMETHOD SetIdle(PRBool aIdle) { return _to SetIdle(aIdle); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSITIMERINTERNAL(_to) \
  NS_IMETHOD GetIdle(PRBool *aIdle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIdle(aIdle); } \
  NS_IMETHOD SetIdle(PRBool aIdle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetIdle(aIdle); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsTimerInternal : public nsITimerInternal
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSITIMERINTERNAL

  nsTimerInternal();

private:
  ~nsTimerInternal();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsTimerInternal, nsITimerInternal)

nsTimerInternal::nsTimerInternal()
{
  /* member initializers and constructor code */
}

nsTimerInternal::~nsTimerInternal()
{
  /* destructor code */
}

/* attribute boolean idle; */
NS_IMETHODIMP nsTimerInternal::GetIdle(PRBool *aIdle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsTimerInternal::SetIdle(PRBool aIdle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsITimerInternal_h__ */
