/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsISocketTransportService.idl
 */

#ifndef __gen_nsISocketTransportService_h__
#define __gen_nsISocketTransportService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsISocketTransport; /* forward declaration */

class nsIProxyInfo; /* forward declaration */


/* starting interface:    nsISocketTransportService */
#define NS_ISOCKETTRANSPORTSERVICE_IID_STR "7b19ac06-a5fb-11d9-9f82-0011246ecd24"

#define NS_ISOCKETTRANSPORTSERVICE_IID \
  {0x7b19ac06, 0xa5fb, 0x11d9, \
    { 0x9f, 0x82, 0x00, 0x11, 0x24, 0x6e, 0xcd, 0x24 }}

class NS_NO_VTABLE nsISocketTransportService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISOCKETTRANSPORTSERVICE_IID)

  /**
     * Creates a transport for a specified host and port.
     *
     * @param aSocketTypes
     *        array of socket type strings.  null if using default socket type.
     * @param aTypeCount
     *        specifies length of aSocketTypes.
     * @param aHost
     *        specifies the target hostname or IP address literal of the peer
     *        for this socket.
     * @param aPort
     *        specifies the target port of the peer for this socket.
     * @param aProxyInfo
     *        specifies the transport-layer proxy type to use.  null if no
     *        proxy.  used for communicating information about proxies like
     *        SOCKS (which are transparent to upper protocols).
     * 
     * @see nsIProxiedProtocolHandler
     * @see nsIProtocolProxyService::GetProxyInfo
     */
  /* nsISocketTransport createTransport ([array, size_is (aTypeCount)] in string aSocketTypes, in unsigned long aTypeCount, in AUTF8String aHost, in long aPort, in nsIProxyInfo aProxyInfo); */
  NS_IMETHOD CreateTransport(const char **aSocketTypes, PRUint32 aTypeCount, const nsACString & aHost, PRInt32 aPort, nsIProxyInfo *aProxyInfo, nsISocketTransport **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISOCKETTRANSPORTSERVICE \
  NS_IMETHOD CreateTransport(const char **aSocketTypes, PRUint32 aTypeCount, const nsACString & aHost, PRInt32 aPort, nsIProxyInfo *aProxyInfo, nsISocketTransport **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISOCKETTRANSPORTSERVICE(_to) \
  NS_IMETHOD CreateTransport(const char **aSocketTypes, PRUint32 aTypeCount, const nsACString & aHost, PRInt32 aPort, nsIProxyInfo *aProxyInfo, nsISocketTransport **_retval) { return _to CreateTransport(aSocketTypes, aTypeCount, aHost, aPort, aProxyInfo, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISOCKETTRANSPORTSERVICE(_to) \
  NS_IMETHOD CreateTransport(const char **aSocketTypes, PRUint32 aTypeCount, const nsACString & aHost, PRInt32 aPort, nsIProxyInfo *aProxyInfo, nsISocketTransport **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateTransport(aSocketTypes, aTypeCount, aHost, aPort, aProxyInfo, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSocketTransportService : public nsISocketTransportService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISOCKETTRANSPORTSERVICE

  nsSocketTransportService();

private:
  ~nsSocketTransportService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSocketTransportService, nsISocketTransportService)

nsSocketTransportService::nsSocketTransportService()
{
  /* member initializers and constructor code */
}

nsSocketTransportService::~nsSocketTransportService()
{
  /* destructor code */
}

/* nsISocketTransport createTransport ([array, size_is (aTypeCount)] in string aSocketTypes, in unsigned long aTypeCount, in AUTF8String aHost, in long aPort, in nsIProxyInfo aProxyInfo); */
NS_IMETHODIMP nsSocketTransportService::CreateTransport(const char **aSocketTypes, PRUint32 aTypeCount, const nsACString & aHost, PRInt32 aPort, nsIProxyInfo *aProxyInfo, nsISocketTransport **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISocketTransportService_h__ */
