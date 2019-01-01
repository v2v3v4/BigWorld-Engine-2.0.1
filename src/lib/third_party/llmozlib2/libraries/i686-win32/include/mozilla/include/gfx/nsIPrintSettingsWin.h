/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/gfx/idl/nsIPrintSettingsWin.idl
 */

#ifndef __gen_nsIPrintSettingsWin_h__
#define __gen_nsIPrintSettingsWin_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "windows.h"

/* starting interface:    nsIPrintSettingsWin */
#define NS_IPRINTSETTINGSWIN_IID_STR "ff328fe4-41d5-4b78-82ab-6b1fbc7930af"

#define NS_IPRINTSETTINGSWIN_IID \
  {0xff328fe4, 0x41d5, 0x4b78, \
    { 0x82, 0xab, 0x6b, 0x1f, 0xbc, 0x79, 0x30, 0xaf }}

/**
 * Simplified PrintSettings for Windows interface 
 *
 * @status UNDER_REVIEW
 */
class NS_NO_VTABLE nsIPrintSettingsWin : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPRINTSETTINGSWIN_IID)

  /**
   * Data Members
   *
   * Each of these data members make a copy 
   * of the contents. If you get the value, 
   * you own the memory.
   *
   * The following three pieces of data are needed
   * to create a DC for printing. These are typcially 
   * gotten via the PrintDLG call ro can be obtained
   * via the "m_pd" data member of the CPrintDialog
   * in MFC.
   */
  /* [noscript] attribute charPtr deviceName; */
  NS_IMETHOD GetDeviceName(char * *aDeviceName) = 0;
  NS_IMETHOD SetDeviceName(char * aDeviceName) = 0;

  /* [noscript] attribute charPtr driverName; */
  NS_IMETHOD GetDriverName(char * *aDriverName) = 0;
  NS_IMETHOD SetDriverName(char * aDriverName) = 0;

  /* [noscript] attribute nsDevMode devMode; */
  NS_IMETHOD GetDevMode(DEVMODE * *aDevMode) = 0;
  NS_IMETHOD SetDevMode(DEVMODE * aDevMode) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPRINTSETTINGSWIN \
  NS_IMETHOD GetDeviceName(char * *aDeviceName); \
  NS_IMETHOD SetDeviceName(char * aDeviceName); \
  NS_IMETHOD GetDriverName(char * *aDriverName); \
  NS_IMETHOD SetDriverName(char * aDriverName); \
  NS_IMETHOD GetDevMode(DEVMODE * *aDevMode); \
  NS_IMETHOD SetDevMode(DEVMODE * aDevMode); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPRINTSETTINGSWIN(_to) \
  NS_IMETHOD GetDeviceName(char * *aDeviceName) { return _to GetDeviceName(aDeviceName); } \
  NS_IMETHOD SetDeviceName(char * aDeviceName) { return _to SetDeviceName(aDeviceName); } \
  NS_IMETHOD GetDriverName(char * *aDriverName) { return _to GetDriverName(aDriverName); } \
  NS_IMETHOD SetDriverName(char * aDriverName) { return _to SetDriverName(aDriverName); } \
  NS_IMETHOD GetDevMode(DEVMODE * *aDevMode) { return _to GetDevMode(aDevMode); } \
  NS_IMETHOD SetDevMode(DEVMODE * aDevMode) { return _to SetDevMode(aDevMode); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPRINTSETTINGSWIN(_to) \
  NS_IMETHOD GetDeviceName(char * *aDeviceName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDeviceName(aDeviceName); } \
  NS_IMETHOD SetDeviceName(char * aDeviceName) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDeviceName(aDeviceName); } \
  NS_IMETHOD GetDriverName(char * *aDriverName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDriverName(aDriverName); } \
  NS_IMETHOD SetDriverName(char * aDriverName) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDriverName(aDriverName); } \
  NS_IMETHOD GetDevMode(DEVMODE * *aDevMode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDevMode(aDevMode); } \
  NS_IMETHOD SetDevMode(DEVMODE * aDevMode) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDevMode(aDevMode); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsPrintSettingsWin : public nsIPrintSettingsWin
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPRINTSETTINGSWIN

  nsPrintSettingsWin();

private:
  ~nsPrintSettingsWin();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsPrintSettingsWin, nsIPrintSettingsWin)

nsPrintSettingsWin::nsPrintSettingsWin()
{
  /* member initializers and constructor code */
}

nsPrintSettingsWin::~nsPrintSettingsWin()
{
  /* destructor code */
}

/* [noscript] attribute charPtr deviceName; */
NS_IMETHODIMP nsPrintSettingsWin::GetDeviceName(char * *aDeviceName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettingsWin::SetDeviceName(char * aDeviceName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] attribute charPtr driverName; */
NS_IMETHODIMP nsPrintSettingsWin::GetDriverName(char * *aDriverName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettingsWin::SetDriverName(char * aDriverName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] attribute nsDevMode devMode; */
NS_IMETHODIMP nsPrintSettingsWin::GetDevMode(DEVMODE * *aDevMode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettingsWin::SetDevMode(DEVMODE * aDevMode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIPrintSettingsWin_h__ */
