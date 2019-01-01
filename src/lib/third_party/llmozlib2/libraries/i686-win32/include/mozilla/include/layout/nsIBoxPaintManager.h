/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/xul/base/public/nsIBoxPaintManager.idl
 */

#ifndef __gen_nsIBoxPaintManager_h__
#define __gen_nsIBoxPaintManager_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIBoxPaintManager */
#define NS_IBOXPAINTMANAGER_IID_STR "5da1e8f0-6255-4b9c-af80-aa3dad7f2fdb"

#define NS_IBOXPAINTMANAGER_IID \
  {0x5da1e8f0, 0x6255, 0x4b9c, \
    { 0xaf, 0x80, 0xaa, 0x3d, 0xad, 0x7f, 0x2f, 0xdb }}

class NS_NO_VTABLE nsIBoxPaintManager : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IBOXPAINTMANAGER_IID)

  /* void paint (); */
  NS_IMETHOD Paint(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIBOXPAINTMANAGER \
  NS_IMETHOD Paint(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIBOXPAINTMANAGER(_to) \
  NS_IMETHOD Paint(void) { return _to Paint(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIBOXPAINTMANAGER(_to) \
  NS_IMETHOD Paint(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Paint(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsBoxPaintManager : public nsIBoxPaintManager
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIBOXPAINTMANAGER

  nsBoxPaintManager();

private:
  ~nsBoxPaintManager();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsBoxPaintManager, nsIBoxPaintManager)

nsBoxPaintManager::nsBoxPaintManager()
{
  /* member initializers and constructor code */
}

nsBoxPaintManager::~nsBoxPaintManager()
{
  /* destructor code */
}

/* void paint (); */
NS_IMETHODIMP nsBoxPaintManager::Paint()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

nsresult
NS_NewBoxPaintManager(nsIBoxPaintManager** aResult);

#endif /* __gen_nsIBoxPaintManager_h__ */
