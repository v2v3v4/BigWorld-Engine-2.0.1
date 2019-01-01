/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/svg/nsIDOMSVGPathSeg.idl
 */

#ifndef __gen_nsIDOMSVGPathSeg_h__
#define __gen_nsIDOMSVGPathSeg_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMSVGPathSeg */
#define NS_IDOMSVGPATHSEG_IID_STR "b9022da7-e26d-4df3-8c94-b45c4aedda7c"

#define NS_IDOMSVGPATHSEG_IID \
  {0xb9022da7, 0xe26d, 0x4df3, \
    { 0x8c, 0x94, 0xb4, 0x5c, 0x4a, 0xed, 0xda, 0x7c }}

class NS_NO_VTABLE nsIDOMSVGPathSeg : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEG_IID)

  enum { PATHSEG_UNKNOWN = 0U };

  enum { PATHSEG_CLOSEPATH = 1U };

  enum { PATHSEG_MOVETO_ABS = 2U };

  enum { PATHSEG_MOVETO_REL = 3U };

  enum { PATHSEG_LINETO_ABS = 4U };

  enum { PATHSEG_LINETO_REL = 5U };

  enum { PATHSEG_CURVETO_CUBIC_ABS = 6U };

  enum { PATHSEG_CURVETO_CUBIC_REL = 7U };

  enum { PATHSEG_CURVETO_QUADRATIC_ABS = 8U };

  enum { PATHSEG_CURVETO_QUADRATIC_REL = 9U };

  enum { PATHSEG_ARC_ABS = 10U };

  enum { PATHSEG_ARC_REL = 11U };

  enum { PATHSEG_LINETO_HORIZONTAL_ABS = 12U };

  enum { PATHSEG_LINETO_HORIZONTAL_REL = 13U };

  enum { PATHSEG_LINETO_VERTICAL_ABS = 14U };

  enum { PATHSEG_LINETO_VERTICAL_REL = 15U };

  enum { PATHSEG_CURVETO_CUBIC_SMOOTH_ABS = 16U };

  enum { PATHSEG_CURVETO_CUBIC_SMOOTH_REL = 17U };

  enum { PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS = 18U };

  enum { PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL = 19U };

  /* readonly attribute unsigned short pathSegType; */
  NS_IMETHOD GetPathSegType(PRUint16 *aPathSegType) = 0;

  /* readonly attribute DOMString pathSegTypeAsLetter; */
  NS_IMETHOD GetPathSegTypeAsLetter(nsAString & aPathSegTypeAsLetter) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEG \
  NS_IMETHOD GetPathSegType(PRUint16 *aPathSegType); \
  NS_IMETHOD GetPathSegTypeAsLetter(nsAString & aPathSegTypeAsLetter); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEG(_to) \
  NS_IMETHOD GetPathSegType(PRUint16 *aPathSegType) { return _to GetPathSegType(aPathSegType); } \
  NS_IMETHOD GetPathSegTypeAsLetter(nsAString & aPathSegTypeAsLetter) { return _to GetPathSegTypeAsLetter(aPathSegTypeAsLetter); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEG(_to) \
  NS_IMETHOD GetPathSegType(PRUint16 *aPathSegType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPathSegType(aPathSegType); } \
  NS_IMETHOD GetPathSegTypeAsLetter(nsAString & aPathSegTypeAsLetter) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPathSegTypeAsLetter(aPathSegTypeAsLetter); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSeg : public nsIDOMSVGPathSeg
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEG

  nsDOMSVGPathSeg();

private:
  ~nsDOMSVGPathSeg();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSeg, nsIDOMSVGPathSeg)

nsDOMSVGPathSeg::nsDOMSVGPathSeg()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSeg::~nsDOMSVGPathSeg()
{
  /* destructor code */
}

/* readonly attribute unsigned short pathSegType; */
NS_IMETHODIMP nsDOMSVGPathSeg::GetPathSegType(PRUint16 *aPathSegType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString pathSegTypeAsLetter; */
NS_IMETHODIMP nsDOMSVGPathSeg::GetPathSegTypeAsLetter(nsAString & aPathSegTypeAsLetter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegClosePath */
#define NS_IDOMSVGPATHSEGCLOSEPATH_IID_STR "2b72d033-f115-45aa-9748-8c11ea07b845"

#define NS_IDOMSVGPATHSEGCLOSEPATH_IID \
  {0x2b72d033, 0xf115, 0x45aa, \
    { 0x97, 0x48, 0x8c, 0x11, 0xea, 0x07, 0xb8, 0x45 }}

class NS_NO_VTABLE nsIDOMSVGPathSegClosePath : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGCLOSEPATH_IID)

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGCLOSEPATH \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGCLOSEPATH(_to) \
  /* no methods! */

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGCLOSEPATH(_to) \
  /* no methods! */

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegClosePath : public nsIDOMSVGPathSegClosePath
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGCLOSEPATH

  nsDOMSVGPathSegClosePath();

private:
  ~nsDOMSVGPathSegClosePath();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegClosePath, nsIDOMSVGPathSegClosePath)

nsDOMSVGPathSegClosePath::nsDOMSVGPathSegClosePath()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegClosePath::~nsDOMSVGPathSegClosePath()
{
  /* destructor code */
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegMovetoAbs */
#define NS_IDOMSVGPATHSEGMOVETOABS_IID_STR "b0106d01-9746-440b-b067-68ee043dabc3"

#define NS_IDOMSVGPATHSEGMOVETOABS_IID \
  {0xb0106d01, 0x9746, 0x440b, \
    { 0xb0, 0x67, 0x68, 0xee, 0x04, 0x3d, 0xab, 0xc3 }}

class NS_NO_VTABLE nsIDOMSVGPathSegMovetoAbs : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGMOVETOABS_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGMOVETOABS \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGMOVETOABS(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGMOVETOABS(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegMovetoAbs : public nsIDOMSVGPathSegMovetoAbs
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGMOVETOABS

  nsDOMSVGPathSegMovetoAbs();

private:
  ~nsDOMSVGPathSegMovetoAbs();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegMovetoAbs, nsIDOMSVGPathSegMovetoAbs)

nsDOMSVGPathSegMovetoAbs::nsDOMSVGPathSegMovetoAbs()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegMovetoAbs::~nsDOMSVGPathSegMovetoAbs()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegMovetoAbs::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegMovetoAbs::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegMovetoAbs::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegMovetoAbs::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegMovetoRel */
#define NS_IDOMSVGPATHSEGMOVETOREL_IID_STR "c6ee1ddd-8b35-4e1b-b381-c063a28012d9"

#define NS_IDOMSVGPATHSEGMOVETOREL_IID \
  {0xc6ee1ddd, 0x8b35, 0x4e1b, \
    { 0xb3, 0x81, 0xc0, 0x63, 0xa2, 0x80, 0x12, 0xd9 }}

class NS_NO_VTABLE nsIDOMSVGPathSegMovetoRel : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGMOVETOREL_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGMOVETOREL \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGMOVETOREL(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGMOVETOREL(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegMovetoRel : public nsIDOMSVGPathSegMovetoRel
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGMOVETOREL

  nsDOMSVGPathSegMovetoRel();

private:
  ~nsDOMSVGPathSegMovetoRel();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegMovetoRel, nsIDOMSVGPathSegMovetoRel)

nsDOMSVGPathSegMovetoRel::nsDOMSVGPathSegMovetoRel()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegMovetoRel::~nsDOMSVGPathSegMovetoRel()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegMovetoRel::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegMovetoRel::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegMovetoRel::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegMovetoRel::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegLinetoAbs */
#define NS_IDOMSVGPATHSEGLINETOABS_IID_STR "bac3648d-55a3-491b-9863-a18fd7506689"

#define NS_IDOMSVGPATHSEGLINETOABS_IID \
  {0xbac3648d, 0x55a3, 0x491b, \
    { 0x98, 0x63, 0xa1, 0x8f, 0xd7, 0x50, 0x66, 0x89 }}

class NS_NO_VTABLE nsIDOMSVGPathSegLinetoAbs : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGLINETOABS_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGLINETOABS \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGLINETOABS(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGLINETOABS(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegLinetoAbs : public nsIDOMSVGPathSegLinetoAbs
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGLINETOABS

  nsDOMSVGPathSegLinetoAbs();

private:
  ~nsDOMSVGPathSegLinetoAbs();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegLinetoAbs, nsIDOMSVGPathSegLinetoAbs)

nsDOMSVGPathSegLinetoAbs::nsDOMSVGPathSegLinetoAbs()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegLinetoAbs::~nsDOMSVGPathSegLinetoAbs()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegLinetoAbs::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegLinetoAbs::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegLinetoAbs::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegLinetoAbs::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegLinetoRel */
#define NS_IDOMSVGPATHSEGLINETOREL_IID_STR "3294d20e-c707-4e59-a625-bde93fc0b25f"

#define NS_IDOMSVGPATHSEGLINETOREL_IID \
  {0x3294d20e, 0xc707, 0x4e59, \
    { 0xa6, 0x25, 0xbd, 0xe9, 0x3f, 0xc0, 0xb2, 0x5f }}

class NS_NO_VTABLE nsIDOMSVGPathSegLinetoRel : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGLINETOREL_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGLINETOREL \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGLINETOREL(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGLINETOREL(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegLinetoRel : public nsIDOMSVGPathSegLinetoRel
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGLINETOREL

  nsDOMSVGPathSegLinetoRel();

private:
  ~nsDOMSVGPathSegLinetoRel();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegLinetoRel, nsIDOMSVGPathSegLinetoRel)

nsDOMSVGPathSegLinetoRel::nsDOMSVGPathSegLinetoRel()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegLinetoRel::~nsDOMSVGPathSegLinetoRel()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegLinetoRel::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegLinetoRel::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegLinetoRel::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegLinetoRel::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegCurvetoCubicAbs */
#define NS_IDOMSVGPATHSEGCURVETOCUBICABS_IID_STR "ad929b96-ef81-4002-b596-c6a8b3a878e9"

#define NS_IDOMSVGPATHSEGCURVETOCUBICABS_IID \
  {0xad929b96, 0xef81, 0x4002, \
    { 0xb5, 0x96, 0xc6, 0xa8, 0xb3, 0xa8, 0x78, 0xe9 }}

class NS_NO_VTABLE nsIDOMSVGPathSegCurvetoCubicAbs : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGCURVETOCUBICABS_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

  /* attribute float x1; */
  NS_IMETHOD GetX1(float *aX1) = 0;
  NS_IMETHOD SetX1(float aX1) = 0;

  /* attribute float y1; */
  NS_IMETHOD GetY1(float *aY1) = 0;
  NS_IMETHOD SetY1(float aY1) = 0;

  /* attribute float x2; */
  NS_IMETHOD GetX2(float *aX2) = 0;
  NS_IMETHOD SetX2(float aX2) = 0;

  /* attribute float y2; */
  NS_IMETHOD GetY2(float *aY2) = 0;
  NS_IMETHOD SetY2(float aY2) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGCURVETOCUBICABS \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); \
  NS_IMETHOD GetX1(float *aX1); \
  NS_IMETHOD SetX1(float aX1); \
  NS_IMETHOD GetY1(float *aY1); \
  NS_IMETHOD SetY1(float aY1); \
  NS_IMETHOD GetX2(float *aX2); \
  NS_IMETHOD SetX2(float aX2); \
  NS_IMETHOD GetY2(float *aY2); \
  NS_IMETHOD SetY2(float aY2); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGCURVETOCUBICABS(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } \
  NS_IMETHOD GetX1(float *aX1) { return _to GetX1(aX1); } \
  NS_IMETHOD SetX1(float aX1) { return _to SetX1(aX1); } \
  NS_IMETHOD GetY1(float *aY1) { return _to GetY1(aY1); } \
  NS_IMETHOD SetY1(float aY1) { return _to SetY1(aY1); } \
  NS_IMETHOD GetX2(float *aX2) { return _to GetX2(aX2); } \
  NS_IMETHOD SetX2(float aX2) { return _to SetX2(aX2); } \
  NS_IMETHOD GetY2(float *aY2) { return _to GetY2(aY2); } \
  NS_IMETHOD SetY2(float aY2) { return _to SetY2(aY2); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGCURVETOCUBICABS(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } \
  NS_IMETHOD GetX1(float *aX1) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX1(aX1); } \
  NS_IMETHOD SetX1(float aX1) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX1(aX1); } \
  NS_IMETHOD GetY1(float *aY1) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY1(aY1); } \
  NS_IMETHOD SetY1(float aY1) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY1(aY1); } \
  NS_IMETHOD GetX2(float *aX2) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX2(aX2); } \
  NS_IMETHOD SetX2(float aX2) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX2(aX2); } \
  NS_IMETHOD GetY2(float *aY2) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY2(aY2); } \
  NS_IMETHOD SetY2(float aY2) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY2(aY2); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegCurvetoCubicAbs : public nsIDOMSVGPathSegCurvetoCubicAbs
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGCURVETOCUBICABS

  nsDOMSVGPathSegCurvetoCubicAbs();

private:
  ~nsDOMSVGPathSegCurvetoCubicAbs();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegCurvetoCubicAbs, nsIDOMSVGPathSegCurvetoCubicAbs)

nsDOMSVGPathSegCurvetoCubicAbs::nsDOMSVGPathSegCurvetoCubicAbs()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegCurvetoCubicAbs::~nsDOMSVGPathSegCurvetoCubicAbs()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicAbs::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicAbs::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicAbs::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicAbs::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float x1; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicAbs::GetX1(float *aX1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicAbs::SetX1(float aX1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y1; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicAbs::GetY1(float *aY1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicAbs::SetY1(float aY1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float x2; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicAbs::GetX2(float *aX2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicAbs::SetX2(float aX2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y2; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicAbs::GetY2(float *aY2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicAbs::SetY2(float aY2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegCurvetoCubicRel */
#define NS_IDOMSVGPATHSEGCURVETOCUBICREL_IID_STR "dc7ba13f-8cb6-48d2-9e22-a4a6817abbb9"

#define NS_IDOMSVGPATHSEGCURVETOCUBICREL_IID \
  {0xdc7ba13f, 0x8cb6, 0x48d2, \
    { 0x9e, 0x22, 0xa4, 0xa6, 0x81, 0x7a, 0xbb, 0xb9 }}

class NS_NO_VTABLE nsIDOMSVGPathSegCurvetoCubicRel : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGCURVETOCUBICREL_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

  /* attribute float x1; */
  NS_IMETHOD GetX1(float *aX1) = 0;
  NS_IMETHOD SetX1(float aX1) = 0;

  /* attribute float y1; */
  NS_IMETHOD GetY1(float *aY1) = 0;
  NS_IMETHOD SetY1(float aY1) = 0;

  /* attribute float x2; */
  NS_IMETHOD GetX2(float *aX2) = 0;
  NS_IMETHOD SetX2(float aX2) = 0;

  /* attribute float y2; */
  NS_IMETHOD GetY2(float *aY2) = 0;
  NS_IMETHOD SetY2(float aY2) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGCURVETOCUBICREL \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); \
  NS_IMETHOD GetX1(float *aX1); \
  NS_IMETHOD SetX1(float aX1); \
  NS_IMETHOD GetY1(float *aY1); \
  NS_IMETHOD SetY1(float aY1); \
  NS_IMETHOD GetX2(float *aX2); \
  NS_IMETHOD SetX2(float aX2); \
  NS_IMETHOD GetY2(float *aY2); \
  NS_IMETHOD SetY2(float aY2); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGCURVETOCUBICREL(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } \
  NS_IMETHOD GetX1(float *aX1) { return _to GetX1(aX1); } \
  NS_IMETHOD SetX1(float aX1) { return _to SetX1(aX1); } \
  NS_IMETHOD GetY1(float *aY1) { return _to GetY1(aY1); } \
  NS_IMETHOD SetY1(float aY1) { return _to SetY1(aY1); } \
  NS_IMETHOD GetX2(float *aX2) { return _to GetX2(aX2); } \
  NS_IMETHOD SetX2(float aX2) { return _to SetX2(aX2); } \
  NS_IMETHOD GetY2(float *aY2) { return _to GetY2(aY2); } \
  NS_IMETHOD SetY2(float aY2) { return _to SetY2(aY2); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGCURVETOCUBICREL(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } \
  NS_IMETHOD GetX1(float *aX1) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX1(aX1); } \
  NS_IMETHOD SetX1(float aX1) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX1(aX1); } \
  NS_IMETHOD GetY1(float *aY1) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY1(aY1); } \
  NS_IMETHOD SetY1(float aY1) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY1(aY1); } \
  NS_IMETHOD GetX2(float *aX2) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX2(aX2); } \
  NS_IMETHOD SetX2(float aX2) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX2(aX2); } \
  NS_IMETHOD GetY2(float *aY2) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY2(aY2); } \
  NS_IMETHOD SetY2(float aY2) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY2(aY2); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegCurvetoCubicRel : public nsIDOMSVGPathSegCurvetoCubicRel
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGCURVETOCUBICREL

  nsDOMSVGPathSegCurvetoCubicRel();

private:
  ~nsDOMSVGPathSegCurvetoCubicRel();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegCurvetoCubicRel, nsIDOMSVGPathSegCurvetoCubicRel)

nsDOMSVGPathSegCurvetoCubicRel::nsDOMSVGPathSegCurvetoCubicRel()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegCurvetoCubicRel::~nsDOMSVGPathSegCurvetoCubicRel()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicRel::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicRel::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicRel::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicRel::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float x1; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicRel::GetX1(float *aX1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicRel::SetX1(float aX1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y1; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicRel::GetY1(float *aY1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicRel::SetY1(float aY1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float x2; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicRel::GetX2(float *aX2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicRel::SetX2(float aX2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y2; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicRel::GetY2(float *aY2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicRel::SetY2(float aY2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegCurvetoQuadraticAbs */
#define NS_IDOMSVGPATHSEGCURVETOQUADRATICABS_IID_STR "ec4e8f65-5f4a-495e-a5f2-00e18d5e5f96"

#define NS_IDOMSVGPATHSEGCURVETOQUADRATICABS_IID \
  {0xec4e8f65, 0x5f4a, 0x495e, \
    { 0xa5, 0xf2, 0x00, 0xe1, 0x8d, 0x5e, 0x5f, 0x96 }}

class NS_NO_VTABLE nsIDOMSVGPathSegCurvetoQuadraticAbs : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGCURVETOQUADRATICABS_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

  /* attribute float x1; */
  NS_IMETHOD GetX1(float *aX1) = 0;
  NS_IMETHOD SetX1(float aX1) = 0;

  /* attribute float y1; */
  NS_IMETHOD GetY1(float *aY1) = 0;
  NS_IMETHOD SetY1(float aY1) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGCURVETOQUADRATICABS \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); \
  NS_IMETHOD GetX1(float *aX1); \
  NS_IMETHOD SetX1(float aX1); \
  NS_IMETHOD GetY1(float *aY1); \
  NS_IMETHOD SetY1(float aY1); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGCURVETOQUADRATICABS(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } \
  NS_IMETHOD GetX1(float *aX1) { return _to GetX1(aX1); } \
  NS_IMETHOD SetX1(float aX1) { return _to SetX1(aX1); } \
  NS_IMETHOD GetY1(float *aY1) { return _to GetY1(aY1); } \
  NS_IMETHOD SetY1(float aY1) { return _to SetY1(aY1); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGCURVETOQUADRATICABS(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } \
  NS_IMETHOD GetX1(float *aX1) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX1(aX1); } \
  NS_IMETHOD SetX1(float aX1) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX1(aX1); } \
  NS_IMETHOD GetY1(float *aY1) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY1(aY1); } \
  NS_IMETHOD SetY1(float aY1) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY1(aY1); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegCurvetoQuadraticAbs : public nsIDOMSVGPathSegCurvetoQuadraticAbs
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGCURVETOQUADRATICABS

  nsDOMSVGPathSegCurvetoQuadraticAbs();

private:
  ~nsDOMSVGPathSegCurvetoQuadraticAbs();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegCurvetoQuadraticAbs, nsIDOMSVGPathSegCurvetoQuadraticAbs)

nsDOMSVGPathSegCurvetoQuadraticAbs::nsDOMSVGPathSegCurvetoQuadraticAbs()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegCurvetoQuadraticAbs::~nsDOMSVGPathSegCurvetoQuadraticAbs()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticAbs::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticAbs::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticAbs::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticAbs::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float x1; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticAbs::GetX1(float *aX1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticAbs::SetX1(float aX1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y1; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticAbs::GetY1(float *aY1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticAbs::SetY1(float aY1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegCurvetoQuadraticRel */
#define NS_IDOMSVGPATHSEGCURVETOQUADRATICREL_IID_STR "7007113c-e06b-4256-8530-4884d5d769c6"

#define NS_IDOMSVGPATHSEGCURVETOQUADRATICREL_IID \
  {0x7007113c, 0xe06b, 0x4256, \
    { 0x85, 0x30, 0x48, 0x84, 0xd5, 0xd7, 0x69, 0xc6 }}

class NS_NO_VTABLE nsIDOMSVGPathSegCurvetoQuadraticRel : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGCURVETOQUADRATICREL_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

  /* attribute float x1; */
  NS_IMETHOD GetX1(float *aX1) = 0;
  NS_IMETHOD SetX1(float aX1) = 0;

  /* attribute float y1; */
  NS_IMETHOD GetY1(float *aY1) = 0;
  NS_IMETHOD SetY1(float aY1) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGCURVETOQUADRATICREL \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); \
  NS_IMETHOD GetX1(float *aX1); \
  NS_IMETHOD SetX1(float aX1); \
  NS_IMETHOD GetY1(float *aY1); \
  NS_IMETHOD SetY1(float aY1); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGCURVETOQUADRATICREL(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } \
  NS_IMETHOD GetX1(float *aX1) { return _to GetX1(aX1); } \
  NS_IMETHOD SetX1(float aX1) { return _to SetX1(aX1); } \
  NS_IMETHOD GetY1(float *aY1) { return _to GetY1(aY1); } \
  NS_IMETHOD SetY1(float aY1) { return _to SetY1(aY1); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGCURVETOQUADRATICREL(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } \
  NS_IMETHOD GetX1(float *aX1) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX1(aX1); } \
  NS_IMETHOD SetX1(float aX1) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX1(aX1); } \
  NS_IMETHOD GetY1(float *aY1) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY1(aY1); } \
  NS_IMETHOD SetY1(float aY1) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY1(aY1); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegCurvetoQuadraticRel : public nsIDOMSVGPathSegCurvetoQuadraticRel
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGCURVETOQUADRATICREL

  nsDOMSVGPathSegCurvetoQuadraticRel();

private:
  ~nsDOMSVGPathSegCurvetoQuadraticRel();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegCurvetoQuadraticRel, nsIDOMSVGPathSegCurvetoQuadraticRel)

nsDOMSVGPathSegCurvetoQuadraticRel::nsDOMSVGPathSegCurvetoQuadraticRel()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegCurvetoQuadraticRel::~nsDOMSVGPathSegCurvetoQuadraticRel()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticRel::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticRel::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticRel::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticRel::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float x1; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticRel::GetX1(float *aX1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticRel::SetX1(float aX1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y1; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticRel::GetY1(float *aY1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticRel::SetY1(float aY1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegArcAbs */
#define NS_IDOMSVGPATHSEGARCABS_IID_STR "c26e1779-604b-4bad-8a29-02d2a2113769"

#define NS_IDOMSVGPATHSEGARCABS_IID \
  {0xc26e1779, 0x604b, 0x4bad, \
    { 0x8a, 0x29, 0x02, 0xd2, 0xa2, 0x11, 0x37, 0x69 }}

class NS_NO_VTABLE nsIDOMSVGPathSegArcAbs : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGARCABS_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

  /* attribute float r1; */
  NS_IMETHOD GetR1(float *aR1) = 0;
  NS_IMETHOD SetR1(float aR1) = 0;

  /* attribute float r2; */
  NS_IMETHOD GetR2(float *aR2) = 0;
  NS_IMETHOD SetR2(float aR2) = 0;

  /* attribute float angle; */
  NS_IMETHOD GetAngle(float *aAngle) = 0;
  NS_IMETHOD SetAngle(float aAngle) = 0;

  /* attribute boolean largeArcFlag; */
  NS_IMETHOD GetLargeArcFlag(PRBool *aLargeArcFlag) = 0;
  NS_IMETHOD SetLargeArcFlag(PRBool aLargeArcFlag) = 0;

  /* attribute boolean sweepFlag; */
  NS_IMETHOD GetSweepFlag(PRBool *aSweepFlag) = 0;
  NS_IMETHOD SetSweepFlag(PRBool aSweepFlag) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGARCABS \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); \
  NS_IMETHOD GetR1(float *aR1); \
  NS_IMETHOD SetR1(float aR1); \
  NS_IMETHOD GetR2(float *aR2); \
  NS_IMETHOD SetR2(float aR2); \
  NS_IMETHOD GetAngle(float *aAngle); \
  NS_IMETHOD SetAngle(float aAngle); \
  NS_IMETHOD GetLargeArcFlag(PRBool *aLargeArcFlag); \
  NS_IMETHOD SetLargeArcFlag(PRBool aLargeArcFlag); \
  NS_IMETHOD GetSweepFlag(PRBool *aSweepFlag); \
  NS_IMETHOD SetSweepFlag(PRBool aSweepFlag); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGARCABS(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } \
  NS_IMETHOD GetR1(float *aR1) { return _to GetR1(aR1); } \
  NS_IMETHOD SetR1(float aR1) { return _to SetR1(aR1); } \
  NS_IMETHOD GetR2(float *aR2) { return _to GetR2(aR2); } \
  NS_IMETHOD SetR2(float aR2) { return _to SetR2(aR2); } \
  NS_IMETHOD GetAngle(float *aAngle) { return _to GetAngle(aAngle); } \
  NS_IMETHOD SetAngle(float aAngle) { return _to SetAngle(aAngle); } \
  NS_IMETHOD GetLargeArcFlag(PRBool *aLargeArcFlag) { return _to GetLargeArcFlag(aLargeArcFlag); } \
  NS_IMETHOD SetLargeArcFlag(PRBool aLargeArcFlag) { return _to SetLargeArcFlag(aLargeArcFlag); } \
  NS_IMETHOD GetSweepFlag(PRBool *aSweepFlag) { return _to GetSweepFlag(aSweepFlag); } \
  NS_IMETHOD SetSweepFlag(PRBool aSweepFlag) { return _to SetSweepFlag(aSweepFlag); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGARCABS(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } \
  NS_IMETHOD GetR1(float *aR1) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetR1(aR1); } \
  NS_IMETHOD SetR1(float aR1) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetR1(aR1); } \
  NS_IMETHOD GetR2(float *aR2) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetR2(aR2); } \
  NS_IMETHOD SetR2(float aR2) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetR2(aR2); } \
  NS_IMETHOD GetAngle(float *aAngle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAngle(aAngle); } \
  NS_IMETHOD SetAngle(float aAngle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetAngle(aAngle); } \
  NS_IMETHOD GetLargeArcFlag(PRBool *aLargeArcFlag) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLargeArcFlag(aLargeArcFlag); } \
  NS_IMETHOD SetLargeArcFlag(PRBool aLargeArcFlag) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetLargeArcFlag(aLargeArcFlag); } \
  NS_IMETHOD GetSweepFlag(PRBool *aSweepFlag) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSweepFlag(aSweepFlag); } \
  NS_IMETHOD SetSweepFlag(PRBool aSweepFlag) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSweepFlag(aSweepFlag); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegArcAbs : public nsIDOMSVGPathSegArcAbs
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGARCABS

  nsDOMSVGPathSegArcAbs();

private:
  ~nsDOMSVGPathSegArcAbs();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegArcAbs, nsIDOMSVGPathSegArcAbs)

nsDOMSVGPathSegArcAbs::nsDOMSVGPathSegArcAbs()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegArcAbs::~nsDOMSVGPathSegArcAbs()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegArcAbs::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegArcAbs::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegArcAbs::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegArcAbs::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float r1; */
NS_IMETHODIMP nsDOMSVGPathSegArcAbs::GetR1(float *aR1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegArcAbs::SetR1(float aR1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float r2; */
NS_IMETHODIMP nsDOMSVGPathSegArcAbs::GetR2(float *aR2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegArcAbs::SetR2(float aR2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float angle; */
NS_IMETHODIMP nsDOMSVGPathSegArcAbs::GetAngle(float *aAngle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegArcAbs::SetAngle(float aAngle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean largeArcFlag; */
NS_IMETHODIMP nsDOMSVGPathSegArcAbs::GetLargeArcFlag(PRBool *aLargeArcFlag)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegArcAbs::SetLargeArcFlag(PRBool aLargeArcFlag)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean sweepFlag; */
NS_IMETHODIMP nsDOMSVGPathSegArcAbs::GetSweepFlag(PRBool *aSweepFlag)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegArcAbs::SetSweepFlag(PRBool aSweepFlag)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegArcRel */
#define NS_IDOMSVGPATHSEGARCREL_IID_STR "a685997e-fb47-47c0-a34c-5da11cb66537"

#define NS_IDOMSVGPATHSEGARCREL_IID \
  {0xa685997e, 0xfb47, 0x47c0, \
    { 0xa3, 0x4c, 0x5d, 0xa1, 0x1c, 0xb6, 0x65, 0x37 }}

class NS_NO_VTABLE nsIDOMSVGPathSegArcRel : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGARCREL_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

  /* attribute float r1; */
  NS_IMETHOD GetR1(float *aR1) = 0;
  NS_IMETHOD SetR1(float aR1) = 0;

  /* attribute float r2; */
  NS_IMETHOD GetR2(float *aR2) = 0;
  NS_IMETHOD SetR2(float aR2) = 0;

  /* attribute float angle; */
  NS_IMETHOD GetAngle(float *aAngle) = 0;
  NS_IMETHOD SetAngle(float aAngle) = 0;

  /* attribute boolean largeArcFlag; */
  NS_IMETHOD GetLargeArcFlag(PRBool *aLargeArcFlag) = 0;
  NS_IMETHOD SetLargeArcFlag(PRBool aLargeArcFlag) = 0;

  /* attribute boolean sweepFlag; */
  NS_IMETHOD GetSweepFlag(PRBool *aSweepFlag) = 0;
  NS_IMETHOD SetSweepFlag(PRBool aSweepFlag) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGARCREL \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); \
  NS_IMETHOD GetR1(float *aR1); \
  NS_IMETHOD SetR1(float aR1); \
  NS_IMETHOD GetR2(float *aR2); \
  NS_IMETHOD SetR2(float aR2); \
  NS_IMETHOD GetAngle(float *aAngle); \
  NS_IMETHOD SetAngle(float aAngle); \
  NS_IMETHOD GetLargeArcFlag(PRBool *aLargeArcFlag); \
  NS_IMETHOD SetLargeArcFlag(PRBool aLargeArcFlag); \
  NS_IMETHOD GetSweepFlag(PRBool *aSweepFlag); \
  NS_IMETHOD SetSweepFlag(PRBool aSweepFlag); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGARCREL(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } \
  NS_IMETHOD GetR1(float *aR1) { return _to GetR1(aR1); } \
  NS_IMETHOD SetR1(float aR1) { return _to SetR1(aR1); } \
  NS_IMETHOD GetR2(float *aR2) { return _to GetR2(aR2); } \
  NS_IMETHOD SetR2(float aR2) { return _to SetR2(aR2); } \
  NS_IMETHOD GetAngle(float *aAngle) { return _to GetAngle(aAngle); } \
  NS_IMETHOD SetAngle(float aAngle) { return _to SetAngle(aAngle); } \
  NS_IMETHOD GetLargeArcFlag(PRBool *aLargeArcFlag) { return _to GetLargeArcFlag(aLargeArcFlag); } \
  NS_IMETHOD SetLargeArcFlag(PRBool aLargeArcFlag) { return _to SetLargeArcFlag(aLargeArcFlag); } \
  NS_IMETHOD GetSweepFlag(PRBool *aSweepFlag) { return _to GetSweepFlag(aSweepFlag); } \
  NS_IMETHOD SetSweepFlag(PRBool aSweepFlag) { return _to SetSweepFlag(aSweepFlag); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGARCREL(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } \
  NS_IMETHOD GetR1(float *aR1) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetR1(aR1); } \
  NS_IMETHOD SetR1(float aR1) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetR1(aR1); } \
  NS_IMETHOD GetR2(float *aR2) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetR2(aR2); } \
  NS_IMETHOD SetR2(float aR2) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetR2(aR2); } \
  NS_IMETHOD GetAngle(float *aAngle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAngle(aAngle); } \
  NS_IMETHOD SetAngle(float aAngle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetAngle(aAngle); } \
  NS_IMETHOD GetLargeArcFlag(PRBool *aLargeArcFlag) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLargeArcFlag(aLargeArcFlag); } \
  NS_IMETHOD SetLargeArcFlag(PRBool aLargeArcFlag) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetLargeArcFlag(aLargeArcFlag); } \
  NS_IMETHOD GetSweepFlag(PRBool *aSweepFlag) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSweepFlag(aSweepFlag); } \
  NS_IMETHOD SetSweepFlag(PRBool aSweepFlag) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSweepFlag(aSweepFlag); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegArcRel : public nsIDOMSVGPathSegArcRel
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGARCREL

  nsDOMSVGPathSegArcRel();

private:
  ~nsDOMSVGPathSegArcRel();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegArcRel, nsIDOMSVGPathSegArcRel)

nsDOMSVGPathSegArcRel::nsDOMSVGPathSegArcRel()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegArcRel::~nsDOMSVGPathSegArcRel()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegArcRel::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegArcRel::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegArcRel::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegArcRel::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float r1; */
NS_IMETHODIMP nsDOMSVGPathSegArcRel::GetR1(float *aR1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegArcRel::SetR1(float aR1)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float r2; */
NS_IMETHODIMP nsDOMSVGPathSegArcRel::GetR2(float *aR2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegArcRel::SetR2(float aR2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float angle; */
NS_IMETHODIMP nsDOMSVGPathSegArcRel::GetAngle(float *aAngle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegArcRel::SetAngle(float aAngle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean largeArcFlag; */
NS_IMETHODIMP nsDOMSVGPathSegArcRel::GetLargeArcFlag(PRBool *aLargeArcFlag)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegArcRel::SetLargeArcFlag(PRBool aLargeArcFlag)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean sweepFlag; */
NS_IMETHODIMP nsDOMSVGPathSegArcRel::GetSweepFlag(PRBool *aSweepFlag)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegArcRel::SetSweepFlag(PRBool aSweepFlag)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegLinetoHorizontalAbs */
#define NS_IDOMSVGPATHSEGLINETOHORIZONTALABS_IID_STR "e74b55ef-1c44-4a40-9f51-a2196b11283a"

#define NS_IDOMSVGPATHSEGLINETOHORIZONTALABS_IID \
  {0xe74b55ef, 0x1c44, 0x4a40, \
    { 0x9f, 0x51, 0xa2, 0x19, 0x6b, 0x11, 0x28, 0x3a }}

class NS_NO_VTABLE nsIDOMSVGPathSegLinetoHorizontalAbs : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGLINETOHORIZONTALABS_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGLINETOHORIZONTALABS \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGLINETOHORIZONTALABS(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGLINETOHORIZONTALABS(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegLinetoHorizontalAbs : public nsIDOMSVGPathSegLinetoHorizontalAbs
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGLINETOHORIZONTALABS

  nsDOMSVGPathSegLinetoHorizontalAbs();

private:
  ~nsDOMSVGPathSegLinetoHorizontalAbs();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegLinetoHorizontalAbs, nsIDOMSVGPathSegLinetoHorizontalAbs)

nsDOMSVGPathSegLinetoHorizontalAbs::nsDOMSVGPathSegLinetoHorizontalAbs()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegLinetoHorizontalAbs::~nsDOMSVGPathSegLinetoHorizontalAbs()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegLinetoHorizontalAbs::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegLinetoHorizontalAbs::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegLinetoHorizontalRel */
#define NS_IDOMSVGPATHSEGLINETOHORIZONTALREL_IID_STR "0a797fdc-8b60-4cb3-a0da-4c898832ba30"

#define NS_IDOMSVGPATHSEGLINETOHORIZONTALREL_IID \
  {0x0a797fdc, 0x8b60, 0x4cb3, \
    { 0xa0, 0xda, 0x4c, 0x89, 0x88, 0x32, 0xba, 0x30 }}

class NS_NO_VTABLE nsIDOMSVGPathSegLinetoHorizontalRel : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGLINETOHORIZONTALREL_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGLINETOHORIZONTALREL \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGLINETOHORIZONTALREL(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGLINETOHORIZONTALREL(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegLinetoHorizontalRel : public nsIDOMSVGPathSegLinetoHorizontalRel
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGLINETOHORIZONTALREL

  nsDOMSVGPathSegLinetoHorizontalRel();

private:
  ~nsDOMSVGPathSegLinetoHorizontalRel();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegLinetoHorizontalRel, nsIDOMSVGPathSegLinetoHorizontalRel)

nsDOMSVGPathSegLinetoHorizontalRel::nsDOMSVGPathSegLinetoHorizontalRel()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegLinetoHorizontalRel::~nsDOMSVGPathSegLinetoHorizontalRel()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegLinetoHorizontalRel::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegLinetoHorizontalRel::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegLinetoVerticalAbs */
#define NS_IDOMSVGPATHSEGLINETOVERTICALABS_IID_STR "0811d434-3d90-4eec-8fa2-066dde037917"

#define NS_IDOMSVGPATHSEGLINETOVERTICALABS_IID \
  {0x0811d434, 0x3d90, 0x4eec, \
    { 0x8f, 0xa2, 0x06, 0x6d, 0xde, 0x03, 0x79, 0x17 }}

class NS_NO_VTABLE nsIDOMSVGPathSegLinetoVerticalAbs : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGLINETOVERTICALABS_IID)

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGLINETOVERTICALABS \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGLINETOVERTICALABS(_to) \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGLINETOVERTICALABS(_to) \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegLinetoVerticalAbs : public nsIDOMSVGPathSegLinetoVerticalAbs
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGLINETOVERTICALABS

  nsDOMSVGPathSegLinetoVerticalAbs();

private:
  ~nsDOMSVGPathSegLinetoVerticalAbs();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegLinetoVerticalAbs, nsIDOMSVGPathSegLinetoVerticalAbs)

nsDOMSVGPathSegLinetoVerticalAbs::nsDOMSVGPathSegLinetoVerticalAbs()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegLinetoVerticalAbs::~nsDOMSVGPathSegLinetoVerticalAbs()
{
  /* destructor code */
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegLinetoVerticalAbs::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegLinetoVerticalAbs::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegLinetoVerticalRel */
#define NS_IDOMSVGPATHSEGLINETOVERTICALREL_IID_STR "93db35b1-6b33-49d5-ad25-1ed1a7611ad2"

#define NS_IDOMSVGPATHSEGLINETOVERTICALREL_IID \
  {0x93db35b1, 0x6b33, 0x49d5, \
    { 0xad, 0x25, 0x1e, 0xd1, 0xa7, 0x61, 0x1a, 0xd2 }}

class NS_NO_VTABLE nsIDOMSVGPathSegLinetoVerticalRel : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGLINETOVERTICALREL_IID)

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGLINETOVERTICALREL \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGLINETOVERTICALREL(_to) \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGLINETOVERTICALREL(_to) \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegLinetoVerticalRel : public nsIDOMSVGPathSegLinetoVerticalRel
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGLINETOVERTICALREL

  nsDOMSVGPathSegLinetoVerticalRel();

private:
  ~nsDOMSVGPathSegLinetoVerticalRel();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegLinetoVerticalRel, nsIDOMSVGPathSegLinetoVerticalRel)

nsDOMSVGPathSegLinetoVerticalRel::nsDOMSVGPathSegLinetoVerticalRel()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegLinetoVerticalRel::~nsDOMSVGPathSegLinetoVerticalRel()
{
  /* destructor code */
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegLinetoVerticalRel::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegLinetoVerticalRel::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegCurvetoCubicSmoothAbs */
#define NS_IDOMSVGPATHSEGCURVETOCUBICSMOOTHABS_IID_STR "eb422132-514e-4a1c-81ec-b84a5df5fb96"

#define NS_IDOMSVGPATHSEGCURVETOCUBICSMOOTHABS_IID \
  {0xeb422132, 0x514e, 0x4a1c, \
    { 0x81, 0xec, 0xb8, 0x4a, 0x5d, 0xf5, 0xfb, 0x96 }}

class NS_NO_VTABLE nsIDOMSVGPathSegCurvetoCubicSmoothAbs : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGCURVETOCUBICSMOOTHABS_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

  /* attribute float x2; */
  NS_IMETHOD GetX2(float *aX2) = 0;
  NS_IMETHOD SetX2(float aX2) = 0;

  /* attribute float y2; */
  NS_IMETHOD GetY2(float *aY2) = 0;
  NS_IMETHOD SetY2(float aY2) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGCURVETOCUBICSMOOTHABS \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); \
  NS_IMETHOD GetX2(float *aX2); \
  NS_IMETHOD SetX2(float aX2); \
  NS_IMETHOD GetY2(float *aY2); \
  NS_IMETHOD SetY2(float aY2); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGCURVETOCUBICSMOOTHABS(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } \
  NS_IMETHOD GetX2(float *aX2) { return _to GetX2(aX2); } \
  NS_IMETHOD SetX2(float aX2) { return _to SetX2(aX2); } \
  NS_IMETHOD GetY2(float *aY2) { return _to GetY2(aY2); } \
  NS_IMETHOD SetY2(float aY2) { return _to SetY2(aY2); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGCURVETOCUBICSMOOTHABS(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } \
  NS_IMETHOD GetX2(float *aX2) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX2(aX2); } \
  NS_IMETHOD SetX2(float aX2) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX2(aX2); } \
  NS_IMETHOD GetY2(float *aY2) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY2(aY2); } \
  NS_IMETHOD SetY2(float aY2) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY2(aY2); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegCurvetoCubicSmoothAbs : public nsIDOMSVGPathSegCurvetoCubicSmoothAbs
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGCURVETOCUBICSMOOTHABS

  nsDOMSVGPathSegCurvetoCubicSmoothAbs();

private:
  ~nsDOMSVGPathSegCurvetoCubicSmoothAbs();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegCurvetoCubicSmoothAbs, nsIDOMSVGPathSegCurvetoCubicSmoothAbs)

nsDOMSVGPathSegCurvetoCubicSmoothAbs::nsDOMSVGPathSegCurvetoCubicSmoothAbs()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegCurvetoCubicSmoothAbs::~nsDOMSVGPathSegCurvetoCubicSmoothAbs()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothAbs::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothAbs::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothAbs::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothAbs::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float x2; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothAbs::GetX2(float *aX2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothAbs::SetX2(float aX2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y2; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothAbs::GetY2(float *aY2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothAbs::SetY2(float aY2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegCurvetoCubicSmoothRel */
#define NS_IDOMSVGPATHSEGCURVETOCUBICSMOOTHREL_IID_STR "5860bccd-f86b-47f8-86c1-cb1245b6a8e1"

#define NS_IDOMSVGPATHSEGCURVETOCUBICSMOOTHREL_IID \
  {0x5860bccd, 0xf86b, 0x47f8, \
    { 0x86, 0xc1, 0xcb, 0x12, 0x45, 0xb6, 0xa8, 0xe1 }}

class NS_NO_VTABLE nsIDOMSVGPathSegCurvetoCubicSmoothRel : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGCURVETOCUBICSMOOTHREL_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

  /* attribute float x2; */
  NS_IMETHOD GetX2(float *aX2) = 0;
  NS_IMETHOD SetX2(float aX2) = 0;

  /* attribute float y2; */
  NS_IMETHOD GetY2(float *aY2) = 0;
  NS_IMETHOD SetY2(float aY2) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGCURVETOCUBICSMOOTHREL \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); \
  NS_IMETHOD GetX2(float *aX2); \
  NS_IMETHOD SetX2(float aX2); \
  NS_IMETHOD GetY2(float *aY2); \
  NS_IMETHOD SetY2(float aY2); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGCURVETOCUBICSMOOTHREL(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } \
  NS_IMETHOD GetX2(float *aX2) { return _to GetX2(aX2); } \
  NS_IMETHOD SetX2(float aX2) { return _to SetX2(aX2); } \
  NS_IMETHOD GetY2(float *aY2) { return _to GetY2(aY2); } \
  NS_IMETHOD SetY2(float aY2) { return _to SetY2(aY2); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGCURVETOCUBICSMOOTHREL(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } \
  NS_IMETHOD GetX2(float *aX2) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX2(aX2); } \
  NS_IMETHOD SetX2(float aX2) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX2(aX2); } \
  NS_IMETHOD GetY2(float *aY2) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY2(aY2); } \
  NS_IMETHOD SetY2(float aY2) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY2(aY2); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegCurvetoCubicSmoothRel : public nsIDOMSVGPathSegCurvetoCubicSmoothRel
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGCURVETOCUBICSMOOTHREL

  nsDOMSVGPathSegCurvetoCubicSmoothRel();

private:
  ~nsDOMSVGPathSegCurvetoCubicSmoothRel();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegCurvetoCubicSmoothRel, nsIDOMSVGPathSegCurvetoCubicSmoothRel)

nsDOMSVGPathSegCurvetoCubicSmoothRel::nsDOMSVGPathSegCurvetoCubicSmoothRel()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegCurvetoCubicSmoothRel::~nsDOMSVGPathSegCurvetoCubicSmoothRel()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothRel::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothRel::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothRel::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothRel::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float x2; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothRel::GetX2(float *aX2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothRel::SetX2(float aX2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y2; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothRel::GetY2(float *aY2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoCubicSmoothRel::SetY2(float aY2)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegCurvetoQuadraticSmoothAbs */
#define NS_IDOMSVGPATHSEGCURVETOQUADRATICSMOOTHABS_IID_STR "3ce86063-0a35-48ec-b372-f198b7d04755"

#define NS_IDOMSVGPATHSEGCURVETOQUADRATICSMOOTHABS_IID \
  {0x3ce86063, 0x0a35, 0x48ec, \
    { 0xb3, 0x72, 0xf1, 0x98, 0xb7, 0xd0, 0x47, 0x55 }}

class NS_NO_VTABLE nsIDOMSVGPathSegCurvetoQuadraticSmoothAbs : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGCURVETOQUADRATICSMOOTHABS_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGCURVETOQUADRATICSMOOTHABS \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGCURVETOQUADRATICSMOOTHABS(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGCURVETOQUADRATICSMOOTHABS(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegCurvetoQuadraticSmoothAbs : public nsIDOMSVGPathSegCurvetoQuadraticSmoothAbs
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGCURVETOQUADRATICSMOOTHABS

  nsDOMSVGPathSegCurvetoQuadraticSmoothAbs();

private:
  ~nsDOMSVGPathSegCurvetoQuadraticSmoothAbs();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegCurvetoQuadraticSmoothAbs, nsIDOMSVGPathSegCurvetoQuadraticSmoothAbs)

nsDOMSVGPathSegCurvetoQuadraticSmoothAbs::nsDOMSVGPathSegCurvetoQuadraticSmoothAbs()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegCurvetoQuadraticSmoothAbs::~nsDOMSVGPathSegCurvetoQuadraticSmoothAbs()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticSmoothAbs::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticSmoothAbs::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticSmoothAbs::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticSmoothAbs::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMSVGPathSegCurvetoQuadraticSmoothRel */
#define NS_IDOMSVGPATHSEGCURVETOQUADRATICSMOOTHREL_IID_STR "5c0e4d25-a9f1-4aab-936c-2b61ed6c085f"

#define NS_IDOMSVGPATHSEGCURVETOQUADRATICSMOOTHREL_IID \
  {0x5c0e4d25, 0xa9f1, 0x4aab, \
    { 0x93, 0x6c, 0x2b, 0x61, 0xed, 0x6c, 0x08, 0x5f }}

class NS_NO_VTABLE nsIDOMSVGPathSegCurvetoQuadraticSmoothRel : public nsIDOMSVGPathSeg {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMSVGPATHSEGCURVETOQUADRATICSMOOTHREL_IID)

  /* attribute float x; */
  NS_IMETHOD GetX(float *aX) = 0;
  NS_IMETHOD SetX(float aX) = 0;

  /* attribute float y; */
  NS_IMETHOD GetY(float *aY) = 0;
  NS_IMETHOD SetY(float aY) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSVGPATHSEGCURVETOQUADRATICSMOOTHREL \
  NS_IMETHOD GetX(float *aX); \
  NS_IMETHOD SetX(float aX); \
  NS_IMETHOD GetY(float *aY); \
  NS_IMETHOD SetY(float aY); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSVGPATHSEGCURVETOQUADRATICSMOOTHREL(_to) \
  NS_IMETHOD GetX(float *aX) { return _to GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return _to SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return _to GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return _to SetY(aY); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSVGPATHSEGCURVETOQUADRATICSMOOTHREL(_to) \
  NS_IMETHOD GetX(float *aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetX(aX); } \
  NS_IMETHOD SetX(float aX) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetX(aX); } \
  NS_IMETHOD GetY(float *aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetY(aY); } \
  NS_IMETHOD SetY(float aY) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetY(aY); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMSVGPathSegCurvetoQuadraticSmoothRel : public nsIDOMSVGPathSegCurvetoQuadraticSmoothRel
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSVGPATHSEGCURVETOQUADRATICSMOOTHREL

  nsDOMSVGPathSegCurvetoQuadraticSmoothRel();

private:
  ~nsDOMSVGPathSegCurvetoQuadraticSmoothRel();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMSVGPathSegCurvetoQuadraticSmoothRel, nsIDOMSVGPathSegCurvetoQuadraticSmoothRel)

nsDOMSVGPathSegCurvetoQuadraticSmoothRel::nsDOMSVGPathSegCurvetoQuadraticSmoothRel()
{
  /* member initializers and constructor code */
}

nsDOMSVGPathSegCurvetoQuadraticSmoothRel::~nsDOMSVGPathSegCurvetoQuadraticSmoothRel()
{
  /* destructor code */
}

/* attribute float x; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticSmoothRel::GetX(float *aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticSmoothRel::SetX(float aX)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute float y; */
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticSmoothRel::GetY(float *aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMSVGPathSegCurvetoQuadraticSmoothRel::SetY(float aY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMSVGPathSeg_h__ */
