/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/threads/nsIProcess.idl
 */

#ifndef __gen_nsIProcess_h__
#define __gen_nsIProcess_h__


#ifndef __gen_nsIFile_h__
#include "nsIFile.h"
#endif

#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIProcess */
#define NS_IPROCESS_IID_STR "9da0b650-d07e-4617-a18a-250035572ac8"

#define NS_IPROCESS_IID \
  {0x9da0b650, 0xd07e, 0x4617, \
    { 0xa1, 0x8a, 0x25, 0x00, 0x35, 0x57, 0x2a, 0xc8 }}

class NS_NO_VTABLE nsIProcess : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPROCESS_IID)

  /* void init (in nsIFile executable); */
  NS_IMETHOD Init(nsIFile *executable) = 0;

  /* void initWithPid (in unsigned long pid); */
  NS_IMETHOD InitWithPid(PRUint32 pid) = 0;

  /* void kill (); */
  NS_IMETHOD Kill(void) = 0;

  /** XXX what charset? **/
/** Executes the file this object was initialized with
         * @param blocking Whether to wait until the process terminates before returning or not
         * @param args An array of arguments to pass to the process
         * @param count The length of the args array
         * @return the PID of the newly spawned process */
  /* unsigned long run (in boolean blocking, [array, size_is (count)] in string args, in unsigned long count); */
  NS_IMETHOD Run(PRBool blocking, const char **args, PRUint32 count, PRUint32 *_retval) = 0;

  /* readonly attribute nsIFile location; */
  NS_IMETHOD GetLocation(nsIFile * *aLocation) = 0;

  /* readonly attribute unsigned long pid; */
  NS_IMETHOD GetPid(PRUint32 *aPid) = 0;

  /* readonly attribute string processName; */
  NS_IMETHOD GetProcessName(char * *aProcessName) = 0;

  /* readonly attribute unsigned long processSignature; */
  NS_IMETHOD GetProcessSignature(PRUint32 *aProcessSignature) = 0;

  /* readonly attribute long exitValue; */
  NS_IMETHOD GetExitValue(PRInt32 *aExitValue) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPROCESS \
  NS_IMETHOD Init(nsIFile *executable); \
  NS_IMETHOD InitWithPid(PRUint32 pid); \
  NS_IMETHOD Kill(void); \
  NS_IMETHOD Run(PRBool blocking, const char **args, PRUint32 count, PRUint32 *_retval); \
  NS_IMETHOD GetLocation(nsIFile * *aLocation); \
  NS_IMETHOD GetPid(PRUint32 *aPid); \
  NS_IMETHOD GetProcessName(char * *aProcessName); \
  NS_IMETHOD GetProcessSignature(PRUint32 *aProcessSignature); \
  NS_IMETHOD GetExitValue(PRInt32 *aExitValue); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPROCESS(_to) \
  NS_IMETHOD Init(nsIFile *executable) { return _to Init(executable); } \
  NS_IMETHOD InitWithPid(PRUint32 pid) { return _to InitWithPid(pid); } \
  NS_IMETHOD Kill(void) { return _to Kill(); } \
  NS_IMETHOD Run(PRBool blocking, const char **args, PRUint32 count, PRUint32 *_retval) { return _to Run(blocking, args, count, _retval); } \
  NS_IMETHOD GetLocation(nsIFile * *aLocation) { return _to GetLocation(aLocation); } \
  NS_IMETHOD GetPid(PRUint32 *aPid) { return _to GetPid(aPid); } \
  NS_IMETHOD GetProcessName(char * *aProcessName) { return _to GetProcessName(aProcessName); } \
  NS_IMETHOD GetProcessSignature(PRUint32 *aProcessSignature) { return _to GetProcessSignature(aProcessSignature); } \
  NS_IMETHOD GetExitValue(PRInt32 *aExitValue) { return _to GetExitValue(aExitValue); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPROCESS(_to) \
  NS_IMETHOD Init(nsIFile *executable) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(executable); } \
  NS_IMETHOD InitWithPid(PRUint32 pid) { return !_to ? NS_ERROR_NULL_POINTER : _to->InitWithPid(pid); } \
  NS_IMETHOD Kill(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Kill(); } \
  NS_IMETHOD Run(PRBool blocking, const char **args, PRUint32 count, PRUint32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Run(blocking, args, count, _retval); } \
  NS_IMETHOD GetLocation(nsIFile * *aLocation) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLocation(aLocation); } \
  NS_IMETHOD GetPid(PRUint32 *aPid) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPid(aPid); } \
  NS_IMETHOD GetProcessName(char * *aProcessName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetProcessName(aProcessName); } \
  NS_IMETHOD GetProcessSignature(PRUint32 *aProcessSignature) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetProcessSignature(aProcessSignature); } \
  NS_IMETHOD GetExitValue(PRInt32 *aExitValue) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetExitValue(aExitValue); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsProcess : public nsIProcess
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPROCESS

  nsProcess();

private:
  ~nsProcess();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsProcess, nsIProcess)

nsProcess::nsProcess()
{
  /* member initializers and constructor code */
}

nsProcess::~nsProcess()
{
  /* destructor code */
}

/* void init (in nsIFile executable); */
NS_IMETHODIMP nsProcess::Init(nsIFile *executable)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void initWithPid (in unsigned long pid); */
NS_IMETHODIMP nsProcess::InitWithPid(PRUint32 pid)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void kill (); */
NS_IMETHODIMP nsProcess::Kill()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* unsigned long run (in boolean blocking, [array, size_is (count)] in string args, in unsigned long count); */
NS_IMETHODIMP nsProcess::Run(PRBool blocking, const char **args, PRUint32 count, PRUint32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIFile location; */
NS_IMETHODIMP nsProcess::GetLocation(nsIFile * *aLocation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long pid; */
NS_IMETHODIMP nsProcess::GetPid(PRUint32 *aPid)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute string processName; */
NS_IMETHODIMP nsProcess::GetProcessName(char * *aProcessName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long processSignature; */
NS_IMETHODIMP nsProcess::GetProcessSignature(PRUint32 *aProcessSignature)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long exitValue; */
NS_IMETHODIMP nsProcess::GetExitValue(PRInt32 *aExitValue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

#define NS_PROCESS_CONTRACTID "@mozilla.org/process/util;1"
#define NS_PROCESS_CLASSNAME "Process Specification"

#endif /* __gen_nsIProcess_h__ */
