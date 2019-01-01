/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsINetUtil.idl
 */

#ifndef __gen_nsINetUtil_h__
#define __gen_nsINetUtil_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsINetUtil */
#define NS_INETUTIL_IID_STR "f0c5dddb-4713-4603-af2d-bf671838996b"

#define NS_INETUTIL_IID \
  {0xf0c5dddb, 0x4713, 0x4603, \
    { 0xaf, 0x2d, 0xbf, 0x67, 0x18, 0x38, 0x99, 0x6b }}

/**
 * nsINetUtil provides various network-related utility methods.
 */
class NS_NO_VTABLE nsINetUtil : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_INETUTIL_IID)

  /**
   * Parse a content-type header and return the content type and
   * charset (if any).
   *
   * @param aTypeHeader the header string to parse
   * @param [out] aCharset the charset parameter specified in the
   *              header, if any.
   * @param [out] aHadCharset whether a charset was explicitly specified.
   * @return the MIME type specified in the header, in lower-case.
   */
  /* AUTF8String parseContentType (in AUTF8String aTypeHeader, out AUTF8String aCharset, out boolean aHadCharset); */
  NS_IMETHOD ParseContentType(const nsACString & aTypeHeader, nsACString & aCharset, PRBool *aHadCharset, nsACString & _retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSINETUTIL \
  NS_IMETHOD ParseContentType(const nsACString & aTypeHeader, nsACString & aCharset, PRBool *aHadCharset, nsACString & _retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSINETUTIL(_to) \
  NS_IMETHOD ParseContentType(const nsACString & aTypeHeader, nsACString & aCharset, PRBool *aHadCharset, nsACString & _retval) { return _to ParseContentType(aTypeHeader, aCharset, aHadCharset, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSINETUTIL(_to) \
  NS_IMETHOD ParseContentType(const nsACString & aTypeHeader, nsACString & aCharset, PRBool *aHadCharset, nsACString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ParseContentType(aTypeHeader, aCharset, aHadCharset, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsNetUtil : public nsINetUtil
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSINETUTIL

  nsNetUtil();

private:
  ~nsNetUtil();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsNetUtil, nsINetUtil)

nsNetUtil::nsNetUtil()
{
  /* member initializers and constructor code */
}

nsNetUtil::~nsNetUtil()
{
  /* destructor code */
}

/* AUTF8String parseContentType (in AUTF8String aTypeHeader, out AUTF8String aCharset, out boolean aHadCharset); */
NS_IMETHODIMP nsNetUtil::ParseContentType(const nsACString & aTypeHeader, nsACString & aCharset, PRBool *aHadCharset, nsACString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsINetUtil_h__ */
