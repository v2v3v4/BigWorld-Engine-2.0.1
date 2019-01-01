/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/base/public/nsIFrameLoader.idl
 */

#ifndef __gen_nsIFrameLoader_h__
#define __gen_nsIFrameLoader_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDocShell; /* forward declaration */


/* starting interface:    nsIFrameLoader */
#define NS_IFRAMELOADER_IID_STR "88800e93-c6af-4d69-9ee0-29c1100ff431"

#define NS_IFRAMELOADER_IID \
  {0x88800e93, 0xc6af, 0x4d69, \
    { 0x9e, 0xe0, 0x29, 0xc1, 0x10, 0x0f, 0xf4, 0x31 }}

class NS_NO_VTABLE nsIFrameLoader : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IFRAMELOADER_IID)

  /**
   * Get the docshell from the frame loader.
   */
  /* readonly attribute nsIDocShell docShell; */
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell) = 0;

  /**
   * Start loading the frame. This method figures out what to load
   * from the owner content in the frame loader.
   */
  /* void loadFrame (); */
  NS_IMETHOD LoadFrame(void) = 0;

  /**
   * Destroy the frame loader and everything inside it. This will
   * clear the weak owner content reference.
   */
  /* void destroy (); */
  NS_IMETHOD Destroy(void) = 0;

  /**
   * Find out whether the loader's frame is at too great a depth in
   * the frame tree.  This can be used to decide what operations may
   * or may not be allowed on the loader's docshell.
   */
  /* readonly attribute boolean depthTooGreat; */
  NS_IMETHOD GetDepthTooGreat(PRBool *aDepthTooGreat) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIFRAMELOADER \
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell); \
  NS_IMETHOD LoadFrame(void); \
  NS_IMETHOD Destroy(void); \
  NS_IMETHOD GetDepthTooGreat(PRBool *aDepthTooGreat); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIFRAMELOADER(_to) \
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell) { return _to GetDocShell(aDocShell); } \
  NS_IMETHOD LoadFrame(void) { return _to LoadFrame(); } \
  NS_IMETHOD Destroy(void) { return _to Destroy(); } \
  NS_IMETHOD GetDepthTooGreat(PRBool *aDepthTooGreat) { return _to GetDepthTooGreat(aDepthTooGreat); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIFRAMELOADER(_to) \
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDocShell(aDocShell); } \
  NS_IMETHOD LoadFrame(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->LoadFrame(); } \
  NS_IMETHOD Destroy(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Destroy(); } \
  NS_IMETHOD GetDepthTooGreat(PRBool *aDepthTooGreat) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDepthTooGreat(aDepthTooGreat); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsFrameLoader : public nsIFrameLoader
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIFRAMELOADER

  nsFrameLoader();

private:
  ~nsFrameLoader();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsFrameLoader, nsIFrameLoader)

nsFrameLoader::nsFrameLoader()
{
  /* member initializers and constructor code */
}

nsFrameLoader::~nsFrameLoader()
{
  /* destructor code */
}

/* readonly attribute nsIDocShell docShell; */
NS_IMETHODIMP nsFrameLoader::GetDocShell(nsIDocShell * *aDocShell)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void loadFrame (); */
NS_IMETHODIMP nsFrameLoader::LoadFrame()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void destroy (); */
NS_IMETHODIMP nsFrameLoader::Destroy()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean depthTooGreat; */
NS_IMETHODIMP nsFrameLoader::GetDepthTooGreat(PRBool *aDepthTooGreat)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIFrameLoaderOwner */
#define NS_IFRAMELOADEROWNER_IID_STR "feaf9285-05ac-4898-a69f-c3bd350767e4"

#define NS_IFRAMELOADEROWNER_IID \
  {0xfeaf9285, 0x05ac, 0x4898, \
    { 0xa6, 0x9f, 0xc3, 0xbd, 0x35, 0x07, 0x67, 0xe4 }}

class NS_NO_VTABLE nsIFrameLoaderOwner : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IFRAMELOADEROWNER_IID)

  /* readonly attribute nsIFrameLoader frameLoader; */
  NS_IMETHOD GetFrameLoader(nsIFrameLoader * *aFrameLoader) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIFRAMELOADEROWNER \
  NS_IMETHOD GetFrameLoader(nsIFrameLoader * *aFrameLoader); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIFRAMELOADEROWNER(_to) \
  NS_IMETHOD GetFrameLoader(nsIFrameLoader * *aFrameLoader) { return _to GetFrameLoader(aFrameLoader); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIFRAMELOADEROWNER(_to) \
  NS_IMETHOD GetFrameLoader(nsIFrameLoader * *aFrameLoader) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFrameLoader(aFrameLoader); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsFrameLoaderOwner : public nsIFrameLoaderOwner
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIFRAMELOADEROWNER

  nsFrameLoaderOwner();

private:
  ~nsFrameLoaderOwner();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsFrameLoaderOwner, nsIFrameLoaderOwner)

nsFrameLoaderOwner::nsFrameLoaderOwner()
{
  /* member initializers and constructor code */
}

nsFrameLoaderOwner::~nsFrameLoaderOwner()
{
  /* destructor code */
}

/* readonly attribute nsIFrameLoader frameLoader; */
NS_IMETHODIMP nsFrameLoaderOwner::GetFrameLoader(nsIFrameLoader * *aFrameLoader)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIFrameLoader_h__ */
