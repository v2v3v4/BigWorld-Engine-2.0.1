/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/gfx/idl/nsIPrintOptions.idl
 */

#ifndef __gen_nsIPrintOptions_h__
#define __gen_nsIPrintOptions_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsIPrintSettings_h__
#include "nsIPrintSettings.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "nsFont.h"
class nsISimpleEnumerator; /* forward declaration */


/* starting interface:    nsIPrintOptions */
#define NS_IPRINTOPTIONS_IID_STR "d9948a4d-f49c-4456-938a-acda2c8d7741"

#define NS_IPRINTOPTIONS_IID \
  {0xd9948a4d, 0xf49c, 0x4456, \
    { 0x93, 0x8a, 0xac, 0xda, 0x2c, 0x8d, 0x77, 0x41 }}

/**
 * Print options interface
 *
 * Do not attempt to freeze this API - it still needs lots of work. Consult
 * John Keiser <jkeiser@netscape.com> and Roland Mainz
 * <roland.mainz@informatik.med.uni-giessen.de> for futher details.
 */
class NS_NO_VTABLE nsIPrintOptions : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPRINTOPTIONS_IID)

  /**
   * Show Native Print Options dialog, this may not be supported on all platforms
   */
  /* void ShowPrintSetupDialog (in nsIPrintSettings aThePrintSettings); */
  NS_IMETHOD ShowPrintSetupDialog(nsIPrintSettings *aThePrintSettings) = 0;

  /**
   * Creates a new PrintSettnigs Object
   * and initializes it from prefs
   */
  /* nsIPrintSettings CreatePrintSettings (); */
  NS_IMETHOD CreatePrintSettings(nsIPrintSettings **_retval) = 0;

  /**
   * available Printers
   * It returns an enumerator object or throws an exception on error cases
   * like if internal setup failed and/or no printers are available.
   */
  /* nsISimpleEnumerator availablePrinters (); */
  NS_IMETHOD AvailablePrinters(nsISimpleEnumerator **_retval) = 0;

  /**
   * Get a prefixed integer pref 
   */
  /* PRInt32 getPrinterPrefInt (in nsIPrintSettings aPrintSettings, in wstring aPrefName); */
  NS_IMETHOD GetPrinterPrefInt(nsIPrintSettings *aPrintSettings, const PRUnichar *aPrefName, PRInt32 *_retval) = 0;

  /**
   * display Printer Job Properties dialog
   */
  /* void displayJobProperties (in wstring aPrinter, in nsIPrintSettings aPrintSettings, out boolean aDisplayed); */
  NS_IMETHOD DisplayJobProperties(const PRUnichar *aPrinter, nsIPrintSettings *aPrintSettings, PRBool *aDisplayed) = 0;

  /* void setFontNamePointSize (in AString aName, in PRInt32 aPointSize); */
  NS_IMETHOD SetFontNamePointSize(const nsAString & aName, PRInt32 aPointSize) = 0;

  /* [noscript] void SetDefaultFont (in nsNativeFontRef aFont); */
  NS_IMETHOD SetDefaultFont(nsFont & aFont) = 0;

  /* [noscript] void GetDefaultFont (in nsNativeFontRef aFont); */
  NS_IMETHOD GetDefaultFont(nsFont & aFont) = 0;

  /**
   * Native data constants
   */
  enum { kNativeDataPrintRecord = 0 };

  /* [noscript] voidPtr GetNativeData (in short aDataType); */
  NS_IMETHOD GetNativeData(PRInt16 aDataType, void * *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPRINTOPTIONS \
  NS_IMETHOD ShowPrintSetupDialog(nsIPrintSettings *aThePrintSettings); \
  NS_IMETHOD CreatePrintSettings(nsIPrintSettings **_retval); \
  NS_IMETHOD AvailablePrinters(nsISimpleEnumerator **_retval); \
  NS_IMETHOD GetPrinterPrefInt(nsIPrintSettings *aPrintSettings, const PRUnichar *aPrefName, PRInt32 *_retval); \
  NS_IMETHOD DisplayJobProperties(const PRUnichar *aPrinter, nsIPrintSettings *aPrintSettings, PRBool *aDisplayed); \
  NS_IMETHOD SetFontNamePointSize(const nsAString & aName, PRInt32 aPointSize); \
  NS_IMETHOD SetDefaultFont(nsFont & aFont); \
  NS_IMETHOD GetDefaultFont(nsFont & aFont); \
  NS_IMETHOD GetNativeData(PRInt16 aDataType, void * *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPRINTOPTIONS(_to) \
  NS_IMETHOD ShowPrintSetupDialog(nsIPrintSettings *aThePrintSettings) { return _to ShowPrintSetupDialog(aThePrintSettings); } \
  NS_IMETHOD CreatePrintSettings(nsIPrintSettings **_retval) { return _to CreatePrintSettings(_retval); } \
  NS_IMETHOD AvailablePrinters(nsISimpleEnumerator **_retval) { return _to AvailablePrinters(_retval); } \
  NS_IMETHOD GetPrinterPrefInt(nsIPrintSettings *aPrintSettings, const PRUnichar *aPrefName, PRInt32 *_retval) { return _to GetPrinterPrefInt(aPrintSettings, aPrefName, _retval); } \
  NS_IMETHOD DisplayJobProperties(const PRUnichar *aPrinter, nsIPrintSettings *aPrintSettings, PRBool *aDisplayed) { return _to DisplayJobProperties(aPrinter, aPrintSettings, aDisplayed); } \
  NS_IMETHOD SetFontNamePointSize(const nsAString & aName, PRInt32 aPointSize) { return _to SetFontNamePointSize(aName, aPointSize); } \
  NS_IMETHOD SetDefaultFont(nsFont & aFont) { return _to SetDefaultFont(aFont); } \
  NS_IMETHOD GetDefaultFont(nsFont & aFont) { return _to GetDefaultFont(aFont); } \
  NS_IMETHOD GetNativeData(PRInt16 aDataType, void * *_retval) { return _to GetNativeData(aDataType, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPRINTOPTIONS(_to) \
  NS_IMETHOD ShowPrintSetupDialog(nsIPrintSettings *aThePrintSettings) { return !_to ? NS_ERROR_NULL_POINTER : _to->ShowPrintSetupDialog(aThePrintSettings); } \
  NS_IMETHOD CreatePrintSettings(nsIPrintSettings **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreatePrintSettings(_retval); } \
  NS_IMETHOD AvailablePrinters(nsISimpleEnumerator **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->AvailablePrinters(_retval); } \
  NS_IMETHOD GetPrinterPrefInt(nsIPrintSettings *aPrintSettings, const PRUnichar *aPrefName, PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrinterPrefInt(aPrintSettings, aPrefName, _retval); } \
  NS_IMETHOD DisplayJobProperties(const PRUnichar *aPrinter, nsIPrintSettings *aPrintSettings, PRBool *aDisplayed) { return !_to ? NS_ERROR_NULL_POINTER : _to->DisplayJobProperties(aPrinter, aPrintSettings, aDisplayed); } \
  NS_IMETHOD SetFontNamePointSize(const nsAString & aName, PRInt32 aPointSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFontNamePointSize(aName, aPointSize); } \
  NS_IMETHOD SetDefaultFont(nsFont & aFont) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDefaultFont(aFont); } \
  NS_IMETHOD GetDefaultFont(nsFont & aFont) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDefaultFont(aFont); } \
  NS_IMETHOD GetNativeData(PRInt16 aDataType, void * *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNativeData(aDataType, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsPrintOptions : public nsIPrintOptions
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPRINTOPTIONS

  nsPrintOptions();

private:
  ~nsPrintOptions();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsPrintOptions, nsIPrintOptions)

nsPrintOptions::nsPrintOptions()
{
  /* member initializers and constructor code */
}

nsPrintOptions::~nsPrintOptions()
{
  /* destructor code */
}

/* void ShowPrintSetupDialog (in nsIPrintSettings aThePrintSettings); */
NS_IMETHODIMP nsPrintOptions::ShowPrintSetupDialog(nsIPrintSettings *aThePrintSettings)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIPrintSettings CreatePrintSettings (); */
NS_IMETHODIMP nsPrintOptions::CreatePrintSettings(nsIPrintSettings **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsISimpleEnumerator availablePrinters (); */
NS_IMETHODIMP nsPrintOptions::AvailablePrinters(nsISimpleEnumerator **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* PRInt32 getPrinterPrefInt (in nsIPrintSettings aPrintSettings, in wstring aPrefName); */
NS_IMETHODIMP nsPrintOptions::GetPrinterPrefInt(nsIPrintSettings *aPrintSettings, const PRUnichar *aPrefName, PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void displayJobProperties (in wstring aPrinter, in nsIPrintSettings aPrintSettings, out boolean aDisplayed); */
NS_IMETHODIMP nsPrintOptions::DisplayJobProperties(const PRUnichar *aPrinter, nsIPrintSettings *aPrintSettings, PRBool *aDisplayed)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setFontNamePointSize (in AString aName, in PRInt32 aPointSize); */
NS_IMETHODIMP nsPrintOptions::SetFontNamePointSize(const nsAString & aName, PRInt32 aPointSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void SetDefaultFont (in nsNativeFontRef aFont); */
NS_IMETHODIMP nsPrintOptions::SetDefaultFont(nsFont & aFont)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void GetDefaultFont (in nsNativeFontRef aFont); */
NS_IMETHODIMP nsPrintOptions::GetDefaultFont(nsFont & aFont)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] voidPtr GetNativeData (in short aDataType); */
NS_IMETHODIMP nsPrintOptions::GetNativeData(PRInt16 aDataType, void * *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIPrinterEnumerator */
#define NS_IPRINTERENUMERATOR_IID_STR "a6cf9128-15b3-11d2-932e-00805f8add32"

#define NS_IPRINTERENUMERATOR_IID \
  {0xa6cf9128, 0x15b3, 0x11d2, \
    { 0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32 }}

class NS_NO_VTABLE nsIPrinterEnumerator : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPRINTERENUMERATOR_IID)

  /**
   * The name of the default printer 
   * This name must be in the list of printer names returned by
   * "availablePrinters"
   * 
   */
  /* readonly attribute wstring defaultPrinterName; */
  NS_IMETHOD GetDefaultPrinterName(PRUnichar * *aDefaultPrinterName) = 0;

  /**
   * Initializes certain settings from the native printer into the PrintSettings
   * These settings include, but are not limited to:
   *   Page Orientation
   *   Page Size
   *   Number of Copies
   */
  /* void initPrintSettingsFromPrinter (in wstring aPrinterName, in nsIPrintSettings aPrintSettings); */
  NS_IMETHOD InitPrintSettingsFromPrinter(const PRUnichar *aPrinterName, nsIPrintSettings *aPrintSettings) = 0;

  /**
   * Returns an array of the names of all installed printers.
   *
   * @param  aCount     returns number of printers returned
   * @param  aResult    returns array of names
   * @return void
   */
  /* void enumeratePrinters (out PRUint32 aCount, [array, size_is (aCount), retval] out wstring aResult); */
  NS_IMETHOD EnumeratePrinters(PRUint32 *aCount, PRUnichar ***aResult) = 0;

  /* void displayPropertiesDlg (in wstring aPrinter, in nsIPrintSettings aPrintSettings); */
  NS_IMETHOD DisplayPropertiesDlg(const PRUnichar *aPrinter, nsIPrintSettings *aPrintSettings) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPRINTERENUMERATOR \
  NS_IMETHOD GetDefaultPrinterName(PRUnichar * *aDefaultPrinterName); \
  NS_IMETHOD InitPrintSettingsFromPrinter(const PRUnichar *aPrinterName, nsIPrintSettings *aPrintSettings); \
  NS_IMETHOD EnumeratePrinters(PRUint32 *aCount, PRUnichar ***aResult); \
  NS_IMETHOD DisplayPropertiesDlg(const PRUnichar *aPrinter, nsIPrintSettings *aPrintSettings); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPRINTERENUMERATOR(_to) \
  NS_IMETHOD GetDefaultPrinterName(PRUnichar * *aDefaultPrinterName) { return _to GetDefaultPrinterName(aDefaultPrinterName); } \
  NS_IMETHOD InitPrintSettingsFromPrinter(const PRUnichar *aPrinterName, nsIPrintSettings *aPrintSettings) { return _to InitPrintSettingsFromPrinter(aPrinterName, aPrintSettings); } \
  NS_IMETHOD EnumeratePrinters(PRUint32 *aCount, PRUnichar ***aResult) { return _to EnumeratePrinters(aCount, aResult); } \
  NS_IMETHOD DisplayPropertiesDlg(const PRUnichar *aPrinter, nsIPrintSettings *aPrintSettings) { return _to DisplayPropertiesDlg(aPrinter, aPrintSettings); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPRINTERENUMERATOR(_to) \
  NS_IMETHOD GetDefaultPrinterName(PRUnichar * *aDefaultPrinterName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDefaultPrinterName(aDefaultPrinterName); } \
  NS_IMETHOD InitPrintSettingsFromPrinter(const PRUnichar *aPrinterName, nsIPrintSettings *aPrintSettings) { return !_to ? NS_ERROR_NULL_POINTER : _to->InitPrintSettingsFromPrinter(aPrinterName, aPrintSettings); } \
  NS_IMETHOD EnumeratePrinters(PRUint32 *aCount, PRUnichar ***aResult) { return !_to ? NS_ERROR_NULL_POINTER : _to->EnumeratePrinters(aCount, aResult); } \
  NS_IMETHOD DisplayPropertiesDlg(const PRUnichar *aPrinter, nsIPrintSettings *aPrintSettings) { return !_to ? NS_ERROR_NULL_POINTER : _to->DisplayPropertiesDlg(aPrinter, aPrintSettings); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsPrinterEnumerator : public nsIPrinterEnumerator
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPRINTERENUMERATOR

  nsPrinterEnumerator();

private:
  ~nsPrinterEnumerator();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsPrinterEnumerator, nsIPrinterEnumerator)

nsPrinterEnumerator::nsPrinterEnumerator()
{
  /* member initializers and constructor code */
}

nsPrinterEnumerator::~nsPrinterEnumerator()
{
  /* destructor code */
}

/* readonly attribute wstring defaultPrinterName; */
NS_IMETHODIMP nsPrinterEnumerator::GetDefaultPrinterName(PRUnichar * *aDefaultPrinterName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void initPrintSettingsFromPrinter (in wstring aPrinterName, in nsIPrintSettings aPrintSettings); */
NS_IMETHODIMP nsPrinterEnumerator::InitPrintSettingsFromPrinter(const PRUnichar *aPrinterName, nsIPrintSettings *aPrintSettings)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void enumeratePrinters (out PRUint32 aCount, [array, size_is (aCount), retval] out wstring aResult); */
NS_IMETHODIMP nsPrinterEnumerator::EnumeratePrinters(PRUint32 *aCount, PRUnichar ***aResult)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void displayPropertiesDlg (in wstring aPrinter, in nsIPrintSettings aPrintSettings); */
NS_IMETHODIMP nsPrinterEnumerator::DisplayPropertiesDlg(const PRUnichar *aPrinter, nsIPrintSettings *aPrintSettings)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIPrintOptions_h__ */
