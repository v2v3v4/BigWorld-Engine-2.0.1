/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsIURIChecker.idl
 */

#ifndef __gen_nsIURIChecker_h__
#define __gen_nsIURIChecker_h__


#ifndef __gen_nsIRequest_h__
#include "nsIRequest.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIURI; /* forward declaration */

class nsIChannel; /* forward declaration */

class nsIRequestObserver; /* forward declaration */


/* starting interface:    nsIURIChecker */
#define NS_IURICHECKER_IID_STR "4660c1a1-be2d-4c78-9baf-c22984176c28"

#define NS_IURICHECKER_IID \
  {0x4660c1a1, 0xbe2d, 0x4c78, \
    { 0x9b, 0xaf, 0xc2, 0x29, 0x84, 0x17, 0x6c, 0x28 }}

/**
 * nsIURIChecker
 *
 * The URI checker is a component that can be used to verify the existance
 * of a resource at the location specified by a given URI.  It will use
 * protocol specific methods to verify the URI (e.g., use of HEAD request
 * for HTTP URIs).
 */
class NS_NO_VTABLE nsIURIChecker : public nsIRequest {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IURICHECKER_IID)

  /**
     * Initializes the URI checker.  After this method is called, it is valid
     * to further configure the URI checker by calling its nsIRequest methods.
     * This method creates the channel that will be used to verify the URI.
     * In the case of the HTTP protocol, only a HEAD request will be issued.
     *
     * @param aURI
     *        The URI to be checked.
     */
  /* void init (in nsIURI aURI); */
  NS_IMETHOD Init(nsIURI *aURI) = 0;

  /**
     * Returns the base channel that will be used to verify the URI.
     */
  /* readonly attribute nsIChannel baseChannel; */
  NS_IMETHOD GetBaseChannel(nsIChannel * *aBaseChannel) = 0;

  /**
     * Begin asynchronous checking URI for validity.  Notification will be
     * asynchronous through the nsIRequestObserver callback interface.  When
     * OnStartRequest is fired, the baseChannel attribute will have been
     * updated to reflect the final channel used (corresponding to any redirects
     * that may have been followed).
     *
     * Our interpretations of the nsIRequestObserver status codes:
     *   NS_BINDING_SUCCEEDED:   link is valid
     *   NS_BINDING_FAILED:      link is invalid (gave an error)
     *   NS_BINDING_ABORTED:     timed out, or cancelled
     *
     * @param aObserver
     *        The object to notify when the link is verified.  We will
     *        call aObserver.OnStartRequest followed immediately by
     *        aObserver.OnStopRequest.  It is recommended that the caller use
     *        OnStopRequest to act on the link's status.  The underlying request
     *        will not be cancelled until after OnStopRequest has been called.
     * @param aContext
     *        A closure that will be passed back to the nsIRequestObserver
     *        methods.
     */
  /* void asyncCheck (in nsIRequestObserver aObserver, in nsISupports aContext); */
  NS_IMETHOD AsyncCheck(nsIRequestObserver *aObserver, nsISupports *aContext) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIURICHECKER \
  NS_IMETHOD Init(nsIURI *aURI); \
  NS_IMETHOD GetBaseChannel(nsIChannel * *aBaseChannel); \
  NS_IMETHOD AsyncCheck(nsIRequestObserver *aObserver, nsISupports *aContext); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIURICHECKER(_to) \
  NS_IMETHOD Init(nsIURI *aURI) { return _to Init(aURI); } \
  NS_IMETHOD GetBaseChannel(nsIChannel * *aBaseChannel) { return _to GetBaseChannel(aBaseChannel); } \
  NS_IMETHOD AsyncCheck(nsIRequestObserver *aObserver, nsISupports *aContext) { return _to AsyncCheck(aObserver, aContext); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIURICHECKER(_to) \
  NS_IMETHOD Init(nsIURI *aURI) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(aURI); } \
  NS_IMETHOD GetBaseChannel(nsIChannel * *aBaseChannel) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBaseChannel(aBaseChannel); } \
  NS_IMETHOD AsyncCheck(nsIRequestObserver *aObserver, nsISupports *aContext) { return !_to ? NS_ERROR_NULL_POINTER : _to->AsyncCheck(aObserver, aContext); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsURIChecker : public nsIURIChecker
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIURICHECKER

  nsURIChecker();

private:
  ~nsURIChecker();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsURIChecker, nsIURIChecker)

nsURIChecker::nsURIChecker()
{
  /* member initializers and constructor code */
}

nsURIChecker::~nsURIChecker()
{
  /* destructor code */
}

/* void init (in nsIURI aURI); */
NS_IMETHODIMP nsURIChecker::Init(nsIURI *aURI)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIChannel baseChannel; */
NS_IMETHODIMP nsURIChecker::GetBaseChannel(nsIChannel * *aBaseChannel)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void asyncCheck (in nsIRequestObserver aObserver, in nsISupports aContext); */
NS_IMETHODIMP nsURIChecker::AsyncCheck(nsIRequestObserver *aObserver, nsISupports *aContext)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIURIChecker_h__ */
