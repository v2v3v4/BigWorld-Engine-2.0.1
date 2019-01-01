/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/svg/renderer/public/nsISVGGeometrySource.idl
 */

#ifndef __gen_nsISVGGeometrySource_h__
#define __gen_nsISVGGeometrySource_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "nsColor.h"
#include "nsIURI.h"
class nsIDOMSVGMatrix; /* forward declaration */

class nsPresContext; /* forward declaration */

class nsIURI; /* forward declaration */

class nsISVGGradient; /* forward declaration */


/* starting interface:    nsISVGGeometrySource */
#define NS_ISVGGEOMETRYSOURCE_IID_STR "b2c3119b-a27d-4b25-97a9-a9d60981a95e"

#define NS_ISVGGEOMETRYSOURCE_IID \
  {0xb2c3119b, 0xa27d, 0x4b25, \
    { 0x97, 0xa9, 0xa9, 0xd6, 0x09, 0x81, 0xa9, 0x5e }}

/**
 * \addtogroup rendering_backend_interfaces Rendering Backend Interfaces
 * @{
 */
/**
 * Describes a 'geometry' object (either a path or a glyph) in the SVG
 * rendering backend. The rendering backend maintains an object
 * implementing this interface for each rendering engine-native
 * geometry object.
 *
 * An engine-native geometry object will be informed of changes in a
 * geometry's description with a call to its 'update' method with an
 * OR-ed combination of the UPDATEMASK_* constants defined in this
 * interface (or one of its sub-interfaces).
 *
 * @nosubgrouping
 */
class NS_NO_VTABLE nsISVGGeometrySource : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGGEOMETRYSOURCE_IID)

  /**
   * @name Generic updatemasks
   * @{
   */
  enum { UPDATEMASK_NOTHING = 0U };

  enum { UPDATEMASK_ALL = 4294967295U };

  /** @} */
/**
   * @name Presentation context
   * @{
   */
  /* readonly attribute nsPresContext presContext; */
  NS_IMETHOD GetPresContext(nsPresContext * *aPresContext) = 0;

  enum { UPDATEMASK_PRES_CONTEXT = 1U };

  /** @} */
/**
   * @name Canvas transform matrix
   * @{
   */
  /* readonly attribute nsIDOMSVGMatrix canvasTM; */
  NS_IMETHOD GetCanvasTM(nsIDOMSVGMatrix * *aCanvasTM) = 0;

  enum { UPDATEMASK_CANVAS_TM = 2U };

  /** @} */
/**
   * @name Stroke opacity
   * @{
   */
  /* readonly attribute float strokeOpacity; */
  NS_IMETHOD GetStrokeOpacity(float *aStrokeOpacity) = 0;

  enum { UPDATEMASK_STROKE_OPACITY = 4U };

  /** @} */
/**
   * @name Stroke width
   * @{
   */
  /* readonly attribute float strokeWidth; */
  NS_IMETHOD GetStrokeWidth(float *aStrokeWidth) = 0;

  enum { UPDATEMASK_STROKE_WIDTH = 8U };

  /** @} */
/**
   * @name Stroke dash-array
   * @{
   */
  /* void getStrokeDashArray ([array, size_is (count)] out float arr, out unsigned long count); */
  NS_IMETHOD GetStrokeDashArray(float **arr, PRUint32 *count) = 0;

  enum { UPDATEMASK_STROKE_DASH_ARRAY = 16U };

  /** @} */
/**
   * @name Stroke dash-offset
   * @{
   */
  /* readonly attribute float strokeDashoffset; */
  NS_IMETHOD GetStrokeDashoffset(float *aStrokeDashoffset) = 0;

  enum { UPDATEMASK_STROKE_DASHOFFSET = 32U };

  /** @} */
/**
   * @name Stroke line-cap
   * @{
   */
  enum { STROKE_LINECAP_BUTT = 0U };

  enum { STROKE_LINECAP_ROUND = 1U };

  enum { STROKE_LINECAP_SQUARE = 2U };

  /* readonly attribute unsigned short strokeLinecap; */
  NS_IMETHOD GetStrokeLinecap(PRUint16 *aStrokeLinecap) = 0;

  enum { UPDATEMASK_STROKE_LINECAP = 64U };

  /** @} */
/**
   * @name Stroke line-join
   * @{
   */
  enum { STROKE_LINEJOIN_MITER = 0U };

  enum { STROKE_LINEJOIN_ROUND = 1U };

  enum { STROKE_LINEJOIN_BEVEL = 2U };

  /* readonly attribute unsigned short strokeLinejoin; */
  NS_IMETHOD GetStrokeLinejoin(PRUint16 *aStrokeLinejoin) = 0;

  enum { UPDATEMASK_STROKE_LINEJOIN = 128U };

  /** @} */
/**
   * @name Miterlimit
   * @{
   */
  /* readonly attribute float strokeMiterlimit; */
  NS_IMETHOD GetStrokeMiterlimit(float *aStrokeMiterlimit) = 0;

  enum { UPDATEMASK_STROKE_MITERLIMIT = 256U };

  /** @} */
/**
   * @name Fill opacity
   * @{
   */
  /* readonly attribute float fillOpacity; */
  NS_IMETHOD GetFillOpacity(float *aFillOpacity) = 0;

  enum { UPDATEMASK_FILL_OPACITY = 512U };

  /** @} */
/**
   * @name Fill rule
   * @{
   */
  enum { FILL_RULE_NONZERO = 0U };

  enum { FILL_RULE_EVENODD = 1U };

  /* readonly attribute unsigned short fillRule; */
  NS_IMETHOD GetFillRule(PRUint16 *aFillRule) = 0;

  /* readonly attribute unsigned short clipRule; */
  NS_IMETHOD GetClipRule(PRUint16 *aClipRule) = 0;

  enum { UPDATEMASK_FILL_RULE = 1024U };

  /** @} */
/**
   * @name Paint type constants for stroke and fill paint
   * @{
   */
  enum { PAINT_TYPE_NONE = 0U };

  enum { PAINT_TYPE_SOLID_COLOR = 1U };

  enum { PAINT_TYPE_SERVER = 2U };

  enum { PAINT_TYPE_GRADIENT = 3U };

  enum { PAINT_TYPE_PATTERN = 4U };

  /** @} */
/**
   * @name Stroke paint
   * @{
   */
  /* readonly attribute unsigned short strokePaintType; */
  NS_IMETHOD GetStrokePaintType(PRUint16 *aStrokePaintType) = 0;

  enum { UPDATEMASK_STROKE_PAINT_TYPE = 2048U };

  /* readonly attribute unsigned short strokePaintServerType; */
  NS_IMETHOD GetStrokePaintServerType(PRUint16 *aStrokePaintServerType) = 0;

  /* readonly attribute nscolor strokePaint; */
  NS_IMETHOD GetStrokePaint(nscolor *aStrokePaint) = 0;

  /* void GetStrokeGradient (out nsISVGGradient aGrad); */
  NS_IMETHOD GetStrokeGradient(nsISVGGradient **aGrad) = 0;

  enum { UPDATEMASK_STROKE_PAINT = 4096U };

  /** @} */
/**
   * @name Fill paint
   * @{
   */
  /* readonly attribute unsigned short fillPaintType; */
  NS_IMETHOD GetFillPaintType(PRUint16 *aFillPaintType) = 0;

  enum { UPDATEMASK_FILL_PAINT_TYPE = 16384U };

  /* readonly attribute unsigned short fillPaintServerType; */
  NS_IMETHOD GetFillPaintServerType(PRUint16 *aFillPaintServerType) = 0;

  /* readonly attribute nscolor fillPaint; */
  NS_IMETHOD GetFillPaint(nscolor *aFillPaint) = 0;

  /* void GetFillGradient (out nsISVGGradient aGrad); */
  NS_IMETHOD GetFillGradient(nsISVGGradient **aGrad) = 0;

  enum { UPDATEMASK_FILL_PAINT = 32768U };

  /** @} */
  /* boolean IsClipChild (); */
  NS_IMETHOD IsClipChild(PRBool *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGGEOMETRYSOURCE \
  NS_IMETHOD GetPresContext(nsPresContext * *aPresContext); \
  NS_IMETHOD GetCanvasTM(nsIDOMSVGMatrix * *aCanvasTM); \
  NS_IMETHOD GetStrokeOpacity(float *aStrokeOpacity); \
  NS_IMETHOD GetStrokeWidth(float *aStrokeWidth); \
  NS_IMETHOD GetStrokeDashArray(float **arr, PRUint32 *count); \
  NS_IMETHOD GetStrokeDashoffset(float *aStrokeDashoffset); \
  NS_IMETHOD GetStrokeLinecap(PRUint16 *aStrokeLinecap); \
  NS_IMETHOD GetStrokeLinejoin(PRUint16 *aStrokeLinejoin); \
  NS_IMETHOD GetStrokeMiterlimit(float *aStrokeMiterlimit); \
  NS_IMETHOD GetFillOpacity(float *aFillOpacity); \
  NS_IMETHOD GetFillRule(PRUint16 *aFillRule); \
  NS_IMETHOD GetClipRule(PRUint16 *aClipRule); \
  NS_IMETHOD GetStrokePaintType(PRUint16 *aStrokePaintType); \
  NS_IMETHOD GetStrokePaintServerType(PRUint16 *aStrokePaintServerType); \
  NS_IMETHOD GetStrokePaint(nscolor *aStrokePaint); \
  NS_IMETHOD GetStrokeGradient(nsISVGGradient **aGrad); \
  NS_IMETHOD GetFillPaintType(PRUint16 *aFillPaintType); \
  NS_IMETHOD GetFillPaintServerType(PRUint16 *aFillPaintServerType); \
  NS_IMETHOD GetFillPaint(nscolor *aFillPaint); \
  NS_IMETHOD GetFillGradient(nsISVGGradient **aGrad); \
  NS_IMETHOD IsClipChild(PRBool *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGGEOMETRYSOURCE(_to) \
  NS_IMETHOD GetPresContext(nsPresContext * *aPresContext) { return _to GetPresContext(aPresContext); } \
  NS_IMETHOD GetCanvasTM(nsIDOMSVGMatrix * *aCanvasTM) { return _to GetCanvasTM(aCanvasTM); } \
  NS_IMETHOD GetStrokeOpacity(float *aStrokeOpacity) { return _to GetStrokeOpacity(aStrokeOpacity); } \
  NS_IMETHOD GetStrokeWidth(float *aStrokeWidth) { return _to GetStrokeWidth(aStrokeWidth); } \
  NS_IMETHOD GetStrokeDashArray(float **arr, PRUint32 *count) { return _to GetStrokeDashArray(arr, count); } \
  NS_IMETHOD GetStrokeDashoffset(float *aStrokeDashoffset) { return _to GetStrokeDashoffset(aStrokeDashoffset); } \
  NS_IMETHOD GetStrokeLinecap(PRUint16 *aStrokeLinecap) { return _to GetStrokeLinecap(aStrokeLinecap); } \
  NS_IMETHOD GetStrokeLinejoin(PRUint16 *aStrokeLinejoin) { return _to GetStrokeLinejoin(aStrokeLinejoin); } \
  NS_IMETHOD GetStrokeMiterlimit(float *aStrokeMiterlimit) { return _to GetStrokeMiterlimit(aStrokeMiterlimit); } \
  NS_IMETHOD GetFillOpacity(float *aFillOpacity) { return _to GetFillOpacity(aFillOpacity); } \
  NS_IMETHOD GetFillRule(PRUint16 *aFillRule) { return _to GetFillRule(aFillRule); } \
  NS_IMETHOD GetClipRule(PRUint16 *aClipRule) { return _to GetClipRule(aClipRule); } \
  NS_IMETHOD GetStrokePaintType(PRUint16 *aStrokePaintType) { return _to GetStrokePaintType(aStrokePaintType); } \
  NS_IMETHOD GetStrokePaintServerType(PRUint16 *aStrokePaintServerType) { return _to GetStrokePaintServerType(aStrokePaintServerType); } \
  NS_IMETHOD GetStrokePaint(nscolor *aStrokePaint) { return _to GetStrokePaint(aStrokePaint); } \
  NS_IMETHOD GetStrokeGradient(nsISVGGradient **aGrad) { return _to GetStrokeGradient(aGrad); } \
  NS_IMETHOD GetFillPaintType(PRUint16 *aFillPaintType) { return _to GetFillPaintType(aFillPaintType); } \
  NS_IMETHOD GetFillPaintServerType(PRUint16 *aFillPaintServerType) { return _to GetFillPaintServerType(aFillPaintServerType); } \
  NS_IMETHOD GetFillPaint(nscolor *aFillPaint) { return _to GetFillPaint(aFillPaint); } \
  NS_IMETHOD GetFillGradient(nsISVGGradient **aGrad) { return _to GetFillGradient(aGrad); } \
  NS_IMETHOD IsClipChild(PRBool *_retval) { return _to IsClipChild(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGGEOMETRYSOURCE(_to) \
  NS_IMETHOD GetPresContext(nsPresContext * *aPresContext) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPresContext(aPresContext); } \
  NS_IMETHOD GetCanvasTM(nsIDOMSVGMatrix * *aCanvasTM) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCanvasTM(aCanvasTM); } \
  NS_IMETHOD GetStrokeOpacity(float *aStrokeOpacity) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStrokeOpacity(aStrokeOpacity); } \
  NS_IMETHOD GetStrokeWidth(float *aStrokeWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStrokeWidth(aStrokeWidth); } \
  NS_IMETHOD GetStrokeDashArray(float **arr, PRUint32 *count) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStrokeDashArray(arr, count); } \
  NS_IMETHOD GetStrokeDashoffset(float *aStrokeDashoffset) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStrokeDashoffset(aStrokeDashoffset); } \
  NS_IMETHOD GetStrokeLinecap(PRUint16 *aStrokeLinecap) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStrokeLinecap(aStrokeLinecap); } \
  NS_IMETHOD GetStrokeLinejoin(PRUint16 *aStrokeLinejoin) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStrokeLinejoin(aStrokeLinejoin); } \
  NS_IMETHOD GetStrokeMiterlimit(float *aStrokeMiterlimit) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStrokeMiterlimit(aStrokeMiterlimit); } \
  NS_IMETHOD GetFillOpacity(float *aFillOpacity) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFillOpacity(aFillOpacity); } \
  NS_IMETHOD GetFillRule(PRUint16 *aFillRule) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFillRule(aFillRule); } \
  NS_IMETHOD GetClipRule(PRUint16 *aClipRule) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetClipRule(aClipRule); } \
  NS_IMETHOD GetStrokePaintType(PRUint16 *aStrokePaintType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStrokePaintType(aStrokePaintType); } \
  NS_IMETHOD GetStrokePaintServerType(PRUint16 *aStrokePaintServerType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStrokePaintServerType(aStrokePaintServerType); } \
  NS_IMETHOD GetStrokePaint(nscolor *aStrokePaint) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStrokePaint(aStrokePaint); } \
  NS_IMETHOD GetStrokeGradient(nsISVGGradient **aGrad) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStrokeGradient(aGrad); } \
  NS_IMETHOD GetFillPaintType(PRUint16 *aFillPaintType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFillPaintType(aFillPaintType); } \
  NS_IMETHOD GetFillPaintServerType(PRUint16 *aFillPaintServerType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFillPaintServerType(aFillPaintServerType); } \
  NS_IMETHOD GetFillPaint(nscolor *aFillPaint) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFillPaint(aFillPaint); } \
  NS_IMETHOD GetFillGradient(nsISVGGradient **aGrad) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFillGradient(aGrad); } \
  NS_IMETHOD IsClipChild(PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->IsClipChild(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGGeometrySource : public nsISVGGeometrySource
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGGEOMETRYSOURCE

  nsSVGGeometrySource();

private:
  ~nsSVGGeometrySource();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGGeometrySource, nsISVGGeometrySource)

nsSVGGeometrySource::nsSVGGeometrySource()
{
  /* member initializers and constructor code */
}

nsSVGGeometrySource::~nsSVGGeometrySource()
{
  /* destructor code */
}

/* readonly attribute nsPresContext presContext; */
NS_IMETHODIMP nsSVGGeometrySource::GetPresContext(nsPresContext * *aPresContext)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMSVGMatrix canvasTM; */
NS_IMETHODIMP nsSVGGeometrySource::GetCanvasTM(nsIDOMSVGMatrix * *aCanvasTM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float strokeOpacity; */
NS_IMETHODIMP nsSVGGeometrySource::GetStrokeOpacity(float *aStrokeOpacity)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float strokeWidth; */
NS_IMETHODIMP nsSVGGeometrySource::GetStrokeWidth(float *aStrokeWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getStrokeDashArray ([array, size_is (count)] out float arr, out unsigned long count); */
NS_IMETHODIMP nsSVGGeometrySource::GetStrokeDashArray(float **arr, PRUint32 *count)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float strokeDashoffset; */
NS_IMETHODIMP nsSVGGeometrySource::GetStrokeDashoffset(float *aStrokeDashoffset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned short strokeLinecap; */
NS_IMETHODIMP nsSVGGeometrySource::GetStrokeLinecap(PRUint16 *aStrokeLinecap)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned short strokeLinejoin; */
NS_IMETHODIMP nsSVGGeometrySource::GetStrokeLinejoin(PRUint16 *aStrokeLinejoin)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float strokeMiterlimit; */
NS_IMETHODIMP nsSVGGeometrySource::GetStrokeMiterlimit(float *aStrokeMiterlimit)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float fillOpacity; */
NS_IMETHODIMP nsSVGGeometrySource::GetFillOpacity(float *aFillOpacity)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned short fillRule; */
NS_IMETHODIMP nsSVGGeometrySource::GetFillRule(PRUint16 *aFillRule)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned short clipRule; */
NS_IMETHODIMP nsSVGGeometrySource::GetClipRule(PRUint16 *aClipRule)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned short strokePaintType; */
NS_IMETHODIMP nsSVGGeometrySource::GetStrokePaintType(PRUint16 *aStrokePaintType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned short strokePaintServerType; */
NS_IMETHODIMP nsSVGGeometrySource::GetStrokePaintServerType(PRUint16 *aStrokePaintServerType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nscolor strokePaint; */
NS_IMETHODIMP nsSVGGeometrySource::GetStrokePaint(nscolor *aStrokePaint)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void GetStrokeGradient (out nsISVGGradient aGrad); */
NS_IMETHODIMP nsSVGGeometrySource::GetStrokeGradient(nsISVGGradient **aGrad)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned short fillPaintType; */
NS_IMETHODIMP nsSVGGeometrySource::GetFillPaintType(PRUint16 *aFillPaintType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned short fillPaintServerType; */
NS_IMETHODIMP nsSVGGeometrySource::GetFillPaintServerType(PRUint16 *aFillPaintServerType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nscolor fillPaint; */
NS_IMETHODIMP nsSVGGeometrySource::GetFillPaint(nscolor *aFillPaint)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void GetFillGradient (out nsISVGGradient aGrad); */
NS_IMETHODIMP nsSVGGeometrySource::GetFillGradient(nsISVGGradient **aGrad)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean IsClipChild (); */
NS_IMETHODIMP nsSVGGeometrySource::IsClipChild(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISVGGeometrySource_h__ */
