/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/svg/nsIDOMSVGPathElement.idl
 */

#ifndef __gen_nsIDOMSVGPathElement_h__
#define __gen_nsIDOMSVGPathElement_h__


#ifndef __gen_nsIDOMSVGElement_h__
#include "nsIDOMSVGElement.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMSVGAnimatedNumber; /* forward declaration */

class nsIDOMSVGPoint; /* forward declaration */

class nsIDOMSVGPathSegClosePath; /* forward declaration */

class nsIDOMSVGPathSegMovetoAbs; /* forward declaration */

class nsIDOMSVGPathSegMovetoRel; /* forward declaration */

class nsIDOMSVGPathSegLinetoAbs; /* forward declaration */

class nsIDOMSVGPathSegLinetoRel; /* forward declaration */

class nsIDOMSVGPathSegCurvetoCubicAbs; /* forward declaration */

class nsIDOMSVGPathSegCurvetoCubicRel; /* forward declaration */

class nsIDOMSVGPathSegCurvetoQuadraticAbs; /* forward declaration */

class nsIDOMSVGPathSegCurvetoQuadraticRel; /* forward declaration */

class nsIDOMSVGPathSegArcAbs; /* forward declaration */

class nsIDOMSVGPathSegArcRel; /* forward declaration */

class nsIDOMSVGPathSegLinetoHorizontalAbs; /* forward declaration */

class nsIDOMSVGPathSegLinetoHorizontalRel; /* forward declaration */

class nsIDOMSVGPathSegLinetoVerticalAbs; /* forward declaration */

class nsIDOMSVGPathSegLinetoVerticalRel; /* forward declaration */

class nsIDOMSVGPathSegCurvetoCubicSmoothAbs; /* forward declaration */

class nsIDOMSVGPathSegCurvetoCubicSmoothRel; /* forward declaration */

class nsIDOMSVGPathSegCurvetoQuadraticSmoothAbs; /* forward declaration */

class nsIDOMSVGPathSegCurvetoQuadraticSmoothRel; /* forward declaration */


/* starting interface:    nsIDOMSVGPathElement */
#define NS_IDOMSVGPATHELEMENT_IID_STR "2b19e692-3338-440f-a998-3cb1e8474999"

#define NS_IDOMSVGPATHELEMENT_IID \
  {0x2b19e692, 0x3338, 0x440f, \
    { 0xa9, 0x98, 0x3c, 0xb1, 0xe8, 0x47, 0x49, 0x99 }}

class NS_NO_VTABLE nsIDOMSVGPathElement : public nsIDOMSVGElement {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHELEMENT_IID)

  /* readonly attribute nsIDOMSVGAnimatedNumber pathLength; */
  NS_IMETHOD GetPathLength(nsIDOMSVGAnimatedNumber * *aPathLength) = 0;

  /* float getTotalLength (); */
  NS_IMETHOD GetTotalLength(float *_retval) = 0;

  /* nsIDOMSVGPoint getPointAtLength (in float distance); */
  NS_IMETHOD GetPointAtLength(float distance, nsIDOMSVGPoint **_retval) = 0;

  /* unsigned long getPathSegAtLength (in float distance); */
  NS_IMETHOD GetPathSegAtLength(float distance, PRUint32 *_retval) = 0;

  /* nsIDOMSVGPathSegClosePath createSVGPathSegClosePath (); */
  NS_IMETHOD CreateSVGPathSegClosePath(nsIDOMSVGPathSegClosePath **_retval) = 0;

  /* nsIDOMSVGPathSegMovetoAbs createSVGPathSegMovetoAbs (in float x, in float y); */
  NS_IMETHOD CreateSVGPathSegMovetoAbs(float x, float y, nsIDOMSVGPathSegMovetoAbs **_retval) = 0;

  /* nsIDOMSVGPathSegMovetoRel createSVGPathSegMovetoRel (in float x, in float y); */
  NS_IMETHOD CreateSVGPathSegMovetoRel(float x, float y, nsIDOMSVGPathSegMovetoRel **_retval) = 0;

  /* nsIDOMSVGPathSegLinetoAbs createSVGPathSegLinetoAbs (in float x, in float y); */
  NS_IMETHOD CreateSVGPathSegLinetoAbs(float x, float y, nsIDOMSVGPathSegLinetoAbs **_retval) = 0;

  /* nsIDOMSVGPathSegLinetoRel createSVGPathSegLinetoRel (in float x, in float y); */
  NS_IMETHOD CreateSVGPathSegLinetoRel(float x, float y, nsIDOMSVGPathSegLinetoRel **_retval) = 0;

  /* nsIDOMSVGPathSegCurvetoCubicAbs createSVGPathSegCurvetoCubicAbs (in float x, in float y, in float x1, in float y1, in float x2, in float y2); */
  NS_IMETHOD CreateSVGPathSegCurvetoCubicAbs(float x, float y, float x1, float y1, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicAbs **_retval) = 0;

  /* nsIDOMSVGPathSegCurvetoCubicRel createSVGPathSegCurvetoCubicRel (in float x, in float y, in float x1, in float y1, in float x2, in float y2); */
  NS_IMETHOD CreateSVGPathSegCurvetoCubicRel(float x, float y, float x1, float y1, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicRel **_retval) = 0;

  /* nsIDOMSVGPathSegCurvetoQuadraticAbs createSVGPathSegCurvetoQuadraticAbs (in float x, in float y, in float x1, in float y1); */
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticAbs(float x, float y, float x1, float y1, nsIDOMSVGPathSegCurvetoQuadraticAbs **_retval) = 0;

  /* nsIDOMSVGPathSegCurvetoQuadraticRel createSVGPathSegCurvetoQuadraticRel (in float x, in float y, in float x1, in float y1); */
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticRel(float x, float y, float x1, float y1, nsIDOMSVGPathSegCurvetoQuadraticRel **_retval) = 0;

  /* nsIDOMSVGPathSegArcAbs createSVGPathSegArcAbs (in float x, in float y, in float r1, in float r2, in float angle, in boolean largeArcFlag, in boolean sweepFlag); */
  NS_IMETHOD CreateSVGPathSegArcAbs(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag, nsIDOMSVGPathSegArcAbs **_retval) = 0;

  /* nsIDOMSVGPathSegArcRel createSVGPathSegArcRel (in float x, in float y, in float r1, in float r2, in float angle, in boolean largeArcFlag, in boolean sweepFlag); */
  NS_IMETHOD CreateSVGPathSegArcRel(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag, nsIDOMSVGPathSegArcRel **_retval) = 0;

  /* nsIDOMSVGPathSegLinetoHorizontalAbs createSVGPathSegLinetoHorizontalAbs (in float x); */
  NS_IMETHOD CreateSVGPathSegLinetoHorizontalAbs(float x, nsIDOMSVGPathSegLinetoHorizontalAbs **_retval) = 0;

  /* nsIDOMSVGPathSegLinetoHorizontalRel createSVGPathSegLinetoHorizontalRel (in float x); */
  NS_IMETHOD CreateSVGPathSegLinetoHorizontalRel(float x, nsIDOMSVGPathSegLinetoHorizontalRel **_retval) = 0;

  /* nsIDOMSVGPathSegLinetoVerticalAbs createSVGPathSegLinetoVerticalAbs (in float y); */
  NS_IMETHOD CreateSVGPathSegLinetoVerticalAbs(float y, nsIDOMSVGPathSegLinetoVerticalAbs **_retval) = 0;

  /* nsIDOMSVGPathSegLinetoVerticalRel createSVGPathSegLinetoVerticalRel (in float y); */
  NS_IMETHOD CreateSVGPathSegLinetoVerticalRel(float y, nsIDOMSVGPathSegLinetoVerticalRel **_retval) = 0;

  /* nsIDOMSVGPathSegCurvetoCubicSmoothAbs createSVGPathSegCurvetoCubicSmoothAbs (in float x, in float y, in float x2, in float y2); */
  NS_IMETHOD CreateSVGPathSegCurvetoCubicSmoothAbs(float x, float y, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicSmoothAbs **_retval) = 0;

  /* nsIDOMSVGPathSegCurvetoCubicSmoothRel createSVGPathSegCurvetoCubicSmoothRel (in float x, in float y, in float x2, in float y2); */
  NS_IMETHOD CreateSVGPathSegCurvetoCubicSmoothRel(float x, float y, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicSmoothRel **_retval) = 0;

  /* nsIDOMSVGPathSegCurvetoQuadraticSmoothAbs createSVGPathSegCurvetoQuadraticSmoothAbs (in float x, in float y); */
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticSmoothAbs(float x, float y, nsIDOMSVGPathSegCurvetoQuadraticSmoothAbs **_retval) = 0;

  /* nsIDOMSVGPathSegCurvetoQuadraticSmoothRel createSVGPathSegCurvetoQuadraticSmoothRel (in float x, in float y); */
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticSmoothRel(float x, float y, nsIDOMSVGPathSegCurvetoQuadraticSmoothRel **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHELEMENT \
  NS_IMETHOD GetPathLength(nsIDOMSVGAnimatedNumber * *aPathLength); \
  NS_IMETHOD GetTotalLength(float *_retval); \
  NS_IMETHOD GetPointAtLength(float distance, nsIDOMSVGPoint **_retval); \
  NS_IMETHOD GetPathSegAtLength(float distance, PRUint32 *_retval); \
  NS_IMETHOD CreateSVGPathSegClosePath(nsIDOMSVGPathSegClosePath **_retval); \
  NS_IMETHOD CreateSVGPathSegMovetoAbs(float x, float y, nsIDOMSVGPathSegMovetoAbs **_retval); \
  NS_IMETHOD CreateSVGPathSegMovetoRel(float x, float y, nsIDOMSVGPathSegMovetoRel **_retval); \
  NS_IMETHOD CreateSVGPathSegLinetoAbs(float x, float y, nsIDOMSVGPathSegLinetoAbs **_retval); \
  NS_IMETHOD CreateSVGPathSegLinetoRel(float x, float y, nsIDOMSVGPathSegLinetoRel **_retval); \
  NS_IMETHOD CreateSVGPathSegCurvetoCubicAbs(float x, float y, float x1, float y1, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicAbs **_retval); \
  NS_IMETHOD CreateSVGPathSegCurvetoCubicRel(float x, float y, float x1, float y1, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicRel **_retval); \
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticAbs(float x, float y, float x1, float y1, nsIDOMSVGPathSegCurvetoQuadraticAbs **_retval); \
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticRel(float x, float y, float x1, float y1, nsIDOMSVGPathSegCurvetoQuadraticRel **_retval); \
  NS_IMETHOD CreateSVGPathSegArcAbs(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag, nsIDOMSVGPathSegArcAbs **_retval); \
  NS_IMETHOD CreateSVGPathSegArcRel(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag, nsIDOMSVGPathSegArcRel **_retval); \
  NS_IMETHOD CreateSVGPathSegLinetoHorizontalAbs(float x, nsIDOMSVGPathSegLinetoHorizontalAbs **_retval); \
  NS_IMETHOD CreateSVGPathSegLinetoHorizontalRel(float x, nsIDOMSVGPathSegLinetoHorizontalRel **_retval); \
  NS_IMETHOD CreateSVGPathSegLinetoVerticalAbs(float y, nsIDOMSVGPathSegLinetoVerticalAbs **_retval); \
  NS_IMETHOD CreateSVGPathSegLinetoVerticalRel(float y, nsIDOMSVGPathSegLinetoVerticalRel **_retval); \
  NS_IMETHOD CreateSVGPathSegCurvetoCubicSmoothAbs(float x, float y, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicSmoothAbs **_retval); \
  NS_IMETHOD CreateSVGPathSegCurvetoCubicSmoothRel(float x, float y, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicSmoothRel **_retval); \
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticSmoothAbs(float x, float y, nsIDOMSVGPathSegCurvetoQuadraticSmoothAbs **_retval); \
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticSmoothRel(float x, float y, nsIDOMSVGPathSegCurvetoQuadraticSmoothRel **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHELEMENT(_to) \
  NS_IMETHOD GetPathLength(nsIDOMSVGAnimatedNumber * *aPathLength) { return _to GetPathLength(aPathLength); } \
  NS_IMETHOD GetTotalLength(float *_retval) { return _to GetTotalLength(_retval); } \
  NS_IMETHOD GetPointAtLength(float distance, nsIDOMSVGPoint **_retval) { return _to GetPointAtLength(distance, _retval); } \
  NS_IMETHOD GetPathSegAtLength(float distance, PRUint32 *_retval) { return _to GetPathSegAtLength(distance, _retval); } \
  NS_IMETHOD CreateSVGPathSegClosePath(nsIDOMSVGPathSegClosePath **_retval) { return _to CreateSVGPathSegClosePath(_retval); } \
  NS_IMETHOD CreateSVGPathSegMovetoAbs(float x, float y, nsIDOMSVGPathSegMovetoAbs **_retval) { return _to CreateSVGPathSegMovetoAbs(x, y, _retval); } \
  NS_IMETHOD CreateSVGPathSegMovetoRel(float x, float y, nsIDOMSVGPathSegMovetoRel **_retval) { return _to CreateSVGPathSegMovetoRel(x, y, _retval); } \
  NS_IMETHOD CreateSVGPathSegLinetoAbs(float x, float y, nsIDOMSVGPathSegLinetoAbs **_retval) { return _to CreateSVGPathSegLinetoAbs(x, y, _retval); } \
  NS_IMETHOD CreateSVGPathSegLinetoRel(float x, float y, nsIDOMSVGPathSegLinetoRel **_retval) { return _to CreateSVGPathSegLinetoRel(x, y, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoCubicAbs(float x, float y, float x1, float y1, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicAbs **_retval) { return _to CreateSVGPathSegCurvetoCubicAbs(x, y, x1, y1, x2, y2, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoCubicRel(float x, float y, float x1, float y1, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicRel **_retval) { return _to CreateSVGPathSegCurvetoCubicRel(x, y, x1, y1, x2, y2, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticAbs(float x, float y, float x1, float y1, nsIDOMSVGPathSegCurvetoQuadraticAbs **_retval) { return _to CreateSVGPathSegCurvetoQuadraticAbs(x, y, x1, y1, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticRel(float x, float y, float x1, float y1, nsIDOMSVGPathSegCurvetoQuadraticRel **_retval) { return _to CreateSVGPathSegCurvetoQuadraticRel(x, y, x1, y1, _retval); } \
  NS_IMETHOD CreateSVGPathSegArcAbs(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag, nsIDOMSVGPathSegArcAbs **_retval) { return _to CreateSVGPathSegArcAbs(x, y, r1, r2, angle, largeArcFlag, sweepFlag, _retval); } \
  NS_IMETHOD CreateSVGPathSegArcRel(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag, nsIDOMSVGPathSegArcRel **_retval) { return _to CreateSVGPathSegArcRel(x, y, r1, r2, angle, largeArcFlag, sweepFlag, _retval); } \
  NS_IMETHOD CreateSVGPathSegLinetoHorizontalAbs(float x, nsIDOMSVGPathSegLinetoHorizontalAbs **_retval) { return _to CreateSVGPathSegLinetoHorizontalAbs(x, _retval); } \
  NS_IMETHOD CreateSVGPathSegLinetoHorizontalRel(float x, nsIDOMSVGPathSegLinetoHorizontalRel **_retval) { return _to CreateSVGPathSegLinetoHorizontalRel(x, _retval); } \
  NS_IMETHOD CreateSVGPathSegLinetoVerticalAbs(float y, nsIDOMSVGPathSegLinetoVerticalAbs **_retval) { return _to CreateSVGPathSegLinetoVerticalAbs(y, _retval); } \
  NS_IMETHOD CreateSVGPathSegLinetoVerticalRel(float y, nsIDOMSVGPathSegLinetoVerticalRel **_retval) { return _to CreateSVGPathSegLinetoVerticalRel(y, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoCubicSmoothAbs(float x, float y, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicSmoothAbs **_retval) { return _to CreateSVGPathSegCurvetoCubicSmoothAbs(x, y, x2, y2, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoCubicSmoothRel(float x, float y, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicSmoothRel **_retval) { return _to CreateSVGPathSegCurvetoCubicSmoothRel(x, y, x2, y2, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticSmoothAbs(float x, float y, nsIDOMSVGPathSegCurvetoQuadraticSmoothAbs **_retval) { return _to CreateSVGPathSegCurvetoQuadraticSmoothAbs(x, y, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticSmoothRel(float x, float y, nsIDOMSVGPathSegCurvetoQuadraticSmoothRel **_retval) { return _to CreateSVGPathSegCurvetoQuadraticSmoothRel(x, y, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHELEMENT(_to) \
  NS_IMETHOD GetPathLength(nsIDOMSVGAnimatedNumber * *aPathLength) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPathLength(aPathLength); } \
  NS_IMETHOD GetTotalLength(float *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTotalLength(_retval); } \
  NS_IMETHOD GetPointAtLength(float distance, nsIDOMSVGPoint **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPointAtLength(distance, _retval); } \
  NS_IMETHOD GetPathSegAtLength(float distance, PRUint32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPathSegAtLength(distance, _retval); } \
  NS_IMETHOD CreateSVGPathSegClosePath(nsIDOMSVGPathSegClosePath **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegClosePath(_retval); } \
  NS_IMETHOD CreateSVGPathSegMovetoAbs(float x, float y, nsIDOMSVGPathSegMovetoAbs **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegMovetoAbs(x, y, _retval); } \
  NS_IMETHOD CreateSVGPathSegMovetoRel(float x, float y, nsIDOMSVGPathSegMovetoRel **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegMovetoRel(x, y, _retval); } \
  NS_IMETHOD CreateSVGPathSegLinetoAbs(float x, float y, nsIDOMSVGPathSegLinetoAbs **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegLinetoAbs(x, y, _retval); } \
  NS_IMETHOD CreateSVGPathSegLinetoRel(float x, float y, nsIDOMSVGPathSegLinetoRel **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegLinetoRel(x, y, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoCubicAbs(float x, float y, float x1, float y1, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicAbs **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegCurvetoCubicAbs(x, y, x1, y1, x2, y2, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoCubicRel(float x, float y, float x1, float y1, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicRel **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegCurvetoCubicRel(x, y, x1, y1, x2, y2, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticAbs(float x, float y, float x1, float y1, nsIDOMSVGPathSegCurvetoQuadraticAbs **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegCurvetoQuadraticAbs(x, y, x1, y1, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticRel(float x, float y, float x1, float y1, nsIDOMSVGPathSegCurvetoQuadraticRel **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegCurvetoQuadraticRel(x, y, x1, y1, _retval); } \
  NS_IMETHOD CreateSVGPathSegArcAbs(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag, nsIDOMSVGPathSegArcAbs **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegArcAbs(x, y, r1, r2, angle, largeArcFlag, sweepFlag, _retval); } \
  NS_IMETHOD CreateSVGPathSegArcRel(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag, nsIDOMSVGPathSegArcRel **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegArcRel(x, y, r1, r2, angle, largeArcFlag, sweepFlag, _retval); } \
  NS_IMETHOD CreateSVGPathSegLinetoHorizontalAbs(float x, nsIDOMSVGPathSegLinetoHorizontalAbs **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegLinetoHorizontalAbs(x, _retval); } \
  NS_IMETHOD CreateSVGPathSegLinetoHorizontalRel(float x, nsIDOMSVGPathSegLinetoHorizontalRel **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegLinetoHorizontalRel(x, _retval); } \
  NS_IMETHOD CreateSVGPathSegLinetoVerticalAbs(float y, nsIDOMSVGPathSegLinetoVerticalAbs **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegLinetoVerticalAbs(y, _retval); } \
  NS_IMETHOD CreateSVGPathSegLinetoVerticalRel(float y, nsIDOMSVGPathSegLinetoVerticalRel **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegLinetoVerticalRel(y, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoCubicSmoothAbs(float x, float y, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicSmoothAbs **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegCurvetoCubicSmoothAbs(x, y, x2, y2, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoCubicSmoothRel(float x, float y, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicSmoothRel **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegCurvetoCubicSmoothRel(x, y, x2, y2, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticSmoothAbs(float x, float y, nsIDOMSVGPathSegCurvetoQuadraticSmoothAbs **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegCurvetoQuadraticSmoothAbs(x, y, _retval); } \
  NS_IMETHOD CreateSVGPathSegCurvetoQuadraticSmoothRel(float x, float y, nsIDOMSVGPathSegCurvetoQuadraticSmoothRel **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPathSegCurvetoQuadraticSmoothRel(x, y, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathElement : public nsIDOMSVGPathElement
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHELEMENT

  nsDOMSVGPathElement();

private:
  ~nsDOMSVGPathElement();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathElement, nsIDOMSVGPathElement)

nsDOMSVGPathElement::nsDOMSVGPathElement()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathElement::~nsDOMSVGPathElement()
{
  /* destructor code */
}

/* readonly attribute nsIDOMSVGAnimatedNumber pathLength; */
NS_IMETHODIMP nsDOMSVGPathElement::GetPathLength(nsIDOMSVGAnimatedNumber * *aPathLength)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* float getTotalLength (); */
NS_IMETHODIMP nsDOMSVGPathElement::GetTotalLength(float *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPoint getPointAtLength (in float distance); */
NS_IMETHODIMP nsDOMSVGPathElement::GetPointAtLength(float distance, nsIDOMSVGPoint **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* unsigned long getPathSegAtLength (in float distance); */
NS_IMETHODIMP nsDOMSVGPathElement::GetPathSegAtLength(float distance, PRUint32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegClosePath createSVGPathSegClosePath (); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegClosePath(nsIDOMSVGPathSegClosePath **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegMovetoAbs createSVGPathSegMovetoAbs (in float x, in float y); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegMovetoAbs(float x, float y, nsIDOMSVGPathSegMovetoAbs **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegMovetoRel createSVGPathSegMovetoRel (in float x, in float y); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegMovetoRel(float x, float y, nsIDOMSVGPathSegMovetoRel **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegLinetoAbs createSVGPathSegLinetoAbs (in float x, in float y); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegLinetoAbs(float x, float y, nsIDOMSVGPathSegLinetoAbs **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegLinetoRel createSVGPathSegLinetoRel (in float x, in float y); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegLinetoRel(float x, float y, nsIDOMSVGPathSegLinetoRel **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegCurvetoCubicAbs createSVGPathSegCurvetoCubicAbs (in float x, in float y, in float x1, in float y1, in float x2, in float y2); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegCurvetoCubicAbs(float x, float y, float x1, float y1, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicAbs **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegCurvetoCubicRel createSVGPathSegCurvetoCubicRel (in float x, in float y, in float x1, in float y1, in float x2, in float y2); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegCurvetoCubicRel(float x, float y, float x1, float y1, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicRel **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegCurvetoQuadraticAbs createSVGPathSegCurvetoQuadraticAbs (in float x, in float y, in float x1, in float y1); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegCurvetoQuadraticAbs(float x, float y, float x1, float y1, nsIDOMSVGPathSegCurvetoQuadraticAbs **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegCurvetoQuadraticRel createSVGPathSegCurvetoQuadraticRel (in float x, in float y, in float x1, in float y1); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegCurvetoQuadraticRel(float x, float y, float x1, float y1, nsIDOMSVGPathSegCurvetoQuadraticRel **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegArcAbs createSVGPathSegArcAbs (in float x, in float y, in float r1, in float r2, in float angle, in boolean largeArcFlag, in boolean sweepFlag); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegArcAbs(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag, nsIDOMSVGPathSegArcAbs **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegArcRel createSVGPathSegArcRel (in float x, in float y, in float r1, in float r2, in float angle, in boolean largeArcFlag, in boolean sweepFlag); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegArcRel(float x, float y, float r1, float r2, float angle, PRBool largeArcFlag, PRBool sweepFlag, nsIDOMSVGPathSegArcRel **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegLinetoHorizontalAbs createSVGPathSegLinetoHorizontalAbs (in float x); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegLinetoHorizontalAbs(float x, nsIDOMSVGPathSegLinetoHorizontalAbs **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegLinetoHorizontalRel createSVGPathSegLinetoHorizontalRel (in float x); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegLinetoHorizontalRel(float x, nsIDOMSVGPathSegLinetoHorizontalRel **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegLinetoVerticalAbs createSVGPathSegLinetoVerticalAbs (in float y); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegLinetoVerticalAbs(float y, nsIDOMSVGPathSegLinetoVerticalAbs **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegLinetoVerticalRel createSVGPathSegLinetoVerticalRel (in float y); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegLinetoVerticalRel(float y, nsIDOMSVGPathSegLinetoVerticalRel **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegCurvetoCubicSmoothAbs createSVGPathSegCurvetoCubicSmoothAbs (in float x, in float y, in float x2, in float y2); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegCurvetoCubicSmoothAbs(float x, float y, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicSmoothAbs **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegCurvetoCubicSmoothRel createSVGPathSegCurvetoCubicSmoothRel (in float x, in float y, in float x2, in float y2); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegCurvetoCubicSmoothRel(float x, float y, float x2, float y2, nsIDOMSVGPathSegCurvetoCubicSmoothRel **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegCurvetoQuadraticSmoothAbs createSVGPathSegCurvetoQuadraticSmoothAbs (in float x, in float y); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegCurvetoQuadraticSmoothAbs(float x, float y, nsIDOMSVGPathSegCurvetoQuadraticSmoothAbs **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPathSegCurvetoQuadraticSmoothRel createSVGPathSegCurvetoQuadraticSmoothRel (in float x, in float y); */
NS_IMETHODIMP nsDOMSVGPathElement::CreateSVGPathSegCurvetoQuadraticSmoothRel(float x, float y, nsIDOMSVGPathSegCurvetoQuadraticSmoothRel **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMSVGPathElement_h__ */
