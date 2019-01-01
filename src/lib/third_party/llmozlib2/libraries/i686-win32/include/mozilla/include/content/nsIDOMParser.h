/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/base/public/nsIDOMParser.idl
 */

#ifndef __gen_nsIDOMParser_h__
#define __gen_nsIDOMParser_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIInputStream; /* forward declaration */

class nsIDOMDocument; /* forward declaration */

class nsIURI; /* forward declaration */


/* starting interface:    nsIDOMParser */
#define NS_IDOMPARSER_IID_STR "4f45513e-55e5-411c-a844-e899057026c1"

#define NS_IDOMPARSER_IID \
  {0x4f45513e, 0x55e5, 0x411c, \
    { 0xa8, 0x44, 0xe8, 0x99, 0x05, 0x70, 0x26, 0xc1 }}

/**
 * The nsIDOMParser interface is a non-SAX interface that can be used
 * to parse a string or byte stream containing XML or HTML content
 * to a DOM document. Parsing is always synchronous - a document is always
 * returned from the parsing methods. This is as opposed to loading and
 * parsing with the XMLHttpRequest interface, which can be used for
 * asynchronous (callback-based) loading.
 */
class NS_NO_VTABLE nsIDOMParser : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMPARSER_IID)

  /**
   * The string passed in is parsed into a DOM document.
   *
   * @param str The UTF16 string to be parsed
   * @param contentType The content type of the string (see parseFromStream)
   * @returns The DOM document created as a result of parsing the 
   *          string
   */
  /* nsIDOMDocument parseFromString (in wstring str, in string contentType); */
  NS_IMETHOD ParseFromString(const PRUnichar *str, const char *contentType, nsIDOMDocument **_retval) = 0;

  /**
   * The buffer is parsed into a DOM document.
   * The charset is determined from the xml entity decl.
   *
   * @param buf The octet array data to be parsed
   * @param bufLen Length (in bytes) of the data
   * @param contentType The content type of the data (see parseFromStream)
   * @returns The DOM document created as a result of parsing the 
   *          string
   */
  /* nsIDOMDocument parseFromBuffer ([array, size_is (bufLen), const] in octet buf, in PRUint32 bufLen, in string contentType); */
  NS_IMETHOD ParseFromBuffer(const PRUint8 *buf, PRUint32 bufLen, const char *contentType, nsIDOMDocument **_retval) = 0;

  /**
   * The byte stream passed in is parsed into a DOM document.
   *
   * Not accessible from web content.
   *
   * @param stream The byte stream whose contents are parsed
   * @param charset The character set that was used to encode the byte
   *                stream. NULL if not specified.
   * @param contentLength The number of bytes in the input stream.
   * @param contentType The content type of the string - either text/xml,
   *                    application/xml, or application/xhtml+xml.
   *                    Must not be NULL.
   * @returns The DOM document created as a result of parsing the 
   *          stream
   */
  /* nsIDOMDocument parseFromStream (in nsIInputStream stream, in string charset, in long contentLength, in string contentType); */
  NS_IMETHOD ParseFromStream(nsIInputStream *stream, const char *charset, PRInt32 contentLength, const char *contentType, nsIDOMDocument **_retval) = 0;

  /**
   * Set/Get the baseURI, may be needed when called from native code.
   */
  /* [noscript] attribute nsIURI baseURI; */
  NS_IMETHOD GetBaseURI(nsIURI * *aBaseURI) = 0;
  NS_IMETHOD SetBaseURI(nsIURI * aBaseURI) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMPARSER \
  NS_IMETHOD ParseFromString(const PRUnichar *str, const char *contentType, nsIDOMDocument **_retval); \
  NS_IMETHOD ParseFromBuffer(const PRUint8 *buf, PRUint32 bufLen, const char *contentType, nsIDOMDocument **_retval); \
  NS_IMETHOD ParseFromStream(nsIInputStream *stream, const char *charset, PRInt32 contentLength, const char *contentType, nsIDOMDocument **_retval); \
  NS_IMETHOD GetBaseURI(nsIURI * *aBaseURI); \
  NS_IMETHOD SetBaseURI(nsIURI * aBaseURI); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMPARSER(_to) \
  NS_IMETHOD ParseFromString(const PRUnichar *str, const char *contentType, nsIDOMDocument **_retval) { return _to ParseFromString(str, contentType, _retval); } \
  NS_IMETHOD ParseFromBuffer(const PRUint8 *buf, PRUint32 bufLen, const char *contentType, nsIDOMDocument **_retval) { return _to ParseFromBuffer(buf, bufLen, contentType, _retval); } \
  NS_IMETHOD ParseFromStream(nsIInputStream *stream, const char *charset, PRInt32 contentLength, const char *contentType, nsIDOMDocument **_retval) { return _to ParseFromStream(stream, charset, contentLength, contentType, _retval); } \
  NS_IMETHOD GetBaseURI(nsIURI * *aBaseURI) { return _to GetBaseURI(aBaseURI); } \
  NS_IMETHOD SetBaseURI(nsIURI * aBaseURI) { return _to SetBaseURI(aBaseURI); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMPARSER(_to) \
  NS_IMETHOD ParseFromString(const PRUnichar *str, const char *contentType, nsIDOMDocument **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ParseFromString(str, contentType, _retval); } \
  NS_IMETHOD ParseFromBuffer(const PRUint8 *buf, PRUint32 bufLen, const char *contentType, nsIDOMDocument **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ParseFromBuffer(buf, bufLen, contentType, _retval); } \
  NS_IMETHOD ParseFromStream(nsIInputStream *stream, const char *charset, PRInt32 contentLength, const char *contentType, nsIDOMDocument **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ParseFromStream(stream, charset, contentLength, contentType, _retval); } \
  NS_IMETHOD GetBaseURI(nsIURI * *aBaseURI) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBaseURI(aBaseURI); } \
  NS_IMETHOD SetBaseURI(nsIURI * aBaseURI) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBaseURI(aBaseURI); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMParser : public nsIDOMParser
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMPARSER

  nsDOMParser();

private:
  ~nsDOMParser();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMParser, nsIDOMParser)

nsDOMParser::nsDOMParser()
{
  /* member initializers and constructor code */
}

nsDOMParser::~nsDOMParser()
{
  /* destructor code */
}

/* nsIDOMDocument parseFromString (in wstring str, in string contentType); */
NS_IMETHODIMP nsDOMParser::ParseFromString(const PRUnichar *str, const char *contentType, nsIDOMDocument **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMDocument parseFromBuffer ([array, size_is (bufLen), const] in octet buf, in PRUint32 bufLen, in string contentType); */
NS_IMETHODIMP nsDOMParser::ParseFromBuffer(const PRUint8 *buf, PRUint32 bufLen, const char *contentType, nsIDOMDocument **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMDocument parseFromStream (in nsIInputStream stream, in string charset, in long contentLength, in string contentType); */
NS_IMETHODIMP nsDOMParser::ParseFromStream(nsIInputStream *stream, const char *charset, PRInt32 contentLength, const char *contentType, nsIDOMDocument **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] attribute nsIURI baseURI; */
NS_IMETHODIMP nsDOMParser::GetBaseURI(nsIURI * *aBaseURI)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMParser::SetBaseURI(nsIURI * aBaseURI)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

#define NS_DOMPARSER_CID                            \
 { /* 3a8a3a50-512c-11d4-9a54-000064657374 */       \
   0x3a8a3a50, 0x512c, 0x11d4,                      \
  {0x9a, 0x54, 0x00, 0x00, 0x64, 0x65, 0x73, 0x74} }
#define NS_DOMPARSER_CONTRACTID \
"@mozilla.org/xmlextras/domparser;1"

#endif /* __gen_nsIDOMParser_h__ */
