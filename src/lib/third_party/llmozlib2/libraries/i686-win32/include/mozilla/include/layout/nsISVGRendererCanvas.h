/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/svg/renderer/public/nsISVGRendererCanvas.idl
 */

#ifndef __gen_nsISVGRendererCanvas_h__
#define __gen_nsISVGRendererCanvas_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
struct nsRect;
typedef PRUint32 nscolor;

class nsIRenderingContext; /* forward declaration */

class nsPresContext; /* forward declaration */

class nsIDOMSVGMatrix; /* forward declaration */

class nsISVGRendererSurface; /* forward declaration */


/* starting interface:    nsISVGRendererCanvas */
#define NS_ISVGRENDERERCANVAS_IID_STR "2e64a227-de4b-4a69-ab82-5dda1579e90f"

#define NS_ISVGRENDERERCANVAS_IID \
  {0x2e64a227, 0xde4b, 0x4a69, \
    { 0xab, 0x82, 0x5d, 0xda, 0x15, 0x79, 0xe9, 0x0f }}

/**
 * \addtogroup renderer_interfaces Rendering Engine Interfaces
 * @{
 */
/**
 * One of a number of interfaces (all starting with nsISVGRenderer*)
 * to be implemented by an SVG rendering engine. See nsISVGRenderer
 * for more details.
 *
 * This interface abstracts a rendering engine-native canvas object
 * onto which path and glyph geometries can be painted.
 *
 * A canvas object is instantiated by the backend for a given
 * Mozilla-native rendering object with a call to
 * nsISVGRenderer::createCanvas().
 */
class NS_NO_VTABLE nsISVGRendererCanvas : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGRENDERERCANVAS_IID)

  /**
   * Lock a portion of the canvas and obtain a rendering context onto
   * which Mozilla can paint non-svg content. This is needed for
   * (partial) foreignObject support.
   *
   * lockRenderingContext() calls are paired with
   * unlockRenderingContext() calls.
   *
   * @param rect Area to be locked.
   * @return Mozilla-native rendering context for the locked area.
   */
  /* [noscript] nsIRenderingContext lockRenderingContext ([const] in nsRectRef rect); */
  NS_IMETHOD LockRenderingContext(const nsRect & rect, nsIRenderingContext **_retval) = 0;

  /**
   * Unlock the canvas portion locked with a previous call to
   * lockRenderingContext(). 
   */
  /* void unlockRenderingContext (); */
  NS_IMETHOD UnlockRenderingContext(void) = 0;

  /* nsPresContext getPresContext (); */
  NS_IMETHOD GetPresContext(nsPresContext **_retval) = 0;

  /**
   * Fill the canvas with the given color.
   *
   * @param color Fill color.
   */
  /* void clear (in nscolor color); */
  NS_IMETHOD Clear(nscolor color) = 0;

  /**
   * Ensure that all renderings on the canvas have been flushed to the
   * Mozilla-native rendering context.
   */
  /* void flush (); */
  NS_IMETHOD Flush(void) = 0;

  /**
   * Set render mode (clipping or normal draw)
   */
  enum { SVG_RENDER_MODE_NORMAL = 0U };

  enum { SVG_RENDER_MODE_CLIP = 1U };

  /* attribute unsigned short renderMode; */
  NS_IMETHOD GetRenderMode(PRUint16 *aRenderMode) = 0;
  NS_IMETHOD SetRenderMode(PRUint16 aRenderMode) = 0;

  /**
   * Push/Pop clip path.
   */
  /* void pushClip (); */
  NS_IMETHOD PushClip(void) = 0;

  /* void popClip (); */
  NS_IMETHOD PopClip(void) = 0;

  /**
   * Set rectangular clip region.
   */
  /* void setClipRect (in nsIDOMSVGMatrix canvasTM, in float x, in float y, in float width, in float height); */
  NS_IMETHOD SetClipRect(nsIDOMSVGMatrix *canvasTM, float x, float y, float width, float height) = 0;

  /**
   * Push/Pop surface as primary drawing surface.
   */
  /* void pushSurface (in nsISVGRendererSurface surface); */
  NS_IMETHOD PushSurface(nsISVGRendererSurface *surface) = 0;

  /* void popSurface (); */
  NS_IMETHOD PopSurface(void) = 0;

  /**
   * Surface composition.
   */
  /* void compositeSurface (in nsISVGRendererSurface surface, in unsigned long x, in unsigned long y, in float opacity); */
  NS_IMETHOD CompositeSurface(nsISVGRendererSurface *surface, PRUint32 x, PRUint32 y, float opacity) = 0;

  /* void compositeSurfaceMatrix (in nsISVGRendererSurface surface, in nsIDOMSVGMatrix canvasTM, in float opacity); */
  NS_IMETHOD CompositeSurfaceMatrix(nsISVGRendererSurface *surface, nsIDOMSVGMatrix *canvasTM, float opacity) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGRENDERERCANVAS \
  NS_IMETHOD LockRenderingContext(const nsRect & rect, nsIRenderingContext **_retval); \
  NS_IMETHOD UnlockRenderingContext(void); \
  NS_IMETHOD GetPresContext(nsPresContext **_retval); \
  NS_IMETHOD Clear(nscolor color); \
  NS_IMETHOD Flush(void); \
  NS_IMETHOD GetRenderMode(PRUint16 *aRenderMode); \
  NS_IMETHOD SetRenderMode(PRUint16 aRenderMode); \
  NS_IMETHOD PushClip(void); \
  NS_IMETHOD PopClip(void); \
  NS_IMETHOD SetClipRect(nsIDOMSVGMatrix *canvasTM, float x, float y, float width, float height); \
  NS_IMETHOD PushSurface(nsISVGRendererSurface *surface); \
  NS_IMETHOD PopSurface(void); \
  NS_IMETHOD CompositeSurface(nsISVGRendererSurface *surface, PRUint32 x, PRUint32 y, float opacity); \
  NS_IMETHOD CompositeSurfaceMatrix(nsISVGRendererSurface *surface, nsIDOMSVGMatrix *canvasTM, float opacity); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGRENDERERCANVAS(_to) \
  NS_IMETHOD LockRenderingContext(const nsRect & rect, nsIRenderingContext **_retval) { return _to LockRenderingContext(rect, _retval); } \
  NS_IMETHOD UnlockRenderingContext(void) { return _to UnlockRenderingContext(); } \
  NS_IMETHOD GetPresContext(nsPresContext **_retval) { return _to GetPresContext(_retval); } \
  NS_IMETHOD Clear(nscolor color) { return _to Clear(color); } \
  NS_IMETHOD Flush(void) { return _to Flush(); } \
  NS_IMETHOD GetRenderMode(PRUint16 *aRenderMode) { return _to GetRenderMode(aRenderMode); } \
  NS_IMETHOD SetRenderMode(PRUint16 aRenderMode) { return _to SetRenderMode(aRenderMode); } \
  NS_IMETHOD PushClip(void) { return _to PushClip(); } \
  NS_IMETHOD PopClip(void) { return _to PopClip(); } \
  NS_IMETHOD SetClipRect(nsIDOMSVGMatrix *canvasTM, float x, float y, float width, float height) { return _to SetClipRect(canvasTM, x, y, width, height); } \
  NS_IMETHOD PushSurface(nsISVGRendererSurface *surface) { return _to PushSurface(surface); } \
  NS_IMETHOD PopSurface(void) { return _to PopSurface(); } \
  NS_IMETHOD CompositeSurface(nsISVGRendererSurface *surface, PRUint32 x, PRUint32 y, float opacity) { return _to CompositeSurface(surface, x, y, opacity); } \
  NS_IMETHOD CompositeSurfaceMatrix(nsISVGRendererSurface *surface, nsIDOMSVGMatrix *canvasTM, float opacity) { return _to CompositeSurfaceMatrix(surface, canvasTM, opacity); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGRENDERERCANVAS(_to) \
  NS_IMETHOD LockRenderingContext(const nsRect & rect, nsIRenderingContext **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->LockRenderingContext(rect, _retval); } \
  NS_IMETHOD UnlockRenderingContext(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnlockRenderingContext(); } \
  NS_IMETHOD GetPresContext(nsPresContext **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPresContext(_retval); } \
  NS_IMETHOD Clear(nscolor color) { return !_to ? NS_ERROR_NULL_POINTER : _to->Clear(color); } \
  NS_IMETHOD Flush(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Flush(); } \
  NS_IMETHOD GetRenderMode(PRUint16 *aRenderMode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRenderMode(aRenderMode); } \
  NS_IMETHOD SetRenderMode(PRUint16 aRenderMode) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetRenderMode(aRenderMode); } \
  NS_IMETHOD PushClip(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->PushClip(); } \
  NS_IMETHOD PopClip(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->PopClip(); } \
  NS_IMETHOD SetClipRect(nsIDOMSVGMatrix *canvasTM, float x, float y, float width, float height) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetClipRect(canvasTM, x, y, width, height); } \
  NS_IMETHOD PushSurface(nsISVGRendererSurface *surface) { return !_to ? NS_ERROR_NULL_POINTER : _to->PushSurface(surface); } \
  NS_IMETHOD PopSurface(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->PopSurface(); } \
  NS_IMETHOD CompositeSurface(nsISVGRendererSurface *surface, PRUint32 x, PRUint32 y, float opacity) { return !_to ? NS_ERROR_NULL_POINTER : _to->CompositeSurface(surface, x, y, opacity); } \
  NS_IMETHOD CompositeSurfaceMatrix(nsISVGRendererSurface *surface, nsIDOMSVGMatrix *canvasTM, float opacity) { return !_to ? NS_ERROR_NULL_POINTER : _to->CompositeSurfaceMatrix(surface, canvasTM, opacity); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGRendererCanvas : public nsISVGRendererCanvas
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGRENDERERCANVAS

  nsSVGRendererCanvas();

private:
  ~nsSVGRendererCanvas();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGRendererCanvas, nsISVGRendererCanvas)

nsSVGRendererCanvas::nsSVGRendererCanvas()
{
  /* member initializers and constructor code */
}

nsSVGRendererCanvas::~nsSVGRendererCanvas()
{
  /* destructor code */
}

/* [noscript] nsIRenderingContext lockRenderingContext ([const] in nsRectRef rect); */
NS_IMETHODIMP nsSVGRendererCanvas::LockRenderingContext(const nsRect & rect, nsIRenderingContext **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unlockRenderingContext (); */
NS_IMETHODIMP nsSVGRendererCanvas::UnlockRenderingContext()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsPresContext getPresContext (); */
NS_IMETHODIMP nsSVGRendererCanvas::GetPresContext(nsPresContext **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void clear (in nscolor color); */
NS_IMETHODIMP nsSVGRendererCanvas::Clear(nscolor color)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void flush (); */
NS_IMETHODIMP nsSVGRendererCanvas::Flush()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute unsigned short renderMode; */
NS_IMETHODIMP nsSVGRendererCanvas::GetRenderMode(PRUint16 *aRenderMode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsSVGRendererCanvas::SetRenderMode(PRUint16 aRenderMode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void pushClip (); */
NS_IMETHODIMP nsSVGRendererCanvas::PushClip()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void popClip (); */
NS_IMETHODIMP nsSVGRendererCanvas::PopClip()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setClipRect (in nsIDOMSVGMatrix canvasTM, in float x, in float y, in float width, in float height); */
NS_IMETHODIMP nsSVGRendererCanvas::SetClipRect(nsIDOMSVGMatrix *canvasTM, float x, float y, float width, float height)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void pushSurface (in nsISVGRendererSurface surface); */
NS_IMETHODIMP nsSVGRendererCanvas::PushSurface(nsISVGRendererSurface *surface)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void popSurface (); */
NS_IMETHODIMP nsSVGRendererCanvas::PopSurface()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void compositeSurface (in nsISVGRendererSurface surface, in unsigned long x, in unsigned long y, in float opacity); */
NS_IMETHODIMP nsSVGRendererCanvas::CompositeSurface(nsISVGRendererSurface *surface, PRUint32 x, PRUint32 y, float opacity)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void compositeSurfaceMatrix (in nsISVGRendererSurface surface, in nsIDOMSVGMatrix canvasTM, in float opacity); */
NS_IMETHODIMP nsSVGRendererCanvas::CompositeSurfaceMatrix(nsISVGRendererSurface *surface, nsIDOMSVGMatrix *canvasTM, float opacity)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISVGRendererCanvas_h__ */
