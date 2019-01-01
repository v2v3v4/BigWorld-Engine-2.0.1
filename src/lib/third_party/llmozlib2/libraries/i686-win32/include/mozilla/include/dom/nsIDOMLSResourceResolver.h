/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/ls/nsIDOMLSResourceResolver.idl
 */

#ifndef __gen_nsIDOMLSResourceResolver_h__
#define __gen_nsIDOMLSResourceResolver_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMLSInput; /* forward declaration */


/* starting interface:    nsIDOMLSResourceResolver */
#define NS_IDOMLSRESOURCERESOLVER_IID_STR "9e61c7c8-8698-4477-9971-0923513919bd"

#define NS_IDOMLSRESOURCERESOLVER_IID \
  {0x9e61c7c8, 0x8698, 0x4477, \
    { 0x99, 0x71, 0x09, 0x23, 0x51, 0x39, 0x19, 0xbd }}

class NS_NO_VTABLE nsIDOMLSResourceResolver : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMLSRESOURCERESOLVER_IID)

  /* nsIDOMLSInput resolveResource (in DOMString type, in DOMString namespaceURI, in DOMString publicId, in DOMString systemId, in DOMString baseURI); */
  NS_IMETHOD ResolveResource(const nsAString & type, const nsAString & namespaceURI, const nsAString & publicId, const nsAString & systemId, const nsAString & baseURI, nsIDOMLSInput **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMLSRESOURCERESOLVER \
  NS_IMETHOD ResolveResource(const nsAString & type, const nsAString & namespaceURI, const nsAString & publicId, const nsAString & systemId, const nsAString & baseURI, nsIDOMLSInput **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMLSRESOURCERESOLVER(_to) \
  NS_IMETHOD ResolveResource(const nsAString & type, const nsAString & namespaceURI, const nsAString & publicId, const nsAString & systemId, const nsAString & baseURI, nsIDOMLSInput **_retval) { return _to ResolveResource(type, namespaceURI, publicId, systemId, baseURI, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMLSRESOURCERESOLVER(_to) \
  NS_IMETHOD ResolveResource(const nsAString & type, const nsAString & namespaceURI, const nsAString & publicId, const nsAString & systemId, const nsAString & baseURI, nsIDOMLSInput **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ResolveResource(type, namespaceURI, publicId, systemId, baseURI, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMLSResourceResolver : public nsIDOMLSResourceResolver
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMLSRESOURCERESOLVER

  nsDOMLSResourceResolver();

private:
  ~nsDOMLSResourceResolver();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMLSResourceResolver, nsIDOMLSResourceResolver)

nsDOMLSResourceResolver::nsDOMLSResourceResolver()
{
  /* member initializers and constructor code */
}

nsDOMLSResourceResolver::~nsDOMLSResourceResolver()
{
  /* destructor code */
}

/* nsIDOMLSInput resolveResource (in DOMString type, in DOMString namespaceURI, in DOMString publicId, in DOMString systemId, in DOMString baseURI); */
NS_IMETHODIMP nsDOMLSResourceResolver::ResolveResource(const nsAString & type, const nsAString & namespaceURI, const nsAString & publicId, const nsAString & systemId, const nsAString & baseURI, nsIDOMLSInput **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMLSResourceResolver_h__ */
