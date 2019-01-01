/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/io/nsILocalFileWin.idl
 */

#ifndef __gen_nsILocalFileWin_h__
#define __gen_nsILocalFileWin_h__


#ifndef __gen_nsILocalFile_h__
#include "nsILocalFile.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsILocalFileWin */
#define NS_ILOCALFILEWIN_IID_STR "dc42f467-4094-437d-9e3e-8912a072aede"

#define NS_ILOCALFILEWIN_IID \
  {0xdc42f467, 0x4094, 0x437d, \
    { 0x9e, 0x3e, 0x89, 0x12, 0xa0, 0x72, 0xae, 0xde }}

class NS_NO_VTABLE nsILocalFileWin : public nsILocalFile {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ILOCALFILEWIN_IID)

  /**
    * getVersionInfoValue
    *
    * Retrieve a metadata field from the file's VERSIONINFO block.
    * Throws NS_ERROR_FAILURE if no value is found, or the value is empty.
    *
    * @param   aField         The field to look up.
    *
    */
  /* AString getVersionInfoField (in string aField); */
  NS_IMETHOD GetVersionInfoField(const char *aField, nsAString & _retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSILOCALFILEWIN \
  NS_IMETHOD GetVersionInfoField(const char *aField, nsAString & _retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSILOCALFILEWIN(_to) \
  NS_IMETHOD GetVersionInfoField(const char *aField, nsAString & _retval) { return _to GetVersionInfoField(aField, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSILOCALFILEWIN(_to) \
  NS_IMETHOD GetVersionInfoField(const char *aField, nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetVersionInfoField(aField, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsLocalFileWin : public nsILocalFileWin
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSILOCALFILEWIN

  nsLocalFileWin();

private:
  ~nsLocalFileWin();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsLocalFileWin, nsILocalFileWin)

nsLocalFileWin::nsLocalFileWin()
{
  /* member initializers and constructor code */
}

nsLocalFileWin::~nsLocalFileWin()
{
  /* destructor code */
}

/* AString getVersionInfoField (in string aField); */
NS_IMETHODIMP nsLocalFileWin::GetVersionInfoField(const char *aField, nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsILocalFileWin_h__ */
