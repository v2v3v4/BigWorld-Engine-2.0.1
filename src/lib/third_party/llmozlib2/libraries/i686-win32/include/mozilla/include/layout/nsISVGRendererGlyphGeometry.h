/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/svg/renderer/public/nsISVGRendererGlyphGeometry.idl
 */

#ifndef __gen_nsISVGRendererGlyphGeometry_h__
#define __gen_nsISVGRendererGlyphGeometry_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsISVGRendererRegion; /* forward declaration */

class nsISVGRendererGlyphMetrics; /* forward declaration */

class nsISVGRendererCanvas; /* forward declaration */

class nsIDOMSVGRect; /* forward declaration */


/* starting interface:    nsISVGRendererGlyphGeometry */
#define NS_ISVGRENDERERGLYPHGEOMETRY_IID_STR "e2b739d0-8415-4f36-982c-3579876e1435"

#define NS_ISVGRENDERERGLYPHGEOMETRY_IID \
  {0xe2b739d0, 0x8415, 0x4f36, \
    { 0x98, 0x2c, 0x35, 0x79, 0x87, 0x6e, 0x14, 0x35 }}

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
 * rendering engine-native glyph objects.
 *
 * A glyph object is instantiated by the rendering backend for a given
 * nsISVGGlyphGeometrySource object with a call to
 * nsISVGRenderer::createGlyphGeometry(). The glyph object is assumed
 * to store a reference to its associated source object and provide
 * glyph rendering and hit-testing for the (composite) glyph described
 * by the nsISVGGlyphGeometrySource members.
 */
class NS_NO_VTABLE nsISVGRendererGlyphGeometry : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGRENDERERGLYPHGEOMETRY_IID)

  /**
   * Paint this object.
   *
   * @param canvas The canvas to render to.
   */
  /* void render (in nsISVGRendererCanvas canvas); */
  NS_IMETHOD Render(nsISVGRendererCanvas *canvas) = 0;

  /**
   * Called by this object's corresponding nsISVGGlyphGeometrySource
   * as a notification that some of the source's data (identified by
   * paramter 'updatemask') has changed.
   *
   * @param updatemask An OR-ed combination of the UPDATEMASK_*
   * constants defined in nsISVGGlyphGeometrySource.
   * @return Region that needs to be redrawn.
   */
  /* nsISVGRendererRegion update (in unsigned long updatemask); */
  NS_IMETHOD Update(PRUint32 updatemask, nsISVGRendererRegion **_retval) = 0;

  /**
   * Get a region object describing the area covered with paint by
   * this glyph geometry.
   *
   * @return Covered region.
   */
  /* nsISVGRendererRegion getCoveredRegion (); */
  NS_IMETHOD GetCoveredRegion(nsISVGRendererRegion **_retval) = 0;

  /**
   * Hit-testing method. Does this glyph geometry (with all relevant
   * transformations applied) contain the point x,y?  Should work on a
   * character-cell basis, rather than the actual character outline.
   *
   * @param x X-coordinate of test point.  @param y Y-coordinate of
   * test point.
   * @return PR_TRUE if the glyph geometry contains the point,
   * PR_FALSE otherwise.
   */
  /* boolean containsPoint (in float x, in float y); */
  NS_IMETHOD ContainsPoint(float x, float y, PRBool *_retval) = 0;

  /**
   * Transformed bounding box (does not include stroke width)
   */
  /* readonly attribute nsIDOMSVGRect boundingBox; */
  NS_IMETHOD GetBoundingBox(nsIDOMSVGRect * *aBoundingBox) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGRENDERERGLYPHGEOMETRY \
  NS_IMETHOD Render(nsISVGRendererCanvas *canvas); \
  NS_IMETHOD Update(PRUint32 updatemask, nsISVGRendererRegion **_retval); \
  NS_IMETHOD GetCoveredRegion(nsISVGRendererRegion **_retval); \
  NS_IMETHOD ContainsPoint(float x, float y, PRBool *_retval); \
  NS_IMETHOD GetBoundingBox(nsIDOMSVGRect * *aBoundingBox); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGRENDERERGLYPHGEOMETRY(_to) \
  NS_IMETHOD Render(nsISVGRendererCanvas *canvas) { return _to Render(canvas); } \
  NS_IMETHOD Update(PRUint32 updatemask, nsISVGRendererRegion **_retval) { return _to Update(updatemask, _retval); } \
  NS_IMETHOD GetCoveredRegion(nsISVGRendererRegion **_retval) { return _to GetCoveredRegion(_retval); } \
  NS_IMETHOD ContainsPoint(float x, float y, PRBool *_retval) { return _to ContainsPoint(x, y, _retval); } \
  NS_IMETHOD GetBoundingBox(nsIDOMSVGRect * *aBoundingBox) { return _to GetBoundingBox(aBoundingBox); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGRENDERERGLYPHGEOMETRY(_to) \
  NS_IMETHOD Render(nsISVGRendererCanvas *canvas) { return !_to ? NS_ERROR_NULL_POINTER : _to->Render(canvas); } \
  NS_IMETHOD Update(PRUint32 updatemask, nsISVGRendererRegion **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Update(updatemask, _retval); } \
  NS_IMETHOD GetCoveredRegion(nsISVGRendererRegion **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCoveredRegion(_retval); } \
  NS_IMETHOD ContainsPoint(float x, float y, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ContainsPoint(x, y, _retval); } \
  NS_IMETHOD GetBoundingBox(nsIDOMSVGRect * *aBoundingBox) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBoundingBox(aBoundingBox); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGRendererGlyphGeometry : public nsISVGRendererGlyphGeometry
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGRENDERERGLYPHGEOMETRY

  nsSVGRendererGlyphGeometry();

private:
  ~nsSVGRendererGlyphGeometry();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGRendererGlyphGeometry, nsISVGRendererGlyphGeometry)

nsSVGRendererGlyphGeometry::nsSVGRendererGlyphGeometry()
{
  /* member initializers and constructor code */
}

nsSVGRendererGlyphGeometry::~nsSVGRendererGlyphGeometry()
{
  /* destructor code */
}

/* void render (in nsISVGRendererCanvas canvas); */
NS_IMETHODIMP nsSVGRendererGlyphGeometry::Render(nsISVGRendererCanvas *canvas)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsISVGRendererRegion update (in unsigned long updatemask); */
NS_IMETHODIMP nsSVGRendererGlyphGeometry::Update(PRUint32 updatemask, nsISVGRendererRegion **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsISVGRendererRegion getCoveredRegion (); */
NS_IMETHODIMP nsSVGRendererGlyphGeometry::GetCoveredRegion(nsISVGRendererRegion **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean containsPoint (in float x, in float y); */
NS_IMETHODIMP nsSVGRendererGlyphGeometry::ContainsPoint(float x, float y, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMSVGRect boundingBox; */
NS_IMETHODIMP nsSVGRendererGlyphGeometry::GetBoundingBox(nsIDOMSVGRect * *aBoundingBox)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISVGRendererGlyphGeometry_h__ */
