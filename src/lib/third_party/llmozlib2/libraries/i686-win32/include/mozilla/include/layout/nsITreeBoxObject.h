/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/xul/base/src/tree/public/nsITreeBoxObject.idl
 */

#ifndef __gen_nsITreeBoxObject_h__
#define __gen_nsITreeBoxObject_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsITreeView; /* forward declaration */

class nsITreeSelection; /* forward declaration */

class nsITreeColumn; /* forward declaration */

class nsITreeColumns; /* forward declaration */


/* starting interface:    nsITreeBoxObject */
#define NS_ITREEBOXOBJECT_IID_STR "55f3b431-1aa8-4e23-ad3d-a9f5644bdaa6"

#define NS_ITREEBOXOBJECT_IID \
  {0x55f3b431, 0x1aa8, 0x4e23, \
    { 0xad, 0x3d, 0xa9, 0xf5, 0x64, 0x4b, 0xda, 0xa6 }}

class NS_NO_VTABLE nsITreeBoxObject : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ITREEBOXOBJECT_IID)

  /**
   * Obtain the columns.
   */
  /* readonly attribute nsITreeColumns columns; */
  NS_IMETHOD GetColumns(nsITreeColumns * *aColumns) = 0;

  /**
   * The view that backs the tree and that supplies it with its data.
   * It is dynamically settable, either using a view attribute on the
   * tree tag or by setting this attribute to a new value.
   */
  /* attribute nsITreeView view; */
  NS_IMETHOD GetView(nsITreeView * *aView) = 0;
  NS_IMETHOD SetView(nsITreeView * aView) = 0;

  /**
   * Whether or not we are currently focused.
   */
  /* attribute boolean focused; */
  NS_IMETHOD GetFocused(PRBool *aFocused) = 0;
  NS_IMETHOD SetFocused(PRBool aFocused) = 0;

  /**
   * Obtain the treebody content node
   */
  /* readonly attribute nsIDOMElement treeBody; */
  NS_IMETHOD GetTreeBody(nsIDOMElement * *aTreeBody) = 0;

  /**
   * Obtain the height of a row.
   */
  /* readonly attribute long rowHeight; */
  NS_IMETHOD GetRowHeight(PRInt32 *aRowHeight) = 0;

  /**
   * Get the index of the first visible row.
   */
  /* long getFirstVisibleRow (); */
  NS_IMETHOD GetFirstVisibleRow(PRInt32 *_retval) = 0;

  /**
   * Get the index of the last visible row.
   */
  /* long getLastVisibleRow (); */
  NS_IMETHOD GetLastVisibleRow(PRInt32 *_retval) = 0;

  /**
   * Gets the number of possible visible rows.
   */
  /* long getPageLength (); */
  NS_IMETHOD GetPageLength(PRInt32 *_retval) = 0;

  /**
   * Ensures that a row at a given index is visible.
   */
  /* void ensureRowIsVisible (in long index); */
  NS_IMETHOD EnsureRowIsVisible(PRInt32 index) = 0;

  /**
   * Scrolls such that the row at index is at the top of the visible view.
   */
  /* void scrollToRow (in long index); */
  NS_IMETHOD ScrollToRow(PRInt32 index) = 0;

  /**
   * Scroll the tree up or down by numLines lines. Positive
   * values move down in the tree. Prevents scrolling off the
   * end of the tree. 
   */
  /* void scrollByLines (in long numLines); */
  NS_IMETHOD ScrollByLines(PRInt32 numLines) = 0;

  /**
   * Scroll the tree up or down by numPages pages. A page
   * is considered to be the amount displayed by the tree.
   * Positive values move down in the tree. Prevents scrolling
   * off the end of the tree.
   */
  /* void scrollByPages (in long numPages); */
  NS_IMETHOD ScrollByPages(PRInt32 numPages) = 0;

  /**
   * Invalidation methods for fine-grained painting control.
   */
  /* void invalidate (); */
  NS_IMETHOD Invalidate(void) = 0;

  /* void invalidateColumn (in nsITreeColumn col); */
  NS_IMETHOD InvalidateColumn(nsITreeColumn *col) = 0;

  /* void invalidateRow (in long index); */
  NS_IMETHOD InvalidateRow(PRInt32 index) = 0;

  /* void invalidateCell (in long row, in nsITreeColumn col); */
  NS_IMETHOD InvalidateCell(PRInt32 row, nsITreeColumn *col) = 0;

  /* void invalidateRange (in long startIndex, in long endIndex); */
  NS_IMETHOD InvalidateRange(PRInt32 startIndex, PRInt32 endIndex) = 0;

  /**
   * A hit test that can tell you what row the mouse is over.
   * returns -1 for invalid mouse coordinates.
   *
   * The coordinate system is the client coordinate system for the
   * document this boxObject lives in, and the units are CSS pixels.
   */
  /* long getRowAt (in long x, in long y); */
  NS_IMETHOD GetRowAt(PRInt32 x, PRInt32 y, PRInt32 *_retval) = 0;

  /**
   * A hit test that can tell you what cell the mouse is over.  Row is the row index
   * hit,  returns -1 for invalid mouse coordinates.  ColID is the column hit.
   * ChildElt is the pseudoelement hit: this can have values of
   * "cell", "twisty", "image", and "text".
   *
   * The coordinate system is the client coordinate system for the
   * document this boxObject lives in, and the units are CSS pixels.
   */
  /* void getCellAt (in long x, in long y, out long row, out nsITreeColumn col, out ACString childElt); */
  NS_IMETHOD GetCellAt(PRInt32 x, PRInt32 y, PRInt32 *row, nsITreeColumn **col, nsACString & childElt) = 0;

  /** 
   * Find the coordinates of an element within a specific cell. 
   */
  /* void getCoordsForCellItem (in long row, in nsITreeColumn col, in ACString element, out long x, out long y, out long width, out long height); */
  NS_IMETHOD GetCoordsForCellItem(PRInt32 row, nsITreeColumn *col, const nsACString & element, PRInt32 *x, PRInt32 *y, PRInt32 *width, PRInt32 *height) = 0;

  /** 
   * Determine if the text of a cell is being cropped or not.
   */
  /* boolean isCellCropped (in long row, in nsITreeColumn col); */
  NS_IMETHOD IsCellCropped(PRInt32 row, nsITreeColumn *col, PRBool *_retval) = 0;

  /**
   * The view is responsible for calling these notification methods when
   * rows are added or removed.  Index is the position at which the new
   * rows were added or at which rows were removed.  For
   * non-contiguous additions/removals, this method should be called multiple times.
   */
  /* void rowCountChanged (in long index, in long count); */
  NS_IMETHOD RowCountChanged(PRInt32 index, PRInt32 count) = 0;

  /**
   * Notify the tree that the view is about to perform a batch
   * update, that is, add, remove or invalidate several rows at once.
   * This must be followed by calling endUpdateBatch(), otherwise the tree
   * will get out of sync.
   */
  /* void beginUpdateBatch (); */
  NS_IMETHOD BeginUpdateBatch(void) = 0;

  /**
   * Notify the tree that the view has completed a batch update.
   */
  /* void endUpdateBatch (); */
  NS_IMETHOD EndUpdateBatch(void) = 0;

  /**
   * Called on a theme switch to flush out the tree's style and image caches.
   */
  /* void clearStyleAndImageCaches (); */
  NS_IMETHOD ClearStyleAndImageCaches(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSITREEBOXOBJECT \
  NS_IMETHOD GetColumns(nsITreeColumns * *aColumns); \
  NS_IMETHOD GetView(nsITreeView * *aView); \
  NS_IMETHOD SetView(nsITreeView * aView); \
  NS_IMETHOD GetFocused(PRBool *aFocused); \
  NS_IMETHOD SetFocused(PRBool aFocused); \
  NS_IMETHOD GetTreeBody(nsIDOMElement * *aTreeBody); \
  NS_IMETHOD GetRowHeight(PRInt32 *aRowHeight); \
  NS_IMETHOD GetFirstVisibleRow(PRInt32 *_retval); \
  NS_IMETHOD GetLastVisibleRow(PRInt32 *_retval); \
  NS_IMETHOD GetPageLength(PRInt32 *_retval); \
  NS_IMETHOD EnsureRowIsVisible(PRInt32 index); \
  NS_IMETHOD ScrollToRow(PRInt32 index); \
  NS_IMETHOD ScrollByLines(PRInt32 numLines); \
  NS_IMETHOD ScrollByPages(PRInt32 numPages); \
  NS_IMETHOD Invalidate(void); \
  NS_IMETHOD InvalidateColumn(nsITreeColumn *col); \
  NS_IMETHOD InvalidateRow(PRInt32 index); \
  NS_IMETHOD InvalidateCell(PRInt32 row, nsITreeColumn *col); \
  NS_IMETHOD InvalidateRange(PRInt32 startIndex, PRInt32 endIndex); \
  NS_IMETHOD GetRowAt(PRInt32 x, PRInt32 y, PRInt32 *_retval); \
  NS_IMETHOD GetCellAt(PRInt32 x, PRInt32 y, PRInt32 *row, nsITreeColumn **col, nsACString & childElt); \
  NS_IMETHOD GetCoordsForCellItem(PRInt32 row, nsITreeColumn *col, const nsACString & element, PRInt32 *x, PRInt32 *y, PRInt32 *width, PRInt32 *height); \
  NS_IMETHOD IsCellCropped(PRInt32 row, nsITreeColumn *col, PRBool *_retval); \
  NS_IMETHOD RowCountChanged(PRInt32 index, PRInt32 count); \
  NS_IMETHOD BeginUpdateBatch(void); \
  NS_IMETHOD EndUpdateBatch(void); \
  NS_IMETHOD ClearStyleAndImageCaches(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSITREEBOXOBJECT(_to) \
  NS_IMETHOD GetColumns(nsITreeColumns * *aColumns) { return _to GetColumns(aColumns); } \
  NS_IMETHOD GetView(nsITreeView * *aView) { return _to GetView(aView); } \
  NS_IMETHOD SetView(nsITreeView * aView) { return _to SetView(aView); } \
  NS_IMETHOD GetFocused(PRBool *aFocused) { return _to GetFocused(aFocused); } \
  NS_IMETHOD SetFocused(PRBool aFocused) { return _to SetFocused(aFocused); } \
  NS_IMETHOD GetTreeBody(nsIDOMElement * *aTreeBody) { return _to GetTreeBody(aTreeBody); } \
  NS_IMETHOD GetRowHeight(PRInt32 *aRowHeight) { return _to GetRowHeight(aRowHeight); } \
  NS_IMETHOD GetFirstVisibleRow(PRInt32 *_retval) { return _to GetFirstVisibleRow(_retval); } \
  NS_IMETHOD GetLastVisibleRow(PRInt32 *_retval) { return _to GetLastVisibleRow(_retval); } \
  NS_IMETHOD GetPageLength(PRInt32 *_retval) { return _to GetPageLength(_retval); } \
  NS_IMETHOD EnsureRowIsVisible(PRInt32 index) { return _to EnsureRowIsVisible(index); } \
  NS_IMETHOD ScrollToRow(PRInt32 index) { return _to ScrollToRow(index); } \
  NS_IMETHOD ScrollByLines(PRInt32 numLines) { return _to ScrollByLines(numLines); } \
  NS_IMETHOD ScrollByPages(PRInt32 numPages) { return _to ScrollByPages(numPages); } \
  NS_IMETHOD Invalidate(void) { return _to Invalidate(); } \
  NS_IMETHOD InvalidateColumn(nsITreeColumn *col) { return _to InvalidateColumn(col); } \
  NS_IMETHOD InvalidateRow(PRInt32 index) { return _to InvalidateRow(index); } \
  NS_IMETHOD InvalidateCell(PRInt32 row, nsITreeColumn *col) { return _to InvalidateCell(row, col); } \
  NS_IMETHOD InvalidateRange(PRInt32 startIndex, PRInt32 endIndex) { return _to InvalidateRange(startIndex, endIndex); } \
  NS_IMETHOD GetRowAt(PRInt32 x, PRInt32 y, PRInt32 *_retval) { return _to GetRowAt(x, y, _retval); } \
  NS_IMETHOD GetCellAt(PRInt32 x, PRInt32 y, PRInt32 *row, nsITreeColumn **col, nsACString & childElt) { return _to GetCellAt(x, y, row, col, childElt); } \
  NS_IMETHOD GetCoordsForCellItem(PRInt32 row, nsITreeColumn *col, const nsACString & element, PRInt32 *x, PRInt32 *y, PRInt32 *width, PRInt32 *height) { return _to GetCoordsForCellItem(row, col, element, x, y, width, height); } \
  NS_IMETHOD IsCellCropped(PRInt32 row, nsITreeColumn *col, PRBool *_retval) { return _to IsCellCropped(row, col, _retval); } \
  NS_IMETHOD RowCountChanged(PRInt32 index, PRInt32 count) { return _to RowCountChanged(index, count); } \
  NS_IMETHOD BeginUpdateBatch(void) { return _to BeginUpdateBatch(); } \
  NS_IMETHOD EndUpdateBatch(void) { return _to EndUpdateBatch(); } \
  NS_IMETHOD ClearStyleAndImageCaches(void) { return _to ClearStyleAndImageCaches(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSITREEBOXOBJECT(_to) \
  NS_IMETHOD GetColumns(nsITreeColumns * *aColumns) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetColumns(aColumns); } \
  NS_IMETHOD GetView(nsITreeView * *aView) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetView(aView); } \
  NS_IMETHOD SetView(nsITreeView * aView) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetView(aView); } \
  NS_IMETHOD GetFocused(PRBool *aFocused) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFocused(aFocused); } \
  NS_IMETHOD SetFocused(PRBool aFocused) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFocused(aFocused); } \
  NS_IMETHOD GetTreeBody(nsIDOMElement * *aTreeBody) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTreeBody(aTreeBody); } \
  NS_IMETHOD GetRowHeight(PRInt32 *aRowHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRowHeight(aRowHeight); } \
  NS_IMETHOD GetFirstVisibleRow(PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFirstVisibleRow(_retval); } \
  NS_IMETHOD GetLastVisibleRow(PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLastVisibleRow(_retval); } \
  NS_IMETHOD GetPageLength(PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPageLength(_retval); } \
  NS_IMETHOD EnsureRowIsVisible(PRInt32 index) { return !_to ? NS_ERROR_NULL_POINTER : _to->EnsureRowIsVisible(index); } \
  NS_IMETHOD ScrollToRow(PRInt32 index) { return !_to ? NS_ERROR_NULL_POINTER : _to->ScrollToRow(index); } \
  NS_IMETHOD ScrollByLines(PRInt32 numLines) { return !_to ? NS_ERROR_NULL_POINTER : _to->ScrollByLines(numLines); } \
  NS_IMETHOD ScrollByPages(PRInt32 numPages) { return !_to ? NS_ERROR_NULL_POINTER : _to->ScrollByPages(numPages); } \
  NS_IMETHOD Invalidate(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Invalidate(); } \
  NS_IMETHOD InvalidateColumn(nsITreeColumn *col) { return !_to ? NS_ERROR_NULL_POINTER : _to->InvalidateColumn(col); } \
  NS_IMETHOD InvalidateRow(PRInt32 index) { return !_to ? NS_ERROR_NULL_POINTER : _to->InvalidateRow(index); } \
  NS_IMETHOD InvalidateCell(PRInt32 row, nsITreeColumn *col) { return !_to ? NS_ERROR_NULL_POINTER : _to->InvalidateCell(row, col); } \
  NS_IMETHOD InvalidateRange(PRInt32 startIndex, PRInt32 endIndex) { return !_to ? NS_ERROR_NULL_POINTER : _to->InvalidateRange(startIndex, endIndex); } \
  NS_IMETHOD GetRowAt(PRInt32 x, PRInt32 y, PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRowAt(x, y, _retval); } \
  NS_IMETHOD GetCellAt(PRInt32 x, PRInt32 y, PRInt32 *row, nsITreeColumn **col, nsACString & childElt) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCellAt(x, y, row, col, childElt); } \
  NS_IMETHOD GetCoordsForCellItem(PRInt32 row, nsITreeColumn *col, const nsACString & element, PRInt32 *x, PRInt32 *y, PRInt32 *width, PRInt32 *height) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCoordsForCellItem(row, col, element, x, y, width, height); } \
  NS_IMETHOD IsCellCropped(PRInt32 row, nsITreeColumn *col, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->IsCellCropped(row, col, _retval); } \
  NS_IMETHOD RowCountChanged(PRInt32 index, PRInt32 count) { return !_to ? NS_ERROR_NULL_POINTER : _to->RowCountChanged(index, count); } \
  NS_IMETHOD BeginUpdateBatch(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->BeginUpdateBatch(); } \
  NS_IMETHOD EndUpdateBatch(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->EndUpdateBatch(); } \
  NS_IMETHOD ClearStyleAndImageCaches(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ClearStyleAndImageCaches(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsTreeBoxObject : public nsITreeBoxObject
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSITREEBOXOBJECT

  nsTreeBoxObject();

private:
  ~nsTreeBoxObject();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsTreeBoxObject, nsITreeBoxObject)

nsTreeBoxObject::nsTreeBoxObject()
{
  /* member initializers and constructor code */
}

nsTreeBoxObject::~nsTreeBoxObject()
{
  /* destructor code */
}

/* readonly attribute nsITreeColumns columns; */
NS_IMETHODIMP nsTreeBoxObject::GetColumns(nsITreeColumns * *aColumns)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsITreeView view; */
NS_IMETHODIMP nsTreeBoxObject::GetView(nsITreeView * *aView)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsTreeBoxObject::SetView(nsITreeView * aView)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean focused; */
NS_IMETHODIMP nsTreeBoxObject::GetFocused(PRBool *aFocused)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsTreeBoxObject::SetFocused(PRBool aFocused)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMElement treeBody; */
NS_IMETHODIMP nsTreeBoxObject::GetTreeBody(nsIDOMElement * *aTreeBody)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long rowHeight; */
NS_IMETHODIMP nsTreeBoxObject::GetRowHeight(PRInt32 *aRowHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* long getFirstVisibleRow (); */
NS_IMETHODIMP nsTreeBoxObject::GetFirstVisibleRow(PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* long getLastVisibleRow (); */
NS_IMETHODIMP nsTreeBoxObject::GetLastVisibleRow(PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* long getPageLength (); */
NS_IMETHODIMP nsTreeBoxObject::GetPageLength(PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void ensureRowIsVisible (in long index); */
NS_IMETHODIMP nsTreeBoxObject::EnsureRowIsVisible(PRInt32 index)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void scrollToRow (in long index); */
NS_IMETHODIMP nsTreeBoxObject::ScrollToRow(PRInt32 index)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void scrollByLines (in long numLines); */
NS_IMETHODIMP nsTreeBoxObject::ScrollByLines(PRInt32 numLines)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void scrollByPages (in long numPages); */
NS_IMETHODIMP nsTreeBoxObject::ScrollByPages(PRInt32 numPages)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void invalidate (); */
NS_IMETHODIMP nsTreeBoxObject::Invalidate()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void invalidateColumn (in nsITreeColumn col); */
NS_IMETHODIMP nsTreeBoxObject::InvalidateColumn(nsITreeColumn *col)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void invalidateRow (in long index); */
NS_IMETHODIMP nsTreeBoxObject::InvalidateRow(PRInt32 index)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void invalidateCell (in long row, in nsITreeColumn col); */
NS_IMETHODIMP nsTreeBoxObject::InvalidateCell(PRInt32 row, nsITreeColumn *col)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void invalidateRange (in long startIndex, in long endIndex); */
NS_IMETHODIMP nsTreeBoxObject::InvalidateRange(PRInt32 startIndex, PRInt32 endIndex)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* long getRowAt (in long x, in long y); */
NS_IMETHODIMP nsTreeBoxObject::GetRowAt(PRInt32 x, PRInt32 y, PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getCellAt (in long x, in long y, out long row, out nsITreeColumn col, out ACString childElt); */
NS_IMETHODIMP nsTreeBoxObject::GetCellAt(PRInt32 x, PRInt32 y, PRInt32 *row, nsITreeColumn **col, nsACString & childElt)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getCoordsForCellItem (in long row, in nsITreeColumn col, in ACString element, out long x, out long y, out long width, out long height); */
NS_IMETHODIMP nsTreeBoxObject::GetCoordsForCellItem(PRInt32 row, nsITreeColumn *col, const nsACString & element, PRInt32 *x, PRInt32 *y, PRInt32 *width, PRInt32 *height)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isCellCropped (in long row, in nsITreeColumn col); */
NS_IMETHODIMP nsTreeBoxObject::IsCellCropped(PRInt32 row, nsITreeColumn *col, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void rowCountChanged (in long index, in long count); */
NS_IMETHODIMP nsTreeBoxObject::RowCountChanged(PRInt32 index, PRInt32 count)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void beginUpdateBatch (); */
NS_IMETHODIMP nsTreeBoxObject::BeginUpdateBatch()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void endUpdateBatch (); */
NS_IMETHODIMP nsTreeBoxObject::EndUpdateBatch()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void clearStyleAndImageCaches (); */
NS_IMETHODIMP nsTreeBoxObject::ClearStyleAndImageCaches()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsITreeBoxObject_h__ */
