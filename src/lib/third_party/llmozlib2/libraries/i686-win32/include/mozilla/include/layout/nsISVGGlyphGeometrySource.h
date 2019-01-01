/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/svg/renderer/public/nsISVGGlyphGeometrySource.idl
 */

#ifndef __gen_nsISVGGlyphGeometrySource_h__
#define __gen_nsISVGGlyphGeometrySource_h__


#ifndef __gen_nsISVGGlyphMetricsSource_h__
#include "nsISVGGlyphMetricsSource.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsISVGRendererGlyphMetrics; /* forward declaration */


/* starting interface:    nsISVGGlyphGeometrySource */
#define NS_ISVGGLYPHGEOMETRYSOURCE_IID_STR "b36065f1-c52b-4eda-b9ad-e483cf1a63bf"

#define NS_ISVGGLYPHGEOMETRYSOURCE_IID \
  {0xb36065f1, 0xc52b, 0x4eda, \
    { 0xb9, 0xad, 0xe4, 0x83, 0xcf, 0x1a, 0x63, 0xbf }}

/**
 * \addtogroup rendering_backend_interfaces Rendering Backend Interfaces
 * @{
 */
/**
 * Abstracts a description of a 'composite glyph' (character string
 * with associated font and styling information) in the SVG rendering
 * backend for use by a rendering engine's nsISVGRendererGlyphGeometry
 * objects. In addition to the attributes of the
 * nsISVGGlyphMetricsSource interface, this interface contains
 * absolute positioning and other information, such as e.g. individual
 * character highlighting, which doesn't affect the actual metrics of
 * the glyph. The metrics of the glyph, as given by the rendering
 * engine-native object implementing nsISVGRendererGlyphMetrics are
 * also provided as an attribute.
 *
 * An engine-native glyph geometry object will be informed of changes
 * in its associated composite glyph with a call to its
 * nsISVGRendererGlyphMetrics::update() method with an OR-ed
 * combination of the UPDATEMASK_* constants defined in this interface
 * (and its base-interface).
 *
 * @nosubgrouping
 */
class NS_NO_VTABLE nsISVGGlyphGeometrySource : public nsISVGGlyphMetricsSource {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGGLYPHGEOMETRYSOURCE_IID)

  /**
   * @name Glyph metrics
   * @{
   */
  /* readonly attribute nsISVGRendererGlyphMetrics metrics; */
  NS_IMETHOD GetMetrics(nsISVGRendererGlyphMetrics * *aMetrics) = 0;

  enum { UPDATEMASK_METRICS = 262144U };

  /** @} */
/**
   * @name Glyph start position
   * @{
   */
  /* readonly attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;

  enum { UPDATEMASK_X = 524288U };

  /* readonly attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;

  enum { UPDATEMASK_Y = 1048576U };

  /** @} */
/**
   * @name Partial highlighting for selection feedback
   * @{
   */
  /* readonly attribute boolean hasHighlight; */
  NS_IMETHOD GetHasHighlight(PRBool *aHasHighlight) = 0;

  enum { UPDATEMASK_HAS_HIGHLIGHT = 2097152U };

  /* [noscript] void getHighlight (out unsigned long charnum, out unsigned long nchars, out nscolor foreground, out nscolor background); */
  NS_IMETHOD GetHighlight(PRUint32 *charnum, PRUint32 *nchars, nscolor *foreground, nscolor *background) = 0;

  enum { UPDATEMASK_HIGHLIGHT = 4194304U };

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGGLYPHGEOMETRYSOURCE \
  NS_IMETHOD GetMetrics(nsISVGRendererGlyphMetrics * *aMetrics); \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD GetHasHighlight(PRBool *aHasHighlight); \
  NS_IMETHOD GetHighlight(PRUint32 *charnum, PRUint32 *nchars, nscolor *foreground, nscolor *background); \

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGGLYPHGEOMETRYSOURCE(_to) \
  NS_IMETHOD GetMetrics(nsISVGRendererGlyphMetrics * *aMetrics) { return _to GetMetrics(aMetrics); } \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD GetHasHighlight(PRBool *aHasHighlight) { return _to GetHasHighlight(aHasHighlight); } \
  NS_IMETHOD GetHighlight(PRUint32 *charnum, PRUint32 *nchars, nscolor *foreground, nscolor *background) { return _to GetHighlight(charnum, nchars, foreground, background); } \

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGGLYPHGEOMETRYSOURCE(_to) \
  NS_IMETHOD GetMetrics(nsISVGRendererGlyphMetrics * *aMetrics) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMetrics(aMetrics); } \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD GetHasHighlight(PRBool *aHasHighlight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHasHighlight(aHasHighlight); } \
  NS_IMETHOD GetHighlight(PRUint32 *charnum, PRUint32 *nchars, nscolor *foreground, nscolor *background) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHighlight(charnum, nchars, foreground, background); } \

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGGlyphGeometrySource : public nsISVGGlyphGeometrySource
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGGLYPHGEOMETRYSOURCE

  nsSVGGlyphGeometrySource();

private:
  ~nsSVGGlyphGeometrySource();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGGlyphGeometrySource, nsISVGGlyphGeometrySource)

nsSVGGlyphGeometrySource::nsSVGGlyphGeometrySource()
{
  /* member initializers and constructor code */
}

nsSVGGlyphGeometrySource::~nsSVGGlyphGeometrySource()
{
  /* destructor code */
}

/* readonly attribute nsISVGRendererGlyphMetrics metrics; */
NS_IMETHODIMP nsSVGGlyphGeometrySource::GetMetrics(nsISVGRendererGlyphMetrics * *aMetrics)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float x; */
NS_IMETHODIMP nsSVGGlyphGeometrySource::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float y; */
NS_IMETHODIMP nsSVGGlyphGeometrySource::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean hasHighlight; */
NS_IMETHODIMP nsSVGGlyphGeometrySource::GetHasHighlight(PRBool *aHasHighlight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void getHighlight (out unsigned long charnum, out unsigned long nchars, out nscolor foreground, out nscolor background); */
NS_IMETHODIMP nsSVGGlyphGeometrySource::GetHighlight(PRUint32 *charnum, PRUint32 *nchars, nscolor *foreground, nscolor *background)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISVGGlyphGeometrySource_h__ */
