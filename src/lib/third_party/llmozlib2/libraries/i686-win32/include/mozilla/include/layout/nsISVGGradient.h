/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/svg/renderer/public/nsISVGGradient.idl
 */

#ifndef __gen_nsISVGGradient_h__
#define __gen_nsISVGGradient_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "nsColor.h"
class nsIDOMSVGMatrix; /* forward declaration */

class nsISVGLinearGradient; /* forward declaration */

class nsISVGRadialGradient; /* forward declaration */

class nsIFrame; /* forward declaration */

class nsISVGGeometrySource; /* forward declaration */


/* starting interface:    nsISVGGradient */
#define NS_ISVGGRADIENT_IID_STR "62e79ab2-5bf9-4372-b397-7a942bc4c649"

#define NS_ISVGGRADIENT_IID \
  {0x62e79ab2, 0x5bf9, 0x4372, \
    { 0xb3, 0x97, 0x7a, 0x94, 0x2b, 0xc4, 0xc6, 0x49 }}

/**
 * \addtogroup rendering_backend_interfaces Rendering Backend Interfaces
 * @{
 */
/**
 * Describes the 'gradient' objects (either linear or a radial) to the
 * rendering backends.
 *
 * @nosubgrouping
 */
class NS_NO_VTABLE nsISVGGradient : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGGRADIENT_IID)

  enum { SVG_UNKNOWN_GRADIENT = 0U };

  enum { SVG_LINEAR_GRADIENT = 1U };

  enum { SVG_RADIAL_GRADIENT = 2U };

  /* readonly attribute PRUint32 gradientType; */
  NS_IMETHOD GetGradientType(PRUint32 *aGradientType) = 0;

  /* readonly attribute PRUint16 gradientUnits; */
  NS_IMETHOD GetGradientUnits(PRUint16 *aGradientUnits) = 0;

  /* readonly attribute PRUint16 spreadMethod; */
  NS_IMETHOD GetSpreadMethod(PRUint16 *aSpreadMethod) = 0;

  /* void GetStopCount (out PRUint32 aStopCount); */
  NS_IMETHOD GetStopCount(PRUint32 *aStopCount) = 0;

  /* void GetStopOffset (in PRInt32 aIndex, out float aOffset); */
  NS_IMETHOD GetStopOffset(PRInt32 aIndex, float *aOffset) = 0;

  /* void GetStopColor (in PRInt32 aIndex, out nscolor aStopColor); */
  NS_IMETHOD GetStopColor(PRInt32 aIndex, nscolor *aStopColor) = 0;

  /* void GetStopOpacity (in PRInt32 aIndex, out float aStopOpacity); */
  NS_IMETHOD GetStopOpacity(PRInt32 aIndex, float *aStopOpacity) = 0;

  /* void GetNextGradient (out nsISVGGradient aNextGrad, in PRUint32 aType); */
  NS_IMETHOD GetNextGradient(nsISVGGradient **aNextGrad, PRUint32 aType) = 0;

  /* void GetGradientTransform (out nsIDOMSVGMatrix retval, in nsISVGGeometrySource aSource); */
  NS_IMETHOD GetGradientTransform(nsIDOMSVGMatrix **retval, nsISVGGeometrySource *aSource) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGGRADIENT \
  NS_IMETHOD GetGradientType(PRUint32 *aGradientType); \
  NS_IMETHOD GetGradientUnits(PRUint16 *aGradientUnits); \
  NS_IMETHOD GetSpreadMethod(PRUint16 *aSpreadMethod); \
  NS_IMETHOD GetStopCount(PRUint32 *aStopCount); \
  NS_IMETHOD GetStopOffset(PRInt32 aIndex, float *aOffset); \
  NS_IMETHOD GetStopColor(PRInt32 aIndex, nscolor *aStopColor); \
  NS_IMETHOD GetStopOpacity(PRInt32 aIndex, float *aStopOpacity); \
  NS_IMETHOD GetNextGradient(nsISVGGradient **aNextGrad, PRUint32 aType); \
  NS_IMETHOD GetGradientTransform(nsIDOMSVGMatrix **retval, nsISVGGeometrySource *aSource); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGGRADIENT(_to) \
  NS_IMETHOD GetGradientType(PRUint32 *aGradientType) { return _to GetGradientType(aGradientType); } \
  NS_IMETHOD GetGradientUnits(PRUint16 *aGradientUnits) { return _to GetGradientUnits(aGradientUnits); } \
  NS_IMETHOD GetSpreadMethod(PRUint16 *aSpreadMethod) { return _to GetSpreadMethod(aSpreadMethod); } \
  NS_IMETHOD GetStopCount(PRUint32 *aStopCount) { return _to GetStopCount(aStopCount); } \
  NS_IMETHOD GetStopOffset(PRInt32 aIndex, float *aOffset) { return _to GetStopOffset(aIndex, aOffset); } \
  NS_IMETHOD GetStopColor(PRInt32 aIndex, nscolor *aStopColor) { return _to GetStopColor(aIndex, aStopColor); } \
  NS_IMETHOD GetStopOpacity(PRInt32 aIndex, float *aStopOpacity) { return _to GetStopOpacity(aIndex, aStopOpacity); } \
  NS_IMETHOD GetNextGradient(nsISVGGradient **aNextGrad, PRUint32 aType) { return _to GetNextGradient(aNextGrad, aType); } \
  NS_IMETHOD GetGradientTransform(nsIDOMSVGMatrix **retval, nsISVGGeometrySource *aSource) { return _to GetGradientTransform(retval, aSource); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGGRADIENT(_to) \
  NS_IMETHOD GetGradientType(PRUint32 *aGradientType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetGradientType(aGradientType); } \
  NS_IMETHOD GetGradientUnits(PRUint16 *aGradientUnits) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetGradientUnits(aGradientUnits); } \
  NS_IMETHOD GetSpreadMethod(PRUint16 *aSpreadMethod) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSpreadMethod(aSpreadMethod); } \
  NS_IMETHOD GetStopCount(PRUint32 *aStopCount) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStopCount(aStopCount); } \
  NS_IMETHOD GetStopOffset(PRInt32 aIndex, float *aOffset) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStopOffset(aIndex, aOffset); } \
  NS_IMETHOD GetStopColor(PRInt32 aIndex, nscolor *aStopColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStopColor(aIndex, aStopColor); } \
  NS_IMETHOD GetStopOpacity(PRInt32 aIndex, float *aStopOpacity) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStopOpacity(aIndex, aStopOpacity); } \
  NS_IMETHOD GetNextGradient(nsISVGGradient **aNextGrad, PRUint32 aType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNextGradient(aNextGrad, aType); } \
  NS_IMETHOD GetGradientTransform(nsIDOMSVGMatrix **retval, nsISVGGeometrySource *aSource) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetGradientTransform(retval, aSource); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGGradient : public nsISVGGradient
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGGRADIENT

  nsSVGGradient();

private:
  ~nsSVGGradient();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGGradient, nsISVGGradient)

nsSVGGradient::nsSVGGradient()
{
  /* member initializers and constructor code */
}

nsSVGGradient::~nsSVGGradient()
{
  /* destructor code */
}

/* readonly attribute PRUint32 gradientType; */
NS_IMETHODIMP nsSVGGradient::GetGradientType(PRUint32 *aGradientType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRUint16 gradientUnits; */
NS_IMETHODIMP nsSVGGradient::GetGradientUnits(PRUint16 *aGradientUnits)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRUint16 spreadMethod; */
NS_IMETHODIMP nsSVGGradient::GetSpreadMethod(PRUint16 *aSpreadMethod)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void GetStopCount (out PRUint32 aStopCount); */
NS_IMETHODIMP nsSVGGradient::GetStopCount(PRUint32 *aStopCount)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void GetStopOffset (in PRInt32 aIndex, out float aOffset); */
NS_IMETHODIMP nsSVGGradient::GetStopOffset(PRInt32 aIndex, float *aOffset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void GetStopColor (in PRInt32 aIndex, out nscolor aStopColor); */
NS_IMETHODIMP nsSVGGradient::GetStopColor(PRInt32 aIndex, nscolor *aStopColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void GetStopOpacity (in PRInt32 aIndex, out float aStopOpacity); */
NS_IMETHODIMP nsSVGGradient::GetStopOpacity(PRInt32 aIndex, float *aStopOpacity)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void GetNextGradient (out nsISVGGradient aNextGrad, in PRUint32 aType); */
NS_IMETHODIMP nsSVGGradient::GetNextGradient(nsISVGGradient **aNextGrad, PRUint32 aType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void GetGradientTransform (out nsIDOMSVGMatrix retval, in nsISVGGeometrySource aSource); */
NS_IMETHODIMP nsSVGGradient::GetGradientTransform(nsIDOMSVGMatrix **retval, nsISVGGeometrySource *aSource)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsISVGLinearGradient */
#define NS_ISVGLINEARGRADIENT_IID_STR "995ad9e6-6bb1-47c5-b402-fc93ce12f5e7"

#define NS_ISVGLINEARGRADIENT_IID \
  {0x995ad9e6, 0x6bb1, 0x47c5, \
    { 0xb4, 0x02, 0xfc, 0x93, 0xce, 0x12, 0xf5, 0xe7 }}

class NS_NO_VTABLE nsISVGLinearGradient : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGLINEARGRADIENT_IID)

  /** @} */
  /* readonly attribute float X1; */
  NS_IMETHOD GetX1(float *aX1) = 0;

  /* readonly attribute float X2; */
  NS_IMETHOD GetX2(float *aX2) = 0;

  /* readonly attribute float Y1; */
  NS_IMETHOD GetY1(float *aY1) = 0;

  /* readonly attribute float Y2; */
  NS_IMETHOD GetY2(float *aY2) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGLINEARGRADIENT \
  NS_IMETHOD GetX1(float *aX1); \
  NS_IMETHOD GetX2(float *aX2); \
  NS_IMETHOD GetY1(float *aY1); \
  NS_IMETHOD GetY2(float *aY2); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGLINEARGRADIENT(_to) \
  NS_IMETHOD GetX1(float *aX1) { return _to GetX1(aX1); } \
  NS_IMETHOD GetX2(float *aX2) { return _to GetX2(aX2); } \
  NS_IMETHOD GetY1(float *aY1) { return _to GetY1(aY1); } \
  NS_IMETHOD GetY2(float *aY2) { return _to GetY2(aY2); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGLINEARGRADIENT(_to) \
  NS_IMETHOD GetX1(float *aX1) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX1(aX1); } \
  NS_IMETHOD GetX2(float *aX2) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX2(aX2); } \
  NS_IMETHOD GetY1(float *aY1) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY1(aY1); } \
  NS_IMETHOD GetY2(float *aY2) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY2(aY2); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGLinearGradient : public nsISVGLinearGradient
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGLINEARGRADIENT

  nsSVGLinearGradient();

private:
  ~nsSVGLinearGradient();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGLinearGradient, nsISVGLinearGradient)

nsSVGLinearGradient::nsSVGLinearGradient()
{
  /* member initializers and constructor code */
}

nsSVGLinearGradient::~nsSVGLinearGradient()
{
  /* destructor code */
}

/* readonly attribute float X1; */
NS_IMETHODIMP nsSVGLinearGradient::GetX1(float *aX1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float X2; */
NS_IMETHODIMP nsSVGLinearGradient::GetX2(float *aX2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float Y1; */
NS_IMETHODIMP nsSVGLinearGradient::GetY1(float *aY1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float Y2; */
NS_IMETHODIMP nsSVGLinearGradient::GetY2(float *aY2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsISVGRadialGradient */
#define NS_ISVGRADIALGRADIENT_IID_STR "446435ff-6699-4b4d-85c1-09c18145f5ce"

#define NS_ISVGRADIALGRADIENT_IID \
  {0x446435ff, 0x6699, 0x4b4d, \
    { 0x85, 0xc1, 0x09, 0xc1, 0x81, 0x45, 0xf5, 0xce }}

class NS_NO_VTABLE nsISVGRadialGradient : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGRADIALGRADIENT_IID)

  /** @} */
  /* readonly attribute float Cx; */
  NS_IMETHOD GetCx(float *aCx) = 0;

  /* readonly attribute float Cy; */
  NS_IMETHOD GetCy(float *aCy) = 0;

  /* readonly attribute float R; */
  NS_IMETHOD GetR(float *aR) = 0;

  /* readonly attribute float Fx; */
  NS_IMETHOD GetFx(float *aFx) = 0;

  /* readonly attribute float Fy; */
  NS_IMETHOD GetFy(float *aFy) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGRADIALGRADIENT \
  NS_IMETHOD GetCx(float *aCx); \
  NS_IMETHOD GetCy(float *aCy); \
  NS_IMETHOD GetR(float *aR); \
  NS_IMETHOD GetFx(float *aFx); \
  NS_IMETHOD GetFy(float *aFy); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGRADIALGRADIENT(_to) \
  NS_IMETHOD GetCx(float *aCx) { return _to GetCx(aCx); } \
  NS_IMETHOD GetCy(float *aCy) { return _to GetCy(aCy); } \
  NS_IMETHOD GetR(float *aR) { return _to GetR(aR); } \
  NS_IMETHOD GetFx(float *aFx) { return _to GetFx(aFx); } \
  NS_IMETHOD GetFy(float *aFy) { return _to GetFy(aFy); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGRADIALGRADIENT(_to) \
  NS_IMETHOD GetCx(float *aCx) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCx(aCx); } \
  NS_IMETHOD GetCy(float *aCy) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCy(aCy); } \
  NS_IMETHOD GetR(float *aR) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetR(aR); } \
  NS_IMETHOD GetFx(float *aFx) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFx(aFx); } \
  NS_IMETHOD GetFy(float *aFy) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFy(aFy); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGRadialGradient : public nsISVGRadialGradient
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGRADIALGRADIENT

  nsSVGRadialGradient();

private:
  ~nsSVGRadialGradient();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGRadialGradient, nsISVGRadialGradient)

nsSVGRadialGradient::nsSVGRadialGradient()
{
  /* member initializers and constructor code */
}

nsSVGRadialGradient::~nsSVGRadialGradient()
{
  /* destructor code */
}

/* readonly attribute float Cx; */
NS_IMETHODIMP nsSVGRadialGradient::GetCx(float *aCx)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float Cy; */
NS_IMETHODIMP nsSVGRadialGradient::GetCy(float *aCy)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float R; */
NS_IMETHODIMP nsSVGRadialGradient::GetR(float *aR)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float Fx; */
NS_IMETHODIMP nsSVGRadialGradient::GetFx(float *aFx)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float Fy; */
NS_IMETHODIMP nsSVGRadialGradient::GetFy(float *aFy)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISVGGradient_h__ */
