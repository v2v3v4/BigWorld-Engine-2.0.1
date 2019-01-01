/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/docshell/base/nsIURIFixup.idl
 */

#ifndef __gen_nsIURIFixup_h__
#define __gen_nsIURIFixup_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIURI; /* forward declaration */


/* starting interface:    nsIURIFixup */
#define NS_IURIFIXUP_IID_STR "2efd4a40-a5e1-11d4-9589-0020183bf181"

#define NS_IURIFIXUP_IID \
  {0x2efd4a40, 0xa5e1, 0x11d4, \
    { 0x95, 0x89, 0x00, 0x20, 0x18, 0x3b, 0xf1, 0x81 }}

/**
 * Interface implemented by objects capable of fixing up strings into URIs
 */
class NS_NO_VTABLE nsIURIFixup : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IURIFIXUP_IID)

  /** No fixup flags. */
  enum { FIXUP_FLAG_NONE = 0U };

  /**
     * Allow the fixup to use a keyword lookup service to complete the URI.
     * The fixup object implementer should honour this flag and only perform
     * any lengthy keyword (or search) operation if it is set.
     */
  enum { FIXUP_FLAG_ALLOW_KEYWORD_LOOKUP = 1U };

  /**
     * Tell the fixup to make an alternate URI from the input URI, for example
     * to turn foo into www.foo.com.
     */
  enum { FIXUP_FLAGS_MAKE_ALTERNATE_URI = 2U };

  /**
     * Converts an internal URI (e.g. a wyciwyg URI) into one which we can
     * expose to the user, for example on the URL bar.
     *
     * @param  aURI       The URI to be converted
     * @return nsIURI     The converted, exposable URI
     * @throws NS_ERROR_MALFORMED_URI when the exposable portion of aURI is malformed
     * @throws NS_ERROR_UNKNOWN_PROTOCOL when we can't get a protocol handler service
     *         for the URI scheme.
     */
  /* nsIURI createExposableURI (in nsIURI aURI); */
  NS_IMETHOD CreateExposableURI(nsIURI *aURI, nsIURI **_retval) = 0;

  /**
     * Converts the specified string into a URI, first attempting
     * to correct any errors in the syntax or other vagaries. Returns
     * a wellformed URI or nsnull if it can't.
     *
     * @param aURIText    Candidate URI.
     * @param aFixupFlags Flags that govern ways the URI may be fixed up.
     */
  /* nsIURI createFixupURI (in AUTF8String aURIText, in unsigned long aFixupFlags); */
  NS_IMETHOD CreateFixupURI(const nsACString & aURIText, PRUint32 aFixupFlags, nsIURI **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIURIFIXUP \
  NS_IMETHOD CreateExposableURI(nsIURI *aURI, nsIURI **_retval); \
  NS_IMETHOD CreateFixupURI(const nsACString & aURIText, PRUint32 aFixupFlags, nsIURI **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIURIFIXUP(_to) \
  NS_IMETHOD CreateExposableURI(nsIURI *aURI, nsIURI **_retval) { return _to CreateExposableURI(aURI, _retval); } \
  NS_IMETHOD CreateFixupURI(const nsACString & aURIText, PRUint32 aFixupFlags, nsIURI **_retval) { return _to CreateFixupURI(aURIText, aFixupFlags, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIURIFIXUP(_to) \
  NS_IMETHOD CreateExposableURI(nsIURI *aURI, nsIURI **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateExposableURI(aURI, _retval); } \
  NS_IMETHOD CreateFixupURI(const nsACString & aURIText, PRUint32 aFixupFlags, nsIURI **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateFixupURI(aURIText, aFixupFlags, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsURIFixup : public nsIURIFixup
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIURIFIXUP

  nsURIFixup();

private:
  ~nsURIFixup();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsURIFixup, nsIURIFixup)

nsURIFixup::nsURIFixup()
{
  /* member initializers and constructor code */
}

nsURIFixup::~nsURIFixup()
{
  /* destructor code */
}

/* nsIURI createExposableURI (in nsIURI aURI); */
NS_IMETHODIMP nsURIFixup::CreateExposableURI(nsIURI *aURI, nsIURI **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIURI createFixupURI (in AUTF8String aURIText, in unsigned long aFixupFlags); */
NS_IMETHODIMP nsURIFixup::CreateFixupURI(const nsACString & aURIText, PRUint32 aFixupFlags, nsIURI **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIURIFixup_MOZILLA_1_8_BRANCH */
#define NS_IURIFIXUP_MOZILLA_1_8_BRANCH_IID_STR "6ca37983-16aa-4013-b753-77b770ff93d6"

#define NS_IURIFIXUP_MOZILLA_1_8_BRANCH_IID \
  {0x6ca37983, 0x16aa, 0x4013, \
    { 0xb7, 0x53, 0x77, 0xb7, 0x70, 0xff, 0x93, 0xd6 }}

class NS_NO_VTABLE nsIURIFixup_MOZILLA_1_8_BRANCH : public nsIURIFixup {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IURIFIXUP_MOZILLA_1_8_BRANCH_IID)

  /**
     * Converts the specified keyword string into a URI.  Note that it's the
     * caller's responsibility to check whether keywords are enabled and
     * whether aKeyword is a sensible keyword.
     */
  /* nsIURI keywordToURI (in AUTF8String aKeyword); */
  NS_IMETHOD KeywordToURI(const nsACString & aKeyword, nsIURI **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIURIFIXUP_MOZILLA_1_8_BRANCH \
  NS_IMETHOD KeywordToURI(const nsACString & aKeyword, nsIURI **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIURIFIXUP_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD KeywordToURI(const nsACString & aKeyword, nsIURI **_retval) { return _to KeywordToURI(aKeyword, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIURIFIXUP_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD KeywordToURI(const nsACString & aKeyword, nsIURI **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->KeywordToURI(aKeyword, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsURIFixup_MOZILLA_1_8_BRANCH : public nsIURIFixup_MOZILLA_1_8_BRANCH
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIURIFIXUP_MOZILLA_1_8_BRANCH

  nsURIFixup_MOZILLA_1_8_BRANCH();

private:
  ~nsURIFixup_MOZILLA_1_8_BRANCH();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsURIFixup_MOZILLA_1_8_BRANCH, nsIURIFixup_MOZILLA_1_8_BRANCH)

nsURIFixup_MOZILLA_1_8_BRANCH::nsURIFixup_MOZILLA_1_8_BRANCH()
{
  /* member initializers and constructor code */
}

nsURIFixup_MOZILLA_1_8_BRANCH::~nsURIFixup_MOZILLA_1_8_BRANCH()
{
  /* destructor code */
}

/* nsIURI keywordToURI (in AUTF8String aKeyword); */
NS_IMETHODIMP nsURIFixup_MOZILLA_1_8_BRANCH::KeywordToURI(const nsACString & aKeyword, nsIURI **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIURIFixup_h__ */
