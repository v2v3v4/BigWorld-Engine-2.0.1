/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/base/public/nsISyncLoadDOMService.idl
 */

#ifndef __gen_nsISyncLoadDOMService_h__
#define __gen_nsISyncLoadDOMService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIURI; /* forward declaration */

class nsIDOMDocument; /* forward declaration */

class nsIChannel; /* forward declaration */


/* starting interface:    nsISyncLoadDOMService */
#define NS_ISYNCLOADDOMSERVICE_IID_STR "96a13c30-695a-492c-918b-04ae3edb4e4c"

#define NS_ISYNCLOADDOMSERVICE_IID \
  {0x96a13c30, 0x695a, 0x492c, \
    { 0x91, 0x8b, 0x04, 0xae, 0x3e, 0xdb, 0x4e, 0x4c }}

/*************************************************************************
 *                                                                       *
 *                          **** NOTICE ****                             *
 *                                                                       *
 *    nsISyncLoadDOMService defines synchronous methods to download      *
 *    data from the network.  Any delays from the server will            *
 *    appear as a hang in the mozilla UI.  Therefore, this interface     *
 *    should be avoided as much as possible.                             *
 *                                                                       *
 *    Don't make me come over there!!                                    *
 *                                                                       *
 *                                                                       *
 ************************************************************************/
/**
 * The nsISyncDOMLoadService interface can be used to synchronously load
 * a document.
 */
class NS_NO_VTABLE nsISyncLoadDOMService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISYNCLOADDOMSERVICE_IID)

  /**
     * Synchronously load the document from the specified channel.
     *
     * @param aChannel   The channel to load the document from.
     * @param aLoaderURI URI of loading document. For security checks
     *                   null if no securitychecks should be done
     *
     * @returns The document loaded from the URI.
     */
  /* nsIDOMDocument loadDocument (in nsIChannel aChannel, in nsIURI aLoaderURI); */
  NS_IMETHOD LoadDocument(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval) = 0;

  /* nsIDOMDocument loadDocumentAsXML (in nsIChannel aChannel, in nsIURI aLoaderURI); */
  NS_IMETHOD LoadDocumentAsXML(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval) = 0;

  /**
     * Synchronously load an XML document from the specified
     * channel. The channel must be possible to open synchronously.
     *
     * @param aChannel   The channel to load the document from.
     * @param aLoaderURI URI of loading document. For security checks
     *                   null if no securitychecks should be done
     *
     * @returns The document loaded from the URI.
     */
  /* nsIDOMDocument loadLocalDocument (in nsIChannel aChannel, in nsIURI aLoaderURI); */
  NS_IMETHOD LoadLocalDocument(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval) = 0;

  /**
     * Synchronously load the xbl-document from the specified channel. The channel
     * must be possible to open synchronously.
     *
     * @param aChannel   The channel to load the document from.
     *
     * @returns The document loaded from the URI.
     */
  /* nsIDOMDocument loadLocalXBLDocument (in nsIChannel aChannel); */
  NS_IMETHOD LoadLocalXBLDocument(nsIChannel *aChannel, nsIDOMDocument **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISYNCLOADDOMSERVICE \
  NS_IMETHOD LoadDocument(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval); \
  NS_IMETHOD LoadDocumentAsXML(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval); \
  NS_IMETHOD LoadLocalDocument(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval); \
  NS_IMETHOD LoadLocalXBLDocument(nsIChannel *aChannel, nsIDOMDocument **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISYNCLOADDOMSERVICE(_to) \
  NS_IMETHOD LoadDocument(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval) { return _to LoadDocument(aChannel, aLoaderURI, _retval); } \
  NS_IMETHOD LoadDocumentAsXML(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval) { return _to LoadDocumentAsXML(aChannel, aLoaderURI, _retval); } \
  NS_IMETHOD LoadLocalDocument(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval) { return _to LoadLocalDocument(aChannel, aLoaderURI, _retval); } \
  NS_IMETHOD LoadLocalXBLDocument(nsIChannel *aChannel, nsIDOMDocument **_retval) { return _to LoadLocalXBLDocument(aChannel, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISYNCLOADDOMSERVICE(_to) \
  NS_IMETHOD LoadDocument(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->LoadDocument(aChannel, aLoaderURI, _retval); } \
  NS_IMETHOD LoadDocumentAsXML(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->LoadDocumentAsXML(aChannel, aLoaderURI, _retval); } \
  NS_IMETHOD LoadLocalDocument(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->LoadLocalDocument(aChannel, aLoaderURI, _retval); } \
  NS_IMETHOD LoadLocalXBLDocument(nsIChannel *aChannel, nsIDOMDocument **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->LoadLocalXBLDocument(aChannel, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSyncLoadDOMService : public nsISyncLoadDOMService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISYNCLOADDOMSERVICE

  nsSyncLoadDOMService();

private:
  ~nsSyncLoadDOMService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSyncLoadDOMService, nsISyncLoadDOMService)

nsSyncLoadDOMService::nsSyncLoadDOMService()
{
  /* member initializers and constructor code */
}

nsSyncLoadDOMService::~nsSyncLoadDOMService()
{
  /* destructor code */
}

/* nsIDOMDocument loadDocument (in nsIChannel aChannel, in nsIURI aLoaderURI); */
NS_IMETHODIMP nsSyncLoadDOMService::LoadDocument(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMDocument loadDocumentAsXML (in nsIChannel aChannel, in nsIURI aLoaderURI); */
NS_IMETHODIMP nsSyncLoadDOMService::LoadDocumentAsXML(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMDocument loadLocalDocument (in nsIChannel aChannel, in nsIURI aLoaderURI); */
NS_IMETHODIMP nsSyncLoadDOMService::LoadLocalDocument(nsIChannel *aChannel, nsIURI *aLoaderURI, nsIDOMDocument **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMDocument loadLocalXBLDocument (in nsIChannel aChannel); */
NS_IMETHODIMP nsSyncLoadDOMService::LoadLocalXBLDocument(nsIChannel *aChannel, nsIDOMDocument **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISyncLoadDOMService_h__ */
