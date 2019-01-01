/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/socket/base/nsISSLSocketControl.idl
 */

#ifndef __gen_nsISSLSocketControl_h__
#define __gen_nsISSLSocketControl_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIInterfaceRequestor; /* forward declaration */


/* starting interface:    nsISSLSocketControl */
#define NS_ISSLSOCKETCONTROL_IID_STR "8b3e8488-1dd2-11b2-b547-956290be347c"

#define NS_ISSLSOCKETCONTROL_IID \
  {0x8b3e8488, 0x1dd2, 0x11b2, \
    { 0xb5, 0x47, 0x95, 0x62, 0x90, 0xbe, 0x34, 0x7c }}

class NS_NO_VTABLE nsISSLSocketControl : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISSLSOCKETCONTROL_IID)

  /* attribute nsIInterfaceRequestor notificationCallbacks; */
  NS_IMETHOD GetNotificationCallbacks(nsIInterfaceRequestor * *aNotificationCallbacks) = 0;
  NS_IMETHOD SetNotificationCallbacks(nsIInterfaceRequestor * aNotificationCallbacks) = 0;

  /* attribute boolean forceHandshake; */
  NS_IMETHOD GetForceHandshake(PRBool *aForceHandshake) = 0;
  NS_IMETHOD SetForceHandshake(PRBool aForceHandshake) = 0;

  /* void proxyStartSSL (); */
  NS_IMETHOD ProxyStartSSL(void) = 0;

  /* void StartTLS (); */
  NS_IMETHOD StartTLS(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISSLSOCKETCONTROL \
  NS_IMETHOD GetNotificationCallbacks(nsIInterfaceRequestor * *aNotificationCallbacks); \
  NS_IMETHOD SetNotificationCallbacks(nsIInterfaceRequestor * aNotificationCallbacks); \
  NS_IMETHOD GetForceHandshake(PRBool *aForceHandshake); \
  NS_IMETHOD SetForceHandshake(PRBool aForceHandshake); \
  NS_IMETHOD ProxyStartSSL(void); \
  NS_IMETHOD StartTLS(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISSLSOCKETCONTROL(_to) \
  NS_IMETHOD GetNotificationCallbacks(nsIInterfaceRequestor * *aNotificationCallbacks) { return _to GetNotificationCallbacks(aNotificationCallbacks); } \
  NS_IMETHOD SetNotificationCallbacks(nsIInterfaceRequestor * aNotificationCallbacks) { return _to SetNotificationCallbacks(aNotificationCallbacks); } \
  NS_IMETHOD GetForceHandshake(PRBool *aForceHandshake) { return _to GetForceHandshake(aForceHandshake); } \
  NS_IMETHOD SetForceHandshake(PRBool aForceHandshake) { return _to SetForceHandshake(aForceHandshake); } \
  NS_IMETHOD ProxyStartSSL(void) { return _to ProxyStartSSL(); } \
  NS_IMETHOD StartTLS(void) { return _to StartTLS(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISSLSOCKETCONTROL(_to) \
  NS_IMETHOD GetNotificationCallbacks(nsIInterfaceRequestor * *aNotificationCallbacks) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNotificationCallbacks(aNotificationCallbacks); } \
  NS_IMETHOD SetNotificationCallbacks(nsIInterfaceRequestor * aNotificationCallbacks) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetNotificationCallbacks(aNotificationCallbacks); } \
  NS_IMETHOD GetForceHandshake(PRBool *aForceHandshake) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetForceHandshake(aForceHandshake); } \
  NS_IMETHOD SetForceHandshake(PRBool aForceHandshake) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetForceHandshake(aForceHandshake); } \
  NS_IMETHOD ProxyStartSSL(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ProxyStartSSL(); } \
  NS_IMETHOD StartTLS(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->StartTLS(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSSLSocketControl : public nsISSLSocketControl
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISSLSOCKETCONTROL

  nsSSLSocketControl();

private:
  ~nsSSLSocketControl();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSSLSocketControl, nsISSLSocketControl)

nsSSLSocketControl::nsSSLSocketControl()
{
  /* member initializers and constructor code */
}

nsSSLSocketControl::~nsSSLSocketControl()
{
  /* destructor code */
}

/* attribute nsIInterfaceRequestor notificationCallbacks; */
NS_IMETHODIMP nsSSLSocketControl::GetNotificationCallbacks(nsIInterfaceRequestor * *aNotificationCallbacks)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsSSLSocketControl::SetNotificationCallbacks(nsIInterfaceRequestor * aNotificationCallbacks)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean forceHandshake; */
NS_IMETHODIMP nsSSLSocketControl::GetForceHandshake(PRBool *aForceHandshake)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsSSLSocketControl::SetForceHandshake(PRBool aForceHandshake)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void proxyStartSSL (); */
NS_IMETHODIMP nsSSLSocketControl::ProxyStartSSL()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void StartTLS (); */
NS_IMETHODIMP nsSSLSocketControl::StartTLS()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISSLSocketControl_h__ */
