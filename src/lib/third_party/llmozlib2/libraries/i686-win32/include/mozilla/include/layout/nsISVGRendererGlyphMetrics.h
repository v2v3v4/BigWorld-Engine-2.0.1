/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/svg/renderer/public/nsISVGRendererGlyphMetrics.idl
 */

#ifndef __gen_nsISVGRendererGlyphMetrics_h__
#define __gen_nsISVGRendererGlyphMetrics_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMSVGRect; /* forward declaration */


/* starting interface:    nsISVGRendererGlyphMetrics */
#define NS_ISVGRENDERERGLYPHMETRICS_IID_STR "2cdc98a4-594f-42a7-970c-e4dcb7a72aa0"

#define NS_ISVGRENDERERGLYPHMETRICS_IID \
  {0x2cdc98a4, 0x594f, 0x42a7, \
    { 0x97, 0x0c, 0xe4, 0xdc, 0xb7, 0xa7, 0x2a, 0xa0 }}

/**
 * \addtogroup renderer_interfaces Rendering Engine Interfaces
 * @{
 */
/**
 * One of a number of interfaces (all starting with nsISVGRenderer*)
 * to be implemented by an SVG rendering engine. See nsISVGRenderer
 * for more details.
 *
 * The SVG rendering backend uses this interface to communicate to the
 * rendering engine-native glyph metrics objects.
 *
 * A glyph metrics object is instantiated by the backend for a given
 * nsISVGGlyphMetricsSource object with a call to
 * nsISVGRenderer::createGlyphMetrics(). The glyph metrics object is
 * assumed to store a reference to its associated source object and
 * provide metrics information about the (composite) glyph described
 * by nsISVGGlyphMetricsSource::characterData,
 * nsISVGGlyphMetricsSource::font, nsISVGGlyphMetricsSource::canvasTM, and
 * other relevant style such as nsISVGGlyphMetricsSource::strokeWidth.
 */
class NS_NO_VTABLE nsISVGRendererGlyphMetrics : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGRENDERERGLYPHMETRICS_IID)

  /**
   * Untransformed width of the composite glyph in pixels.
   */
  /* readonly attribute float advance; */
  NS_IMETHOD GetAdvance(float *aAdvance) = 0;

  /**
   * Get the untransformed bounding box of an individual glyph.
   *
   * @param charnum The index of the character in
   * nsISVGGlyphMetricsSource::characterData whose glyph bounding box
   * is to be determined
   * @return The untransformed bounding box in pixel coordinates
   */
  /* nsIDOMSVGRect getExtentOfChar (in unsigned long charnum); */
  NS_IMETHOD GetExtentOfChar(PRUint32 charnum, nsIDOMSVGRect **_retval) = 0;

  /**
   * Get the advance of an individual glyph.
   */
  /* float getAdvanceOfChar (in unsigned long charnum); */
  NS_IMETHOD GetAdvanceOfChar(PRUint32 charnum, float *_retval) = 0;

  /**
   * @name Baseline offset constants for getBaselineOffset()
   * @{
   */
  enum { BASELINE_ALPHABETIC = 0U };

  enum { BASELINE_HANGING = 1U };

  enum { BASELINE_IDEOGRAPHC = 2U };

  enum { BASELINE_MATHEMATICAL = 3U };

  enum { BASELINE_CENTRAL = 4U };

  enum { BASELINE_MIDDLE = 5U };

  enum { BASELINE_TEXT_BEFORE_EDGE = 6U };

  enum { BASELINE_TEXT_AFTER_EDGE = 7U };

  /** @} */
/**
   * Retrieve the (y-axis) offset of the given baseline.
   * 
   * @param baselineIdentifier One of the BASELINE_* constants defined
   * in this interface.
   * @return Y-axis offset in pixels relative to top of bounding box.
   */
  /* float getBaselineOffset (in unsigned short baselineIdentifier); */
  NS_IMETHOD GetBaselineOffset(PRUint16 baselineIdentifier, float *_retval) = 0;

  /**
   * Called by this object's corresponding nsISVGGlyphMetricsSource as
   * a notification that some of the source's data (identified by
   * paramter 'updatemask') has changed.
   *
   * @param updatemask An OR-ed combination of the UPDATEMASK_*
   * constants defined in nsISVGGlyphMetricsSource.
   * @return PR_TRUE if the metrics have changed as a result of the
   * source update, PR_FALSE otherwise.
   */
  /* boolean update (in unsigned long updatemask); */
  NS_IMETHOD Update(PRUint32 updatemask, PRBool *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGRENDERERGLYPHMETRICS \
  NS_IMETHOD GetAdvance(float *aAdvance); \
  NS_IMETHOD GetExtentOfChar(PRUint32 charnum, nsIDOMSVGRect **_retval); \
  NS_IMETHOD GetAdvanceOfChar(PRUint32 charnum, float *_retval); \
  NS_IMETHOD GetBaselineOffset(PRUint16 baselineIdentifier, float *_retval); \
  NS_IMETHOD Update(PRUint32 updatemask, PRBool *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGRENDERERGLYPHMETRICS(_to) \
  NS_IMETHOD GetAdvance(float *aAdvance) { return _to GetAdvance(aAdvance); } \
  NS_IMETHOD GetExtentOfChar(PRUint32 charnum, nsIDOMSVGRect **_retval) { return _to GetExtentOfChar(charnum, _retval); } \
  NS_IMETHOD GetAdvanceOfChar(PRUint32 charnum, float *_retval) { return _to GetAdvanceOfChar(charnum, _retval); } \
  NS_IMETHOD GetBaselineOffset(PRUint16 baselineIdentifier, float *_retval) { return _to GetBaselineOffset(baselineIdentifier, _retval); } \
  NS_IMETHOD Update(PRUint32 updatemask, PRBool *_retval) { return _to Update(updatemask, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGRENDERERGLYPHMETRICS(_to) \
  NS_IMETHOD GetAdvance(float *aAdvance) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAdvance(aAdvance); } \
  NS_IMETHOD GetExtentOfChar(PRUint32 charnum, nsIDOMSVGRect **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetExtentOfChar(charnum, _retval); } \
  NS_IMETHOD GetAdvanceOfChar(PRUint32 charnum, float *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAdvanceOfChar(charnum, _retval); } \
  NS_IMETHOD GetBaselineOffset(PRUint16 baselineIdentifier, float *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBaselineOffset(baselineIdentifier, _retval); } \
  NS_IMETHOD Update(PRUint32 updatemask, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Update(updatemask, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGRendererGlyphMetrics : public nsISVGRendererGlyphMetrics
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGRENDERERGLYPHMETRICS

  nsSVGRendererGlyphMetrics();

private:
  ~nsSVGRendererGlyphMetrics();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGRendererGlyphMetrics, nsISVGRendererGlyphMetrics)

nsSVGRendererGlyphMetrics::nsSVGRendererGlyphMetrics()
{
  /* member initializers and constructor code */
}

nsSVGRendererGlyphMetrics::~nsSVGRendererGlyphMetrics()
{
  /* destructor code */
}

/* readonly attribute float advance; */
NS_IMETHODIMP nsSVGRendererGlyphMetrics::GetAdvance(float *aAdvance)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGRect getExtentOfChar (in unsigned long charnum); */
NS_IMETHODIMP nsSVGRendererGlyphMetrics::GetExtentOfChar(PRUint32 charnum, nsIDOMSVGRect **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* float getAdvanceOfChar (in unsigned long charnum); */
NS_IMETHODIMP nsSVGRendererGlyphMetrics::GetAdvanceOfChar(PRUint32 charnum, float *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* float getBaselineOffset (in unsigned short baselineIdentifier); */
NS_IMETHODIMP nsSVGRendererGlyphMetrics::GetBaselineOffset(PRUint16 baselineIdentifier, float *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean update (in unsigned long updatemask); */
NS_IMETHODIMP nsSVGRendererGlyphMetrics::Update(PRUint32 updatemask, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISVGRendererGlyphMetrics_h__ */
