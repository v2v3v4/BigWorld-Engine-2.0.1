/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/xul/base/public/nsIBrowserBoxObject.idl
 */

#ifndef __gen_nsIBrowserBoxObject_h__
#define __gen_nsIBrowserBoxObject_h__


#ifndef __gen_nsIBoxObject_h__
#include "nsIBoxObject.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDocShell; /* forward declaration */


/* starting interface:    nsIBrowserBoxObject */
#define NS_IBROWSERBOXOBJECT_IID_STR "f2504c26-7cf5-426a-86a7-e50998ac57c1"

#define NS_IBROWSERBOXOBJECT_IID \
  {0xf2504c26, 0x7cf5, 0x426a, \
    { 0x86, 0xa7, 0xe5, 0x09, 0x98, 0xac, 0x57, 0xc1 }}

class NS_NO_VTABLE nsIBrowserBoxObject : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IBROWSERBOXOBJECT_IID)

  /* readonly attribute nsIDocShell docShell; */
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIBROWSERBOXOBJECT \
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIBROWSERBOXOBJECT(_to) \
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell) { return _to GetDocShell(aDocShell); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIBROWSERBOXOBJECT(_to) \
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDocShell(aDocShell); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsBrowserBoxObject : public nsIBrowserBoxObject
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIBROWSERBOXOBJECT

  nsBrowserBoxObject();

private:
  ~nsBrowserBoxObject();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsBrowserBoxObject, nsIBrowserBoxObject)

nsBrowserBoxObject::nsBrowserBoxObject()
{
  /* member initializers and constructor code */
}

nsBrowserBoxObject::~nsBrowserBoxObject()
{
  /* destructor code */
}

/* readonly attribute nsIDocShell docShell; */
NS_IMETHODIMP nsBrowserBoxObject::GetDocShell(nsIDocShell * *aDocShell)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

nsresult
NS_NewBrowserBoxObject(nsIBoxObject** aResult);

#endif /* __gen_nsIBrowserBoxObject_h__ */
