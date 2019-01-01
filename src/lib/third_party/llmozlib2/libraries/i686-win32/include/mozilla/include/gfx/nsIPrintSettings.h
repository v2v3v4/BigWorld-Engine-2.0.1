/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/gfx/idl/nsIPrintSettings.idl
 */

#ifndef __gen_nsIPrintSettings_h__
#define __gen_nsIPrintSettings_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "nsMargin.h"
class nsIPrintSession; /* forward declaration */


/* starting interface:    nsIPrintSettings */
#define NS_IPRINTSETTINGS_IID_STR "f1094df6-ce0e-42c9-9847-2f663172c38d"

#define NS_IPRINTSETTINGS_IID \
  {0xf1094df6, 0xce0e, 0x42c9, \
    { 0x98, 0x47, 0x2f, 0x66, 0x31, 0x72, 0xc3, 0x8d }}

/**
 * Simplified graphics interface for JS rendering.
 *
 * @status UNDER_REVIEW
 */
class NS_NO_VTABLE nsIPrintSettings : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPRINTSETTINGS_IID)

  /**
   * PrintSettings to be Saved Navigation Constants
   */
  enum { kInitSaveOddEvenPages = 1U };

  enum { kInitSaveHeaderLeft = 2U };

  enum { kInitSaveHeaderCenter = 4U };

  enum { kInitSaveHeaderRight = 8U };

  enum { kInitSaveFooterLeft = 16U };

  enum { kInitSaveFooterCenter = 32U };

  enum { kInitSaveFooterRight = 64U };

  enum { kInitSaveBGColors = 128U };

  enum { kInitSaveBGImages = 256U };

  enum { kInitSavePaperSize = 512U };

  enum { kInitSavePaperName = 1024U };

  enum { kInitSavePaperSizeUnit = 2048U };

  enum { kInitSavePaperSizeType = 4096U };

  enum { kInitSavePaperData = 8192U };

  enum { kInitSavePaperWidth = 16384U };

  enum { kInitSavePaperHeight = 32768U };

  enum { kInitSaveReversed = 65536U };

  enum { kInitSaveInColor = 131072U };

  enum { kInitSaveOrientation = 262144U };

  enum { kInitSavePrintCommand = 524288U };

  enum { kInitSavePrinterName = 1048576U };

  enum { kInitSavePrintToFile = 2097152U };

  enum { kInitSaveToFileName = 4194304U };

  enum { kInitSavePageDelay = 8388608U };

  enum { kInitSaveMargins = 16777216U };

  enum { kInitSaveNativeData = 33554432U };

  enum { kInitSavePlexName = 67108864U };

  enum { kInitSaveShrinkToFit = 134217728U };

  enum { kInitSaveScaling = 268435456U };

  enum { kInitSaveColorspace = 536870912U };

  enum { kInitSaveResolutionName = 1073741824U };

  enum { kInitSaveDownloadFonts = 2147483648U };

  enum { kInitSaveAll = 4294967295U };

  enum { kPrintOddPages = 1 };

  enum { kPrintEvenPages = 2 };

  enum { kEnableSelectionRB = 4 };

  enum { kRangeAllPages = 0 };

  enum { kRangeSpecifiedPageRange = 1 };

  enum { kRangeSelection = 2 };

  enum { kRangeFocusFrame = 3 };

  enum { kJustLeft = 0 };

  enum { kJustCenter = 1 };

  enum { kJustRight = 2 };

  /**
   * FrameSet Default Type Constants
   */
  enum { kUseInternalDefault = 0 };

  enum { kUseSettingWhenPossible = 1 };

  /**
   * Page Size Type Constants
   */
  enum { kPaperSizeNativeData = 0 };

  enum { kPaperSizeDefined = 1 };

  /**
   * Page Size Unit Constants
   */
  enum { kPaperSizeInches = 0 };

  enum { kPaperSizeMillimeters = 1 };

  /**
   * Orientation Constants
   */
  enum { kPortraitOrientation = 0 };

  enum { kLandscapeOrientation = 1 };

  /**
   * Print Frame Constants
   */
  enum { kNoFrames = 0 };

  enum { kFramesAsIs = 1 };

  enum { kSelectedFrame = 2 };

  enum { kEachFrameSep = 3 };

  /**
   * How to Enable Frame Set Printing Constants
   */
  enum { kFrameEnableNone = 0 };

  enum { kFrameEnableAll = 1 };

  enum { kFrameEnableAsIsAndEach = 2 };

  /**
   * Set PrintOptions 
   */
  /* void SetPrintOptions (in PRInt32 aType, in PRBool aTurnOnOff); */
  NS_IMETHOD SetPrintOptions(PRInt32 aType, PRBool aTurnOnOff) = 0;

  /**
   * Get PrintOptions 
   */
  /* PRBool GetPrintOptions (in PRInt32 aType); */
  NS_IMETHOD GetPrintOptions(PRInt32 aType, PRBool *_retval) = 0;

  /**
   * Set PrintOptions Bit field
   */
  /* PRInt32 GetPrintOptionsBits (); */
  NS_IMETHOD GetPrintOptionsBits(PRInt32 *_retval) = 0;

  /**
   * Returns W/H in Twips from Paper Size H/W
   */
  /* void GetPageSizeInTwips (out long aWidth, out long aHeight); */
  NS_IMETHOD GetPageSizeInTwips(PRInt32 *aWidth, PRInt32 *aHeight) = 0;

  /**
   * Makes a new copy
   */
  /* nsIPrintSettings clone (); */
  NS_IMETHOD Clone(nsIPrintSettings **_retval) = 0;

  /**
   * Assigns the internal values from the "in" arg to the current object
   */
  /* void assign (in nsIPrintSettings aPS); */
  NS_IMETHOD Assign(nsIPrintSettings *aPS) = 0;

  /**
   * Data Members
   */
  /* [noscript] attribute nsIPrintSession printSession; */
  NS_IMETHOD GetPrintSession(nsIPrintSession * *aPrintSession) = 0;
  NS_IMETHOD SetPrintSession(nsIPrintSession * aPrintSession) = 0;

  /* attribute long startPageRange; */
  NS_IMETHOD GetStartPageRange(PRInt32 *aStartPageRange) = 0;
  NS_IMETHOD SetStartPageRange(PRInt32 aStartPageRange) = 0;

  /* attribute long endPageRange; */
  NS_IMETHOD GetEndPageRange(PRInt32 *aEndPageRange) = 0;
  NS_IMETHOD SetEndPageRange(PRInt32 aEndPageRange) = 0;

  /* attribute double marginTop; */
  NS_IMETHOD GetMarginTop(double *aMarginTop) = 0;
  NS_IMETHOD SetMarginTop(double aMarginTop) = 0;

  /* attribute double marginLeft; */
  NS_IMETHOD GetMarginLeft(double *aMarginLeft) = 0;
  NS_IMETHOD SetMarginLeft(double aMarginLeft) = 0;

  /* attribute double marginBottom; */
  NS_IMETHOD GetMarginBottom(double *aMarginBottom) = 0;
  NS_IMETHOD SetMarginBottom(double aMarginBottom) = 0;

  /* attribute double marginRight; */
  NS_IMETHOD GetMarginRight(double *aMarginRight) = 0;
  NS_IMETHOD SetMarginRight(double aMarginRight) = 0;

  /* attribute double scaling; */
  NS_IMETHOD GetScaling(double *aScaling) = 0;
  NS_IMETHOD SetScaling(double aScaling) = 0;

  /* attribute boolean printBGColors; */
  NS_IMETHOD GetPrintBGColors(PRBool *aPrintBGColors) = 0;
  NS_IMETHOD SetPrintBGColors(PRBool aPrintBGColors) = 0;

  /* attribute boolean printBGImages; */
  NS_IMETHOD GetPrintBGImages(PRBool *aPrintBGImages) = 0;
  NS_IMETHOD SetPrintBGImages(PRBool aPrintBGImages) = 0;

  /* attribute short printRange; */
  NS_IMETHOD GetPrintRange(PRInt16 *aPrintRange) = 0;
  NS_IMETHOD SetPrintRange(PRInt16 aPrintRange) = 0;

  /* attribute wstring title; */
  NS_IMETHOD GetTitle(PRUnichar * *aTitle) = 0;
  NS_IMETHOD SetTitle(const PRUnichar * aTitle) = 0;

  /* attribute wstring docURL; */
  NS_IMETHOD GetDocURL(PRUnichar * *aDocURL) = 0;
  NS_IMETHOD SetDocURL(const PRUnichar * aDocURL) = 0;

  /* attribute wstring headerStrLeft; */
  NS_IMETHOD GetHeaderStrLeft(PRUnichar * *aHeaderStrLeft) = 0;
  NS_IMETHOD SetHeaderStrLeft(const PRUnichar * aHeaderStrLeft) = 0;

  /* attribute wstring headerStrCenter; */
  NS_IMETHOD GetHeaderStrCenter(PRUnichar * *aHeaderStrCenter) = 0;
  NS_IMETHOD SetHeaderStrCenter(const PRUnichar * aHeaderStrCenter) = 0;

  /* attribute wstring headerStrRight; */
  NS_IMETHOD GetHeaderStrRight(PRUnichar * *aHeaderStrRight) = 0;
  NS_IMETHOD SetHeaderStrRight(const PRUnichar * aHeaderStrRight) = 0;

  /* attribute wstring footerStrLeft; */
  NS_IMETHOD GetFooterStrLeft(PRUnichar * *aFooterStrLeft) = 0;
  NS_IMETHOD SetFooterStrLeft(const PRUnichar * aFooterStrLeft) = 0;

  /* attribute wstring footerStrCenter; */
  NS_IMETHOD GetFooterStrCenter(PRUnichar * *aFooterStrCenter) = 0;
  NS_IMETHOD SetFooterStrCenter(const PRUnichar * aFooterStrCenter) = 0;

  /* attribute wstring footerStrRight; */
  NS_IMETHOD GetFooterStrRight(PRUnichar * *aFooterStrRight) = 0;
  NS_IMETHOD SetFooterStrRight(const PRUnichar * aFooterStrRight) = 0;

  /* attribute short howToEnableFrameUI; */
  NS_IMETHOD GetHowToEnableFrameUI(PRInt16 *aHowToEnableFrameUI) = 0;
  NS_IMETHOD SetHowToEnableFrameUI(PRInt16 aHowToEnableFrameUI) = 0;

  /* attribute boolean isCancelled; */
  NS_IMETHOD GetIsCancelled(PRBool *aIsCancelled) = 0;
  NS_IMETHOD SetIsCancelled(PRBool aIsCancelled) = 0;

  /* attribute short printFrameTypeUsage; */
  NS_IMETHOD GetPrintFrameTypeUsage(PRInt16 *aPrintFrameTypeUsage) = 0;
  NS_IMETHOD SetPrintFrameTypeUsage(PRInt16 aPrintFrameTypeUsage) = 0;

  /* attribute short printFrameType; */
  NS_IMETHOD GetPrintFrameType(PRInt16 *aPrintFrameType) = 0;
  NS_IMETHOD SetPrintFrameType(PRInt16 aPrintFrameType) = 0;

  /* attribute boolean printSilent; */
  NS_IMETHOD GetPrintSilent(PRBool *aPrintSilent) = 0;
  NS_IMETHOD SetPrintSilent(PRBool aPrintSilent) = 0;

  /* attribute boolean shrinkToFit; */
  NS_IMETHOD GetShrinkToFit(PRBool *aShrinkToFit) = 0;
  NS_IMETHOD SetShrinkToFit(PRBool aShrinkToFit) = 0;

  /* attribute boolean showPrintProgress; */
  NS_IMETHOD GetShowPrintProgress(PRBool *aShowPrintProgress) = 0;
  NS_IMETHOD SetShowPrintProgress(PRBool aShowPrintProgress) = 0;

  /* attribute wstring paperName; */
  NS_IMETHOD GetPaperName(PRUnichar * *aPaperName) = 0;
  NS_IMETHOD SetPaperName(const PRUnichar * aPaperName) = 0;

  /* attribute short paperSizeType; */
  NS_IMETHOD GetPaperSizeType(PRInt16 *aPaperSizeType) = 0;
  NS_IMETHOD SetPaperSizeType(PRInt16 aPaperSizeType) = 0;

  /* attribute short paperData; */
  NS_IMETHOD GetPaperData(PRInt16 *aPaperData) = 0;
  NS_IMETHOD SetPaperData(PRInt16 aPaperData) = 0;

  /* attribute double paperWidth; */
  NS_IMETHOD GetPaperWidth(double *aPaperWidth) = 0;
  NS_IMETHOD SetPaperWidth(double aPaperWidth) = 0;

  /* attribute double paperHeight; */
  NS_IMETHOD GetPaperHeight(double *aPaperHeight) = 0;
  NS_IMETHOD SetPaperHeight(double aPaperHeight) = 0;

  /* attribute short paperSizeUnit; */
  NS_IMETHOD GetPaperSizeUnit(PRInt16 *aPaperSizeUnit) = 0;
  NS_IMETHOD SetPaperSizeUnit(PRInt16 aPaperSizeUnit) = 0;

  /* attribute wstring plexName; */
  NS_IMETHOD GetPlexName(PRUnichar * *aPlexName) = 0;
  NS_IMETHOD SetPlexName(const PRUnichar * aPlexName) = 0;

  /* attribute wstring colorspace; */
  NS_IMETHOD GetColorspace(PRUnichar * *aColorspace) = 0;
  NS_IMETHOD SetColorspace(const PRUnichar * aColorspace) = 0;

  /* attribute wstring resolutionName; */
  NS_IMETHOD GetResolutionName(PRUnichar * *aResolutionName) = 0;
  NS_IMETHOD SetResolutionName(const PRUnichar * aResolutionName) = 0;

  /* attribute boolean downloadFonts; */
  NS_IMETHOD GetDownloadFonts(PRBool *aDownloadFonts) = 0;
  NS_IMETHOD SetDownloadFonts(PRBool aDownloadFonts) = 0;

  /* attribute boolean printReversed; */
  NS_IMETHOD GetPrintReversed(PRBool *aPrintReversed) = 0;
  NS_IMETHOD SetPrintReversed(PRBool aPrintReversed) = 0;

  /* attribute boolean printInColor; */
  NS_IMETHOD GetPrintInColor(PRBool *aPrintInColor) = 0;
  NS_IMETHOD SetPrintInColor(PRBool aPrintInColor) = 0;

  /* attribute long paperSize; */
  NS_IMETHOD GetPaperSize(PRInt32 *aPaperSize) = 0;
  NS_IMETHOD SetPaperSize(PRInt32 aPaperSize) = 0;

  /* attribute long orientation; */
  NS_IMETHOD GetOrientation(PRInt32 *aOrientation) = 0;
  NS_IMETHOD SetOrientation(PRInt32 aOrientation) = 0;

  /* attribute wstring printCommand; */
  NS_IMETHOD GetPrintCommand(PRUnichar * *aPrintCommand) = 0;
  NS_IMETHOD SetPrintCommand(const PRUnichar * aPrintCommand) = 0;

  /* attribute long numCopies; */
  NS_IMETHOD GetNumCopies(PRInt32 *aNumCopies) = 0;
  NS_IMETHOD SetNumCopies(PRInt32 aNumCopies) = 0;

  /* attribute wstring printerName; */
  NS_IMETHOD GetPrinterName(PRUnichar * *aPrinterName) = 0;
  NS_IMETHOD SetPrinterName(const PRUnichar * aPrinterName) = 0;

  /* attribute boolean printToFile; */
  NS_IMETHOD GetPrintToFile(PRBool *aPrintToFile) = 0;
  NS_IMETHOD SetPrintToFile(PRBool aPrintToFile) = 0;

  /* attribute wstring toFileName; */
  NS_IMETHOD GetToFileName(PRUnichar * *aToFileName) = 0;
  NS_IMETHOD SetToFileName(const PRUnichar * aToFileName) = 0;

  /* attribute long printPageDelay; */
  NS_IMETHOD GetPrintPageDelay(PRInt32 *aPrintPageDelay) = 0;
  NS_IMETHOD SetPrintPageDelay(PRInt32 aPrintPageDelay) = 0;

  /**
   * This attribute tracks whether the PS has been initialized 
   * from a printer specified by the "printerName" attr. 
   * If a different name is set into the "printerName" 
   * attribute than the one it was initialized with the PS
   * will then get intialized from that printer.
   */
  /* attribute boolean isInitializedFromPrinter; */
  NS_IMETHOD GetIsInitializedFromPrinter(PRBool *aIsInitializedFromPrinter) = 0;
  NS_IMETHOD SetIsInitializedFromPrinter(PRBool aIsInitializedFromPrinter) = 0;

  /**
   * This attribute tracks whether the PS has been initialized 
   * from prefs. If a different name is set into the "printerName" 
   * attribute than the one it was initialized with the PS
   * will then get intialized from prefs again.
   */
  /* attribute boolean isInitializedFromPrefs; */
  NS_IMETHOD GetIsInitializedFromPrefs(PRBool *aIsInitializedFromPrefs) = 0;
  NS_IMETHOD SetIsInitializedFromPrefs(PRBool aIsInitializedFromPrefs) = 0;

  /* [noscript] void SetMarginInTwips (in nsNativeMarginRef aMargin); */
  NS_IMETHOD SetMarginInTwips(nsMargin & aMargin) = 0;

  /* [noscript] void GetMarginInTwips (in nsNativeMarginRef aMargin); */
  NS_IMETHOD GetMarginInTwips(nsMargin & aMargin) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPRINTSETTINGS \
  NS_IMETHOD SetPrintOptions(PRInt32 aType, PRBool aTurnOnOff); \
  NS_IMETHOD GetPrintOptions(PRInt32 aType, PRBool *_retval); \
  NS_IMETHOD GetPrintOptionsBits(PRInt32 *_retval); \
  NS_IMETHOD GetPageSizeInTwips(PRInt32 *aWidth, PRInt32 *aHeight); \
  NS_IMETHOD Clone(nsIPrintSettings **_retval); \
  NS_IMETHOD Assign(nsIPrintSettings *aPS); \
  NS_IMETHOD GetPrintSession(nsIPrintSession * *aPrintSession); \
  NS_IMETHOD SetPrintSession(nsIPrintSession * aPrintSession); \
  NS_IMETHOD GetStartPageRange(PRInt32 *aStartPageRange); \
  NS_IMETHOD SetStartPageRange(PRInt32 aStartPageRange); \
  NS_IMETHOD GetEndPageRange(PRInt32 *aEndPageRange); \
  NS_IMETHOD SetEndPageRange(PRInt32 aEndPageRange); \
  NS_IMETHOD GetMarginTop(double *aMarginTop); \
  NS_IMETHOD SetMarginTop(double aMarginTop); \
  NS_IMETHOD GetMarginLeft(double *aMarginLeft); \
  NS_IMETHOD SetMarginLeft(double aMarginLeft); \
  NS_IMETHOD GetMarginBottom(double *aMarginBottom); \
  NS_IMETHOD SetMarginBottom(double aMarginBottom); \
  NS_IMETHOD GetMarginRight(double *aMarginRight); \
  NS_IMETHOD SetMarginRight(double aMarginRight); \
  NS_IMETHOD GetScaling(double *aScaling); \
  NS_IMETHOD SetScaling(double aScaling); \
  NS_IMETHOD GetPrintBGColors(PRBool *aPrintBGColors); \
  NS_IMETHOD SetPrintBGColors(PRBool aPrintBGColors); \
  NS_IMETHOD GetPrintBGImages(PRBool *aPrintBGImages); \
  NS_IMETHOD SetPrintBGImages(PRBool aPrintBGImages); \
  NS_IMETHOD GetPrintRange(PRInt16 *aPrintRange); \
  NS_IMETHOD SetPrintRange(PRInt16 aPrintRange); \
  NS_IMETHOD GetTitle(PRUnichar * *aTitle); \
  NS_IMETHOD SetTitle(const PRUnichar * aTitle); \
  NS_IMETHOD GetDocURL(PRUnichar * *aDocURL); \
  NS_IMETHOD SetDocURL(const PRUnichar * aDocURL); \
  NS_IMETHOD GetHeaderStrLeft(PRUnichar * *aHeaderStrLeft); \
  NS_IMETHOD SetHeaderStrLeft(const PRUnichar * aHeaderStrLeft); \
  NS_IMETHOD GetHeaderStrCenter(PRUnichar * *aHeaderStrCenter); \
  NS_IMETHOD SetHeaderStrCenter(const PRUnichar * aHeaderStrCenter); \
  NS_IMETHOD GetHeaderStrRight(PRUnichar * *aHeaderStrRight); \
  NS_IMETHOD SetHeaderStrRight(const PRUnichar * aHeaderStrRight); \
  NS_IMETHOD GetFooterStrLeft(PRUnichar * *aFooterStrLeft); \
  NS_IMETHOD SetFooterStrLeft(const PRUnichar * aFooterStrLeft); \
  NS_IMETHOD GetFooterStrCenter(PRUnichar * *aFooterStrCenter); \
  NS_IMETHOD SetFooterStrCenter(const PRUnichar * aFooterStrCenter); \
  NS_IMETHOD GetFooterStrRight(PRUnichar * *aFooterStrRight); \
  NS_IMETHOD SetFooterStrRight(const PRUnichar * aFooterStrRight); \
  NS_IMETHOD GetHowToEnableFrameUI(PRInt16 *aHowToEnableFrameUI); \
  NS_IMETHOD SetHowToEnableFrameUI(PRInt16 aHowToEnableFrameUI); \
  NS_IMETHOD GetIsCancelled(PRBool *aIsCancelled); \
  NS_IMETHOD SetIsCancelled(PRBool aIsCancelled); \
  NS_IMETHOD GetPrintFrameTypeUsage(PRInt16 *aPrintFrameTypeUsage); \
  NS_IMETHOD SetPrintFrameTypeUsage(PRInt16 aPrintFrameTypeUsage); \
  NS_IMETHOD GetPrintFrameType(PRInt16 *aPrintFrameType); \
  NS_IMETHOD SetPrintFrameType(PRInt16 aPrintFrameType); \
  NS_IMETHOD GetPrintSilent(PRBool *aPrintSilent); \
  NS_IMETHOD SetPrintSilent(PRBool aPrintSilent); \
  NS_IMETHOD GetShrinkToFit(PRBool *aShrinkToFit); \
  NS_IMETHOD SetShrinkToFit(PRBool aShrinkToFit); \
  NS_IMETHOD GetShowPrintProgress(PRBool *aShowPrintProgress); \
  NS_IMETHOD SetShowPrintProgress(PRBool aShowPrintProgress); \
  NS_IMETHOD GetPaperName(PRUnichar * *aPaperName); \
  NS_IMETHOD SetPaperName(const PRUnichar * aPaperName); \
  NS_IMETHOD GetPaperSizeType(PRInt16 *aPaperSizeType); \
  NS_IMETHOD SetPaperSizeType(PRInt16 aPaperSizeType); \
  NS_IMETHOD GetPaperData(PRInt16 *aPaperData); \
  NS_IMETHOD SetPaperData(PRInt16 aPaperData); \
  NS_IMETHOD GetPaperWidth(double *aPaperWidth); \
  NS_IMETHOD SetPaperWidth(double aPaperWidth); \
  NS_IMETHOD GetPaperHeight(double *aPaperHeight); \
  NS_IMETHOD SetPaperHeight(double aPaperHeight); \
  NS_IMETHOD GetPaperSizeUnit(PRInt16 *aPaperSizeUnit); \
  NS_IMETHOD SetPaperSizeUnit(PRInt16 aPaperSizeUnit); \
  NS_IMETHOD GetPlexName(PRUnichar * *aPlexName); \
  NS_IMETHOD SetPlexName(const PRUnichar * aPlexName); \
  NS_IMETHOD GetColorspace(PRUnichar * *aColorspace); \
  NS_IMETHOD SetColorspace(const PRUnichar * aColorspace); \
  NS_IMETHOD GetResolutionName(PRUnichar * *aResolutionName); \
  NS_IMETHOD SetResolutionName(const PRUnichar * aResolutionName); \
  NS_IMETHOD GetDownloadFonts(PRBool *aDownloadFonts); \
  NS_IMETHOD SetDownloadFonts(PRBool aDownloadFonts); \
  NS_IMETHOD GetPrintReversed(PRBool *aPrintReversed); \
  NS_IMETHOD SetPrintReversed(PRBool aPrintReversed); \
  NS_IMETHOD GetPrintInColor(PRBool *aPrintInColor); \
  NS_IMETHOD SetPrintInColor(PRBool aPrintInColor); \
  NS_IMETHOD GetPaperSize(PRInt32 *aPaperSize); \
  NS_IMETHOD SetPaperSize(PRInt32 aPaperSize); \
  NS_IMETHOD GetOrientation(PRInt32 *aOrientation); \
  NS_IMETHOD SetOrientation(PRInt32 aOrientation); \
  NS_IMETHOD GetPrintCommand(PRUnichar * *aPrintCommand); \
  NS_IMETHOD SetPrintCommand(const PRUnichar * aPrintCommand); \
  NS_IMETHOD GetNumCopies(PRInt32 *aNumCopies); \
  NS_IMETHOD SetNumCopies(PRInt32 aNumCopies); \
  NS_IMETHOD GetPrinterName(PRUnichar * *aPrinterName); \
  NS_IMETHOD SetPrinterName(const PRUnichar * aPrinterName); \
  NS_IMETHOD GetPrintToFile(PRBool *aPrintToFile); \
  NS_IMETHOD SetPrintToFile(PRBool aPrintToFile); \
  NS_IMETHOD GetToFileName(PRUnichar * *aToFileName); \
  NS_IMETHOD SetToFileName(const PRUnichar * aToFileName); \
  NS_IMETHOD GetPrintPageDelay(PRInt32 *aPrintPageDelay); \
  NS_IMETHOD SetPrintPageDelay(PRInt32 aPrintPageDelay); \
  NS_IMETHOD GetIsInitializedFromPrinter(PRBool *aIsInitializedFromPrinter); \
  NS_IMETHOD SetIsInitializedFromPrinter(PRBool aIsInitializedFromPrinter); \
  NS_IMETHOD GetIsInitializedFromPrefs(PRBool *aIsInitializedFromPrefs); \
  NS_IMETHOD SetIsInitializedFromPrefs(PRBool aIsInitializedFromPrefs); \
  NS_IMETHOD SetMarginInTwips(nsMargin & aMargin); \
  NS_IMETHOD GetMarginInTwips(nsMargin & aMargin); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPRINTSETTINGS(_to) \
  NS_IMETHOD SetPrintOptions(PRInt32 aType, PRBool aTurnOnOff) { return _to SetPrintOptions(aType, aTurnOnOff); } \
  NS_IMETHOD GetPrintOptions(PRInt32 aType, PRBool *_retval) { return _to GetPrintOptions(aType, _retval); } \
  NS_IMETHOD GetPrintOptionsBits(PRInt32 *_retval) { return _to GetPrintOptionsBits(_retval); } \
  NS_IMETHOD GetPageSizeInTwips(PRInt32 *aWidth, PRInt32 *aHeight) { return _to GetPageSizeInTwips(aWidth, aHeight); } \
  NS_IMETHOD Clone(nsIPrintSettings **_retval) { return _to Clone(_retval); } \
  NS_IMETHOD Assign(nsIPrintSettings *aPS) { return _to Assign(aPS); } \
  NS_IMETHOD GetPrintSession(nsIPrintSession * *aPrintSession) { return _to GetPrintSession(aPrintSession); } \
  NS_IMETHOD SetPrintSession(nsIPrintSession * aPrintSession) { return _to SetPrintSession(aPrintSession); } \
  NS_IMETHOD GetStartPageRange(PRInt32 *aStartPageRange) { return _to GetStartPageRange(aStartPageRange); } \
  NS_IMETHOD SetStartPageRange(PRInt32 aStartPageRange) { return _to SetStartPageRange(aStartPageRange); } \
  NS_IMETHOD GetEndPageRange(PRInt32 *aEndPageRange) { return _to GetEndPageRange(aEndPageRange); } \
  NS_IMETHOD SetEndPageRange(PRInt32 aEndPageRange) { return _to SetEndPageRange(aEndPageRange); } \
  NS_IMETHOD GetMarginTop(double *aMarginTop) { return _to GetMarginTop(aMarginTop); } \
  NS_IMETHOD SetMarginTop(double aMarginTop) { return _to SetMarginTop(aMarginTop); } \
  NS_IMETHOD GetMarginLeft(double *aMarginLeft) { return _to GetMarginLeft(aMarginLeft); } \
  NS_IMETHOD SetMarginLeft(double aMarginLeft) { return _to SetMarginLeft(aMarginLeft); } \
  NS_IMETHOD GetMarginBottom(double *aMarginBottom) { return _to GetMarginBottom(aMarginBottom); } \
  NS_IMETHOD SetMarginBottom(double aMarginBottom) { return _to SetMarginBottom(aMarginBottom); } \
  NS_IMETHOD GetMarginRight(double *aMarginRight) { return _to GetMarginRight(aMarginRight); } \
  NS_IMETHOD SetMarginRight(double aMarginRight) { return _to SetMarginRight(aMarginRight); } \
  NS_IMETHOD GetScaling(double *aScaling) { return _to GetScaling(aScaling); } \
  NS_IMETHOD SetScaling(double aScaling) { return _to SetScaling(aScaling); } \
  NS_IMETHOD GetPrintBGColors(PRBool *aPrintBGColors) { return _to GetPrintBGColors(aPrintBGColors); } \
  NS_IMETHOD SetPrintBGColors(PRBool aPrintBGColors) { return _to SetPrintBGColors(aPrintBGColors); } \
  NS_IMETHOD GetPrintBGImages(PRBool *aPrintBGImages) { return _to GetPrintBGImages(aPrintBGImages); } \
  NS_IMETHOD SetPrintBGImages(PRBool aPrintBGImages) { return _to SetPrintBGImages(aPrintBGImages); } \
  NS_IMETHOD GetPrintRange(PRInt16 *aPrintRange) { return _to GetPrintRange(aPrintRange); } \
  NS_IMETHOD SetPrintRange(PRInt16 aPrintRange) { return _to SetPrintRange(aPrintRange); } \
  NS_IMETHOD GetTitle(PRUnichar * *aTitle) { return _to GetTitle(aTitle); } \
  NS_IMETHOD SetTitle(const PRUnichar * aTitle) { return _to SetTitle(aTitle); } \
  NS_IMETHOD GetDocURL(PRUnichar * *aDocURL) { return _to GetDocURL(aDocURL); } \
  NS_IMETHOD SetDocURL(const PRUnichar * aDocURL) { return _to SetDocURL(aDocURL); } \
  NS_IMETHOD GetHeaderStrLeft(PRUnichar * *aHeaderStrLeft) { return _to GetHeaderStrLeft(aHeaderStrLeft); } \
  NS_IMETHOD SetHeaderStrLeft(const PRUnichar * aHeaderStrLeft) { return _to SetHeaderStrLeft(aHeaderStrLeft); } \
  NS_IMETHOD GetHeaderStrCenter(PRUnichar * *aHeaderStrCenter) { return _to GetHeaderStrCenter(aHeaderStrCenter); } \
  NS_IMETHOD SetHeaderStrCenter(const PRUnichar * aHeaderStrCenter) { return _to SetHeaderStrCenter(aHeaderStrCenter); } \
  NS_IMETHOD GetHeaderStrRight(PRUnichar * *aHeaderStrRight) { return _to GetHeaderStrRight(aHeaderStrRight); } \
  NS_IMETHOD SetHeaderStrRight(const PRUnichar * aHeaderStrRight) { return _to SetHeaderStrRight(aHeaderStrRight); } \
  NS_IMETHOD GetFooterStrLeft(PRUnichar * *aFooterStrLeft) { return _to GetFooterStrLeft(aFooterStrLeft); } \
  NS_IMETHOD SetFooterStrLeft(const PRUnichar * aFooterStrLeft) { return _to SetFooterStrLeft(aFooterStrLeft); } \
  NS_IMETHOD GetFooterStrCenter(PRUnichar * *aFooterStrCenter) { return _to GetFooterStrCenter(aFooterStrCenter); } \
  NS_IMETHOD SetFooterStrCenter(const PRUnichar * aFooterStrCenter) { return _to SetFooterStrCenter(aFooterStrCenter); } \
  NS_IMETHOD GetFooterStrRight(PRUnichar * *aFooterStrRight) { return _to GetFooterStrRight(aFooterStrRight); } \
  NS_IMETHOD SetFooterStrRight(const PRUnichar * aFooterStrRight) { return _to SetFooterStrRight(aFooterStrRight); } \
  NS_IMETHOD GetHowToEnableFrameUI(PRInt16 *aHowToEnableFrameUI) { return _to GetHowToEnableFrameUI(aHowToEnableFrameUI); } \
  NS_IMETHOD SetHowToEnableFrameUI(PRInt16 aHowToEnableFrameUI) { return _to SetHowToEnableFrameUI(aHowToEnableFrameUI); } \
  NS_IMETHOD GetIsCancelled(PRBool *aIsCancelled) { return _to GetIsCancelled(aIsCancelled); } \
  NS_IMETHOD SetIsCancelled(PRBool aIsCancelled) { return _to SetIsCancelled(aIsCancelled); } \
  NS_IMETHOD GetPrintFrameTypeUsage(PRInt16 *aPrintFrameTypeUsage) { return _to GetPrintFrameTypeUsage(aPrintFrameTypeUsage); } \
  NS_IMETHOD SetPrintFrameTypeUsage(PRInt16 aPrintFrameTypeUsage) { return _to SetPrintFrameTypeUsage(aPrintFrameTypeUsage); } \
  NS_IMETHOD GetPrintFrameType(PRInt16 *aPrintFrameType) { return _to GetPrintFrameType(aPrintFrameType); } \
  NS_IMETHOD SetPrintFrameType(PRInt16 aPrintFrameType) { return _to SetPrintFrameType(aPrintFrameType); } \
  NS_IMETHOD GetPrintSilent(PRBool *aPrintSilent) { return _to GetPrintSilent(aPrintSilent); } \
  NS_IMETHOD SetPrintSilent(PRBool aPrintSilent) { return _to SetPrintSilent(aPrintSilent); } \
  NS_IMETHOD GetShrinkToFit(PRBool *aShrinkToFit) { return _to GetShrinkToFit(aShrinkToFit); } \
  NS_IMETHOD SetShrinkToFit(PRBool aShrinkToFit) { return _to SetShrinkToFit(aShrinkToFit); } \
  NS_IMETHOD GetShowPrintProgress(PRBool *aShowPrintProgress) { return _to GetShowPrintProgress(aShowPrintProgress); } \
  NS_IMETHOD SetShowPrintProgress(PRBool aShowPrintProgress) { return _to SetShowPrintProgress(aShowPrintProgress); } \
  NS_IMETHOD GetPaperName(PRUnichar * *aPaperName) { return _to GetPaperName(aPaperName); } \
  NS_IMETHOD SetPaperName(const PRUnichar * aPaperName) { return _to SetPaperName(aPaperName); } \
  NS_IMETHOD GetPaperSizeType(PRInt16 *aPaperSizeType) { return _to GetPaperSizeType(aPaperSizeType); } \
  NS_IMETHOD SetPaperSizeType(PRInt16 aPaperSizeType) { return _to SetPaperSizeType(aPaperSizeType); } \
  NS_IMETHOD GetPaperData(PRInt16 *aPaperData) { return _to GetPaperData(aPaperData); } \
  NS_IMETHOD SetPaperData(PRInt16 aPaperData) { return _to SetPaperData(aPaperData); } \
  NS_IMETHOD GetPaperWidth(double *aPaperWidth) { return _to GetPaperWidth(aPaperWidth); } \
  NS_IMETHOD SetPaperWidth(double aPaperWidth) { return _to SetPaperWidth(aPaperWidth); } \
  NS_IMETHOD GetPaperHeight(double *aPaperHeight) { return _to GetPaperHeight(aPaperHeight); } \
  NS_IMETHOD SetPaperHeight(double aPaperHeight) { return _to SetPaperHeight(aPaperHeight); } \
  NS_IMETHOD GetPaperSizeUnit(PRInt16 *aPaperSizeUnit) { return _to GetPaperSizeUnit(aPaperSizeUnit); } \
  NS_IMETHOD SetPaperSizeUnit(PRInt16 aPaperSizeUnit) { return _to SetPaperSizeUnit(aPaperSizeUnit); } \
  NS_IMETHOD GetPlexName(PRUnichar * *aPlexName) { return _to GetPlexName(aPlexName); } \
  NS_IMETHOD SetPlexName(const PRUnichar * aPlexName) { return _to SetPlexName(aPlexName); } \
  NS_IMETHOD GetColorspace(PRUnichar * *aColorspace) { return _to GetColorspace(aColorspace); } \
  NS_IMETHOD SetColorspace(const PRUnichar * aColorspace) { return _to SetColorspace(aColorspace); } \
  NS_IMETHOD GetResolutionName(PRUnichar * *aResolutionName) { return _to GetResolutionName(aResolutionName); } \
  NS_IMETHOD SetResolutionName(const PRUnichar * aResolutionName) { return _to SetResolutionName(aResolutionName); } \
  NS_IMETHOD GetDownloadFonts(PRBool *aDownloadFonts) { return _to GetDownloadFonts(aDownloadFonts); } \
  NS_IMETHOD SetDownloadFonts(PRBool aDownloadFonts) { return _to SetDownloadFonts(aDownloadFonts); } \
  NS_IMETHOD GetPrintReversed(PRBool *aPrintReversed) { return _to GetPrintReversed(aPrintReversed); } \
  NS_IMETHOD SetPrintReversed(PRBool aPrintReversed) { return _to SetPrintReversed(aPrintReversed); } \
  NS_IMETHOD GetPrintInColor(PRBool *aPrintInColor) { return _to GetPrintInColor(aPrintInColor); } \
  NS_IMETHOD SetPrintInColor(PRBool aPrintInColor) { return _to SetPrintInColor(aPrintInColor); } \
  NS_IMETHOD GetPaperSize(PRInt32 *aPaperSize) { return _to GetPaperSize(aPaperSize); } \
  NS_IMETHOD SetPaperSize(PRInt32 aPaperSize) { return _to SetPaperSize(aPaperSize); } \
  NS_IMETHOD GetOrientation(PRInt32 *aOrientation) { return _to GetOrientation(aOrientation); } \
  NS_IMETHOD SetOrientation(PRInt32 aOrientation) { return _to SetOrientation(aOrientation); } \
  NS_IMETHOD GetPrintCommand(PRUnichar * *aPrintCommand) { return _to GetPrintCommand(aPrintCommand); } \
  NS_IMETHOD SetPrintCommand(const PRUnichar * aPrintCommand) { return _to SetPrintCommand(aPrintCommand); } \
  NS_IMETHOD GetNumCopies(PRInt32 *aNumCopies) { return _to GetNumCopies(aNumCopies); } \
  NS_IMETHOD SetNumCopies(PRInt32 aNumCopies) { return _to SetNumCopies(aNumCopies); } \
  NS_IMETHOD GetPrinterName(PRUnichar * *aPrinterName) { return _to GetPrinterName(aPrinterName); } \
  NS_IMETHOD SetPrinterName(const PRUnichar * aPrinterName) { return _to SetPrinterName(aPrinterName); } \
  NS_IMETHOD GetPrintToFile(PRBool *aPrintToFile) { return _to GetPrintToFile(aPrintToFile); } \
  NS_IMETHOD SetPrintToFile(PRBool aPrintToFile) { return _to SetPrintToFile(aPrintToFile); } \
  NS_IMETHOD GetToFileName(PRUnichar * *aToFileName) { return _to GetToFileName(aToFileName); } \
  NS_IMETHOD SetToFileName(const PRUnichar * aToFileName) { return _to SetToFileName(aToFileName); } \
  NS_IMETHOD GetPrintPageDelay(PRInt32 *aPrintPageDelay) { return _to GetPrintPageDelay(aPrintPageDelay); } \
  NS_IMETHOD SetPrintPageDelay(PRInt32 aPrintPageDelay) { return _to SetPrintPageDelay(aPrintPageDelay); } \
  NS_IMETHOD GetIsInitializedFromPrinter(PRBool *aIsInitializedFromPrinter) { return _to GetIsInitializedFromPrinter(aIsInitializedFromPrinter); } \
  NS_IMETHOD SetIsInitializedFromPrinter(PRBool aIsInitializedFromPrinter) { return _to SetIsInitializedFromPrinter(aIsInitializedFromPrinter); } \
  NS_IMETHOD GetIsInitializedFromPrefs(PRBool *aIsInitializedFromPrefs) { return _to GetIsInitializedFromPrefs(aIsInitializedFromPrefs); } \
  NS_IMETHOD SetIsInitializedFromPrefs(PRBool aIsInitializedFromPrefs) { return _to SetIsInitializedFromPrefs(aIsInitializedFromPrefs); } \
  NS_IMETHOD SetMarginInTwips(nsMargin & aMargin) { return _to SetMarginInTwips(aMargin); } \
  NS_IMETHOD GetMarginInTwips(nsMargin & aMargin) { return _to GetMarginInTwips(aMargin); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPRINTSETTINGS(_to) \
  NS_IMETHOD SetPrintOptions(PRInt32 aType, PRBool aTurnOnOff) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPrintOptions(aType, aTurnOnOff); } \
  NS_IMETHOD GetPrintOptions(PRInt32 aType, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrintOptions(aType, _retval); } \
  NS_IMETHOD GetPrintOptionsBits(PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrintOptionsBits(_retval); } \
  NS_IMETHOD GetPageSizeInTwips(PRInt32 *aWidth, PRInt32 *aHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPageSizeInTwips(aWidth, aHeight); } \
  NS_IMETHOD Clone(nsIPrintSettings **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Clone(_retval); } \
  NS_IMETHOD Assign(nsIPrintSettings *aPS) { return !_to ? NS_ERROR_NULL_POINTER : _to->Assign(aPS); } \
  NS_IMETHOD GetPrintSession(nsIPrintSession * *aPrintSession) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrintSession(aPrintSession); } \
  NS_IMETHOD SetPrintSession(nsIPrintSession * aPrintSession) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPrintSession(aPrintSession); } \
  NS_IMETHOD GetStartPageRange(PRInt32 *aStartPageRange) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStartPageRange(aStartPageRange); } \
  NS_IMETHOD SetStartPageRange(PRInt32 aStartPageRange) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetStartPageRange(aStartPageRange); } \
  NS_IMETHOD GetEndPageRange(PRInt32 *aEndPageRange) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetEndPageRange(aEndPageRange); } \
  NS_IMETHOD SetEndPageRange(PRInt32 aEndPageRange) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetEndPageRange(aEndPageRange); } \
  NS_IMETHOD GetMarginTop(double *aMarginTop) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMarginTop(aMarginTop); } \
  NS_IMETHOD SetMarginTop(double aMarginTop) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMarginTop(aMarginTop); } \
  NS_IMETHOD GetMarginLeft(double *aMarginLeft) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMarginLeft(aMarginLeft); } \
  NS_IMETHOD SetMarginLeft(double aMarginLeft) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMarginLeft(aMarginLeft); } \
  NS_IMETHOD GetMarginBottom(double *aMarginBottom) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMarginBottom(aMarginBottom); } \
  NS_IMETHOD SetMarginBottom(double aMarginBottom) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMarginBottom(aMarginBottom); } \
  NS_IMETHOD GetMarginRight(double *aMarginRight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMarginRight(aMarginRight); } \
  NS_IMETHOD SetMarginRight(double aMarginRight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMarginRight(aMarginRight); } \
  NS_IMETHOD GetScaling(double *aScaling) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScaling(aScaling); } \
  NS_IMETHOD SetScaling(double aScaling) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetScaling(aScaling); } \
  NS_IMETHOD GetPrintBGColors(PRBool *aPrintBGColors) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrintBGColors(aPrintBGColors); } \
  NS_IMETHOD SetPrintBGColors(PRBool aPrintBGColors) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPrintBGColors(aPrintBGColors); } \
  NS_IMETHOD GetPrintBGImages(PRBool *aPrintBGImages) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrintBGImages(aPrintBGImages); } \
  NS_IMETHOD SetPrintBGImages(PRBool aPrintBGImages) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPrintBGImages(aPrintBGImages); } \
  NS_IMETHOD GetPrintRange(PRInt16 *aPrintRange) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrintRange(aPrintRange); } \
  NS_IMETHOD SetPrintRange(PRInt16 aPrintRange) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPrintRange(aPrintRange); } \
  NS_IMETHOD GetTitle(PRUnichar * *aTitle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTitle(aTitle); } \
  NS_IMETHOD SetTitle(const PRUnichar * aTitle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTitle(aTitle); } \
  NS_IMETHOD GetDocURL(PRUnichar * *aDocURL) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDocURL(aDocURL); } \
  NS_IMETHOD SetDocURL(const PRUnichar * aDocURL) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDocURL(aDocURL); } \
  NS_IMETHOD GetHeaderStrLeft(PRUnichar * *aHeaderStrLeft) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHeaderStrLeft(aHeaderStrLeft); } \
  NS_IMETHOD SetHeaderStrLeft(const PRUnichar * aHeaderStrLeft) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetHeaderStrLeft(aHeaderStrLeft); } \
  NS_IMETHOD GetHeaderStrCenter(PRUnichar * *aHeaderStrCenter) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHeaderStrCenter(aHeaderStrCenter); } \
  NS_IMETHOD SetHeaderStrCenter(const PRUnichar * aHeaderStrCenter) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetHeaderStrCenter(aHeaderStrCenter); } \
  NS_IMETHOD GetHeaderStrRight(PRUnichar * *aHeaderStrRight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHeaderStrRight(aHeaderStrRight); } \
  NS_IMETHOD SetHeaderStrRight(const PRUnichar * aHeaderStrRight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetHeaderStrRight(aHeaderStrRight); } \
  NS_IMETHOD GetFooterStrLeft(PRUnichar * *aFooterStrLeft) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFooterStrLeft(aFooterStrLeft); } \
  NS_IMETHOD SetFooterStrLeft(const PRUnichar * aFooterStrLeft) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFooterStrLeft(aFooterStrLeft); } \
  NS_IMETHOD GetFooterStrCenter(PRUnichar * *aFooterStrCenter) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFooterStrCenter(aFooterStrCenter); } \
  NS_IMETHOD SetFooterStrCenter(const PRUnichar * aFooterStrCenter) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFooterStrCenter(aFooterStrCenter); } \
  NS_IMETHOD GetFooterStrRight(PRUnichar * *aFooterStrRight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFooterStrRight(aFooterStrRight); } \
  NS_IMETHOD SetFooterStrRight(const PRUnichar * aFooterStrRight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFooterStrRight(aFooterStrRight); } \
  NS_IMETHOD GetHowToEnableFrameUI(PRInt16 *aHowToEnableFrameUI) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHowToEnableFrameUI(aHowToEnableFrameUI); } \
  NS_IMETHOD SetHowToEnableFrameUI(PRInt16 aHowToEnableFrameUI) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetHowToEnableFrameUI(aHowToEnableFrameUI); } \
  NS_IMETHOD GetIsCancelled(PRBool *aIsCancelled) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIsCancelled(aIsCancelled); } \
  NS_IMETHOD SetIsCancelled(PRBool aIsCancelled) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetIsCancelled(aIsCancelled); } \
  NS_IMETHOD GetPrintFrameTypeUsage(PRInt16 *aPrintFrameTypeUsage) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrintFrameTypeUsage(aPrintFrameTypeUsage); } \
  NS_IMETHOD SetPrintFrameTypeUsage(PRInt16 aPrintFrameTypeUsage) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPrintFrameTypeUsage(aPrintFrameTypeUsage); } \
  NS_IMETHOD GetPrintFrameType(PRInt16 *aPrintFrameType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrintFrameType(aPrintFrameType); } \
  NS_IMETHOD SetPrintFrameType(PRInt16 aPrintFrameType) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPrintFrameType(aPrintFrameType); } \
  NS_IMETHOD GetPrintSilent(PRBool *aPrintSilent) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrintSilent(aPrintSilent); } \
  NS_IMETHOD SetPrintSilent(PRBool aPrintSilent) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPrintSilent(aPrintSilent); } \
  NS_IMETHOD GetShrinkToFit(PRBool *aShrinkToFit) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetShrinkToFit(aShrinkToFit); } \
  NS_IMETHOD SetShrinkToFit(PRBool aShrinkToFit) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetShrinkToFit(aShrinkToFit); } \
  NS_IMETHOD GetShowPrintProgress(PRBool *aShowPrintProgress) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetShowPrintProgress(aShowPrintProgress); } \
  NS_IMETHOD SetShowPrintProgress(PRBool aShowPrintProgress) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetShowPrintProgress(aShowPrintProgress); } \
  NS_IMETHOD GetPaperName(PRUnichar * *aPaperName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPaperName(aPaperName); } \
  NS_IMETHOD SetPaperName(const PRUnichar * aPaperName) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPaperName(aPaperName); } \
  NS_IMETHOD GetPaperSizeType(PRInt16 *aPaperSizeType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPaperSizeType(aPaperSizeType); } \
  NS_IMETHOD SetPaperSizeType(PRInt16 aPaperSizeType) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPaperSizeType(aPaperSizeType); } \
  NS_IMETHOD GetPaperData(PRInt16 *aPaperData) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPaperData(aPaperData); } \
  NS_IMETHOD SetPaperData(PRInt16 aPaperData) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPaperData(aPaperData); } \
  NS_IMETHOD GetPaperWidth(double *aPaperWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPaperWidth(aPaperWidth); } \
  NS_IMETHOD SetPaperWidth(double aPaperWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPaperWidth(aPaperWidth); } \
  NS_IMETHOD GetPaperHeight(double *aPaperHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPaperHeight(aPaperHeight); } \
  NS_IMETHOD SetPaperHeight(double aPaperHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPaperHeight(aPaperHeight); } \
  NS_IMETHOD GetPaperSizeUnit(PRInt16 *aPaperSizeUnit) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPaperSizeUnit(aPaperSizeUnit); } \
  NS_IMETHOD SetPaperSizeUnit(PRInt16 aPaperSizeUnit) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPaperSizeUnit(aPaperSizeUnit); } \
  NS_IMETHOD GetPlexName(PRUnichar * *aPlexName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPlexName(aPlexName); } \
  NS_IMETHOD SetPlexName(const PRUnichar * aPlexName) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPlexName(aPlexName); } \
  NS_IMETHOD GetColorspace(PRUnichar * *aColorspace) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetColorspace(aColorspace); } \
  NS_IMETHOD SetColorspace(const PRUnichar * aColorspace) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetColorspace(aColorspace); } \
  NS_IMETHOD GetResolutionName(PRUnichar * *aResolutionName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetResolutionName(aResolutionName); } \
  NS_IMETHOD SetResolutionName(const PRUnichar * aResolutionName) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetResolutionName(aResolutionName); } \
  NS_IMETHOD GetDownloadFonts(PRBool *aDownloadFonts) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDownloadFonts(aDownloadFonts); } \
  NS_IMETHOD SetDownloadFonts(PRBool aDownloadFonts) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDownloadFonts(aDownloadFonts); } \
  NS_IMETHOD GetPrintReversed(PRBool *aPrintReversed) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrintReversed(aPrintReversed); } \
  NS_IMETHOD SetPrintReversed(PRBool aPrintReversed) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPrintReversed(aPrintReversed); } \
  NS_IMETHOD GetPrintInColor(PRBool *aPrintInColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrintInColor(aPrintInColor); } \
  NS_IMETHOD SetPrintInColor(PRBool aPrintInColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPrintInColor(aPrintInColor); } \
  NS_IMETHOD GetPaperSize(PRInt32 *aPaperSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPaperSize(aPaperSize); } \
  NS_IMETHOD SetPaperSize(PRInt32 aPaperSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPaperSize(aPaperSize); } \
  NS_IMETHOD GetOrientation(PRInt32 *aOrientation) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOrientation(aOrientation); } \
  NS_IMETHOD SetOrientation(PRInt32 aOrientation) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOrientation(aOrientation); } \
  NS_IMETHOD GetPrintCommand(PRUnichar * *aPrintCommand) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrintCommand(aPrintCommand); } \
  NS_IMETHOD SetPrintCommand(const PRUnichar * aPrintCommand) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPrintCommand(aPrintCommand); } \
  NS_IMETHOD GetNumCopies(PRInt32 *aNumCopies) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNumCopies(aNumCopies); } \
  NS_IMETHOD SetNumCopies(PRInt32 aNumCopies) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetNumCopies(aNumCopies); } \
  NS_IMETHOD GetPrinterName(PRUnichar * *aPrinterName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrinterName(aPrinterName); } \
  NS_IMETHOD SetPrinterName(const PRUnichar * aPrinterName) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPrinterName(aPrinterName); } \
  NS_IMETHOD GetPrintToFile(PRBool *aPrintToFile) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrintToFile(aPrintToFile); } \
  NS_IMETHOD SetPrintToFile(PRBool aPrintToFile) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPrintToFile(aPrintToFile); } \
  NS_IMETHOD GetToFileName(PRUnichar * *aToFileName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetToFileName(aToFileName); } \
  NS_IMETHOD SetToFileName(const PRUnichar * aToFileName) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetToFileName(aToFileName); } \
  NS_IMETHOD GetPrintPageDelay(PRInt32 *aPrintPageDelay) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrintPageDelay(aPrintPageDelay); } \
  NS_IMETHOD SetPrintPageDelay(PRInt32 aPrintPageDelay) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPrintPageDelay(aPrintPageDelay); } \
  NS_IMETHOD GetIsInitializedFromPrinter(PRBool *aIsInitializedFromPrinter) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIsInitializedFromPrinter(aIsInitializedFromPrinter); } \
  NS_IMETHOD SetIsInitializedFromPrinter(PRBool aIsInitializedFromPrinter) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetIsInitializedFromPrinter(aIsInitializedFromPrinter); } \
  NS_IMETHOD GetIsInitializedFromPrefs(PRBool *aIsInitializedFromPrefs) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIsInitializedFromPrefs(aIsInitializedFromPrefs); } \
  NS_IMETHOD SetIsInitializedFromPrefs(PRBool aIsInitializedFromPrefs) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetIsInitializedFromPrefs(aIsInitializedFromPrefs); } \
  NS_IMETHOD SetMarginInTwips(nsMargin & aMargin) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMarginInTwips(aMargin); } \
  NS_IMETHOD GetMarginInTwips(nsMargin & aMargin) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMarginInTwips(aMargin); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsPrintSettings : public nsIPrintSettings
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPRINTSETTINGS

  nsPrintSettings();

private:
  ~nsPrintSettings();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsPrintSettings, nsIPrintSettings)

nsPrintSettings::nsPrintSettings()
{
  /* member initializers and constructor code */
}

nsPrintSettings::~nsPrintSettings()
{
  /* destructor code */
}

/* void SetPrintOptions (in PRInt32 aType, in PRBool aTurnOnOff); */
NS_IMETHODIMP nsPrintSettings::SetPrintOptions(PRInt32 aType, PRBool aTurnOnOff)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* PRBool GetPrintOptions (in PRInt32 aType); */
NS_IMETHODIMP nsPrintSettings::GetPrintOptions(PRInt32 aType, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* PRInt32 GetPrintOptionsBits (); */
NS_IMETHODIMP nsPrintSettings::GetPrintOptionsBits(PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void GetPageSizeInTwips (out long aWidth, out long aHeight); */
NS_IMETHODIMP nsPrintSettings::GetPageSizeInTwips(PRInt32 *aWidth, PRInt32 *aHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIPrintSettings clone (); */
NS_IMETHODIMP nsPrintSettings::Clone(nsIPrintSettings **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void assign (in nsIPrintSettings aPS); */
NS_IMETHODIMP nsPrintSettings::Assign(nsIPrintSettings *aPS)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] attribute nsIPrintSession printSession; */
NS_IMETHODIMP nsPrintSettings::GetPrintSession(nsIPrintSession * *aPrintSession)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPrintSession(nsIPrintSession * aPrintSession)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long startPageRange; */
NS_IMETHODIMP nsPrintSettings::GetStartPageRange(PRInt32 *aStartPageRange)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetStartPageRange(PRInt32 aStartPageRange)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long endPageRange; */
NS_IMETHODIMP nsPrintSettings::GetEndPageRange(PRInt32 *aEndPageRange)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetEndPageRange(PRInt32 aEndPageRange)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute double marginTop; */
NS_IMETHODIMP nsPrintSettings::GetMarginTop(double *aMarginTop)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetMarginTop(double aMarginTop)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute double marginLeft; */
NS_IMETHODIMP nsPrintSettings::GetMarginLeft(double *aMarginLeft)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetMarginLeft(double aMarginLeft)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute double marginBottom; */
NS_IMETHODIMP nsPrintSettings::GetMarginBottom(double *aMarginBottom)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetMarginBottom(double aMarginBottom)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute double marginRight; */
NS_IMETHODIMP nsPrintSettings::GetMarginRight(double *aMarginRight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetMarginRight(double aMarginRight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute double scaling; */
NS_IMETHODIMP nsPrintSettings::GetScaling(double *aScaling)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetScaling(double aScaling)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean printBGColors; */
NS_IMETHODIMP nsPrintSettings::GetPrintBGColors(PRBool *aPrintBGColors)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPrintBGColors(PRBool aPrintBGColors)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean printBGImages; */
NS_IMETHODIMP nsPrintSettings::GetPrintBGImages(PRBool *aPrintBGImages)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPrintBGImages(PRBool aPrintBGImages)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute short printRange; */
NS_IMETHODIMP nsPrintSettings::GetPrintRange(PRInt16 *aPrintRange)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPrintRange(PRInt16 aPrintRange)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring title; */
NS_IMETHODIMP nsPrintSettings::GetTitle(PRUnichar * *aTitle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetTitle(const PRUnichar * aTitle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring docURL; */
NS_IMETHODIMP nsPrintSettings::GetDocURL(PRUnichar * *aDocURL)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetDocURL(const PRUnichar * aDocURL)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring headerStrLeft; */
NS_IMETHODIMP nsPrintSettings::GetHeaderStrLeft(PRUnichar * *aHeaderStrLeft)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetHeaderStrLeft(const PRUnichar * aHeaderStrLeft)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring headerStrCenter; */
NS_IMETHODIMP nsPrintSettings::GetHeaderStrCenter(PRUnichar * *aHeaderStrCenter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetHeaderStrCenter(const PRUnichar * aHeaderStrCenter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring headerStrRight; */
NS_IMETHODIMP nsPrintSettings::GetHeaderStrRight(PRUnichar * *aHeaderStrRight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetHeaderStrRight(const PRUnichar * aHeaderStrRight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring footerStrLeft; */
NS_IMETHODIMP nsPrintSettings::GetFooterStrLeft(PRUnichar * *aFooterStrLeft)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetFooterStrLeft(const PRUnichar * aFooterStrLeft)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring footerStrCenter; */
NS_IMETHODIMP nsPrintSettings::GetFooterStrCenter(PRUnichar * *aFooterStrCenter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetFooterStrCenter(const PRUnichar * aFooterStrCenter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring footerStrRight; */
NS_IMETHODIMP nsPrintSettings::GetFooterStrRight(PRUnichar * *aFooterStrRight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetFooterStrRight(const PRUnichar * aFooterStrRight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute short howToEnableFrameUI; */
NS_IMETHODIMP nsPrintSettings::GetHowToEnableFrameUI(PRInt16 *aHowToEnableFrameUI)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetHowToEnableFrameUI(PRInt16 aHowToEnableFrameUI)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean isCancelled; */
NS_IMETHODIMP nsPrintSettings::GetIsCancelled(PRBool *aIsCancelled)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetIsCancelled(PRBool aIsCancelled)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute short printFrameTypeUsage; */
NS_IMETHODIMP nsPrintSettings::GetPrintFrameTypeUsage(PRInt16 *aPrintFrameTypeUsage)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPrintFrameTypeUsage(PRInt16 aPrintFrameTypeUsage)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute short printFrameType; */
NS_IMETHODIMP nsPrintSettings::GetPrintFrameType(PRInt16 *aPrintFrameType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPrintFrameType(PRInt16 aPrintFrameType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean printSilent; */
NS_IMETHODIMP nsPrintSettings::GetPrintSilent(PRBool *aPrintSilent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPrintSilent(PRBool aPrintSilent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean shrinkToFit; */
NS_IMETHODIMP nsPrintSettings::GetShrinkToFit(PRBool *aShrinkToFit)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetShrinkToFit(PRBool aShrinkToFit)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean showPrintProgress; */
NS_IMETHODIMP nsPrintSettings::GetShowPrintProgress(PRBool *aShowPrintProgress)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetShowPrintProgress(PRBool aShowPrintProgress)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring paperName; */
NS_IMETHODIMP nsPrintSettings::GetPaperName(PRUnichar * *aPaperName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPaperName(const PRUnichar * aPaperName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute short paperSizeType; */
NS_IMETHODIMP nsPrintSettings::GetPaperSizeType(PRInt16 *aPaperSizeType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPaperSizeType(PRInt16 aPaperSizeType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute short paperData; */
NS_IMETHODIMP nsPrintSettings::GetPaperData(PRInt16 *aPaperData)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPaperData(PRInt16 aPaperData)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute double paperWidth; */
NS_IMETHODIMP nsPrintSettings::GetPaperWidth(double *aPaperWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPaperWidth(double aPaperWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute double paperHeight; */
NS_IMETHODIMP nsPrintSettings::GetPaperHeight(double *aPaperHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPaperHeight(double aPaperHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute short paperSizeUnit; */
NS_IMETHODIMP nsPrintSettings::GetPaperSizeUnit(PRInt16 *aPaperSizeUnit)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPaperSizeUnit(PRInt16 aPaperSizeUnit)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring plexName; */
NS_IMETHODIMP nsPrintSettings::GetPlexName(PRUnichar * *aPlexName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPlexName(const PRUnichar * aPlexName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring colorspace; */
NS_IMETHODIMP nsPrintSettings::GetColorspace(PRUnichar * *aColorspace)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetColorspace(const PRUnichar * aColorspace)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring resolutionName; */
NS_IMETHODIMP nsPrintSettings::GetResolutionName(PRUnichar * *aResolutionName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetResolutionName(const PRUnichar * aResolutionName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean downloadFonts; */
NS_IMETHODIMP nsPrintSettings::GetDownloadFonts(PRBool *aDownloadFonts)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetDownloadFonts(PRBool aDownloadFonts)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean printReversed; */
NS_IMETHODIMP nsPrintSettings::GetPrintReversed(PRBool *aPrintReversed)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPrintReversed(PRBool aPrintReversed)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean printInColor; */
NS_IMETHODIMP nsPrintSettings::GetPrintInColor(PRBool *aPrintInColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPrintInColor(PRBool aPrintInColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long paperSize; */
NS_IMETHODIMP nsPrintSettings::GetPaperSize(PRInt32 *aPaperSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPaperSize(PRInt32 aPaperSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long orientation; */
NS_IMETHODIMP nsPrintSettings::GetOrientation(PRInt32 *aOrientation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetOrientation(PRInt32 aOrientation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring printCommand; */
NS_IMETHODIMP nsPrintSettings::GetPrintCommand(PRUnichar * *aPrintCommand)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPrintCommand(const PRUnichar * aPrintCommand)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long numCopies; */
NS_IMETHODIMP nsPrintSettings::GetNumCopies(PRInt32 *aNumCopies)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetNumCopies(PRInt32 aNumCopies)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring printerName; */
NS_IMETHODIMP nsPrintSettings::GetPrinterName(PRUnichar * *aPrinterName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPrinterName(const PRUnichar * aPrinterName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean printToFile; */
NS_IMETHODIMP nsPrintSettings::GetPrintToFile(PRBool *aPrintToFile)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPrintToFile(PRBool aPrintToFile)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring toFileName; */
NS_IMETHODIMP nsPrintSettings::GetToFileName(PRUnichar * *aToFileName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetToFileName(const PRUnichar * aToFileName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long printPageDelay; */
NS_IMETHODIMP nsPrintSettings::GetPrintPageDelay(PRInt32 *aPrintPageDelay)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetPrintPageDelay(PRInt32 aPrintPageDelay)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean isInitializedFromPrinter; */
NS_IMETHODIMP nsPrintSettings::GetIsInitializedFromPrinter(PRBool *aIsInitializedFromPrinter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetIsInitializedFromPrinter(PRBool aIsInitializedFromPrinter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean isInitializedFromPrefs; */
NS_IMETHODIMP nsPrintSettings::GetIsInitializedFromPrefs(PRBool *aIsInitializedFromPrefs)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPrintSettings::SetIsInitializedFromPrefs(PRBool aIsInitializedFromPrefs)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void SetMarginInTwips (in nsNativeMarginRef aMargin); */
NS_IMETHODIMP nsPrintSettings::SetMarginInTwips(nsMargin & aMargin)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void GetMarginInTwips (in nsNativeMarginRef aMargin); */
NS_IMETHODIMP nsPrintSettings::GetMarginInTwips(nsMargin & aMargin)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIPrintSettings_h__ */
