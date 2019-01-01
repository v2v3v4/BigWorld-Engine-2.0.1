/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/gfx/idl/nsIFontList.idl
 */

#ifndef __gen_nsIFontList_h__
#define __gen_nsIFontList_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsISimpleEnumerator; /* forward declaration */


/* starting interface:    nsIFontList */
#define NS_IFONTLIST_IID_STR "702909c6-1dd2-11b2-b833-8a740f643539"

#define NS_IFONTLIST_IID \
  {0x702909c6, 0x1dd2, 0x11b2, \
    { 0xb8, 0x33, 0x8a, 0x74, 0x0f, 0x64, 0x35, 0x39 }}

/**
 * The nsIFontList interface provides an application the  
 * necessary information so that the user can select the font to use 
 * as the default style sheet. This is used if the style sheet is 
 * missing or does not specify a font.
 * <P>Font lists are specified per language group.
 *
 */
class NS_NO_VTABLE nsIFontList : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IFONTLIST_IID)

  /**
   * Get the list of available fonts for a language group
   * and for use as the given CSS generic font.
   *
   * @param aLangGroup limits the fonts to fonts in a language 
   *          group; eg: x-western (American/Western European), 
   *          ar (Arabic), el (Greek), he (Hebrew), ja (Japanese),
   *          ko (Korean), th (Thai), tr (Turkish),
   *          x-baltic (Baltic), x-central-euro (Eastern European),
   *          x-cyrillic (Russian), zh-CN (China), zh-TW (Taiwan)
   *
   * @param aFontType limits the fonts to the fonts with this CSS 
   *          generic font type; eg: serif, sans-serif, cursive,
   *          fantasy, monospace.
   *
   * @return a simple enumerator of the available fonts for a 
   *          language group / generic type.  These strings are
   *          the text supplied by the operating system's font 
   *          system.
   */
  /* nsISimpleEnumerator availableFonts (in wstring aLangGroup, in wstring aFontType); */
  NS_IMETHOD AvailableFonts(const PRUnichar *aLangGroup, const PRUnichar *aFontType, nsISimpleEnumerator **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIFONTLIST \
  NS_IMETHOD AvailableFonts(const PRUnichar *aLangGroup, const PRUnichar *aFontType, nsISimpleEnumerator **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIFONTLIST(_to) \
  NS_IMETHOD AvailableFonts(const PRUnichar *aLangGroup, const PRUnichar *aFontType, nsISimpleEnumerator **_retval) { return _to AvailableFonts(aLangGroup, aFontType, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIFONTLIST(_to) \
  NS_IMETHOD AvailableFonts(const PRUnichar *aLangGroup, const PRUnichar *aFontType, nsISimpleEnumerator **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->AvailableFonts(aLangGroup, aFontType, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsFontList : public nsIFontList
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIFONTLIST

  nsFontList();

private:
  ~nsFontList();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsFontList, nsIFontList)

nsFontList::nsFontList()
{
  /* member initializers and constructor code */
}

nsFontList::~nsFontList()
{
  /* destructor code */
}

/* nsISimpleEnumerator availableFonts (in wstring aLangGroup, in wstring aFontType); */
NS_IMETHODIMP nsFontList::AvailableFonts(const PRUnichar *aLangGroup, const PRUnichar *aFontType, nsISimpleEnumerator **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIFontList_h__ */
