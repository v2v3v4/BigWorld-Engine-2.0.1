/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/protocol/about/public/nsIAboutModule.idl
 */

#ifndef __gen_nsIAboutModule_h__
#define __gen_nsIAboutModule_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsIChannel_h__
#include "nsIChannel.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIURI; /* forward declaration */

class nsILoadGroup; /* forward declaration */

class nsIInterfaceRequestor; /* forward declaration */

class nsIEventQueue; /* forward declaration */


/* starting interface:    nsIAboutModule */
#define NS_IABOUTMODULE_IID_STR "692303c0-2f83-11d3-8cd0-0060b0fc14a3"

#define NS_IABOUTMODULE_IID \
  {0x692303c0, 0x2f83, 0x11d3, \
    { 0x8c, 0xd0, 0x00, 0x60, 0xb0, 0xfc, 0x14, 0xa3 }}

class NS_NO_VTABLE nsIAboutModule : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IABOUTMODULE_IID)

  /**
     * Constructs a new channel for the about protocol module. 
     *
     * @param aURI the uri of the new channel
     */
  /* nsIChannel newChannel (in nsIURI aURI); */
  NS_IMETHOD NewChannel(nsIURI *aURI, nsIChannel **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIABOUTMODULE \
  NS_IMETHOD NewChannel(nsIURI *aURI, nsIChannel **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIABOUTMODULE(_to) \
  NS_IMETHOD NewChannel(nsIURI *aURI, nsIChannel **_retval) { return _to NewChannel(aURI, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIABOUTMODULE(_to) \
  NS_IMETHOD NewChannel(nsIURI *aURI, nsIChannel **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->NewChannel(aURI, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsAboutModule : public nsIAboutModule
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIABOUTMODULE

  nsAboutModule();

private:
  ~nsAboutModule();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsAboutModule, nsIAboutModule)

nsAboutModule::nsAboutModule()
{
  /* member initializers and constructor code */
}

nsAboutModule::~nsAboutModule()
{
  /* destructor code */
}

/* nsIChannel newChannel (in nsIURI aURI); */
NS_IMETHODIMP nsAboutModule::NewChannel(nsIURI *aURI, nsIChannel **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

#define NS_ABOUT_MODULE_CONTRACTID        "@mozilla.org/network/protocol/about;1" 
#define NS_ABOUT_MODULE_CONTRACTID_PREFIX NS_ABOUT_MODULE_CONTRACTID "?what=" 
#define NS_ABOUT_MODULE_CONTRACTID_LENGTH 49      // nsCRT::strlen(NS_ABOUT_MODULE_CONTRACTID_PREFIX)

#endif /* __gen_nsIAboutModule_h__ */
