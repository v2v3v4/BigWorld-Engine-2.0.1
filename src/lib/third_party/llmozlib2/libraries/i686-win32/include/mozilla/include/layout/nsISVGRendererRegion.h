/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/svg/renderer/public/nsISVGRendererRegion.idl
 */

#ifndef __gen_nsISVGRendererRegion_h__
#define __gen_nsISVGRendererRegion_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsISVGRectangleSink; /* forward declaration */


/* starting interface:    nsISVGRendererRegion */
#define NS_ISVGRENDERERREGION_IID_STR "9356e1c6-66e6-49a0-8c67-7e910270ed1e"

#define NS_ISVGRENDERERREGION_IID \
  {0x9356e1c6, 0x66e6, 0x49a0, \
    { 0x8c, 0x67, 0x7e, 0x91, 0x02, 0x70, 0xed, 0x1e }}

/**
 * \addtogroup renderer_interfaces Rendering Engine Interfaces
 * @{
 */
/**
 * One of a number of interfaces (all starting with nsISVGRenderer*)
 * to be implemented by an SVG rendering engine. See nsISVGRenderer
 * for more details.
 *
 * This interface abstracts a rendering engine-native region object.
 */
class NS_NO_VTABLE nsISVGRendererRegion : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGRENDERERREGION_IID)

  /**
   * Return union of this region with another region.
   *
   * @param other Region to combine with.
   * @return Union region.
   */
  /* nsISVGRendererRegion combine (in nsISVGRendererRegion other); */
  NS_IMETHOD Combine(nsISVGRendererRegion *other, nsISVGRendererRegion **_retval) = 0;

  /**
   * Write a sequence of rectangles approximating this region to the
   * sink object. The approximation can be crude but should fully
   * contain the actual region area.
   *
   * @param sink Rectangle sink to write to.
   */
  /* void getRectangleScans (in nsISVGRectangleSink sink); */
  NS_IMETHOD GetRectangleScans(nsISVGRectangleSink *sink) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGRENDERERREGION \
  NS_IMETHOD Combine(nsISVGRendererRegion *other, nsISVGRendererRegion **_retval); \
  NS_IMETHOD GetRectangleScans(nsISVGRectangleSink *sink); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGRENDERERREGION(_to) \
  NS_IMETHOD Combine(nsISVGRendererRegion *other, nsISVGRendererRegion **_retval) { return _to Combine(other, _retval); } \
  NS_IMETHOD GetRectangleScans(nsISVGRectangleSink *sink) { return _to GetRectangleScans(sink); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGRENDERERREGION(_to) \
  NS_IMETHOD Combine(nsISVGRendererRegion *other, nsISVGRendererRegion **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Combine(other, _retval); } \
  NS_IMETHOD GetRectangleScans(nsISVGRectangleSink *sink) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRectangleScans(sink); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGRendererRegion : public nsISVGRendererRegion
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGRENDERERREGION

  nsSVGRendererRegion();

private:
  ~nsSVGRendererRegion();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGRendererRegion, nsISVGRendererRegion)

nsSVGRendererRegion::nsSVGRendererRegion()
{
  /* member initializers and constructor code */
}

nsSVGRendererRegion::~nsSVGRendererRegion()
{
  /* destructor code */
}

/* nsISVGRendererRegion combine (in nsISVGRendererRegion other); */
NS_IMETHODIMP nsSVGRendererRegion::Combine(nsISVGRendererRegion *other, nsISVGRendererRegion **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getRectangleScans (in nsISVGRectangleSink sink); */
NS_IMETHODIMP nsSVGRendererRegion::GetRectangleScans(nsISVGRectangleSink *sink)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISVGRendererRegion_h__ */
