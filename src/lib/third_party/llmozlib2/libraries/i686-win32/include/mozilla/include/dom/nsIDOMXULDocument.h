/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/xul/nsIDOMXULDocument.idl
 */

#ifndef __gen_nsIDOMXULDocument_h__
#define __gen_nsIDOMXULDocument_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMXULCommandDispatcher; /* forward declaration */

class nsIObserver; /* forward declaration */


/* starting interface:    nsIDOMXULDocument */
#define NS_IDOMXULDOCUMENT_IID_STR "e64bb081-13ba-430e-ab70-73a9f1d3de58"

#define NS_IDOMXULDOCUMENT_IID \
  {0xe64bb081, 0x13ba, 0x430e, \
    { 0xab, 0x70, 0x73, 0xa9, 0xf1, 0xd3, 0xde, 0x58 }}

class NS_NO_VTABLE nsIDOMXULDocument : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMXULDOCUMENT_IID)

  /* attribute nsIDOMNode popupNode; */
  NS_IMETHOD GetPopupNode(nsIDOMNode * *aPopupNode) = 0;
  NS_IMETHOD SetPopupNode(nsIDOMNode * aPopupNode) = 0;

  /* attribute nsIDOMNode tooltipNode; */
  NS_IMETHOD GetTooltipNode(nsIDOMNode * *aTooltipNode) = 0;
  NS_IMETHOD SetTooltipNode(nsIDOMNode * aTooltipNode) = 0;

  /* readonly attribute nsIDOMXULCommandDispatcher commandDispatcher; */
  NS_IMETHOD GetCommandDispatcher(nsIDOMXULCommandDispatcher * *aCommandDispatcher) = 0;

  /* readonly attribute long width; */
  NS_IMETHOD GetWidth(PRInt32 *aWidth) = 0;

  /* readonly attribute long height; */
  NS_IMETHOD GetHeight(PRInt32 *aHeight) = 0;

  /* nsIDOMNodeList getElementsByAttribute (in DOMString name, in DOMString value); */
  NS_IMETHOD GetElementsByAttribute(const nsAString & name, const nsAString & value, nsIDOMNodeList **_retval) = 0;

  /* void addBroadcastListenerFor (in nsIDOMElement broadcaster, in nsIDOMElement observer, in DOMString attr); */
  NS_IMETHOD AddBroadcastListenerFor(nsIDOMElement *broadcaster, nsIDOMElement *observer, const nsAString & attr) = 0;

  /* void removeBroadcastListenerFor (in nsIDOMElement broadcaster, in nsIDOMElement observer, in DOMString attr); */
  NS_IMETHOD RemoveBroadcastListenerFor(nsIDOMElement *broadcaster, nsIDOMElement *observer, const nsAString & attr) = 0;

  /* void persist (in DOMString id, in DOMString attr); */
  NS_IMETHOD Persist(const nsAString & id, const nsAString & attr) = 0;

  /**
   * Loads a XUL overlay and merges it with the current document, notifying an
   * observer when the merge is complete. 
   * @param   url
   *          The URL of the overlay to load and merge
   * @param   observer
   *          An object implementing nsIObserver that will be notified with a
   *          message of topic "xul-overlay-merged" when the merge is complete. 
   *          The subject parameter of |observe| will QI to a nsIURI - the URI 
   *          of the merged overlay. This parameter is optional and may be null.
   *
   * NOTICE:  In the 2.0 timeframe this API will change such that the 
   *          implementation will fire a DOMXULOverlayMerged event upon merge
   *          completion rather than notifying an observer. Do not rely on this
   *          API's behavior _not_ to change because it will!
   *          - Ben Goodger (8/23/2005)
   */
  /* void loadOverlay (in DOMString url, in nsIObserver aObserver); */
  NS_IMETHOD LoadOverlay(const nsAString & url, nsIObserver *aObserver) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMXULDOCUMENT \
  NS_IMETHOD GetPopupNode(nsIDOMNode * *aPopupNode); \
  NS_IMETHOD SetPopupNode(nsIDOMNode * aPopupNode); \
  NS_IMETHOD GetTooltipNode(nsIDOMNode * *aTooltipNode); \
  NS_IMETHOD SetTooltipNode(nsIDOMNode * aTooltipNode); \
  NS_IMETHOD GetCommandDispatcher(nsIDOMXULCommandDispatcher * *aCommandDispatcher); \
  NS_IMETHOD GetWidth(PRInt32 *aWidth); \
  NS_IMETHOD GetHeight(PRInt32 *aHeight); \
  NS_IMETHOD GetElementsByAttribute(const nsAString & name, const nsAString & value, nsIDOMNodeList **_retval); \
  NS_IMETHOD AddBroadcastListenerFor(nsIDOMElement *broadcaster, nsIDOMElement *observer, const nsAString & attr); \
  NS_IMETHOD RemoveBroadcastListenerFor(nsIDOMElement *broadcaster, nsIDOMElement *observer, const nsAString & attr); \
  NS_IMETHOD Persist(const nsAString & id, const nsAString & attr); \
  NS_IMETHOD LoadOverlay(const nsAString & url, nsIObserver *aObserver); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMXULDOCUMENT(_to) \
  NS_IMETHOD GetPopupNode(nsIDOMNode * *aPopupNode) { return _to GetPopupNode(aPopupNode); } \
  NS_IMETHOD SetPopupNode(nsIDOMNode * aPopupNode) { return _to SetPopupNode(aPopupNode); } \
  NS_IMETHOD GetTooltipNode(nsIDOMNode * *aTooltipNode) { return _to GetTooltipNode(aTooltipNode); } \
  NS_IMETHOD SetTooltipNode(nsIDOMNode * aTooltipNode) { return _to SetTooltipNode(aTooltipNode); } \
  NS_IMETHOD GetCommandDispatcher(nsIDOMXULCommandDispatcher * *aCommandDispatcher) { return _to GetCommandDispatcher(aCommandDispatcher); } \
  NS_IMETHOD GetWidth(PRInt32 *aWidth) { return _to GetWidth(aWidth); } \
  NS_IMETHOD GetHeight(PRInt32 *aHeight) { return _to GetHeight(aHeight); } \
  NS_IMETHOD GetElementsByAttribute(const nsAString & name, const nsAString & value, nsIDOMNodeList **_retval) { return _to GetElementsByAttribute(name, value, _retval); } \
  NS_IMETHOD AddBroadcastListenerFor(nsIDOMElement *broadcaster, nsIDOMElement *observer, const nsAString & attr) { return _to AddBroadcastListenerFor(broadcaster, observer, attr); } \
  NS_IMETHOD RemoveBroadcastListenerFor(nsIDOMElement *broadcaster, nsIDOMElement *observer, const nsAString & attr) { return _to RemoveBroadcastListenerFor(broadcaster, observer, attr); } \
  NS_IMETHOD Persist(const nsAString & id, const nsAString & attr) { return _to Persist(id, attr); } \
  NS_IMETHOD LoadOverlay(const nsAString & url, nsIObserver *aObserver) { return _to LoadOverlay(url, aObserver); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMXULDOCUMENT(_to) \
  NS_IMETHOD GetPopupNode(nsIDOMNode * *aPopupNode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPopupNode(aPopupNode); } \
  NS_IMETHOD SetPopupNode(nsIDOMNode * aPopupNode) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPopupNode(aPopupNode); } \
  NS_IMETHOD GetTooltipNode(nsIDOMNode * *aTooltipNode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTooltipNode(aTooltipNode); } \
  NS_IMETHOD SetTooltipNode(nsIDOMNode * aTooltipNode) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTooltipNode(aTooltipNode); } \
  NS_IMETHOD GetCommandDispatcher(nsIDOMXULCommandDispatcher * *aCommandDispatcher) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCommandDispatcher(aCommandDispatcher); } \
  NS_IMETHOD GetWidth(PRInt32 *aWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetWidth(aWidth); } \
  NS_IMETHOD GetHeight(PRInt32 *aHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHeight(aHeight); } \
  NS_IMETHOD GetElementsByAttribute(const nsAString & name, const nsAString & value, nsIDOMNodeList **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetElementsByAttribute(name, value, _retval); } \
  NS_IMETHOD AddBroadcastListenerFor(nsIDOMElement *broadcaster, nsIDOMElement *observer, const nsAString & attr) { return !_to ? NS_ERROR_NULL_POINTER : _to->AddBroadcastListenerFor(broadcaster, observer, attr); } \
  NS_IMETHOD RemoveBroadcastListenerFor(nsIDOMElement *broadcaster, nsIDOMElement *observer, const nsAString & attr) { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveBroadcastListenerFor(broadcaster, observer, attr); } \
  NS_IMETHOD Persist(const nsAString & id, const nsAString & attr) { return !_to ? NS_ERROR_NULL_POINTER : _to->Persist(id, attr); } \
  NS_IMETHOD LoadOverlay(const nsAString & url, nsIObserver *aObserver) { return !_to ? NS_ERROR_NULL_POINTER : _to->LoadOverlay(url, aObserver); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMXULDocument : public nsIDOMXULDocument
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMXULDOCUMENT

  nsDOMXULDocument();

private:
  ~nsDOMXULDocument();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMXULDocument, nsIDOMXULDocument)

nsDOMXULDocument::nsDOMXULDocument()
{
  /* member initializers and constructor code */
}

nsDOMXULDocument::~nsDOMXULDocument()
{
  /* destructor code */
}

/* attribute nsIDOMNode popupNode; */
NS_IMETHODIMP nsDOMXULDocument::GetPopupNode(nsIDOMNode * *aPopupNode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMXULDocument::SetPopupNode(nsIDOMNode * aPopupNode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIDOMNode tooltipNode; */
NS_IMETHODIMP nsDOMXULDocument::GetTooltipNode(nsIDOMNode * *aTooltipNode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMXULDocument::SetTooltipNode(nsIDOMNode * aTooltipNode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMXULCommandDispatcher commandDispatcher; */
NS_IMETHODIMP nsDOMXULDocument::GetCommandDispatcher(nsIDOMXULCommandDispatcher * *aCommandDispatcher)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long width; */
NS_IMETHODIMP nsDOMXULDocument::GetWidth(PRInt32 *aWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long height; */
NS_IMETHODIMP nsDOMXULDocument::GetHeight(PRInt32 *aHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMNodeList getElementsByAttribute (in DOMString name, in DOMString value); */
NS_IMETHODIMP nsDOMXULDocument::GetElementsByAttribute(const nsAString & name, const nsAString & value, nsIDOMNodeList **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void addBroadcastListenerFor (in nsIDOMElement broadcaster, in nsIDOMElement observer, in DOMString attr); */
NS_IMETHODIMP nsDOMXULDocument::AddBroadcastListenerFor(nsIDOMElement *broadcaster, nsIDOMElement *observer, const nsAString & attr)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeBroadcastListenerFor (in nsIDOMElement broadcaster, in nsIDOMElement observer, in DOMString attr); */
NS_IMETHODIMP nsDOMXULDocument::RemoveBroadcastListenerFor(nsIDOMElement *broadcaster, nsIDOMElement *observer, const nsAString & attr)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void persist (in DOMString id, in DOMString attr); */
NS_IMETHODIMP nsDOMXULDocument::Persist(const nsAString & id, const nsAString & attr)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void loadOverlay (in DOMString url, in nsIObserver aObserver); */
NS_IMETHODIMP nsDOMXULDocument::LoadOverlay(const nsAString & url, nsIObserver *aObserver)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMXULDocument2 */
#define NS_IDOMXULDOCUMENT2_IID_STR "87545e48-b966-4579-829a-ca01b0fca9b3"

#define NS_IDOMXULDOCUMENT2_IID \
  {0x87545e48, 0xb966, 0x4579, \
    { 0x82, 0x9a, 0xca, 0x01, 0xb0, 0xfc, 0xa9, 0xb3 }}

/**
 * This interface is a hack for bug 319434 since we have to preserve the
 * nsIDOMXULDocument interface on the branch. It provides non-scriptable
 * functions that don't do security checks to access the popup and tooltip
 * nodes.
 *
 * @status TEMPORARY
 */
class NS_NO_VTABLE nsIDOMXULDocument2 : public nsIDOMXULDocument {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMXULDOCUMENT2_IID)

  /* nsIDOMNode trustedGetPopupNode (); */
  NS_IMETHOD TrustedGetPopupNode(nsIDOMNode **_retval) = 0;

  /* nsIDOMNode trustedGetTooltipNode (); */
  NS_IMETHOD TrustedGetTooltipNode(nsIDOMNode **_retval) = 0;

  /* attribute nsIDOMEvent trustedPopupEvent; */
  NS_IMETHOD GetTrustedPopupEvent(nsIDOMEvent * *aTrustedPopupEvent) = 0;
  NS_IMETHOD SetTrustedPopupEvent(nsIDOMEvent * aTrustedPopupEvent) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMXULDOCUMENT2 \
  NS_IMETHOD TrustedGetPopupNode(nsIDOMNode **_retval); \
  NS_IMETHOD TrustedGetTooltipNode(nsIDOMNode **_retval); \
  NS_IMETHOD GetTrustedPopupEvent(nsIDOMEvent * *aTrustedPopupEvent); \
  NS_IMETHOD SetTrustedPopupEvent(nsIDOMEvent * aTrustedPopupEvent); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMXULDOCUMENT2(_to) \
  NS_IMETHOD TrustedGetPopupNode(nsIDOMNode **_retval) { return _to TrustedGetPopupNode(_retval); } \
  NS_IMETHOD TrustedGetTooltipNode(nsIDOMNode **_retval) { return _to TrustedGetTooltipNode(_retval); } \
  NS_IMETHOD GetTrustedPopupEvent(nsIDOMEvent * *aTrustedPopupEvent) { return _to GetTrustedPopupEvent(aTrustedPopupEvent); } \
  NS_IMETHOD SetTrustedPopupEvent(nsIDOMEvent * aTrustedPopupEvent) { return _to SetTrustedPopupEvent(aTrustedPopupEvent); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMXULDOCUMENT2(_to) \
  NS_IMETHOD TrustedGetPopupNode(nsIDOMNode **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->TrustedGetPopupNode(_retval); } \
  NS_IMETHOD TrustedGetTooltipNode(nsIDOMNode **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->TrustedGetTooltipNode(_retval); } \
  NS_IMETHOD GetTrustedPopupEvent(nsIDOMEvent * *aTrustedPopupEvent) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTrustedPopupEvent(aTrustedPopupEvent); } \
  NS_IMETHOD SetTrustedPopupEvent(nsIDOMEvent * aTrustedPopupEvent) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTrustedPopupEvent(aTrustedPopupEvent); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMXULDocument2 : public nsIDOMXULDocument2
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMXULDOCUMENT2

  nsDOMXULDocument2();

private:
  ~nsDOMXULDocument2();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMXULDocument2, nsIDOMXULDocument2)

nsDOMXULDocument2::nsDOMXULDocument2()
{
  /* member initializers and constructor code */
}

nsDOMXULDocument2::~nsDOMXULDocument2()
{
  /* destructor code */
}

/* nsIDOMNode trustedGetPopupNode (); */
NS_IMETHODIMP nsDOMXULDocument2::TrustedGetPopupNode(nsIDOMNode **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMNode trustedGetTooltipNode (); */
NS_IMETHODIMP nsDOMXULDocument2::TrustedGetTooltipNode(nsIDOMNode **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIDOMEvent trustedPopupEvent; */
NS_IMETHODIMP nsDOMXULDocument2::GetTrustedPopupEvent(nsIDOMEvent * *aTrustedPopupEvent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMXULDocument2::SetTrustedPopupEvent(nsIDOMEvent * aTrustedPopupEvent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMXULDocument_MOZILLA_1_8_BRANCH */
#define NS_IDOMXULDOCUMENT_MOZILLA_1_8_BRANCH_IID_STR "8cc6316d-ecbc-4c5f-8fda-28c91fb56e3d"

#define NS_IDOMXULDOCUMENT_MOZILLA_1_8_BRANCH_IID \
  {0x8cc6316d, 0xecbc, 0x4c5f, \
    { 0x8f, 0xda, 0x28, 0xc9, 0x1f, 0xb5, 0x6e, 0x3d }}

/**
 * This interface adds the popup event for the inline spell checker on brach,
 * because we can't change nsIDOMXULDoucument. We can't use nsIDOMXULDocument2
 * because it isn't scriptable.
 *
 * @status TEMPORARY
 */
class NS_NO_VTABLE nsIDOMXULDocument_MOZILLA_1_8_BRANCH : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMXULDOCUMENT_MOZILLA_1_8_BRANCH_IID)

  /**
   * These attributes correspond to trustedGetPopupNode().rangeOffset and
   * rangeParent. They will help you find where in the DOM the popup is
   * happening. Can be accessed from chrome only, and only during a popup
   * event. Accessing any other time will be an error.
   */
  /* readonly attribute nsIDOMNode popupRangeParent; */
  NS_IMETHOD GetPopupRangeParent(nsIDOMNode * *aPopupRangeParent) = 0;

  /* readonly attribute long popupRangeOffset; */
  NS_IMETHOD GetPopupRangeOffset(PRInt32 *aPopupRangeOffset) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMXULDOCUMENT_MOZILLA_1_8_BRANCH \
  NS_IMETHOD GetPopupRangeParent(nsIDOMNode * *aPopupRangeParent); \
  NS_IMETHOD GetPopupRangeOffset(PRInt32 *aPopupRangeOffset); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMXULDOCUMENT_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD GetPopupRangeParent(nsIDOMNode * *aPopupRangeParent) { return _to GetPopupRangeParent(aPopupRangeParent); } \
  NS_IMETHOD GetPopupRangeOffset(PRInt32 *aPopupRangeOffset) { return _to GetPopupRangeOffset(aPopupRangeOffset); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMXULDOCUMENT_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD GetPopupRangeParent(nsIDOMNode * *aPopupRangeParent) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPopupRangeParent(aPopupRangeParent); } \
  NS_IMETHOD GetPopupRangeOffset(PRInt32 *aPopupRangeOffset) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPopupRangeOffset(aPopupRangeOffset); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMXULDocument_MOZILLA_1_8_BRANCH : public nsIDOMXULDocument_MOZILLA_1_8_BRANCH
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMXULDOCUMENT_MOZILLA_1_8_BRANCH

  nsDOMXULDocument_MOZILLA_1_8_BRANCH();

private:
  ~nsDOMXULDocument_MOZILLA_1_8_BRANCH();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMXULDocument_MOZILLA_1_8_BRANCH, nsIDOMXULDocument_MOZILLA_1_8_BRANCH)

nsDOMXULDocument_MOZILLA_1_8_BRANCH::nsDOMXULDocument_MOZILLA_1_8_BRANCH()
{
  /* member initializers and constructor code */
}

nsDOMXULDocument_MOZILLA_1_8_BRANCH::~nsDOMXULDocument_MOZILLA_1_8_BRANCH()
{
  /* destructor code */
}

/* readonly attribute nsIDOMNode popupRangeParent; */
NS_IMETHODIMP nsDOMXULDocument_MOZILLA_1_8_BRANCH::GetPopupRangeParent(nsIDOMNode * *aPopupRangeParent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long popupRangeOffset; */
NS_IMETHODIMP nsDOMXULDocument_MOZILLA_1_8_BRANCH::GetPopupRangeOffset(PRInt32 *aPopupRangeOffset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMXULDocument_h__ */
