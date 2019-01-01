/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsPISocketTransportService.idl
 */

#ifndef __gen_nsPISocketTransportService_h__
#define __gen_nsPISocketTransportService_h__


#ifndef __gen_nsISocketTransportService_h__
#include "nsISocketTransportService.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsPISocketTransportService */
#define NS_PISOCKETTRANSPORTSERVICE_IID_STR "6f704e69-a5fb-11d9-8ce8-0011246ecd24"

#define NS_PISOCKETTRANSPORTSERVICE_IID \
  {0x6f704e69, 0xa5fb, 0x11d9, \
    { 0x8c, 0xe8, 0x00, 0x11, 0x24, 0x6e, 0xcd, 0x24 }}

/**
 * This is a private interface used by the internals of the networking library.
 * It will never be frozen.  Do not use it in external code.
 */
class NS_NO_VTABLE nsPISocketTransportService : public nsISocketTransportService {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_PISOCKETTRANSPORTSERVICE_IID)

  /**
   * init/shutdown routines.
   */
  /* void init (); */
  NS_IMETHOD Init(void) = 0;

  /* void shutdown (); */
  NS_IMETHOD Shutdown(void) = 0;

  /**
   * controls whether or not the socket transport service should poke
   * the autodialer on connection failure.
   */
  /* attribute boolean autodialEnabled; */
  NS_IMETHOD GetAutodialEnabled(PRBool *aAutodialEnabled) = 0;
  NS_IMETHOD SetAutodialEnabled(PRBool aAutodialEnabled) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSPISOCKETTRANSPORTSERVICE \
  NS_IMETHOD Init(void); \
  NS_IMETHOD Shutdown(void); \
  NS_IMETHOD GetAutodialEnabled(PRBool *aAutodialEnabled); \
  NS_IMETHOD SetAutodialEnabled(PRBool aAutodialEnabled); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSPISOCKETTRANSPORTSERVICE(_to) \
  NS_IMETHOD Init(void) { return _to Init(); } \
  NS_IMETHOD Shutdown(void) { return _to Shutdown(); } \
  NS_IMETHOD GetAutodialEnabled(PRBool *aAutodialEnabled) { return _to GetAutodialEnabled(aAutodialEnabled); } \
  NS_IMETHOD SetAutodialEnabled(PRBool aAutodialEnabled) { return _to SetAutodialEnabled(aAutodialEnabled); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSPISOCKETTRANSPORTSERVICE(_to) \
  NS_IMETHOD Init(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(); } \
  NS_IMETHOD Shutdown(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Shutdown(); } \
  NS_IMETHOD GetAutodialEnabled(PRBool *aAutodialEnabled) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAutodialEnabled(aAutodialEnabled); } \
  NS_IMETHOD SetAutodialEnabled(PRBool aAutodialEnabled) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetAutodialEnabled(aAutodialEnabled); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public nsPISocketTransportService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSPISOCKETTRANSPORTSERVICE

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(_MYCLASS_, nsPISocketTransportService)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void init (); */
NS_IMETHODIMP _MYCLASS_::Init()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void shutdown (); */
NS_IMETHODIMP _MYCLASS_::Shutdown()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean autodialEnabled; */
NS_IMETHODIMP _MYCLASS_::GetAutodialEnabled(PRBool *aAutodialEnabled)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP _MYCLASS_::SetAutodialEnabled(PRBool aAutodialEnabled)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsPISocketTransportService_h__ */
