/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsPIProtocolProxyService.idl
 */

#ifndef __gen_nsPIProtocolProxyService_h__
#define __gen_nsPIProtocolProxyService_h__


#ifndef __gen_nsIProtocolProxyService_h__
#include "nsIProtocolProxyService.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsPIProtocolProxyService */
#define NS_PIPROTOCOLPROXYSERVICE_IID_STR "d2c7b3eb-7778-468b-ae9b-c106c2afb5d1"

#define NS_PIPROTOCOLPROXYSERVICE_IID \
  {0xd2c7b3eb, 0x7778, 0x468b, \
    { 0xae, 0x9b, 0xc1, 0x06, 0xc2, 0xaf, 0xb5, 0xd1 }}

/**
 * THIS IS A PRIVATE INTERFACE
 *
 * It exists purely as a hack to support the configureFromPAC method used by
 * the preference panels in the various apps.  Those apps need to be taught to
 * just use the preferences API to "reload" the PAC file.  Then, at that point,
 * we can eliminate this interface completely.
 */
class NS_NO_VTABLE nsPIProtocolProxyService : public nsIProtocolProxyService {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_PIPROTOCOLPROXYSERVICE_IID)

  /**
     * This method may be called to re-configure proxy settings given a URI
     * to a new proxy auto config file.  This method may return before the
     * configuration actually takes affect (i.e., the URI may be loaded
     * asynchronously).
     *
     * WARNING: This method is considered harmful since it may cause the PAC
     * preferences to be out of sync with the state of the Protocol Proxy
     * Service.  This method is going to be eliminated in the near future.
     *
     * @param aURI
     *        The location of the PAC file to load.  If this value is empty,
     *        then the PAC configuration will be removed.
     */
  /* void configureFromPAC (in AUTF8String aURI); */
  NS_IMETHOD ConfigureFromPAC(const nsACString & aURI) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSPIPROTOCOLPROXYSERVICE \
  NS_IMETHOD ConfigureFromPAC(const nsACString & aURI); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSPIPROTOCOLPROXYSERVICE(_to) \
  NS_IMETHOD ConfigureFromPAC(const nsACString & aURI) { return _to ConfigureFromPAC(aURI); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSPIPROTOCOLPROXYSERVICE(_to) \
  NS_IMETHOD ConfigureFromPAC(const nsACString & aURI) { return !_to ? NS_ERROR_NULL_POINTER : _to->ConfigureFromPAC(aURI); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public nsPIProtocolProxyService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSPIPROTOCOLPROXYSERVICE

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(_MYCLASS_, nsPIProtocolProxyService)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void configureFromPAC (in AUTF8String aURI); */
NS_IMETHODIMP _MYCLASS_::ConfigureFromPAC(const nsACString & aURI)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsPIProtocolProxyService_h__ */
