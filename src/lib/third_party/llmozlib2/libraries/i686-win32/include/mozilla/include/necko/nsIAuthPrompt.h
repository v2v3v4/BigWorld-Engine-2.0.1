/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsIAuthPrompt.idl
 */

#ifndef __gen_nsIAuthPrompt_h__
#define __gen_nsIAuthPrompt_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIPrompt; /* forward declaration */


/* starting interface:    nsIAuthPrompt */
#define NS_IAUTHPROMPT_IID_STR "2f977d45-5485-11d4-87e2-0010a4e75ef2"

#define NS_IAUTHPROMPT_IID \
  {0x2f977d45, 0x5485, 0x11d4, \
    { 0x87, 0xe2, 0x00, 0x10, 0xa4, 0xe7, 0x5e, 0xf2 }}

/**
 * @status UNDER_REVIEW
 */
class NS_NO_VTABLE nsIAuthPrompt : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IAUTHPROMPT_IID)

  enum { SAVE_PASSWORD_NEVER = 0U };

  enum { SAVE_PASSWORD_FOR_SESSION = 1U };

  enum { SAVE_PASSWORD_PERMANENTLY = 2U };

  /**
     * Puts up a text input dialog with OK and Cancel buttons.
     * @return true for OK, false for Cancel
     */
  /* boolean prompt (in wstring dialogTitle, in wstring text, in wstring passwordRealm, in PRUint32 savePassword, in wstring defaultText, out wstring result); */
  NS_IMETHOD Prompt(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, const PRUnichar *defaultText, PRUnichar **result, PRBool *_retval) = 0;

  /**
     * Puts up a username/password dialog with OK and Cancel buttons.
     * @return true for OK, false for Cancel
     */
  /* boolean promptUsernameAndPassword (in wstring dialogTitle, in wstring text, in wstring passwordRealm, in PRUint32 savePassword, out wstring user, out wstring pwd); */
  NS_IMETHOD PromptUsernameAndPassword(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, PRUnichar **user, PRUnichar **pwd, PRBool *_retval) = 0;

  /**
     * Puts up a password dialog with OK and Cancel buttons.
     * @return true for OK, false for Cancel
     */
  /* boolean promptPassword (in wstring dialogTitle, in wstring text, in wstring passwordRealm, in PRUint32 savePassword, out wstring pwd); */
  NS_IMETHOD PromptPassword(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, PRUnichar **pwd, PRBool *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIAUTHPROMPT \
  NS_IMETHOD Prompt(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, const PRUnichar *defaultText, PRUnichar **result, PRBool *_retval); \
  NS_IMETHOD PromptUsernameAndPassword(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, PRUnichar **user, PRUnichar **pwd, PRBool *_retval); \
  NS_IMETHOD PromptPassword(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, PRUnichar **pwd, PRBool *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIAUTHPROMPT(_to) \
  NS_IMETHOD Prompt(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, const PRUnichar *defaultText, PRUnichar **result, PRBool *_retval) { return _to Prompt(dialogTitle, text, passwordRealm, savePassword, defaultText, result, _retval); } \
  NS_IMETHOD PromptUsernameAndPassword(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, PRUnichar **user, PRUnichar **pwd, PRBool *_retval) { return _to PromptUsernameAndPassword(dialogTitle, text, passwordRealm, savePassword, user, pwd, _retval); } \
  NS_IMETHOD PromptPassword(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, PRUnichar **pwd, PRBool *_retval) { return _to PromptPassword(dialogTitle, text, passwordRealm, savePassword, pwd, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIAUTHPROMPT(_to) \
  NS_IMETHOD Prompt(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, const PRUnichar *defaultText, PRUnichar **result, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Prompt(dialogTitle, text, passwordRealm, savePassword, defaultText, result, _retval); } \
  NS_IMETHOD PromptUsernameAndPassword(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, PRUnichar **user, PRUnichar **pwd, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->PromptUsernameAndPassword(dialogTitle, text, passwordRealm, savePassword, user, pwd, _retval); } \
  NS_IMETHOD PromptPassword(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, PRUnichar **pwd, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->PromptPassword(dialogTitle, text, passwordRealm, savePassword, pwd, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsAuthPrompt : public nsIAuthPrompt
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIAUTHPROMPT

  nsAuthPrompt();

private:
  ~nsAuthPrompt();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsAuthPrompt, nsIAuthPrompt)

nsAuthPrompt::nsAuthPrompt()
{
  /* member initializers and constructor code */
}

nsAuthPrompt::~nsAuthPrompt()
{
  /* destructor code */
}

/* boolean prompt (in wstring dialogTitle, in wstring text, in wstring passwordRealm, in PRUint32 savePassword, in wstring defaultText, out wstring result); */
NS_IMETHODIMP nsAuthPrompt::Prompt(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, const PRUnichar *defaultText, PRUnichar **result, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean promptUsernameAndPassword (in wstring dialogTitle, in wstring text, in wstring passwordRealm, in PRUint32 savePassword, out wstring user, out wstring pwd); */
NS_IMETHODIMP nsAuthPrompt::PromptUsernameAndPassword(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, PRUnichar **user, PRUnichar **pwd, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean promptPassword (in wstring dialogTitle, in wstring text, in wstring passwordRealm, in PRUint32 savePassword, out wstring pwd); */
NS_IMETHODIMP nsAuthPrompt::PromptPassword(const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *passwordRealm, PRUint32 savePassword, PRUnichar **pwd, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIAuthPrompt_h__ */
