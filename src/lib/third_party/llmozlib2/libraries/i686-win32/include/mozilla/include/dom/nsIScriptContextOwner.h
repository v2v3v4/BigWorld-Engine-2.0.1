/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/nsIScriptContextOwner.idl
 */

#ifndef __gen_nsIScriptContextOwner_h__
#define __gen_nsIScriptContextOwner_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "nscore.h"
#include "nsIScriptContext.h"
class nsIScriptContext; /* forward declaration */

class nsIScriptGlobalObject; /* forward declaration */


/* starting interface:    nsIScriptContextOwner */
#define NS_ISCRIPTCONTEXTOWNER_IID_STR "a94ec640-0bba-11d2-b326-00805f8a3859"

#define NS_ISCRIPTCONTEXTOWNER_IID \
  {0xa94ec640, 0x0bba, 0x11d2, \
    { 0xb3, 0x26, 0x00, 0x80, 0x5f, 0x8a, 0x38, 0x59 }}

class NS_NO_VTABLE nsIScriptContextOwner : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISCRIPTCONTEXTOWNER_IID)

  /**
   * Returns a script context. The assumption is that the
   * script context has an associated script global object and
   * is ready for script evaluation.
   */
  /* nsIScriptContext getScriptContext (); */
  NS_IMETHOD GetScriptContext(nsIScriptContext **_retval) = 0;

  /**
   * Returns the script global object
   */
  /* nsIScriptGlobalObject getScriptGlobalObject (); */
  NS_IMETHOD GetScriptGlobalObject(nsIScriptGlobalObject **_retval) = 0;

  /**
   * Called to indicate that the script context is no longer needed.
   * The caller should <B>not</B> also call the context's Release()
   * method.
   */
  /* void releaseScriptContext (in nsIScriptContext aContext); */
  NS_IMETHOD ReleaseScriptContext(nsIScriptContext *aContext) = 0;

  /**
   * Error notification method. Informs the owner that an error 
   * occurred while a script was being evaluted.
   */
  /* void reportScriptError (in string aErrorString, in string aFileName, in long aLineNo, in string aLineBuf); */
  NS_IMETHOD ReportScriptError(const char *aErrorString, const char *aFileName, PRInt32 aLineNo, const char *aLineBuf) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISCRIPTCONTEXTOWNER \
  NS_IMETHOD GetScriptContext(nsIScriptContext **_retval); \
  NS_IMETHOD GetScriptGlobalObject(nsIScriptGlobalObject **_retval); \
  NS_IMETHOD ReleaseScriptContext(nsIScriptContext *aContext); \
  NS_IMETHOD ReportScriptError(const char *aErrorString, const char *aFileName, PRInt32 aLineNo, const char *aLineBuf); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISCRIPTCONTEXTOWNER(_to) \
  NS_IMETHOD GetScriptContext(nsIScriptContext **_retval) { return _to GetScriptContext(_retval); } \
  NS_IMETHOD GetScriptGlobalObject(nsIScriptGlobalObject **_retval) { return _to GetScriptGlobalObject(_retval); } \
  NS_IMETHOD ReleaseScriptContext(nsIScriptContext *aContext) { return _to ReleaseScriptContext(aContext); } \
  NS_IMETHOD ReportScriptError(const char *aErrorString, const char *aFileName, PRInt32 aLineNo, const char *aLineBuf) { return _to ReportScriptError(aErrorString, aFileName, aLineNo, aLineBuf); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISCRIPTCONTEXTOWNER(_to) \
  NS_IMETHOD GetScriptContext(nsIScriptContext **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScriptContext(_retval); } \
  NS_IMETHOD GetScriptGlobalObject(nsIScriptGlobalObject **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScriptGlobalObject(_retval); } \
  NS_IMETHOD ReleaseScriptContext(nsIScriptContext *aContext) { return !_to ? NS_ERROR_NULL_POINTER : _to->ReleaseScriptContext(aContext); } \
  NS_IMETHOD ReportScriptError(const char *aErrorString, const char *aFileName, PRInt32 aLineNo, const char *aLineBuf) { return !_to ? NS_ERROR_NULL_POINTER : _to->ReportScriptError(aErrorString, aFileName, aLineNo, aLineBuf); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsScriptContextOwner : public nsIScriptContextOwner
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISCRIPTCONTEXTOWNER

  nsScriptContextOwner();

private:
  ~nsScriptContextOwner();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsScriptContextOwner, nsIScriptContextOwner)

nsScriptContextOwner::nsScriptContextOwner()
{
  /* member initializers and constructor code */
}

nsScriptContextOwner::~nsScriptContextOwner()
{
  /* destructor code */
}

/* nsIScriptContext getScriptContext (); */
NS_IMETHODIMP nsScriptContextOwner::GetScriptContext(nsIScriptContext **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIScriptGlobalObject getScriptGlobalObject (); */
NS_IMETHODIMP nsScriptContextOwner::GetScriptGlobalObject(nsIScriptGlobalObject **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void releaseScriptContext (in nsIScriptContext aContext); */
NS_IMETHODIMP nsScriptContextOwner::ReleaseScriptContext(nsIScriptContext *aContext)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void reportScriptError (in string aErrorString, in string aFileName, in long aLineNo, in string aLineBuf); */
NS_IMETHODIMP nsScriptContextOwner::ReportScriptError(const char *aErrorString, const char *aFileName, PRInt32 aLineNo, const char *aLineBuf)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIScriptContextOwner_h__ */
