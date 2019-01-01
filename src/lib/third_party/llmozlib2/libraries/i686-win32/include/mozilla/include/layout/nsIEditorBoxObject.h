/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/xul/base/public/nsIEditorBoxObject.idl
 */

#ifndef __gen_nsIEditorBoxObject_h__
#define __gen_nsIEditorBoxObject_h__


#ifndef __gen_nsIBoxObject_h__
#include "nsIBoxObject.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIEditor; /* forward declaration */

class nsIDocShell; /* forward declaration */


/* starting interface:    nsIEditorBoxObject */
#define NS_IEDITORBOXOBJECT_IID_STR "14b3b669-3414-4548-aa03-edf257d889c8"

#define NS_IEDITORBOXOBJECT_IID \
  {0x14b3b669, 0x3414, 0x4548, \
    { 0xaa, 0x03, 0xed, 0xf2, 0x57, 0xd8, 0x89, 0xc8 }}

class NS_NO_VTABLE nsIEditorBoxObject : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IEDITORBOXOBJECT_IID)

  /* readonly attribute nsIDocShell docShell; */
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIEDITORBOXOBJECT \
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIEDITORBOXOBJECT(_to) \
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell) { return _to GetDocShell(aDocShell); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIEDITORBOXOBJECT(_to) \
  NS_IMETHOD GetDocShell(nsIDocShell * *aDocShell) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDocShell(aDocShell); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsEditorBoxObject : public nsIEditorBoxObject
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIEDITORBOXOBJECT

  nsEditorBoxObject();

private:
  ~nsEditorBoxObject();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsEditorBoxObject, nsIEditorBoxObject)

nsEditorBoxObject::nsEditorBoxObject()
{
  /* member initializers and constructor code */
}

nsEditorBoxObject::~nsEditorBoxObject()
{
  /* destructor code */
}

/* readonly attribute nsIDocShell docShell; */
NS_IMETHODIMP nsEditorBoxObject::GetDocShell(nsIDocShell * *aDocShell)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

nsresult
NS_NewEditorBoxObject(nsIBoxObject** aResult);

#endif /* __gen_nsIEditorBoxObject_h__ */
