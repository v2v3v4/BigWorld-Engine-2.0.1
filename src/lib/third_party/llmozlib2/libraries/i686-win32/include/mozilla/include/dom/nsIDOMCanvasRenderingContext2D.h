/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/canvas/nsIDOMCanvasRenderingContext2D.idl
 */

#ifndef __gen_nsIDOMCanvasRenderingContext2D_h__
#define __gen_nsIDOMCanvasRenderingContext2D_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsIVariant_h__
#include "nsIVariant.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMWindow; /* forward declaration */

class nsIDOMHTMLElement; /* forward declaration */

class nsIDOMHTMLImageElement; /* forward declaration */

class nsIDOMHTMLCanvasElement; /* forward declaration */


/* starting interface:    nsIDOMCanvasGradient */
#define NS_IDOMCANVASGRADIENT_IID_STR "bbb20a59-524e-4662-981e-5e142814b20c"

#define NS_IDOMCANVASGRADIENT_IID \
  {0xbbb20a59, 0x524e, 0x4662, \
    { 0x98, 0x1e, 0x5e, 0x14, 0x28, 0x14, 0xb2, 0x0c }}

class NS_NO_VTABLE nsIDOMCanvasGradient : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMCANVASGRADIENT_IID)

  /* void addColorStop (in float offset, in DOMString color); */
  NS_IMETHOD AddColorStop(float offset, const nsAString & color) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMCANVASGRADIENT \
  NS_IMETHOD AddColorStop(float offset, const nsAString & color); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMCANVASGRADIENT(_to) \
  NS_IMETHOD AddColorStop(float offset, const nsAString & color) { return _to AddColorStop(offset, color); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMCANVASGRADIENT(_to) \
  NS_IMETHOD AddColorStop(float offset, const nsAString & color) { return !_to ? NS_ERROR_NULL_POINTER : _to->AddColorStop(offset, color); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMCanvasGradient : public nsIDOMCanvasGradient
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMCANVASGRADIENT

  nsDOMCanvasGradient();

private:
  ~nsDOMCanvasGradient();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMCanvasGradient, nsIDOMCanvasGradient)

nsDOMCanvasGradient::nsDOMCanvasGradient()
{
  /* member initializers and constructor code */
}

nsDOMCanvasGradient::~nsDOMCanvasGradient()
{
  /* destructor code */
}

/* void addColorStop (in float offset, in DOMString color); */
NS_IMETHODIMP nsDOMCanvasGradient::AddColorStop(float offset, const nsAString & color)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMCanvasPattern */
#define NS_IDOMCANVASPATTERN_IID_STR "21dea65c-5c08-4eb1-ac82-81fe95be77b8"

#define NS_IDOMCANVASPATTERN_IID \
  {0x21dea65c, 0x5c08, 0x4eb1, \
    { 0xac, 0x82, 0x81, 0xfe, 0x95, 0xbe, 0x77, 0xb8 }}

class NS_NO_VTABLE nsIDOMCanvasPattern : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMCANVASPATTERN_IID)

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMCANVASPATTERN \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMCANVASPATTERN(_to) \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMCANVASPATTERN(_to) \
  /* no methods! */

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMCanvasPattern : public nsIDOMCanvasPattern
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMCANVASPATTERN

  nsDOMCanvasPattern();

private:
  ~nsDOMCanvasPattern();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMCanvasPattern, nsIDOMCanvasPattern)

nsDOMCanvasPattern::nsDOMCanvasPattern()
{
  /* member initializers and constructor code */
}

nsDOMCanvasPattern::~nsDOMCanvasPattern()
{
  /* destructor code */
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMCanvasRenderingContext2D */
#define NS_IDOMCANVASRENDERINGCONTEXT2D_IID_STR "ab27f42d-e1e1-4ef6-9c83-059a81da479b"

#define NS_IDOMCANVASRENDERINGCONTEXT2D_IID \
  {0xab27f42d, 0xe1e1, 0x4ef6, \
    { 0x9c, 0x83, 0x05, 0x9a, 0x81, 0xda, 0x47, 0x9b }}

class NS_NO_VTABLE nsIDOMCanvasRenderingContext2D : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMCANVASRENDERINGCONTEXT2D_IID)

  /* readonly attribute nsIDOMHTMLCanvasElement canvas; */
  NS_IMETHOD GetCanvas(nsIDOMHTMLCanvasElement * *aCanvas) = 0;

  /* void save (); */
  NS_IMETHOD Save(void) = 0;

  /* void restore (); */
  NS_IMETHOD Restore(void) = 0;

  /* void scale (in float x, in float y); */
  NS_IMETHOD Scale(float x, float y) = 0;

  /* void rotate (in float angle); */
  NS_IMETHOD Rotate(float angle) = 0;

  /* void translate (in float x, in float y); */
  NS_IMETHOD Translate(float x, float y) = 0;

  /* attribute float globalAlpha; */
  NS_IMETHOD GetGlobalAlpha(float *aGlobalAlpha) = 0;
  NS_IMETHOD SetGlobalAlpha(float aGlobalAlpha) = 0;

  /* attribute DOMString globalCompositeOperation; */
  NS_IMETHOD GetGlobalCompositeOperation(nsAString & aGlobalCompositeOperation) = 0;
  NS_IMETHOD SetGlobalCompositeOperation(const nsAString & aGlobalCompositeOperation) = 0;

  /* attribute nsIVariant strokeStyle; */
  NS_IMETHOD GetStrokeStyle(nsIVariant * *aStrokeStyle) = 0;
  NS_IMETHOD SetStrokeStyle(nsIVariant * aStrokeStyle) = 0;

  /* attribute nsIVariant fillStyle; */
  NS_IMETHOD GetFillStyle(nsIVariant * *aFillStyle) = 0;
  NS_IMETHOD SetFillStyle(nsIVariant * aFillStyle) = 0;

  /* nsIDOMCanvasGradient createLinearGradient (in float x0, in float y0, in float x1, in float y1); */
  NS_IMETHOD CreateLinearGradient(float x0, float y0, float x1, float y1, nsIDOMCanvasGradient **_retval) = 0;

  /* nsIDOMCanvasGradient createRadialGradient (in float x0, in float y0, in float r0, in float x1, in float y1, in float r1); */
  NS_IMETHOD CreateRadialGradient(float x0, float y0, float r0, float x1, float y1, float r1, nsIDOMCanvasGradient **_retval) = 0;

  /* nsIDOMCanvasPattern createPattern (in nsIDOMHTMLElement image, in DOMString repetition); */
  NS_IMETHOD CreatePattern(nsIDOMHTMLElement *image, const nsAString & repetition, nsIDOMCanvasPattern **_retval) = 0;

  /* attribute float lineWidth; */
  NS_IMETHOD GetLineWidth(float *aLineWidth) = 0;
  NS_IMETHOD SetLineWidth(float aLineWidth) = 0;

  /* attribute DOMString lineCap; */
  NS_IMETHOD GetLineCap(nsAString & aLineCap) = 0;
  NS_IMETHOD SetLineCap(const nsAString & aLineCap) = 0;

  /* attribute DOMString lineJoin; */
  NS_IMETHOD GetLineJoin(nsAString & aLineJoin) = 0;
  NS_IMETHOD SetLineJoin(const nsAString & aLineJoin) = 0;

  /* attribute float miterLimit; */
  NS_IMETHOD GetMiterLimit(float *aMiterLimit) = 0;
  NS_IMETHOD SetMiterLimit(float aMiterLimit) = 0;

  /* attribute float shadowOffsetX; */
  NS_IMETHOD GetShadowOffsetX(float *aShadowOffsetX) = 0;
  NS_IMETHOD SetShadowOffsetX(float aShadowOffsetX) = 0;

  /* attribute float shadowOffsetY; */
  NS_IMETHOD GetShadowOffsetY(float *aShadowOffsetY) = 0;
  NS_IMETHOD SetShadowOffsetY(float aShadowOffsetY) = 0;

  /* attribute float shadowBlur; */
  NS_IMETHOD GetShadowBlur(float *aShadowBlur) = 0;
  NS_IMETHOD SetShadowBlur(float aShadowBlur) = 0;

  /* attribute DOMString shadowColor; */
  NS_IMETHOD GetShadowColor(nsAString & aShadowColor) = 0;
  NS_IMETHOD SetShadowColor(const nsAString & aShadowColor) = 0;

  /* void clearRect (in float x, in float y, in float w, in float h); */
  NS_IMETHOD ClearRect(float x, float y, float w, float h) = 0;

  /* void fillRect (in float x, in float y, in float w, in float h); */
  NS_IMETHOD FillRect(float x, float y, float w, float h) = 0;

  /* void strokeRect (in float x, in float y, in float w, in float h); */
  NS_IMETHOD StrokeRect(float x, float y, float w, float h) = 0;

  /* void beginPath (); */
  NS_IMETHOD BeginPath(void) = 0;

  /* void closePath (); */
  NS_IMETHOD ClosePath(void) = 0;

  /* void moveTo (in float x, in float y); */
  NS_IMETHOD MoveTo(float x, float y) = 0;

  /* void lineTo (in float x, in float y); */
  NS_IMETHOD LineTo(float x, float y) = 0;

  /* void quadraticCurveTo (in float cpx, in float cpy, in float x, in float y); */
  NS_IMETHOD QuadraticCurveTo(float cpx, float cpy, float x, float y) = 0;

  /* void bezierCurveTo (in float cp1x, in float cp1y, in float cp2x, in float cp2y, in float x, in float y); */
  NS_IMETHOD BezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y) = 0;

  /* void arcTo (in float x1, in float y1, in float x2, in float y2, in float radius); */
  NS_IMETHOD ArcTo(float x1, float y1, float x2, float y2, float radius) = 0;

  /* void arc (in float x, in float y, in float r, in float startAngle, in float endAngle, in boolean clockwise); */
  NS_IMETHOD Arc(float x, float y, float r, float startAngle, float endAngle, PRBool clockwise) = 0;

  /* void rect (in float x, in float y, in float w, in float h); */
  NS_IMETHOD Rect(float x, float y, float w, float h) = 0;

  /* void fill (); */
  NS_IMETHOD Fill(void) = 0;

  /* void stroke (); */
  NS_IMETHOD Stroke(void) = 0;

  /* void clip (); */
  NS_IMETHOD Clip(void) = 0;

  /* void drawImage (); */
  NS_IMETHOD DrawImage(void) = 0;

  /* boolean isPointInPath (in float x, in float y); */
  NS_IMETHOD IsPointInPath(float x, float y, PRBool *_retval) = 0;

  /* void getImageData (); */
  NS_IMETHOD GetImageData(void) = 0;

  /* void putImageData (); */
  NS_IMETHOD PutImageData(void) = 0;

  /**
   * Renders a region of a window into the canvas.  The contents of
   * the window's viewport are rendered, ignoring viewport clipping
   * and scrolling.
   *
   * @param x
   * @param y
   * @param w
   * @param h specify the area of the window to render, in CSS
   * pixels.
   *
   * @param backgroundColor the canvas is filled with this color
   * before we render the window into it. This color may be
   * transparent/translucent. It is given as a CSS color string
   * (e.g., rgb() or rgba()).
   *
   * Of course, the rendering obeys the current scale, transform and
   * globalAlpha values.
   *
   * Hints:
   * -- If 'rgba(0,0,0,0)' is used for the background color, the
   * drawing will be transparent wherever the window is transparent.
   * -- Top-level browsed documents are usually not transparent
   * because the user's background-color preference is applied,
   * but IFRAMEs are transparent if the page doesn't set a background.
   * -- If an opaque color is used for the background color, rendering
   * will be faster because we won't have to compute the window's
   * transparency.
   *
   * This API cannot currently be used by Web content. It is chrome
   * only.
   */
  /* void drawWindow (in nsIDOMWindow window, in long x, in long y, in long w, in long h, in DOMString bgColor); */
  NS_IMETHOD DrawWindow(nsIDOMWindow *window, PRInt32 x, PRInt32 y, PRInt32 w, PRInt32 h, const nsAString & bgColor) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMCANVASRENDERINGCONTEXT2D \
  NS_IMETHOD GetCanvas(nsIDOMHTMLCanvasElement * *aCanvas); \
  NS_IMETHOD Save(void); \
  NS_IMETHOD Restore(void); \
  NS_IMETHOD Scale(float x, float y); \
  NS_IMETHOD Rotate(float angle); \
  NS_IMETHOD Translate(float x, float y); \
  NS_IMETHOD GetGlobalAlpha(float *aGlobalAlpha); \
  NS_IMETHOD SetGlobalAlpha(float aGlobalAlpha); \
  NS_IMETHOD GetGlobalCompositeOperation(nsAString & aGlobalCompositeOperation); \
  NS_IMETHOD SetGlobalCompositeOperation(const nsAString & aGlobalCompositeOperation); \
  NS_IMETHOD GetStrokeStyle(nsIVariant * *aStrokeStyle); \
  NS_IMETHOD SetStrokeStyle(nsIVariant * aStrokeStyle); \
  NS_IMETHOD GetFillStyle(nsIVariant * *aFillStyle); \
  NS_IMETHOD SetFillStyle(nsIVariant * aFillStyle); \
  NS_IMETHOD CreateLinearGradient(float x0, float y0, float x1, float y1, nsIDOMCanvasGradient **_retval); \
  NS_IMETHOD CreateRadialGradient(float x0, float y0, float r0, float x1, float y1, float r1, nsIDOMCanvasGradient **_retval); \
  NS_IMETHOD CreatePattern(nsIDOMHTMLElement *image, const nsAString & repetition, nsIDOMCanvasPattern **_retval); \
  NS_IMETHOD GetLineWidth(float *aLineWidth); \
  NS_IMETHOD SetLineWidth(float aLineWidth); \
  NS_IMETHOD GetLineCap(nsAString & aLineCap); \
  NS_IMETHOD SetLineCap(const nsAString & aLineCap); \
  NS_IMETHOD GetLineJoin(nsAString & aLineJoin); \
  NS_IMETHOD SetLineJoin(const nsAString & aLineJoin); \
  NS_IMETHOD GetMiterLimit(float *aMiterLimit); \
  NS_IMETHOD SetMiterLimit(float aMiterLimit); \
  NS_IMETHOD GetShadowOffsetX(float *aShadowOffsetX); \
  NS_IMETHOD SetShadowOffsetX(float aShadowOffsetX); \
  NS_IMETHOD GetShadowOffsetY(float *aShadowOffsetY); \
  NS_IMETHOD SetShadowOffsetY(float aShadowOffsetY); \
  NS_IMETHOD GetShadowBlur(float *aShadowBlur); \
  NS_IMETHOD SetShadowBlur(float aShadowBlur); \
  NS_IMETHOD GetShadowColor(nsAString & aShadowColor); \
  NS_IMETHOD SetShadowColor(const nsAString & aShadowColor); \
  NS_IMETHOD ClearRect(float x, float y, float w, float h); \
  NS_IMETHOD FillRect(float x, float y, float w, float h); \
  NS_IMETHOD StrokeRect(float x, float y, float w, float h); \
  NS_IMETHOD BeginPath(void); \
  NS_IMETHOD ClosePath(void); \
  NS_IMETHOD MoveTo(float x, float y); \
  NS_IMETHOD LineTo(float x, float y); \
  NS_IMETHOD QuadraticCurveTo(float cpx, float cpy, float x, float y); \
  NS_IMETHOD BezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y); \
  NS_IMETHOD ArcTo(float x1, float y1, float x2, float y2, float radius); \
  NS_IMETHOD Arc(float x, float y, float r, float startAngle, float endAngle, PRBool clockwise); \
  NS_IMETHOD Rect(float x, float y, float w, float h); \
  NS_IMETHOD Fill(void); \
  NS_IMETHOD Stroke(void); \
  NS_IMETHOD Clip(void); \
  NS_IMETHOD DrawImage(void); \
  NS_IMETHOD IsPointInPath(float x, float y, PRBool *_retval); \
  NS_IMETHOD GetImageData(void); \
  NS_IMETHOD PutImageData(void); \
  NS_IMETHOD DrawWindow(nsIDOMWindow *window, PRInt32 x, PRInt32 y, PRInt32 w, PRInt32 h, const nsAString & bgColor); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMCANVASRENDERINGCONTEXT2D(_to) \
  NS_IMETHOD GetCanvas(nsIDOMHTMLCanvasElement * *aCanvas) { return _to GetCanvas(aCanvas); } \
  NS_IMETHOD Save(void) { return _to Save(); } \
  NS_IMETHOD Restore(void) { return _to Restore(); } \
  NS_IMETHOD Scale(float x, float y) { return _to Scale(x, y); } \
  NS_IMETHOD Rotate(float angle) { return _to Rotate(angle); } \
  NS_IMETHOD Translate(float x, float y) { return _to Translate(x, y); } \
  NS_IMETHOD GetGlobalAlpha(float *aGlobalAlpha) { return _to GetGlobalAlpha(aGlobalAlpha); } \
  NS_IMETHOD SetGlobalAlpha(float aGlobalAlpha) { return _to SetGlobalAlpha(aGlobalAlpha); } \
  NS_IMETHOD GetGlobalCompositeOperation(nsAString & aGlobalCompositeOperation) { return _to GetGlobalCompositeOperation(aGlobalCompositeOperation); } \
  NS_IMETHOD SetGlobalCompositeOperation(const nsAString & aGlobalCompositeOperation) { return _to SetGlobalCompositeOperation(aGlobalCompositeOperation); } \
  NS_IMETHOD GetStrokeStyle(nsIVariant * *aStrokeStyle) { return _to GetStrokeStyle(aStrokeStyle); } \
  NS_IMETHOD SetStrokeStyle(nsIVariant * aStrokeStyle) { return _to SetStrokeStyle(aStrokeStyle); } \
  NS_IMETHOD GetFillStyle(nsIVariant * *aFillStyle) { return _to GetFillStyle(aFillStyle); } \
  NS_IMETHOD SetFillStyle(nsIVariant * aFillStyle) { return _to SetFillStyle(aFillStyle); } \
  NS_IMETHOD CreateLinearGradient(float x0, float y0, float x1, float y1, nsIDOMCanvasGradient **_retval) { return _to CreateLinearGradient(x0, y0, x1, y1, _retval); } \
  NS_IMETHOD CreateRadialGradient(float x0, float y0, float r0, float x1, float y1, float r1, nsIDOMCanvasGradient **_retval) { return _to CreateRadialGradient(x0, y0, r0, x1, y1, r1, _retval); } \
  NS_IMETHOD CreatePattern(nsIDOMHTMLElement *image, const nsAString & repetition, nsIDOMCanvasPattern **_retval) { return _to CreatePattern(image, repetition, _retval); } \
  NS_IMETHOD GetLineWidth(float *aLineWidth) { return _to GetLineWidth(aLineWidth); } \
  NS_IMETHOD SetLineWidth(float aLineWidth) { return _to SetLineWidth(aLineWidth); } \
  NS_IMETHOD GetLineCap(nsAString & aLineCap) { return _to GetLineCap(aLineCap); } \
  NS_IMETHOD SetLineCap(const nsAString & aLineCap) { return _to SetLineCap(aLineCap); } \
  NS_IMETHOD GetLineJoin(nsAString & aLineJoin) { return _to GetLineJoin(aLineJoin); } \
  NS_IMETHOD SetLineJoin(const nsAString & aLineJoin) { return _to SetLineJoin(aLineJoin); } \
  NS_IMETHOD GetMiterLimit(float *aMiterLimit) { return _to GetMiterLimit(aMiterLimit); } \
  NS_IMETHOD SetMiterLimit(float aMiterLimit) { return _to SetMiterLimit(aMiterLimit); } \
  NS_IMETHOD GetShadowOffsetX(float *aShadowOffsetX) { return _to GetShadowOffsetX(aShadowOffsetX); } \
  NS_IMETHOD SetShadowOffsetX(float aShadowOffsetX) { return _to SetShadowOffsetX(aShadowOffsetX); } \
  NS_IMETHOD GetShadowOffsetY(float *aShadowOffsetY) { return _to GetShadowOffsetY(aShadowOffsetY); } \
  NS_IMETHOD SetShadowOffsetY(float aShadowOffsetY) { return _to SetShadowOffsetY(aShadowOffsetY); } \
  NS_IMETHOD GetShadowBlur(float *aShadowBlur) { return _to GetShadowBlur(aShadowBlur); } \
  NS_IMETHOD SetShadowBlur(float aShadowBlur) { return _to SetShadowBlur(aShadowBlur); } \
  NS_IMETHOD GetShadowColor(nsAString & aShadowColor) { return _to GetShadowColor(aShadowColor); } \
  NS_IMETHOD SetShadowColor(const nsAString & aShadowColor) { return _to SetShadowColor(aShadowColor); } \
  NS_IMETHOD ClearRect(float x, float y, float w, float h) { return _to ClearRect(x, y, w, h); } \
  NS_IMETHOD FillRect(float x, float y, float w, float h) { return _to FillRect(x, y, w, h); } \
  NS_IMETHOD StrokeRect(float x, float y, float w, float h) { return _to StrokeRect(x, y, w, h); } \
  NS_IMETHOD BeginPath(void) { return _to BeginPath(); } \
  NS_IMETHOD ClosePath(void) { return _to ClosePath(); } \
  NS_IMETHOD MoveTo(float x, float y) { return _to MoveTo(x, y); } \
  NS_IMETHOD LineTo(float x, float y) { return _to LineTo(x, y); } \
  NS_IMETHOD QuadraticCurveTo(float cpx, float cpy, float x, float y) { return _to QuadraticCurveTo(cpx, cpy, x, y); } \
  NS_IMETHOD BezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y) { return _to BezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y); } \
  NS_IMETHOD ArcTo(float x1, float y1, float x2, float y2, float radius) { return _to ArcTo(x1, y1, x2, y2, radius); } \
  NS_IMETHOD Arc(float x, float y, float r, float startAngle, float endAngle, PRBool clockwise) { return _to Arc(x, y, r, startAngle, endAngle, clockwise); } \
  NS_IMETHOD Rect(float x, float y, float w, float h) { return _to Rect(x, y, w, h); } \
  NS_IMETHOD Fill(void) { return _to Fill(); } \
  NS_IMETHOD Stroke(void) { return _to Stroke(); } \
  NS_IMETHOD Clip(void) { return _to Clip(); } \
  NS_IMETHOD DrawImage(void) { return _to DrawImage(); } \
  NS_IMETHOD IsPointInPath(float x, float y, PRBool *_retval) { return _to IsPointInPath(x, y, _retval); } \
  NS_IMETHOD GetImageData(void) { return _to GetImageData(); } \
  NS_IMETHOD PutImageData(void) { return _to PutImageData(); } \
  NS_IMETHOD DrawWindow(nsIDOMWindow *window, PRInt32 x, PRInt32 y, PRInt32 w, PRInt32 h, const nsAString & bgColor) { return _to DrawWindow(window, x, y, w, h, bgColor); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMCANVASRENDERINGCONTEXT2D(_to) \
  NS_IMETHOD GetCanvas(nsIDOMHTMLCanvasElement * *aCanvas) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCanvas(aCanvas); } \
  NS_IMETHOD Save(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Save(); } \
  NS_IMETHOD Restore(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Restore(); } \
  NS_IMETHOD Scale(float x, float y) { return !_to ? NS_ERROR_NULL_POINTER : _to->Scale(x, y); } \
  NS_IMETHOD Rotate(float angle) { return !_to ? NS_ERROR_NULL_POINTER : _to->Rotate(angle); } \
  NS_IMETHOD Translate(float x, float y) { return !_to ? NS_ERROR_NULL_POINTER : _to->Translate(x, y); } \
  NS_IMETHOD GetGlobalAlpha(float *aGlobalAlpha) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetGlobalAlpha(aGlobalAlpha); } \
  NS_IMETHOD SetGlobalAlpha(float aGlobalAlpha) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetGlobalAlpha(aGlobalAlpha); } \
  NS_IMETHOD GetGlobalCompositeOperation(nsAString & aGlobalCompositeOperation) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetGlobalCompositeOperation(aGlobalCompositeOperation); } \
  NS_IMETHOD SetGlobalCompositeOperation(const nsAString & aGlobalCompositeOperation) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetGlobalCompositeOperation(aGlobalCompositeOperation); } \
  NS_IMETHOD GetStrokeStyle(nsIVariant * *aStrokeStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStrokeStyle(aStrokeStyle); } \
  NS_IMETHOD SetStrokeStyle(nsIVariant * aStrokeStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetStrokeStyle(aStrokeStyle); } \
  NS_IMETHOD GetFillStyle(nsIVariant * *aFillStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFillStyle(aFillStyle); } \
  NS_IMETHOD SetFillStyle(nsIVariant * aFillStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFillStyle(aFillStyle); } \
  NS_IMETHOD CreateLinearGradient(float x0, float y0, float x1, float y1, nsIDOMCanvasGradient **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateLinearGradient(x0, y0, x1, y1, _retval); } \
  NS_IMETHOD CreateRadialGradient(float x0, float y0, float r0, float x1, float y1, float r1, nsIDOMCanvasGradient **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateRadialGradient(x0, y0, r0, x1, y1, r1, _retval); } \
  NS_IMETHOD CreatePattern(nsIDOMHTMLElement *image, const nsAString & repetition, nsIDOMCanvasPattern **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreatePattern(image, repetition, _retval); } \
  NS_IMETHOD GetLineWidth(float *aLineWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLineWidth(aLineWidth); } \
  NS_IMETHOD SetLineWidth(float aLineWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetLineWidth(aLineWidth); } \
  NS_IMETHOD GetLineCap(nsAString & aLineCap) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLineCap(aLineCap); } \
  NS_IMETHOD SetLineCap(const nsAString & aLineCap) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetLineCap(aLineCap); } \
  NS_IMETHOD GetLineJoin(nsAString & aLineJoin) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLineJoin(aLineJoin); } \
  NS_IMETHOD SetLineJoin(const nsAString & aLineJoin) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetLineJoin(aLineJoin); } \
  NS_IMETHOD GetMiterLimit(float *aMiterLimit) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMiterLimit(aMiterLimit); } \
  NS_IMETHOD SetMiterLimit(float aMiterLimit) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMiterLimit(aMiterLimit); } \
  NS_IMETHOD GetShadowOffsetX(float *aShadowOffsetX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetShadowOffsetX(aShadowOffsetX); } \
  NS_IMETHOD SetShadowOffsetX(float aShadowOffsetX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetShadowOffsetX(aShadowOffsetX); } \
  NS_IMETHOD GetShadowOffsetY(float *aShadowOffsetY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetShadowOffsetY(aShadowOffsetY); } \
  NS_IMETHOD SetShadowOffsetY(float aShadowOffsetY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetShadowOffsetY(aShadowOffsetY); } \
  NS_IMETHOD GetShadowBlur(float *aShadowBlur) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetShadowBlur(aShadowBlur); } \
  NS_IMETHOD SetShadowBlur(float aShadowBlur) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetShadowBlur(aShadowBlur); } \
  NS_IMETHOD GetShadowColor(nsAString & aShadowColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetShadowColor(aShadowColor); } \
  NS_IMETHOD SetShadowColor(const nsAString & aShadowColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetShadowColor(aShadowColor); } \
  NS_IMETHOD ClearRect(float x, float y, float w, float h) { return !_to ? NS_ERROR_NULL_POINTER : _to->ClearRect(x, y, w, h); } \
  NS_IMETHOD FillRect(float x, float y, float w, float h) { return !_to ? NS_ERROR_NULL_POINTER : _to->FillRect(x, y, w, h); } \
  NS_IMETHOD StrokeRect(float x, float y, float w, float h) { return !_to ? NS_ERROR_NULL_POINTER : _to->StrokeRect(x, y, w, h); } \
  NS_IMETHOD BeginPath(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->BeginPath(); } \
  NS_IMETHOD ClosePath(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ClosePath(); } \
  NS_IMETHOD MoveTo(float x, float y) { return !_to ? NS_ERROR_NULL_POINTER : _to->MoveTo(x, y); } \
  NS_IMETHOD LineTo(float x, float y) { return !_to ? NS_ERROR_NULL_POINTER : _to->LineTo(x, y); } \
  NS_IMETHOD QuadraticCurveTo(float cpx, float cpy, float x, float y) { return !_to ? NS_ERROR_NULL_POINTER : _to->QuadraticCurveTo(cpx, cpy, x, y); } \
  NS_IMETHOD BezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y) { return !_to ? NS_ERROR_NULL_POINTER : _to->BezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y); } \
  NS_IMETHOD ArcTo(float x1, float y1, float x2, float y2, float radius) { return !_to ? NS_ERROR_NULL_POINTER : _to->ArcTo(x1, y1, x2, y2, radius); } \
  NS_IMETHOD Arc(float x, float y, float r, float startAngle, float endAngle, PRBool clockwise) { return !_to ? NS_ERROR_NULL_POINTER : _to->Arc(x, y, r, startAngle, endAngle, clockwise); } \
  NS_IMETHOD Rect(float x, float y, float w, float h) { return !_to ? NS_ERROR_NULL_POINTER : _to->Rect(x, y, w, h); } \
  NS_IMETHOD Fill(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Fill(); } \
  NS_IMETHOD Stroke(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Stroke(); } \
  NS_IMETHOD Clip(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Clip(); } \
  NS_IMETHOD DrawImage(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->DrawImage(); } \
  NS_IMETHOD IsPointInPath(float x, float y, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->IsPointInPath(x, y, _retval); } \
  NS_IMETHOD GetImageData(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetImageData(); } \
  NS_IMETHOD PutImageData(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->PutImageData(); } \
  NS_IMETHOD DrawWindow(nsIDOMWindow *window, PRInt32 x, PRInt32 y, PRInt32 w, PRInt32 h, const nsAString & bgColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->DrawWindow(window, x, y, w, h, bgColor); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMCanvasRenderingContext2D : public nsIDOMCanvasRenderingContext2D
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMCANVASRENDERINGCONTEXT2D

  nsDOMCanvasRenderingContext2D();

private:
  ~nsDOMCanvasRenderingContext2D();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMCanvasRenderingContext2D, nsIDOMCanvasRenderingContext2D)

nsDOMCanvasRenderingContext2D::nsDOMCanvasRenderingContext2D()
{
  /* member initializers and constructor code */
}

nsDOMCanvasRenderingContext2D::~nsDOMCanvasRenderingContext2D()
{
  /* destructor code */
}

/* readonly attribute nsIDOMHTMLCanvasElement canvas; */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::GetCanvas(nsIDOMHTMLCanvasElement * *aCanvas)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void save (); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::Save()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void restore (); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::Restore()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void scale (in float x, in float y); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::Scale(float x, float y)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void rotate (in float angle); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::Rotate(float angle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void translate (in float x, in float y); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::Translate(float x, float y)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float globalAlpha; */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::GetGlobalAlpha(float *aGlobalAlpha)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::SetGlobalAlpha(float aGlobalAlpha)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString globalCompositeOperation; */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::GetGlobalCompositeOperation(nsAString & aGlobalCompositeOperation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::SetGlobalCompositeOperation(const nsAString & aGlobalCompositeOperation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIVariant strokeStyle; */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::GetStrokeStyle(nsIVariant * *aStrokeStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::SetStrokeStyle(nsIVariant * aStrokeStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIVariant fillStyle; */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::GetFillStyle(nsIVariant * *aFillStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::SetFillStyle(nsIVariant * aFillStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMCanvasGradient createLinearGradient (in float x0, in float y0, in float x1, in float y1); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::CreateLinearGradient(float x0, float y0, float x1, float y1, nsIDOMCanvasGradient **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMCanvasGradient createRadialGradient (in float x0, in float y0, in float r0, in float x1, in float y1, in float r1); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::CreateRadialGradient(float x0, float y0, float r0, float x1, float y1, float r1, nsIDOMCanvasGradient **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMCanvasPattern createPattern (in nsIDOMHTMLElement image, in DOMString repetition); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::CreatePattern(nsIDOMHTMLElement *image, const nsAString & repetition, nsIDOMCanvasPattern **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float lineWidth; */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::GetLineWidth(float *aLineWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::SetLineWidth(float aLineWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString lineCap; */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::GetLineCap(nsAString & aLineCap)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::SetLineCap(const nsAString & aLineCap)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString lineJoin; */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::GetLineJoin(nsAString & aLineJoin)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::SetLineJoin(const nsAString & aLineJoin)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float miterLimit; */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::GetMiterLimit(float *aMiterLimit)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::SetMiterLimit(float aMiterLimit)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float shadowOffsetX; */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::GetShadowOffsetX(float *aShadowOffsetX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::SetShadowOffsetX(float aShadowOffsetX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float shadowOffsetY; */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::GetShadowOffsetY(float *aShadowOffsetY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::SetShadowOffsetY(float aShadowOffsetY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float shadowBlur; */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::GetShadowBlur(float *aShadowBlur)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::SetShadowBlur(float aShadowBlur)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString shadowColor; */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::GetShadowColor(nsAString & aShadowColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::SetShadowColor(const nsAString & aShadowColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void clearRect (in float x, in float y, in float w, in float h); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::ClearRect(float x, float y, float w, float h)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void fillRect (in float x, in float y, in float w, in float h); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::FillRect(float x, float y, float w, float h)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void strokeRect (in float x, in float y, in float w, in float h); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::StrokeRect(float x, float y, float w, float h)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void beginPath (); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::BeginPath()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void closePath (); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::ClosePath()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void moveTo (in float x, in float y); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::MoveTo(float x, float y)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void lineTo (in float x, in float y); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::LineTo(float x, float y)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void quadraticCurveTo (in float cpx, in float cpy, in float x, in float y); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::QuadraticCurveTo(float cpx, float cpy, float x, float y)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void bezierCurveTo (in float cp1x, in float cp1y, in float cp2x, in float cp2y, in float x, in float y); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::BezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void arcTo (in float x1, in float y1, in float x2, in float y2, in float radius); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::ArcTo(float x1, float y1, float x2, float y2, float radius)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void arc (in float x, in float y, in float r, in float startAngle, in float endAngle, in boolean clockwise); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::Arc(float x, float y, float r, float startAngle, float endAngle, PRBool clockwise)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void rect (in float x, in float y, in float w, in float h); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::Rect(float x, float y, float w, float h)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void fill (); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::Fill()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void stroke (); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::Stroke()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void clip (); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::Clip()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void drawImage (); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::DrawImage()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isPointInPath (in float x, in float y); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::IsPointInPath(float x, float y, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getImageData (); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::GetImageData()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void putImageData (); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::PutImageData()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void drawWindow (in nsIDOMWindow window, in long x, in long y, in long w, in long h, in DOMString bgColor); */
NS_IMETHODIMP nsDOMCanvasRenderingContext2D::DrawWindow(nsIDOMWindow *window, PRInt32 x, PRInt32 y, PRInt32 w, PRInt32 h, const nsAString & bgColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMCanvasRenderingContext2D_h__ */
