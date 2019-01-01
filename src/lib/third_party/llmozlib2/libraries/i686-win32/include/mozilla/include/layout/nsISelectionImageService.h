/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/base/nsISelectionImageService.idl
 */

#ifndef __gen_nsISelectionImageService_h__
#define __gen_nsISelectionImageService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class imgIContainer;

/* starting interface:    nsISelectionImageService */
#define NS_ISELECTIONIMAGESERVICE_IID_STR "f6f68e3c-f078-4235-bf71-53d180c37d26"

#define NS_ISELECTIONIMAGESERVICE_IID \
  {0xf6f68e3c, 0xf078, 0x4235, \
    { 0xbf, 0x71, 0x53, 0xd1, 0x80, 0xc3, 0x7d, 0x26 }}

class NS_NO_VTABLE nsISelectionImageService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISELECTIONIMAGESERVICE_IID)

  /**
	* the current image is marked as invalid
	*/
  /* void reset (); */
  NS_IMETHOD Reset(void) = 0;

  /**
	* retrieve the image for alpha blending
	*/
  /* void getImage (in short selectionValue, out imgIContainer container); */
  NS_IMETHOD GetImage(PRInt16 selectionValue, imgIContainer * *container) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISELECTIONIMAGESERVICE \
  NS_IMETHOD Reset(void); \
  NS_IMETHOD GetImage(PRInt16 selectionValue, imgIContainer * *container); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISELECTIONIMAGESERVICE(_to) \
  NS_IMETHOD Reset(void) { return _to Reset(); } \
  NS_IMETHOD GetImage(PRInt16 selectionValue, imgIContainer * *container) { return _to GetImage(selectionValue, container); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISELECTIONIMAGESERVICE(_to) \
  NS_IMETHOD Reset(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Reset(); } \
  NS_IMETHOD GetImage(PRInt16 selectionValue, imgIContainer * *container) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetImage(selectionValue, container); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSelectionImageService : public nsISelectionImageService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISELECTIONIMAGESERVICE

  nsSelectionImageService();

private:
  ~nsSelectionImageService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSelectionImageService, nsISelectionImageService)

nsSelectionImageService::nsSelectionImageService()
{
  /* member initializers and constructor code */
}

nsSelectionImageService::~nsSelectionImageService()
{
  /* destructor code */
}

/* void reset (); */
NS_IMETHODIMP nsSelectionImageService::Reset()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getImage (in short selectionValue, out imgIContainer container); */
NS_IMETHODIMP nsSelectionImageService::GetImage(PRInt16 selectionValue, imgIContainer * *container)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISelectionImageService_h__ */
