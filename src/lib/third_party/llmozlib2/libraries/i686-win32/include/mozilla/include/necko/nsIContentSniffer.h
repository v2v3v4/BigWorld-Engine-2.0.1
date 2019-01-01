/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsIContentSniffer.idl
 */

#ifndef __gen_nsIContentSniffer_h__
#define __gen_nsIContentSniffer_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIRequest; /* forward declaration */


/* starting interface:    nsIContentSniffer */
#define NS_ICONTENTSNIFFER_IID_STR "a5710331-74ec-45fb-aa85-ed3bc7c36924"

#define NS_ICONTENTSNIFFER_IID \
  {0xa5710331, 0x74ec, 0x45fb, \
    { 0xaa, 0x85, 0xed, 0x3b, 0xc7, 0xc3, 0x69, 0x24 }}

/**
 * Content sniffer interface. Components implementing this interface can
 * determine a MIME type from a chunk of bytes.
 */
class NS_NO_VTABLE nsIContentSniffer : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICONTENTSNIFFER_IID)

  /**
   * Given a chunk of data, determines a MIME type.
   *
   * @param aData Data to check
   * @param aLength Length of the data
   *
   * @throw NS_ERROR_NOT_AVAILABLE if no MIME type could be determined.
   */
  /* ACString getMIMETypeFromContent ([array, size_is (aLength), const] in octet aData, in unsigned long aLength); */
  NS_IMETHOD GetMIMETypeFromContent(const PRUint8 *aData, PRUint32 aLength, nsACString & _retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICONTENTSNIFFER \
  NS_IMETHOD GetMIMETypeFromContent(const PRUint8 *aData, PRUint32 aLength, nsACString & _retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICONTENTSNIFFER(_to) \
  NS_IMETHOD GetMIMETypeFromContent(const PRUint8 *aData, PRUint32 aLength, nsACString & _retval) { return _to GetMIMETypeFromContent(aData, aLength, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICONTENTSNIFFER(_to) \
  NS_IMETHOD GetMIMETypeFromContent(const PRUint8 *aData, PRUint32 aLength, nsACString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMIMETypeFromContent(aData, aLength, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsContentSniffer : public nsIContentSniffer
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICONTENTSNIFFER

  nsContentSniffer();

private:
  ~nsContentSniffer();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsContentSniffer, nsIContentSniffer)

nsContentSniffer::nsContentSniffer()
{
  /* member initializers and constructor code */
}

nsContentSniffer::~nsContentSniffer()
{
  /* destructor code */
}

/* ACString getMIMETypeFromContent ([array, size_is (aLength), const] in octet aData, in unsigned long aLength); */
NS_IMETHODIMP nsContentSniffer::GetMIMETypeFromContent(const PRUint8 *aData, PRUint32 aLength, nsACString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIContentSniffer_MOZILLA_1_8_BRANCH */
#define NS_ICONTENTSNIFFER_MOZILLA_1_8_BRANCH_IID_STR "a5772d1b-fc63-495e-a169-96e8d3311af0"

#define NS_ICONTENTSNIFFER_MOZILLA_1_8_BRANCH_IID \
  {0xa5772d1b, 0xfc63, 0x495e, \
    { 0xa1, 0x69, 0x96, 0xe8, 0xd3, 0x31, 0x1a, 0xf0 }}

/**
 * Content sniffer interface. Components implementing this interface can
 * determine a MIME type from a chunk of bytes.
 *
 * NOTE: This interface is equivalent to nsIContentSniffer, but its method has
 * an additional paramter. It exists only on the 1.8 branch of Mozilla; use
 * nsIContentSniffer on newer branches.
 *
 * @status TEMPORARY
 */
class NS_NO_VTABLE nsIContentSniffer_MOZILLA_1_8_BRANCH : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICONTENTSNIFFER_MOZILLA_1_8_BRANCH_IID)

  /**
   * Given a chunk of data, determines a MIME type. Information from the given
   * request may be used in order to make a better decision.
   *
   * @param aRequest The request where this data came from. May be null.
   * @param aData Data to check
   * @param aLength Length of the data
   *
   * @return The content type
   *
   * @throw NS_ERROR_NOT_AVAILABLE if no MIME type could be determined.
   *
   * @note Implementations should consider the request read-only. Especially,
   * they should not attempt to set the content type property that subclasses of
   * nsIRequest might offer.
   */
  /* ACString getMIMETypeFromContent (in nsIRequest aRequest, [array, size_is (aLength), const] in octet aData, in unsigned long aLength); */
  NS_IMETHOD GetMIMETypeFromContent(nsIRequest *aRequest, const PRUint8 *aData, PRUint32 aLength, nsACString & _retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICONTENTSNIFFER_MOZILLA_1_8_BRANCH \
  NS_IMETHOD GetMIMETypeFromContent(nsIRequest *aRequest, const PRUint8 *aData, PRUint32 aLength, nsACString & _retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICONTENTSNIFFER_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD GetMIMETypeFromContent(nsIRequest *aRequest, const PRUint8 *aData, PRUint32 aLength, nsACString & _retval) { return _to GetMIMETypeFromContent(aRequest, aData, aLength, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICONTENTSNIFFER_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD GetMIMETypeFromContent(nsIRequest *aRequest, const PRUint8 *aData, PRUint32 aLength, nsACString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMIMETypeFromContent(aRequest, aData, aLength, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsContentSniffer_MOZILLA_1_8_BRANCH : public nsIContentSniffer_MOZILLA_1_8_BRANCH
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICONTENTSNIFFER_MOZILLA_1_8_BRANCH

  nsContentSniffer_MOZILLA_1_8_BRANCH();

private:
  ~nsContentSniffer_MOZILLA_1_8_BRANCH();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsContentSniffer_MOZILLA_1_8_BRANCH, nsIContentSniffer_MOZILLA_1_8_BRANCH)

nsContentSniffer_MOZILLA_1_8_BRANCH::nsContentSniffer_MOZILLA_1_8_BRANCH()
{
  /* member initializers and constructor code */
}

nsContentSniffer_MOZILLA_1_8_BRANCH::~nsContentSniffer_MOZILLA_1_8_BRANCH()
{
  /* destructor code */
}

/* ACString getMIMETypeFromContent (in nsIRequest aRequest, [array, size_is (aLength), const] in octet aData, in unsigned long aLength); */
NS_IMETHODIMP nsContentSniffer_MOZILLA_1_8_BRANCH::GetMIMETypeFromContent(nsIRequest *aRequest, const PRUint8 *aData, PRUint32 aLength, nsACString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIContentSniffer_h__ */
