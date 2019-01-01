/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/io/nsIByteArrayInputStream.idl
 */

#ifndef __gen_nsIByteArrayInputStream_h__
#define __gen_nsIByteArrayInputStream_h__


#ifndef __gen_nsIInputStream_h__
#include "nsIInputStream.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIByteArrayInputStream */
#define NS_IBYTEARRAYINPUTSTREAM_IID_STR "b5a21556-35fc-4815-aff1-f9142639686e"

#define NS_IBYTEARRAYINPUTSTREAM_IID \
  {0xb5a21556, 0x35fc, 0x4815, \
    { 0xaf, 0xf1, 0xf9, 0x14, 0x26, 0x39, 0x68, 0x6e }}

class NS_NO_VTABLE nsIByteArrayInputStream : public nsIInputStream {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IBYTEARRAYINPUTSTREAM_IID)

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIBYTEARRAYINPUTSTREAM \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIBYTEARRAYINPUTSTREAM(_to) \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIBYTEARRAYINPUTSTREAM(_to) \
  /* no methods! */

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsByteArrayInputStream : public nsIByteArrayInputStream
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIBYTEARRAYINPUTSTREAM

  nsByteArrayInputStream();

private:
  ~nsByteArrayInputStream();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsByteArrayInputStream, nsIByteArrayInputStream)

nsByteArrayInputStream::nsByteArrayInputStream()
{
  /* member initializers and constructor code */
}

nsByteArrayInputStream::~nsByteArrayInputStream()
{
  /* destructor code */
}

/* End of implementation class template. */
#endif

extern NS_COM nsresult
NS_NewByteArrayInputStream (nsIByteArrayInputStream ** aResult, char * buffer, unsigned long size);

#endif /* __gen_nsIByteArrayInputStream_h__ */
