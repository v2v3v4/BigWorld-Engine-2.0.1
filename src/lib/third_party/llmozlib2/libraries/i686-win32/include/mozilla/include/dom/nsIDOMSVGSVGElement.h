/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/svg/nsIDOMSVGSVGElement.idl
 */

#ifndef __gen_nsIDOMSVGSVGElement_h__
#define __gen_nsIDOMSVGSVGElement_h__


#ifndef __gen_nsIDOMSVGElement_h__
#include "nsIDOMSVGElement.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMSVGAnimatedLength; /* forward declaration */

class nsIDOMSVGRect; /* forward declaration */

class nsIDOMSVGViewSpec; /* forward declaration */

class nsIDOMSVGPoint; /* forward declaration */

class nsIDOMSVGNumber; /* forward declaration */

class nsIDOMSVGLength; /* forward declaration */

class nsIDOMSVGAngle; /* forward declaration */

class nsIDOMSVGMatrix; /* forward declaration */

class nsIDOMSVGTransform; /* forward declaration */


/* starting interface:    nsIDOMSVGSVGElement */
#define NS_IDOMSVGSVGELEMENT_IID_STR "67b8f41e-3577-4c8a-b1de-bef51186fe08"

#define NS_IDOMSVGSVGELEMENT_IID \
  {0x67b8f41e, 0x3577, 0x4c8a, \
    { 0xb1, 0xde, 0xbe, 0xf5, 0x11, 0x86, 0xfe, 0x08 }}

class NS_NO_VTABLE nsIDOMSVGSVGElement : public nsIDOMSVGElement {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGSVGELEMENT_IID)

  /* readonly attribute nsIDOMSVGAnimatedLength x; */
  NS_IMETHOD GetX(nsIDOMSVGAnimatedLength * *aX) = 0;

  /* readonly attribute nsIDOMSVGAnimatedLength y; */
  NS_IMETHOD GetY(nsIDOMSVGAnimatedLength * *aY) = 0;

  /* readonly attribute nsIDOMSVGAnimatedLength width; */
  NS_IMETHOD GetWidth(nsIDOMSVGAnimatedLength * *aWidth) = 0;

  /* readonly attribute nsIDOMSVGAnimatedLength height; */
  NS_IMETHOD GetHeight(nsIDOMSVGAnimatedLength * *aHeight) = 0;

  /* attribute DOMString contentScriptType; */
  NS_IMETHOD GetContentScriptType(nsAString & aContentScriptType) = 0;
  NS_IMETHOD SetContentScriptType(const nsAString & aContentScriptType) = 0;

  /* attribute DOMString contentStyleType; */
  NS_IMETHOD GetContentStyleType(nsAString & aContentStyleType) = 0;
  NS_IMETHOD SetContentStyleType(const nsAString & aContentStyleType) = 0;

  /* readonly attribute nsIDOMSVGRect viewport; */
  NS_IMETHOD GetViewport(nsIDOMSVGRect * *aViewport) = 0;

  /* readonly attribute float pixelUnitToMillimeterX; */
  NS_IMETHOD GetPixelUnitToMillimeterX(float *aPixelUnitToMillimeterX) = 0;

  /* readonly attribute float pixelUnitToMillimeterY; */
  NS_IMETHOD GetPixelUnitToMillimeterY(float *aPixelUnitToMillimeterY) = 0;

  /* readonly attribute float screenPixelToMillimeterX; */
  NS_IMETHOD GetScreenPixelToMillimeterX(float *aScreenPixelToMillimeterX) = 0;

  /* readonly attribute float screenPixelToMillimeterY; */
  NS_IMETHOD GetScreenPixelToMillimeterY(float *aScreenPixelToMillimeterY) = 0;

  /* attribute boolean useCurrentView; */
  NS_IMETHOD GetUseCurrentView(PRBool *aUseCurrentView) = 0;
  NS_IMETHOD SetUseCurrentView(PRBool aUseCurrentView) = 0;

  /* readonly attribute nsIDOMSVGViewSpec currentView; */
  NS_IMETHOD GetCurrentView(nsIDOMSVGViewSpec * *aCurrentView) = 0;

  /* attribute float currentScale; */
  NS_IMETHOD GetCurrentScale(float *aCurrentScale) = 0;
  NS_IMETHOD SetCurrentScale(float aCurrentScale) = 0;

  /* readonly attribute nsIDOMSVGPoint currentTranslate; */
  NS_IMETHOD GetCurrentTranslate(nsIDOMSVGPoint * *aCurrentTranslate) = 0;

  /* unsigned long suspendRedraw (in unsigned long max_wait_milliseconds); */
  NS_IMETHOD SuspendRedraw(PRUint32 max_wait_milliseconds, PRUint32 *_retval) = 0;

  /* void unsuspendRedraw (in unsigned long suspend_handle_id); */
  NS_IMETHOD UnsuspendRedraw(PRUint32 suspend_handle_id) = 0;

  /* void unsuspendRedrawAll (); */
  NS_IMETHOD UnsuspendRedrawAll(void) = 0;

  /* void forceRedraw (); */
  NS_IMETHOD ForceRedraw(void) = 0;

  /* void pauseAnimations (); */
  NS_IMETHOD PauseAnimations(void) = 0;

  /* void unpauseAnimations (); */
  NS_IMETHOD UnpauseAnimations(void) = 0;

  /* boolean animationsPaused (); */
  NS_IMETHOD AnimationsPaused(PRBool *_retval) = 0;

  /* float getCurrentTime (); */
  NS_IMETHOD GetCurrentTime(float *_retval) = 0;

  /* void setCurrentTime (in float seconds); */
  NS_IMETHOD SetCurrentTime(float seconds) = 0;

  /* nsIDOMNodeList getIntersectionList (in nsIDOMSVGRect rect, in nsIDOMSVGElement referenceElement); */
  NS_IMETHOD GetIntersectionList(nsIDOMSVGRect *rect, nsIDOMSVGElement *referenceElement, nsIDOMNodeList **_retval) = 0;

  /* nsIDOMNodeList getEnclosureList (in nsIDOMSVGRect rect, in nsIDOMSVGElement referenceElement); */
  NS_IMETHOD GetEnclosureList(nsIDOMSVGRect *rect, nsIDOMSVGElement *referenceElement, nsIDOMNodeList **_retval) = 0;

  /* boolean checkIntersection (in nsIDOMSVGElement element, in nsIDOMSVGRect rect); */
  NS_IMETHOD CheckIntersection(nsIDOMSVGElement *element, nsIDOMSVGRect *rect, PRBool *_retval) = 0;

  /* boolean checkEnclosure (in nsIDOMSVGElement element, in nsIDOMSVGRect rect); */
  NS_IMETHOD CheckEnclosure(nsIDOMSVGElement *element, nsIDOMSVGRect *rect, PRBool *_retval) = 0;

  /* void deSelectAll (); */
  NS_IMETHOD DeSelectAll(void) = 0;

  /* nsIDOMSVGNumber createSVGNumber (); */
  NS_IMETHOD CreateSVGNumber(nsIDOMSVGNumber **_retval) = 0;

  /* nsIDOMSVGLength createSVGLength (); */
  NS_IMETHOD CreateSVGLength(nsIDOMSVGLength **_retval) = 0;

  /* nsIDOMSVGAngle createSVGAngle (); */
  NS_IMETHOD CreateSVGAngle(nsIDOMSVGAngle **_retval) = 0;

  /* nsIDOMSVGPoint createSVGPoint (); */
  NS_IMETHOD CreateSVGPoint(nsIDOMSVGPoint **_retval) = 0;

  /* nsIDOMSVGMatrix createSVGMatrix (); */
  NS_IMETHOD CreateSVGMatrix(nsIDOMSVGMatrix **_retval) = 0;

  /* nsIDOMSVGRect createSVGRect (); */
  NS_IMETHOD CreateSVGRect(nsIDOMSVGRect **_retval) = 0;

  /* nsIDOMSVGTransform createSVGTransform (); */
  NS_IMETHOD CreateSVGTransform(nsIDOMSVGTransform **_retval) = 0;

  /* nsIDOMSVGTransform createSVGTransformFromMatrix (in nsIDOMSVGMatrix matrix); */
  NS_IMETHOD CreateSVGTransformFromMatrix(nsIDOMSVGMatrix *matrix, nsIDOMSVGTransform **_retval) = 0;

  /* DOMString createSVGString (); */
  NS_IMETHOD CreateSVGString(nsAString & _retval) = 0;

  /* nsIDOMElement getElementById (in DOMString elementId); */
  NS_IMETHOD GetElementById(const nsAString & elementId, nsIDOMElement **_retval) = 0;

  /* nsIDOMSVGMatrix getViewboxToViewportTransform (); */
  NS_IMETHOD GetViewboxToViewportTransform(nsIDOMSVGMatrix **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGSVGELEMENT \
  NS_IMETHOD GetX(nsIDOMSVGAnimatedLength * *aX); \
  NS_IMETHOD GetY(nsIDOMSVGAnimatedLength * *aY); \
  NS_IMETHOD GetWidth(nsIDOMSVGAnimatedLength * *aWidth); \
  NS_IMETHOD GetHeight(nsIDOMSVGAnimatedLength * *aHeight); \
  NS_IMETHOD GetContentScriptType(nsAString & aContentScriptType); \
  NS_IMETHOD SetContentScriptType(const nsAString & aContentScriptType); \
  NS_IMETHOD GetContentStyleType(nsAString & aContentStyleType); \
  NS_IMETHOD SetContentStyleType(const nsAString & aContentStyleType); \
  NS_IMETHOD GetViewport(nsIDOMSVGRect * *aViewport); \
  NS_IMETHOD GetPixelUnitToMillimeterX(float *aPixelUnitToMillimeterX); \
  NS_IMETHOD GetPixelUnitToMillimeterY(float *aPixelUnitToMillimeterY); \
  NS_IMETHOD GetScreenPixelToMillimeterX(float *aScreenPixelToMillimeterX); \
  NS_IMETHOD GetScreenPixelToMillimeterY(float *aScreenPixelToMillimeterY); \
  NS_IMETHOD GetUseCurrentView(PRBool *aUseCurrentView); \
  NS_IMETHOD SetUseCurrentView(PRBool aUseCurrentView); \
  NS_IMETHOD GetCurrentView(nsIDOMSVGViewSpec * *aCurrentView); \
  NS_IMETHOD GetCurrentScale(float *aCurrentScale); \
  NS_IMETHOD SetCurrentScale(float aCurrentScale); \
  NS_IMETHOD GetCurrentTranslate(nsIDOMSVGPoint * *aCurrentTranslate); \
  NS_IMETHOD SuspendRedraw(PRUint32 max_wait_milliseconds, PRUint32 *_retval); \
  NS_IMETHOD UnsuspendRedraw(PRUint32 suspend_handle_id); \
  NS_IMETHOD UnsuspendRedrawAll(void); \
  NS_IMETHOD ForceRedraw(void); \
  NS_IMETHOD PauseAnimations(void); \
  NS_IMETHOD UnpauseAnimations(void); \
  NS_IMETHOD AnimationsPaused(PRBool *_retval); \
  NS_IMETHOD GetCurrentTime(float *_retval); \
  NS_IMETHOD SetCurrentTime(float seconds); \
  NS_IMETHOD GetIntersectionList(nsIDOMSVGRect *rect, nsIDOMSVGElement *referenceElement, nsIDOMNodeList **_retval); \
  NS_IMETHOD GetEnclosureList(nsIDOMSVGRect *rect, nsIDOMSVGElement *referenceElement, nsIDOMNodeList **_retval); \
  NS_IMETHOD CheckIntersection(nsIDOMSVGElement *element, nsIDOMSVGRect *rect, PRBool *_retval); \
  NS_IMETHOD CheckEnclosure(nsIDOMSVGElement *element, nsIDOMSVGRect *rect, PRBool *_retval); \
  NS_IMETHOD DeSelectAll(void); \
  NS_IMETHOD CreateSVGNumber(nsIDOMSVGNumber **_retval); \
  NS_IMETHOD CreateSVGLength(nsIDOMSVGLength **_retval); \
  NS_IMETHOD CreateSVGAngle(nsIDOMSVGAngle **_retval); \
  NS_IMETHOD CreateSVGPoint(nsIDOMSVGPoint **_retval); \
  NS_IMETHOD CreateSVGMatrix(nsIDOMSVGMatrix **_retval); \
  NS_IMETHOD CreateSVGRect(nsIDOMSVGRect **_retval); \
  NS_IMETHOD CreateSVGTransform(nsIDOMSVGTransform **_retval); \
  NS_IMETHOD CreateSVGTransformFromMatrix(nsIDOMSVGMatrix *matrix, nsIDOMSVGTransform **_retval); \
  NS_IMETHOD CreateSVGString(nsAString & _retval); \
  NS_IMETHOD GetElementById(const nsAString & elementId, nsIDOMElement **_retval); \
  NS_IMETHOD GetViewboxToViewportTransform(nsIDOMSVGMatrix **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGSVGELEMENT(_to) \
  NS_IMETHOD GetX(nsIDOMSVGAnimatedLength * *aX) { return _to GetX(aX); } \
  NS_IMETHOD GetY(nsIDOMSVGAnimatedLength * *aY) { return _to GetY(aY); } \
  NS_IMETHOD GetWidth(nsIDOMSVGAnimatedLength * *aWidth) { return _to GetWidth(aWidth); } \
  NS_IMETHOD GetHeight(nsIDOMSVGAnimatedLength * *aHeight) { return _to GetHeight(aHeight); } \
  NS_IMETHOD GetContentScriptType(nsAString & aContentScriptType) { return _to GetContentScriptType(aContentScriptType); } \
  NS_IMETHOD SetContentScriptType(const nsAString & aContentScriptType) { return _to SetContentScriptType(aContentScriptType); } \
  NS_IMETHOD GetContentStyleType(nsAString & aContentStyleType) { return _to GetContentStyleType(aContentStyleType); } \
  NS_IMETHOD SetContentStyleType(const nsAString & aContentStyleType) { return _to SetContentStyleType(aContentStyleType); } \
  NS_IMETHOD GetViewport(nsIDOMSVGRect * *aViewport) { return _to GetViewport(aViewport); } \
  NS_IMETHOD GetPixelUnitToMillimeterX(float *aPixelUnitToMillimeterX) { return _to GetPixelUnitToMillimeterX(aPixelUnitToMillimeterX); } \
  NS_IMETHOD GetPixelUnitToMillimeterY(float *aPixelUnitToMillimeterY) { return _to GetPixelUnitToMillimeterY(aPixelUnitToMillimeterY); } \
  NS_IMETHOD GetScreenPixelToMillimeterX(float *aScreenPixelToMillimeterX) { return _to GetScreenPixelToMillimeterX(aScreenPixelToMillimeterX); } \
  NS_IMETHOD GetScreenPixelToMillimeterY(float *aScreenPixelToMillimeterY) { return _to GetScreenPixelToMillimeterY(aScreenPixelToMillimeterY); } \
  NS_IMETHOD GetUseCurrentView(PRBool *aUseCurrentView) { return _to GetUseCurrentView(aUseCurrentView); } \
  NS_IMETHOD SetUseCurrentView(PRBool aUseCurrentView) { return _to SetUseCurrentView(aUseCurrentView); } \
  NS_IMETHOD GetCurrentView(nsIDOMSVGViewSpec * *aCurrentView) { return _to GetCurrentView(aCurrentView); } \
  NS_IMETHOD GetCurrentScale(float *aCurrentScale) { return _to GetCurrentScale(aCurrentScale); } \
  NS_IMETHOD SetCurrentScale(float aCurrentScale) { return _to SetCurrentScale(aCurrentScale); } \
  NS_IMETHOD GetCurrentTranslate(nsIDOMSVGPoint * *aCurrentTranslate) { return _to GetCurrentTranslate(aCurrentTranslate); } \
  NS_IMETHOD SuspendRedraw(PRUint32 max_wait_milliseconds, PRUint32 *_retval) { return _to SuspendRedraw(max_wait_milliseconds, _retval); } \
  NS_IMETHOD UnsuspendRedraw(PRUint32 suspend_handle_id) { return _to UnsuspendRedraw(suspend_handle_id); } \
  NS_IMETHOD UnsuspendRedrawAll(void) { return _to UnsuspendRedrawAll(); } \
  NS_IMETHOD ForceRedraw(void) { return _to ForceRedraw(); } \
  NS_IMETHOD PauseAnimations(void) { return _to PauseAnimations(); } \
  NS_IMETHOD UnpauseAnimations(void) { return _to UnpauseAnimations(); } \
  NS_IMETHOD AnimationsPaused(PRBool *_retval) { return _to AnimationsPaused(_retval); } \
  NS_IMETHOD GetCurrentTime(float *_retval) { return _to GetCurrentTime(_retval); } \
  NS_IMETHOD SetCurrentTime(float seconds) { return _to SetCurrentTime(seconds); } \
  NS_IMETHOD GetIntersectionList(nsIDOMSVGRect *rect, nsIDOMSVGElement *referenceElement, nsIDOMNodeList **_retval) { return _to GetIntersectionList(rect, referenceElement, _retval); } \
  NS_IMETHOD GetEnclosureList(nsIDOMSVGRect *rect, nsIDOMSVGElement *referenceElement, nsIDOMNodeList **_retval) { return _to GetEnclosureList(rect, referenceElement, _retval); } \
  NS_IMETHOD CheckIntersection(nsIDOMSVGElement *element, nsIDOMSVGRect *rect, PRBool *_retval) { return _to CheckIntersection(element, rect, _retval); } \
  NS_IMETHOD CheckEnclosure(nsIDOMSVGElement *element, nsIDOMSVGRect *rect, PRBool *_retval) { return _to CheckEnclosure(element, rect, _retval); } \
  NS_IMETHOD DeSelectAll(void) { return _to DeSelectAll(); } \
  NS_IMETHOD CreateSVGNumber(nsIDOMSVGNumber **_retval) { return _to CreateSVGNumber(_retval); } \
  NS_IMETHOD CreateSVGLength(nsIDOMSVGLength **_retval) { return _to CreateSVGLength(_retval); } \
  NS_IMETHOD CreateSVGAngle(nsIDOMSVGAngle **_retval) { return _to CreateSVGAngle(_retval); } \
  NS_IMETHOD CreateSVGPoint(nsIDOMSVGPoint **_retval) { return _to CreateSVGPoint(_retval); } \
  NS_IMETHOD CreateSVGMatrix(nsIDOMSVGMatrix **_retval) { return _to CreateSVGMatrix(_retval); } \
  NS_IMETHOD CreateSVGRect(nsIDOMSVGRect **_retval) { return _to CreateSVGRect(_retval); } \
  NS_IMETHOD CreateSVGTransform(nsIDOMSVGTransform **_retval) { return _to CreateSVGTransform(_retval); } \
  NS_IMETHOD CreateSVGTransformFromMatrix(nsIDOMSVGMatrix *matrix, nsIDOMSVGTransform **_retval) { return _to CreateSVGTransformFromMatrix(matrix, _retval); } \
  NS_IMETHOD CreateSVGString(nsAString & _retval) { return _to CreateSVGString(_retval); } \
  NS_IMETHOD GetElementById(const nsAString & elementId, nsIDOMElement **_retval) { return _to GetElementById(elementId, _retval); } \
  NS_IMETHOD GetViewboxToViewportTransform(nsIDOMSVGMatrix **_retval) { return _to GetViewboxToViewportTransform(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGSVGELEMENT(_to) \
  NS_IMETHOD GetX(nsIDOMSVGAnimatedLength * *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD GetY(nsIDOMSVGAnimatedLength * *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD GetWidth(nsIDOMSVGAnimatedLength * *aWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetWidth(aWidth); } \
  NS_IMETHOD GetHeight(nsIDOMSVGAnimatedLength * *aHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHeight(aHeight); } \
  NS_IMETHOD GetContentScriptType(nsAString & aContentScriptType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetContentScriptType(aContentScriptType); } \
  NS_IMETHOD SetContentScriptType(const nsAString & aContentScriptType) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetContentScriptType(aContentScriptType); } \
  NS_IMETHOD GetContentStyleType(nsAString & aContentStyleType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetContentStyleType(aContentStyleType); } \
  NS_IMETHOD SetContentStyleType(const nsAString & aContentStyleType) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetContentStyleType(aContentStyleType); } \
  NS_IMETHOD GetViewport(nsIDOMSVGRect * *aViewport) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetViewport(aViewport); } \
  NS_IMETHOD GetPixelUnitToMillimeterX(float *aPixelUnitToMillimeterX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPixelUnitToMillimeterX(aPixelUnitToMillimeterX); } \
  NS_IMETHOD GetPixelUnitToMillimeterY(float *aPixelUnitToMillimeterY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPixelUnitToMillimeterY(aPixelUnitToMillimeterY); } \
  NS_IMETHOD GetScreenPixelToMillimeterX(float *aScreenPixelToMillimeterX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScreenPixelToMillimeterX(aScreenPixelToMillimeterX); } \
  NS_IMETHOD GetScreenPixelToMillimeterY(float *aScreenPixelToMillimeterY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScreenPixelToMillimeterY(aScreenPixelToMillimeterY); } \
  NS_IMETHOD GetUseCurrentView(PRBool *aUseCurrentView) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetUseCurrentView(aUseCurrentView); } \
  NS_IMETHOD SetUseCurrentView(PRBool aUseCurrentView) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetUseCurrentView(aUseCurrentView); } \
  NS_IMETHOD GetCurrentView(nsIDOMSVGViewSpec * *aCurrentView) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCurrentView(aCurrentView); } \
  NS_IMETHOD GetCurrentScale(float *aCurrentScale) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCurrentScale(aCurrentScale); } \
  NS_IMETHOD SetCurrentScale(float aCurrentScale) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCurrentScale(aCurrentScale); } \
  NS_IMETHOD GetCurrentTranslate(nsIDOMSVGPoint * *aCurrentTranslate) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCurrentTranslate(aCurrentTranslate); } \
  NS_IMETHOD SuspendRedraw(PRUint32 max_wait_milliseconds, PRUint32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SuspendRedraw(max_wait_milliseconds, _retval); } \
  NS_IMETHOD UnsuspendRedraw(PRUint32 suspend_handle_id) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnsuspendRedraw(suspend_handle_id); } \
  NS_IMETHOD UnsuspendRedrawAll(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnsuspendRedrawAll(); } \
  NS_IMETHOD ForceRedraw(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ForceRedraw(); } \
  NS_IMETHOD PauseAnimations(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->PauseAnimations(); } \
  NS_IMETHOD UnpauseAnimations(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnpauseAnimations(); } \
  NS_IMETHOD AnimationsPaused(PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->AnimationsPaused(_retval); } \
  NS_IMETHOD GetCurrentTime(float *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCurrentTime(_retval); } \
  NS_IMETHOD SetCurrentTime(float seconds) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCurrentTime(seconds); } \
  NS_IMETHOD GetIntersectionList(nsIDOMSVGRect *rect, nsIDOMSVGElement *referenceElement, nsIDOMNodeList **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIntersectionList(rect, referenceElement, _retval); } \
  NS_IMETHOD GetEnclosureList(nsIDOMSVGRect *rect, nsIDOMSVGElement *referenceElement, nsIDOMNodeList **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetEnclosureList(rect, referenceElement, _retval); } \
  NS_IMETHOD CheckIntersection(nsIDOMSVGElement *element, nsIDOMSVGRect *rect, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CheckIntersection(element, rect, _retval); } \
  NS_IMETHOD CheckEnclosure(nsIDOMSVGElement *element, nsIDOMSVGRect *rect, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CheckEnclosure(element, rect, _retval); } \
  NS_IMETHOD DeSelectAll(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->DeSelectAll(); } \
  NS_IMETHOD CreateSVGNumber(nsIDOMSVGNumber **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGNumber(_retval); } \
  NS_IMETHOD CreateSVGLength(nsIDOMSVGLength **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGLength(_retval); } \
  NS_IMETHOD CreateSVGAngle(nsIDOMSVGAngle **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGAngle(_retval); } \
  NS_IMETHOD CreateSVGPoint(nsIDOMSVGPoint **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGPoint(_retval); } \
  NS_IMETHOD CreateSVGMatrix(nsIDOMSVGMatrix **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGMatrix(_retval); } \
  NS_IMETHOD CreateSVGRect(nsIDOMSVGRect **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGRect(_retval); } \
  NS_IMETHOD CreateSVGTransform(nsIDOMSVGTransform **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGTransform(_retval); } \
  NS_IMETHOD CreateSVGTransformFromMatrix(nsIDOMSVGMatrix *matrix, nsIDOMSVGTransform **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGTransformFromMatrix(matrix, _retval); } \
  NS_IMETHOD CreateSVGString(nsAString & _retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateSVGString(_retval); } \
  NS_IMETHOD GetElementById(const nsAString & elementId, nsIDOMElement **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetElementById(elementId, _retval); } \
  NS_IMETHOD GetViewboxToViewportTransform(nsIDOMSVGMatrix **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetViewboxToViewportTransform(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGSVGElement : public nsIDOMSVGSVGElement
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGSVGELEMENT

  nsDOMSVGSVGElement();

private:
  ~nsDOMSVGSVGElement();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGSVGElement, nsIDOMSVGSVGElement)

nsDOMSVGSVGElement::nsDOMSVGSVGElement()
{
  /* member initializers and constructor code */
}

nsDOMSVGSVGElement::~nsDOMSVGSVGElement()
{
  /* destructor code */
}

/* readonly attribute nsIDOMSVGAnimatedLength x; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetX(nsIDOMSVGAnimatedLength * *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMSVGAnimatedLength y; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetY(nsIDOMSVGAnimatedLength * *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMSVGAnimatedLength width; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetWidth(nsIDOMSVGAnimatedLength * *aWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMSVGAnimatedLength height; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetHeight(nsIDOMSVGAnimatedLength * *aHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString contentScriptType; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetContentScriptType(nsAString & aContentScriptType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGSVGElement::SetContentScriptType(const nsAString & aContentScriptType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString contentStyleType; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetContentStyleType(nsAString & aContentStyleType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGSVGElement::SetContentStyleType(const nsAString & aContentStyleType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMSVGRect viewport; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetViewport(nsIDOMSVGRect * *aViewport)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float pixelUnitToMillimeterX; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetPixelUnitToMillimeterX(float *aPixelUnitToMillimeterX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float pixelUnitToMillimeterY; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetPixelUnitToMillimeterY(float *aPixelUnitToMillimeterY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float screenPixelToMillimeterX; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetScreenPixelToMillimeterX(float *aScreenPixelToMillimeterX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float screenPixelToMillimeterY; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetScreenPixelToMillimeterY(float *aScreenPixelToMillimeterY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean useCurrentView; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetUseCurrentView(PRBool *aUseCurrentView)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGSVGElement::SetUseCurrentView(PRBool aUseCurrentView)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMSVGViewSpec currentView; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetCurrentView(nsIDOMSVGViewSpec * *aCurrentView)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float currentScale; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetCurrentScale(float *aCurrentScale)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGSVGElement::SetCurrentScale(float aCurrentScale)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMSVGPoint currentTranslate; */
NS_IMETHODIMP nsDOMSVGSVGElement::GetCurrentTranslate(nsIDOMSVGPoint * *aCurrentTranslate)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* unsigned long suspendRedraw (in unsigned long max_wait_milliseconds); */
NS_IMETHODIMP nsDOMSVGSVGElement::SuspendRedraw(PRUint32 max_wait_milliseconds, PRUint32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unsuspendRedraw (in unsigned long suspend_handle_id); */
NS_IMETHODIMP nsDOMSVGSVGElement::UnsuspendRedraw(PRUint32 suspend_handle_id)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unsuspendRedrawAll (); */
NS_IMETHODIMP nsDOMSVGSVGElement::UnsuspendRedrawAll()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void forceRedraw (); */
NS_IMETHODIMP nsDOMSVGSVGElement::ForceRedraw()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void pauseAnimations (); */
NS_IMETHODIMP nsDOMSVGSVGElement::PauseAnimations()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unpauseAnimations (); */
NS_IMETHODIMP nsDOMSVGSVGElement::UnpauseAnimations()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean animationsPaused (); */
NS_IMETHODIMP nsDOMSVGSVGElement::AnimationsPaused(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* float getCurrentTime (); */
NS_IMETHODIMP nsDOMSVGSVGElement::GetCurrentTime(float *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setCurrentTime (in float seconds); */
NS_IMETHODIMP nsDOMSVGSVGElement::SetCurrentTime(float seconds)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMNodeList getIntersectionList (in nsIDOMSVGRect rect, in nsIDOMSVGElement referenceElement); */
NS_IMETHODIMP nsDOMSVGSVGElement::GetIntersectionList(nsIDOMSVGRect *rect, nsIDOMSVGElement *referenceElement, nsIDOMNodeList **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMNodeList getEnclosureList (in nsIDOMSVGRect rect, in nsIDOMSVGElement referenceElement); */
NS_IMETHODIMP nsDOMSVGSVGElement::GetEnclosureList(nsIDOMSVGRect *rect, nsIDOMSVGElement *referenceElement, nsIDOMNodeList **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean checkIntersection (in nsIDOMSVGElement element, in nsIDOMSVGRect rect); */
NS_IMETHODIMP nsDOMSVGSVGElement::CheckIntersection(nsIDOMSVGElement *element, nsIDOMSVGRect *rect, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean checkEnclosure (in nsIDOMSVGElement element, in nsIDOMSVGRect rect); */
NS_IMETHODIMP nsDOMSVGSVGElement::CheckEnclosure(nsIDOMSVGElement *element, nsIDOMSVGRect *rect, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void deSelectAll (); */
NS_IMETHODIMP nsDOMSVGSVGElement::DeSelectAll()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGNumber createSVGNumber (); */
NS_IMETHODIMP nsDOMSVGSVGElement::CreateSVGNumber(nsIDOMSVGNumber **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGLength createSVGLength (); */
NS_IMETHODIMP nsDOMSVGSVGElement::CreateSVGLength(nsIDOMSVGLength **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGAngle createSVGAngle (); */
NS_IMETHODIMP nsDOMSVGSVGElement::CreateSVGAngle(nsIDOMSVGAngle **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGPoint createSVGPoint (); */
NS_IMETHODIMP nsDOMSVGSVGElement::CreateSVGPoint(nsIDOMSVGPoint **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGMatrix createSVGMatrix (); */
NS_IMETHODIMP nsDOMSVGSVGElement::CreateSVGMatrix(nsIDOMSVGMatrix **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGRect createSVGRect (); */
NS_IMETHODIMP nsDOMSVGSVGElement::CreateSVGRect(nsIDOMSVGRect **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGTransform createSVGTransform (); */
NS_IMETHODIMP nsDOMSVGSVGElement::CreateSVGTransform(nsIDOMSVGTransform **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGTransform createSVGTransformFromMatrix (in nsIDOMSVGMatrix matrix); */
NS_IMETHODIMP nsDOMSVGSVGElement::CreateSVGTransformFromMatrix(nsIDOMSVGMatrix *matrix, nsIDOMSVGTransform **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* DOMString createSVGString (); */
NS_IMETHODIMP nsDOMSVGSVGElement::CreateSVGString(nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMElement getElementById (in DOMString elementId); */
NS_IMETHODIMP nsDOMSVGSVGElement::GetElementById(const nsAString & elementId, nsIDOMElement **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMSVGMatrix getViewboxToViewportTransform (); */
NS_IMETHODIMP nsDOMSVGSVGElement::GetViewboxToViewportTransform(nsIDOMSVGMatrix **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMSVGSVGElement_h__ */
