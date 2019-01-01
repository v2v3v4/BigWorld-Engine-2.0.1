/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/core/nsIDOMUserDataHandler.idl
 */

#ifndef __gen_nsIDOMUserDataHandler_h__
#define __gen_nsIDOMUserDataHandler_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

#ifndef __gen_nsIVariant_h__
#include "nsIVariant.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMUserDataHandler */
#define NS_IDOMUSERDATAHANDLER_IID_STR "5470deff-03c9-41b7-a824-e3225266b343"

#define NS_IDOMUSERDATAHANDLER_IID \
  {0x5470deff, 0x03c9, 0x41b7, \
    { 0xa8, 0x24, 0xe3, 0x22, 0x52, 0x66, 0xb3, 0x43 }}

class NS_NO_VTABLE nsIDOMUserDataHandler {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMUSERDATAHANDLER_IID)

  enum { NODE_CLONED = 1U };

  enum { NODE_IMPORTED = 2U };

  enum { NODE_DELETED = 3U };

  enum { NODE_RENAMED = 4U };

  /* void handle (in unsigned short operation, in DOMString key, in nsIVariant data, in nsIDOMNode src, in nsIDOMNode dst); */
  NS_IMETHOD Handle(PRUint16 operation, const nsAString & key, nsIVariant *data, nsIDOMNode *src, nsIDOMNode *dst) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMUSERDATAHANDLER \
  NS_IMETHOD Handle(PRUint16 operation, const nsAString & key, nsIVariant *data, nsIDOMNode *src, nsIDOMNode *dst); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMUSERDATAHANDLER(_to) \
  NS_IMETHOD Handle(PRUint16 operation, const nsAString & key, nsIVariant *data, nsIDOMNode *src, nsIDOMNode *dst) { return _to Handle(operation, key, data, src, dst); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMUSERDATAHANDLER(_to) \
  NS_IMETHOD Handle(PRUint16 operation, const nsAString & key, nsIVariant *data, nsIDOMNode *src, nsIDOMNode *dst) { return !_to ? NS_ERROR_NULL_POINTER : _to->Handle(operation, key, data, src, dst); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMUserDataHandler : public nsIDOMUserDataHandler
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMUSERDATAHANDLER

  nsDOMUserDataHandler();

private:
  ~nsDOMUserDataHandler();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMUserDataHandler, nsIDOMUserDataHandler)

nsDOMUserDataHandler::nsDOMUserDataHandler()
{
  /* member initializers and constructor code */
}

nsDOMUserDataHandler::~nsDOMUserDataHandler()
{
  /* destructor code */
}

/* void handle (in unsigned short operation, in DOMString key, in nsIVariant data, in nsIDOMNode src, in nsIDOMNode dst); */
NS_IMETHODIMP nsDOMUserDataHandler::Handle(PRUint16 operation, const nsAString & key, nsIVariant *data, nsIDOMNode *src, nsIDOMNode *dst)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMUserDataHandler_h__ */
