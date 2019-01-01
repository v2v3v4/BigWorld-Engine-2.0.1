/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/gfx/idl/nsIScreenManager.idl
 */

#ifndef __gen_nsIScreenManager_h__
#define __gen_nsIScreenManager_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsIScreen_h__
#include "nsIScreen.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIScreenManager */
#define NS_ISCREENMANAGER_IID_STR "662e7b78-1dd2-11b2-a3d3-fc1e5f5fb9d4"

#define NS_ISCREENMANAGER_IID \
  {0x662e7b78, 0x1dd2, 0x11b2, \
    { 0xa3, 0xd3, 0xfc, 0x1e, 0x5f, 0x5f, 0xb9, 0xd4 }}

class NS_NO_VTABLE nsIScreenManager : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISCREENMANAGER_IID)

  /* nsIScreen screenForRect (in long left, in long top, in long width, in long height); */
  NS_IMETHOD ScreenForRect(PRInt32 left, PRInt32 top, PRInt32 width, PRInt32 height, nsIScreen **_retval) = 0;

  /* readonly attribute nsIScreen primaryScreen; */
  NS_IMETHOD GetPrimaryScreen(nsIScreen * *aPrimaryScreen) = 0;

  /* readonly attribute unsigned long numberOfScreens; */
  NS_IMETHOD GetNumberOfScreens(PRUint32 *aNumberOfScreens) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISCREENMANAGER \
  NS_IMETHOD ScreenForRect(PRInt32 left, PRInt32 top, PRInt32 width, PRInt32 height, nsIScreen **_retval); \
  NS_IMETHOD GetPrimaryScreen(nsIScreen * *aPrimaryScreen); \
  NS_IMETHOD GetNumberOfScreens(PRUint32 *aNumberOfScreens); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISCREENMANAGER(_to) \
  NS_IMETHOD ScreenForRect(PRInt32 left, PRInt32 top, PRInt32 width, PRInt32 height, nsIScreen **_retval) { return _to ScreenForRect(left, top, width, height, _retval); } \
  NS_IMETHOD GetPrimaryScreen(nsIScreen * *aPrimaryScreen) { return _to GetPrimaryScreen(aPrimaryScreen); } \
  NS_IMETHOD GetNumberOfScreens(PRUint32 *aNumberOfScreens) { return _to GetNumberOfScreens(aNumberOfScreens); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISCREENMANAGER(_to) \
  NS_IMETHOD ScreenForRect(PRInt32 left, PRInt32 top, PRInt32 width, PRInt32 height, nsIScreen **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ScreenForRect(left, top, width, height, _retval); } \
  NS_IMETHOD GetPrimaryScreen(nsIScreen * *aPrimaryScreen) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrimaryScreen(aPrimaryScreen); } \
  NS_IMETHOD GetNumberOfScreens(PRUint32 *aNumberOfScreens) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNumberOfScreens(aNumberOfScreens); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsScreenManager : public nsIScreenManager
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISCREENMANAGER

  nsScreenManager();

private:
  ~nsScreenManager();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsScreenManager, nsIScreenManager)

nsScreenManager::nsScreenManager()
{
  /* member initializers and constructor code */
}

nsScreenManager::~nsScreenManager()
{
  /* destructor code */
}

/* nsIScreen screenForRect (in long left, in long top, in long width, in long height); */
NS_IMETHODIMP nsScreenManager::ScreenForRect(PRInt32 left, PRInt32 top, PRInt32 width, PRInt32 height, nsIScreen **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIScreen primaryScreen; */
NS_IMETHODIMP nsScreenManager::GetPrimaryScreen(nsIScreen * *aPrimaryScreen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long numberOfScreens; */
NS_IMETHODIMP nsScreenManager::GetNumberOfScreens(PRUint32 *aNumberOfScreens)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIScreenManager_h__ */
