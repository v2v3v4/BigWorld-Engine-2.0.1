/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/xtf/public/nsIXTFSVGVisual.idl
 */

#ifndef __gen_nsIXTFSVGVisual_h__
#define __gen_nsIXTFSVGVisual_h__


#ifndef __gen_nsIXTFVisual_h__
#include "nsIXTFVisual.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIXTFSVGVisualWrapper; /* forward declaration */

class nsIDOMElement; /* forward declaration */


/* starting interface:    nsIXTFSVGVisual */
#define NS_IXTFSVGVISUAL_IID_STR "5fd47925-03b2-4318-b55e-c37134124b6a"

#define NS_IXTFSVGVISUAL_IID \
  {0x5fd47925, 0x03b2, 0x4318, \
    { 0xb5, 0x5e, 0xc3, 0x71, 0x34, 0x12, 0x4b, 0x6a }}

class NS_NO_VTABLE nsIXTFSVGVisual : public nsIXTFVisual {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXTFSVGVISUAL_IID)

  /* void onCreated (in nsIXTFSVGVisualWrapper wrapper); */
  NS_IMETHOD OnCreated(nsIXTFSVGVisualWrapper *wrapper) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXTFSVGVISUAL \
  NS_IMETHOD OnCreated(nsIXTFSVGVisualWrapper *wrapper); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXTFSVGVISUAL(_to) \
  NS_IMETHOD OnCreated(nsIXTFSVGVisualWrapper *wrapper) { return _to OnCreated(wrapper); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXTFSVGVISUAL(_to) \
  NS_IMETHOD OnCreated(nsIXTFSVGVisualWrapper *wrapper) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnCreated(wrapper); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXTFSVGVisual : public nsIXTFSVGVisual
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFSVGVISUAL

  nsXTFSVGVisual();

private:
  ~nsXTFSVGVisual();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXTFSVGVisual, nsIXTFSVGVisual)

nsXTFSVGVisual::nsXTFSVGVisual()
{
  /* member initializers and constructor code */
}

nsXTFSVGVisual::~nsXTFSVGVisual()
{
  /* destructor code */
}

/* void onCreated (in nsIXTFSVGVisualWrapper wrapper); */
NS_IMETHODIMP nsXTFSVGVisual::OnCreated(nsIXTFSVGVisualWrapper *wrapper)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXTFSVGVisual_h__ */
