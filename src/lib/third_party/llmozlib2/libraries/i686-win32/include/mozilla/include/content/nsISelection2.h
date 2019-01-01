/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/base/public/nsISelection2.idl
 */

#ifndef __gen_nsISelection2_h__
#define __gen_nsISelection2_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsISelection_h__
#include "nsISelection.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMNode; /* forward declaration */

class nsIDOMRange; /* forward declaration */

#include "nsTArray.h"

/* starting interface:    nsISelection2 */
#define NS_ISELECTION2_IID_STR "eab4ae76-efdc-4e09-828c-bd921e9a662f"

#define NS_ISELECTION2_IID \
  {0xeab4ae76, 0xefdc, 0x4e09, \
    { 0x82, 0x8c, 0xbd, 0x92, 0x1e, 0x9a, 0x66, 0x2f }}

class NS_NO_VTABLE nsISelection2 : public nsISelection {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISELECTION2_IID)

  /* void GetRangesForInterval (in nsIDOMNode beginNode, in PRInt32 beginOffset, in nsIDOMNode endNode, in PRInt32 endOffset, in PRBool allowAdjacent, out PRUint32 resultCount, [array, size_is (resultCount), retval] out nsIDOMRange results); */
  NS_IMETHOD GetRangesForInterval(nsIDOMNode *beginNode, PRInt32 beginOffset, nsIDOMNode *endNode, PRInt32 endOffset, PRBool allowAdjacent, PRUint32 *resultCount, nsIDOMRange ***results) = 0;

  /* [noscript] void GetRangesForIntervalCOMArray (in nsIDOMNode beginNode, in PRInt32 beginOffset, in nsIDOMNode endNode, in PRInt32 endOffset, in PRBool allowAdjacent, in RangeArray results); */
  NS_IMETHOD GetRangesForIntervalCOMArray(nsIDOMNode *beginNode, PRInt32 beginOffset, nsIDOMNode *endNode, PRInt32 endOffset, PRBool allowAdjacent, nsCOMArray<nsIDOMRange> * results) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISELECTION2 \
  NS_IMETHOD GetRangesForInterval(nsIDOMNode *beginNode, PRInt32 beginOffset, nsIDOMNode *endNode, PRInt32 endOffset, PRBool allowAdjacent, PRUint32 *resultCount, nsIDOMRange ***results); \
  NS_IMETHOD GetRangesForIntervalCOMArray(nsIDOMNode *beginNode, PRInt32 beginOffset, nsIDOMNode *endNode, PRInt32 endOffset, PRBool allowAdjacent, nsCOMArray<nsIDOMRange> * results); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISELECTION2(_to) \
  NS_IMETHOD GetRangesForInterval(nsIDOMNode *beginNode, PRInt32 beginOffset, nsIDOMNode *endNode, PRInt32 endOffset, PRBool allowAdjacent, PRUint32 *resultCount, nsIDOMRange ***results) { return _to GetRangesForInterval(beginNode, beginOffset, endNode, endOffset, allowAdjacent, resultCount, results); } \
  NS_IMETHOD GetRangesForIntervalCOMArray(nsIDOMNode *beginNode, PRInt32 beginOffset, nsIDOMNode *endNode, PRInt32 endOffset, PRBool allowAdjacent, nsCOMArray<nsIDOMRange> * results) { return _to GetRangesForIntervalCOMArray(beginNode, beginOffset, endNode, endOffset, allowAdjacent, results); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISELECTION2(_to) \
  NS_IMETHOD GetRangesForInterval(nsIDOMNode *beginNode, PRInt32 beginOffset, nsIDOMNode *endNode, PRInt32 endOffset, PRBool allowAdjacent, PRUint32 *resultCount, nsIDOMRange ***results) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRangesForInterval(beginNode, beginOffset, endNode, endOffset, allowAdjacent, resultCount, results); } \
  NS_IMETHOD GetRangesForIntervalCOMArray(nsIDOMNode *beginNode, PRInt32 beginOffset, nsIDOMNode *endNode, PRInt32 endOffset, PRBool allowAdjacent, nsCOMArray<nsIDOMRange> * results) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRangesForIntervalCOMArray(beginNode, beginOffset, endNode, endOffset, allowAdjacent, results); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSelection2 : public nsISelection2
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISELECTION2

  nsSelection2();

private:
  ~nsSelection2();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSelection2, nsISelection2)

nsSelection2::nsSelection2()
{
  /* member initializers and constructor code */
}

nsSelection2::~nsSelection2()
{
  /* destructor code */
}

/* void GetRangesForInterval (in nsIDOMNode beginNode, in PRInt32 beginOffset, in nsIDOMNode endNode, in PRInt32 endOffset, in PRBool allowAdjacent, out PRUint32 resultCount, [array, size_is (resultCount), retval] out nsIDOMRange results); */
NS_IMETHODIMP nsSelection2::GetRangesForInterval(nsIDOMNode *beginNode, PRInt32 beginOffset, nsIDOMNode *endNode, PRInt32 endOffset, PRBool allowAdjacent, PRUint32 *resultCount, nsIDOMRange ***results)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void GetRangesForIntervalCOMArray (in nsIDOMNode beginNode, in PRInt32 beginOffset, in nsIDOMNode endNode, in PRInt32 endOffset, in PRBool allowAdjacent, in RangeArray results); */
NS_IMETHODIMP nsSelection2::GetRangesForIntervalCOMArray(nsIDOMNode *beginNode, PRInt32 beginOffset, nsIDOMNode *endNode, PRInt32 endOffset, PRBool allowAdjacent, nsCOMArray<nsIDOMRange> * results)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISelection2_h__ */
