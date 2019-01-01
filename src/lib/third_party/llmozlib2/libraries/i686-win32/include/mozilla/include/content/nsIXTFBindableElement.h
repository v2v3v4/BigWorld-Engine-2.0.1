/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/xtf/public/nsIXTFBindableElement.idl
 */

#ifndef __gen_nsIXTFBindableElement_h__
#define __gen_nsIXTFBindableElement_h__


#ifndef __gen_nsIXTFElement_h__
#include "nsIXTFElement.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIXTFBindableElementWrapper; /* forward declaration */


/* starting interface:    nsIXTFBindableElement */
#define NS_IXTFBINDABLEELEMENT_IID_STR "8dcc630c-9adc-4c60-9954-a004cb45e4a7"

#define NS_IXTFBINDABLEELEMENT_IID \
  {0x8dcc630c, 0x9adc, 0x4c60, \
    { 0x99, 0x54, 0xa0, 0x04, 0xcb, 0x45, 0xe4, 0xa7 }}

/**
 * nsIXTFBindableElement can be used to add support for new interfaces to
 * normal XML elements. XBL bindings do work with this kind of XTF elements.
 *
 * @note getScriptingInterfaces don't work at the moment with
 * nsIXTFBindableElements. The problem is that XBL does not play nicely
 * with that kind of interfaces.
 * However, if nsIXTFBindableElement implements scriptable interfaces, those
 * can be used by explicitly QIing to them.
 */
class NS_NO_VTABLE nsIXTFBindableElement : public nsIXTFElement {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXTFBINDABLEELEMENT_IID)

  /**
   * onCreated will be called before any notifications are sent to
   * the xtf element.
   *
   * @param wrapper is a weak proxy to the wrapping element
   * (i.e. holding a reference to this will not create a cycle).
   */
  /* void onCreated (in nsIXTFBindableElementWrapper wrapper); */
  NS_IMETHOD OnCreated(nsIXTFBindableElementWrapper *wrapper) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXTFBINDABLEELEMENT \
  NS_IMETHOD OnCreated(nsIXTFBindableElementWrapper *wrapper); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXTFBINDABLEELEMENT(_to) \
  NS_IMETHOD OnCreated(nsIXTFBindableElementWrapper *wrapper) { return _to OnCreated(wrapper); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXTFBINDABLEELEMENT(_to) \
  NS_IMETHOD OnCreated(nsIXTFBindableElementWrapper *wrapper) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnCreated(wrapper); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXTFBindableElement : public nsIXTFBindableElement
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFBINDABLEELEMENT

  nsXTFBindableElement();

private:
  ~nsXTFBindableElement();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXTFBindableElement, nsIXTFBindableElement)

nsXTFBindableElement::nsXTFBindableElement()
{
  /* member initializers and constructor code */
}

nsXTFBindableElement::~nsXTFBindableElement()
{
  /* destructor code */
}

/* void onCreated (in nsIXTFBindableElementWrapper wrapper); */
NS_IMETHODIMP nsXTFBindableElement::OnCreated(nsIXTFBindableElementWrapper *wrapper)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXTFBindableElement_h__ */
