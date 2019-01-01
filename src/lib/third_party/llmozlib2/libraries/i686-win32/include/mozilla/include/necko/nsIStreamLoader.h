/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsIStreamLoader.idl
 */

#ifndef __gen_nsIStreamLoader_h__
#define __gen_nsIStreamLoader_h__


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
class nsIRequest; /* forward declaration */

class nsIURI; /* forward declaration */

class nsILoadGroup; /* forward declaration */

class nsIStreamLoader; /* forward declaration */

class nsIInterfaceRequestor; /* forward declaration */


/* starting interface:    nsIStreamLoaderObserver */
#define NS_ISTREAMLOADEROBSERVER_IID_STR "359f7990-d4e9-11d3-a1a5-0050041caf44"

#define NS_ISTREAMLOADEROBSERVER_IID \
  {0x359f7990, 0xd4e9, 0x11d3, \
    { 0xa1, 0xa5, 0x00, 0x50, 0x04, 0x1c, 0xaf, 0x44 }}

class NS_NO_VTABLE nsIStreamLoaderObserver : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISTREAMLOADEROBSERVER_IID)

  /* void onStreamComplete (in nsIStreamLoader loader, in nsISupports ctxt, in nsresult status, in unsigned long resultLength, [array, size_is (resultLength), const] in octet result); */
  NS_IMETHOD OnStreamComplete(nsIStreamLoader *loader, nsISupports *ctxt, nsresult status, PRUint32 resultLength, const PRUint8 *result) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISTREAMLOADEROBSERVER \
  NS_IMETHOD OnStreamComplete(nsIStreamLoader *loader, nsISupports *ctxt, nsresult status, PRUint32 resultLength, const PRUint8 *result); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISTREAMLOADEROBSERVER(_to) \
  NS_IMETHOD OnStreamComplete(nsIStreamLoader *loader, nsISupports *ctxt, nsresult status, PRUint32 resultLength, const PRUint8 *result) { return _to OnStreamComplete(loader, ctxt, status, resultLength, result); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISTREAMLOADEROBSERVER(_to) \
  NS_IMETHOD OnStreamComplete(nsIStreamLoader *loader, nsISupports *ctxt, nsresult status, PRUint32 resultLength, const PRUint8 *result) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnStreamComplete(loader, ctxt, status, resultLength, result); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsStreamLoaderObserver : public nsIStreamLoaderObserver
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISTREAMLOADEROBSERVER

  nsStreamLoaderObserver();

private:
  ~nsStreamLoaderObserver();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsStreamLoaderObserver, nsIStreamLoaderObserver)

nsStreamLoaderObserver::nsStreamLoaderObserver()
{
  /* member initializers and constructor code */
}

nsStreamLoaderObserver::~nsStreamLoaderObserver()
{
  /* destructor code */
}

/* void onStreamComplete (in nsIStreamLoader loader, in nsISupports ctxt, in nsresult status, in unsigned long resultLength, [array, size_is (resultLength), const] in octet result); */
NS_IMETHODIMP nsStreamLoaderObserver::OnStreamComplete(nsIStreamLoader *loader, nsISupports *ctxt, nsresult status, PRUint32 resultLength, const PRUint8 *result)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIStreamLoader */
#define NS_ISTREAMLOADER_IID_STR "31d37360-8e5a-11d3-93ad-00104ba0fd40"

#define NS_ISTREAMLOADER_IID \
  {0x31d37360, 0x8e5a, 0x11d3, \
    { 0x93, 0xad, 0x00, 0x10, 0x4b, 0xa0, 0xfd, 0x40 }}

class NS_NO_VTABLE nsIStreamLoader : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISTREAMLOADER_IID)

  /**
 * Asynchronously loads a channel into a memory buffer.
 *
 * XXX define behaviour for sizes >4 GB
 */
/**
     * Initialize this stream loader, and start loading the data.
     *
     * @param aChannel
     *        A Channel to load data from. This must not be asyncOpen'd yet!
     * @param aObserver
     *        An observer that will be notified when the data is complete.
     * @param aContext
     *        May be null. Will be passed to the observer.
     *
     * @note Failure to open the channel will be indicated by an async callback
     *       to the observer.
     */
  /* void init (in nsIChannel aChannel, in nsIStreamLoaderObserver aObserver, in nsISupports aContext); */
  NS_IMETHOD Init(nsIChannel *aChannel, nsIStreamLoaderObserver *aObserver, nsISupports *aContext) = 0;

  /**
     * Gets the number of bytes read so far.
     */
  /* readonly attribute unsigned long numBytesRead; */
  NS_IMETHOD GetNumBytesRead(PRUint32 *aNumBytesRead) = 0;

  /**
     * Gets the request that loaded this file.
     * null after the request has finished loading.
     */
  /* readonly attribute nsIRequest request; */
  NS_IMETHOD GetRequest(nsIRequest * *aRequest) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISTREAMLOADER \
  NS_IMETHOD Init(nsIChannel *aChannel, nsIStreamLoaderObserver *aObserver, nsISupports *aContext); \
  NS_IMETHOD GetNumBytesRead(PRUint32 *aNumBytesRead); \
  NS_IMETHOD GetRequest(nsIRequest * *aRequest); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISTREAMLOADER(_to) \
  NS_IMETHOD Init(nsIChannel *aChannel, nsIStreamLoaderObserver *aObserver, nsISupports *aContext) { return _to Init(aChannel, aObserver, aContext); } \
  NS_IMETHOD GetNumBytesRead(PRUint32 *aNumBytesRead) { return _to GetNumBytesRead(aNumBytesRead); } \
  NS_IMETHOD GetRequest(nsIRequest * *aRequest) { return _to GetRequest(aRequest); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISTREAMLOADER(_to) \
  NS_IMETHOD Init(nsIChannel *aChannel, nsIStreamLoaderObserver *aObserver, nsISupports *aContext) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(aChannel, aObserver, aContext); } \
  NS_IMETHOD GetNumBytesRead(PRUint32 *aNumBytesRead) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNumBytesRead(aNumBytesRead); } \
  NS_IMETHOD GetRequest(nsIRequest * *aRequest) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRequest(aRequest); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsStreamLoader : public nsIStreamLoader
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISTREAMLOADER

  nsStreamLoader();

private:
  ~nsStreamLoader();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsStreamLoader, nsIStreamLoader)

nsStreamLoader::nsStreamLoader()
{
  /* member initializers and constructor code */
}

nsStreamLoader::~nsStreamLoader()
{
  /* destructor code */
}

/* void init (in nsIChannel aChannel, in nsIStreamLoaderObserver aObserver, in nsISupports aContext); */
NS_IMETHODIMP nsStreamLoader::Init(nsIChannel *aChannel, nsIStreamLoaderObserver *aObserver, nsISupports *aContext)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long numBytesRead; */
NS_IMETHODIMP nsStreamLoader::GetNumBytesRead(PRUint32 *aNumBytesRead)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIRequest request; */
NS_IMETHODIMP nsStreamLoader::GetRequest(nsIRequest * *aRequest)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIStreamLoader_h__ */
