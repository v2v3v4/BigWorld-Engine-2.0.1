/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/dns/public/nsIDNSRecord.idl
 */

#ifndef __gen_nsIDNSRecord_h__
#define __gen_nsIDNSRecord_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDNSRecord */
#define NS_IDNSRECORD_IID_STR "31c9c52e-1100-457d-abac-d2729e43f506"

#define NS_IDNSRECORD_IID \
  {0x31c9c52e, 0x1100, 0x457d, \
    { 0xab, 0xac, 0xd2, 0x72, 0x9e, 0x43, 0xf5, 0x06 }}

/**
 * nsIDNSRecord
 *
 * this interface represents the result of a DNS lookup.  since a DNS
 * query may return more than one resolved IP address, the record acts
 * like an enumerator, allowing the caller to easily step through the
 * list of IP addresses.
 */
class NS_NO_VTABLE nsIDNSRecord : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDNSRECORD_IID)

  /**
     * @return the canonical hostname for this record.  this value is empty if
     * the record was not fetched with the RESOLVE_CANONICAL_NAME flag.
     *
     * e.g., www.mozilla.org --> rheet.mozilla.org
     */
  /* readonly attribute ACString canonicalName; */
  NS_IMETHOD GetCanonicalName(nsACString & aCanonicalName) = 0;

  /**
     * this function copies the value of the next IP address into the
     * given PRNetAddr struct and increments the internal address iterator.
     *
     * @param aPort
     *        A port number to initialize the PRNetAddr with.
     *
     * @throws NS_ERROR_NOT_AVAILABLE if there is not another IP address in
     * the record.
     */
  /* [noscript] PRNetAddr getNextAddr (in PRUint16 aPort); */
  NS_IMETHOD GetNextAddr(PRUint16 aPort, union PRNetAddr *_retval) = 0;

  /**
     * this function returns the value of the next IP address as a
     * string and increments the internal address iterator.
     *
     * @throws NS_ERROR_NOT_AVAILABLE if there is not another IP address in
     * the record.
     */
  /* ACString getNextAddrAsString (); */
  NS_IMETHOD GetNextAddrAsString(nsACString & _retval) = 0;

  /**
     * this function returns true if there is another address in the record.
     */
  /* boolean hasMore (); */
  NS_IMETHOD HasMore(PRBool *_retval) = 0;

  /**
     * this function resets the internal address iterator to the first
     * address in the record.
     */
  /* void rewind (); */
  NS_IMETHOD Rewind(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDNSRECORD \
  NS_IMETHOD GetCanonicalName(nsACString & aCanonicalName); \
  NS_IMETHOD GetNextAddr(PRUint16 aPort, union PRNetAddr *_retval); \
  NS_IMETHOD GetNextAddrAsString(nsACString & _retval); \
  NS_IMETHOD HasMore(PRBool *_retval); \
  NS_IMETHOD Rewind(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDNSRECORD(_to) \
  NS_IMETHOD GetCanonicalName(nsACString & aCanonicalName) { return _to GetCanonicalName(aCanonicalName); } \
  NS_IMETHOD GetNextAddr(PRUint16 aPort, union PRNetAddr *_retval) { return _to GetNextAddr(aPort, _retval); } \
  NS_IMETHOD GetNextAddrAsString(nsACString & _retval) { return _to GetNextAddrAsString(_retval); } \
  NS_IMETHOD HasMore(PRBool *_retval) { return _to HasMore(_retval); } \
  NS_IMETHOD Rewind(void) { return _to Rewind(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDNSRECORD(_to) \
  NS_IMETHOD GetCanonicalName(nsACString & aCanonicalName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCanonicalName(aCanonicalName); } \
  NS_IMETHOD GetNextAddr(PRUint16 aPort, union PRNetAddr *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNextAddr(aPort, _retval); } \
  NS_IMETHOD GetNextAddrAsString(nsACString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNextAddrAsString(_retval); } \
  NS_IMETHOD HasMore(PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->HasMore(_retval); } \
  NS_IMETHOD Rewind(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Rewind(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDNSRecord : public nsIDNSRecord
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDNSRECORD

  nsDNSRecord();

private:
  ~nsDNSRecord();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDNSRecord, nsIDNSRecord)

nsDNSRecord::nsDNSRecord()
{
  /* member initializers and constructor code */
}

nsDNSRecord::~nsDNSRecord()
{
  /* destructor code */
}

/* readonly attribute ACString canonicalName; */
NS_IMETHODIMP nsDNSRecord::GetCanonicalName(nsACString & aCanonicalName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] PRNetAddr getNextAddr (in PRUint16 aPort); */
NS_IMETHODIMP nsDNSRecord::GetNextAddr(PRUint16 aPort, union PRNetAddr *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* ACString getNextAddrAsString (); */
NS_IMETHODIMP nsDNSRecord::GetNextAddrAsString(nsACString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean hasMore (); */
NS_IMETHODIMP nsDNSRecord::HasMore(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void rewind (); */
NS_IMETHODIMP nsDNSRecord::Rewind()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDNSRecord_h__ */
