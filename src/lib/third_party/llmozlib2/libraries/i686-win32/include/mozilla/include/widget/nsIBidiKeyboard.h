/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/widget/public/nsIBidiKeyboard.idl
 */

#ifndef __gen_nsIBidiKeyboard_h__
#define __gen_nsIBidiKeyboard_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIBidiKeyboard */
#define NS_IBIDIKEYBOARD_IID_STR "bb961ae1-7432-11d4-b77a-00104b4119f8"

#define NS_IBIDIKEYBOARD_IID \
  {0xbb961ae1, 0x7432, 0x11d4, \
    { 0xb7, 0x7a, 0x00, 0x10, 0x4b, 0x41, 0x19, 0xf8 }}

class NS_NO_VTABLE nsIBidiKeyboard : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IBIDIKEYBOARD_IID)

  /**
   * Determines if the current keyboard language is right-to-left
   */
  /* void isLangRTL (out PRBool aIsRTL); */
  NS_IMETHOD IsLangRTL(PRBool *aIsRTL) = 0;

  /**
   * Sets the keyboard language to left-to-right or right-to-left
   * @param aLevel - if odd set the keyboard to RTL, if even set LTR 
   */
  /* void setLangFromBidiLevel (in PRUint8 aLevel); */
  NS_IMETHOD SetLangFromBidiLevel(PRUint8 aLevel) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIBIDIKEYBOARD \
  NS_IMETHOD IsLangRTL(PRBool *aIsRTL); \
  NS_IMETHOD SetLangFromBidiLevel(PRUint8 aLevel); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIBIDIKEYBOARD(_to) \
  NS_IMETHOD IsLangRTL(PRBool *aIsRTL) { return _to IsLangRTL(aIsRTL); } \
  NS_IMETHOD SetLangFromBidiLevel(PRUint8 aLevel) { return _to SetLangFromBidiLevel(aLevel); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIBIDIKEYBOARD(_to) \
  NS_IMETHOD IsLangRTL(PRBool *aIsRTL) { return !_to ? NS_ERROR_NULL_POINTER : _to->IsLangRTL(aIsRTL); } \
  NS_IMETHOD SetLangFromBidiLevel(PRUint8 aLevel) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetLangFromBidiLevel(aLevel); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsBidiKeyboard : public nsIBidiKeyboard
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIBIDIKEYBOARD

  nsBidiKeyboard();

private:
  ~nsBidiKeyboard();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsBidiKeyboard, nsIBidiKeyboard)

nsBidiKeyboard::nsBidiKeyboard()
{
  /* member initializers and constructor code */
}

nsBidiKeyboard::~nsBidiKeyboard()
{
  /* destructor code */
}

/* void isLangRTL (out PRBool aIsRTL); */
NS_IMETHODIMP nsBidiKeyboard::IsLangRTL(PRBool *aIsRTL)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setLangFromBidiLevel (in PRUint8 aLevel); */
NS_IMETHODIMP nsBidiKeyboard::SetLangFromBidiLevel(PRUint8 aLevel)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIBidiKeyboard_h__ */
