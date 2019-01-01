/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/base/nsIDOMClientInformation.idl
 */

#ifndef __gen_nsIDOMClientInformation_h__
#define __gen_nsIDOMClientInformation_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMClientInformation */
#define NS_IDOMCLIENTINFORMATION_IID_STR "abbb51a4-be75-4d7f-bd4c-373fd7b52f85"

#define NS_IDOMCLIENTINFORMATION_IID \
  {0xabbb51a4, 0xbe75, 0x4d7f, \
    { 0xbd, 0x4c, 0x37, 0x3f, 0xd7, 0xb5, 0x2f, 0x85 }}

class NS_NO_VTABLE nsIDOMClientInformation : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMCLIENTINFORMATION_IID)

  /**
   * Web Applications 1.0 Browser State: registerContentHandler
   * Allows web services to register themselves as handlers for certain content
   * types.
   * http://whatwg.org/specs/web-apps/current-work/
   */
  /* void registerContentHandler (in DOMString mimeType, in DOMString uri, in DOMString title); */
  NS_IMETHOD RegisterContentHandler(const nsAString & mimeType, const nsAString & uri, const nsAString & title) = 0;

  /* void registerProtocolHandler (in DOMString protocol, in DOMString uri, in DOMString title); */
  NS_IMETHOD RegisterProtocolHandler(const nsAString & protocol, const nsAString & uri, const nsAString & title) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMCLIENTINFORMATION \
  NS_IMETHOD RegisterContentHandler(const nsAString & mimeType, const nsAString & uri, const nsAString & title); \
  NS_IMETHOD RegisterProtocolHandler(const nsAString & protocol, const nsAString & uri, const nsAString & title); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMCLIENTINFORMATION(_to) \
  NS_IMETHOD RegisterContentHandler(const nsAString & mimeType, const nsAString & uri, const nsAString & title) { return _to RegisterContentHandler(mimeType, uri, title); } \
  NS_IMETHOD RegisterProtocolHandler(const nsAString & protocol, const nsAString & uri, const nsAString & title) { return _to RegisterProtocolHandler(protocol, uri, title); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMCLIENTINFORMATION(_to) \
  NS_IMETHOD RegisterContentHandler(const nsAString & mimeType, const nsAString & uri, const nsAString & title) { return !_to ? NS_ERROR_NULL_POINTER : _to->RegisterContentHandler(mimeType, uri, title); } \
  NS_IMETHOD RegisterProtocolHandler(const nsAString & protocol, const nsAString & uri, const nsAString & title) { return !_to ? NS_ERROR_NULL_POINTER : _to->RegisterProtocolHandler(protocol, uri, title); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMClientInformation : public nsIDOMClientInformation
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMCLIENTINFORMATION

  nsDOMClientInformation();

private:
  ~nsDOMClientInformation();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMClientInformation, nsIDOMClientInformation)

nsDOMClientInformation::nsDOMClientInformation()
{
  /* member initializers and constructor code */
}

nsDOMClientInformation::~nsDOMClientInformation()
{
  /* destructor code */
}

/* void registerContentHandler (in DOMString mimeType, in DOMString uri, in DOMString title); */
NS_IMETHODIMP nsDOMClientInformation::RegisterContentHandler(const nsAString & mimeType, const nsAString & uri, const nsAString & title)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void registerProtocolHandler (in DOMString protocol, in DOMString uri, in DOMString title); */
NS_IMETHODIMP nsDOMClientInformation::RegisterProtocolHandler(const nsAString & protocol, const nsAString & uri, const nsAString & title)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMClientInformation_h__ */
