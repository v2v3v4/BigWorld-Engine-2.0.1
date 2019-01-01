/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/svg/renderer/public/nsISVGRenderer.idl
 */

#ifndef __gen_nsISVGRenderer_h__
#define __gen_nsISVGRenderer_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
struct nsRect;
class nsISVGPathGeometrySource; /* forward declaration */

class nsISVGGlyphMetricsSource; /* forward declaration */

class nsISVGGlyphGeometrySource; /* forward declaration */

class nsISVGRendererPathGeometry; /* forward declaration */

class nsISVGRendererGlyphMetrics; /* forward declaration */

class nsISVGRendererGlyphGeometry; /* forward declaration */

class nsISVGRendererCanvas; /* forward declaration */

class nsISVGRendererSurface; /* forward declaration */

class nsIRenderingContext; /* forward declaration */

class nsISVGRendererRegion; /* forward declaration */

class nsPresContext; /* forward declaration */


/* starting interface:    nsISVGRenderer */
#define NS_ISVGRENDERER_IID_STR "14e914e0-f283-4fd0-9d71-d3e842927007"

#define NS_ISVGRENDERER_IID \
  {0x14e914e0, 0xf283, 0x4fd0, \
    { 0x9d, 0x71, 0xd3, 0xe8, 0x42, 0x92, 0x70, 0x07 }}

/**
 * \addtogroup renderer_interfaces Rendering Engine Interfaces
 * @{
 */
/**
 * One of a number of interfaces (all starting with nsISVGRenderer*)
 * to be implemented by an SVG rendering engine. 
 *
 * This interface serves as a factory for rendering engine-related
 * objects. Each rendering engine needs to make available an
 * nsIRenderer-object with a contract id of the form
 * "@mozilla.org/svg/renderer;1?tech=NAME".
 *
 * Engines implemented at the moment include an ms windows gdi+ engine
 * ("@mozilla.org/svg/renderer;1?tech=GDIPLUS") and a (somewhat)
 * cross-platform libart engine
 * ("@mozilla.org/svg/renderer;1?tech=LIBART").
 */
class NS_NO_VTABLE nsISVGRenderer : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGRENDERER_IID)

  /**
   * Create a rendering engine-native path geometry object for the
   * source object given by 'src'.
   *
   * @param src The source object describing the path for which
   * this object is being created.
   * @return A rendering engine-native path geometry object.
   */
  /* nsISVGRendererPathGeometry createPathGeometry (in nsISVGPathGeometrySource src); */
  NS_IMETHOD CreatePathGeometry(nsISVGPathGeometrySource *src, nsISVGRendererPathGeometry **_retval) = 0;

  /**
   * Create a rendering engine-native glyph metrics object for the
   * source object given by 'src'.
   *
   * @param src The source object describing the glyph for which
   * this metrics object is being created.
   * @return A rendering engine-native glyph metrics object.
   */
  /* nsISVGRendererGlyphMetrics createGlyphMetrics (in nsISVGGlyphMetricsSource src); */
  NS_IMETHOD CreateGlyphMetrics(nsISVGGlyphMetricsSource *src, nsISVGRendererGlyphMetrics **_retval) = 0;

  /**
   * Create a rendering engine-native glyph geometry object for the
   * source object given by 'src'.
   *
   * @param src The source object describing the glyph for which
   * this object is being created.
   * @return A rendering engine-native glyph geometry object.
   */
  /* nsISVGRendererGlyphGeometry createGlyphGeometry (in nsISVGGlyphGeometrySource src); */
  NS_IMETHOD CreateGlyphGeometry(nsISVGGlyphGeometrySource *src, nsISVGRendererGlyphGeometry **_retval) = 0;

  /**
   * Create a rendering engine-native canvas object for the
   * Mozilla-native rendering context 'ctx' and presentation context
   * 'presContext'.
   *
   * @param ctx Mozilla-native rendering context.
   * @param presContext Presentation context.
   * @param dirtyRect Area that the canvas should cover.
   * @return A rendering engine-native canvas object.
   */
  /* [noscript] nsISVGRendererCanvas createCanvas (in nsIRenderingContext ctx, in nsPresContext presContext, [const] in nsRectRef dirtyRect); */
  NS_IMETHOD CreateCanvas(nsIRenderingContext *ctx, nsPresContext *presContext, const nsRect & dirtyRect, nsISVGRendererCanvas **_retval) = 0;

  /**
   * Create a rendering engine-native region object for the
   * given axis-aligned rectangle.
   *
   * @param x X-coordinate of rectangle (pixels).
   * @param y Y-coordinate of rectangle (pixels).
   * @param width Width of rectangle (pixels).
   * @param height Height of rectangle (pixels).
   *
   * @return A rendering engine-native region object.
   */
  /* nsISVGRendererRegion createRectRegion (in float x, in float y, in float width, in float height); */
  NS_IMETHOD CreateRectRegion(float x, float y, float width, float height, nsISVGRendererRegion **_retval) = 0;

  /**
   * Create a rendering engine-native surface object.
   *
   * @param width Width of rectangle (pixels).
   * @param height Height of rectangle (pixels).
   *
   * @return A rendering engine-native surface object.
   */
  /* [noscript] nsISVGRendererSurface createSurface (in unsigned long width, in unsigned long height); */
  NS_IMETHOD CreateSurface(PRUint32 width, PRUint32 height, nsISVGRendererSurface **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGRENDERER \
  NS_IMETHOD CreatePathGeometry(nsISVGPathGeometrySource *src, nsISVGRendererPathGeometry **_retval); \
  NS_IMETHOD CreateGlyphMetrics(nsISVGGlyphMetricsSource *src, nsISVGRendererGlyphMetrics **_retval); \
  NS_IMETHOD CreateGlyphGeometry(nsISVGGlyphGeometrySource *src, nsISVGRendererGlyphGeometry **_retval); \
  NS_IMETHOD CreateCanvas(nsIRenderingContext *ctx, nsPresContext *presContext, const nsRect & dirtyRect, nsISVGRendererCanvas **_retval); \
  NS_IMETHOD CreateRectRegion(float x, float y, float width, float height, nsISVGRendererRegion **_retval); \
  NS_IMETHOD CreateSurface(PRUint32 width, PRUint32 height, nsISVGRendererSurface **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGRENDERER(_to) \
  NS_IMETHOD CreatePathGeometry(nsISVGPathGeometrySource *src, nsISVGRendererPathGeometry **_retval) { return _to CreatePathGeometry(src, _retval); } \
  NS_IMETHOD CreateGlyphMetrics(nsISVGGlyphMetricsSource *src, nsISVGRendererGlyphMetrics **_retval) { return _to CreateGlyphMetrics(src, _retval); } \
  NS_IMETHOD CreateGlyphGeometry(nsISVGGlyphGeometrySource *src, nsISVGRendererGlyphGeometry **_retval) { return _to CreateGlyphGeometry(src, _retval); } \
  NS_IMETHOD CreateCanvas(nsIRenderingContext *ctx, nsPresContext *presContext, const nsRect & dirtyRect, nsISVGRendererCanvas **_retval) { return _to CreateCanvas(ctx, presContext, dirtyRect, _retval); } \
  NS_IMETHOD CreateRectRegion(float x, float y, float width, float height, nsISVGRendererRegion **_retval) { return _to CreateRectRegion(x, y, width, height, _retval); } \
  NS_IMETHOD CreateSurface(PRUint32 width, PRUint32 height, nsISVGRendererSurface **_retval) { return _to CreateSurface(width, height, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGRENDERER(_to) \
  NS_IMETHOD CreatePathGeometry(nsISVGPathGeometrySource *src, nsISVGRendererPathGeometry **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreatePathGeometry(src, _retval); } \
  NS_IMETHOD CreateGlyphMetrics(nsISVGGlyphMetricsSource *src, nsISVGRendererGlyphMetrics **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateGlyphMetrics(src, _retval); } \
  NS_IMETHOD CreateGlyphGeometry(nsISVGGlyphGeometrySource *src, nsISVGRendererGlyphGeometry **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateGlyphGeometry(src, _retval); } \
  NS_IMETHOD CreateCanvas(nsIRenderingContext *ctx, nsPresContext *presContext, const nsRect & dirtyRect, nsISVGRendererCanvas **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateCanvas(ctx, presContext, dirtyRect, _retval); } \
  NS_IMETHOD CreateRectRegion(float x, float y, float width, float height, nsISVGRendererRegion **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateRectRegion(x, y, width, height, _retval); } \
  NS_IMETHOD CreateSurface(PRUint32 width, PRUint32 height, nsISVGRendererSurface **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSurface(width, height, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGRenderer : public nsISVGRenderer
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGRENDERER

  nsSVGRenderer();

private:
  ~nsSVGRenderer();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGRenderer, nsISVGRenderer)

nsSVGRenderer::nsSVGRenderer()
{
  /* member initializers and constructor code */
}

nsSVGRenderer::~nsSVGRenderer()
{
  /* destructor code */
}

/* nsISVGRendererPathGeometry createPathGeometry (in nsISVGPathGeometrySource src); */
NS_IMETHODIMP nsSVGRenderer::CreatePathGeometry(nsISVGPathGeometrySource *src, nsISVGRendererPathGeometry **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsISVGRendererGlyphMetrics createGlyphMetrics (in nsISVGGlyphMetricsSource src); */
NS_IMETHODIMP nsSVGRenderer::CreateGlyphMetrics(nsISVGGlyphMetricsSource *src, nsISVGRendererGlyphMetrics **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsISVGRendererGlyphGeometry createGlyphGeometry (in nsISVGGlyphGeometrySource src); */
NS_IMETHODIMP nsSVGRenderer::CreateGlyphGeometry(nsISVGGlyphGeometrySource *src, nsISVGRendererGlyphGeometry **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] nsISVGRendererCanvas createCanvas (in nsIRenderingContext ctx, in nsPresContext presContext, [const] in nsRectRef dirtyRect); */
NS_IMETHODIMP nsSVGRenderer::CreateCanvas(nsIRenderingContext *ctx, nsPresContext *presContext, const nsRect & dirtyRect, nsISVGRendererCanvas **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsISVGRendererRegion createRectRegion (in float x, in float y, in float width, in float height); */
NS_IMETHODIMP nsSVGRenderer::CreateRectRegion(float x, float y, float width, float height, nsISVGRendererRegion **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] nsISVGRendererSurface createSurface (in unsigned long width, in unsigned long height); */
NS_IMETHODIMP nsSVGRenderer::CreateSurface(PRUint32 width, PRUint32 height, nsISVGRendererSurface **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

    
#define NS_SVG_RENDERER_CONTRACTID         "@mozilla.org/svg/renderer;1"
#define NS_SVG_RENDERER_CONTRACTID_PREFIX  NS_SVG_RENDERER_CONTRACTID "?tech="
#define NS_SVG_RENDERER_LIBART_CONTRACTID NS_SVG_RENDERER_CONTRACTID_PREFIX "LIBART"
// {A88E949D-AA36-4734-9C6E-F0FBCEF4FC47}
#define NS_SVG_RENDERER_LIBART_CID \
{ 0xa88e949d, 0xaa36, 0x4734, { 0x9c, 0x6e, 0xf0, 0xfb, 0xce, 0xf4, 0xfc, 0x47 } }
    
#define NS_SVG_RENDERER_GDIPLUS_CONTRACTID NS_SVG_RENDERER_CONTRACTID_PREFIX "GDIPLUS"
// {D260F971-DB9D-425B-8C9B-4EB9605CE35D}
#define NS_SVG_RENDERER_GDIPLUS_CID \
{ 0xd260f971, 0xdb9d, 0x425b, { 0x8c, 0x9b, 0x4e, 0xb9, 0x60, 0x5c, 0xe3, 0x5d } }
#define NS_SVG_RENDERER_CAIRO_CONTRACTID NS_SVG_RENDERER_CONTRACTID_PREFIX "CAIRO"
// {9f0fa438-1b1a-4a1b-a28d-91460542276e}
#define NS_SVG_RENDERER_CAIRO_CID \
{ 0x9f0fa438, 0x1b1a, 0x4a1b, { 0xa2, 0x8d, 0x91, 0x46, 0x05, 0x42, 0x27, 0x6e } }

#endif /* __gen_nsISVGRenderer_h__ */
