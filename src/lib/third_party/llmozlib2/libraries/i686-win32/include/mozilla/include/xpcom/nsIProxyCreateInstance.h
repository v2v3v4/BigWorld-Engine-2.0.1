/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/proxy/public/nsIProxyCreateInstance.idl
 */

#ifndef __gen_nsIProxyCreateInstance_h__
#define __gen_nsIProxyCreateInstance_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIProxyCreateInstance */
#define NS_IPROXYCREATEINSTANCE_IID_STR "948c2080-0398-11d3-915e-0000863011c4"

#define NS_IPROXYCREATEINSTANCE_IID \
  {0x948c2080, 0x0398, 0x11d3, \
    { 0x91, 0x5e, 0x00, 0x00, 0x86, 0x30, 0x11, 0xc4 }}

class NS_NO_VTABLE nsIProxyCreateInstance : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPROXYCREATEINSTANCE_IID)

  /* [noscript] void CreateInstanceByIID (in nsIIDRef cid, in nsISupports aOuter, in nsIIDRef iid, out voidPtr result); */
  NS_IMETHOD CreateInstanceByIID(const nsIID & cid, nsISupports *aOuter, const nsIID & iid, void * *result) = 0;

  /* [noscript] void CreateInstanceByContractID (in string aContractID, in nsISupports aOuter, in nsIIDRef iid, out voidPtr result); */
  NS_IMETHOD CreateInstanceByContractID(const char *aContractID, nsISupports *aOuter, const nsIID & iid, void * *result) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPROXYCREATEINSTANCE \
  NS_IMETHOD CreateInstanceByIID(const nsIID & cid, nsISupports *aOuter, const nsIID & iid, void * *result); \
  NS_IMETHOD CreateInstanceByContractID(const char *aContractID, nsISupports *aOuter, const nsIID & iid, void * *result); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPROXYCREATEINSTANCE(_to) \
  NS_IMETHOD CreateInstanceByIID(const nsIID & cid, nsISupports *aOuter, const nsIID & iid, void * *result) { return _to CreateInstanceByIID(cid, aOuter, iid, result); } \
  NS_IMETHOD CreateInstanceByContractID(const char *aContractID, nsISupports *aOuter, const nsIID & iid, void * *result) { return _to CreateInstanceByContractID(aContractID, aOuter, iid, result); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPROXYCREATEINSTANCE(_to) \
  NS_IMETHOD CreateInstanceByIID(const nsIID & cid, nsISupports *aOuter, const nsIID & iid, void * *result) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateInstanceByIID(cid, aOuter, iid, result); } \
  NS_IMETHOD CreateInstanceByContractID(const char *aContractID, nsISupports *aOuter, const nsIID & iid, void * *result) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateInstanceByContractID(aContractID, aOuter, iid, result); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsProxyCreateInstance : public nsIProxyCreateInstance
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPROXYCREATEINSTANCE

  nsProxyCreateInstance();

private:
  ~nsProxyCreateInstance();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsProxyCreateInstance, nsIProxyCreateInstance)

nsProxyCreateInstance::nsProxyCreateInstance()
{
  /* member initializers and constructor code */
}

nsProxyCreateInstance::~nsProxyCreateInstance()
{
  /* destructor code */
}

/* [noscript] void CreateInstanceByIID (in nsIIDRef cid, in nsISupports aOuter, in nsIIDRef iid, out voidPtr result); */
NS_IMETHODIMP nsProxyCreateInstance::CreateInstanceByIID(const nsIID & cid, nsISupports *aOuter, const nsIID & iid, void * *result)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void CreateInstanceByContractID (in string aContractID, in nsISupports aOuter, in nsIIDRef iid, out voidPtr result); */
NS_IMETHODIMP nsProxyCreateInstance::CreateInstanceByContractID(const char *aContractID, nsISupports *aOuter, const nsIID & iid, void * *result)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIProxyCreateInstance_h__ */
