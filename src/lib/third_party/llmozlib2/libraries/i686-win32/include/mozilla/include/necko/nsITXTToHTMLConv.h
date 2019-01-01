/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/streamconv/public/nsITXTToHTMLConv.idl
 */

#ifndef __gen_nsITXTToHTMLConv_h__
#define __gen_nsITXTToHTMLConv_h__


#ifndef __gen_nsIStreamConverter_h__
#include "nsIStreamConverter.h"
#endif

#ifndef __gen_nsrootidl_h__
#include "nsrootidl.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsITXTToHTMLConv */
#define NS_ITXTTOHTMLCONV_IID_STR "933355f6-1dd2-11b2-a9b0-d335b9e35983"

#define NS_ITXTTOHTMLCONV_IID \
  {0x933355f6, 0x1dd2, 0x11b2, \
    { 0xa9, 0xb0, 0xd3, 0x35, 0xb9, 0xe3, 0x59, 0x83 }}

class NS_NO_VTABLE nsITXTToHTMLConv : public nsIStreamConverter {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ITXTTOHTMLCONV_IID)

  /**
   * @param text: Title to set for the HTML document.  Only applicable if
   *              preFormatHTML(true) is called.
   * @result      The given title will be used to form an HTML document
   *              from the plain text document.
   */
  /* void setTitle (in wstring text); */
  NS_IMETHOD SetTitle(const PRUnichar *text) = 0;

  /**
   * @param value: true to use an HTML header and footer on the document,
   *               false to omit it.
   * @result       The document will use a header and footer if value is
   *               true.
   */
  /* void preFormatHTML (in boolean value); */
  NS_IMETHOD PreFormatHTML(PRBool value) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSITXTTOHTMLCONV \
  NS_IMETHOD SetTitle(const PRUnichar *text); \
  NS_IMETHOD PreFormatHTML(PRBool value); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSITXTTOHTMLCONV(_to) \
  NS_IMETHOD SetTitle(const PRUnichar *text) { return _to SetTitle(text); } \
  NS_IMETHOD PreFormatHTML(PRBool value) { return _to PreFormatHTML(value); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSITXTTOHTMLCONV(_to) \
  NS_IMETHOD SetTitle(const PRUnichar *text) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTitle(text); } \
  NS_IMETHOD PreFormatHTML(PRBool value) { return !_to ? NS_ERROR_NULL_POINTER : _to->PreFormatHTML(value); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsTXTToHTMLConv : public nsITXTToHTMLConv
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSITXTTOHTMLCONV

  nsTXTToHTMLConv();

private:
  ~nsTXTToHTMLConv();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsTXTToHTMLConv, nsITXTToHTMLConv)

nsTXTToHTMLConv::nsTXTToHTMLConv()
{
  /* member initializers and constructor code */
}

nsTXTToHTMLConv::~nsTXTToHTMLConv()
{
  /* destructor code */
}

/* void setTitle (in wstring text); */
NS_IMETHODIMP nsTXTToHTMLConv::SetTitle(const PRUnichar *text)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void preFormatHTML (in boolean value); */
NS_IMETHODIMP nsTXTToHTMLConv::PreFormatHTML(PRBool value)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsITXTToHTMLConv_h__ */
