/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/base/nsIDOMMimeTypeArray.idl
 */

#ifndef __gen_nsIDOMMimeTypeArray_h__
#define __gen_nsIDOMMimeTypeArray_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMMimeTypeArray */
#define NS_IDOMMIMETYPEARRAY_IID_STR "f6134683-f28b-11d2-8360-c90899049c3c"

#define NS_IDOMMIMETYPEARRAY_IID \
  {0xf6134683, 0xf28b, 0x11d2, \
    { 0x83, 0x60, 0xc9, 0x08, 0x99, 0x04, 0x9c, 0x3c }}

class NS_NO_VTABLE nsIDOMMimeTypeArray : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMMIMETYPEARRAY_IID)

  /* readonly attribute unsigned long length; */
  NS_IMETHOD GetLength(PRUint32 *aLength) = 0;

  /* nsIDOMMimeType item (in unsigned long index); */
  NS_IMETHOD Item(PRUint32 index, nsIDOMMimeType **_retval) = 0;

  /* nsIDOMMimeType namedItem (in DOMString name); */
  NS_IMETHOD NamedItem(const nsAString & name, nsIDOMMimeType **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMMIMETYPEARRAY \
  NS_IMETHOD GetLength(PRUint32 *aLength); \
  NS_IMETHOD Item(PRUint32 index, nsIDOMMimeType **_retval); \
  NS_IMETHOD NamedItem(const nsAString & name, nsIDOMMimeType **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMMIMETYPEARRAY(_to) \
  NS_IMETHOD GetLength(PRUint32 *aLength) { return _to GetLength(aLength); } \
  NS_IMETHOD Item(PRUint32 index, nsIDOMMimeType **_retval) { return _to Item(index, _retval); } \
  NS_IMETHOD NamedItem(const nsAString & name, nsIDOMMimeType **_retval) { return _to NamedItem(name, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMMIMETYPEARRAY(_to) \
  NS_IMETHOD GetLength(PRUint32 *aLength) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLength(aLength); } \
  NS_IMETHOD Item(PRUint32 index, nsIDOMMimeType **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Item(index, _retval); } \
  NS_IMETHOD NamedItem(const nsAString & name, nsIDOMMimeType **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->NamedItem(name, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMMimeTypeArray : public nsIDOMMimeTypeArray
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMMIMETYPEARRAY

  nsDOMMimeTypeArray();

private:
  ~nsDOMMimeTypeArray();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMMimeTypeArray, nsIDOMMimeTypeArray)

nsDOMMimeTypeArray::nsDOMMimeTypeArray()
{
  /* member initializers and constructor code */
}

nsDOMMimeTypeArray::~nsDOMMimeTypeArray()
{
  /* destructor code */
}

/* readonly attribute unsigned long length; */
NS_IMETHODIMP nsDOMMimeTypeArray::GetLength(PRUint32 *aLength)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMMimeType item (in unsigned long index); */
NS_IMETHODIMP nsDOMMimeTypeArray::Item(PRUint32 index, nsIDOMMimeType **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMMimeType namedItem (in DOMString name); */
NS_IMETHODIMP nsDOMMimeTypeArray::NamedItem(const nsAString & name, nsIDOMMimeType **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMMimeTypeArray_h__ */
