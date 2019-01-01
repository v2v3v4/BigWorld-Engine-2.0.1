/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/components/nsIComponentLoader.idl
 */

#ifndef __gen_nsIComponentLoader_h__
#define __gen_nsIComponentLoader_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsIFactory_h__
#include "nsIFactory.h"
#endif

#ifndef __gen_nsIFile_h__
#include "nsIFile.h"
#endif

#ifndef __gen_nsIComponentManager_h__
#include "nsIComponentManager.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIComponentLoader */
#define NS_ICOMPONENTLOADER_IID_STR "c073cfc0-567c-11d3-aec1-0000f8e25c06"

#define NS_ICOMPONENTLOADER_IID \
  {0xc073cfc0, 0x567c, 0x11d3, \
    { 0xae, 0xc1, 0x00, 0x00, 0xf8, 0xe2, 0x5c, 0x06 }}

class NS_NO_VTABLE nsIComponentLoader : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICOMPONENTLOADER_IID)

  /**
     * Get the factory for a given component.
     */
  /* nsIFactory getFactory (in nsIIDRef aCID, in string aLocation, in string aType); */
  NS_IMETHOD GetFactory(const nsIID & aCID, const char *aLocation, const char *aType, nsIFactory **_retval) = 0;

  /**
     * Initialize the loader.
     *
     * We use nsISupports here because nsIRegistry isn't IDLized yet.
     */
  /* void init (in nsIComponentManager aCompMgr, in nsISupports aRegistry); */
  NS_IMETHOD Init(nsIComponentManager *aCompMgr, nsISupports *aRegistry) = 0;

  /**
     * Called when a component of the appropriate type is registered,
     * to give the component loader an opportunity to do things like
     * annotate the registry and such.
     */
  /* void onRegister (in nsIIDRef aCID, in string aType, in string aClassName, in string aContractID, in string aLocation, in boolean aReplace, in boolean aPersist); */
  NS_IMETHOD OnRegister(const nsIID & aCID, const char *aType, const char *aClassName, const char *aContractID, const char *aLocation, PRBool aReplace, PRBool aPersist) = 0;

  /**
     * When is AutoRegistration occuring?
     */
  enum { Startup = 0 };

  enum { Component = 1 };

  enum { Timer = 2 };

  /**
     * AutoRegister components in the given directory.
     */
  /* void autoRegisterComponents (in long aWhen, in nsIFile aDirectory); */
  NS_IMETHOD AutoRegisterComponents(PRInt32 aWhen, nsIFile *aDirectory) = 0;

  /**
     * AutoRegister the given component.
     *
     * Returns true if the component was registered, false if it couldn't
     * attempt to register the component (wrong type) and ``throws'' an
     * NS_FAILED code if there was an error during registration.
     */
  /* boolean autoRegisterComponent (in long aWhen, in nsIFile aComponent); */
  NS_IMETHOD AutoRegisterComponent(PRInt32 aWhen, nsIFile *aComponent, PRBool *_retval) = 0;

  /**
     * AutoUnregister the given component.
     * Returns true if the component was unregistered, false if it coudln't
     * attempt to unregister the component (not found, wrong type).
     */
  /* boolean autoUnregisterComponent (in long aWhen, in nsIFile aComponent); */
  NS_IMETHOD AutoUnregisterComponent(PRInt32 aWhen, nsIFile *aComponent, PRBool *_retval) = 0;

  /**
     * Register any deferred (NS_ERROR_FACTORY_REGISTER_AGAIN) components.
     * Return registered-any-components?
     */
  /* boolean registerDeferredComponents (in long aWhen); */
  NS_IMETHOD RegisterDeferredComponents(PRInt32 aWhen, PRBool *_retval) = 0;

  /**
     * Unload all components that are willing.
     */
  /* void unloadAll (in long aWhen); */
  NS_IMETHOD UnloadAll(PRInt32 aWhen) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICOMPONENTLOADER \
  NS_IMETHOD GetFactory(const nsIID & aCID, const char *aLocation, const char *aType, nsIFactory **_retval); \
  NS_IMETHOD Init(nsIComponentManager *aCompMgr, nsISupports *aRegistry); \
  NS_IMETHOD OnRegister(const nsIID & aCID, const char *aType, const char *aClassName, const char *aContractID, const char *aLocation, PRBool aReplace, PRBool aPersist); \
  NS_IMETHOD AutoRegisterComponents(PRInt32 aWhen, nsIFile *aDirectory); \
  NS_IMETHOD AutoRegisterComponent(PRInt32 aWhen, nsIFile *aComponent, PRBool *_retval); \
  NS_IMETHOD AutoUnregisterComponent(PRInt32 aWhen, nsIFile *aComponent, PRBool *_retval); \
  NS_IMETHOD RegisterDeferredComponents(PRInt32 aWhen, PRBool *_retval); \
  NS_IMETHOD UnloadAll(PRInt32 aWhen); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICOMPONENTLOADER(_to) \
  NS_IMETHOD GetFactory(const nsIID & aCID, const char *aLocation, const char *aType, nsIFactory **_retval) { return _to GetFactory(aCID, aLocation, aType, _retval); } \
  NS_IMETHOD Init(nsIComponentManager *aCompMgr, nsISupports *aRegistry) { return _to Init(aCompMgr, aRegistry); } \
  NS_IMETHOD OnRegister(const nsIID & aCID, const char *aType, const char *aClassName, const char *aContractID, const char *aLocation, PRBool aReplace, PRBool aPersist) { return _to OnRegister(aCID, aType, aClassName, aContractID, aLocation, aReplace, aPersist); } \
  NS_IMETHOD AutoRegisterComponents(PRInt32 aWhen, nsIFile *aDirectory) { return _to AutoRegisterComponents(aWhen, aDirectory); } \
  NS_IMETHOD AutoRegisterComponent(PRInt32 aWhen, nsIFile *aComponent, PRBool *_retval) { return _to AutoRegisterComponent(aWhen, aComponent, _retval); } \
  NS_IMETHOD AutoUnregisterComponent(PRInt32 aWhen, nsIFile *aComponent, PRBool *_retval) { return _to AutoUnregisterComponent(aWhen, aComponent, _retval); } \
  NS_IMETHOD RegisterDeferredComponents(PRInt32 aWhen, PRBool *_retval) { return _to RegisterDeferredComponents(aWhen, _retval); } \
  NS_IMETHOD UnloadAll(PRInt32 aWhen) { return _to UnloadAll(aWhen); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICOMPONENTLOADER(_to) \
  NS_IMETHOD GetFactory(const nsIID & aCID, const char *aLocation, const char *aType, nsIFactory **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFactory(aCID, aLocation, aType, _retval); } \
  NS_IMETHOD Init(nsIComponentManager *aCompMgr, nsISupports *aRegistry) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(aCompMgr, aRegistry); } \
  NS_IMETHOD OnRegister(const nsIID & aCID, const char *aType, const char *aClassName, const char *aContractID, const char *aLocation, PRBool aReplace, PRBool aPersist) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnRegister(aCID, aType, aClassName, aContractID, aLocation, aReplace, aPersist); } \
  NS_IMETHOD AutoRegisterComponents(PRInt32 aWhen, nsIFile *aDirectory) { return !_to ? NS_ERROR_NULL_POINTER : _to->AutoRegisterComponents(aWhen, aDirectory); } \
  NS_IMETHOD AutoRegisterComponent(PRInt32 aWhen, nsIFile *aComponent, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->AutoRegisterComponent(aWhen, aComponent, _retval); } \
  NS_IMETHOD AutoUnregisterComponent(PRInt32 aWhen, nsIFile *aComponent, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->AutoUnregisterComponent(aWhen, aComponent, _retval); } \
  NS_IMETHOD RegisterDeferredComponents(PRInt32 aWhen, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->RegisterDeferredComponents(aWhen, _retval); } \
  NS_IMETHOD UnloadAll(PRInt32 aWhen) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnloadAll(aWhen); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsComponentLoader : public nsIComponentLoader
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICOMPONENTLOADER

  nsComponentLoader();

private:
  ~nsComponentLoader();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsComponentLoader, nsIComponentLoader)

nsComponentLoader::nsComponentLoader()
{
  /* member initializers and constructor code */
}

nsComponentLoader::~nsComponentLoader()
{
  /* destructor code */
}

/* nsIFactory getFactory (in nsIIDRef aCID, in string aLocation, in string aType); */
NS_IMETHODIMP nsComponentLoader::GetFactory(const nsIID & aCID, const char *aLocation, const char *aType, nsIFactory **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void init (in nsIComponentManager aCompMgr, in nsISupports aRegistry); */
NS_IMETHODIMP nsComponentLoader::Init(nsIComponentManager *aCompMgr, nsISupports *aRegistry)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onRegister (in nsIIDRef aCID, in string aType, in string aClassName, in string aContractID, in string aLocation, in boolean aReplace, in boolean aPersist); */
NS_IMETHODIMP nsComponentLoader::OnRegister(const nsIID & aCID, const char *aType, const char *aClassName, const char *aContractID, const char *aLocation, PRBool aReplace, PRBool aPersist)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void autoRegisterComponents (in long aWhen, in nsIFile aDirectory); */
NS_IMETHODIMP nsComponentLoader::AutoRegisterComponents(PRInt32 aWhen, nsIFile *aDirectory)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean autoRegisterComponent (in long aWhen, in nsIFile aComponent); */
NS_IMETHODIMP nsComponentLoader::AutoRegisterComponent(PRInt32 aWhen, nsIFile *aComponent, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean autoUnregisterComponent (in long aWhen, in nsIFile aComponent); */
NS_IMETHODIMP nsComponentLoader::AutoUnregisterComponent(PRInt32 aWhen, nsIFile *aComponent, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean registerDeferredComponents (in long aWhen); */
NS_IMETHODIMP nsComponentLoader::RegisterDeferredComponents(PRInt32 aWhen, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unloadAll (in long aWhen); */
NS_IMETHODIMP nsComponentLoader::UnloadAll(PRInt32 aWhen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIComponentLoader_h__ */
