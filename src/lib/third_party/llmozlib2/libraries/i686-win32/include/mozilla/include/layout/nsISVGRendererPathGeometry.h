/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/svg/renderer/public/nsISVGRendererPathGeometry.idl
 */

#ifndef __gen_nsISVGRendererPathGeometry_h__
#define __gen_nsISVGRendererPathGeometry_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsISVGRendererRegion; /* forward declaration */

class nsISVGRendererCanvas; /* forward declaration */

class nsIDOMSVGRect; /* forward declaration */

class nsSVGPathData; /* forward declaration */


/* starting interface:    nsISVGRendererPathGeometry */
#define NS_ISVGRENDERERPATHGEOMETRY_IID_STR "95f9e432-90e6-48c1-a242-5346517b93d1"

#define NS_ISVGRENDERERPATHGEOMETRY_IID \
  {0x95f9e432, 0x90e6, 0x48c1, \
    { 0xa2, 0x42, 0x53, 0x46, 0x51, 0x7b, 0x93, 0xd1 }}

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
 * rendering engine-native path objects.
 *
 * A path geometry object is instantiated by the rendering backend for
 * a given nsISVGPathGeometrySource object with a call to
 * nsISVGRenderer::createPathGeometry(). The path geometry object is
 * assumed to store a reference to its associated source object and
 * provide rendering, hit-testing and metrics for the path described
 * by the nsISVGPathGeometrySource members.
 */
class NS_NO_VTABLE nsISVGRendererPathGeometry : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGRENDERERPATHGEOMETRY_IID)

  /**
   * Paint this object.
   *
   * @param canvas The canvas to render to.
   */
  /* void render (in nsISVGRendererCanvas canvas); */
  NS_IMETHOD Render(nsISVGRendererCanvas *canvas) = 0;

  /**
   * Called by this object's corresponding nsISVGPathGeometrySource as
   * a notification that some of the source's data (identified by
   * paramter 'updatemask') has changed.
   *
   * @param updatemask An OR-ed combination of the UPDATEMASK_*
   * constants defined in nsISVGPathGeometrySource.
   * @return Region that needs to be redrawn.
   */
  /* nsISVGRendererRegion update (in unsigned long updatemask); */
  NS_IMETHOD Update(PRUint32 updatemask, nsISVGRendererRegion **_retval) = 0;

  /**
   * Get a region object describing the area covered with paint by
   * this path geometry.
   *
   * @return Covered region.
   */
  /* nsISVGRendererRegion getCoveredRegion (); */
  NS_IMETHOD GetCoveredRegion(nsISVGRendererRegion **_retval) = 0;

  /**
   * Hit-testing method. Does this path geometry (with all relevant
   * transformations applied) contain the point x,y? Mode of operation
   * (e.g. whether to test fill or stroke) is determined by
   * nsISVGPathGeometrySource::hittestMask.
   *
   * @param x X-coordinate of test point.  @param y Y-coordinate of
   * test point.
   * @return PR_TRUE if the path geometry contains the point,
   * PR_FALSE otherwise.
   */
  /* boolean containsPoint (in float x, in float y); */
  NS_IMETHOD ContainsPoint(float x, float y, PRBool *_retval) = 0;

  /**
   * Bounding box (does not include stroke width)
   */
  /* readonly attribute nsIDOMSVGRect boundingBox; */
  NS_IMETHOD GetBoundingBox(nsIDOMSVGRect * *aBoundingBox) = 0;

  /* void flatten (out nsSVGPathData data); */
  NS_IMETHOD Flatten(nsSVGPathData **data) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGRENDERERPATHGEOMETRY \
  NS_IMETHOD Render(nsISVGRendererCanvas *canvas); \
  NS_IMETHOD Update(PRUint32 updatemask, nsISVGRendererRegion **_retval); \
  NS_IMETHOD GetCoveredRegion(nsISVGRendererRegion **_retval); \
  NS_IMETHOD ContainsPoint(float x, float y, PRBool *_retval); \
  NS_IMETHOD GetBoundingBox(nsIDOMSVGRect * *aBoundingBox); \
  NS_IMETHOD Flatten(nsSVGPathData **data); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGRENDERERPATHGEOMETRY(_to) \
  NS_IMETHOD Render(nsISVGRendererCanvas *canvas) { return _to Render(canvas); } \
  NS_IMETHOD Update(PRUint32 updatemask, nsISVGRendererRegion **_retval) { return _to Update(updatemask, _retval); } \
  NS_IMETHOD GetCoveredRegion(nsISVGRendererRegion **_retval) { return _to GetCoveredRegion(_retval); } \
  NS_IMETHOD ContainsPoint(float x, float y, PRBool *_retval) { return _to ContainsPoint(x, y, _retval); } \
  NS_IMETHOD GetBoundingBox(nsIDOMSVGRect * *aBoundingBox) { return _to GetBoundingBox(aBoundingBox); } \
  NS_IMETHOD Flatten(nsSVGPathData **data) { return _to Flatten(data); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGRENDERERPATHGEOMETRY(_to) \
  NS_IMETHOD Render(nsISVGRendererCanvas *canvas) { return !_to ? NS_ERROR_NULL_POINTER : _to->Render(canvas); } \
  NS_IMETHOD Update(PRUint32 updatemask, nsISVGRendererRegion **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Update(updatemask, _retval); } \
  NS_IMETHOD GetCoveredRegion(nsISVGRendererRegion **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCoveredRegion(_retval); } \
  NS_IMETHOD ContainsPoint(float x, float y, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ContainsPoint(x, y, _retval); } \
  NS_IMETHOD GetBoundingBox(nsIDOMSVGRect * *aBoundingBox) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBoundingBox(aBoundingBox); } \
  NS_IMETHOD Flatten(nsSVGPathData **data) { return !_to ? NS_ERROR_NULL_POINTER : _to->Flatten(data); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGRendererPathGeometry : public nsISVGRendererPathGeometry
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGRENDERERPATHGEOMETRY

  nsSVGRendererPathGeometry();

private:
  ~nsSVGRendererPathGeometry();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGRendererPathGeometry, nsISVGRendererPathGeometry)

nsSVGRendererPathGeometry::nsSVGRendererPathGeometry()
{
  /* member initializers and constructor code */
}

nsSVGRendererPathGeometry::~nsSVGRendererPathGeometry()
{
  /* destructor code */
}

/* void render (in nsISVGRendererCanvas canvas); */
NS_IMETHODIMP nsSVGRendererPathGeometry::Render(nsISVGRendererCanvas *canvas)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsISVGRendererRegion update (in unsigned long updatemask); */
NS_IMETHODIMP nsSVGRendererPathGeometry::Update(PRUint32 updatemask, nsISVGRendererRegion **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsISVGRendererRegion getCoveredRegion (); */
NS_IMETHODIMP nsSVGRendererPathGeometry::GetCoveredRegion(nsISVGRendererRegion **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean containsPoint (in float x, in float y); */
NS_IMETHODIMP nsSVGRendererPathGeometry::ContainsPoint(float x, float y, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMSVGRect boundingBox; */
NS_IMETHODIMP nsSVGRendererPathGeometry::GetBoundingBox(nsIDOMSVGRect * *aBoundingBox)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void flatten (out nsSVGPathData data); */
NS_IMETHODIMP nsSVGRendererPathGeometry::Flatten(nsSVGPathData **data)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISVGRendererPathGeometry_h__ */
