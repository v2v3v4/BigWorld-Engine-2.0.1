/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/widget/public/nsIClipboard.idl
 */

#ifndef __gen_nsIClipboard_h__
#define __gen_nsIClipboard_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsISupportsArray_h__
#include "nsISupportsArray.h"
#endif

#ifndef __gen_nsITransferable_h__
#include "nsITransferable.h"
#endif

#ifndef __gen_nsIClipboardOwner_h__
#include "nsIClipboardOwner.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIClipboard */
#define NS_ICLIPBOARD_IID_STR "8b5314ba-db01-11d2-96ce-0060b0fb9956"

#define NS_ICLIPBOARD_IID \
  {0x8b5314ba, 0xdb01, 0x11d2, \
    { 0x96, 0xce, 0x00, 0x60, 0xb0, 0xfb, 0x99, 0x56 }}

class NS_NO_VTABLE nsIClipboard : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICLIPBOARD_IID)

  enum { kSelectionClipboard = 0 };

  enum { kGlobalClipboard = 1 };

  /**
    * Given a transferable, set the data on the native clipboard
    *
    * @param  aTransferable The transferable
    * @param  anOwner The owner of the transferable
    * @param  aWhichClipboard Specifies the clipboard to which this operation applies.
    * @result NS_Ok if no errors
    */
  /* void setData (in nsITransferable aTransferable, in nsIClipboardOwner anOwner, in long aWhichClipboard); */
  NS_IMETHOD SetData(nsITransferable *aTransferable, nsIClipboardOwner *anOwner, PRInt32 aWhichClipboard) = 0;

  /**
    * Given a transferable, get the clipboard data.
    *
    * @param  aTransferable The transferable
    * @param  aWhichClipboard Specifies the clipboard to which this operation applies.
    * @result NS_Ok if no errors
    */
  /* void getData (in nsITransferable aTransferable, in long aWhichClipboard); */
  NS_IMETHOD GetData(nsITransferable *aTransferable, PRInt32 aWhichClipboard) = 0;

  /**
    * This empties the clipboard and notifies the clipboard owner.
    * This empties the "logical" clipboard. It does not clear the native clipboard.
    *
    * @param  aWhichClipboard Specifies the clipboard to which this operation applies.
    * @result NS_OK if successful.
    */
  /* void emptyClipboard (in long aWhichClipboard); */
  NS_IMETHOD EmptyClipboard(PRInt32 aWhichClipboard) = 0;

  /**
    * This provides a way to give correct UI feedback about, for instance, a paste 
    * should be allowed. It does _NOT_ actually retreive the data and should be a very
    * inexpensive call. All it does is check if there is data on the clipboard matching
    * any of the flavors in the given list.
    *
    * @aFlavorList - nsISupportsCString's in a nsISupportsArray (for JavaScript).
    * @param  aWhichClipboard Specifies the clipboard to which this operation applies.
    * @outResult - if data is present matching one of 
    * @result NS_OK if successful.
    */
  /* boolean hasDataMatchingFlavors (in nsISupportsArray aFlavorList, in long aWhichClipboard); */
  NS_IMETHOD HasDataMatchingFlavors(nsISupportsArray *aFlavorList, PRInt32 aWhichClipboard, PRBool *_retval) = 0;

  /**
    * Allows clients to determine if the implementation supports the concept of a 
    * separate clipboard for selection.
    * 
    * @outResult - true if 
    * @result NS_OK if successful.
    */
  /* boolean supportsSelectionClipboard (); */
  NS_IMETHOD SupportsSelectionClipboard(PRBool *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICLIPBOARD \
  NS_IMETHOD SetData(nsITransferable *aTransferable, nsIClipboardOwner *anOwner, PRInt32 aWhichClipboard); \
  NS_IMETHOD GetData(nsITransferable *aTransferable, PRInt32 aWhichClipboard); \
  NS_IMETHOD EmptyClipboard(PRInt32 aWhichClipboard); \
  NS_IMETHOD HasDataMatchingFlavors(nsISupportsArray *aFlavorList, PRInt32 aWhichClipboard, PRBool *_retval); \
  NS_IMETHOD SupportsSelectionClipboard(PRBool *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICLIPBOARD(_to) \
  NS_IMETHOD SetData(nsITransferable *aTransferable, nsIClipboardOwner *anOwner, PRInt32 aWhichClipboard) { return _to SetData(aTransferable, anOwner, aWhichClipboard); } \
  NS_IMETHOD GetData(nsITransferable *aTransferable, PRInt32 aWhichClipboard) { return _to GetData(aTransferable, aWhichClipboard); } \
  NS_IMETHOD EmptyClipboard(PRInt32 aWhichClipboard) { return _to EmptyClipboard(aWhichClipboard); } \
  NS_IMETHOD HasDataMatchingFlavors(nsISupportsArray *aFlavorList, PRInt32 aWhichClipboard, PRBool *_retval) { return _to HasDataMatchingFlavors(aFlavorList, aWhichClipboard, _retval); } \
  NS_IMETHOD SupportsSelectionClipboard(PRBool *_retval) { return _to SupportsSelectionClipboard(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICLIPBOARD(_to) \
  NS_IMETHOD SetData(nsITransferable *aTransferable, nsIClipboardOwner *anOwner, PRInt32 aWhichClipboard) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetData(aTransferable, anOwner, aWhichClipboard); } \
  NS_IMETHOD GetData(nsITransferable *aTransferable, PRInt32 aWhichClipboard) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetData(aTransferable, aWhichClipboard); } \
  NS_IMETHOD EmptyClipboard(PRInt32 aWhichClipboard) { return !_to ? NS_ERROR_NULL_POINTER : _to->EmptyClipboard(aWhichClipboard); } \
  NS_IMETHOD HasDataMatchingFlavors(nsISupportsArray *aFlavorList, PRInt32 aWhichClipboard, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->HasDataMatchingFlavors(aFlavorList, aWhichClipboard, _retval); } \
  NS_IMETHOD SupportsSelectionClipboard(PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SupportsSelectionClipboard(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsClipboard : public nsIClipboard
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICLIPBOARD

  nsClipboard();

private:
  ~nsClipboard();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsClipboard, nsIClipboard)

nsClipboard::nsClipboard()
{
  /* member initializers and constructor code */
}

nsClipboard::~nsClipboard()
{
  /* destructor code */
}

/* void setData (in nsITransferable aTransferable, in nsIClipboardOwner anOwner, in long aWhichClipboard); */
NS_IMETHODIMP nsClipboard::SetData(nsITransferable *aTransferable, nsIClipboardOwner *anOwner, PRInt32 aWhichClipboard)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getData (in nsITransferable aTransferable, in long aWhichClipboard); */
NS_IMETHODIMP nsClipboard::GetData(nsITransferable *aTransferable, PRInt32 aWhichClipboard)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void emptyClipboard (in long aWhichClipboard); */
NS_IMETHODIMP nsClipboard::EmptyClipboard(PRInt32 aWhichClipboard)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean hasDataMatchingFlavors (in nsISupportsArray aFlavorList, in long aWhichClipboard); */
NS_IMETHODIMP nsClipboard::HasDataMatchingFlavors(nsISupportsArray *aFlavorList, PRInt32 aWhichClipboard, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean supportsSelectionClipboard (); */
NS_IMETHODIMP nsClipboard::SupportsSelectionClipboard(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIClipboardImage */
#define NS_ICLIPBOARDIMAGE_IID_STR "db21eb6c-aebb-4d16-94ec-bcd8bbf513ae"

#define NS_ICLIPBOARDIMAGE_IID \
  {0xdb21eb6c, 0xaebb, 0x4d16, \
    { 0x94, 0xec, 0xbc, 0xd8, 0xbb, 0xf5, 0x13, 0xae }}

class NS_NO_VTABLE nsIClipboardImage : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICLIPBOARDIMAGE_IID)

  /* [noscript] void setNativeImage (in voidPtr aNativeImageData); */
  NS_IMETHOD SetNativeImage(void * aNativeImageData) = 0;

  /* [noscript] void getNativeImage (in voidPtr aNativeImageData); */
  NS_IMETHOD GetNativeImage(void * aNativeImageData) = 0;

  /* [noscript] void releaseNativeImage (in voidPtr aNativeImageData); */
  NS_IMETHOD ReleaseNativeImage(void * aNativeImageData) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICLIPBOARDIMAGE \
  NS_IMETHOD SetNativeImage(void * aNativeImageData); \
  NS_IMETHOD GetNativeImage(void * aNativeImageData); \
  NS_IMETHOD ReleaseNativeImage(void * aNativeImageData); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICLIPBOARDIMAGE(_to) \
  NS_IMETHOD SetNativeImage(void * aNativeImageData) { return _to SetNativeImage(aNativeImageData); } \
  NS_IMETHOD GetNativeImage(void * aNativeImageData) { return _to GetNativeImage(aNativeImageData); } \
  NS_IMETHOD ReleaseNativeImage(void * aNativeImageData) { return _to ReleaseNativeImage(aNativeImageData); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICLIPBOARDIMAGE(_to) \
  NS_IMETHOD SetNativeImage(void * aNativeImageData) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetNativeImage(aNativeImageData); } \
  NS_IMETHOD GetNativeImage(void * aNativeImageData) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNativeImage(aNativeImageData); } \
  NS_IMETHOD ReleaseNativeImage(void * aNativeImageData) { return !_to ? NS_ERROR_NULL_POINTER : _to->ReleaseNativeImage(aNativeImageData); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsClipboardImage : public nsIClipboardImage
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICLIPBOARDIMAGE

  nsClipboardImage();

private:
  ~nsClipboardImage();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsClipboardImage, nsIClipboardImage)

nsClipboardImage::nsClipboardImage()
{
  /* member initializers and constructor code */
}

nsClipboardImage::~nsClipboardImage()
{
  /* destructor code */
}

/* [noscript] void setNativeImage (in voidPtr aNativeImageData); */
NS_IMETHODIMP nsClipboardImage::SetNativeImage(void * aNativeImageData)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void getNativeImage (in voidPtr aNativeImageData); */
NS_IMETHODIMP nsClipboardImage::GetNativeImage(void * aNativeImageData)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void releaseNativeImage (in voidPtr aNativeImageData); */
NS_IMETHODIMP nsClipboardImage::ReleaseNativeImage(void * aNativeImageData)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIClipboard_h__ */
