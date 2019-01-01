/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/base/nsIDOMNSHistory.idl
 */

#ifndef __gen_nsIDOMNSHistory_h__
#define __gen_nsIDOMNSHistory_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMNSHistory */
#define NS_IDOMNSHISTORY_IID_STR "5fb96f46-1dd2-11b2-a5dc-998ca8636ea9"

#define NS_IDOMNSHISTORY_IID \
  {0x5fb96f46, 0x1dd2, 0x11b2, \
    { 0xa5, 0xdc, 0x99, 0x8c, 0xa8, 0x63, 0x6e, 0xa9 }}

class NS_NO_VTABLE nsIDOMNSHistory : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMNSHISTORY_IID)

  /**
   * go() can be called with an integer argument or no arguments (a
   * nop) from JS
   */
  /* void go (); */
  NS_IMETHOD Go(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMNSHISTORY \
  NS_IMETHOD Go(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMNSHISTORY(_to) \
  NS_IMETHOD Go(void) { return _to Go(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMNSHISTORY(_to) \
  NS_IMETHOD Go(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Go(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMNSHistory : public nsIDOMNSHistory
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMNSHISTORY

  nsDOMNSHistory();

private:
  ~nsDOMNSHistory();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMNSHistory, nsIDOMNSHistory)

nsDOMNSHistory::nsDOMNSHistory()
{
  /* member initializers and constructor code */
}

nsDOMNSHistory::~nsDOMNSHistory()
{
  /* destructor code */
}

/* void go (); */
NS_IMETHODIMP nsDOMNSHistory::Go()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMNSHistory_h__ */
