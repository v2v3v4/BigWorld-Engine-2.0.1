/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/html/document/public/nsIWyciwygChannel.idl
 */

#ifndef __gen_nsIWyciwygChannel_h__
#define __gen_nsIWyciwygChannel_h__


#ifndef __gen_nsIChannel_h__
#include "nsIChannel.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIWyciwygChannel */
#define NS_IWYCIWYGCHANNEL_IID_STR "280da566-6f19-4487-a8ca-70c5ba1602c1"

#define NS_IWYCIWYGCHANNEL_IID \
  {0x280da566, 0x6f19, 0x4487, \
    { 0xa8, 0xca, 0x70, 0xc5, 0xba, 0x16, 0x02, 0xc1 }}

/**
 * A channel to  manage all cache-related interactions for layout
 * when it is dealing with dynamic pages created through 
 * document.write(). This interface provides methods that will
 * help layout save dynamic pages in cache for future retrievals.
 */
class NS_NO_VTABLE nsIWyciwygChannel : public nsIChannel {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IWYCIWYGCHANNEL_IID)

  /**
   * Append data to the cache entry; opens the cache entry if necessary.
   */
  /* void writeToCacheEntry (in AString aData); */
  NS_IMETHOD WriteToCacheEntry(const nsAString & aData) = 0;

  /**
   * Close the cache entry; subsequent writes have undefined behavior.
   */
  /* void closeCacheEntry (in nsresult reason); */
  NS_IMETHOD CloseCacheEntry(nsresult reason) = 0;

  /**
   * Set the wyciwyg channels security info
   */
  /* void setSecurityInfo (in nsISupports aSecurityInfo); */
  NS_IMETHOD SetSecurityInfo(nsISupports *aSecurityInfo) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIWYCIWYGCHANNEL \
  NS_IMETHOD WriteToCacheEntry(const nsAString & aData); \
  NS_IMETHOD CloseCacheEntry(nsresult reason); \
  NS_IMETHOD SetSecurityInfo(nsISupports *aSecurityInfo); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIWYCIWYGCHANNEL(_to) \
  NS_IMETHOD WriteToCacheEntry(const nsAString & aData) { return _to WriteToCacheEntry(aData); } \
  NS_IMETHOD CloseCacheEntry(nsresult reason) { return _to CloseCacheEntry(reason); } \
  NS_IMETHOD SetSecurityInfo(nsISupports *aSecurityInfo) { return _to SetSecurityInfo(aSecurityInfo); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIWYCIWYGCHANNEL(_to) \
  NS_IMETHOD WriteToCacheEntry(const nsAString & aData) { return !_to ? NS_ERROR_NULL_POINTER : _to->WriteToCacheEntry(aData); } \
  NS_IMETHOD CloseCacheEntry(nsresult reason) { return !_to ? NS_ERROR_NULL_POINTER : _to->CloseCacheEntry(reason); } \
  NS_IMETHOD SetSecurityInfo(nsISupports *aSecurityInfo) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSecurityInfo(aSecurityInfo); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsWyciwygChannel : public nsIWyciwygChannel
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIWYCIWYGCHANNEL

  nsWyciwygChannel();

private:
  ~nsWyciwygChannel();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsWyciwygChannel, nsIWyciwygChannel)

nsWyciwygChannel::nsWyciwygChannel()
{
  /* member initializers and constructor code */
}

nsWyciwygChannel::~nsWyciwygChannel()
{
  /* destructor code */
}

/* void writeToCacheEntry (in AString aData); */
NS_IMETHODIMP nsWyciwygChannel::WriteToCacheEntry(const nsAString & aData)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void closeCacheEntry (in nsresult reason); */
NS_IMETHODIMP nsWyciwygChannel::CloseCacheEntry(nsresult reason)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setSecurityInfo (in nsISupports aSecurityInfo); */
NS_IMETHODIMP nsWyciwygChannel::SetSecurityInfo(nsISupports *aSecurityInfo)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIWyciwygChannel_h__ */
