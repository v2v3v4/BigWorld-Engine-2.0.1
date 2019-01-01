/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/svg/renderer/public/nsISVGPathGeometrySource.idl
 */

#ifndef __gen_nsISVGPathGeometrySource_h__
#define __gen_nsISVGPathGeometrySource_h__


#ifndef __gen_nsISVGGeometrySource_h__
#include "nsISVGGeometrySource.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsISVGRendererPathBuilder; /* forward declaration */


/* starting interface:    nsISVGPathGeometrySource */
#define NS_ISVGPATHGEOMETRYSOURCE_IID_STR "73c9350e-6b0b-4458-aa24-aa5333476eff"

#define NS_ISVGPATHGEOMETRYSOURCE_IID \
  {0x73c9350e, 0x6b0b, 0x4458, \
    { 0xaa, 0x24, 0xaa, 0x53, 0x33, 0x47, 0x6e, 0xff }}

/**
 * \addtogroup rendering_backend_interfaces Rendering Backend Interfaces
 * @{
 */
/**
 * Describes a 'path geometry' object in the SVG rendering backend,
 * i.e. a graphical object composed of lines, Bezier curves and
 * elliptical arcs, that can be stroked and filled. The rendering
 * backend maintains an object implementing this interface for each
 * rendering engine-native path geometry object.
 *
 * An engine-native path geometry object will be informed of changes
 * in a path geometry's description with a call to its
 * nsISVGRendererPathGeometry::update() method with an OR-ed
 * combination of the UPDATEMASK_* constants defined in this interface
 * (and its base-interface).
 *
 * @nosubgrouping
 */
class NS_NO_VTABLE nsISVGPathGeometrySource : public nsISVGGeometrySource {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGPATHGEOMETRYSOURCE_IID)

  /**
   * @name Path description
   * @{
   */
/**
   * Write a description of the path to the 'pathBuilder'.
   *
   * @param pathBuilder Object to write path description to.
   */
  /* void constructPath (in nsISVGRendererPathBuilder pathBuilder); */
  NS_IMETHOD ConstructPath(nsISVGRendererPathBuilder *pathBuilder) = 0;

  enum { UPDATEMASK_PATH = 32768U };

  /** @} */
/**
   * @name Hittest mode of operation
   * @{
   */
  enum { HITTEST_MASK_FILL = 1U };

  enum { HITTEST_MASK_STROKE = 2U };

  /**
   * Determines mode of operation expected of the
   * nsISVGRendererPathGeometry::containsPoint() method.  A
   * combination of the 'HITTEST_MASK_*' constants defined in this
   * interface.
   */
  /* readonly attribute unsigned short hittestMask; */
  NS_IMETHOD GetHittestMask(PRUint16 *aHittestMask) = 0;

  enum { UPDATEMASK_HITTEST_MASK = 65536U };

  /** @} */
/**
   * @name Shape rendering hints
   * @{
   */
  enum { SHAPE_RENDERING_AUTO = 0U };

  enum { SHAPE_RENDERING_OPTIMIZESPEED = 1U };

  enum { SHAPE_RENDERING_CRISPEDGES = 2U };

  enum { SHAPE_RENDERING_GEOMETRICPRECISION = 3U };

  /* readonly attribute unsigned short shapeRendering; */
  NS_IMETHOD GetShapeRendering(PRUint16 *aShapeRendering) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGPATHGEOMETRYSOURCE \
  NS_IMETHOD ConstructPath(nsISVGRendererPathBuilder *pathBuilder); \
  NS_IMETHOD GetHittestMask(PRUint16 *aHittestMask); \
  NS_IMETHOD GetShapeRendering(PRUint16 *aShapeRendering); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGPATHGEOMETRYSOURCE(_to) \
  NS_IMETHOD ConstructPath(nsISVGRendererPathBuilder *pathBuilder) { return _to ConstructPath(pathBuilder); } \
  NS_IMETHOD GetHittestMask(PRUint16 *aHittestMask) { return _to GetHittestMask(aHittestMask); } \
  NS_IMETHOD GetShapeRendering(PRUint16 *aShapeRendering) { return _to GetShapeRendering(aShapeRendering); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGPATHGEOMETRYSOURCE(_to) \
  NS_IMETHOD ConstructPath(nsISVGRendererPathBuilder *pathBuilder) { return !_to ? NS_ERROR_NULL_POINTER : _to->ConstructPath(pathBuilder); } \
  NS_IMETHOD GetHittestMask(PRUint16 *aHittestMask) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHittestMask(aHittestMask); } \
  NS_IMETHOD GetShapeRendering(PRUint16 *aShapeRendering) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetShapeRendering(aShapeRendering); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGPathGeometrySource : public nsISVGPathGeometrySource
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGPATHGEOMETRYSOURCE

  nsSVGPathGeometrySource();

private:
  ~nsSVGPathGeometrySource();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGPathGeometrySource, nsISVGPathGeometrySource)

nsSVGPathGeometrySource::nsSVGPathGeometrySource()
{
  /* member initializers and constructor code */
}

nsSVGPathGeometrySource::~nsSVGPathGeometrySource()
{
  /* destructor code */
}

/* void constructPath (in nsISVGRendererPathBuilder pathBuilder); */
NS_IMETHODIMP nsSVGPathGeometrySource::ConstructPath(nsISVGRendererPathBuilder *pathBuilder)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned short hittestMask; */
NS_IMETHODIMP nsSVGPathGeometrySource::GetHittestMask(PRUint16 *aHittestMask)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned short shapeRendering; */
NS_IMETHODIMP nsSVGPathGeometrySource::GetShapeRendering(PRUint16 *aShapeRendering)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISVGPathGeometrySource_h__ */
