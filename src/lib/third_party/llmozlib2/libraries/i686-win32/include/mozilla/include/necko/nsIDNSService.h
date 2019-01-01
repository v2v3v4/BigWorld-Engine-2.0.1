/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/dns/public/nsIDNSService.idl
 */

#ifndef __gen_nsIDNSService_h__
#define __gen_nsIDNSService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsICancelable; /* forward declaration */

class nsIEventTarget; /* forward declaration */

class nsIDNSRecord; /* forward declaration */

class nsIDNSListener; /* forward declaration */


/* starting interface:    nsIDNSService */
#define NS_IDNSSERVICE_IID_STR "5c8ec09d-bfbf-4eaf-8a36-0d84b5c8f35b"

#define NS_IDNSSERVICE_IID \
  {0x5c8ec09d, 0xbfbf, 0x4eaf, \
    { 0x8a, 0x36, 0x0d, 0x84, 0xb5, 0xc8, 0xf3, 0x5b }}

/**
 * nsIDNSService
 */
class NS_NO_VTABLE nsIDNSService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDNSSERVICE_IID)

  /**
     * kicks off an asynchronous host lookup.
     *
     * @param aHostName
     *        the hostname or IP-address-literal to resolve.
     * @param aFlags
     *        a bitwise OR of the RESOLVE_ prefixed constants defined below.
     * @param aListener
     *        the listener to be notified when the result is available.
     * @param aListenerEventTarget
     *        optional parameter (may be null).  if non-null, this parameter
     *        specifies the nsIEventTarget of the thread on which the listener's
     *        onLookupComplete should be called.  however, if this parameter is
     *        null, then onLookupComplete will be called on an unspecified
     *        thread (possibly recursively).
     *
     * @return An object that can be used to cancel the host lookup.
     */
  /* nsICancelable asyncResolve (in AUTF8String aHostName, in unsigned long aFlags, in nsIDNSListener aListener, in nsIEventTarget aListenerEventTarget); */
  NS_IMETHOD AsyncResolve(const nsACString & aHostName, PRUint32 aFlags, nsIDNSListener *aListener, nsIEventTarget *aListenerEventTarget, nsICancelable **_retval) = 0;

  /**
     * called to synchronously resolve a hostname.  warning this method may
     * block the calling thread for a long period of time.  it is extremely
     * unwise to call this function on the UI thread of an application.
     *
     * @param aHostName
     *        the hostname or IP-address-literal to resolve.
     * @param aFlags
     *        a bitwise OR of the RESOLVE_ prefixed constants defined below.
     *
     * @return DNS record corresponding to the given hostname.
     * @throws NS_ERROR_UNKNOWN_HOST if host could not be resolved.
     */
  /* nsIDNSRecord resolve (in AUTF8String aHostName, in unsigned long aFlags); */
  NS_IMETHOD Resolve(const nsACString & aHostName, PRUint32 aFlags, nsIDNSRecord **_retval) = 0;

  /**
     * @return the hostname of the operating system.
     */
  /* readonly attribute AUTF8String myHostName; */
  NS_IMETHOD GetMyHostName(nsACString & aMyHostName) = 0;

  /*************************************************************************
     * Listed below are the various flags that may be OR'd together to form
     * the aFlags parameter passed to asyncResolve() and resolve().
     */
/**
     * if set, this flag suppresses the internal DNS lookup cache.
     */
  enum { RESOLVE_BYPASS_CACHE = 1U };

  /**
     * if set, the canonical name of the specified host will be queried.
     */
  enum { RESOLVE_CANONICAL_NAME = 2U };

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDNSSERVICE \
  NS_IMETHOD AsyncResolve(const nsACString & aHostName, PRUint32 aFlags, nsIDNSListener *aListener, nsIEventTarget *aListenerEventTarget, nsICancelable **_retval); \
  NS_IMETHOD Resolve(const nsACString & aHostName, PRUint32 aFlags, nsIDNSRecord **_retval); \
  NS_IMETHOD GetMyHostName(nsACString & aMyHostName); \

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDNSSERVICE(_to) \
  NS_IMETHOD AsyncResolve(const nsACString & aHostName, PRUint32 aFlags, nsIDNSListener *aListener, nsIEventTarget *aListenerEventTarget, nsICancelable **_retval) { return _to AsyncResolve(aHostName, aFlags, aListener, aListenerEventTarget, _retval); } \
  NS_IMETHOD Resolve(const nsACString & aHostName, PRUint32 aFlags, nsIDNSRecord **_retval) { return _to Resolve(aHostName, aFlags, _retval); } \
  NS_IMETHOD GetMyHostName(nsACString & aMyHostName) { return _to GetMyHostName(aMyHostName); } \

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDNSSERVICE(_to) \
  NS_IMETHOD AsyncResolve(const nsACString & aHostName, PRUint32 aFlags, nsIDNSListener *aListener, nsIEventTarget *aListenerEventTarget, nsICancelable **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->AsyncResolve(aHostName, aFlags, aListener, aListenerEventTarget, _retval); } \
  NS_IMETHOD Resolve(const nsACString & aHostName, PRUint32 aFlags, nsIDNSRecord **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Resolve(aHostName, aFlags, _retval); } \
  NS_IMETHOD GetMyHostName(nsACString & aMyHostName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMyHostName(aMyHostName); } \

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDNSService : public nsIDNSService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDNSSERVICE

  nsDNSService();

private:
  ~nsDNSService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDNSService, nsIDNSService)

nsDNSService::nsDNSService()
{
  /* member initializers and constructor code */
}

nsDNSService::~nsDNSService()
{
  /* destructor code */
}

/* nsICancelable asyncResolve (in AUTF8String aHostName, in unsigned long aFlags, in nsIDNSListener aListener, in nsIEventTarget aListenerEventTarget); */
NS_IMETHODIMP nsDNSService::AsyncResolve(const nsACString & aHostName, PRUint32 aFlags, nsIDNSListener *aListener, nsIEventTarget *aListenerEventTarget, nsICancelable **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDNSRecord resolve (in AUTF8String aHostName, in unsigned long aFlags); */
NS_IMETHODIMP nsDNSService::Resolve(const nsACString & aHostName, PRUint32 aFlags, nsIDNSRecord **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute AUTF8String myHostName; */
NS_IMETHODIMP nsDNSService::GetMyHostName(nsACString & aMyHostName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDNSService_h__ */
