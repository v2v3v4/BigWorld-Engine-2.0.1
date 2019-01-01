/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/xtf/public/nsIXTFXMLVisual.idl
 */

#ifndef __gen_nsIXTFXMLVisual_h__
#define __gen_nsIXTFXMLVisual_h__


#ifndef __gen_nsIXTFVisual_h__
#include "nsIXTFVisual.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIXTFXMLVisualWrapper; /* forward declaration */

class nsIDOMElement; /* forward declaration */


/* starting interface:    nsIXTFXMLVisual */
#define NS_IXTFXMLVISUAL_IID_STR "e63d240d-bd00-4857-ba65-2f9cc599eead"

#define NS_IXTFXMLVISUAL_IID \
  {0xe63d240d, 0xbd00, 0x4857, \
    { 0xba, 0x65, 0x2f, 0x9c, 0xc5, 0x99, 0xee, 0xad }}

class NS_NO_VTABLE nsIXTFXMLVisual : public nsIXTFVisual {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXTFXMLVISUAL_IID)

  /* void onCreated (in nsIXTFXMLVisualWrapper wrapper); */
  NS_IMETHOD OnCreated(nsIXTFXMLVisualWrapper *wrapper) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXTFXMLVISUAL \
  NS_IMETHOD OnCreated(nsIXTFXMLVisualWrapper *wrapper); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXTFXMLVISUAL(_to) \
  NS_IMETHOD OnCreated(nsIXTFXMLVisualWrapper *wrapper) { return _to OnCreated(wrapper); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXTFXMLVISUAL(_to) \
  NS_IMETHOD OnCreated(nsIXTFXMLVisualWrapper *wrapper) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnCreated(wrapper); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXTFXMLVisual : public nsIXTFXMLVisual
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFXMLVISUAL

  nsXTFXMLVisual();

private:
  ~nsXTFXMLVisual();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXTFXMLVisual, nsIXTFXMLVisual)

nsXTFXMLVisual::nsXTFXMLVisual()
{
  /* member initializers and constructor code */
}

nsXTFXMLVisual::~nsXTFXMLVisual()
{
  /* destructor code */
}

/* void onCreated (in nsIXTFXMLVisualWrapper wrapper); */
NS_IMETHODIMP nsXTFXMLVisual::OnCreated(nsIXTFXMLVisualWrapper *wrapper)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXTFXMLVisual_h__ */
