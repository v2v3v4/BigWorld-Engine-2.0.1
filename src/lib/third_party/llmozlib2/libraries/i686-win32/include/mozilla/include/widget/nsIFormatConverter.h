/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/widget/public/nsIFormatConverter.idl
 */

#ifndef __gen_nsIFormatConverter_h__
#define __gen_nsIFormatConverter_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsISupportsArray_h__
#include "nsISupportsArray.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIFormatConverter */
#define NS_IFORMATCONVERTER_IID_STR "948a0023-e3a7-11d2-96cf-0060b0fb9956"

#define NS_IFORMATCONVERTER_IID \
  {0x948a0023, 0xe3a7, 0x11d2, \
    { 0x96, 0xcf, 0x00, 0x60, 0xb0, 0xfb, 0x99, 0x56 }}

class NS_NO_VTABLE nsIFormatConverter : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IFORMATCONVERTER_IID)

  /**
    * Get the list of the "input" data flavors (mime types as nsISupportsCString),
    * in otherwords, the flavors that this converter can convert "from" (the 
    * incoming data to the converter).
    */
  /* nsISupportsArray getInputDataFlavors (); */
  NS_IMETHOD GetInputDataFlavors(nsISupportsArray **_retval) = 0;

  /**
    * Get the list of the "output" data flavors (mime types as nsISupportsCString),
    * in otherwords, the flavors that this converter can convert "to" (the 
    * outgoing data to the converter).
    *
    * @param  aDataFlavorList fills list with supported flavors
    */
  /* nsISupportsArray getOutputDataFlavors (); */
  NS_IMETHOD GetOutputDataFlavors(nsISupportsArray **_retval) = 0;

  /**
    * Determines whether a converion from one flavor to another is supported
    *
    * @param  aFromFormatConverter flavor to convert from
    * @param  aFromFormatConverter flavor to convert to
    */
  /* boolean canConvert (in string aFromDataFlavor, in string aToDataFlavor); */
  NS_IMETHOD CanConvert(const char *aFromDataFlavor, const char *aToDataFlavor, PRBool *_retval) = 0;

  /**
    * Converts from one flavor to another.
    *
    * @param  aFromFormatConverter flavor to convert from
    * @param  aFromFormatConverter flavor to convert to (destination own the memory)
    * @returns returns NS_OK if it was converted
    */
  /* void convert (in string aFromDataFlavor, in nsISupports aFromData, in unsigned long aDataLen, in string aToDataFlavor, out nsISupports aToData, out unsigned long aDataToLen); */
  NS_IMETHOD Convert(const char *aFromDataFlavor, nsISupports *aFromData, PRUint32 aDataLen, const char *aToDataFlavor, nsISupports **aToData, PRUint32 *aDataToLen) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIFORMATCONVERTER \
  NS_IMETHOD GetInputDataFlavors(nsISupportsArray **_retval); \
  NS_IMETHOD GetOutputDataFlavors(nsISupportsArray **_retval); \
  NS_IMETHOD CanConvert(const char *aFromDataFlavor, const char *aToDataFlavor, PRBool *_retval); \
  NS_IMETHOD Convert(const char *aFromDataFlavor, nsISupports *aFromData, PRUint32 aDataLen, const char *aToDataFlavor, nsISupports **aToData, PRUint32 *aDataToLen); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIFORMATCONVERTER(_to) \
  NS_IMETHOD GetInputDataFlavors(nsISupportsArray **_retval) { return _to GetInputDataFlavors(_retval); } \
  NS_IMETHOD GetOutputDataFlavors(nsISupportsArray **_retval) { return _to GetOutputDataFlavors(_retval); } \
  NS_IMETHOD CanConvert(const char *aFromDataFlavor, const char *aToDataFlavor, PRBool *_retval) { return _to CanConvert(aFromDataFlavor, aToDataFlavor, _retval); } \
  NS_IMETHOD Convert(const char *aFromDataFlavor, nsISupports *aFromData, PRUint32 aDataLen, const char *aToDataFlavor, nsISupports **aToData, PRUint32 *aDataToLen) { return _to Convert(aFromDataFlavor, aFromData, aDataLen, aToDataFlavor, aToData, aDataToLen); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIFORMATCONVERTER(_to) \
  NS_IMETHOD GetInputDataFlavors(nsISupportsArray **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetInputDataFlavors(_retval); } \
  NS_IMETHOD GetOutputDataFlavors(nsISupportsArray **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOutputDataFlavors(_retval); } \
  NS_IMETHOD CanConvert(const char *aFromDataFlavor, const char *aToDataFlavor, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CanConvert(aFromDataFlavor, aToDataFlavor, _retval); } \
  NS_IMETHOD Convert(const char *aFromDataFlavor, nsISupports *aFromData, PRUint32 aDataLen, const char *aToDataFlavor, nsISupports **aToData, PRUint32 *aDataToLen) { return !_to ? NS_ERROR_NULL_POINTER : _to->Convert(aFromDataFlavor, aFromData, aDataLen, aToDataFlavor, aToData, aDataToLen); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsFormatConverter : public nsIFormatConverter
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIFORMATCONVERTER

  nsFormatConverter();

private:
  ~nsFormatConverter();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsFormatConverter, nsIFormatConverter)

nsFormatConverter::nsFormatConverter()
{
  /* member initializers and constructor code */
}

nsFormatConverter::~nsFormatConverter()
{
  /* destructor code */
}

/* nsISupportsArray getInputDataFlavors (); */
NS_IMETHODIMP nsFormatConverter::GetInputDataFlavors(nsISupportsArray **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsISupportsArray getOutputDataFlavors (); */
NS_IMETHODIMP nsFormatConverter::GetOutputDataFlavors(nsISupportsArray **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean canConvert (in string aFromDataFlavor, in string aToDataFlavor); */
NS_IMETHODIMP nsFormatConverter::CanConvert(const char *aFromDataFlavor, const char *aToDataFlavor, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void convert (in string aFromDataFlavor, in nsISupports aFromData, in unsigned long aDataLen, in string aToDataFlavor, out nsISupports aToData, out unsigned long aDataToLen); */
NS_IMETHODIMP nsFormatConverter::Convert(const char *aFromDataFlavor, nsISupports *aFromData, PRUint32 aDataLen, const char *aToDataFlavor, nsISupports **aToData, PRUint32 *aDataToLen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIFormatConverter_h__ */
