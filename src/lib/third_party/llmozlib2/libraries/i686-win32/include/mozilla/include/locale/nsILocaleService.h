/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/intl/locale/idl/nsILocaleService.idl
 */

#ifndef __gen_nsILocaleService_h__
#define __gen_nsILocaleService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsILocale_h__
#include "nsILocale.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsILocaleDefinition */
#define NS_ILOCALEDEFINITION_IID_STR "7c094410-4558-11d3-91cd-00105aa3f7dc"

#define NS_ILOCALEDEFINITION_IID \
  {0x7c094410, 0x4558, 0x11d3, \
    { 0x91, 0xcd, 0x00, 0x10, 0x5a, 0xa3, 0xf7, 0xdc }}

class NS_NO_VTABLE nsILocaleDefinition : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ILOCALEDEFINITION_IID)

  /* void setLocaleCategory (in AString category, in AString value); */
  NS_IMETHOD SetLocaleCategory(const nsAString & category, const nsAString & value) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSILOCALEDEFINITION \
  NS_IMETHOD SetLocaleCategory(const nsAString & category, const nsAString & value); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSILOCALEDEFINITION(_to) \
  NS_IMETHOD SetLocaleCategory(const nsAString & category, const nsAString & value) { return _to SetLocaleCategory(category, value); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSILOCALEDEFINITION(_to) \
  NS_IMETHOD SetLocaleCategory(const nsAString & category, const nsAString & value) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetLocaleCategory(category, value); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsLocaleDefinition : public nsILocaleDefinition
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSILOCALEDEFINITION

  nsLocaleDefinition();

private:
  ~nsLocaleDefinition();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsLocaleDefinition, nsILocaleDefinition)

nsLocaleDefinition::nsLocaleDefinition()
{
  /* member initializers and constructor code */
}

nsLocaleDefinition::~nsLocaleDefinition()
{
  /* destructor code */
}

/* void setLocaleCategory (in AString category, in AString value); */
NS_IMETHODIMP nsLocaleDefinition::SetLocaleCategory(const nsAString & category, const nsAString & value)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsILocaleService */
#define NS_ILOCALESERVICE_IID_STR "48ab1fa0-4550-11d3-91cd-00105aa3f7dc"

#define NS_ILOCALESERVICE_IID \
  {0x48ab1fa0, 0x4550, 0x11d3, \
    { 0x91, 0xcd, 0x00, 0x10, 0x5a, 0xa3, 0xf7, 0xdc }}

class NS_NO_VTABLE nsILocaleService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ILOCALESERVICE_IID)

  /* nsILocale newLocale (in AString aLocale); */
  NS_IMETHOD NewLocale(const nsAString & aLocale, nsILocale **_retval) = 0;

  /* nsILocale newLocaleObject (in nsILocaleDefinition localeDefinition); */
  NS_IMETHOD NewLocaleObject(nsILocaleDefinition *localeDefinition, nsILocale **_retval) = 0;

  /* nsILocale getSystemLocale (); */
  NS_IMETHOD GetSystemLocale(nsILocale **_retval) = 0;

  /* nsILocale getApplicationLocale (); */
  NS_IMETHOD GetApplicationLocale(nsILocale **_retval) = 0;

  /* nsILocale getLocaleFromAcceptLanguage (in string acceptLanguage); */
  NS_IMETHOD GetLocaleFromAcceptLanguage(const char *acceptLanguage, nsILocale **_retval) = 0;

  /* AString getLocaleComponentForUserAgent (); */
  NS_IMETHOD GetLocaleComponentForUserAgent(nsAString & _retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSILOCALESERVICE \
  NS_IMETHOD NewLocale(const nsAString & aLocale, nsILocale **_retval); \
  NS_IMETHOD NewLocaleObject(nsILocaleDefinition *localeDefinition, nsILocale **_retval); \
  NS_IMETHOD GetSystemLocale(nsILocale **_retval); \
  NS_IMETHOD GetApplicationLocale(nsILocale **_retval); \
  NS_IMETHOD GetLocaleFromAcceptLanguage(const char *acceptLanguage, nsILocale **_retval); \
  NS_IMETHOD GetLocaleComponentForUserAgent(nsAString & _retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSILOCALESERVICE(_to) \
  NS_IMETHOD NewLocale(const nsAString & aLocale, nsILocale **_retval) { return _to NewLocale(aLocale, _retval); } \
  NS_IMETHOD NewLocaleObject(nsILocaleDefinition *localeDefinition, nsILocale **_retval) { return _to NewLocaleObject(localeDefinition, _retval); } \
  NS_IMETHOD GetSystemLocale(nsILocale **_retval) { return _to GetSystemLocale(_retval); } \
  NS_IMETHOD GetApplicationLocale(nsILocale **_retval) { return _to GetApplicationLocale(_retval); } \
  NS_IMETHOD GetLocaleFromAcceptLanguage(const char *acceptLanguage, nsILocale **_retval) { return _to GetLocaleFromAcceptLanguage(acceptLanguage, _retval); } \
  NS_IMETHOD GetLocaleComponentForUserAgent(nsAString & _retval) { return _to GetLocaleComponentForUserAgent(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSILOCALESERVICE(_to) \
  NS_IMETHOD NewLocale(const nsAString & aLocale, nsILocale **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->NewLocale(aLocale, _retval); } \
  NS_IMETHOD NewLocaleObject(nsILocaleDefinition *localeDefinition, nsILocale **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->NewLocaleObject(localeDefinition, _retval); } \
  NS_IMETHOD GetSystemLocale(nsILocale **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSystemLocale(_retval); } \
  NS_IMETHOD GetApplicationLocale(nsILocale **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetApplicationLocale(_retval); } \
  NS_IMETHOD GetLocaleFromAcceptLanguage(const char *acceptLanguage, nsILocale **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLocaleFromAcceptLanguage(acceptLanguage, _retval); } \
  NS_IMETHOD GetLocaleComponentForUserAgent(nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLocaleComponentForUserAgent(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsLocaleService : public nsILocaleService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSILOCALESERVICE

  nsLocaleService();

private:
  ~nsLocaleService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsLocaleService, nsILocaleService)

nsLocaleService::nsLocaleService()
{
  /* member initializers and constructor code */
}

nsLocaleService::~nsLocaleService()
{
  /* destructor code */
}

/* nsILocale newLocale (in AString aLocale); */
NS_IMETHODIMP nsLocaleService::NewLocale(const nsAString & aLocale, nsILocale **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsILocale newLocaleObject (in nsILocaleDefinition localeDefinition); */
NS_IMETHODIMP nsLocaleService::NewLocaleObject(nsILocaleDefinition *localeDefinition, nsILocale **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsILocale getSystemLocale (); */
NS_IMETHODIMP nsLocaleService::GetSystemLocale(nsILocale **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsILocale getApplicationLocale (); */
NS_IMETHODIMP nsLocaleService::GetApplicationLocale(nsILocale **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsILocale getLocaleFromAcceptLanguage (in string acceptLanguage); */
NS_IMETHODIMP nsLocaleService::GetLocaleFromAcceptLanguage(const char *acceptLanguage, nsILocale **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* AString getLocaleComponentForUserAgent (); */
NS_IMETHODIMP nsLocaleService::GetLocaleComponentForUserAgent(nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

// {C8E518C1-47AE-11d3-91CD-00105AA3F7DC}
#define NS_LOCALESERVICE_CID {0xc8e518c1,0x47ae,0x11d3,{0x91,0xcd,0x0,0x10,0x5a,0xa3,0xf7,0xdc}}
#define NS_LOCALESERVICE_CONTRACTID "@mozilla.org/intl/nslocaleservice;1"
extern nsresult
NS_NewLocaleService(nsILocaleService** result);
extern nsresult
NS_NewLocaleDefinition(nsILocaleDefinition** result);

#endif /* __gen_nsILocaleService_h__ */
