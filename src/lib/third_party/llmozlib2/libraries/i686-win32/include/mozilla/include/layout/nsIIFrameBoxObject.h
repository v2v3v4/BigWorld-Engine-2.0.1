/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/xul/base/public/nsIIFrameBoxObject.idl
 */

#ifndef __gen_nsIIFrameBoxObject_h__
#define __gen_nsIIFrameBoxObject_h__


#ifndef __gen_nsIBoxObject_h__
#include "nsIBoxObject.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDocShell; /* forward declaration */


/* starting interface:    nsIIFrameBoxObject */
#define NS_IIFRAMEBOXOBJECT_IID_STR "dd9ab9be-fed3-4bff-a72d-5390d52dd887"

#define NS_IIFRAMEBOXOBJECT_IID \
  {0xdd9ab9be, 0xfed3, 0x4bff, \
    { 0xa7, 0x2d, 0x53, 0x90, 0xd5, 0x2d, 0xd8, 0x87 }}

class NS_NO_VTABLE nsIIFrameBoxObject : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IIFRAMEBOXOBJECT_IID)

  /* readonly attribute nsIDocShell docShell; */
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIIFRAMEBOXOBJECT \
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIIFRAMEBOXOBJECT(_to) \
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell) { return _to GetDocShell(aDocShell); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIIFRAMEBOXOBJECT(_to) \
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDocShell(aDocShell); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsIFrameBoxObject : public nsIIFrameBoxObject
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIIFRAMEBOXOBJECT

  nsIFrameBoxObject();

private:
  ~nsIFrameBoxObject();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsIFrameBoxObject, nsIIFrameBoxObject)

nsIFrameBoxObject::nsIFrameBoxObject()
{
  /* member initializers and constructor code */
}

nsIFrameBoxObject::~nsIFrameBoxObject()
{
  /* destructor code */
}

/* readonly attribute nsIDocShell docShell; */
NS_IMETHODIMP nsIFrameBoxObject::GetDocShell(nsIDocShell * *aDocShell)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

nsresult
NS_NewIFrameBoxObject(nsIBoxObject** aResult);

#endif /* __gen_nsIIFrameBoxObject_h__ */
