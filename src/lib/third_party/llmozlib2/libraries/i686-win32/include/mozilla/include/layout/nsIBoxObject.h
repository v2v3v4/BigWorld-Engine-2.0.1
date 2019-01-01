/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/xul/base/public/nsIBoxObject.idl
 */

#ifndef __gen_nsIBoxObject_h__
#define __gen_nsIBoxObject_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsIBoxLayoutManager_h__
#include "nsIBoxLayoutManager.h"
#endif

#ifndef __gen_nsIBoxPaintManager_h__
#include "nsIBoxPaintManager.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMElement; /* forward declaration */


/* starting interface:    nsIBoxObject */
#define NS_IBOXOBJECT_IID_STR "caabf76f-9d35-401f-beac-3955817c645c"

#define NS_IBOXOBJECT_IID \
  {0xcaabf76f, 0x9d35, 0x401f, \
    { 0xbe, 0xac, 0x39, 0x55, 0x81, 0x7c, 0x64, 0x5c }}

class NS_NO_VTABLE nsIBoxObject : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IBOXOBJECT_IID)

  /* readonly attribute nsIDOMElement element; */
  NS_IMETHOD GetElement(nsIDOMElement * *aElement) = 0;

  /* attribute nsIBoxLayoutManager layoutManager; */
  NS_IMETHOD GetLayoutManager(nsIBoxLayoutManager * *aLayoutManager) = 0;
  NS_IMETHOD SetLayoutManager(nsIBoxLayoutManager * aLayoutManager) = 0;

  /* attribute nsIBoxPaintManager paintManager; */
  NS_IMETHOD GetPaintManager(nsIBoxPaintManager * *aPaintManager) = 0;
  NS_IMETHOD SetPaintManager(nsIBoxPaintManager * aPaintManager) = 0;

  /* readonly attribute long x; */
  NS_IMETHOD GetX(PRInt32 *aX) = 0;

  /* readonly attribute long y; */
  NS_IMETHOD GetY(PRInt32 *aY) = 0;

  /* readonly attribute long screenX; */
  NS_IMETHOD GetScreenX(PRInt32 *aScreenX) = 0;

  /* readonly attribute long screenY; */
  NS_IMETHOD GetScreenY(PRInt32 *aScreenY) = 0;

  /* readonly attribute long width; */
  NS_IMETHOD GetWidth(PRInt32 *aWidth) = 0;

  /* readonly attribute long height; */
  NS_IMETHOD GetHeight(PRInt32 *aHeight) = 0;

  /* nsISupports getPropertyAsSupports (in wstring propertyName); */
  NS_IMETHOD GetPropertyAsSupports(const PRUnichar *propertyName, nsISupports **_retval) = 0;

  /* void setPropertyAsSupports (in wstring propertyName, in nsISupports value); */
  NS_IMETHOD SetPropertyAsSupports(const PRUnichar *propertyName, nsISupports *value) = 0;

  /* wstring getProperty (in wstring propertyName); */
  NS_IMETHOD GetProperty(const PRUnichar *propertyName, PRUnichar **_retval) = 0;

  /* void setProperty (in wstring propertyName, in wstring propertyValue); */
  NS_IMETHOD SetProperty(const PRUnichar *propertyName, const PRUnichar *propertyValue) = 0;

  /* void removeProperty (in wstring propertyName); */
  NS_IMETHOD RemoveProperty(const PRUnichar *propertyName) = 0;

  /* readonly attribute nsIDOMElement parentBox; */
  NS_IMETHOD GetParentBox(nsIDOMElement * *aParentBox) = 0;

  /* readonly attribute nsIDOMElement firstChild; */
  NS_IMETHOD GetFirstChild(nsIDOMElement * *aFirstChild) = 0;

  /* readonly attribute nsIDOMElement lastChild; */
  NS_IMETHOD GetLastChild(nsIDOMElement * *aLastChild) = 0;

  /* readonly attribute nsIDOMElement nextSibling; */
  NS_IMETHOD GetNextSibling(nsIDOMElement * *aNextSibling) = 0;

  /* readonly attribute nsIDOMElement previousSibling; */
  NS_IMETHOD GetPreviousSibling(nsIDOMElement * *aPreviousSibling) = 0;

  /* wstring getLookAndFeelMetric (in wstring propertyName); */
  NS_IMETHOD GetLookAndFeelMetric(const PRUnichar *propertyName, PRUnichar **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIBOXOBJECT \
  NS_IMETHOD GetElement(nsIDOMElement * *aElement); \
  NS_IMETHOD GetLayoutManager(nsIBoxLayoutManager * *aLayoutManager); \
  NS_IMETHOD SetLayoutManager(nsIBoxLayoutManager * aLayoutManager); \
  NS_IMETHOD GetPaintManager(nsIBoxPaintManager * *aPaintManager); \
  NS_IMETHOD SetPaintManager(nsIBoxPaintManager * aPaintManager); \
  NS_IMETHOD GetX(PRInt32 *aX); \
  NS_IMETHOD GetY(PRInt32 *aY); \
  NS_IMETHOD GetScreenX(PRInt32 *aScreenX); \
  NS_IMETHOD GetScreenY(PRInt32 *aScreenY); \
  NS_IMETHOD GetWidth(PRInt32 *aWidth); \
  NS_IMETHOD GetHeight(PRInt32 *aHeight); \
  NS_IMETHOD GetPropertyAsSupports(const PRUnichar *propertyName, nsISupports **_retval); \
  NS_IMETHOD SetPropertyAsSupports(const PRUnichar *propertyName, nsISupports *value); \
  NS_IMETHOD GetProperty(const PRUnichar *propertyName, PRUnichar **_retval); \
  NS_IMETHOD SetProperty(const PRUnichar *propertyName, const PRUnichar *propertyValue); \
  NS_IMETHOD RemoveProperty(const PRUnichar *propertyName); \
  NS_IMETHOD GetParentBox(nsIDOMElement * *aParentBox); \
  NS_IMETHOD GetFirstChild(nsIDOMElement * *aFirstChild); \
  NS_IMETHOD GetLastChild(nsIDOMElement * *aLastChild); \
  NS_IMETHOD GetNextSibling(nsIDOMElement * *aNextSibling); \
  NS_IMETHOD GetPreviousSibling(nsIDOMElement * *aPreviousSibling); \
  NS_IMETHOD GetLookAndFeelMetric(const PRUnichar *propertyName, PRUnichar **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIBOXOBJECT(_to) \
  NS_IMETHOD GetElement(nsIDOMElement * *aElement) { return _to GetElement(aElement); } \
  NS_IMETHOD GetLayoutManager(nsIBoxLayoutManager * *aLayoutManager) { return _to GetLayoutManager(aLayoutManager); } \
  NS_IMETHOD SetLayoutManager(nsIBoxLayoutManager * aLayoutManager) { return _to SetLayoutManager(aLayoutManager); } \
  NS_IMETHOD GetPaintManager(nsIBoxPaintManager * *aPaintManager) { return _to GetPaintManager(aPaintManager); } \
  NS_IMETHOD SetPaintManager(nsIBoxPaintManager * aPaintManager) { return _to SetPaintManager(aPaintManager); } \
  NS_IMETHOD GetX(PRInt32 *aX) { return _to GetX(aX); } \
  NS_IMETHOD GetY(PRInt32 *aY) { return _to GetY(aY); } \
  NS_IMETHOD GetScreenX(PRInt32 *aScreenX) { return _to GetScreenX(aScreenX); } \
  NS_IMETHOD GetScreenY(PRInt32 *aScreenY) { return _to GetScreenY(aScreenY); } \
  NS_IMETHOD GetWidth(PRInt32 *aWidth) { return _to GetWidth(aWidth); } \
  NS_IMETHOD GetHeight(PRInt32 *aHeight) { return _to GetHeight(aHeight); } \
  NS_IMETHOD GetPropertyAsSupports(const PRUnichar *propertyName, nsISupports **_retval) { return _to GetPropertyAsSupports(propertyName, _retval); } \
  NS_IMETHOD SetPropertyAsSupports(const PRUnichar *propertyName, nsISupports *value) { return _to SetPropertyAsSupports(propertyName, value); } \
  NS_IMETHOD GetProperty(const PRUnichar *propertyName, PRUnichar **_retval) { return _to GetProperty(propertyName, _retval); } \
  NS_IMETHOD SetProperty(const PRUnichar *propertyName, const PRUnichar *propertyValue) { return _to SetProperty(propertyName, propertyValue); } \
  NS_IMETHOD RemoveProperty(const PRUnichar *propertyName) { return _to RemoveProperty(propertyName); } \
  NS_IMETHOD GetParentBox(nsIDOMElement * *aParentBox) { return _to GetParentBox(aParentBox); } \
  NS_IMETHOD GetFirstChild(nsIDOMElement * *aFirstChild) { return _to GetFirstChild(aFirstChild); } \
  NS_IMETHOD GetLastChild(nsIDOMElement * *aLastChild) { return _to GetLastChild(aLastChild); } \
  NS_IMETHOD GetNextSibling(nsIDOMElement * *aNextSibling) { return _to GetNextSibling(aNextSibling); } \
  NS_IMETHOD GetPreviousSibling(nsIDOMElement * *aPreviousSibling) { return _to GetPreviousSibling(aPreviousSibling); } \
  NS_IMETHOD GetLookAndFeelMetric(const PRUnichar *propertyName, PRUnichar **_retval) { return _to GetLookAndFeelMetric(propertyName, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIBOXOBJECT(_to) \
  NS_IMETHOD GetElement(nsIDOMElement * *aElement) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetElement(aElement); } \
  NS_IMETHOD GetLayoutManager(nsIBoxLayoutManager * *aLayoutManager) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLayoutManager(aLayoutManager); } \
  NS_IMETHOD SetLayoutManager(nsIBoxLayoutManager * aLayoutManager) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetLayoutManager(aLayoutManager); } \
  NS_IMETHOD GetPaintManager(nsIBoxPaintManager * *aPaintManager) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPaintManager(aPaintManager); } \
  NS_IMETHOD SetPaintManager(nsIBoxPaintManager * aPaintManager) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPaintManager(aPaintManager); } \
  NS_IMETHOD GetX(PRInt32 *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD GetY(PRInt32 *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD GetScreenX(PRInt32 *aScreenX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScreenX(aScreenX); } \
  NS_IMETHOD GetScreenY(PRInt32 *aScreenY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScreenY(aScreenY); } \
  NS_IMETHOD GetWidth(PRInt32 *aWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetWidth(aWidth); } \
  NS_IMETHOD GetHeight(PRInt32 *aHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHeight(aHeight); } \
  NS_IMETHOD GetPropertyAsSupports(const PRUnichar *propertyName, nsISupports **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPropertyAsSupports(propertyName, _retval); } \
  NS_IMETHOD SetPropertyAsSupports(const PRUnichar *propertyName, nsISupports *value) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPropertyAsSupports(propertyName, value); } \
  NS_IMETHOD GetProperty(const PRUnichar *propertyName, PRUnichar **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetProperty(propertyName, _retval); } \
  NS_IMETHOD SetProperty(const PRUnichar *propertyName, const PRUnichar *propertyValue) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetProperty(propertyName, propertyValue); } \
  NS_IMETHOD RemoveProperty(const PRUnichar *propertyName) { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveProperty(propertyName); } \
  NS_IMETHOD GetParentBox(nsIDOMElement * *aParentBox) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetParentBox(aParentBox); } \
  NS_IMETHOD GetFirstChild(nsIDOMElement * *aFirstChild) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFirstChild(aFirstChild); } \
  NS_IMETHOD GetLastChild(nsIDOMElement * *aLastChild) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLastChild(aLastChild); } \
  NS_IMETHOD GetNextSibling(nsIDOMElement * *aNextSibling) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNextSibling(aNextSibling); } \
  NS_IMETHOD GetPreviousSibling(nsIDOMElement * *aPreviousSibling) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPreviousSibling(aPreviousSibling); } \
  NS_IMETHOD GetLookAndFeelMetric(const PRUnichar *propertyName, PRUnichar **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLookAndFeelMetric(propertyName, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsBoxObject : public nsIBoxObject
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIBOXOBJECT

  nsBoxObject();

private:
  ~nsBoxObject();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsBoxObject, nsIBoxObject)

nsBoxObject::nsBoxObject()
{
  /* member initializers and constructor code */
}

nsBoxObject::~nsBoxObject()
{
  /* destructor code */
}

/* readonly attribute nsIDOMElement element; */
NS_IMETHODIMP nsBoxObject::GetElement(nsIDOMElement * *aElement)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIBoxLayoutManager layoutManager; */
NS_IMETHODIMP nsBoxObject::GetLayoutManager(nsIBoxLayoutManager * *aLayoutManager)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsBoxObject::SetLayoutManager(nsIBoxLayoutManager * aLayoutManager)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIBoxPaintManager paintManager; */
NS_IMETHODIMP nsBoxObject::GetPaintManager(nsIBoxPaintManager * *aPaintManager)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsBoxObject::SetPaintManager(nsIBoxPaintManager * aPaintManager)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long x; */
NS_IMETHODIMP nsBoxObject::GetX(PRInt32 *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long y; */
NS_IMETHODIMP nsBoxObject::GetY(PRInt32 *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long screenX; */
NS_IMETHODIMP nsBoxObject::GetScreenX(PRInt32 *aScreenX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long screenY; */
NS_IMETHODIMP nsBoxObject::GetScreenY(PRInt32 *aScreenY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long width; */
NS_IMETHODIMP nsBoxObject::GetWidth(PRInt32 *aWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long height; */
NS_IMETHODIMP nsBoxObject::GetHeight(PRInt32 *aHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsISupports getPropertyAsSupports (in wstring propertyName); */
NS_IMETHODIMP nsBoxObject::GetPropertyAsSupports(const PRUnichar *propertyName, nsISupports **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setPropertyAsSupports (in wstring propertyName, in nsISupports value); */
NS_IMETHODIMP nsBoxObject::SetPropertyAsSupports(const PRUnichar *propertyName, nsISupports *value)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* wstring getProperty (in wstring propertyName); */
NS_IMETHODIMP nsBoxObject::GetProperty(const PRUnichar *propertyName, PRUnichar **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setProperty (in wstring propertyName, in wstring propertyValue); */
NS_IMETHODIMP nsBoxObject::SetProperty(const PRUnichar *propertyName, const PRUnichar *propertyValue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeProperty (in wstring propertyName); */
NS_IMETHODIMP nsBoxObject::RemoveProperty(const PRUnichar *propertyName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMElement parentBox; */
NS_IMETHODIMP nsBoxObject::GetParentBox(nsIDOMElement * *aParentBox)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMElement firstChild; */
NS_IMETHODIMP nsBoxObject::GetFirstChild(nsIDOMElement * *aFirstChild)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMElement lastChild; */
NS_IMETHODIMP nsBoxObject::GetLastChild(nsIDOMElement * *aLastChild)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMElement nextSibling; */
NS_IMETHODIMP nsBoxObject::GetNextSibling(nsIDOMElement * *aNextSibling)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMElement previousSibling; */
NS_IMETHODIMP nsBoxObject::GetPreviousSibling(nsIDOMElement * *aPreviousSibling)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* wstring getLookAndFeelMetric (in wstring propertyName); */
NS_IMETHODIMP nsBoxObject::GetLookAndFeelMetric(const PRUnichar *propertyName, PRUnichar **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

nsresult
NS_NewBoxObject(nsIBoxObject** aResult);

#endif /* __gen_nsIBoxObject_h__ */
