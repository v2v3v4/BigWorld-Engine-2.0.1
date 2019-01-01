/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/xul/nsIDOMXULSelectCntrlEl.idl
 */

#ifndef __gen_nsIDOMXULSelectCntrlEl_h__
#define __gen_nsIDOMXULSelectCntrlEl_h__


#ifndef __gen_nsIDOMXULControlElement_h__
#include "nsIDOMXULControlElement.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMXULSelectControlItemElement; /* forward declaration */


/* starting interface:    nsIDOMXULSelectControlElement */
#define NS_IDOMXULSELECTCONTROLELEMENT_IID_STR "59fec127-2a0e-445b-84b5-a66dc90245db"

#define NS_IDOMXULSELECTCONTROLELEMENT_IID \
  {0x59fec127, 0x2a0e, 0x445b, \
    { 0x84, 0xb5, 0xa6, 0x6d, 0xc9, 0x02, 0x45, 0xdb }}

class NS_NO_VTABLE nsIDOMXULSelectControlElement : public nsIDOMXULControlElement {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMXULSELECTCONTROLELEMENT_IID)

  /* attribute nsIDOMXULSelectControlItemElement selectedItem; */
  NS_IMETHOD GetSelectedItem(nsIDOMXULSelectControlItemElement * *aSelectedItem) = 0;
  NS_IMETHOD SetSelectedItem(nsIDOMXULSelectControlItemElement * aSelectedItem) = 0;

  /* attribute long selectedIndex; */
  NS_IMETHOD GetSelectedIndex(PRInt32 *aSelectedIndex) = 0;
  NS_IMETHOD SetSelectedIndex(PRInt32 aSelectedIndex) = 0;

  /* attribute DOMString value; */
  NS_IMETHOD GetValue(nsAString & aValue) = 0;
  NS_IMETHOD SetValue(const nsAString & aValue) = 0;

  /* nsIDOMXULSelectControlItemElement appendItem (in DOMString label, in DOMString value); */
  NS_IMETHOD AppendItem(const nsAString & label, const nsAString & value, nsIDOMXULSelectControlItemElement **_retval) = 0;

  /* nsIDOMXULSelectControlItemElement insertItemAt (in long index, in DOMString label, in DOMString value); */
  NS_IMETHOD InsertItemAt(PRInt32 index, const nsAString & label, const nsAString & value, nsIDOMXULSelectControlItemElement **_retval) = 0;

  /* nsIDOMXULSelectControlItemElement removeItemAt (in long index); */
  NS_IMETHOD RemoveItemAt(PRInt32 index, nsIDOMXULSelectControlItemElement **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMXULSELECTCONTROLELEMENT \
  NS_IMETHOD GetSelectedItem(nsIDOMXULSelectControlItemElement * *aSelectedItem); \
  NS_IMETHOD SetSelectedItem(nsIDOMXULSelectControlItemElement * aSelectedItem); \
  NS_IMETHOD GetSelectedIndex(PRInt32 *aSelectedIndex); \
  NS_IMETHOD SetSelectedIndex(PRInt32 aSelectedIndex); \
  NS_IMETHOD GetValue(nsAString & aValue); \
  NS_IMETHOD SetValue(const nsAString & aValue); \
  NS_IMETHOD AppendItem(const nsAString & label, const nsAString & value, nsIDOMXULSelectControlItemElement **_retval); \
  NS_IMETHOD InsertItemAt(PRInt32 index, const nsAString & label, const nsAString & value, nsIDOMXULSelectControlItemElement **_retval); \
  NS_IMETHOD RemoveItemAt(PRInt32 index, nsIDOMXULSelectControlItemElement **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMXULSELECTCONTROLELEMENT(_to) \
  NS_IMETHOD GetSelectedItem(nsIDOMXULSelectControlItemElement * *aSelectedItem) { return _to GetSelectedItem(aSelectedItem); } \
  NS_IMETHOD SetSelectedItem(nsIDOMXULSelectControlItemElement * aSelectedItem) { return _to SetSelectedItem(aSelectedItem); } \
  NS_IMETHOD GetSelectedIndex(PRInt32 *aSelectedIndex) { return _to GetSelectedIndex(aSelectedIndex); } \
  NS_IMETHOD SetSelectedIndex(PRInt32 aSelectedIndex) { return _to SetSelectedIndex(aSelectedIndex); } \
  NS_IMETHOD GetValue(nsAString & aValue) { return _to GetValue(aValue); } \
  NS_IMETHOD SetValue(const nsAString & aValue) { return _to SetValue(aValue); } \
  NS_IMETHOD AppendItem(const nsAString & label, const nsAString & value, nsIDOMXULSelectControlItemElement **_retval) { return _to AppendItem(label, value, _retval); } \
  NS_IMETHOD InsertItemAt(PRInt32 index, const nsAString & label, const nsAString & value, nsIDOMXULSelectControlItemElement **_retval) { return _to InsertItemAt(index, label, value, _retval); } \
  NS_IMETHOD RemoveItemAt(PRInt32 index, nsIDOMXULSelectControlItemElement **_retval) { return _to RemoveItemAt(index, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMXULSELECTCONTROLELEMENT(_to) \
  NS_IMETHOD GetSelectedItem(nsIDOMXULSelectControlItemElement * *aSelectedItem) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSelectedItem(aSelectedItem); } \
  NS_IMETHOD SetSelectedItem(nsIDOMXULSelectControlItemElement * aSelectedItem) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSelectedItem(aSelectedItem); } \
  NS_IMETHOD GetSelectedIndex(PRInt32 *aSelectedIndex) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSelectedIndex(aSelectedIndex); } \
  NS_IMETHOD SetSelectedIndex(PRInt32 aSelectedIndex) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSelectedIndex(aSelectedIndex); } \
  NS_IMETHOD GetValue(nsAString & aValue) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetValue(aValue); } \
  NS_IMETHOD SetValue(const nsAString & aValue) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetValue(aValue); } \
  NS_IMETHOD AppendItem(const nsAString & label, const nsAString & value, nsIDOMXULSelectControlItemElement **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->AppendItem(label, value, _retval); } \
  NS_IMETHOD InsertItemAt(PRInt32 index, const nsAString & label, const nsAString & value, nsIDOMXULSelectControlItemElement **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->InsertItemAt(index, label, value, _retval); } \
  NS_IMETHOD RemoveItemAt(PRInt32 index, nsIDOMXULSelectControlItemElement **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveItemAt(index, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMXULSelectControlElement : public nsIDOMXULSelectControlElement
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMXULSELECTCONTROLELEMENT

  nsDOMXULSelectControlElement();

private:
  ~nsDOMXULSelectControlElement();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMXULSelectControlElement, nsIDOMXULSelectControlElement)

nsDOMXULSelectControlElement::nsDOMXULSelectControlElement()
{
  /* member initializers and constructor code */
}

nsDOMXULSelectControlElement::~nsDOMXULSelectControlElement()
{
  /* destructor code */
}

/* attribute nsIDOMXULSelectControlItemElement selectedItem; */
NS_IMETHODIMP nsDOMXULSelectControlElement::GetSelectedItem(nsIDOMXULSelectControlItemElement * *aSelectedItem)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMXULSelectControlElement::SetSelectedItem(nsIDOMXULSelectControlItemElement * aSelectedItem)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long selectedIndex; */
NS_IMETHODIMP nsDOMXULSelectControlElement::GetSelectedIndex(PRInt32 *aSelectedIndex)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMXULSelectControlElement::SetSelectedIndex(PRInt32 aSelectedIndex)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString value; */
NS_IMETHODIMP nsDOMXULSelectControlElement::GetValue(nsAString & aValue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMXULSelectControlElement::SetValue(const nsAString & aValue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMXULSelectControlItemElement appendItem (in DOMString label, in DOMString value); */
NS_IMETHODIMP nsDOMXULSelectControlElement::AppendItem(const nsAString & label, const nsAString & value, nsIDOMXULSelectControlItemElement **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMXULSelectControlItemElement insertItemAt (in long index, in DOMString label, in DOMString value); */
NS_IMETHODIMP nsDOMXULSelectControlElement::InsertItemAt(PRInt32 index, const nsAString & label, const nsAString & value, nsIDOMXULSelectControlItemElement **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMXULSelectControlItemElement removeItemAt (in long index); */
NS_IMETHODIMP nsDOMXULSelectControlElement::RemoveItemAt(PRInt32 index, nsIDOMXULSelectControlItemElement **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMXULSelectCntrlEl_h__ */
