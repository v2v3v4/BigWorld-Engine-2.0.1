/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/svg/renderer/public/nsISVGRendererPathBuilder.idl
 */

#ifndef __gen_nsISVGRendererPathBuilder_h__
#define __gen_nsISVGRendererPathBuilder_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsISVGRendererPathBuilder */
#define NS_ISVGRENDERERPATHBUILDER_IID_STR "c3cd294e-39ae-4718-b2bc-87c0fad97a12"

#define NS_ISVGRENDERERPATHBUILDER_IID \
  {0xc3cd294e, 0x39ae, 0x4718, \
    { 0xb2, 0xbc, 0x87, 0xc0, 0xfa, 0xd9, 0x7a, 0x12 }}

/**
 * \addtogroup renderer_interfaces Rendering Engine Interfaces
 * @{
 */
/**
 * One of a number of interfaces (all starting with nsISVGRenderer*)
 * to be implemented by an SVG rendering engine. See nsISVGRenderer
 * for more details.
 *
 * This interface is used by an nsISVGRendererPathGeometry object in
 * a call to nsISVGPathGeometrySource::constructPath() to obtain a
 * native representation of the path described by
 * nsISVGPathGeometrySource.
 */
class NS_NO_VTABLE nsISVGRendererPathBuilder : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGRENDERERPATHBUILDER_IID)

  /**
   * Move current position and start new sub-path.
   *
   * @param x X-coordinate (untransformed).
   * @param y Y-coordinate (untransformed).
   */
  /* void moveto (in float x, in float y); */
  NS_IMETHOD Moveto(float x, float y) = 0;

  /**
   * Draw a straight line from the current position to (x,y). Advance
   * current position to (x,y).
   *
   * @param x X-coordinate of end point (untransformed).
   * @param y Y-coordinate of end point (untransformed).
   */
  /* void lineto (in float x, in float y); */
  NS_IMETHOD Lineto(float x, float y) = 0;

  /**
   * Draw cubic Bezier curve from the current position to (x,y) using
   * (x1,y1) as the control point at the beginning og the curve and
   * (x2,y2) as the control point at the end of the curve. Advance
   * current position to (x,y).
   *
   * @param x  X-coordinate of end point (untransformed).
   * @param y  Y-coordinate of end point (untransformed).
   * @param x1 X-coordinate of first control point (untransformed).
   * @param y1 Y-coordinate of first control point (untransformed).
   * @param x2 X-coordinate of second control point (untransformed).
   * @param y2 Y-coordinate of second control point (untransformed).
   */
  /* void curveto (in float x, in float y, in float x1, in float y1, in float x2, in float y2); */
  NS_IMETHOD Curveto(float x, float y, float x1, float y1, float x2, float y2) = 0;

  /**
   * Draw an elliptical arc from the current position to
   * (x,y). Advance current position to (x,y).
   *
   * @param x            X-coordinate of end point (untransformed).
   * @param y            Y-coordinate of end point (untransformed).
   * @param r1           Radius of ellipse in X direction (untransformed).
   * @param r2           Radius of ellipse in Y direction (untransformed).
   * @param angle        Rotation of ellipse as a whole (untransformed).
   * @param largeArcFlag PR_TRUE: choose the large arc (>=180 degrees),
   *                     PR_FALSE: choose the smaller arc (<=180 degrees)
   * @param sweepFlag    PR_TRUE: sweep in positive angle direction,
   *                     PR_FALSE: sweep in negative angle direction
   */
  /* void arcto (in float x, in float y, in float r1, in float r2, in float angle, in boolean largeArcFlag, in boolean sweepFlag); */
  NS_IMETHOD Arcto(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag) = 0;

  /**
   * Close the current subpath. Move current position back to
   * beginning of subpath.
   *
   * @param newX X-coordinate of new current position (untransformed).
   * @param newY Y-coordinate of new current position (untransformed).
   */
  /* void closePath (out float newX, out float newY); */
  NS_IMETHOD ClosePath(float *newX, float *newY) = 0;

  /**
   * End the path description. Guaranteed to be the last function
   * called.
   */
  /* void endPath (); */
  NS_IMETHOD EndPath(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGRENDERERPATHBUILDER \
  NS_IMETHOD Moveto(float x, float y); \
  NS_IMETHOD Lineto(float x, float y); \
  NS_IMETHOD Curveto(float x, float y, float x1, float y1, float x2, float y2); \
  NS_IMETHOD Arcto(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag); \
  NS_IMETHOD ClosePath(float *newX, float *newY); \
  NS_IMETHOD EndPath(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGRENDERERPATHBUILDER(_to) \
  NS_IMETHOD Moveto(float x, float y) { return _to Moveto(x, y); } \
  NS_IMETHOD Lineto(float x, float y) { return _to Lineto(x, y); } \
  NS_IMETHOD Curveto(float x, float y, float x1, float y1, float x2, float y2) { return _to Curveto(x, y, x1, y1, x2, y2); } \
  NS_IMETHOD Arcto(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag) { return _to Arcto(x, y, r1, r2, angle, largeArcFlag, sweepFlag); } \
  NS_IMETHOD ClosePath(float *newX, float *newY) { return _to ClosePath(newX, newY); } \
  NS_IMETHOD EndPath(void) { return _to EndPath(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGRENDERERPATHBUILDER(_to) \
  NS_IMETHOD Moveto(float x, float y) { return !_to ? NS_ERROR_NULL_POINTER : _to->Moveto(x, y); } \
  NS_IMETHOD Lineto(float x, float y) { return !_to ? NS_ERROR_NULL_POINTER : _to->Lineto(x, y); } \
  NS_IMETHOD Curveto(float x, float y, float x1, float y1, float x2, float y2) { return !_to ? NS_ERROR_NULL_POINTER : _to->Curveto(x, y, x1, y1, x2, y2); } \
  NS_IMETHOD Arcto(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag) { return !_to ? NS_ERROR_NULL_POINTER : _to->Arcto(x, y, r1, r2, angle, largeArcFlag, sweepFlag); } \
  NS_IMETHOD ClosePath(float *newX, float *newY) { return !_to ? NS_ERROR_NULL_POINTER : _to->ClosePath(newX, newY); } \
  NS_IMETHOD EndPath(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->EndPath(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGRendererPathBuilder : public nsISVGRendererPathBuilder
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGRENDERERPATHBUILDER

  nsSVGRendererPathBuilder();

private:
  ~nsSVGRendererPathBuilder();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGRendererPathBuilder, nsISVGRendererPathBuilder)

nsSVGRendererPathBuilder::nsSVGRendererPathBuilder()
{
  /* member initializers and constructor code */
}

nsSVGRendererPathBuilder::~nsSVGRendererPathBuilder()
{
  /* destructor code */
}

/* void moveto (in float x, in float y); */
NS_IMETHODIMP nsSVGRendererPathBuilder::Moveto(float x, float y)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void lineto (in float x, in float y); */
NS_IMETHODIMP nsSVGRendererPathBuilder::Lineto(float x, float y)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void curveto (in float x, in float y, in float x1, in float y1, in float x2, in float y2); */
NS_IMETHODIMP nsSVGRendererPathBuilder::Curveto(float x, float y, float x1, float y1, float x2, float y2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void arcto (in float x, in float y, in float r1, in float r2, in float angle, in boolean largeArcFlag, in boolean sweepFlag); */
NS_IMETHODIMP nsSVGRendererPathBuilder::Arcto(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void closePath (out float newX, out float newY); */
NS_IMETHODIMP nsSVGRendererPathBuilder::ClosePath(float *newX, float *newY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void endPath (); */
NS_IMETHODIMP nsSVGRendererPathBuilder::EndPath()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISVGRendererPathBuilder_h__ */
