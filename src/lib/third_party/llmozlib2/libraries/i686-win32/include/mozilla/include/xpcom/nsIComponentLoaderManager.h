/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/components/nsIComponentLoaderManager.idl
 */

#ifndef __gen_nsIComponentLoaderManager_h__
#define __gen_nsIComponentLoaderManager_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIFile; /* forward declaration */


/* starting interface:    nsIComponentLoaderManager */
#define NS_ICOMPONENTLOADERMANAGER_IID_STR "fce83d37-a3c0-4e09-ad9f-6842a984dbdf"

#define NS_ICOMPONENTLOADERMANAGER_IID \
  {0xfce83d37, 0xa3c0, 0x4e09, \
    { 0xad, 0x9f, 0x68, 0x42, 0xa9, 0x84, 0xdb, 0xdf }}

class NS_NO_VTABLE nsIComponentLoaderManager : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICOMPONENTLOADERMANAGER_IID)

  /* boolean hasFileChanged (in nsIFile file, in string loaderString, in PRInt64 modDate); */
  NS_IMETHOD HasFileChanged(nsIFile *file, const char *loaderString, PRInt64 modDate, PRBool *_retval) = 0;

  /* void saveFileInfo (in nsIFile file, in string loaderString, in PRInt64 modDate); */
  NS_IMETHOD SaveFileInfo(nsIFile *file, const char *loaderString, PRInt64 modDate) = 0;

  /* void removeFileInfo (in nsIFile file, in string loaderString); */
  NS_IMETHOD RemoveFileInfo(nsIFile *file, const char *loaderString) = 0;

  /* void flushPersistentStore (in boolean now); */
  NS_IMETHOD FlushPersistentStore(PRBool now) = 0;

  /* string getOptionalData (in nsIFile file, in string loaderString); */
  NS_IMETHOD GetOptionalData(nsIFile *file, const char *loaderString, char **_retval) = 0;

  /* void setOptionalData (in nsIFile file, in string loaderString, in string value); */
  NS_IMETHOD SetOptionalData(nsIFile *file, const char *loaderString, const char *value) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICOMPONENTLOADERMANAGER \
  NS_IMETHOD HasFileChanged(nsIFile *file, const char *loaderString, PRInt64 modDate, PRBool *_retval); \
  NS_IMETHOD SaveFileInfo(nsIFile *file, const char *loaderString, PRInt64 modDate); \
  NS_IMETHOD RemoveFileInfo(nsIFile *file, const char *loaderString); \
  NS_IMETHOD FlushPersistentStore(PRBool now); \
  NS_IMETHOD GetOptionalData(nsIFile *file, const char *loaderString, char **_retval); \
  NS_IMETHOD SetOptionalData(nsIFile *file, const char *loaderString, const char *value); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICOMPONENTLOADERMANAGER(_to) \
  NS_IMETHOD HasFileChanged(nsIFile *file, const char *loaderString, PRInt64 modDate, PRBool *_retval) { return _to HasFileChanged(file, loaderString, modDate, _retval); } \
  NS_IMETHOD SaveFileInfo(nsIFile *file, const char *loaderString, PRInt64 modDate) { return _to SaveFileInfo(file, loaderString, modDate); } \
  NS_IMETHOD RemoveFileInfo(nsIFile *file, const char *loaderString) { return _to RemoveFileInfo(file, loaderString); } \
  NS_IMETHOD FlushPersistentStore(PRBool now) { return _to FlushPersistentStore(now); } \
  NS_IMETHOD GetOptionalData(nsIFile *file, const char *loaderString, char **_retval) { return _to GetOptionalData(file, loaderString, _retval); } \
  NS_IMETHOD SetOptionalData(nsIFile *file, const char *loaderString, const char *value) { return _to SetOptionalData(file, loaderString, value); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICOMPONENTLOADERMANAGER(_to) \
  NS_IMETHOD HasFileChanged(nsIFile *file, const char *loaderString, PRInt64 modDate, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->HasFileChanged(file, loaderString, modDate, _retval); } \
  NS_IMETHOD SaveFileInfo(nsIFile *file, const char *loaderString, PRInt64 modDate) { return !_to ? NS_ERROR_NULL_POINTER : _to->SaveFileInfo(file, loaderString, modDate); } \
  NS_IMETHOD RemoveFileInfo(nsIFile *file, const char *loaderString) { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveFileInfo(file, loaderString); } \
  NS_IMETHOD FlushPersistentStore(PRBool now) { return !_to ? NS_ERROR_NULL_POINTER : _to->FlushPersistentStore(now); } \
  NS_IMETHOD GetOptionalData(nsIFile *file, const char *loaderString, char **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOptionalData(file, loaderString, _retval); } \
  NS_IMETHOD SetOptionalData(nsIFile *file, const char *loaderString, const char *value) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOptionalData(file, loaderString, value); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsComponentLoaderManager : public nsIComponentLoaderManager
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICOMPONENTLOADERMANAGER

  nsComponentLoaderManager();

private:
  ~nsComponentLoaderManager();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsComponentLoaderManager, nsIComponentLoaderManager)

nsComponentLoaderManager::nsComponentLoaderManager()
{
  /* member initializers and constructor code */
}

nsComponentLoaderManager::~nsComponentLoaderManager()
{
  /* destructor code */
}

/* boolean hasFileChanged (in nsIFile file, in string loaderString, in PRInt64 modDate); */
NS_IMETHODIMP nsComponentLoaderManager::HasFileChanged(nsIFile *file, const char *loaderString, PRInt64 modDate, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void saveFileInfo (in nsIFile file, in string loaderString, in PRInt64 modDate); */
NS_IMETHODIMP nsComponentLoaderManager::SaveFileInfo(nsIFile *file, const char *loaderString, PRInt64 modDate)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeFileInfo (in nsIFile file, in string loaderString); */
NS_IMETHODIMP nsComponentLoaderManager::RemoveFileInfo(nsIFile *file, const char *loaderString)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void flushPersistentStore (in boolean now); */
NS_IMETHODIMP nsComponentLoaderManager::FlushPersistentStore(PRBool now)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* string getOptionalData (in nsIFile file, in string loaderString); */
NS_IMETHODIMP nsComponentLoaderManager::GetOptionalData(nsIFile *file, const char *loaderString, char **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setOptionalData (in nsIFile file, in string loaderString, in string value); */
NS_IMETHODIMP nsComponentLoaderManager::SetOptionalData(nsIFile *file, const char *loaderString, const char *value)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIComponentLoaderManager_h__ */
