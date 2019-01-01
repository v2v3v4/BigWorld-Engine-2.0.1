/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/protocol/file/public/nsIFileChannel.idl
 */

#ifndef __gen_nsIFileChannel_h__
#define __gen_nsIFileChannel_h__


#ifndef __gen_nsIChannel_h__
#include "nsIChannel.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIFile; /* forward declaration */


/* starting interface:    nsIFileChannel */
#define NS_IFILECHANNEL_IID_STR "68a26506-f947-11d3-8cda-0060b0fc14a3"

#define NS_IFILECHANNEL_IID \
  {0x68a26506, 0xf947, 0x11d3, \
    { 0x8c, 0xda, 0x00, 0x60, 0xb0, 0xfc, 0x14, 0xa3 }}

/**
 * nsIFileChannel
 */
class NS_NO_VTABLE nsIFileChannel : public nsIChannel {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IFILECHANNEL_IID)

  /* readonly attribute nsIFile file; */
  NS_IMETHOD GetFile(nsIFile * *aFile) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIFILECHANNEL \
  NS_IMETHOD GetFile(nsIFile * *aFile); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIFILECHANNEL(_to) \
  NS_IMETHOD GetFile(nsIFile * *aFile) { return _to GetFile(aFile); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIFILECHANNEL(_to) \
  NS_IMETHOD GetFile(nsIFile * *aFile) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFile(aFile); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsFileChannel : public nsIFileChannel
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIFILECHANNEL

  nsFileChannel();

private:
  ~nsFileChannel();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsFileChannel, nsIFileChannel)

nsFileChannel::nsFileChannel()
{
  /* member initializers and constructor code */
}

nsFileChannel::~nsFileChannel()
{
  /* destructor code */
}

/* readonly attribute nsIFile file; */
NS_IMETHODIMP nsFileChannel::GetFile(nsIFile * *aFile)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIFileChannel_h__ */
