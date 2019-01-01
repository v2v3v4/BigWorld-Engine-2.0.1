/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/socket/base/nsITransportSecurityInfo.idl
 */

#ifndef __gen_nsITransportSecurityInfo_h__
#define __gen_nsITransportSecurityInfo_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsITransportSecurityInfo */
#define NS_ITRANSPORTSECURITYINFO_IID_STR "68e21b66-1dd2-11b2-aa67-e2b87175e792"

#define NS_ITRANSPORTSECURITYINFO_IID \
  {0x68e21b66, 0x1dd2, 0x11b2, \
    { 0xaa, 0x67, 0xe2, 0xb8, 0x71, 0x75, 0xe7, 0x92 }}

class NS_NO_VTABLE nsITransportSecurityInfo : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ITRANSPORTSECURITYINFO_IID)

  /* readonly attribute unsigned long securityState; */
  NS_IMETHOD GetSecurityState(PRUint32 *aSecurityState) = 0;

  /* readonly attribute wstring shortSecurityDescription; */
  NS_IMETHOD GetShortSecurityDescription(PRUnichar * *aShortSecurityDescription) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSITRANSPORTSECURITYINFO \
  NS_IMETHOD GetSecurityState(PRUint32 *aSecurityState); \
  NS_IMETHOD GetShortSecurityDescription(PRUnichar * *aShortSecurityDescription); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSITRANSPORTSECURITYINFO(_to) \
  NS_IMETHOD GetSecurityState(PRUint32 *aSecurityState) { return _to GetSecurityState(aSecurityState); } \
  NS_IMETHOD GetShortSecurityDescription(PRUnichar * *aShortSecurityDescription) { return _to GetShortSecurityDescription(aShortSecurityDescription); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSITRANSPORTSECURITYINFO(_to) \
  NS_IMETHOD GetSecurityState(PRUint32 *aSecurityState) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSecurityState(aSecurityState); } \
  NS_IMETHOD GetShortSecurityDescription(PRUnichar * *aShortSecurityDescription) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetShortSecurityDescription(aShortSecurityDescription); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsTransportSecurityInfo : public nsITransportSecurityInfo
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSITRANSPORTSECURITYINFO

  nsTransportSecurityInfo();

private:
  ~nsTransportSecurityInfo();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsTransportSecurityInfo, nsITransportSecurityInfo)

nsTransportSecurityInfo::nsTransportSecurityInfo()
{
  /* member initializers and constructor code */
}

nsTransportSecurityInfo::~nsTransportSecurityInfo()
{
  /* destructor code */
}

/* readonly attribute unsigned long securityState; */
NS_IMETHODIMP nsTransportSecurityInfo::GetSecurityState(PRUint32 *aSecurityState)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute wstring shortSecurityDescription; */
NS_IMETHODIMP nsTransportSecurityInfo::GetShortSecurityDescription(PRUnichar * *aShortSecurityDescription)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsITransportSecurityInfo_h__ */
