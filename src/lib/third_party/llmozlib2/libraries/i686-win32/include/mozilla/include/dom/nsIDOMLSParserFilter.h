/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/ls/nsIDOMLSParserFilter.idl
 */

#ifndef __gen_nsIDOMLSParserFilter_h__
#define __gen_nsIDOMLSParserFilter_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMLSParserFilter */
#define NS_IDOMLSPARSERFILTER_IID_STR "10e8893d-ddf5-45d1-8872-615d72065fb4"

#define NS_IDOMLSPARSERFILTER_IID \
  {0x10e8893d, 0xddf5, 0x45d1, \
    { 0x88, 0x72, 0x61, 0x5d, 0x72, 0x06, 0x5f, 0xb4 }}

class NS_NO_VTABLE nsIDOMLSParserFilter : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMLSPARSERFILTER_IID)

  enum { FILTER_ACCEPT = 1 };

  enum { FILTER_REJECT = 2 };

  enum { FILTER_SKIP = 3 };

  enum { FILTER_INTERRUPT = 4 };

  /* unsigned short startElement (in nsIDOMElement elementArg); */
  NS_IMETHOD StartElement(nsIDOMElement *elementArg, PRUint16 *_retval) = 0;

  /* unsigned short acceptNode (in nsIDOMNode nodeArg); */
  NS_IMETHOD AcceptNode(nsIDOMNode *nodeArg, PRUint16 *_retval) = 0;

  /* readonly attribute unsigned long whatToShow; */
  NS_IMETHOD GetWhatToShow(PRUint32 *aWhatToShow) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMLSPARSERFILTER \
  NS_IMETHOD StartElement(nsIDOMElement *elementArg, PRUint16 *_retval); \
  NS_IMETHOD AcceptNode(nsIDOMNode *nodeArg, PRUint16 *_retval); \
  NS_IMETHOD GetWhatToShow(PRUint32 *aWhatToShow); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMLSPARSERFILTER(_to) \
  NS_IMETHOD StartElement(nsIDOMElement *elementArg, PRUint16 *_retval) { return _to StartElement(elementArg, _retval); } \
  NS_IMETHOD AcceptNode(nsIDOMNode *nodeArg, PRUint16 *_retval) { return _to AcceptNode(nodeArg, _retval); } \
  NS_IMETHOD GetWhatToShow(PRUint32 *aWhatToShow) { return _to GetWhatToShow(aWhatToShow); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMLSPARSERFILTER(_to) \
  NS_IMETHOD StartElement(nsIDOMElement *elementArg, PRUint16 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->StartElement(elementArg, _retval); } \
  NS_IMETHOD AcceptNode(nsIDOMNode *nodeArg, PRUint16 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->AcceptNode(nodeArg, _retval); } \
  NS_IMETHOD GetWhatToShow(PRUint32 *aWhatToShow) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetWhatToShow(aWhatToShow); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMLSParserFilter : public nsIDOMLSParserFilter
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMLSPARSERFILTER

  nsDOMLSParserFilter();

private:
  ~nsDOMLSParserFilter();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMLSParserFilter, nsIDOMLSParserFilter)

nsDOMLSParserFilter::nsDOMLSParserFilter()
{
  /* member initializers and constructor code */
}

nsDOMLSParserFilter::~nsDOMLSParserFilter()
{
  /* destructor code */
}

/* unsigned short startElement (in nsIDOMElement elementArg); */
NS_IMETHODIMP nsDOMLSParserFilter::StartElement(nsIDOMElement *elementArg, PRUint16 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* unsigned short acceptNode (in nsIDOMNode nodeArg); */
NS_IMETHODIMP nsDOMLSParserFilter::AcceptNode(nsIDOMNode *nodeArg, PRUint16 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long whatToShow; */
NS_IMETHODIMP nsDOMLSParserFilter::GetWhatToShow(PRUint32 *aWhatToShow)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMLSParserFilter_h__ */
