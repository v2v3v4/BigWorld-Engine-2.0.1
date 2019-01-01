/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/uriloader/base/nsIURILoader.idl
 */

#ifndef __gen_nsIURILoader_h__
#define __gen_nsIURILoader_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIURIContentListener; /* forward declaration */

class nsIURI; /* forward declaration */

class nsILoadGroup; /* forward declaration */

class nsIProgressEventSink; /* forward declaration */

class nsIChannel; /* forward declaration */

class nsIRequest; /* forward declaration */

class nsIStreamListener; /* forward declaration */

class nsIInputStream; /* forward declaration */

class nsIInterfaceRequestor; /* forward declaration */


/* starting interface:    nsIURILoader */
#define NS_IURILOADER_IID_STR "5cf6420c-74f3-4a7c-bc1d-f5756d79ea07"

#define NS_IURILOADER_IID \
  {0x5cf6420c, 0x74f3, 0x4a7c, \
    { 0xbc, 0x1d, 0xf5, 0x75, 0x6d, 0x79, 0xea, 0x07 }}

/**
 * The uri dispatcher is responsible for taking uri's, determining
 * the content and routing the opened url to the correct content 
 * handler. 
 *
 * When you encounter a url you want to open, you typically call 
 * openURI, passing it the content listener for the window the uri is 
 * originating from. The uri dispatcher opens the url to discover the 
 * content type. It then gives the content listener first crack at 
 * handling the content. If it doesn't want it, the dispatcher tries
 * to hand it off one of the registered content listeners. This allows
 * running applications the chance to jump in and handle the content.
 *
 * If that also fails, then the uri dispatcher goes to the registry
 * looking for the preferred content handler for the content type
 * of the uri. The content handler may create an app instance
 * or it may hand the contents off to a platform specific plugin
 * or helper app. Or it may hand the url off to an OS registered 
 * application. 
 */
class NS_NO_VTABLE nsIURILoader : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IURILOADER_IID)

  /**
   * As applications such as messenger and the browser are instantiated,
   * they register content listener's with the uri dispatcher corresponding
   * to content windows within that application. 
   *
   * Note to self: we may want to optimize things a bit more by requiring
   * the content types the registered content listener cares about.
   *
   * @param aContentListener
   *        The listener to register. This listener must implement
   *        nsISupportsWeakReference.
   *
   * @see the nsIURILoader class description
   */
  /* void registerContentListener (in nsIURIContentListener aContentListener); */
  NS_IMETHOD RegisterContentListener(nsIURIContentListener *aContentListener) = 0;

  /* void unRegisterContentListener (in nsIURIContentListener aContentListener); */
  NS_IMETHOD UnRegisterContentListener(nsIURIContentListener *aContentListener) = 0;

  /**
   * OpenURI requires the following parameters.....
   * @param aChannel
   *        The channel that should be opened. This must not be asyncOpen'd yet!
   *        If a loadgroup is set on the channel, it will get replaced with a
   *        different one.
   * @param aIsContentPreferred
   *        Should the content be displayed in a container that prefers the
   *        content-type, or will any container do.
   * @param aWindowContext
   *        If you are running the url from a doc shell or a web shell, this is
   *        your window context. If you have a content listener you want to
   *        give first crack to, the uri loader needs to be able to get it
   *        from the window context. We will also be using the window context
   *        to get at the progress event sink interface.
   *        <b>Must not be null!</b>
   */
  /* void openURI (in nsIChannel aChannel, in boolean aIsContentPreferred, in nsIInterfaceRequestor aWindowContext); */
  NS_IMETHOD OpenURI(nsIChannel *aChannel, PRBool aIsContentPreferred, nsIInterfaceRequestor *aWindowContext) = 0;

  /**
   * Stops an in progress load
   */
  /* void stop (in nsISupports aLoadCookie); */
  NS_IMETHOD Stop(nsISupports *aLoadCookie) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIURILOADER \
  NS_IMETHOD RegisterContentListener(nsIURIContentListener *aContentListener); \
  NS_IMETHOD UnRegisterContentListener(nsIURIContentListener *aContentListener); \
  NS_IMETHOD OpenURI(nsIChannel *aChannel, PRBool aIsContentPreferred, nsIInterfaceRequestor *aWindowContext); \
  NS_IMETHOD Stop(nsISupports *aLoadCookie); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIURILOADER(_to) \
  NS_IMETHOD RegisterContentListener(nsIURIContentListener *aContentListener) { return _to RegisterContentListener(aContentListener); } \
  NS_IMETHOD UnRegisterContentListener(nsIURIContentListener *aContentListener) { return _to UnRegisterContentListener(aContentListener); } \
  NS_IMETHOD OpenURI(nsIChannel *aChannel, PRBool aIsContentPreferred, nsIInterfaceRequestor *aWindowContext) { return _to OpenURI(aChannel, aIsContentPreferred, aWindowContext); } \
  NS_IMETHOD Stop(nsISupports *aLoadCookie) { return _to Stop(aLoadCookie); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIURILOADER(_to) \
  NS_IMETHOD RegisterContentListener(nsIURIContentListener *aContentListener) { return !_to ? NS_ERROR_NULL_POINTER : _to->RegisterContentListener(aContentListener); } \
  NS_IMETHOD UnRegisterContentListener(nsIURIContentListener *aContentListener) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnRegisterContentListener(aContentListener); } \
  NS_IMETHOD OpenURI(nsIChannel *aChannel, PRBool aIsContentPreferred, nsIInterfaceRequestor *aWindowContext) { return !_to ? NS_ERROR_NULL_POINTER : _to->OpenURI(aChannel, aIsContentPreferred, aWindowContext); } \
  NS_IMETHOD Stop(nsISupports *aLoadCookie) { return !_to ? NS_ERROR_NULL_POINTER : _to->Stop(aLoadCookie); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsURILoader : public nsIURILoader
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIURILOADER

  nsURILoader();

private:
  ~nsURILoader();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsURILoader, nsIURILoader)

nsURILoader::nsURILoader()
{
  /* member initializers and constructor code */
}

nsURILoader::~nsURILoader()
{
  /* destructor code */
}

/* void registerContentListener (in nsIURIContentListener aContentListener); */
NS_IMETHODIMP nsURILoader::RegisterContentListener(nsIURIContentListener *aContentListener)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unRegisterContentListener (in nsIURIContentListener aContentListener); */
NS_IMETHODIMP nsURILoader::UnRegisterContentListener(nsIURIContentListener *aContentListener)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void openURI (in nsIChannel aChannel, in boolean aIsContentPreferred, in nsIInterfaceRequestor aWindowContext); */
NS_IMETHODIMP nsURILoader::OpenURI(nsIChannel *aChannel, PRBool aIsContentPreferred, nsIInterfaceRequestor *aWindowContext)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void stop (in nsISupports aLoadCookie); */
NS_IMETHODIMP nsURILoader::Stop(nsISupports *aLoadCookie)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIURILoader_h__ */
