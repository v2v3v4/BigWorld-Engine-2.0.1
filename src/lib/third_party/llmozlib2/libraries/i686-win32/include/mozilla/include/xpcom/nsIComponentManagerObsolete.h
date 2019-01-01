/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/components/nsIComponentManagerObsolete.idl
 */

#ifndef __gen_nsIComponentManagerObsolete_h__
#define __gen_nsIComponentManagerObsolete_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsIFactory_h__
#include "nsIFactory.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIFile; /* forward declaration */

class nsIEnumerator; /* forward declaration */


/* starting interface:    nsIComponentManagerObsolete */
#define NS_ICOMPONENTMANAGEROBSOLETE_IID_STR "8458a740-d5dc-11d2-92fb-00e09805570f"

#define NS_ICOMPONENTMANAGEROBSOLETE_IID \
  {0x8458a740, 0xd5dc, 0x11d2, \
    { 0x92, 0xfb, 0x00, 0xe0, 0x98, 0x05, 0x57, 0x0f }}

class NS_NO_VTABLE nsIComponentManagerObsolete : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICOMPONENTMANAGEROBSOLETE_IID)

  /**
     * findFactory
     *
     * Returns the factory object that can be used to create instances of
     * CID aClass
     *
     * @param aClass The classid of the factory that is being requested
     */
  /* nsIFactory findFactory (in nsCIDRef aClass); */
  NS_IMETHOD FindFactory(const nsCID & aClass, nsIFactory **_retval) = 0;

  /**
     * getClassObject
     *
     * @param aClass : CID of the class whose class object is requested
     * @param aIID : IID of an interface that the class object is known to
     *               to implement. nsISupports and nsIFactory are known to
     *               be implemented by the class object.
     */
  /* [noscript] voidPtr getClassObject (in nsCIDRef aClass, in nsIIDRef aIID); */
  NS_IMETHOD GetClassObject(const nsCID & aClass, const nsIID & aIID, void * *_retval) = 0;

  /**
     * contractIDToClassID
     *
     * Get the ClassID for a given ContractID. Many ClassIDs may implement a
     * ContractID. In such a situation, this returns the preferred ClassID, which
     * happens to be the last registered ClassID.
     * 
     * @param aContractID : Contractid for which ClassID is requested
     * @return aClass : ClassID return
     */
  /* [notxpcom] nsresult contractIDToClassID (in string aContractID, out nsCID aClass); */
  NS_IMETHOD_(nsresult) ContractIDToClassID(const char *aContractID, nsCID *aClass) = 0;

  /**
     * classIDToContractid
     *
     * Get the ContractID for a given ClassID. A ClassIDs may implement multiple
     * ContractIDs. This function return the last registered ContractID.
     *
     * @param aClass : ClassID for which ContractID is requested.
     * @return aClassName : returns class name asssociated with aClass
     * @return : ContractID last registered for aClass
     */
  /* string CLSIDToContractID (in nsCIDRef aClass, out string aClassName); */
  NS_IMETHOD CLSIDToContractID(const nsCID & aClass, char **aClassName, char **_retval) = 0;

  /**
     * createInstance
     *
     * Create an instance of the CID aClass and return the interface aIID.
     *
     * @param aClass : ClassID of object instance requested
     * @param aDelegate : Used for aggregation
     * @param aIID : IID of interface requested
     */
  /* [noscript] voidPtr createInstance (in nsCIDRef aClass, in nsISupports aDelegate, in nsIIDRef aIID); */
  NS_IMETHOD CreateInstance(const nsCID & aClass, nsISupports *aDelegate, const nsIID & aIID, void * *_retval) = 0;

  /**
     * createInstanceByContractID
     *
     * Create an instance of the CID that implements aContractID and return the
     * interface aIID. This is a convenience function that effectively does
     * ContractIDToClassID() followed by CreateInstance().
     *
     * @param aContractID : aContractID of object instance requested
     * @param aDelegate : Used for aggregation
     * @param aIID : IID of interface requested
     */
  /* [noscript] voidPtr createInstanceByContractID (in string aContractID, in nsISupports aDelegate, in nsIIDRef IID); */
  NS_IMETHOD CreateInstanceByContractID(const char *aContractID, nsISupports *aDelegate, const nsIID & IID, void * *_retval) = 0;

  /**
     * registryLocationForSpec
     *
     * Given a file specification, return the registry representation of
     * the filename. Files that are found relative to the components
     * directory will have a registry representation
     * "rel:<relative-native-path>" while filenames that are not, will have
     * "abs:<full-native-path>".
     */
  /* string registryLocationForSpec (in nsIFile aSpec); */
  NS_IMETHOD RegistryLocationForSpec(nsIFile *aSpec, char **_retval) = 0;

  /**
     * specForRegistyLocation
     *
     * Create a file specification for the registry representation (rel:/abs:)
     * got via registryLocationForSpec.
     */
  /* nsIFile specForRegistryLocation (in string aLocation); */
  NS_IMETHOD SpecForRegistryLocation(const char *aLocation, nsIFile **_retval) = 0;

  /**
     * registerFactory
     *
     * Register a factory and ContractID associated with CID aClass
     *
     * @param aClass : CID of object
     * @param aClassName : Class Name of CID
     * @param aContractID : ContractID associated with CID aClass
     * @param aFactory : Factory that will be registered for CID aClass
     * @param aReplace : Boolean that indicates whether to replace a previous
     *                   registration for the CID aClass.
     */
  /* void registerFactory (in nsCIDRef aClass, in string aClassName, in string aContractID, in nsIFactory aFactory, in boolean aReplace); */
  NS_IMETHOD RegisterFactory(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFactory *aFactory, PRBool aReplace) = 0;

  /**
     * registerComponent
     *
     * Register a native dll module via its registry representation as returned
     * by registryLocationForSpec() as the container of CID implemenation
     * aClass and associate aContractID and aClassName to the CID aClass. Native
     * dll component type is assumed.
     *
     * @param aClass : CID implemenation contained in module
     * @param aClassName : Class name associated with CID aClass
     * @param aContractID : ContractID associated with CID aClass
     * @param aLocation : Location of module (dll). Format of this is the
     *                    registry representation as returned by
     *                    registryLocationForSpec()
     * @param aReplace : Boolean that indicates whether to replace a previous
     *                   module registration for aClass.
     * @param aPersist : Remember this registration across sessions.
     */
  /* void registerComponent (in nsCIDRef aClass, in string aClassName, in string aContractID, in string aLocation, in boolean aReplace, in boolean aPersist); */
  NS_IMETHOD RegisterComponent(const nsCID & aClass, const char *aClassName, const char *aContractID, const char *aLocation, PRBool aReplace, PRBool aPersist) = 0;

  /**
     * registerComponentWithType
     *
     * Register a module's location via its registry representation
     * as returned by registryLocationForSpec() as the container of CID implemenation
     * aClass of type aType and associate aContractID and aClassName to the CID aClass.
     *
     * @param aClass : CID implemenation contained in module
     * @param aClassName : Class name associated with CID aClass
     * @param aContractID : ContractID associated with CID aClass
     * @param aSpec : Filename spec for module's location.
     * @param aLocation : Location of module of type aType. Format of this string
     *                    is the registry representation as returned by
     *                    registryLocationForSpec()
     * @param aReplace : Boolean that indicates whether to replace a previous
     *                   loader registration for aClass.
     * @param aPersist : Remember this registration across sessions.
     * @param aType : Component Type of CID aClass.
     */
  /* void registerComponentWithType (in nsCIDRef aClass, in string aClassName, in string aContractID, in nsIFile aSpec, in string aLocation, in boolean aReplace, in boolean aPersist, in string aType); */
  NS_IMETHOD RegisterComponentWithType(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFile *aSpec, const char *aLocation, PRBool aReplace, PRBool aPersist, const char *aType) = 0;

  /**
     * registerComponentSpec
     *
     * Register a native dll module via its file specification as the container
     * of CID implemenation aClass and associate aContractID and aClassName to the
     * CID aClass. Native dll component type is assumed.
     *
     * @param aClass : CID implemenation contained in module
     * @param aClassName : Class name associated with CID aClass
     * @param aContractID : ContractID associated with CID aClass
     * @param aLibrary : File specification Location of module (dll).
     * @param aReplace : Boolean that indicates whether to replace a previous
     *                   module registration for aClass.
     * @param aPersist : Remember this registration across sessions.
     */
  /* void registerComponentSpec (in nsCIDRef aClass, in string aClassName, in string aContractID, in nsIFile aLibrary, in boolean aReplace, in boolean aPersist); */
  NS_IMETHOD RegisterComponentSpec(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFile *aLibrary, PRBool aReplace, PRBool aPersist) = 0;

  /**
     * registerComponentLib
     *
     * Register a native dll module via its dll name (not full path) as the
     * container of CID implemenation aClass and associate aContractID and aClassName
     * to the CID aClass. Native dll component type is assumed and the system
     * services will be used to load this dll.
     *
     * @param aClass : CID implemenation contained in module
     * @param aClassName : Class name associated with CID aClass
     * @param aContractID : ContractID associated with CID aClass
     * @param aDllNameLocation : Dll name of module.
     * @param aReplace : Boolean that indicates whether to replace a previous
     *                   module registration for aClass.
     * @param aPersist : Remember this registration across sessions.
     */
  /* void registerComponentLib (in nsCIDRef aClass, in string aClassName, in string aContractID, in string aDllName, in boolean aReplace, in boolean aPersist); */
  NS_IMETHOD RegisterComponentLib(const nsCID & aClass, const char *aClassName, const char *aContractID, const char *aDllName, PRBool aReplace, PRBool aPersist) = 0;

  /**
     * unregisterFactory
     *
     * Unregister a factory associated with CID aClass.
     *
     * @param aClass : ClassID being unregistered
     * @param aFactory : Factory previously registered to create instances of
     *                   ClassID aClass.
     */
  /* void unregisterFactory (in nsCIDRef aClass, in nsIFactory aFactory); */
  NS_IMETHOD UnregisterFactory(const nsCID & aClass, nsIFactory *aFactory) = 0;

  /**
     * unregisterComponent
     *
     * Disassociate module aLocation represented as registry location as returned
     * by registryLocationForSpec() as containing ClassID aClass.
     *
     * @param aClass : ClassID being unregistered
     * @param aLocation : Location of module. Format of this is the registry
     *                    representation as returned by registryLocationForSpec().
     *                    Components of any type will be unregistered.
     */
  /* void unregisterComponent (in nsCIDRef aClass, in string aLocation); */
  NS_IMETHOD UnregisterComponent(const nsCID & aClass, const char *aLocation) = 0;

  /**
     * unregisterComponentSpec
     *
     * Disassociate module references by file specification aLibrarySpec as
     * containing ClassID aClass.
     */
  /* void unregisterComponentSpec (in nsCIDRef aClass, in nsIFile aLibrarySpec); */
  NS_IMETHOD UnregisterComponentSpec(const nsCID & aClass, nsIFile *aLibrarySpec) = 0;

  /**
     * freeLibraries
     *
     * Enumerates all loaded modules and unloads unused modules.
     */
  /* void freeLibraries (); */
  NS_IMETHOD FreeLibraries(void) = 0;

  /**
     * ID values for 'when'
     */
  enum { NS_Startup = 0 };

  enum { NS_Script = 1 };

  enum { NS_Timer = 2 };

  enum { NS_Shutdown = 3 };

  /**
     * autoRegister
     *
     * Enumerates directory looking for modules of all types and registers
     * modules who have changed (modtime or size) since the last time
     * autoRegister() was invoked.
     *
     * @param when : ID values of when the call is being made.
     * @param directory : Directory the will be enumerated.
     */
  /* void autoRegister (in long when, in nsIFile directory); */
  NS_IMETHOD AutoRegister(PRInt32 when, nsIFile *directory) = 0;

  /**
     * autoRegisterComponent
     *
     * Loads module using appropriate loader and gives it an opportunity to
     * register its CIDs if module's modtime or size changed since the last
     * time this was called.
     *
     * @param when : ID values of when the call is being made.
     * @param aFileLocation : File specification of module.
     */
  /* void autoRegisterComponent (in long when, in nsIFile aFileLocation); */
  NS_IMETHOD AutoRegisterComponent(PRInt32 when, nsIFile *aFileLocation) = 0;

  /**
     * autoUnregisterComponent
     *
     * Loads module using approriate loader and gives it an opportunity to
     * unregister its CIDs
     */
  /* void autoUnregisterComponent (in long when, in nsIFile aFileLocation); */
  NS_IMETHOD AutoUnregisterComponent(PRInt32 when, nsIFile *aFileLocation) = 0;

  /**
     * isRegistered
     *
     * Returns true if a factory or module is registered for CID aClass.
     *
     * @param aClass : ClassID queried for registeration
     * @return : true if a factory or module is registered for CID aClass.
     *           false otherwise.
     */
  /* boolean isRegistered (in nsCIDRef aClass); */
  NS_IMETHOD IsRegistered(const nsCID & aClass, PRBool *_retval) = 0;

  /**
     * enumerateCLSIDs
     *
     * Enumerate the list of all registered ClassIDs.
     *
     * @return : enumerator for ClassIDs.
     */
  /* nsIEnumerator enumerateCLSIDs (); */
  NS_IMETHOD EnumerateCLSIDs(nsIEnumerator **_retval) = 0;

  /**
     * enumerateContractIDs
     *
     * Enumerate the list of all registered ContractIDs.
     *
     * @return : enumerator for ContractIDs.
     */
  /* nsIEnumerator enumerateContractIDs (); */
  NS_IMETHOD EnumerateContractIDs(nsIEnumerator **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICOMPONENTMANAGEROBSOLETE \
  NS_IMETHOD FindFactory(const nsCID & aClass, nsIFactory **_retval); \
  NS_IMETHOD GetClassObject(const nsCID & aClass, const nsIID & aIID, void * *_retval); \
  NS_IMETHOD_(nsresult) ContractIDToClassID(const char *aContractID, nsCID *aClass); \
  NS_IMETHOD CLSIDToContractID(const nsCID & aClass, char **aClassName, char **_retval); \
  NS_IMETHOD CreateInstance(const nsCID & aClass, nsISupports *aDelegate, const nsIID & aIID, void * *_retval); \
  NS_IMETHOD CreateInstanceByContractID(const char *aContractID, nsISupports *aDelegate, const nsIID & IID, void * *_retval); \
  NS_IMETHOD RegistryLocationForSpec(nsIFile *aSpec, char **_retval); \
  NS_IMETHOD SpecForRegistryLocation(const char *aLocation, nsIFile **_retval); \
  NS_IMETHOD RegisterFactory(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFactory *aFactory, PRBool aReplace); \
  NS_IMETHOD RegisterComponent(const nsCID & aClass, const char *aClassName, const char *aContractID, const char *aLocation, PRBool aReplace, PRBool aPersist); \
  NS_IMETHOD RegisterComponentWithType(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFile *aSpec, const char *aLocation, PRBool aReplace, PRBool aPersist, const char *aType); \
  NS_IMETHOD RegisterComponentSpec(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFile *aLibrary, PRBool aReplace, PRBool aPersist); \
  NS_IMETHOD RegisterComponentLib(const nsCID & aClass, const char *aClassName, const char *aContractID, const char *aDllName, PRBool aReplace, PRBool aPersist); \
  NS_IMETHOD UnregisterFactory(const nsCID & aClass, nsIFactory *aFactory); \
  NS_IMETHOD UnregisterComponent(const nsCID & aClass, const char *aLocation); \
  NS_IMETHOD UnregisterComponentSpec(const nsCID & aClass, nsIFile *aLibrarySpec); \
  NS_IMETHOD FreeLibraries(void); \
  NS_IMETHOD AutoRegister(PRInt32 when, nsIFile *directory); \
  NS_IMETHOD AutoRegisterComponent(PRInt32 when, nsIFile *aFileLocation); \
  NS_IMETHOD AutoUnregisterComponent(PRInt32 when, nsIFile *aFileLocation); \
  NS_IMETHOD IsRegistered(const nsCID & aClass, PRBool *_retval); \
  NS_IMETHOD EnumerateCLSIDs(nsIEnumerator **_retval); \
  NS_IMETHOD EnumerateContractIDs(nsIEnumerator **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICOMPONENTMANAGEROBSOLETE(_to) \
  NS_IMETHOD FindFactory(const nsCID & aClass, nsIFactory **_retval) { return _to FindFactory(aClass, _retval); } \
  NS_IMETHOD GetClassObject(const nsCID & aClass, const nsIID & aIID, void * *_retval) { return _to GetClassObject(aClass, aIID, _retval); } \
  NS_IMETHOD_(nsresult) ContractIDToClassID(const char *aContractID, nsCID *aClass) { return _to ContractIDToClassID(aContractID, aClass); } \
  NS_IMETHOD CLSIDToContractID(const nsCID & aClass, char **aClassName, char **_retval) { return _to CLSIDToContractID(aClass, aClassName, _retval); } \
  NS_IMETHOD CreateInstance(const nsCID & aClass, nsISupports *aDelegate, const nsIID & aIID, void * *_retval) { return _to CreateInstance(aClass, aDelegate, aIID, _retval); } \
  NS_IMETHOD CreateInstanceByContractID(const char *aContractID, nsISupports *aDelegate, const nsIID & IID, void * *_retval) { return _to CreateInstanceByContractID(aContractID, aDelegate, IID, _retval); } \
  NS_IMETHOD RegistryLocationForSpec(nsIFile *aSpec, char **_retval) { return _to RegistryLocationForSpec(aSpec, _retval); } \
  NS_IMETHOD SpecForRegistryLocation(const char *aLocation, nsIFile **_retval) { return _to SpecForRegistryLocation(aLocation, _retval); } \
  NS_IMETHOD RegisterFactory(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFactory *aFactory, PRBool aReplace) { return _to RegisterFactory(aClass, aClassName, aContractID, aFactory, aReplace); } \
  NS_IMETHOD RegisterComponent(const nsCID & aClass, const char *aClassName, const char *aContractID, const char *aLocation, PRBool aReplace, PRBool aPersist) { return _to RegisterComponent(aClass, aClassName, aContractID, aLocation, aReplace, aPersist); } \
  NS_IMETHOD RegisterComponentWithType(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFile *aSpec, const char *aLocation, PRBool aReplace, PRBool aPersist, const char *aType) { return _to RegisterComponentWithType(aClass, aClassName, aContractID, aSpec, aLocation, aReplace, aPersist, aType); } \
  NS_IMETHOD RegisterComponentSpec(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFile *aLibrary, PRBool aReplace, PRBool aPersist) { return _to RegisterComponentSpec(aClass, aClassName, aContractID, aLibrary, aReplace, aPersist); } \
  NS_IMETHOD RegisterComponentLib(const nsCID & aClass, const char *aClassName, const char *aContractID, const char *aDllName, PRBool aReplace, PRBool aPersist) { return _to RegisterComponentLib(aClass, aClassName, aContractID, aDllName, aReplace, aPersist); } \
  NS_IMETHOD UnregisterFactory(const nsCID & aClass, nsIFactory *aFactory) { return _to UnregisterFactory(aClass, aFactory); } \
  NS_IMETHOD UnregisterComponent(const nsCID & aClass, const char *aLocation) { return _to UnregisterComponent(aClass, aLocation); } \
  NS_IMETHOD UnregisterComponentSpec(const nsCID & aClass, nsIFile *aLibrarySpec) { return _to UnregisterComponentSpec(aClass, aLibrarySpec); } \
  NS_IMETHOD FreeLibraries(void) { return _to FreeLibraries(); } \
  NS_IMETHOD AutoRegister(PRInt32 when, nsIFile *directory) { return _to AutoRegister(when, directory); } \
  NS_IMETHOD AutoRegisterComponent(PRInt32 when, nsIFile *aFileLocation) { return _to AutoRegisterComponent(when, aFileLocation); } \
  NS_IMETHOD AutoUnregisterComponent(PRInt32 when, nsIFile *aFileLocation) { return _to AutoUnregisterComponent(when, aFileLocation); } \
  NS_IMETHOD IsRegistered(const nsCID & aClass, PRBool *_retval) { return _to IsRegistered(aClass, _retval); } \
  NS_IMETHOD EnumerateCLSIDs(nsIEnumerator **_retval) { return _to EnumerateCLSIDs(_retval); } \
  NS_IMETHOD EnumerateContractIDs(nsIEnumerator **_retval) { return _to EnumerateContractIDs(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICOMPONENTMANAGEROBSOLETE(_to) \
  NS_IMETHOD FindFactory(const nsCID & aClass, nsIFactory **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->FindFactory(aClass, _retval); } \
  NS_IMETHOD GetClassObject(const nsCID & aClass, const nsIID & aIID, void * *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetClassObject(aClass, aIID, _retval); } \
  NS_IMETHOD_(nsresult) ContractIDToClassID(const char *aContractID, nsCID *aClass) { return !_to ? NS_ERROR_NULL_POINTER : _to->ContractIDToClassID(aContractID, aClass); } \
  NS_IMETHOD CLSIDToContractID(const nsCID & aClass, char **aClassName, char **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CLSIDToContractID(aClass, aClassName, _retval); } \
  NS_IMETHOD CreateInstance(const nsCID & aClass, nsISupports *aDelegate, const nsIID & aIID, void * *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateInstance(aClass, aDelegate, aIID, _retval); } \
  NS_IMETHOD CreateInstanceByContractID(const char *aContractID, nsISupports *aDelegate, const nsIID & IID, void * *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateInstanceByContractID(aContractID, aDelegate, IID, _retval); } \
  NS_IMETHOD RegistryLocationForSpec(nsIFile *aSpec, char **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->RegistryLocationForSpec(aSpec, _retval); } \
  NS_IMETHOD SpecForRegistryLocation(const char *aLocation, nsIFile **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SpecForRegistryLocation(aLocation, _retval); } \
  NS_IMETHOD RegisterFactory(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFactory *aFactory, PRBool aReplace) { return !_to ? NS_ERROR_NULL_POINTER : _to->RegisterFactory(aClass, aClassName, aContractID, aFactory, aReplace); } \
  NS_IMETHOD RegisterComponent(const nsCID & aClass, const char *aClassName, const char *aContractID, const char *aLocation, PRBool aReplace, PRBool aPersist) { return !_to ? NS_ERROR_NULL_POINTER : _to->RegisterComponent(aClass, aClassName, aContractID, aLocation, aReplace, aPersist); } \
  NS_IMETHOD RegisterComponentWithType(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFile *aSpec, const char *aLocation, PRBool aReplace, PRBool aPersist, const char *aType) { return !_to ? NS_ERROR_NULL_POINTER : _to->RegisterComponentWithType(aClass, aClassName, aContractID, aSpec, aLocation, aReplace, aPersist, aType); } \
  NS_IMETHOD RegisterComponentSpec(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFile *aLibrary, PRBool aReplace, PRBool aPersist) { return !_to ? NS_ERROR_NULL_POINTER : _to->RegisterComponentSpec(aClass, aClassName, aContractID, aLibrary, aReplace, aPersist); } \
  NS_IMETHOD RegisterComponentLib(const nsCID & aClass, const char *aClassName, const char *aContractID, const char *aDllName, PRBool aReplace, PRBool aPersist) { return !_to ? NS_ERROR_NULL_POINTER : _to->RegisterComponentLib(aClass, aClassName, aContractID, aDllName, aReplace, aPersist); } \
  NS_IMETHOD UnregisterFactory(const nsCID & aClass, nsIFactory *aFactory) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnregisterFactory(aClass, aFactory); } \
  NS_IMETHOD UnregisterComponent(const nsCID & aClass, const char *aLocation) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnregisterComponent(aClass, aLocation); } \
  NS_IMETHOD UnregisterComponentSpec(const nsCID & aClass, nsIFile *aLibrarySpec) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnregisterComponentSpec(aClass, aLibrarySpec); } \
  NS_IMETHOD FreeLibraries(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->FreeLibraries(); } \
  NS_IMETHOD AutoRegister(PRInt32 when, nsIFile *directory) { return !_to ? NS_ERROR_NULL_POINTER : _to->AutoRegister(when, directory); } \
  NS_IMETHOD AutoRegisterComponent(PRInt32 when, nsIFile *aFileLocation) { return !_to ? NS_ERROR_NULL_POINTER : _to->AutoRegisterComponent(when, aFileLocation); } \
  NS_IMETHOD AutoUnregisterComponent(PRInt32 when, nsIFile *aFileLocation) { return !_to ? NS_ERROR_NULL_POINTER : _to->AutoUnregisterComponent(when, aFileLocation); } \
  NS_IMETHOD IsRegistered(const nsCID & aClass, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->IsRegistered(aClass, _retval); } \
  NS_IMETHOD EnumerateCLSIDs(nsIEnumerator **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->EnumerateCLSIDs(_retval); } \
  NS_IMETHOD EnumerateContractIDs(nsIEnumerator **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->EnumerateContractIDs(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsComponentManagerObsolete : public nsIComponentManagerObsolete
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICOMPONENTMANAGEROBSOLETE

  nsComponentManagerObsolete();

private:
  ~nsComponentManagerObsolete();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsComponentManagerObsolete, nsIComponentManagerObsolete)

nsComponentManagerObsolete::nsComponentManagerObsolete()
{
  /* member initializers and constructor code */
}

nsComponentManagerObsolete::~nsComponentManagerObsolete()
{
  /* destructor code */
}

/* nsIFactory findFactory (in nsCIDRef aClass); */
NS_IMETHODIMP nsComponentManagerObsolete::FindFactory(const nsCID & aClass, nsIFactory **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] voidPtr getClassObject (in nsCIDRef aClass, in nsIIDRef aIID); */
NS_IMETHODIMP nsComponentManagerObsolete::GetClassObject(const nsCID & aClass, const nsIID & aIID, void * *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [notxpcom] nsresult contractIDToClassID (in string aContractID, out nsCID aClass); */
NS_IMETHODIMP_(nsresult) nsComponentManagerObsolete::ContractIDToClassID(const char *aContractID, nsCID *aClass)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* string CLSIDToContractID (in nsCIDRef aClass, out string aClassName); */
NS_IMETHODIMP nsComponentManagerObsolete::CLSIDToContractID(const nsCID & aClass, char **aClassName, char **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] voidPtr createInstance (in nsCIDRef aClass, in nsISupports aDelegate, in nsIIDRef aIID); */
NS_IMETHODIMP nsComponentManagerObsolete::CreateInstance(const nsCID & aClass, nsISupports *aDelegate, const nsIID & aIID, void * *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] voidPtr createInstanceByContractID (in string aContractID, in nsISupports aDelegate, in nsIIDRef IID); */
NS_IMETHODIMP nsComponentManagerObsolete::CreateInstanceByContractID(const char *aContractID, nsISupports *aDelegate, const nsIID & IID, void * *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* string registryLocationForSpec (in nsIFile aSpec); */
NS_IMETHODIMP nsComponentManagerObsolete::RegistryLocationForSpec(nsIFile *aSpec, char **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIFile specForRegistryLocation (in string aLocation); */
NS_IMETHODIMP nsComponentManagerObsolete::SpecForRegistryLocation(const char *aLocation, nsIFile **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void registerFactory (in nsCIDRef aClass, in string aClassName, in string aContractID, in nsIFactory aFactory, in boolean aReplace); */
NS_IMETHODIMP nsComponentManagerObsolete::RegisterFactory(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFactory *aFactory, PRBool aReplace)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void registerComponent (in nsCIDRef aClass, in string aClassName, in string aContractID, in string aLocation, in boolean aReplace, in boolean aPersist); */
NS_IMETHODIMP nsComponentManagerObsolete::RegisterComponent(const nsCID & aClass, const char *aClassName, const char *aContractID, const char *aLocation, PRBool aReplace, PRBool aPersist)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void registerComponentWithType (in nsCIDRef aClass, in string aClassName, in string aContractID, in nsIFile aSpec, in string aLocation, in boolean aReplace, in boolean aPersist, in string aType); */
NS_IMETHODIMP nsComponentManagerObsolete::RegisterComponentWithType(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFile *aSpec, const char *aLocation, PRBool aReplace, PRBool aPersist, const char *aType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void registerComponentSpec (in nsCIDRef aClass, in string aClassName, in string aContractID, in nsIFile aLibrary, in boolean aReplace, in boolean aPersist); */
NS_IMETHODIMP nsComponentManagerObsolete::RegisterComponentSpec(const nsCID & aClass, const char *aClassName, const char *aContractID, nsIFile *aLibrary, PRBool aReplace, PRBool aPersist)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void registerComponentLib (in nsCIDRef aClass, in string aClassName, in string aContractID, in string aDllName, in boolean aReplace, in boolean aPersist); */
NS_IMETHODIMP nsComponentManagerObsolete::RegisterComponentLib(const nsCID & aClass, const char *aClassName, const char *aContractID, const char *aDllName, PRBool aReplace, PRBool aPersist)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unregisterFactory (in nsCIDRef aClass, in nsIFactory aFactory); */
NS_IMETHODIMP nsComponentManagerObsolete::UnregisterFactory(const nsCID & aClass, nsIFactory *aFactory)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unregisterComponent (in nsCIDRef aClass, in string aLocation); */
NS_IMETHODIMP nsComponentManagerObsolete::UnregisterComponent(const nsCID & aClass, const char *aLocation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unregisterComponentSpec (in nsCIDRef aClass, in nsIFile aLibrarySpec); */
NS_IMETHODIMP nsComponentManagerObsolete::UnregisterComponentSpec(const nsCID & aClass, nsIFile *aLibrarySpec)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void freeLibraries (); */
NS_IMETHODIMP nsComponentManagerObsolete::FreeLibraries()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void autoRegister (in long when, in nsIFile directory); */
NS_IMETHODIMP nsComponentManagerObsolete::AutoRegister(PRInt32 when, nsIFile *directory)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void autoRegisterComponent (in long when, in nsIFile aFileLocation); */
NS_IMETHODIMP nsComponentManagerObsolete::AutoRegisterComponent(PRInt32 when, nsIFile *aFileLocation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void autoUnregisterComponent (in long when, in nsIFile aFileLocation); */
NS_IMETHODIMP nsComponentManagerObsolete::AutoUnregisterComponent(PRInt32 when, nsIFile *aFileLocation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isRegistered (in nsCIDRef aClass); */
NS_IMETHODIMP nsComponentManagerObsolete::IsRegistered(const nsCID & aClass, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIEnumerator enumerateCLSIDs (); */
NS_IMETHODIMP nsComponentManagerObsolete::EnumerateCLSIDs(nsIEnumerator **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIEnumerator enumerateContractIDs (); */
NS_IMETHODIMP nsComponentManagerObsolete::EnumerateContractIDs(nsIEnumerator **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

/* include after the class def'n, because it needs to see it. */
#include "nsComponentManagerUtils.h"

#endif /* __gen_nsIComponentManagerObsolete_h__ */
