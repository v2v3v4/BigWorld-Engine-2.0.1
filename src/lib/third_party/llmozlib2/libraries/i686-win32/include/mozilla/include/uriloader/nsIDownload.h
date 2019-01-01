/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/uriloader/base/nsIDownload.idl
 */

#ifndef __gen_nsIDownload_h__
#define __gen_nsIDownload_h__


#ifndef __gen_nsITransfer_h__
#include "nsITransfer.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIURI; /* forward declaration */

class nsILocalFile; /* forward declaration */

class nsIObserver; /* forward declaration */

class nsICancelable; /* forward declaration */

class nsIWebProgressListener; /* forward declaration */

class nsIMIMEInfo; /* forward declaration */


/* starting interface:    nsIDownload */
#define NS_IDOWNLOAD_IID_STR "9e1fd9f2-9727-4926-85cd-f16c375bba6d"

#define NS_IDOWNLOAD_IID \
  {0x9e1fd9f2, 0x9727, 0x4926, \
    { 0x85, 0xcd, 0xf1, 0x6c, 0x37, 0x5b, 0xba, 0x6d }}

class NS_NO_VTABLE nsIDownload : public nsITransfer {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOWNLOAD_IID)

  /**
     * The target of a download is always a file on the local file system.
     */
  /* readonly attribute nsILocalFile targetFile; */
  NS_IMETHOD GetTargetFile(nsILocalFile * *aTargetFile) = 0;

  /**
     * The percentage of transfer completed.
     * If the file size is unknown it'll be -1 here.
     */
  /* readonly attribute PRInt32 percentComplete; */
  NS_IMETHOD GetPercentComplete(PRInt32 *aPercentComplete) = 0;

  /**
     * The amount of kbytes downloaded so far.
     */
  /* readonly attribute PRUint64 amountTransferred; */
  NS_IMETHOD GetAmountTransferred(PRUint64 *aAmountTransferred) = 0;

  /**
     * The size of file in kbytes.
     * Unknown size is represented by 0.
     */
  /* readonly attribute PRUint64 size; */
  NS_IMETHOD GetSize(PRUint64 *aSize) = 0;

  /**
     * The source of the transfer.
     */
  /* readonly attribute nsIURI source; */
  NS_IMETHOD GetSource(nsIURI * *aSource) = 0;

  /**
     * The target of the transfer.
     */
  /* readonly attribute nsIURI target; */
  NS_IMETHOD GetTarget(nsIURI * *aTarget) = 0;

  /**
     * Object that can be used to cancel the download.
     * Will be null after the download is finished.
     */
  /* readonly attribute nsICancelable cancelable; */
  NS_IMETHOD GetCancelable(nsICancelable * *aCancelable) = 0;

  /**
     * The user-readable description of the transfer.
     */
  /* readonly attribute wstring displayName; */
  NS_IMETHOD GetDisplayName(PRUnichar * *aDisplayName) = 0;

  /**
     * The time a transfer was started.
     */
  /* readonly attribute long long startTime; */
  NS_IMETHOD GetStartTime(PRInt64 *aStartTime) = 0;

  /**
     * Optional. If set, it will contain the target's relevant MIME information.
     * This includes it's MIME Type, helper app, and whether that helper should be
     * executed.
     */
  /* readonly attribute nsIMIMEInfo MIMEInfo; */
  NS_IMETHOD GetMIMEInfo(nsIMIMEInfo * *aMIMEInfo) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOWNLOAD \
  NS_IMETHOD GetTargetFile(nsILocalFile * *aTargetFile); \
  NS_IMETHOD GetPercentComplete(PRInt32 *aPercentComplete); \
  NS_IMETHOD GetAmountTransferred(PRUint64 *aAmountTransferred); \
  NS_IMETHOD GetSize(PRUint64 *aSize); \
  NS_IMETHOD GetSource(nsIURI * *aSource); \
  NS_IMETHOD GetTarget(nsIURI * *aTarget); \
  NS_IMETHOD GetCancelable(nsICancelable * *aCancelable); \
  NS_IMETHOD GetDisplayName(PRUnichar * *aDisplayName); \
  NS_IMETHOD GetStartTime(PRInt64 *aStartTime); \
  NS_IMETHOD GetMIMEInfo(nsIMIMEInfo * *aMIMEInfo); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOWNLOAD(_to) \
  NS_IMETHOD GetTargetFile(nsILocalFile * *aTargetFile) { return _to GetTargetFile(aTargetFile); } \
  NS_IMETHOD GetPercentComplete(PRInt32 *aPercentComplete) { return _to GetPercentComplete(aPercentComplete); } \
  NS_IMETHOD GetAmountTransferred(PRUint64 *aAmountTransferred) { return _to GetAmountTransferred(aAmountTransferred); } \
  NS_IMETHOD GetSize(PRUint64 *aSize) { return _to GetSize(aSize); } \
  NS_IMETHOD GetSource(nsIURI * *aSource) { return _to GetSource(aSource); } \
  NS_IMETHOD GetTarget(nsIURI * *aTarget) { return _to GetTarget(aTarget); } \
  NS_IMETHOD GetCancelable(nsICancelable * *aCancelable) { return _to GetCancelable(aCancelable); } \
  NS_IMETHOD GetDisplayName(PRUnichar * *aDisplayName) { return _to GetDisplayName(aDisplayName); } \
  NS_IMETHOD GetStartTime(PRInt64 *aStartTime) { return _to GetStartTime(aStartTime); } \
  NS_IMETHOD GetMIMEInfo(nsIMIMEInfo * *aMIMEInfo) { return _to GetMIMEInfo(aMIMEInfo); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOWNLOAD(_to) \
  NS_IMETHOD GetTargetFile(nsILocalFile * *aTargetFile) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTargetFile(aTargetFile); } \
  NS_IMETHOD GetPercentComplete(PRInt32 *aPercentComplete) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPercentComplete(aPercentComplete); } \
  NS_IMETHOD GetAmountTransferred(PRUint64 *aAmountTransferred) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAmountTransferred(aAmountTransferred); } \
  NS_IMETHOD GetSize(PRUint64 *aSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSize(aSize); } \
  NS_IMETHOD GetSource(nsIURI * *aSource) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSource(aSource); } \
  NS_IMETHOD GetTarget(nsIURI * *aTarget) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTarget(aTarget); } \
  NS_IMETHOD GetCancelable(nsICancelable * *aCancelable) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCancelable(aCancelable); } \
  NS_IMETHOD GetDisplayName(PRUnichar * *aDisplayName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDisplayName(aDisplayName); } \
  NS_IMETHOD GetStartTime(PRInt64 *aStartTime) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStartTime(aStartTime); } \
  NS_IMETHOD GetMIMEInfo(nsIMIMEInfo * *aMIMEInfo) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMIMEInfo(aMIMEInfo); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDownload : public nsIDownload
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOWNLOAD

  nsDownload();

private:
  ~nsDownload();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDownload, nsIDownload)

nsDownload::nsDownload()
{
  /* member initializers and constructor code */
}

nsDownload::~nsDownload()
{
  /* destructor code */
}

/* readonly attribute nsILocalFile targetFile; */
NS_IMETHODIMP nsDownload::GetTargetFile(nsILocalFile * *aTargetFile)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRInt32 percentComplete; */
NS_IMETHODIMP nsDownload::GetPercentComplete(PRInt32 *aPercentComplete)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRUint64 amountTransferred; */
NS_IMETHODIMP nsDownload::GetAmountTransferred(PRUint64 *aAmountTransferred)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRUint64 size; */
NS_IMETHODIMP nsDownload::GetSize(PRUint64 *aSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIURI source; */
NS_IMETHODIMP nsDownload::GetSource(nsIURI * *aSource)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIURI target; */
NS_IMETHODIMP nsDownload::GetTarget(nsIURI * *aTarget)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsICancelable cancelable; */
NS_IMETHODIMP nsDownload::GetCancelable(nsICancelable * *aCancelable)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute wstring displayName; */
NS_IMETHODIMP nsDownload::GetDisplayName(PRUnichar * *aDisplayName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long long startTime; */
NS_IMETHODIMP nsDownload::GetStartTime(PRInt64 *aStartTime)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIMIMEInfo MIMEInfo; */
NS_IMETHODIMP nsDownload::GetMIMEInfo(nsIMIMEInfo * *aMIMEInfo)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDownload_MOZILLA_1_8_BRANCH */
#define NS_IDOWNLOAD_MOZILLA_1_8_BRANCH_IID_STR "ff76f0c7-caaf-4e64-8896-154348322696"

#define NS_IDOWNLOAD_MOZILLA_1_8_BRANCH_IID \
  {0xff76f0c7, 0xcaaf, 0x4e64, \
    { 0x88, 0x96, 0x15, 0x43, 0x48, 0x32, 0x26, 0x96 }}

class NS_NO_VTABLE nsIDownload_MOZILLA_1_8_BRANCH : public nsIDownload {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOWNLOAD_MOZILLA_1_8_BRANCH_IID)

  /**
     * The speed of the transfer in bytes/sec.
     */
  /* readonly attribute double speed; */
  NS_IMETHOD GetSpeed(double *aSpeed) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOWNLOAD_MOZILLA_1_8_BRANCH \
  NS_IMETHOD GetSpeed(double *aSpeed); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOWNLOAD_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD GetSpeed(double *aSpeed) { return _to GetSpeed(aSpeed); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOWNLOAD_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD GetSpeed(double *aSpeed) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSpeed(aSpeed); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDownload_MOZILLA_1_8_BRANCH : public nsIDownload_MOZILLA_1_8_BRANCH
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOWNLOAD_MOZILLA_1_8_BRANCH

  nsDownload_MOZILLA_1_8_BRANCH();

private:
  ~nsDownload_MOZILLA_1_8_BRANCH();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDownload_MOZILLA_1_8_BRANCH, nsIDownload_MOZILLA_1_8_BRANCH)

nsDownload_MOZILLA_1_8_BRANCH::nsDownload_MOZILLA_1_8_BRANCH()
{
  /* member initializers and constructor code */
}

nsDownload_MOZILLA_1_8_BRANCH::~nsDownload_MOZILLA_1_8_BRANCH()
{
  /* destructor code */
}

/* readonly attribute double speed; */
NS_IMETHODIMP nsDownload_MOZILLA_1_8_BRANCH::GetSpeed(double *aSpeed)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

// {E3FA9D0A-1DD1-11B2-BDEF-8C720B597445}
#define NS_DOWNLOAD_CID \
    { 0xe3fa9d0a, 0x1dd1, 0x11b2, { 0xbd, 0xef, 0x8c, 0x72, 0x0b, 0x59, 0x74, 0x45 } }

#endif /* __gen_nsIDownload_h__ */
