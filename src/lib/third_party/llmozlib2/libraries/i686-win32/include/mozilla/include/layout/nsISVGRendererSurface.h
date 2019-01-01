/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/svg/renderer/public/nsISVGRendererSurface.idl
 */

#ifndef __gen_nsISVGRendererSurface_h__
#define __gen_nsISVGRendererSurface_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsISVGRendererSurface */
#define NS_ISVGRENDERERSURFACE_IID_STR "0b3c88dc-2e37-4c20-902f-34f470adf711"

#define NS_ISVGRENDERERSURFACE_IID \
  {0x0b3c88dc, 0x2e37, 0x4c20, \
    { 0x90, 0x2f, 0x34, 0xf4, 0x70, 0xad, 0xf7, 0x11 }}

/**
 * \addtogroup rendering_backend_interfaces Rendering Backend Interfaces
 * @{
 */
/**
 * One of a number of interfaces (all starting with nsISVGRenderer*)
 * to be implemented by an SVG rendering engine. See nsISVGRenderer
 * for more details.
 *
 * This interface abstracts a rendering engine-native surface object.
 */
class NS_NO_VTABLE nsISVGRendererSurface : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGRENDERERSURFACE_IID)

  /* readonly attribute unsigned long width; */
  NS_IMETHOD GetWidth(PRUint32 *aWidth) = 0;

  /* readonly attribute unsigned long height; */
  NS_IMETHOD GetHeight(PRUint32 *aHeight) = 0;

  /* void getData ([array, size_is (length)] out PRUint8 bits, out unsigned long length, out long stride); */
  NS_IMETHOD GetData(PRUint8 **bits, PRUint32 *length, PRInt32 *stride) = 0;

  /* void lock (); */
  NS_IMETHOD Lock(void) = 0;

  /* void unlock (); */
  NS_IMETHOD Unlock(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGRENDERERSURFACE \
  NS_IMETHOD GetWidth(PRUint32 *aWidth); \
  NS_IMETHOD GetHeight(PRUint32 *aHeight); \
  NS_IMETHOD GetData(PRUint8 **bits, PRUint32 *length, PRInt32 *stride); \
  NS_IMETHOD Lock(void); \
  NS_IMETHOD Unlock(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGRENDERERSURFACE(_to) \
  NS_IMETHOD GetWidth(PRUint32 *aWidth) { return _to GetWidth(aWidth); } \
  NS_IMETHOD GetHeight(PRUint32 *aHeight) { return _to GetHeight(aHeight); } \
  NS_IMETHOD GetData(PRUint8 **bits, PRUint32 *length, PRInt32 *stride) { return _to GetData(bits, length, stride); } \
  NS_IMETHOD Lock(void) { return _to Lock(); } \
  NS_IMETHOD Unlock(void) { return _to Unlock(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGRENDERERSURFACE(_to) \
  NS_IMETHOD GetWidth(PRUint32 *aWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetWidth(aWidth); } \
  NS_IMETHOD GetHeight(PRUint32 *aHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHeight(aHeight); } \
  NS_IMETHOD GetData(PRUint8 **bits, PRUint32 *length, PRInt32 *stride) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetData(bits, length, stride); } \
  NS_IMETHOD Lock(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Lock(); } \
  NS_IMETHOD Unlock(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Unlock(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGRendererSurface : public nsISVGRendererSurface
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGRENDERERSURFACE

  nsSVGRendererSurface();

private:
  ~nsSVGRendererSurface();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGRendererSurface, nsISVGRendererSurface)

nsSVGRendererSurface::nsSVGRendererSurface()
{
  /* member initializers and constructor code */
}

nsSVGRendererSurface::~nsSVGRendererSurface()
{
  /* destructor code */
}

/* readonly attribute unsigned long width; */
NS_IMETHODIMP nsSVGRendererSurface::GetWidth(PRUint32 *aWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long height; */
NS_IMETHODIMP nsSVGRendererSurface::GetHeight(PRUint32 *aHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getData ([array, size_is (length)] out PRUint8 bits, out unsigned long length, out long stride); */
NS_IMETHODIMP nsSVGRendererSurface::GetData(PRUint8 **bits, PRUint32 *length, PRInt32 *stride)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void lock (); */
NS_IMETHODIMP nsSVGRendererSurface::Lock()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unlock (); */
NS_IMETHODIMP nsSVGRendererSurface::Unlock()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISVGRendererSurface_h__ */
