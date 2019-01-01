/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/base/public/nsISelectionPrivate.idl
 */

#ifndef __gen_nsISelectionPrivate_h__
#define __gen_nsISelectionPrivate_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsISelectionListener_h__
#include "nsISelectionListener.h"
#endif

#ifndef __gen_nsIEnumerator_h__
#include "nsIEnumerator.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIFrameSelection; /* forward declaration */

class nsIDOMRange; /* forward declaration */

class nsISelectionListener; /* forward declaration */

class nsIFrame;
class nsIPresShell;
struct nsPoint;

/* starting interface:    nsISelectionPrivate */
#define NS_ISELECTIONPRIVATE_IID_STR "3225ca54-d7e1-4ff5-8ee9-091b0bfcda1f"

#define NS_ISELECTIONPRIVATE_IID \
  {0x3225ca54, 0xd7e1, 0x4ff5, \
    { 0x8e, 0xe9, 0x09, 0x1b, 0x0b, 0xfc, 0xda, 0x1f }}

class NS_NO_VTABLE nsISelectionPrivate : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISELECTIONPRIVATE_IID)

  enum { ENDOFPRECEDINGLINE = 0 };

  enum { STARTOFNEXTLINE = 1 };

  /* attribute boolean interlinePosition; */
  NS_IMETHOD GetInterlinePosition(PRBool *aInterlinePosition) = 0;
  NS_IMETHOD SetInterlinePosition(PRBool aInterlinePosition) = 0;

  /* void startBatchChanges (); */
  NS_IMETHOD StartBatchChanges(void) = 0;

  /* void endBatchChanges (); */
  NS_IMETHOD EndBatchChanges(void) = 0;

  /* nsIEnumerator getEnumerator (); */
  NS_IMETHOD GetEnumerator(nsIEnumerator **_retval) = 0;

  /* wstring toStringWithFormat (in string formatType, in unsigned long flags, in PRInt32 wrapColumn); */
  NS_IMETHOD ToStringWithFormat(const char *formatType, PRUint32 flags, PRInt32 wrapColumn, PRUnichar **_retval) = 0;

  /* void addSelectionListener (in nsISelectionListener newListener); */
  NS_IMETHOD AddSelectionListener(nsISelectionListener *newListener) = 0;

  /* void removeSelectionListener (in nsISelectionListener listenerToRemove); */
  NS_IMETHOD RemoveSelectionListener(nsISelectionListener *listenerToRemove) = 0;

  enum { TABLESELECTION_NONE = 0 };

  enum { TABLESELECTION_CELL = 1 };

  enum { TABLESELECTION_ROW = 2 };

  enum { TABLESELECTION_COLUMN = 3 };

  enum { TABLESELECTION_TABLE = 4 };

  enum { TABLESELECTION_ALLCELLS = 5 };

  /** Test if supplied range points to a single table element:
      *    Result is one of above constants. "None" means
      *    a table element isn't selected.
      */
  /* long getTableSelectionType (in nsIDOMRange range); */
  NS_IMETHOD GetTableSelectionType(nsIDOMRange *range, PRInt32 *_retval) = 0;

  /* [noscript] void setPresShell (in nsIPresShell aPresShell); */
  NS_IMETHOD SetPresShell(nsIPresShell * aPresShell) = 0;

  /* [noscript] attribute boolean canCacheFrameOffset; */
  NS_IMETHOD GetCanCacheFrameOffset(PRBool *aCanCacheFrameOffset) = 0;
  NS_IMETHOD SetCanCacheFrameOffset(PRBool aCanCacheFrameOffset) = 0;

  /* [noscript] void getCachedFrameOffset (in nsIFrame aFrame, in PRInt32 inOffset, in nsPointRef aPoint); */
  NS_IMETHOD GetCachedFrameOffset(nsIFrame * aFrame, PRInt32 inOffset, nsPoint & aPoint) = 0;

  /* [noscript] nsIFrameSelection getFrameSelection (); */
  NS_IMETHOD GetFrameSelection(nsIFrameSelection **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISELECTIONPRIVATE \
  NS_IMETHOD GetInterlinePosition(PRBool *aInterlinePosition); \
  NS_IMETHOD SetInterlinePosition(PRBool aInterlinePosition); \
  NS_IMETHOD StartBatchChanges(void); \
  NS_IMETHOD EndBatchChanges(void); \
  NS_IMETHOD GetEnumerator(nsIEnumerator **_retval); \
  NS_IMETHOD ToStringWithFormat(const char *formatType, PRUint32 flags, PRInt32 wrapColumn, PRUnichar **_retval); \
  NS_IMETHOD AddSelectionListener(nsISelectionListener *newListener); \
  NS_IMETHOD RemoveSelectionListener(nsISelectionListener *listenerToRemove); \
  NS_IMETHOD GetTableSelectionType(nsIDOMRange *range, PRInt32 *_retval); \
  NS_IMETHOD SetPresShell(nsIPresShell * aPresShell); \
  NS_IMETHOD GetCanCacheFrameOffset(PRBool *aCanCacheFrameOffset); \
  NS_IMETHOD SetCanCacheFrameOffset(PRBool aCanCacheFrameOffset); \
  NS_IMETHOD GetCachedFrameOffset(nsIFrame * aFrame, PRInt32 inOffset, nsPoint & aPoint); \
  NS_IMETHOD GetFrameSelection(nsIFrameSelection **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISELECTIONPRIVATE(_to) \
  NS_IMETHOD GetInterlinePosition(PRBool *aInterlinePosition) { return _to GetInterlinePosition(aInterlinePosition); } \
  NS_IMETHOD SetInterlinePosition(PRBool aInterlinePosition) { return _to SetInterlinePosition(aInterlinePosition); } \
  NS_IMETHOD StartBatchChanges(void) { return _to StartBatchChanges(); } \
  NS_IMETHOD EndBatchChanges(void) { return _to EndBatchChanges(); } \
  NS_IMETHOD GetEnumerator(nsIEnumerator **_retval) { return _to GetEnumerator(_retval); } \
  NS_IMETHOD ToStringWithFormat(const char *formatType, PRUint32 flags, PRInt32 wrapColumn, PRUnichar **_retval) { return _to ToStringWithFormat(formatType, flags, wrapColumn, _retval); } \
  NS_IMETHOD AddSelectionListener(nsISelectionListener *newListener) { return _to AddSelectionListener(newListener); } \
  NS_IMETHOD RemoveSelectionListener(nsISelectionListener *listenerToRemove) { return _to RemoveSelectionListener(listenerToRemove); } \
  NS_IMETHOD GetTableSelectionType(nsIDOMRange *range, PRInt32 *_retval) { return _to GetTableSelectionType(range, _retval); } \
  NS_IMETHOD SetPresShell(nsIPresShell * aPresShell) { return _to SetPresShell(aPresShell); } \
  NS_IMETHOD GetCanCacheFrameOffset(PRBool *aCanCacheFrameOffset) { return _to GetCanCacheFrameOffset(aCanCacheFrameOffset); } \
  NS_IMETHOD SetCanCacheFrameOffset(PRBool aCanCacheFrameOffset) { return _to SetCanCacheFrameOffset(aCanCacheFrameOffset); } \
  NS_IMETHOD GetCachedFrameOffset(nsIFrame * aFrame, PRInt32 inOffset, nsPoint & aPoint) { return _to GetCachedFrameOffset(aFrame, inOffset, aPoint); } \
  NS_IMETHOD GetFrameSelection(nsIFrameSelection **_retval) { return _to GetFrameSelection(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISELECTIONPRIVATE(_to) \
  NS_IMETHOD GetInterlinePosition(PRBool *aInterlinePosition) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetInterlinePosition(aInterlinePosition); } \
  NS_IMETHOD SetInterlinePosition(PRBool aInterlinePosition) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetInterlinePosition(aInterlinePosition); } \
  NS_IMETHOD StartBatchChanges(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->StartBatchChanges(); } \
  NS_IMETHOD EndBatchChanges(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->EndBatchChanges(); } \
  NS_IMETHOD GetEnumerator(nsIEnumerator **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetEnumerator(_retval); } \
  NS_IMETHOD ToStringWithFormat(const char *formatType, PRUint32 flags, PRInt32 wrapColumn, PRUnichar **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ToStringWithFormat(formatType, flags, wrapColumn, _retval); } \
  NS_IMETHOD AddSelectionListener(nsISelectionListener *newListener) { return !_to ? NS_ERROR_NULL_POINTER : _to->AddSelectionListener(newListener); } \
  NS_IMETHOD RemoveSelectionListener(nsISelectionListener *listenerToRemove) { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveSelectionListener(listenerToRemove); } \
  NS_IMETHOD GetTableSelectionType(nsIDOMRange *range, PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTableSelectionType(range, _retval); } \
  NS_IMETHOD SetPresShell(nsIPresShell * aPresShell) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPresShell(aPresShell); } \
  NS_IMETHOD GetCanCacheFrameOffset(PRBool *aCanCacheFrameOffset) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCanCacheFrameOffset(aCanCacheFrameOffset); } \
  NS_IMETHOD SetCanCacheFrameOffset(PRBool aCanCacheFrameOffset) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCanCacheFrameOffset(aCanCacheFrameOffset); } \
  NS_IMETHOD GetCachedFrameOffset(nsIFrame * aFrame, PRInt32 inOffset, nsPoint & aPoint) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCachedFrameOffset(aFrame, inOffset, aPoint); } \
  NS_IMETHOD GetFrameSelection(nsIFrameSelection **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFrameSelection(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSelectionPrivate : public nsISelectionPrivate
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISELECTIONPRIVATE

  nsSelectionPrivate();

private:
  ~nsSelectionPrivate();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSelectionPrivate, nsISelectionPrivate)

nsSelectionPrivate::nsSelectionPrivate()
{
  /* member initializers and constructor code */
}

nsSelectionPrivate::~nsSelectionPrivate()
{
  /* destructor code */
}

/* attribute boolean interlinePosition; */
NS_IMETHODIMP nsSelectionPrivate::GetInterlinePosition(PRBool *aInterlinePosition)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsSelectionPrivate::SetInterlinePosition(PRBool aInterlinePosition)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void startBatchChanges (); */
NS_IMETHODIMP nsSelectionPrivate::StartBatchChanges()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void endBatchChanges (); */
NS_IMETHODIMP nsSelectionPrivate::EndBatchChanges()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIEnumerator getEnumerator (); */
NS_IMETHODIMP nsSelectionPrivate::GetEnumerator(nsIEnumerator **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* wstring toStringWithFormat (in string formatType, in unsigned long flags, in PRInt32 wrapColumn); */
NS_IMETHODIMP nsSelectionPrivate::ToStringWithFormat(const char *formatType, PRUint32 flags, PRInt32 wrapColumn, PRUnichar **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void addSelectionListener (in nsISelectionListener newListener); */
NS_IMETHODIMP nsSelectionPrivate::AddSelectionListener(nsISelectionListener *newListener)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeSelectionListener (in nsISelectionListener listenerToRemove); */
NS_IMETHODIMP nsSelectionPrivate::RemoveSelectionListener(nsISelectionListener *listenerToRemove)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* long getTableSelectionType (in nsIDOMRange range); */
NS_IMETHODIMP nsSelectionPrivate::GetTableSelectionType(nsIDOMRange *range, PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void setPresShell (in nsIPresShell aPresShell); */
NS_IMETHODIMP nsSelectionPrivate::SetPresShell(nsIPresShell * aPresShell)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] attribute boolean canCacheFrameOffset; */
NS_IMETHODIMP nsSelectionPrivate::GetCanCacheFrameOffset(PRBool *aCanCacheFrameOffset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsSelectionPrivate::SetCanCacheFrameOffset(PRBool aCanCacheFrameOffset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void getCachedFrameOffset (in nsIFrame aFrame, in PRInt32 inOffset, in nsPointRef aPoint); */
NS_IMETHODIMP nsSelectionPrivate::GetCachedFrameOffset(nsIFrame * aFrame, PRInt32 inOffset, nsPoint & aPoint)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] nsIFrameSelection getFrameSelection (); */
NS_IMETHODIMP nsSelectionPrivate::GetFrameSelection(nsIFrameSelection **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISelectionPrivate_h__ */
