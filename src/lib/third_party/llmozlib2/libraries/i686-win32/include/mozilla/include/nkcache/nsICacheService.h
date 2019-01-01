/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/cache/public/nsICacheService.idl
 */

#ifndef __gen_nsICacheService_h__
#define __gen_nsICacheService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsICache_h__
#include "nsICache.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsISimpleEnumerator; /* forward declaration */

class nsICacheListener; /* forward declaration */

class nsICacheSession; /* forward declaration */

class nsICacheVisitor; /* forward declaration */


/* starting interface:    nsICacheService */
#define NS_ICACHESERVICE_IID_STR "de114eb4-29fc-4959-b2f7-2d03eb9bc771"

#define NS_ICACHESERVICE_IID \
  {0xde114eb4, 0x29fc, 0x4959, \
    { 0xb2, 0xf7, 0x2d, 0x03, 0xeb, 0x9b, 0xc7, 0x71 }}

class NS_NO_VTABLE nsICacheService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICACHESERVICE_IID)

  /**
     * Create a cache session
     *
     * A cache session represents a client's access into the cache.  The cache
     * session is not "owned" by the cache service.  Hence, it is possible to
     * create duplicate cache sessions.  Entries created by a cache session
     * are invisible to other cache sessions, unless the cache sessions are
     * equivalent.
     *
     * @param clientID - Specifies the name of the client using the cache.
     * @param storagePolicy - Limits the storage policy for all entries
     *   accessed via the returned session.  As a result, devices excluded
     *   by the storage policy will not be searched when opening entries
     *   from the returned session.
     * @param streamBased - Indicates whether or not the data being cached
     *   can be represented as a stream.  The storagePolicy must be 
     *   consistent with the value of this field.  For example, a non-stream-
     *   based cache entry can only have a storage policy of STORE_IN_MEMORY.
     * @return new cache session.
     */
  /* nsICacheSession createSession (in string clientID, in nsCacheStoragePolicy storagePolicy, in boolean streamBased); */
  NS_IMETHOD CreateSession(const char *clientID, nsCacheStoragePolicy storagePolicy, PRBool streamBased, nsICacheSession **_retval) = 0;

  /**
     * Visit entries stored in the cache.  Used to implement about:cache.
     */
  /* void visitEntries (in nsICacheVisitor visitor); */
  NS_IMETHOD VisitEntries(nsICacheVisitor *visitor) = 0;

  /**
     * Evicts all entries in all devices implied by the storage policy.
     */
  /* void evictEntries (in nsCacheStoragePolicy storagePolicy); */
  NS_IMETHOD EvictEntries(nsCacheStoragePolicy storagePolicy) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICACHESERVICE \
  NS_IMETHOD CreateSession(const char *clientID, nsCacheStoragePolicy storagePolicy, PRBool streamBased, nsICacheSession **_retval); \
  NS_IMETHOD VisitEntries(nsICacheVisitor *visitor); \
  NS_IMETHOD EvictEntries(nsCacheStoragePolicy storagePolicy); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICACHESERVICE(_to) \
  NS_IMETHOD CreateSession(const char *clientID, nsCacheStoragePolicy storagePolicy, PRBool streamBased, nsICacheSession **_retval) { return _to CreateSession(clientID, storagePolicy, streamBased, _retval); } \
  NS_IMETHOD VisitEntries(nsICacheVisitor *visitor) { return _to VisitEntries(visitor); } \
  NS_IMETHOD EvictEntries(nsCacheStoragePolicy storagePolicy) { return _to EvictEntries(storagePolicy); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICACHESERVICE(_to) \
  NS_IMETHOD CreateSession(const char *clientID, nsCacheStoragePolicy storagePolicy, PRBool streamBased, nsICacheSession **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSession(clientID, storagePolicy, streamBased, _retval); } \
  NS_IMETHOD VisitEntries(nsICacheVisitor *visitor) { return !_to ? NS_ERROR_NULL_POINTER : _to->VisitEntries(visitor); } \
  NS_IMETHOD EvictEntries(nsCacheStoragePolicy storagePolicy) { return !_to ? NS_ERROR_NULL_POINTER : _to->EvictEntries(storagePolicy); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsCacheService : public nsICacheService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICACHESERVICE

  nsCacheService();

private:
  ~nsCacheService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsCacheService, nsICacheService)

nsCacheService::nsCacheService()
{
  /* member initializers and constructor code */
}

nsCacheService::~nsCacheService()
{
  /* destructor code */
}

/* nsICacheSession createSession (in string clientID, in nsCacheStoragePolicy storagePolicy, in boolean streamBased); */
NS_IMETHODIMP nsCacheService::CreateSession(const char *clientID, nsCacheStoragePolicy storagePolicy, PRBool streamBased, nsICacheSession **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void visitEntries (in nsICacheVisitor visitor); */
NS_IMETHODIMP nsCacheService::VisitEntries(nsICacheVisitor *visitor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void evictEntries (in nsCacheStoragePolicy storagePolicy); */
NS_IMETHODIMP nsCacheService::EvictEntries(nsCacheStoragePolicy storagePolicy)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

#define NS_CACHESERVICE_EMPTYCACHE_TOPIC_ID "cacheservice:empty-cache"

#endif /* __gen_nsICacheService_h__ */
