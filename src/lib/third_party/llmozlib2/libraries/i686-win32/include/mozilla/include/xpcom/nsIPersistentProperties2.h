/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/ds/nsIPersistentProperties2.idl
 */

#ifndef __gen_nsIPersistentProperties2_h__
#define __gen_nsIPersistentProperties2_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsIProperties_h__
#include "nsIProperties.h"
#endif

#ifndef __gen_nsISimpleEnumerator_h__
#include "nsISimpleEnumerator.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIInputStream; /* forward declaration */

class nsIOutputStream; /* forward declaration */


/* starting interface:    nsIPropertyElement */
#define NS_IPROPERTYELEMENT_IID_STR "283ee646-1aef-11d4-98b3-00c04fa0ce9a"

#define NS_IPROPERTYELEMENT_IID \
  {0x283ee646, 0x1aef, 0x11d4, \
    { 0x98, 0xb3, 0x00, 0xc0, 0x4f, 0xa0, 0xce, 0x9a }}

class NS_NO_VTABLE nsIPropertyElement : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPROPERTYELEMENT_IID)

  /* attribute AUTF8String key; */
  NS_IMETHOD GetKey(nsACString & aKey) = 0;
  NS_IMETHOD SetKey(const nsACString & aKey) = 0;

  /* attribute AString value; */
  NS_IMETHOD GetValue(nsAString & aValue) = 0;
  NS_IMETHOD SetValue(const nsAString & aValue) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPROPERTYELEMENT \
  NS_IMETHOD GetKey(nsACString & aKey); \
  NS_IMETHOD SetKey(const nsACString & aKey); \
  NS_IMETHOD GetValue(nsAString & aValue); \
  NS_IMETHOD SetValue(const nsAString & aValue); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPROPERTYELEMENT(_to) \
  NS_IMETHOD GetKey(nsACString & aKey) { return _to GetKey(aKey); } \
  NS_IMETHOD SetKey(const nsACString & aKey) { return _to SetKey(aKey); } \
  NS_IMETHOD GetValue(nsAString & aValue) { return _to GetValue(aValue); } \
  NS_IMETHOD SetValue(const nsAString & aValue) { return _to SetValue(aValue); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPROPERTYELEMENT(_to) \
  NS_IMETHOD GetKey(nsACString & aKey) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetKey(aKey); } \
  NS_IMETHOD SetKey(const nsACString & aKey) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetKey(aKey); } \
  NS_IMETHOD GetValue(nsAString & aValue) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetValue(aValue); } \
  NS_IMETHOD SetValue(const nsAString & aValue) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetValue(aValue); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsPropertyElement : public nsIPropertyElement
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPROPERTYELEMENT

  nsPropertyElement();

private:
  ~nsPropertyElement();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsPropertyElement, nsIPropertyElement)

nsPropertyElement::nsPropertyElement()
{
  /* member initializers and constructor code */
}

nsPropertyElement::~nsPropertyElement()
{
  /* destructor code */
}

/* attribute AUTF8String key; */
NS_IMETHODIMP nsPropertyElement::GetKey(nsACString & aKey)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPropertyElement::SetKey(const nsACString & aKey)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute AString value; */
NS_IMETHODIMP nsPropertyElement::GetValue(nsAString & aValue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPropertyElement::SetValue(const nsAString & aValue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIPersistentProperties */
#define NS_IPERSISTENTPROPERTIES_IID_STR "1a180f60-93b2-11d2-9b8b-00805f8a16d9"

#define NS_IPERSISTENTPROPERTIES_IID \
  {0x1a180f60, 0x93b2, 0x11d2, \
    { 0x9b, 0x8b, 0x00, 0x80, 0x5f, 0x8a, 0x16, 0xd9 }}

class NS_NO_VTABLE nsIPersistentProperties : public nsIProperties {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPERSISTENTPROPERTIES_IID)

  /**
   * load a set of name/value pairs from the input stream
   * names and values should be in UTF8
   */
  /* void load (in nsIInputStream input); */
  NS_IMETHOD Load(nsIInputStream *input) = 0;

  /**
   * output the values to the stream - results will be in UTF8
   */
  /* void save (in nsIOutputStream output, in AUTF8String header); */
  NS_IMETHOD Save(nsIOutputStream *output, const nsACString & header) = 0;

  /**
   * call subclass() to make future calls to load() set the properties
   * in this "superclass" instead
   */
  /* void subclass (in nsIPersistentProperties superclass); */
  NS_IMETHOD Subclass(nsIPersistentProperties *superclass) = 0;

  /**
   * get an enumeration of nsIPropertyElement objects,
   * which are read-only (i.e. setting properties on the element will
   * not make changes back into the source nsIPersistentProperties
   */
  /* nsISimpleEnumerator enumerate (); */
  NS_IMETHOD Enumerate(nsISimpleEnumerator **_retval) = 0;

  /**
   * shortcut to nsIProperty's get() which retrieves a string value
   * directly (and thus faster)
   */
  /* AString getStringProperty (in AUTF8String key); */
  NS_IMETHOD GetStringProperty(const nsACString & key, nsAString & _retval) = 0;

  /**
   * shortcut to nsIProperty's set() which sets a string value
   * directly (and thus faster)
   */
  /* AString setStringProperty (in AUTF8String key, in AString value); */
  NS_IMETHOD SetStringProperty(const nsACString & key, const nsAString & value, nsAString & _retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPERSISTENTPROPERTIES \
  NS_IMETHOD Load(nsIInputStream *input); \
  NS_IMETHOD Save(nsIOutputStream *output, const nsACString & header); \
  NS_IMETHOD Subclass(nsIPersistentProperties *superclass); \
  NS_IMETHOD Enumerate(nsISimpleEnumerator **_retval); \
  NS_IMETHOD GetStringProperty(const nsACString & key, nsAString & _retval); \
  NS_IMETHOD SetStringProperty(const nsACString & key, const nsAString & value, nsAString & _retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPERSISTENTPROPERTIES(_to) \
  NS_IMETHOD Load(nsIInputStream *input) { return _to Load(input); } \
  NS_IMETHOD Save(nsIOutputStream *output, const nsACString & header) { return _to Save(output, header); } \
  NS_IMETHOD Subclass(nsIPersistentProperties *superclass) { return _to Subclass(superclass); } \
  NS_IMETHOD Enumerate(nsISimpleEnumerator **_retval) { return _to Enumerate(_retval); } \
  NS_IMETHOD GetStringProperty(const nsACString & key, nsAString & _retval) { return _to GetStringProperty(key, _retval); } \
  NS_IMETHOD SetStringProperty(const nsACString & key, const nsAString & value, nsAString & _retval) { return _to SetStringProperty(key, value, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPERSISTENTPROPERTIES(_to) \
  NS_IMETHOD Load(nsIInputStream *input) { return !_to ? NS_ERROR_NULL_POINTER : _to->Load(input); } \
  NS_IMETHOD Save(nsIOutputStream *output, const nsACString & header) { return !_to ? NS_ERROR_NULL_POINTER : _to->Save(output, header); } \
  NS_IMETHOD Subclass(nsIPersistentProperties *superclass) { return !_to ? NS_ERROR_NULL_POINTER : _to->Subclass(superclass); } \
  NS_IMETHOD Enumerate(nsISimpleEnumerator **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Enumerate(_retval); } \
  NS_IMETHOD GetStringProperty(const nsACString & key, nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStringProperty(key, _retval); } \
  NS_IMETHOD SetStringProperty(const nsACString & key, const nsAString & value, nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetStringProperty(key, value, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsPersistentProperties : public nsIPersistentProperties
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPERSISTENTPROPERTIES

  nsPersistentProperties();

private:
  ~nsPersistentProperties();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsPersistentProperties, nsIPersistentProperties)

nsPersistentProperties::nsPersistentProperties()
{
  /* member initializers and constructor code */
}

nsPersistentProperties::~nsPersistentProperties()
{
  /* destructor code */
}

/* void load (in nsIInputStream input); */
NS_IMETHODIMP nsPersistentProperties::Load(nsIInputStream *input)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void save (in nsIOutputStream output, in AUTF8String header); */
NS_IMETHODIMP nsPersistentProperties::Save(nsIOutputStream *output, const nsACString & header)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void subclass (in nsIPersistentProperties superclass); */
NS_IMETHODIMP nsPersistentProperties::Subclass(nsIPersistentProperties *superclass)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsISimpleEnumerator enumerate (); */
NS_IMETHODIMP nsPersistentProperties::Enumerate(nsISimpleEnumerator **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* AString getStringProperty (in AUTF8String key); */
NS_IMETHODIMP nsPersistentProperties::GetStringProperty(const nsACString & key, nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* AString setStringProperty (in AUTF8String key, in AString value); */
NS_IMETHODIMP nsPersistentProperties::SetStringProperty(const nsACString & key, const nsAString & value, nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

#define NS_IPERSISTENTPROPERTIES_CID \
{ 0x2245e573, 0x9464, 0x11d2, \
  { 0x9b, 0x8b, 0x0, 0x80, 0x5f, 0x8a, 0x16, 0xd9 } }
#define NS_PERSISTENTPROPERTIES_CONTRACTID "@mozilla.org/persistent-properties;1"
#define NS_PERSISTENTPROPERTIES_CLASSNAME "Persistent Properties"

#endif /* __gen_nsIPersistentProperties2_h__ */
