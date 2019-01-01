/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/xul/base/src/tree/public/nsITreeSelection.idl
 */

#ifndef __gen_nsITreeSelection_h__
#define __gen_nsITreeSelection_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsITreeBoxObject; /* forward declaration */


/* starting interface:    nsITreeSelection */
#define NS_ITREESELECTION_IID_STR "f8a13364-184e-4da3-badf-5c04837537f8"

#define NS_ITREESELECTION_IID \
  {0xf8a13364, 0x184e, 0x4da3, \
    { 0xba, 0xdf, 0x5c, 0x04, 0x83, 0x75, 0x37, 0xf8 }}

class NS_NO_VTABLE nsITreeSelection : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ITREESELECTION_IID)

  /**
   * The tree widget for this selection.
   */
  /* attribute nsITreeBoxObject tree; */
  NS_IMETHOD GetTree(nsITreeBoxObject * *aTree) = 0;
  NS_IMETHOD SetTree(nsITreeBoxObject * aTree) = 0;

  /**
   * This attribute is a boolean indicating single selection.
   */
  /* readonly attribute boolean single; */
  NS_IMETHOD GetSingle(PRBool *aSingle) = 0;

  /**
   * The number of rows currently selected in this tree.
   */
  /* readonly attribute long count; */
  NS_IMETHOD GetCount(PRInt32 *aCount) = 0;

  /**
   * Indicates whether or not the row at the specified index is
   * part of the selection.
   */
  /* boolean isSelected (in long index); */
  NS_IMETHOD IsSelected(PRInt32 index, PRBool *_retval) = 0;

  /**
   * Deselect all rows and select the row at the specified index. 
   */
  /* void select (in long index); */
  NS_IMETHOD Select(PRInt32 index) = 0;

  /**
   * Perform a timed select.
   */
  /* void timedSelect (in long index, in long delay); */
  NS_IMETHOD TimedSelect(PRInt32 index, PRInt32 delay) = 0;

  /**
   * Toggle the selection state of the row at the specified index.
   */
  /* void toggleSelect (in long index); */
  NS_IMETHOD ToggleSelect(PRInt32 index) = 0;

  /**
   * Select the range specified by the indices.  If augment is true,
   * then we add the range to the selection without clearing out anything
   * else.  If augment is false, everything is cleared except for the specified range.
   */
  /* void rangedSelect (in long startIndex, in long endIndex, in boolean augment); */
  NS_IMETHOD RangedSelect(PRInt32 startIndex, PRInt32 endIndex, PRBool augment) = 0;

  /**
   * Clears the range.
   */
  /* void clearRange (in long startIndex, in long endIndex); */
  NS_IMETHOD ClearRange(PRInt32 startIndex, PRInt32 endIndex) = 0;

  /**
   * Clears the selection.
   */
  /* void clearSelection (); */
  NS_IMETHOD ClearSelection(void) = 0;

  /**
   * Inverts the selection.
   */
  /* void invertSelection (); */
  NS_IMETHOD InvertSelection(void) = 0;

  /**
   * Selects all rows.
   */
  /* void selectAll (); */
  NS_IMETHOD SelectAll(void) = 0;

  /**
   * Iterate the selection using these methods.
   */
  /* long getRangeCount (); */
  NS_IMETHOD GetRangeCount(PRInt32 *_retval) = 0;

  /* void getRangeAt (in long i, out long min, out long max); */
  NS_IMETHOD GetRangeAt(PRInt32 i, PRInt32 *min, PRInt32 *max) = 0;

  /**
   * Can be used to invalidate the selection.
   */
  /* void invalidateSelection (); */
  NS_IMETHOD InvalidateSelection(void) = 0;

  /**
   * Called when the row count changes to adjust selection indices.
   */
  /* void adjustSelection (in long index, in long count); */
  NS_IMETHOD AdjustSelection(PRInt32 index, PRInt32 count) = 0;

  /**
   * This attribute is a boolean indicating whether or not the
   * "select" event should fire when the selection is changed using
   * one of our methods.  A view can use this to temporarily suppress
   * the selection while manipulating all of the indices, e.g., on 
   * a sort.
   */
  /* attribute boolean selectEventsSuppressed; */
  NS_IMETHOD GetSelectEventsSuppressed(PRBool *aSelectEventsSuppressed) = 0;
  NS_IMETHOD SetSelectEventsSuppressed(PRBool aSelectEventsSuppressed) = 0;

  /**
   * The current item (the one that gets a focus rect in addition to being
   * selected).
   */
  /* attribute long currentIndex; */
  NS_IMETHOD GetCurrentIndex(PRInt32 *aCurrentIndex) = 0;
  NS_IMETHOD SetCurrentIndex(PRInt32 aCurrentIndex) = 0;

  /**
   * The selection "pivot".  This is the first item the user selected as
   * part of a ranged select.
   */
  /* readonly attribute long shiftSelectPivot; */
  NS_IMETHOD GetShiftSelectPivot(PRInt32 *aShiftSelectPivot) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSITREESELECTION \
  NS_IMETHOD GetTree(nsITreeBoxObject * *aTree); \
  NS_IMETHOD SetTree(nsITreeBoxObject * aTree); \
  NS_IMETHOD GetSingle(PRBool *aSingle); \
  NS_IMETHOD GetCount(PRInt32 *aCount); \
  NS_IMETHOD IsSelected(PRInt32 index, PRBool *_retval); \
  NS_IMETHOD Select(PRInt32 index); \
  NS_IMETHOD TimedSelect(PRInt32 index, PRInt32 delay); \
  NS_IMETHOD ToggleSelect(PRInt32 index); \
  NS_IMETHOD RangedSelect(PRInt32 startIndex, PRInt32 endIndex, PRBool augment); \
  NS_IMETHOD ClearRange(PRInt32 startIndex, PRInt32 endIndex); \
  NS_IMETHOD ClearSelection(void); \
  NS_IMETHOD InvertSelection(void); \
  NS_IMETHOD SelectAll(void); \
  NS_IMETHOD GetRangeCount(PRInt32 *_retval); \
  NS_IMETHOD GetRangeAt(PRInt32 i, PRInt32 *min, PRInt32 *max); \
  NS_IMETHOD InvalidateSelection(void); \
  NS_IMETHOD AdjustSelection(PRInt32 index, PRInt32 count); \
  NS_IMETHOD GetSelectEventsSuppressed(PRBool *aSelectEventsSuppressed); \
  NS_IMETHOD SetSelectEventsSuppressed(PRBool aSelectEventsSuppressed); \
  NS_IMETHOD GetCurrentIndex(PRInt32 *aCurrentIndex); \
  NS_IMETHOD SetCurrentIndex(PRInt32 aCurrentIndex); \
  NS_IMETHOD GetShiftSelectPivot(PRInt32 *aShiftSelectPivot); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSITREESELECTION(_to) \
  NS_IMETHOD GetTree(nsITreeBoxObject * *aTree) { return _to GetTree(aTree); } \
  NS_IMETHOD SetTree(nsITreeBoxObject * aTree) { return _to SetTree(aTree); } \
  NS_IMETHOD GetSingle(PRBool *aSingle) { return _to GetSingle(aSingle); } \
  NS_IMETHOD GetCount(PRInt32 *aCount) { return _to GetCount(aCount); } \
  NS_IMETHOD IsSelected(PRInt32 index, PRBool *_retval) { return _to IsSelected(index, _retval); } \
  NS_IMETHOD Select(PRInt32 index) { return _to Select(index); } \
  NS_IMETHOD TimedSelect(PRInt32 index, PRInt32 delay) { return _to TimedSelect(index, delay); } \
  NS_IMETHOD ToggleSelect(PRInt32 index) { return _to ToggleSelect(index); } \
  NS_IMETHOD RangedSelect(PRInt32 startIndex, PRInt32 endIndex, PRBool augment) { return _to RangedSelect(startIndex, endIndex, augment); } \
  NS_IMETHOD ClearRange(PRInt32 startIndex, PRInt32 endIndex) { return _to ClearRange(startIndex, endIndex); } \
  NS_IMETHOD ClearSelection(void) { return _to ClearSelection(); } \
  NS_IMETHOD InvertSelection(void) { return _to InvertSelection(); } \
  NS_IMETHOD SelectAll(void) { return _to SelectAll(); } \
  NS_IMETHOD GetRangeCount(PRInt32 *_retval) { return _to GetRangeCount(_retval); } \
  NS_IMETHOD GetRangeAt(PRInt32 i, PRInt32 *min, PRInt32 *max) { return _to GetRangeAt(i, min, max); } \
  NS_IMETHOD InvalidateSelection(void) { return _to InvalidateSelection(); } \
  NS_IMETHOD AdjustSelection(PRInt32 index, PRInt32 count) { return _to AdjustSelection(index, count); } \
  NS_IMETHOD GetSelectEventsSuppressed(PRBool *aSelectEventsSuppressed) { return _to GetSelectEventsSuppressed(aSelectEventsSuppressed); } \
  NS_IMETHOD SetSelectEventsSuppressed(PRBool aSelectEventsSuppressed) { return _to SetSelectEventsSuppressed(aSelectEventsSuppressed); } \
  NS_IMETHOD GetCurrentIndex(PRInt32 *aCurrentIndex) { return _to GetCurrentIndex(aCurrentIndex); } \
  NS_IMETHOD SetCurrentIndex(PRInt32 aCurrentIndex) { return _to SetCurrentIndex(aCurrentIndex); } \
  NS_IMETHOD GetShiftSelectPivot(PRInt32 *aShiftSelectPivot) { return _to GetShiftSelectPivot(aShiftSelectPivot); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSITREESELECTION(_to) \
  NS_IMETHOD GetTree(nsITreeBoxObject * *aTree) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTree(aTree); } \
  NS_IMETHOD SetTree(nsITreeBoxObject * aTree) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTree(aTree); } \
  NS_IMETHOD GetSingle(PRBool *aSingle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSingle(aSingle); } \
  NS_IMETHOD GetCount(PRInt32 *aCount) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCount(aCount); } \
  NS_IMETHOD IsSelected(PRInt32 index, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->IsSelected(index, _retval); } \
  NS_IMETHOD Select(PRInt32 index) { return !_to ? NS_ERROR_NULL_POINTER : _to->Select(index); } \
  NS_IMETHOD TimedSelect(PRInt32 index, PRInt32 delay) { return !_to ? NS_ERROR_NULL_POINTER : _to->TimedSelect(index, delay); } \
  NS_IMETHOD ToggleSelect(PRInt32 index) { return !_to ? NS_ERROR_NULL_POINTER : _to->ToggleSelect(index); } \
  NS_IMETHOD RangedSelect(PRInt32 startIndex, PRInt32 endIndex, PRBool augment) { return !_to ? NS_ERROR_NULL_POINTER : _to->RangedSelect(startIndex, endIndex, augment); } \
  NS_IMETHOD ClearRange(PRInt32 startIndex, PRInt32 endIndex) { return !_to ? NS_ERROR_NULL_POINTER : _to->ClearRange(startIndex, endIndex); } \
  NS_IMETHOD ClearSelection(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ClearSelection(); } \
  NS_IMETHOD InvertSelection(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->InvertSelection(); } \
  NS_IMETHOD SelectAll(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->SelectAll(); } \
  NS_IMETHOD GetRangeCount(PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRangeCount(_retval); } \
  NS_IMETHOD GetRangeAt(PRInt32 i, PRInt32 *min, PRInt32 *max) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRangeAt(i, min, max); } \
  NS_IMETHOD InvalidateSelection(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->InvalidateSelection(); } \
  NS_IMETHOD AdjustSelection(PRInt32 index, PRInt32 count) { return !_to ? NS_ERROR_NULL_POINTER : _to->AdjustSelection(index, count); } \
  NS_IMETHOD GetSelectEventsSuppressed(PRBool *aSelectEventsSuppressed) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSelectEventsSuppressed(aSelectEventsSuppressed); } \
  NS_IMETHOD SetSelectEventsSuppressed(PRBool aSelectEventsSuppressed) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSelectEventsSuppressed(aSelectEventsSuppressed); } \
  NS_IMETHOD GetCurrentIndex(PRInt32 *aCurrentIndex) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCurrentIndex(aCurrentIndex); } \
  NS_IMETHOD SetCurrentIndex(PRInt32 aCurrentIndex) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCurrentIndex(aCurrentIndex); } \
  NS_IMETHOD GetShiftSelectPivot(PRInt32 *aShiftSelectPivot) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetShiftSelectPivot(aShiftSelectPivot); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsTreeSelection : public nsITreeSelection
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSITREESELECTION

  nsTreeSelection();

private:
  ~nsTreeSelection();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsTreeSelection, nsITreeSelection)

nsTreeSelection::nsTreeSelection()
{
  /* member initializers and constructor code */
}

nsTreeSelection::~nsTreeSelection()
{
  /* destructor code */
}

/* attribute nsITreeBoxObject tree; */
NS_IMETHODIMP nsTreeSelection::GetTree(nsITreeBoxObject * *aTree)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsTreeSelection::SetTree(nsITreeBoxObject * aTree)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean single; */
NS_IMETHODIMP nsTreeSelection::GetSingle(PRBool *aSingle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long count; */
NS_IMETHODIMP nsTreeSelection::GetCount(PRInt32 *aCount)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isSelected (in long index); */
NS_IMETHODIMP nsTreeSelection::IsSelected(PRInt32 index, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void select (in long index); */
NS_IMETHODIMP nsTreeSelection::Select(PRInt32 index)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void timedSelect (in long index, in long delay); */
NS_IMETHODIMP nsTreeSelection::TimedSelect(PRInt32 index, PRInt32 delay)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void toggleSelect (in long index); */
NS_IMETHODIMP nsTreeSelection::ToggleSelect(PRInt32 index)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void rangedSelect (in long startIndex, in long endIndex, in boolean augment); */
NS_IMETHODIMP nsTreeSelection::RangedSelect(PRInt32 startIndex, PRInt32 endIndex, PRBool augment)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void clearRange (in long startIndex, in long endIndex); */
NS_IMETHODIMP nsTreeSelection::ClearRange(PRInt32 startIndex, PRInt32 endIndex)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void clearSelection (); */
NS_IMETHODIMP nsTreeSelection::ClearSelection()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void invertSelection (); */
NS_IMETHODIMP nsTreeSelection::InvertSelection()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void selectAll (); */
NS_IMETHODIMP nsTreeSelection::SelectAll()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* long getRangeCount (); */
NS_IMETHODIMP nsTreeSelection::GetRangeCount(PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getRangeAt (in long i, out long min, out long max); */
NS_IMETHODIMP nsTreeSelection::GetRangeAt(PRInt32 i, PRInt32 *min, PRInt32 *max)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void invalidateSelection (); */
NS_IMETHODIMP nsTreeSelection::InvalidateSelection()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void adjustSelection (in long index, in long count); */
NS_IMETHODIMP nsTreeSelection::AdjustSelection(PRInt32 index, PRInt32 count)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean selectEventsSuppressed; */
NS_IMETHODIMP nsTreeSelection::GetSelectEventsSuppressed(PRBool *aSelectEventsSuppressed)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsTreeSelection::SetSelectEventsSuppressed(PRBool aSelectEventsSuppressed)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long currentIndex; */
NS_IMETHODIMP nsTreeSelection::GetCurrentIndex(PRInt32 *aCurrentIndex)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsTreeSelection::SetCurrentIndex(PRInt32 aCurrentIndex)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long shiftSelectPivot; */
NS_IMETHODIMP nsTreeSelection::GetShiftSelectPivot(PRInt32 *aShiftSelectPivot)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsITreeSelection_h__ */
