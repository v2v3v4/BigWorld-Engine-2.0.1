/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsIPermissionManager.idl
 */

#ifndef __gen_nsIPermissionManager_h__
#define __gen_nsIPermissionManager_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsISimpleEnumerator_h__
#include "nsISimpleEnumerator.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIURI; /* forward declaration */

class nsIObserver; /* forward declaration */


/* starting interface:    nsIPermissionManager */
#define NS_IPERMISSIONMANAGER_IID_STR "4f6b5e00-0c36-11d5-a535-0010a401eb10"

#define NS_IPERMISSIONMANAGER_IID \
  {0x4f6b5e00, 0x0c36, 0x11d5, \
    { 0xa5, 0x35, 0x00, 0x10, 0xa4, 0x01, 0xeb, 0x10 }}

class NS_NO_VTABLE nsIPermissionManager : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPERMISSIONMANAGER_IID)

  /**
   * Predefined return values for the testPermission method and for
   * the permission param of the add method
   */
  enum { UNKNOWN_ACTION = 0U };

  enum { ALLOW_ACTION = 1U };

  enum { DENY_ACTION = 2U };

  /**
   * Add permission information for a given URI and permission type. This
   * operation will cause the type string to be registered if it does not
   * currently exist.
   *
   * @param uri         the uri to add the permission for
   * @param type        a case-sensitive ASCII string, identifying the consumer.
   *                    Consumers should choose this string to be unique, with
   *                    respect to other consumers. The number of unique type
   *                    indentifiers may be limited.
   * @param permission  an integer from 1 to 15, representing the desired
   *                    action (e.g. allow or deny). The interpretation of
   *                    this number is up to the consumer, and may represent
   *                    different actions for different types. Consumers may
   *                    use one of the enumerated permission actions defined
   *                    above. 0 is reserved for UNKNOWN_ACTION, and shouldn't
   *                    be used.
   * @throws            NS_ERROR_FAILURE if there is no more room for adding
   *                    a new type
   */
  /* void add (in nsIURI uri, in string type, in PRUint32 permission); */
  NS_IMETHOD Add(nsIURI *uri, const char *type, PRUint32 permission) = 0;

  /**
   * Remove permission information for a given URI and permission type.
   * Note that this method takes a host string, not an nsIURI.
   *
   * @param host   the host to remove the permission for
   * @param type   a case-sensitive ASCII string, identifying the consumer. 
   *               The type must have been previously registered using the
   *               add() method.
   */
  /* void remove (in AUTF8String host, in string type); */
  NS_IMETHOD Remove(const nsACString & host, const char *type) = 0;

  /**
   * Clear permission information for all websites.
   */
  /* void removeAll (); */
  NS_IMETHOD RemoveAll(void) = 0;

  /**
   * Test whether a website has permission to perform the given action.
   * @param uri     the uri to be tested
   * @param type    a case-sensitive ASCII string, identifying the consumer
   * @param return  see add(), param permission. returns UNKNOWN_ACTION when
   *                there is no stored permission for this uri and / or type.
   */
  /* PRUint32 testPermission (in nsIURI uri, in string type); */
  NS_IMETHOD TestPermission(nsIURI *uri, const char *type, PRUint32 *_retval) = 0;

  /**
   * Allows enumeration of all stored permissions
   * @return an nsISimpleEnumerator interface that allows access to
   *         nsIPermission objects
   */
  /* readonly attribute nsISimpleEnumerator enumerator; */
  NS_IMETHOD GetEnumerator(nsISimpleEnumerator * *aEnumerator) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPERMISSIONMANAGER \
  NS_IMETHOD Add(nsIURI *uri, const char *type, PRUint32 permission); \
  NS_IMETHOD Remove(const nsACString & host, const char *type); \
  NS_IMETHOD RemoveAll(void); \
  NS_IMETHOD TestPermission(nsIURI *uri, const char *type, PRUint32 *_retval); \
  NS_IMETHOD GetEnumerator(nsISimpleEnumerator * *aEnumerator); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPERMISSIONMANAGER(_to) \
  NS_IMETHOD Add(nsIURI *uri, const char *type, PRUint32 permission) { return _to Add(uri, type, permission); } \
  NS_IMETHOD Remove(const nsACString & host, const char *type) { return _to Remove(host, type); } \
  NS_IMETHOD RemoveAll(void) { return _to RemoveAll(); } \
  NS_IMETHOD TestPermission(nsIURI *uri, const char *type, PRUint32 *_retval) { return _to TestPermission(uri, type, _retval); } \
  NS_IMETHOD GetEnumerator(nsISimpleEnumerator * *aEnumerator) { return _to GetEnumerator(aEnumerator); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPERMISSIONMANAGER(_to) \
  NS_IMETHOD Add(nsIURI *uri, const char *type, PRUint32 permission) { return !_to ? NS_ERROR_NULL_POINTER : _to->Add(uri, type, permission); } \
  NS_IMETHOD Remove(const nsACString & host, const char *type) { return !_to ? NS_ERROR_NULL_POINTER : _to->Remove(host, type); } \
  NS_IMETHOD RemoveAll(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveAll(); } \
  NS_IMETHOD TestPermission(nsIURI *uri, const char *type, PRUint32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->TestPermission(uri, type, _retval); } \
  NS_IMETHOD GetEnumerator(nsISimpleEnumerator * *aEnumerator) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetEnumerator(aEnumerator); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsPermissionManager : public nsIPermissionManager
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPERMISSIONMANAGER

  nsPermissionManager();

private:
  ~nsPermissionManager();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsPermissionManager, nsIPermissionManager)

nsPermissionManager::nsPermissionManager()
{
  /* member initializers and constructor code */
}

nsPermissionManager::~nsPermissionManager()
{
  /* destructor code */
}

/* void add (in nsIURI uri, in string type, in PRUint32 permission); */
NS_IMETHODIMP nsPermissionManager::Add(nsIURI *uri, const char *type, PRUint32 permission)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void remove (in AUTF8String host, in string type); */
NS_IMETHODIMP nsPermissionManager::Remove(const nsACString & host, const char *type)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeAll (); */
NS_IMETHODIMP nsPermissionManager::RemoveAll()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* PRUint32 testPermission (in nsIURI uri, in string type); */
NS_IMETHODIMP nsPermissionManager::TestPermission(nsIURI *uri, const char *type, PRUint32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsISimpleEnumerator enumerator; */
NS_IMETHODIMP nsPermissionManager::GetEnumerator(nsISimpleEnumerator * *aEnumerator)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

#define NS_PERMISSIONMANAGER_CONTRACTID "@mozilla.org/permissionmanager;1"
#define PERM_CHANGE_NOTIFICATION "perm-changed"

#endif /* __gen_nsIPermissionManager_h__ */
