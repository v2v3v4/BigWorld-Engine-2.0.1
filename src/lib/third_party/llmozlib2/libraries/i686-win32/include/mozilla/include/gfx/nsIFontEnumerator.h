/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/gfx/idl/nsIFontEnumerator.idl
 */

#ifndef __gen_nsIFontEnumerator_h__
#define __gen_nsIFontEnumerator_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIFontEnumerator */
#define NS_IFONTENUMERATOR_IID_STR "a6cf9114-15b3-11d2-932e-00805f8add32"

#define NS_IFONTENUMERATOR_IID \
  {0xa6cf9114, 0x15b3, 0x11d2, \
    { 0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32 }}

class NS_NO_VTABLE nsIFontEnumerator : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IFONTENUMERATOR_IID)

  /**
   * Return a sorted array of the names of all installed fonts.
   *
   * @param  aCount     returns number of names returned
   * @param  aResult    returns array of names
   * @return void
   */
  /* void EnumerateAllFonts (out PRUint32 aCount, [array, size_is (aCount), retval] out wstring aResult); */
  NS_IMETHOD EnumerateAllFonts(PRUint32 *aCount, PRUnichar ***aResult) = 0;

  /**
   * Return a sorted array of names of fonts that support the given language
   * group and are suitable for use as the given CSS generic font.
   *
   * @param  aLangGroup language group
   * @param  aGeneric   CSS generic font
   * @param  aCount     returns number of names returned
   * @param  aResult    returns array of names
   * @return void
   */
  /* void EnumerateFonts (in string aLangGroup, in string aGeneric, out PRUint32 aCount, [array, size_is (aCount), retval] out wstring aResult); */
  NS_IMETHOD EnumerateFonts(const char *aLangGroup, const char *aGeneric, PRUint32 *aCount, PRUnichar ***aResult) = 0;

  /**
    @param  aLangGroup language group
    @return bool do we have a font for this language group
   */
  /* void HaveFontFor (in string aLangGroup, [retval] out boolean aResult); */
  NS_IMETHOD HaveFontFor(const char *aLangGroup, PRBool *aResult) = 0;

  /**
   * @param  aLangGroup language group
   * @param  aGeneric CSS generic font
   * @return suggested default font for this language group and generic family
   */
  /* wstring getDefaultFont (in string aLangGroup, in string aGeneric); */
  NS_IMETHOD GetDefaultFont(const char *aLangGroup, const char *aGeneric, PRUnichar **_retval) = 0;

  /**
   * update the global font list
   * return true if font list is changed
   */
  /* boolean updateFontList (); */
  NS_IMETHOD UpdateFontList(PRBool *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIFONTENUMERATOR \
  NS_IMETHOD EnumerateAllFonts(PRUint32 *aCount, PRUnichar ***aResult); \
  NS_IMETHOD EnumerateFonts(const char *aLangGroup, const char *aGeneric, PRUint32 *aCount, PRUnichar ***aResult); \
  NS_IMETHOD HaveFontFor(const char *aLangGroup, PRBool *aResult); \
  NS_IMETHOD GetDefaultFont(const char *aLangGroup, const char *aGeneric, PRUnichar **_retval); \
  NS_IMETHOD UpdateFontList(PRBool *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIFONTENUMERATOR(_to) \
  NS_IMETHOD EnumerateAllFonts(PRUint32 *aCount, PRUnichar ***aResult) { return _to EnumerateAllFonts(aCount, aResult); } \
  NS_IMETHOD EnumerateFonts(const char *aLangGroup, const char *aGeneric, PRUint32 *aCount, PRUnichar ***aResult) { return _to EnumerateFonts(aLangGroup, aGeneric, aCount, aResult); } \
  NS_IMETHOD HaveFontFor(const char *aLangGroup, PRBool *aResult) { return _to HaveFontFor(aLangGroup, aResult); } \
  NS_IMETHOD GetDefaultFont(const char *aLangGroup, const char *aGeneric, PRUnichar **_retval) { return _to GetDefaultFont(aLangGroup, aGeneric, _retval); } \
  NS_IMETHOD UpdateFontList(PRBool *_retval) { return _to UpdateFontList(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIFONTENUMERATOR(_to) \
  NS_IMETHOD EnumerateAllFonts(PRUint32 *aCount, PRUnichar ***aResult) { return !_to ? NS_ERROR_NULL_POINTER : _to->EnumerateAllFonts(aCount, aResult); } \
  NS_IMETHOD EnumerateFonts(const char *aLangGroup, const char *aGeneric, PRUint32 *aCount, PRUnichar ***aResult) { return !_to ? NS_ERROR_NULL_POINTER : _to->EnumerateFonts(aLangGroup, aGeneric, aCount, aResult); } \
  NS_IMETHOD HaveFontFor(const char *aLangGroup, PRBool *aResult) { return !_to ? NS_ERROR_NULL_POINTER : _to->HaveFontFor(aLangGroup, aResult); } \
  NS_IMETHOD GetDefaultFont(const char *aLangGroup, const char *aGeneric, PRUnichar **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDefaultFont(aLangGroup, aGeneric, _retval); } \
  NS_IMETHOD UpdateFontList(PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->UpdateFontList(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsFontEnumerator : public nsIFontEnumerator
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIFONTENUMERATOR

  nsFontEnumerator();

private:
  ~nsFontEnumerator();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsFontEnumerator, nsIFontEnumerator)

nsFontEnumerator::nsFontEnumerator()
{
  /* member initializers and constructor code */
}

nsFontEnumerator::~nsFontEnumerator()
{
  /* destructor code */
}

/* void EnumerateAllFonts (out PRUint32 aCount, [array, size_is (aCount), retval] out wstring aResult); */
NS_IMETHODIMP nsFontEnumerator::EnumerateAllFonts(PRUint32 *aCount, PRUnichar ***aResult)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void EnumerateFonts (in string aLangGroup, in string aGeneric, out PRUint32 aCount, [array, size_is (aCount), retval] out wstring aResult); */
NS_IMETHODIMP nsFontEnumerator::EnumerateFonts(const char *aLangGroup, const char *aGeneric, PRUint32 *aCount, PRUnichar ***aResult)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void HaveFontFor (in string aLangGroup, [retval] out boolean aResult); */
NS_IMETHODIMP nsFontEnumerator::HaveFontFor(const char *aLangGroup, PRBool *aResult)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* wstring getDefaultFont (in string aLangGroup, in string aGeneric); */
NS_IMETHODIMP nsFontEnumerator::GetDefaultFont(const char *aLangGroup, const char *aGeneric, PRUnichar **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean updateFontList (); */
NS_IMETHODIMP nsFontEnumerator::UpdateFontList(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIFontEnumerator_h__ */
