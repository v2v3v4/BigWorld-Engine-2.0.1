/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/toolkit/xre/nsIWinAppHelper.idl
 */

#ifndef __gen_nsIWinAppHelper_h__
#define __gen_nsIWinAppHelper_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsILocalFile; /* forward declaration */


/* starting interface:    nsIWinAppHelper */
#define NS_IWINAPPHELPER_IID_STR "575249a7-de7a-4602-a997-b7ad2b3b6dab"

#define NS_IWINAPPHELPER_IID \
  {0x575249a7, 0xde7a, 0x4602, \
    { 0xa9, 0x97, 0xb7, 0xad, 0x2b, 0x3b, 0x6d, 0xab }}

class NS_NO_VTABLE nsIWinAppHelper : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IWINAPPHELPER_IID)

  /* void postUpdate (in nsILocalFile logFile); */
  NS_IMETHOD PostUpdate(nsILocalFile *logFile) = 0;

  /* void fixReg (); */
  NS_IMETHOD FixReg(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIWINAPPHELPER \
  NS_IMETHOD PostUpdate(nsILocalFile *logFile); \
  NS_IMETHOD FixReg(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIWINAPPHELPER(_to) \
  NS_IMETHOD PostUpdate(nsILocalFile *logFile) { return _to PostUpdate(logFile); } \
  NS_IMETHOD FixReg(void) { return _to FixReg(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIWINAPPHELPER(_to) \
  NS_IMETHOD PostUpdate(nsILocalFile *logFile) { return !_to ? NS_ERROR_NULL_POINTER : _to->PostUpdate(logFile); } \
  NS_IMETHOD FixReg(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->FixReg(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsWinAppHelper : public nsIWinAppHelper
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIWINAPPHELPER

  nsWinAppHelper();

private:
  ~nsWinAppHelper();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsWinAppHelper, nsIWinAppHelper)

nsWinAppHelper::nsWinAppHelper()
{
  /* member initializers and constructor code */
}

nsWinAppHelper::~nsWinAppHelper()
{
  /* destructor code */
}

/* void postUpdate (in nsILocalFile logFile); */
NS_IMETHODIMP nsWinAppHelper::PostUpdate(nsILocalFile *logFile)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void fixReg (); */
NS_IMETHODIMP nsWinAppHelper::FixReg()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIWinAppHelper_h__ */
