/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/base/public/nsIScriptLoader.idl
 */

#ifndef __gen_nsIScriptLoader_h__
#define __gen_nsIScriptLoader_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDocument; /* forward declaration */

class nsIScriptElement; /* forward declaration */

class nsIScriptLoaderObserver; /* forward declaration */


/* starting interface:    nsIScriptLoader */
#define NS_ISCRIPTLOADER_IID_STR "339a4eb5-dac6-4034-8c43-f4f8c645ce57"

#define NS_ISCRIPTLOADER_IID \
  {0x339a4eb5, 0xdac6, 0x4034, \
    { 0x8c, 0x43, 0xf4, 0xf8, 0xc6, 0x45, 0xce, 0x57 }}

class NS_NO_VTABLE nsIScriptLoader : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISCRIPTLOADER_IID)

  /**
   * Initialize loader with a document. The container of this document
   * will be used for getting script evaluation information, including
   * the context in which to do the evaluation. The loader maintains a 
   * strong reference to the document.
   *
   * @param aDocument The document to use as the basis for script
   *        processing.
   */
  /* void init (in nsIDocument aDocument); */
  NS_IMETHOD Init(nsIDocument *aDocument) = 0;

  /**
   * The loader maintains a strong reference to the document with
   * which it is initialized. This call forces the reference to
   * be dropped.
   */
  /* void dropDocumentReference (); */
  NS_IMETHOD DropDocumentReference(void) = 0;

  /**
   * Add an observer for all scripts loaded through this loader.
   *
   * @param aObserver observer for all script processing.
   */
  /* void addObserver (in nsIScriptLoaderObserver aObserver); */
  NS_IMETHOD AddObserver(nsIScriptLoaderObserver *aObserver) = 0;

  /**
   * Remove an observer.
   *
   * @param aObserver observer to be removed
   */
  /* void removeObserver (in nsIScriptLoaderObserver aObserver); */
  NS_IMETHOD RemoveObserver(nsIScriptLoaderObserver *aObserver) = 0;

  /**
   * Process a script element. This will include both loading the 
   * source of the element if it is not inline and evaluating
   * the script itself.
   *
   * @param aElement The element representing the script to be loaded and
   *        evaluated.
   * @param aObserver An observer for this script load only
   *
   */
  /* void processScriptElement (in nsIScriptElement aElement, in nsIScriptLoaderObserver aObserver); */
  NS_IMETHOD ProcessScriptElement(nsIScriptElement *aElement, nsIScriptLoaderObserver *aObserver) = 0;

  /**
   * Gets the currently executing script. This is useful if you want to
   * generate a unique key based on the currently executing script.
   */
  /* nsIScriptElement getCurrentScript (); */
  NS_IMETHOD GetCurrentScript(nsIScriptElement **_retval) = 0;

  /**
   * Whether the loader is enabled or not.
   * When disabled, processing of new script elements is disabled. 
   * Any call to processScriptElement() will fail with a return code of
   * NS_ERROR_NOT_AVAILABLE. Note that this DOES NOT disable
   * currently loading or executing scripts.
   */
  /* attribute boolean enabled; */
  NS_IMETHOD GetEnabled(PRBool *aEnabled) = 0;
  NS_IMETHOD SetEnabled(PRBool aEnabled) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISCRIPTLOADER \
  NS_IMETHOD Init(nsIDocument *aDocument); \
  NS_IMETHOD DropDocumentReference(void); \
  NS_IMETHOD AddObserver(nsIScriptLoaderObserver *aObserver); \
  NS_IMETHOD RemoveObserver(nsIScriptLoaderObserver *aObserver); \
  NS_IMETHOD ProcessScriptElement(nsIScriptElement *aElement, nsIScriptLoaderObserver *aObserver); \
  NS_IMETHOD GetCurrentScript(nsIScriptElement **_retval); \
  NS_IMETHOD GetEnabled(PRBool *aEnabled); \
  NS_IMETHOD SetEnabled(PRBool aEnabled); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISCRIPTLOADER(_to) \
  NS_IMETHOD Init(nsIDocument *aDocument) { return _to Init(aDocument); } \
  NS_IMETHOD DropDocumentReference(void) { return _to DropDocumentReference(); } \
  NS_IMETHOD AddObserver(nsIScriptLoaderObserver *aObserver) { return _to AddObserver(aObserver); } \
  NS_IMETHOD RemoveObserver(nsIScriptLoaderObserver *aObserver) { return _to RemoveObserver(aObserver); } \
  NS_IMETHOD ProcessScriptElement(nsIScriptElement *aElement, nsIScriptLoaderObserver *aObserver) { return _to ProcessScriptElement(aElement, aObserver); } \
  NS_IMETHOD GetCurrentScript(nsIScriptElement **_retval) { return _to GetCurrentScript(_retval); } \
  NS_IMETHOD GetEnabled(PRBool *aEnabled) { return _to GetEnabled(aEnabled); } \
  NS_IMETHOD SetEnabled(PRBool aEnabled) { return _to SetEnabled(aEnabled); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISCRIPTLOADER(_to) \
  NS_IMETHOD Init(nsIDocument *aDocument) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(aDocument); } \
  NS_IMETHOD DropDocumentReference(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->DropDocumentReference(); } \
  NS_IMETHOD AddObserver(nsIScriptLoaderObserver *aObserver) { return !_to ? NS_ERROR_NULL_POINTER : _to->AddObserver(aObserver); } \
  NS_IMETHOD RemoveObserver(nsIScriptLoaderObserver *aObserver) { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveObserver(aObserver); } \
  NS_IMETHOD ProcessScriptElement(nsIScriptElement *aElement, nsIScriptLoaderObserver *aObserver) { return !_to ? NS_ERROR_NULL_POINTER : _to->ProcessScriptElement(aElement, aObserver); } \
  NS_IMETHOD GetCurrentScript(nsIScriptElement **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCurrentScript(_retval); } \
  NS_IMETHOD GetEnabled(PRBool *aEnabled) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetEnabled(aEnabled); } \
  NS_IMETHOD SetEnabled(PRBool aEnabled) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetEnabled(aEnabled); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsScriptLoader : public nsIScriptLoader
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISCRIPTLOADER

  nsScriptLoader();

private:
  ~nsScriptLoader();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsScriptLoader, nsIScriptLoader)

nsScriptLoader::nsScriptLoader()
{
  /* member initializers and constructor code */
}

nsScriptLoader::~nsScriptLoader()
{
  /* destructor code */
}

/* void init (in nsIDocument aDocument); */
NS_IMETHODIMP nsScriptLoader::Init(nsIDocument *aDocument)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void dropDocumentReference (); */
NS_IMETHODIMP nsScriptLoader::DropDocumentReference()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void addObserver (in nsIScriptLoaderObserver aObserver); */
NS_IMETHODIMP nsScriptLoader::AddObserver(nsIScriptLoaderObserver *aObserver)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeObserver (in nsIScriptLoaderObserver aObserver); */
NS_IMETHODIMP nsScriptLoader::RemoveObserver(nsIScriptLoaderObserver *aObserver)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void processScriptElement (in nsIScriptElement aElement, in nsIScriptLoaderObserver aObserver); */
NS_IMETHODIMP nsScriptLoader::ProcessScriptElement(nsIScriptElement *aElement, nsIScriptLoaderObserver *aObserver)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIScriptElement getCurrentScript (); */
NS_IMETHODIMP nsScriptLoader::GetCurrentScript(nsIScriptElement **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean enabled; */
NS_IMETHODIMP nsScriptLoader::GetEnabled(PRBool *aEnabled)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsScriptLoader::SetEnabled(PRBool aEnabled)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIScriptLoader_h__ */
