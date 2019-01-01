/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/xul/base/public/nsIPopupBoxObject.idl
 */

#ifndef __gen_nsIPopupBoxObject_h__
#define __gen_nsIPopupBoxObject_h__


#ifndef __gen_nsIBoxObject_h__
#include "nsIBoxObject.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMElement; /* forward declaration */


/* starting interface:    nsIPopupBoxObject */
#define NS_IPOPUPBOXOBJECT_IID_STR "33c60e14-5150-4876-9a96-2732557e6895"

#define NS_IPOPUPBOXOBJECT_IID \
  {0x33c60e14, 0x5150, 0x4876, \
    { 0x9a, 0x96, 0x27, 0x32, 0x55, 0x7e, 0x68, 0x95 }}

class NS_NO_VTABLE nsIPopupBoxObject : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPOPUPBOXOBJECT_IID)

  /* void showPopup (in nsIDOMElement srcContent, in nsIDOMElement popupContent, in long xpos, in long ypos, in wstring popupType, in wstring anchorAlignment, in wstring popupAlignment); */
  NS_IMETHOD ShowPopup(nsIDOMElement *srcContent, nsIDOMElement *popupContent, PRInt32 xpos, PRInt32 ypos, const PRUnichar *popupType, const PRUnichar *anchorAlignment, const PRUnichar *popupAlignment) = 0;

  /* void hidePopup (); */
  NS_IMETHOD HidePopup(void) = 0;

  /** 
   * Allow the popup to automatically position itself.
   */
  /* attribute boolean autoPosition; */
  NS_IMETHOD GetAutoPosition(PRBool *aAutoPosition) = 0;
  NS_IMETHOD SetAutoPosition(PRBool aAutoPosition) = 0;

  /**
   * Allow the popup to eat all key events
   */
  /* void enableKeyboardNavigator (in boolean enableKeyboardNavigator); */
  NS_IMETHOD EnableKeyboardNavigator(PRBool enableKeyboardNavigator) = 0;

  /** 
   * Enable automatic popup dismissal
   */
  /* void enableRollup (in boolean enableRollup); */
  NS_IMETHOD EnableRollup(PRBool enableRollup) = 0;

  /** 
   * Size the popup to the given dimensions
   */
  /* void sizeTo (in long width, in long height); */
  NS_IMETHOD SizeTo(PRInt32 width, PRInt32 height) = 0;

  /**
   * Move the popup to a point on screen
   */
  /* void moveTo (in long left, in long top); */
  NS_IMETHOD MoveTo(PRInt32 left, PRInt32 top) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPOPUPBOXOBJECT \
  NS_IMETHOD ShowPopup(nsIDOMElement *srcContent, nsIDOMElement *popupContent, PRInt32 xpos, PRInt32 ypos, const PRUnichar *popupType, const PRUnichar *anchorAlignment, const PRUnichar *popupAlignment); \
  NS_IMETHOD HidePopup(void); \
  NS_IMETHOD GetAutoPosition(PRBool *aAutoPosition); \
  NS_IMETHOD SetAutoPosition(PRBool aAutoPosition); \
  NS_IMETHOD EnableKeyboardNavigator(PRBool enableKeyboardNavigator); \
  NS_IMETHOD EnableRollup(PRBool enableRollup); \
  NS_IMETHOD SizeTo(PRInt32 width, PRInt32 height); \
  NS_IMETHOD MoveTo(PRInt32 left, PRInt32 top); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPOPUPBOXOBJECT(_to) \
  NS_IMETHOD ShowPopup(nsIDOMElement *srcContent, nsIDOMElement *popupContent, PRInt32 xpos, PRInt32 ypos, const PRUnichar *popupType, const PRUnichar *anchorAlignment, const PRUnichar *popupAlignment) { return _to ShowPopup(srcContent, popupContent, xpos, ypos, popupType, anchorAlignment, popupAlignment); } \
  NS_IMETHOD HidePopup(void) { return _to HidePopup(); } \
  NS_IMETHOD GetAutoPosition(PRBool *aAutoPosition) { return _to GetAutoPosition(aAutoPosition); } \
  NS_IMETHOD SetAutoPosition(PRBool aAutoPosition) { return _to SetAutoPosition(aAutoPosition); } \
  NS_IMETHOD EnableKeyboardNavigator(PRBool enableKeyboardNavigator) { return _to EnableKeyboardNavigator(enableKeyboardNavigator); } \
  NS_IMETHOD EnableRollup(PRBool enableRollup) { return _to EnableRollup(enableRollup); } \
  NS_IMETHOD SizeTo(PRInt32 width, PRInt32 height) { return _to SizeTo(width, height); } \
  NS_IMETHOD MoveTo(PRInt32 left, PRInt32 top) { return _to MoveTo(left, top); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPOPUPBOXOBJECT(_to) \
  NS_IMETHOD ShowPopup(nsIDOMElement *srcContent, nsIDOMElement *popupContent, PRInt32 xpos, PRInt32 ypos, const PRUnichar *popupType, const PRUnichar *anchorAlignment, const PRUnichar *popupAlignment) { return !_to ? NS_ERROR_NULL_POINTER : _to->ShowPopup(srcContent, popupContent, xpos, ypos, popupType, anchorAlignment, popupAlignment); } \
  NS_IMETHOD HidePopup(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->HidePopup(); } \
  NS_IMETHOD GetAutoPosition(PRBool *aAutoPosition) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAutoPosition(aAutoPosition); } \
  NS_IMETHOD SetAutoPosition(PRBool aAutoPosition) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetAutoPosition(aAutoPosition); } \
  NS_IMETHOD EnableKeyboardNavigator(PRBool enableKeyboardNavigator) { return !_to ? NS_ERROR_NULL_POINTER : _to->EnableKeyboardNavigator(enableKeyboardNavigator); } \
  NS_IMETHOD EnableRollup(PRBool enableRollup) { return !_to ? NS_ERROR_NULL_POINTER : _to->EnableRollup(enableRollup); } \
  NS_IMETHOD SizeTo(PRInt32 width, PRInt32 height) { return !_to ? NS_ERROR_NULL_POINTER : _to->SizeTo(width, height); } \
  NS_IMETHOD MoveTo(PRInt32 left, PRInt32 top) { return !_to ? NS_ERROR_NULL_POINTER : _to->MoveTo(left, top); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsPopupBoxObject : public nsIPopupBoxObject
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPOPUPBOXOBJECT

  nsPopupBoxObject();

private:
  ~nsPopupBoxObject();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsPopupBoxObject, nsIPopupBoxObject)

nsPopupBoxObject::nsPopupBoxObject()
{
  /* member initializers and constructor code */
}

nsPopupBoxObject::~nsPopupBoxObject()
{
  /* destructor code */
}

/* void showPopup (in nsIDOMElement srcContent, in nsIDOMElement popupContent, in long xpos, in long ypos, in wstring popupType, in wstring anchorAlignment, in wstring popupAlignment); */
NS_IMETHODIMP nsPopupBoxObject::ShowPopup(nsIDOMElement *srcContent, nsIDOMElement *popupContent, PRInt32 xpos, PRInt32 ypos, const PRUnichar *popupType, const PRUnichar *anchorAlignment, const PRUnichar *popupAlignment)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void hidePopup (); */
NS_IMETHODIMP nsPopupBoxObject::HidePopup()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean autoPosition; */
NS_IMETHODIMP nsPopupBoxObject::GetAutoPosition(PRBool *aAutoPosition)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPopupBoxObject::SetAutoPosition(PRBool aAutoPosition)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void enableKeyboardNavigator (in boolean enableKeyboardNavigator); */
NS_IMETHODIMP nsPopupBoxObject::EnableKeyboardNavigator(PRBool enableKeyboardNavigator)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void enableRollup (in boolean enableRollup); */
NS_IMETHODIMP nsPopupBoxObject::EnableRollup(PRBool enableRollup)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void sizeTo (in long width, in long height); */
NS_IMETHODIMP nsPopupBoxObject::SizeTo(PRInt32 width, PRInt32 height)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void moveTo (in long left, in long top); */
NS_IMETHODIMP nsPopupBoxObject::MoveTo(PRInt32 left, PRInt32 top)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

nsresult
NS_NewPopupBoxObject(nsIBoxObject** aResult);

#endif /* __gen_nsIPopupBoxObject_h__ */
