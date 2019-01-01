/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/events/nsIDOMPopupBlockedEvent.idl
 */

#ifndef __gen_nsIDOMPopupBlockedEvent_h__
#define __gen_nsIDOMPopupBlockedEvent_h__


#ifndef __gen_nsIDOMEvent_h__
#include "nsIDOMEvent.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIURI; /* forward declaration */


/* starting interface:    nsIDOMPopupBlockedEvent */
#define NS_IDOMPOPUPBLOCKEDEVENT_IID_STR "9e201104-78e9-4cb3-aff5-7f0a9cf446c0"

#define NS_IDOMPOPUPBLOCKEDEVENT_IID \
  {0x9e201104, 0x78e9, 0x4cb3, \
    { 0xaf, 0xf5, 0x7f, 0x0a, 0x9c, 0xf4, 0x46, 0xc0 }}

/**
 * The nsIDOMPopupBlockedEvent interface is the datatype for events
 * posted when a popup window is blocked.
 */
class NS_NO_VTABLE nsIDOMPopupBlockedEvent : public nsIDOMEvent {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMPOPUPBLOCKEDEVENT_IID)

  /* readonly attribute nsIURI requestingWindowURI; */
  NS_IMETHOD GetRequestingWindowURI(nsIURI * *aRequestingWindowURI) = 0;

  /* readonly attribute nsIURI popupWindowURI; */
  NS_IMETHOD GetPopupWindowURI(nsIURI * *aPopupWindowURI) = 0;

  /**
   * The string of features passed to the window.open() call
   * (as the third argument)
   */
  /* readonly attribute DOMString popupWindowFeatures; */
  NS_IMETHOD GetPopupWindowFeatures(nsAString & aPopupWindowFeatures) = 0;

  /* void initPopupBlockedEvent (in DOMString typeArg, in boolean canBubbleArg, in boolean cancelableArg, in nsIURI requestingWindowURI, in nsIURI popupWindowURI, in DOMString popupWindowFeatures); */
  NS_IMETHOD InitPopupBlockedEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIURI *requestingWindowURI, nsIURI *popupWindowURI, const nsAString & popupWindowFeatures) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMPOPUPBLOCKEDEVENT \
  NS_IMETHOD GetRequestingWindowURI(nsIURI * *aRequestingWindowURI); \
  NS_IMETHOD GetPopupWindowURI(nsIURI * *aPopupWindowURI); \
  NS_IMETHOD GetPopupWindowFeatures(nsAString & aPopupWindowFeatures); \
  NS_IMETHOD InitPopupBlockedEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIURI *requestingWindowURI, nsIURI *popupWindowURI, const nsAString & popupWindowFeatures); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMPOPUPBLOCKEDEVENT(_to) \
  NS_IMETHOD GetRequestingWindowURI(nsIURI * *aRequestingWindowURI) { return _to GetRequestingWindowURI(aRequestingWindowURI); } \
  NS_IMETHOD GetPopupWindowURI(nsIURI * *aPopupWindowURI) { return _to GetPopupWindowURI(aPopupWindowURI); } \
  NS_IMETHOD GetPopupWindowFeatures(nsAString & aPopupWindowFeatures) { return _to GetPopupWindowFeatures(aPopupWindowFeatures); } \
  NS_IMETHOD InitPopupBlockedEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIURI *requestingWindowURI, nsIURI *popupWindowURI, const nsAString & popupWindowFeatures) { return _to InitPopupBlockedEvent(typeArg, canBubbleArg, cancelableArg, requestingWindowURI, popupWindowURI, popupWindowFeatures); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMPOPUPBLOCKEDEVENT(_to) \
  NS_IMETHOD GetRequestingWindowURI(nsIURI * *aRequestingWindowURI) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRequestingWindowURI(aRequestingWindowURI); } \
  NS_IMETHOD GetPopupWindowURI(nsIURI * *aPopupWindowURI) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPopupWindowURI(aPopupWindowURI); } \
  NS_IMETHOD GetPopupWindowFeatures(nsAString & aPopupWindowFeatures) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPopupWindowFeatures(aPopupWindowFeatures); } \
  NS_IMETHOD InitPopupBlockedEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIURI *requestingWindowURI, nsIURI *popupWindowURI, const nsAString & popupWindowFeatures) { return !_to ? NS_ERROR_NULL_POINTER : _to->InitPopupBlockedEvent(typeArg, canBubbleArg, cancelableArg, requestingWindowURI, popupWindowURI, popupWindowFeatures); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMPopupBlockedEvent : public nsIDOMPopupBlockedEvent
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMPOPUPBLOCKEDEVENT

  nsDOMPopupBlockedEvent();

private:
  ~nsDOMPopupBlockedEvent();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMPopupBlockedEvent, nsIDOMPopupBlockedEvent)

nsDOMPopupBlockedEvent::nsDOMPopupBlockedEvent()
{
  /* member initializers and constructor code */
}

nsDOMPopupBlockedEvent::~nsDOMPopupBlockedEvent()
{
  /* destructor code */
}

/* readonly attribute nsIURI requestingWindowURI; */
NS_IMETHODIMP nsDOMPopupBlockedEvent::GetRequestingWindowURI(nsIURI * *aRequestingWindowURI)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIURI popupWindowURI; */
NS_IMETHODIMP nsDOMPopupBlockedEvent::GetPopupWindowURI(nsIURI * *aPopupWindowURI)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString popupWindowFeatures; */
NS_IMETHODIMP nsDOMPopupBlockedEvent::GetPopupWindowFeatures(nsAString & aPopupWindowFeatures)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void initPopupBlockedEvent (in DOMString typeArg, in boolean canBubbleArg, in boolean cancelableArg, in nsIURI requestingWindowURI, in nsIURI popupWindowURI, in DOMString popupWindowFeatures); */
NS_IMETHODIMP nsDOMPopupBlockedEvent::InitPopupBlockedEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIURI *requestingWindowURI, nsIURI *popupWindowURI, const nsAString & popupWindowFeatures)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMPopupBlockedEvent_MOZILLA_1_8_BRANCH */
#define NS_IDOMPOPUPBLOCKEDEVENT_MOZILLA_1_8_BRANCH_IID_STR "45d7d2b4-2d98-4cdb-850f-860bc45deeae"

#define NS_IDOMPOPUPBLOCKEDEVENT_MOZILLA_1_8_BRANCH_IID \
  {0x45d7d2b4, 0x2d98, 0x4cdb, \
    { 0x85, 0x0f, 0x86, 0x0b, 0xc4, 0x5d, 0xee, 0xae }}

class NS_NO_VTABLE nsIDOMPopupBlockedEvent_MOZILLA_1_8_BRANCH : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMPOPUPBLOCKEDEVENT_MOZILLA_1_8_BRANCH_IID)

  /**
   * The window object that attempted to open the blocked popup
   * (i.e. the window object on which open() was called).
   */
  /* readonly attribute nsIDOMWindow requestingWindow; */
  NS_IMETHOD GetRequestingWindow(nsIDOMWindow * *aRequestingWindow) = 0;

  /* [noscript] void initPopupBlockedEvent (in DOMString typeArg, in boolean canBubbleArg, in boolean cancelableArg, in nsIDOMWindow requestingWindow, in nsIURI popupWindowURI, in DOMString popupWindowFeatures); */
  NS_IMETHOD InitPopupBlockedEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIDOMWindow *requestingWindow, nsIURI *popupWindowURI, const nsAString & popupWindowFeatures) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMPOPUPBLOCKEDEVENT_MOZILLA_1_8_BRANCH \
  NS_IMETHOD GetRequestingWindow(nsIDOMWindow * *aRequestingWindow); \
  NS_IMETHOD InitPopupBlockedEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIDOMWindow *requestingWindow, nsIURI *popupWindowURI, const nsAString & popupWindowFeatures); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMPOPUPBLOCKEDEVENT_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD GetRequestingWindow(nsIDOMWindow * *aRequestingWindow) { return _to GetRequestingWindow(aRequestingWindow); } \
  NS_IMETHOD InitPopupBlockedEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIDOMWindow *requestingWindow, nsIURI *popupWindowURI, const nsAString & popupWindowFeatures) { return _to InitPopupBlockedEvent(typeArg, canBubbleArg, cancelableArg, requestingWindow, popupWindowURI, popupWindowFeatures); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMPOPUPBLOCKEDEVENT_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD GetRequestingWindow(nsIDOMWindow * *aRequestingWindow) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRequestingWindow(aRequestingWindow); } \
  NS_IMETHOD InitPopupBlockedEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIDOMWindow *requestingWindow, nsIURI *popupWindowURI, const nsAString & popupWindowFeatures) { return !_to ? NS_ERROR_NULL_POINTER : _to->InitPopupBlockedEvent(typeArg, canBubbleArg, cancelableArg, requestingWindow, popupWindowURI, popupWindowFeatures); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMPopupBlockedEvent_MOZILLA_1_8_BRANCH : public nsIDOMPopupBlockedEvent_MOZILLA_1_8_BRANCH
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMPOPUPBLOCKEDEVENT_MOZILLA_1_8_BRANCH

  nsDOMPopupBlockedEvent_MOZILLA_1_8_BRANCH();

private:
  ~nsDOMPopupBlockedEvent_MOZILLA_1_8_BRANCH();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMPopupBlockedEvent_MOZILLA_1_8_BRANCH, nsIDOMPopupBlockedEvent_MOZILLA_1_8_BRANCH)

nsDOMPopupBlockedEvent_MOZILLA_1_8_BRANCH::nsDOMPopupBlockedEvent_MOZILLA_1_8_BRANCH()
{
  /* member initializers and constructor code */
}

nsDOMPopupBlockedEvent_MOZILLA_1_8_BRANCH::~nsDOMPopupBlockedEvent_MOZILLA_1_8_BRANCH()
{
  /* destructor code */
}

/* readonly attribute nsIDOMWindow requestingWindow; */
NS_IMETHODIMP nsDOMPopupBlockedEvent_MOZILLA_1_8_BRANCH::GetRequestingWindow(nsIDOMWindow * *aRequestingWindow)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void initPopupBlockedEvent (in DOMString typeArg, in boolean canBubbleArg, in boolean cancelableArg, in nsIDOMWindow requestingWindow, in nsIURI popupWindowURI, in DOMString popupWindowFeatures); */
NS_IMETHODIMP nsDOMPopupBlockedEvent_MOZILLA_1_8_BRANCH::InitPopupBlockedEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIDOMWindow *requestingWindow, nsIURI *popupWindowURI, const nsAString & popupWindowFeatures)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMPopupBlockedEvent_h__ */
