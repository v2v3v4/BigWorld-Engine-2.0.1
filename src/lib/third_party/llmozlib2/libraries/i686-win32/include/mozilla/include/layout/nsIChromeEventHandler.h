/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/base/nsIChromeEventHandler.idl
 */

#ifndef __gen_nsIChromeEventHandler_h__
#define __gen_nsIChromeEventHandler_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "nsEvent.h"  // for nsEventStatus enum
class nsPresContext;
class nsIDOMEvent; /* forward declaration */


/* starting interface:    nsIChromeEventHandler */
#define NS_ICHROMEEVENTHANDLER_IID_STR "7bc08970-9e6c-11d3-afb2-00a024ffc08c"

#define NS_ICHROMEEVENTHANDLER_IID \
  {0x7bc08970, 0x9e6c, 0x11d3, \
    { 0xaf, 0xb2, 0x00, 0xa0, 0x24, 0xff, 0xc0, 0x8c }}

/**
 * The nsIChromeEventHandler
 */
class NS_NO_VTABLE nsIChromeEventHandler : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICHROMEEVENTHANDLER_IID)

  /**
	* Handle a chrome DOM event.
	*/
  /* [noscript] void handleChromeEvent (in nsPresContext aPresContext, in nsEventPtr aEvent, out nsIDOMEvent aDOMEvent, in unsigned long aFlags, inout nsEventStatus aStatus); */
  NS_IMETHOD HandleChromeEvent(nsPresContext * aPresContext, nsEvent * aEvent, nsIDOMEvent **aDOMEvent, PRUint32 aFlags, nsEventStatus *aStatus) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICHROMEEVENTHANDLER \
  NS_IMETHOD HandleChromeEvent(nsPresContext * aPresContext, nsEvent * aEvent, nsIDOMEvent **aDOMEvent, PRUint32 aFlags, nsEventStatus *aStatus); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICHROMEEVENTHANDLER(_to) \
  NS_IMETHOD HandleChromeEvent(nsPresContext * aPresContext, nsEvent * aEvent, nsIDOMEvent **aDOMEvent, PRUint32 aFlags, nsEventStatus *aStatus) { return _to HandleChromeEvent(aPresContext, aEvent, aDOMEvent, aFlags, aStatus); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICHROMEEVENTHANDLER(_to) \
  NS_IMETHOD HandleChromeEvent(nsPresContext * aPresContext, nsEvent * aEvent, nsIDOMEvent **aDOMEvent, PRUint32 aFlags, nsEventStatus *aStatus) { return !_to ? NS_ERROR_NULL_POINTER : _to->HandleChromeEvent(aPresContext, aEvent, aDOMEvent, aFlags, aStatus); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsChromeEventHandler : public nsIChromeEventHandler
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICHROMEEVENTHANDLER

  nsChromeEventHandler();

private:
  ~nsChromeEventHandler();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsChromeEventHandler, nsIChromeEventHandler)

nsChromeEventHandler::nsChromeEventHandler()
{
  /* member initializers and constructor code */
}

nsChromeEventHandler::~nsChromeEventHandler()
{
  /* destructor code */
}

/* [noscript] void handleChromeEvent (in nsPresContext aPresContext, in nsEventPtr aEvent, out nsIDOMEvent aDOMEvent, in unsigned long aFlags, inout nsEventStatus aStatus); */
NS_IMETHODIMP nsChromeEventHandler::HandleChromeEvent(nsPresContext * aPresContext, nsEvent * aEvent, nsIDOMEvent **aDOMEvent, PRUint32 aFlags, nsEventStatus *aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIChromeEventHandler_h__ */
