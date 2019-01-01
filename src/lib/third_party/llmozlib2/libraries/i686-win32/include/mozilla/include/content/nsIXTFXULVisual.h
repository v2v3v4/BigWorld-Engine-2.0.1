/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/xtf/public/nsIXTFXULVisual.idl
 */

#ifndef __gen_nsIXTFXULVisual_h__
#define __gen_nsIXTFXULVisual_h__


#ifndef __gen_nsIXTFVisual_h__
#include "nsIXTFVisual.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIXTFXULVisualWrapper; /* forward declaration */

class nsIDOMElement; /* forward declaration */


/* starting interface:    nsIXTFXULVisual */
#define NS_IXTFXULVISUAL_IID_STR "a1173d91-4428-4829-8e3e-fe66e558f161"

#define NS_IXTFXULVISUAL_IID \
  {0xa1173d91, 0x4428, 0x4829, \
    { 0x8e, 0x3e, 0xfe, 0x66, 0xe5, 0x58, 0xf1, 0x61 }}

class NS_NO_VTABLE nsIXTFXULVisual : public nsIXTFVisual {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXTFXULVISUAL_IID)

  /* void onCreated (in nsIXTFXULVisualWrapper wrapper); */
  NS_IMETHOD OnCreated(nsIXTFXULVisualWrapper *wrapper) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXTFXULVISUAL \
  NS_IMETHOD OnCreated(nsIXTFXULVisualWrapper *wrapper); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXTFXULVISUAL(_to) \
  NS_IMETHOD OnCreated(nsIXTFXULVisualWrapper *wrapper) { return _to OnCreated(wrapper); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXTFXULVISUAL(_to) \
  NS_IMETHOD OnCreated(nsIXTFXULVisualWrapper *wrapper) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnCreated(wrapper); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXTFXULVisual : public nsIXTFXULVisual
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFXULVISUAL

  nsXTFXULVisual();

private:
  ~nsXTFXULVisual();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXTFXULVisual, nsIXTFXULVisual)

nsXTFXULVisual::nsXTFXULVisual()
{
  /* member initializers and constructor code */
}

nsXTFXULVisual::~nsXTFXULVisual()
{
  /* destructor code */
}

/* void onCreated (in nsIXTFXULVisualWrapper wrapper); */
NS_IMETHODIMP nsXTFXULVisual::OnCreated(nsIXTFXULVisualWrapper *wrapper)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXTFXULVisual_h__ */
