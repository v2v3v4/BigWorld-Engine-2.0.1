/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsIAuthPromptProvider.idl
 */

#ifndef __gen_nsIAuthPromptProvider_h__
#define __gen_nsIAuthPromptProvider_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIAuthPrompt; /* forward declaration */


/* starting interface:    nsIAuthPromptProvider */
#define NS_IAUTHPROMPTPROVIDER_IID_STR "129d3bd5-8a26-4b0b-b8a0-19fdea029196"

#define NS_IAUTHPROMPTPROVIDER_IID \
  {0x129d3bd5, 0x8a26, 0x4b0b, \
    { 0xb8, 0xa0, 0x19, 0xfd, 0xea, 0x02, 0x91, 0x96 }}

class NS_NO_VTABLE nsIAuthPromptProvider : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IAUTHPROMPTPROVIDER_IID)

  /**
     * Normal (non-proxy) prompt request.
     */
  enum { PROMPT_NORMAL = 0U };

  /**
     * Proxy auth request.
     */
  enum { PROMPT_PROXY = 1U };

  /**
     * Request a nsIAuthPrompt interface for the given prompt reason;
     * @throws NS_ERROR_NOT_AVAILABLE if no prompt is allowed or
     * available for the given reason.
     *
     * @param aPromptReason   The reason for the auth prompt;
     *                        one of @PROMPT_NORMAL or @PROMPT_PROXY
     * @returns a nsIAuthPrompt interface, or throws NS_ERROR_NOT_AVAILABLE
     */
  /* nsIAuthPrompt getAuthPrompt (in PRUint32 aPromptReason); */
  NS_IMETHOD GetAuthPrompt(PRUint32 aPromptReason, nsIAuthPrompt **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIAUTHPROMPTPROVIDER \
  NS_IMETHOD GetAuthPrompt(PRUint32 aPromptReason, nsIAuthPrompt **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIAUTHPROMPTPROVIDER(_to) \
  NS_IMETHOD GetAuthPrompt(PRUint32 aPromptReason, nsIAuthPrompt **_retval) { return _to GetAuthPrompt(aPromptReason, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIAUTHPROMPTPROVIDER(_to) \
  NS_IMETHOD GetAuthPrompt(PRUint32 aPromptReason, nsIAuthPrompt **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAuthPrompt(aPromptReason, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsAuthPromptProvider : public nsIAuthPromptProvider
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIAUTHPROMPTPROVIDER

  nsAuthPromptProvider();

private:
  ~nsAuthPromptProvider();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsAuthPromptProvider, nsIAuthPromptProvider)

nsAuthPromptProvider::nsAuthPromptProvider()
{
  /* member initializers and constructor code */
}

nsAuthPromptProvider::~nsAuthPromptProvider()
{
  /* destructor code */
}

/* nsIAuthPrompt getAuthPrompt (in PRUint32 aPromptReason); */
NS_IMETHODIMP nsAuthPromptProvider::GetAuthPrompt(PRUint32 aPromptReason, nsIAuthPrompt **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIAuthPromptProvider_h__ */
