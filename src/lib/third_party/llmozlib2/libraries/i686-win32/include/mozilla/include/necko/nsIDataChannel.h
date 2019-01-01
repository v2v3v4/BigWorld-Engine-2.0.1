/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/protocol/data/public/nsIDataChannel.idl
 */

#ifndef __gen_nsIDataChannel_h__
#define __gen_nsIDataChannel_h__


#ifndef __gen_nsIChannel_h__
#include "nsIChannel.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDataChannel */
#define NS_IDATACHANNEL_IID_STR "7e835f60-5fea-11d3-a177-0050041caf44"

#define NS_IDATACHANNEL_IID \
  {0x7e835f60, 0x5fea, 0x11d3, \
    { 0xa1, 0x77, 0x00, 0x50, 0x04, 0x1c, 0xaf, 0x44 }}

class NS_NO_VTABLE nsIDataChannel : public nsIChannel {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDATACHANNEL_IID)

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDATACHANNEL \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDATACHANNEL(_to) \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDATACHANNEL(_to) \
  /* no methods! */

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDataChannel : public nsIDataChannel
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDATACHANNEL

  nsDataChannel();

private:
  ~nsDataChannel();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDataChannel, nsIDataChannel)

nsDataChannel::nsDataChannel()
{
  /* member initializers and constructor code */
}

nsDataChannel::~nsDataChannel()
{
  /* destructor code */
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDataChannel_h__ */
