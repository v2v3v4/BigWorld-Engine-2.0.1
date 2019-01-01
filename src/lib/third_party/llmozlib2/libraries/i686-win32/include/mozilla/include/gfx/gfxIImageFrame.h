/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/gfx/idl/gfxIImageFrame.idl
 */

#ifndef __gen_gfxIImageFrame_h__
#define __gen_gfxIImageFrame_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_gfxtypes_h__
#include "gfxtypes.h"
#endif

#ifndef __gen_gfxIFormats_h__
#include "gfxIFormats.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "nsRect.h"

/* starting interface:    gfxIImageFrame */
#define GFXIIMAGEFRAME_IID_STR "f6d00ee7-defc-4101-b2dc-e72cf4c37c3c"

#define GFXIIMAGEFRAME_IID \
  {0xf6d00ee7, 0xdefc, 0x4101, \
    { 0xb2, 0xdc, 0xe7, 0x2c, 0xf4, 0xc3, 0x7c, 0x3c }}

/**
 * gfxIImageFrame interface
 *
 * All x, y, width, height values are in pixels.
 *
 * @author Tim Rowley <tor@cs.brown.edu>
 * @author Stuart Parmenter <pavlov@netscape.com>
 * @version 0.1
 */
class NS_NO_VTABLE gfxIImageFrame : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(GFXIIMAGEFRAME_IID)

  /**
   * Create a new \a aWidth x \a aHeight sized image.
   *
   * @param aX The x-offset from the origin of the gfxIImageContainer parent.
   * @param aY The y-offset from the origin of the gfxIImageContainer parent.
   * @param aWidth The width of the image to create.
   * @param aHeight The height of the image to create.
   * @param aFormat the width of the image to create.
   *
   * @note The data of a new image is unspecified (Whats the word i'm looking for here?).
   */
  /* void init (in PRInt32 aX, in PRInt32 aY, in PRInt32 aWidth, in PRInt32 aHeight, in gfx_format aFormat, in gfx_depth aDepth); */
  NS_IMETHOD Init(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight, gfx_format aFormat, gfx_depth aDepth) = 0;

  /**
   * TRUE by default.  When set to FALSE, you will no longer be able to make any modifications
   * to the data of the image.  Any attempts will fail.
   */
  /* attribute boolean mutable; */
  NS_IMETHOD GetMutable(PRBool *aMutable) = 0;
  NS_IMETHOD SetMutable(PRBool aMutable) = 0;

  /**
   * The x-offset of the image.
   */
  /* readonly attribute PRInt32 x; */
  NS_IMETHOD GetX(PRInt32 *aX) = 0;

  /**
   * The y-offset of the image.
   */
  /* readonly attribute PRInt32 y; */
  NS_IMETHOD GetY(PRInt32 *aY) = 0;

  /**
   * The width of the image.
   */
  /* readonly attribute PRInt32 width; */
  NS_IMETHOD GetWidth(PRInt32 *aWidth) = 0;

  /**
   * The height of the image.
   */
  /* readonly attribute PRInt32 height; */
  NS_IMETHOD GetHeight(PRInt32 *aHeight) = 0;

  /**
   * The rectangle this frame ocupies.
   * @param rect this is really an out parameter.
   */
  /* [noscript] void getRect (in nsRectRef rect); */
  NS_IMETHOD GetRect(nsIntRect & rect) = 0;

  /**
   * The image data format the image was created with.
   * @see gfxIFormats
   */
  /* readonly attribute gfx_format format; */
  NS_IMETHOD GetFormat(gfx_format *aFormat) = 0;

  /**
   * returns whether the image requires the background to be painted
   */
  /* readonly attribute boolean needsBackground; */
  NS_IMETHOD GetNeedsBackground(PRBool *aNeedsBackground) = 0;

  /* readonly attribute unsigned long imageBytesPerRow; */
  NS_IMETHOD GetImageBytesPerRow(PRUint32 *aImageBytesPerRow) = 0;

  /**
   * returns the number of bytes allocated for the image
   */
  /* readonly attribute unsigned long imageDataLength; */
  NS_IMETHOD GetImageDataLength(PRUint32 *aImageDataLength) = 0;

  /* void getImageData ([array, size_is (length)] out PRUint8 bits, out unsigned long length); */
  NS_IMETHOD GetImageData(PRUint8 **bits, PRUint32 *length) = 0;

  /**
   * Sets \a length bytes of \a data in this object.
   * @param offset The offset from the first pixel in bytes.  To set
   *   data beginning with the first (top left) pixel in the image, \a offset
   *   should be 0; to set data beginning with, for example, the sixth pixel in
   *   the first row of a RGBA32 image, the offset should be 20.
   * @attension should we use PRUint32 instead?
   */
  /* void setImageData ([array, size_is (length), const] in PRUint8 data, in unsigned long length, in long offset); */
  NS_IMETHOD SetImageData(const PRUint8 *data, PRUint32 length, PRInt32 offset) = 0;

  /**
   * Lock image pixels before addressing the data directly
   */
  /* void lockImageData (); */
  NS_IMETHOD LockImageData(void) = 0;

  /**
   * Unlock image pixels
   */
  /* void unlockImageData (); */
  NS_IMETHOD UnlockImageData(void) = 0;

  /* readonly attribute unsigned long alphaBytesPerRow; */
  NS_IMETHOD GetAlphaBytesPerRow(PRUint32 *aAlphaBytesPerRow) = 0;

  /**
   * returns the number of bytes allocated for the alpha mask
   */
  /* readonly attribute unsigned long alphaDataLength; */
  NS_IMETHOD GetAlphaDataLength(PRUint32 *aAlphaDataLength) = 0;

  /* void getAlphaData ([array, size_is (length)] out PRUint8 bits, out unsigned long length); */
  NS_IMETHOD GetAlphaData(PRUint8 **bits, PRUint32 *length) = 0;

  /**
   * Sets \a length bytes of \a data in this object.
   */
  /* void setAlphaData ([array, size_is (length), const] in PRUint8 data, in unsigned long length, in long offset); */
  NS_IMETHOD SetAlphaData(const PRUint8 *data, PRUint32 length, PRInt32 offset) = 0;

  /**
   * Lock alpha pixels before addressing the data directly
   */
  /* void lockAlphaData (); */
  NS_IMETHOD LockAlphaData(void) = 0;

  /**
   * Unlock alpha pixels
   */
  /* void unlockAlphaData (); */
  NS_IMETHOD UnlockAlphaData(void) = 0;

  /**
   * Blit this frame into another frame. Used for GIF animation compositing
   */
  /* void drawTo (in gfxIImageFrame aDst, in PRInt32 aDX, in PRInt32 aDY, in PRInt32 aDWidth, in PRInt32 aDHeight); */
  NS_IMETHOD DrawTo(gfxIImageFrame *aDst, PRInt32 aDX, PRInt32 aDY, PRInt32 aDWidth, PRInt32 aDHeight) = 0;

  /**
   * Represents the number of milliseconds until the next frame should be displayed.
   * @note -1 means that this frame should be displayed forever.
   */
  /* attribute long timeout; */
  NS_IMETHOD GetTimeout(PRInt32 *aTimeout) = 0;
  NS_IMETHOD SetTimeout(PRInt32 aTimeout) = 0;

  /* attribute long frameDisposalMethod; */
  NS_IMETHOD GetFrameDisposalMethod(PRInt32 *aFrameDisposalMethod) = 0;
  NS_IMETHOD SetFrameDisposalMethod(PRInt32 aFrameDisposalMethod) = 0;

  /* attribute gfx_color backgroundColor; */
  NS_IMETHOD GetBackgroundColor(gfx_color *aBackgroundColor) = 0;
  NS_IMETHOD SetBackgroundColor(gfx_color aBackgroundColor) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_GFXIIMAGEFRAME \
  NS_IMETHOD Init(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight, gfx_format aFormat, gfx_depth aDepth); \
  NS_IMETHOD GetMutable(PRBool *aMutable); \
  NS_IMETHOD SetMutable(PRBool aMutable); \
  NS_IMETHOD GetX(PRInt32 *aX); \
  NS_IMETHOD GetY(PRInt32 *aY); \
  NS_IMETHOD GetWidth(PRInt32 *aWidth); \
  NS_IMETHOD GetHeight(PRInt32 *aHeight); \
  NS_IMETHOD GetRect(nsIntRect & rect); \
  NS_IMETHOD GetFormat(gfx_format *aFormat); \
  NS_IMETHOD GetNeedsBackground(PRBool *aNeedsBackground); \
  NS_IMETHOD GetImageBytesPerRow(PRUint32 *aImageBytesPerRow); \
  NS_IMETHOD GetImageDataLength(PRUint32 *aImageDataLength); \
  NS_IMETHOD GetImageData(PRUint8 **bits, PRUint32 *length); \
  NS_IMETHOD SetImageData(const PRUint8 *data, PRUint32 length, PRInt32 offset); \
  NS_IMETHOD LockImageData(void); \
  NS_IMETHOD UnlockImageData(void); \
  NS_IMETHOD GetAlphaBytesPerRow(PRUint32 *aAlphaBytesPerRow); \
  NS_IMETHOD GetAlphaDataLength(PRUint32 *aAlphaDataLength); \
  NS_IMETHOD GetAlphaData(PRUint8 **bits, PRUint32 *length); \
  NS_IMETHOD SetAlphaData(const PRUint8 *data, PRUint32 length, PRInt32 offset); \
  NS_IMETHOD LockAlphaData(void); \
  NS_IMETHOD UnlockAlphaData(void); \
  NS_IMETHOD DrawTo(gfxIImageFrame *aDst, PRInt32 aDX, PRInt32 aDY, PRInt32 aDWidth, PRInt32 aDHeight); \
  NS_IMETHOD GetTimeout(PRInt32 *aTimeout); \
  NS_IMETHOD SetTimeout(PRInt32 aTimeout); \
  NS_IMETHOD GetFrameDisposalMethod(PRInt32 *aFrameDisposalMethod); \
  NS_IMETHOD SetFrameDisposalMethod(PRInt32 aFrameDisposalMethod); \
  NS_IMETHOD GetBackgroundColor(gfx_color *aBackgroundColor); \
  NS_IMETHOD SetBackgroundColor(gfx_color aBackgroundColor); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_GFXIIMAGEFRAME(_to) \
  NS_IMETHOD Init(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight, gfx_format aFormat, gfx_depth aDepth) { return _to Init(aX, aY, aWidth, aHeight, aFormat, aDepth); } \
  NS_IMETHOD GetMutable(PRBool *aMutable) { return _to GetMutable(aMutable); } \
  NS_IMETHOD SetMutable(PRBool aMutable) { return _to SetMutable(aMutable); } \
  NS_IMETHOD GetX(PRInt32 *aX) { return _to GetX(aX); } \
  NS_IMETHOD GetY(PRInt32 *aY) { return _to GetY(aY); } \
  NS_IMETHOD GetWidth(PRInt32 *aWidth) { return _to GetWidth(aWidth); } \
  NS_IMETHOD GetHeight(PRInt32 *aHeight) { return _to GetHeight(aHeight); } \
  NS_IMETHOD GetRect(nsIntRect & rect) { return _to GetRect(rect); } \
  NS_IMETHOD GetFormat(gfx_format *aFormat) { return _to GetFormat(aFormat); } \
  NS_IMETHOD GetNeedsBackground(PRBool *aNeedsBackground) { return _to GetNeedsBackground(aNeedsBackground); } \
  NS_IMETHOD GetImageBytesPerRow(PRUint32 *aImageBytesPerRow) { return _to GetImageBytesPerRow(aImageBytesPerRow); } \
  NS_IMETHOD GetImageDataLength(PRUint32 *aImageDataLength) { return _to GetImageDataLength(aImageDataLength); } \
  NS_IMETHOD GetImageData(PRUint8 **bits, PRUint32 *length) { return _to GetImageData(bits, length); } \
  NS_IMETHOD SetImageData(const PRUint8 *data, PRUint32 length, PRInt32 offset) { return _to SetImageData(data, length, offset); } \
  NS_IMETHOD LockImageData(void) { return _to LockImageData(); } \
  NS_IMETHOD UnlockImageData(void) { return _to UnlockImageData(); } \
  NS_IMETHOD GetAlphaBytesPerRow(PRUint32 *aAlphaBytesPerRow) { return _to GetAlphaBytesPerRow(aAlphaBytesPerRow); } \
  NS_IMETHOD GetAlphaDataLength(PRUint32 *aAlphaDataLength) { return _to GetAlphaDataLength(aAlphaDataLength); } \
  NS_IMETHOD GetAlphaData(PRUint8 **bits, PRUint32 *length) { return _to GetAlphaData(bits, length); } \
  NS_IMETHOD SetAlphaData(const PRUint8 *data, PRUint32 length, PRInt32 offset) { return _to SetAlphaData(data, length, offset); } \
  NS_IMETHOD LockAlphaData(void) { return _to LockAlphaData(); } \
  NS_IMETHOD UnlockAlphaData(void) { return _to UnlockAlphaData(); } \
  NS_IMETHOD DrawTo(gfxIImageFrame *aDst, PRInt32 aDX, PRInt32 aDY, PRInt32 aDWidth, PRInt32 aDHeight) { return _to DrawTo(aDst, aDX, aDY, aDWidth, aDHeight); } \
  NS_IMETHOD GetTimeout(PRInt32 *aTimeout) { return _to GetTimeout(aTimeout); } \
  NS_IMETHOD SetTimeout(PRInt32 aTimeout) { return _to SetTimeout(aTimeout); } \
  NS_IMETHOD GetFrameDisposalMethod(PRInt32 *aFrameDisposalMethod) { return _to GetFrameDisposalMethod(aFrameDisposalMethod); } \
  NS_IMETHOD SetFrameDisposalMethod(PRInt32 aFrameDisposalMethod) { return _to SetFrameDisposalMethod(aFrameDisposalMethod); } \
  NS_IMETHOD GetBackgroundColor(gfx_color *aBackgroundColor) { return _to GetBackgroundColor(aBackgroundColor); } \
  NS_IMETHOD SetBackgroundColor(gfx_color aBackgroundColor) { return _to SetBackgroundColor(aBackgroundColor); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_GFXIIMAGEFRAME(_to) \
  NS_IMETHOD Init(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight, gfx_format aFormat, gfx_depth aDepth) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(aX, aY, aWidth, aHeight, aFormat, aDepth); } \
  NS_IMETHOD GetMutable(PRBool *aMutable) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMutable(aMutable); } \
  NS_IMETHOD SetMutable(PRBool aMutable) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMutable(aMutable); } \
  NS_IMETHOD GetX(PRInt32 *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD GetY(PRInt32 *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD GetWidth(PRInt32 *aWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetWidth(aWidth); } \
  NS_IMETHOD GetHeight(PRInt32 *aHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHeight(aHeight); } \
  NS_IMETHOD GetRect(nsIntRect & rect) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRect(rect); } \
  NS_IMETHOD GetFormat(gfx_format *aFormat) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFormat(aFormat); } \
  NS_IMETHOD GetNeedsBackground(PRBool *aNeedsBackground) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNeedsBackground(aNeedsBackground); } \
  NS_IMETHOD GetImageBytesPerRow(PRUint32 *aImageBytesPerRow) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetImageBytesPerRow(aImageBytesPerRow); } \
  NS_IMETHOD GetImageDataLength(PRUint32 *aImageDataLength) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetImageDataLength(aImageDataLength); } \
  NS_IMETHOD GetImageData(PRUint8 **bits, PRUint32 *length) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetImageData(bits, length); } \
  NS_IMETHOD SetImageData(const PRUint8 *data, PRUint32 length, PRInt32 offset) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetImageData(data, length, offset); } \
  NS_IMETHOD LockImageData(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->LockImageData(); } \
  NS_IMETHOD UnlockImageData(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnlockImageData(); } \
  NS_IMETHOD GetAlphaBytesPerRow(PRUint32 *aAlphaBytesPerRow) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAlphaBytesPerRow(aAlphaBytesPerRow); } \
  NS_IMETHOD GetAlphaDataLength(PRUint32 *aAlphaDataLength) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAlphaDataLength(aAlphaDataLength); } \
  NS_IMETHOD GetAlphaData(PRUint8 **bits, PRUint32 *length) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAlphaData(bits, length); } \
  NS_IMETHOD SetAlphaData(const PRUint8 *data, PRUint32 length, PRInt32 offset) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetAlphaData(data, length, offset); } \
  NS_IMETHOD LockAlphaData(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->LockAlphaData(); } \
  NS_IMETHOD UnlockAlphaData(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnlockAlphaData(); } \
  NS_IMETHOD DrawTo(gfxIImageFrame *aDst, PRInt32 aDX, PRInt32 aDY, PRInt32 aDWidth, PRInt32 aDHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->DrawTo(aDst, aDX, aDY, aDWidth, aDHeight); } \
  NS_IMETHOD GetTimeout(PRInt32 *aTimeout) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTimeout(aTimeout); } \
  NS_IMETHOD SetTimeout(PRInt32 aTimeout) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTimeout(aTimeout); } \
  NS_IMETHOD GetFrameDisposalMethod(PRInt32 *aFrameDisposalMethod) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFrameDisposalMethod(aFrameDisposalMethod); } \
  NS_IMETHOD SetFrameDisposalMethod(PRInt32 aFrameDisposalMethod) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFrameDisposalMethod(aFrameDisposalMethod); } \
  NS_IMETHOD GetBackgroundColor(gfx_color *aBackgroundColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBackgroundColor(aBackgroundColor); } \
  NS_IMETHOD SetBackgroundColor(gfx_color aBackgroundColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBackgroundColor(aBackgroundColor); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public gfxIImageFrame
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_GFXIIMAGEFRAME

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(_MYCLASS_, gfxIImageFrame)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void init (in PRInt32 aX, in PRInt32 aY, in PRInt32 aWidth, in PRInt32 aHeight, in gfx_format aFormat, in gfx_depth aDepth); */
NS_IMETHODIMP _MYCLASS_::Init(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight, gfx_format aFormat, gfx_depth aDepth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean mutable; */
NS_IMETHODIMP _MYCLASS_::GetMutable(PRBool *aMutable)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP _MYCLASS_::SetMutable(PRBool aMutable)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRInt32 x; */
NS_IMETHODIMP _MYCLASS_::GetX(PRInt32 *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRInt32 y; */
NS_IMETHODIMP _MYCLASS_::GetY(PRInt32 *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRInt32 width; */
NS_IMETHODIMP _MYCLASS_::GetWidth(PRInt32 *aWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRInt32 height; */
NS_IMETHODIMP _MYCLASS_::GetHeight(PRInt32 *aHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void getRect (in nsRectRef rect); */
NS_IMETHODIMP _MYCLASS_::GetRect(nsIntRect & rect)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute gfx_format format; */
NS_IMETHODIMP _MYCLASS_::GetFormat(gfx_format *aFormat)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean needsBackground; */
NS_IMETHODIMP _MYCLASS_::GetNeedsBackground(PRBool *aNeedsBackground)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long imageBytesPerRow; */
NS_IMETHODIMP _MYCLASS_::GetImageBytesPerRow(PRUint32 *aImageBytesPerRow)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long imageDataLength; */
NS_IMETHODIMP _MYCLASS_::GetImageDataLength(PRUint32 *aImageDataLength)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getImageData ([array, size_is (length)] out PRUint8 bits, out unsigned long length); */
NS_IMETHODIMP _MYCLASS_::GetImageData(PRUint8 **bits, PRUint32 *length)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setImageData ([array, size_is (length), const] in PRUint8 data, in unsigned long length, in long offset); */
NS_IMETHODIMP _MYCLASS_::SetImageData(const PRUint8 *data, PRUint32 length, PRInt32 offset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void lockImageData (); */
NS_IMETHODIMP _MYCLASS_::LockImageData()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unlockImageData (); */
NS_IMETHODIMP _MYCLASS_::UnlockImageData()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long alphaBytesPerRow; */
NS_IMETHODIMP _MYCLASS_::GetAlphaBytesPerRow(PRUint32 *aAlphaBytesPerRow)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long alphaDataLength; */
NS_IMETHODIMP _MYCLASS_::GetAlphaDataLength(PRUint32 *aAlphaDataLength)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getAlphaData ([array, size_is (length)] out PRUint8 bits, out unsigned long length); */
NS_IMETHODIMP _MYCLASS_::GetAlphaData(PRUint8 **bits, PRUint32 *length)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setAlphaData ([array, size_is (length), const] in PRUint8 data, in unsigned long length, in long offset); */
NS_IMETHODIMP _MYCLASS_::SetAlphaData(const PRUint8 *data, PRUint32 length, PRInt32 offset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void lockAlphaData (); */
NS_IMETHODIMP _MYCLASS_::LockAlphaData()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unlockAlphaData (); */
NS_IMETHODIMP _MYCLASS_::UnlockAlphaData()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void drawTo (in gfxIImageFrame aDst, in PRInt32 aDX, in PRInt32 aDY, in PRInt32 aDWidth, in PRInt32 aDHeight); */
NS_IMETHODIMP _MYCLASS_::DrawTo(gfxIImageFrame *aDst, PRInt32 aDX, PRInt32 aDY, PRInt32 aDWidth, PRInt32 aDHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long timeout; */
NS_IMETHODIMP _MYCLASS_::GetTimeout(PRInt32 *aTimeout)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP _MYCLASS_::SetTimeout(PRInt32 aTimeout)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long frameDisposalMethod; */
NS_IMETHODIMP _MYCLASS_::GetFrameDisposalMethod(PRInt32 *aFrameDisposalMethod)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP _MYCLASS_::SetFrameDisposalMethod(PRInt32 aFrameDisposalMethod)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute gfx_color backgroundColor; */
NS_IMETHODIMP _MYCLASS_::GetBackgroundColor(gfx_color *aBackgroundColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP _MYCLASS_::SetBackgroundColor(gfx_color aBackgroundColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_gfxIImageFrame_h__ */
