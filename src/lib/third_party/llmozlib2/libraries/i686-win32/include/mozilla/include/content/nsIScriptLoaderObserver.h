/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/base/public/nsIScriptLoaderObserver.idl
 */

#ifndef __gen_nsIScriptLoaderObserver_h__
#define __gen_nsIScriptLoaderObserver_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIScriptElement; /* forward declaration */

class nsIURI; /* forward declaration */


/* starting interface:    nsIScriptLoaderObserver */
#define NS_ISCRIPTLOADEROBSERVER_IID_STR "501209d3-7edf-437d-9948-3c6d1c08ef7f"

#define NS_ISCRIPTLOADEROBSERVER_IID \
  {0x501209d3, 0x7edf, 0x437d, \
    { 0x99, 0x48, 0x3c, 0x6d, 0x1c, 0x08, 0xef, 0x7f }}

class NS_NO_VTABLE nsIScriptLoaderObserver : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISCRIPTLOADEROBSERVER_IID)

  /**
   * The script is available for evaluation. For inline scripts, this
   * method will be called synchronously. For externally loaded scripts,
   * this method will be called when the load completes.
   *
   * @param aResult A result code representing the result of loading
   *        a script. If this is a failure code, script evaluation
   *        will not occur.
   * @param aElement The element being processed.
   * @param aIsInline Is this an inline script or externally loaded?
   * @param aWasPending Did script processing have to be delayed,
   *                    either for loading of an external script or
   *                    because processing of an earlier scheduled
   *                    script was delayed?
   * @param aURI What is the URI of the script (the document URI if
   *        it is inline).
   * @param aLineNo At what line does the script appear (generally 1
   *        if it is a loaded script).
   * @param aScript String representation of the string to be evaluated.
   */
  /* void scriptAvailable (in nsresult aResult, in nsIScriptElement aElement, in boolean aIsInline, in boolean aWasPending, in nsIURI aURI, in PRInt32 aLineNo, in AString aScript); */
  NS_IMETHOD ScriptAvailable(nsresult aResult, nsIScriptElement *aElement, PRBool aIsInline, PRBool aWasPending, nsIURI *aURI, PRInt32 aLineNo, const nsAString & aScript) = 0;

  /**
   * The script has been evaluated.
   *
   * @param aResult A result code representing the success or failure of
   *        the script evaluation.
   * @param aElement The element being processed.
   * @param aIsInline Is this an inline script or externally loaded?
   * @param aWasPending Did script processing have to be delayed,
   *                    either for loading of an external script or
   *                    because processing of an earlier scheduled
   *                    script was delayed?
   */
  /* void scriptEvaluated (in nsresult aResult, in nsIScriptElement aElement, in boolean aIsInline, in boolean aWasPending); */
  NS_IMETHOD ScriptEvaluated(nsresult aResult, nsIScriptElement *aElement, PRBool aIsInline, PRBool aWasPending) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISCRIPTLOADEROBSERVER \
  NS_IMETHOD ScriptAvailable(nsresult aResult, nsIScriptElement *aElement, PRBool aIsInline, PRBool aWasPending, nsIURI *aURI, PRInt32 aLineNo, const nsAString & aScript); \
  NS_IMETHOD ScriptEvaluated(nsresult aResult, nsIScriptElement *aElement, PRBool aIsInline, PRBool aWasPending); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISCRIPTLOADEROBSERVER(_to) \
  NS_IMETHOD ScriptAvailable(nsresult aResult, nsIScriptElement *aElement, PRBool aIsInline, PRBool aWasPending, nsIURI *aURI, PRInt32 aLineNo, const nsAString & aScript) { return _to ScriptAvailable(aResult, aElement, aIsInline, aWasPending, aURI, aLineNo, aScript); } \
  NS_IMETHOD ScriptEvaluated(nsresult aResult, nsIScriptElement *aElement, PRBool aIsInline, PRBool aWasPending) { return _to ScriptEvaluated(aResult, aElement, aIsInline, aWasPending); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISCRIPTLOADEROBSERVER(_to) \
  NS_IMETHOD ScriptAvailable(nsresult aResult, nsIScriptElement *aElement, PRBool aIsInline, PRBool aWasPending, nsIURI *aURI, PRInt32 aLineNo, const nsAString & aScript) { return !_to ? NS_ERROR_NULL_POINTER : _to->ScriptAvailable(aResult, aElement, aIsInline, aWasPending, aURI, aLineNo, aScript); } \
  NS_IMETHOD ScriptEvaluated(nsresult aResult, nsIScriptElement *aElement, PRBool aIsInline, PRBool aWasPending) { return !_to ? NS_ERROR_NULL_POINTER : _to->ScriptEvaluated(aResult, aElement, aIsInline, aWasPending); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsScriptLoaderObserver : public nsIScriptLoaderObserver
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISCRIPTLOADEROBSERVER

  nsScriptLoaderObserver();

private:
  ~nsScriptLoaderObserver();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsScriptLoaderObserver, nsIScriptLoaderObserver)

nsScriptLoaderObserver::nsScriptLoaderObserver()
{
  /* member initializers and constructor code */
}

nsScriptLoaderObserver::~nsScriptLoaderObserver()
{
  /* destructor code */
}

/* void scriptAvailable (in nsresult aResult, in nsIScriptElement aElement, in boolean aIsInline, in boolean aWasPending, in nsIURI aURI, in PRInt32 aLineNo, in AString aScript); */
NS_IMETHODIMP nsScriptLoaderObserver::ScriptAvailable(nsresult aResult, nsIScriptElement *aElement, PRBool aIsInline, PRBool aWasPending, nsIURI *aURI, PRInt32 aLineNo, const nsAString & aScript)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void scriptEvaluated (in nsresult aResult, in nsIScriptElement aElement, in boolean aIsInline, in boolean aWasPending); */
NS_IMETHODIMP nsScriptLoaderObserver::ScriptEvaluated(nsresult aResult, nsIScriptElement *aElement, PRBool aIsInline, PRBool aWasPending)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIScriptLoaderObserver_h__ */
