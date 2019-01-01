/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/svg/renderer/public/nsISVGRectangleSink.idl
 */

#ifndef __gen_nsISVGRectangleSink_h__
#define __gen_nsISVGRectangleSink_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsISVGRectangleSink */
#define NS_ISVGRECTANGLESINK_IID_STR "0340df1d-1096-445f-bbf9-e1d1e5a10827"

#define NS_ISVGRECTANGLESINK_IID \
  {0x0340df1d, 0x1096, 0x445f, \
    { 0xbb, 0xf9, 0xe1, 0xd1, 0xe5, 0xa1, 0x08, 0x27 }}

/**
 * \addtogroup rendering_backend_interfaces Rendering Backend Interfaces
 * @{
 */
/**
 * Interface handed to nsISVGRendererRegion::getRectangleScans() to
 * obtain an approximation of the region with rectangles.
 */
class NS_NO_VTABLE nsISVGRectangleSink : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGRECTANGLESINK_IID)

  /* void sinkRectangle (in float x, in float y, in float width, in float height); */
  NS_IMETHOD SinkRectangle(float x, float y, float width, float height) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGRECTANGLESINK \
  NS_IMETHOD SinkRectangle(float x, float y, float width, float height); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGRECTANGLESINK(_to) \
  NS_IMETHOD SinkRectangle(float x, float y, float width, float height) { return _to SinkRectangle(x, y, width, height); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGRECTANGLESINK(_to) \
  NS_IMETHOD SinkRectangle(float x, float y, float width, float height) { return !_to ? NS_ERROR_NULL_POINTER : _to->SinkRectangle(x, y, width, height); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGRectangleSink : public nsISVGRectangleSink
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGRECTANGLESINK

  nsSVGRectangleSink();

private:
  ~nsSVGRectangleSink();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGRectangleSink, nsISVGRectangleSink)

nsSVGRectangleSink::nsSVGRectangleSink()
{
  /* member initializers and constructor code */
}

nsSVGRectangleSink::~nsSVGRectangleSink()
{
  /* destructor code */
}

/* void sinkRectangle (in float x, in float y, in float width, in float height); */
NS_IMETHODIMP nsSVGRectangleSink::SinkRectangle(float x, float y, float width, float height)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISVGRectangleSink_h__ */
