/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/core/nsIDOMDOMStringList.idl
 */

#ifndef __gen_nsIDOMDOMStringList_h__
#define __gen_nsIDOMDOMStringList_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMDOMStringList */
#define NS_IDOMDOMSTRINGLIST_IID_STR "0bbae65c-1dde-11d9-8c46-000a95dc234c"

#define NS_IDOMDOMSTRINGLIST_IID \
  {0x0bbae65c, 0x1dde, 0x11d9, \
    { 0x8c, 0x46, 0x00, 0x0a, 0x95, 0xdc, 0x23, 0x4c }}

class NS_NO_VTABLE nsIDOMDOMStringList : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMDOMSTRINGLIST_IID)

  /* DOMString item (in unsigned long index); */
  NS_IMETHOD Item(PRUint32 index, nsAString & _retval) = 0;

  /* readonly attribute unsigned long length; */
  NS_IMETHOD GetLength(PRUint32 *aLength) = 0;

  /* boolean contains (in DOMString str); */
  NS_IMETHOD Contains(const nsAString & str, PRBool *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMDOMSTRINGLIST \
  NS_IMETHOD Item(PRUint32 index, nsAString & _retval); \
  NS_IMETHOD GetLength(PRUint32 *aLength); \
  NS_IMETHOD Contains(const nsAString & str, PRBool *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMDOMSTRINGLIST(_to) \
  NS_IMETHOD Item(PRUint32 index, nsAString & _retval) { return _to Item(index, _retval); } \
  NS_IMETHOD GetLength(PRUint32 *aLength) { return _to GetLength(aLength); } \
  NS_IMETHOD Contains(const nsAString & str, PRBool *_retval) { return _to Contains(str, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMDOMSTRINGLIST(_to) \
  NS_IMETHOD Item(PRUint32 index, nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Item(index, _retval); } \
  NS_IMETHOD GetLength(PRUint32 *aLength) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLength(aLength); } \
  NS_IMETHOD Contains(const nsAString & str, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Contains(str, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMDOMStringList : public nsIDOMDOMStringList
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMDOMSTRINGLIST

  nsDOMDOMStringList();

private:
  ~nsDOMDOMStringList();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMDOMStringList, nsIDOMDOMStringList)

nsDOMDOMStringList::nsDOMDOMStringList()
{
  /* member initializers and constructor code */
}

nsDOMDOMStringList::~nsDOMDOMStringList()
{
  /* destructor code */
}

/* DOMString item (in unsigned long index); */
NS_IMETHODIMP nsDOMDOMStringList::Item(PRUint32 index, nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long length; */
NS_IMETHODIMP nsDOMDOMStringList::GetLength(PRUint32 *aLength)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean contains (in DOMString str); */
NS_IMETHODIMP nsDOMDOMStringList::Contains(const nsAString & str, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMDOMStringList_h__ */
