/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/xul/base/public/nsIBoxLayoutManager.idl
 */

#ifndef __gen_nsIBoxLayoutManager_h__
#define __gen_nsIBoxLayoutManager_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIBoxLayoutManager */
#define NS_IBOXLAYOUTMANAGER_IID_STR "dc06b890-15a1-45a9-a1a3-f144b42eea29"

#define NS_IBOXLAYOUTMANAGER_IID \
  {0xdc06b890, 0x15a1, 0x45a9, \
    { 0xa1, 0xa3, 0xf1, 0x44, 0xb4, 0x2e, 0xea, 0x29 }}

class NS_NO_VTABLE nsIBoxLayoutManager : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IBOXLAYOUTMANAGER_IID)

  /* void layout (); */
  NS_IMETHOD Layout(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIBOXLAYOUTMANAGER \
  NS_IMETHOD Layout(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIBOXLAYOUTMANAGER(_to) \
  NS_IMETHOD Layout(void) { return _to Layout(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIBOXLAYOUTMANAGER(_to) \
  NS_IMETHOD Layout(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Layout(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsBoxLayoutManager : public nsIBoxLayoutManager
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIBOXLAYOUTMANAGER

  nsBoxLayoutManager();

private:
  ~nsBoxLayoutManager();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsBoxLayoutManager, nsIBoxLayoutManager)

nsBoxLayoutManager::nsBoxLayoutManager()
{
  /* member initializers and constructor code */
}

nsBoxLayoutManager::~nsBoxLayoutManager()
{
  /* destructor code */
}

/* void layout (); */
NS_IMETHODIMP nsBoxLayoutManager::Layout()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

nsresult
NS_NewBoxLayoutManager(nsIBoxLayoutManager** aResult);

#endif /* __gen_nsIBoxLayoutManager_h__ */
