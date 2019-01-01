/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsIUnicharStreamLoader.idl
 */

#ifndef __gen_nsIUnicharStreamLoader_h__
#define __gen_nsIUnicharStreamLoader_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIUnicharInputStream; /* forward declaration */

class nsIUnicharStreamLoader; /* forward declaration */

class nsIChannel; /* forward declaration */


/* starting interface:    nsIUnicharStreamLoaderObserver */
#define NS_IUNICHARSTREAMLOADEROBSERVER_IID_STR "e06e8b08-8cdd-4503-a0a0-6f3b943602af"

#define NS_IUNICHARSTREAMLOADEROBSERVER_IID \
  {0xe06e8b08, 0x8cdd, 0x4503, \
    { 0xa0, 0xa0, 0x6f, 0x3b, 0x94, 0x36, 0x02, 0xaf }}

class NS_NO_VTABLE nsIUnicharStreamLoaderObserver : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IUNICHARSTREAMLOADEROBSERVER_IID)

  /**
   * Called when the first full segment of data if available.
   *
   * @param aLoader the unichar stream loader
   * @param aContext the aContext parameter passed to the loader's init method
   * @param aFirstSegment the raw bytes of the first full data segment
   * @param aLength the length of aFirstSegment
   *
   * @return charset corresponding to this stream
   */
  /* ACString onDetermineCharset (in nsIUnicharStreamLoader aLoader, in nsISupports aContext, [size_is (aLength)] in string aFirstSegment, in unsigned long aLength); */
  NS_IMETHOD OnDetermineCharset(nsIUnicharStreamLoader *aLoader, nsISupports *aContext, const char *aFirstSegment, PRUint32 aLength, nsACString & _retval) = 0;

  /**
   * Called when the entire stream has been loaded.
   *
   * @param aLoader the unichar stream loader
   * @param aContext the aContext parameter passed to the loader's init method
   * @param aStatus the status of the underlying channel
   * @param aUnicharData the unichar input stream containing the data.  This
   *        can be null in some failure conditions.
   */
  /* void onStreamComplete (in nsIUnicharStreamLoader aLoader, in nsISupports aContext, in nsresult aStatus, in nsIUnicharInputStream aUnicharData); */
  NS_IMETHOD OnStreamComplete(nsIUnicharStreamLoader *aLoader, nsISupports *aContext, nsresult aStatus, nsIUnicharInputStream *aUnicharData) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIUNICHARSTREAMLOADEROBSERVER \
  NS_IMETHOD OnDetermineCharset(nsIUnicharStreamLoader *aLoader, nsISupports *aContext, const char *aFirstSegment, PRUint32 aLength, nsACString & _retval); \
  NS_IMETHOD OnStreamComplete(nsIUnicharStreamLoader *aLoader, nsISupports *aContext, nsresult aStatus, nsIUnicharInputStream *aUnicharData); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIUNICHARSTREAMLOADEROBSERVER(_to) \
  NS_IMETHOD OnDetermineCharset(nsIUnicharStreamLoader *aLoader, nsISupports *aContext, const char *aFirstSegment, PRUint32 aLength, nsACString & _retval) { return _to OnDetermineCharset(aLoader, aContext, aFirstSegment, aLength, _retval); } \
  NS_IMETHOD OnStreamComplete(nsIUnicharStreamLoader *aLoader, nsISupports *aContext, nsresult aStatus, nsIUnicharInputStream *aUnicharData) { return _to OnStreamComplete(aLoader, aContext, aStatus, aUnicharData); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIUNICHARSTREAMLOADEROBSERVER(_to) \
  NS_IMETHOD OnDetermineCharset(nsIUnicharStreamLoader *aLoader, nsISupports *aContext, const char *aFirstSegment, PRUint32 aLength, nsACString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnDetermineCharset(aLoader, aContext, aFirstSegment, aLength, _retval); } \
  NS_IMETHOD OnStreamComplete(nsIUnicharStreamLoader *aLoader, nsISupports *aContext, nsresult aStatus, nsIUnicharInputStream *aUnicharData) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnStreamComplete(aLoader, aContext, aStatus, aUnicharData); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsUnicharStreamLoaderObserver : public nsIUnicharStreamLoaderObserver
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIUNICHARSTREAMLOADEROBSERVER

  nsUnicharStreamLoaderObserver();

private:
  ~nsUnicharStreamLoaderObserver();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsUnicharStreamLoaderObserver, nsIUnicharStreamLoaderObserver)

nsUnicharStreamLoaderObserver::nsUnicharStreamLoaderObserver()
{
  /* member initializers and constructor code */
}

nsUnicharStreamLoaderObserver::~nsUnicharStreamLoaderObserver()
{
  /* destructor code */
}

/* ACString onDetermineCharset (in nsIUnicharStreamLoader aLoader, in nsISupports aContext, [size_is (aLength)] in string aFirstSegment, in unsigned long aLength); */
NS_IMETHODIMP nsUnicharStreamLoaderObserver::OnDetermineCharset(nsIUnicharStreamLoader *aLoader, nsISupports *aContext, const char *aFirstSegment, PRUint32 aLength, nsACString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onStreamComplete (in nsIUnicharStreamLoader aLoader, in nsISupports aContext, in nsresult aStatus, in nsIUnicharInputStream aUnicharData); */
NS_IMETHODIMP nsUnicharStreamLoaderObserver::OnStreamComplete(nsIUnicharStreamLoader *aLoader, nsISupports *aContext, nsresult aStatus, nsIUnicharInputStream *aUnicharData)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIUnicharStreamLoader */
#define NS_IUNICHARSTREAMLOADER_IID_STR "8a3eca16-167e-443d-9485-7e84ed822e95"

#define NS_IUNICHARSTREAMLOADER_IID \
  {0x8a3eca16, 0x167e, 0x443d, \
    { 0x94, 0x85, 0x7e, 0x84, 0xed, 0x82, 0x2e, 0x95 }}

class NS_NO_VTABLE nsIUnicharStreamLoader : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IUNICHARSTREAMLOADER_IID)

  enum { DEFAULT_SEGMENT_SIZE = 4096U };

  /**
   * Initializes the unichar stream loader
   *
   * @param aChannel the channel to read data from.  This should _not_ be
   *        opened; the loader will open the channel itself.
   * @param aObserver the observer to notify when a charset is needed and when
   *        the load is complete
   * @param aContext an opaque context pointer
   * @param aSegmentSize the size of the segments to use for the data, in bytes
   */
  /* void init (in nsIChannel aChannel, in nsIUnicharStreamLoaderObserver aObserver, in nsISupports aContext, in unsigned long aSegmentSize); */
  NS_IMETHOD Init(nsIChannel *aChannel, nsIUnicharStreamLoaderObserver *aObserver, nsISupports *aContext, PRUint32 aSegmentSize) = 0;

  /**
   * The channel attribute is only valid inside the onDetermineCharset
   * and onStreamComplete callbacks.  Otherwise it will be null.
   */
  /* readonly attribute nsIChannel channel; */
  NS_IMETHOD GetChannel(nsIChannel * *aChannel) = 0;

  /**
   * The charset that onDetermineCharset returned, if that's been
   * called.
   */
  /* readonly attribute ACString charset; */
  NS_IMETHOD GetCharset(nsACString & aCharset) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIUNICHARSTREAMLOADER \
  NS_IMETHOD Init(nsIChannel *aChannel, nsIUnicharStreamLoaderObserver *aObserver, nsISupports *aContext, PRUint32 aSegmentSize); \
  NS_IMETHOD GetChannel(nsIChannel * *aChannel); \
  NS_IMETHOD GetCharset(nsACString & aCharset); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIUNICHARSTREAMLOADER(_to) \
  NS_IMETHOD Init(nsIChannel *aChannel, nsIUnicharStreamLoaderObserver *aObserver, nsISupports *aContext, PRUint32 aSegmentSize) { return _to Init(aChannel, aObserver, aContext, aSegmentSize); } \
  NS_IMETHOD GetChannel(nsIChannel * *aChannel) { return _to GetChannel(aChannel); } \
  NS_IMETHOD GetCharset(nsACString & aCharset) { return _to GetCharset(aCharset); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIUNICHARSTREAMLOADER(_to) \
  NS_IMETHOD Init(nsIChannel *aChannel, nsIUnicharStreamLoaderObserver *aObserver, nsISupports *aContext, PRUint32 aSegmentSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(aChannel, aObserver, aContext, aSegmentSize); } \
  NS_IMETHOD GetChannel(nsIChannel * *aChannel) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetChannel(aChannel); } \
  NS_IMETHOD GetCharset(nsACString & aCharset) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCharset(aCharset); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsUnicharStreamLoader : public nsIUnicharStreamLoader
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIUNICHARSTREAMLOADER

  nsUnicharStreamLoader();

private:
  ~nsUnicharStreamLoader();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsUnicharStreamLoader, nsIUnicharStreamLoader)

nsUnicharStreamLoader::nsUnicharStreamLoader()
{
  /* member initializers and constructor code */
}

nsUnicharStreamLoader::~nsUnicharStreamLoader()
{
  /* destructor code */
}

/* void init (in nsIChannel aChannel, in nsIUnicharStreamLoaderObserver aObserver, in nsISupports aContext, in unsigned long aSegmentSize); */
NS_IMETHODIMP nsUnicharStreamLoader::Init(nsIChannel *aChannel, nsIUnicharStreamLoaderObserver *aObserver, nsISupports *aContext, PRUint32 aSegmentSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIChannel channel; */
NS_IMETHODIMP nsUnicharStreamLoader::GetChannel(nsIChannel * *aChannel)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute ACString charset; */
NS_IMETHODIMP nsUnicharStreamLoader::GetCharset(nsACString & aCharset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIUnicharStreamLoader_h__ */
