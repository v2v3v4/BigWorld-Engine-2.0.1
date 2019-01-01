/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/widget/public/nsIToolkit.idl
 */

#ifndef __gen_nsIToolkit_h__
#define __gen_nsIToolkit_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "prthread.h"
class nsIWidget; /* forward declaration */


/* starting interface:    nsIToolkitObserver */
#define NS_ITOOLKITOBSERVER_IID_STR "792dc3f5-adc0-4ef7-a07d-e0fd9be8cc9f"

#define NS_ITOOLKITOBSERVER_IID \
  {0x792dc3f5, 0xadc0, 0x4ef7, \
    { 0xa0, 0x7d, 0xe0, 0xfd, 0x9b, 0xe8, 0xcc, 0x9f }}

class NS_NO_VTABLE nsIToolkitObserver : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ITOOLKITOBSERVER_IID)

  /**
   * Called after a native widget has been invalidated, either manually
   * via nsIWidget::Invalidate or due to a widget being moved, shown or
   * resized. We pass the coordinates of the rectangle that bounds the
   * area that has been invalidated, relative to the origin of nsIWidget
   * in pixels.
   */
  /* void notifyInvalidated (in nsIWidget aWidget, in long x, in long y, in long width, in long height); */
  NS_IMETHOD NotifyInvalidated(nsIWidget *aWidget, PRInt32 x, PRInt32 y, PRInt32 width, PRInt32 height) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSITOOLKITOBSERVER \
  NS_IMETHOD NotifyInvalidated(nsIWidget *aWidget, PRInt32 x, PRInt32 y, PRInt32 width, PRInt32 height); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSITOOLKITOBSERVER(_to) \
  NS_IMETHOD NotifyInvalidated(nsIWidget *aWidget, PRInt32 x, PRInt32 y, PRInt32 width, PRInt32 height) { return _to NotifyInvalidated(aWidget, x, y, width, height); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSITOOLKITOBSERVER(_to) \
  NS_IMETHOD NotifyInvalidated(nsIWidget *aWidget, PRInt32 x, PRInt32 y, PRInt32 width, PRInt32 height) { return !_to ? NS_ERROR_NULL_POINTER : _to->NotifyInvalidated(aWidget, x, y, width, height); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsToolkitObserver : public nsIToolkitObserver
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSITOOLKITOBSERVER

  nsToolkitObserver();

private:
  ~nsToolkitObserver();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsToolkitObserver, nsIToolkitObserver)

nsToolkitObserver::nsToolkitObserver()
{
  /* member initializers and constructor code */
}

nsToolkitObserver::~nsToolkitObserver()
{
  /* destructor code */
}

/* void notifyInvalidated (in nsIWidget aWidget, in long x, in long y, in long width, in long height); */
NS_IMETHODIMP nsToolkitObserver::NotifyInvalidated(nsIWidget *aWidget, PRInt32 x, PRInt32 y, PRInt32 width, PRInt32 height)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIToolkit */
#define NS_ITOOLKIT_IID_STR "18032bd0-b265-11d1-aa2a-000000000000"

#define NS_ITOOLKIT_IID \
  {0x18032bd0, 0xb265, 0x11d1, \
    { 0xaa, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}

class NS_NO_VTABLE nsIToolkit : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ITOOLKIT_IID)

  /**
   * Initialize this toolkit with aThread. 
   * @param aThread The thread passed in runs the message pump.
   *  NULL can be passed in, in which case a new thread gets created
   *  and a message pump will run in that thread
   *
   */
  /* void Init (in PRThread aThread); */
  NS_IMETHOD Init(PRThread * aThread) = 0;

  /* void addObserver (in nsIToolkitObserver aObserver); */
  NS_IMETHOD AddObserver(nsIToolkitObserver *aObserver) = 0;

  /* void removeObserver (in nsIToolkitObserver aObserver); */
  NS_IMETHOD RemoveObserver(nsIToolkitObserver *aObserver) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSITOOLKIT \
  NS_IMETHOD Init(PRThread * aThread); \
  NS_IMETHOD AddObserver(nsIToolkitObserver *aObserver); \
  NS_IMETHOD RemoveObserver(nsIToolkitObserver *aObserver); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSITOOLKIT(_to) \
  NS_IMETHOD Init(PRThread * aThread) { return _to Init(aThread); } \
  NS_IMETHOD AddObserver(nsIToolkitObserver *aObserver) { return _to AddObserver(aObserver); } \
  NS_IMETHOD RemoveObserver(nsIToolkitObserver *aObserver) { return _to RemoveObserver(aObserver); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSITOOLKIT(_to) \
  NS_IMETHOD Init(PRThread * aThread) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(aThread); } \
  NS_IMETHOD AddObserver(nsIToolkitObserver *aObserver) { return !_to ? NS_ERROR_NULL_POINTER : _to->AddObserver(aObserver); } \
  NS_IMETHOD RemoveObserver(nsIToolkitObserver *aObserver) { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveObserver(aObserver); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsToolkit : public nsIToolkit
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSITOOLKIT

  nsToolkit();

private:
  ~nsToolkit();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsToolkit, nsIToolkit)

nsToolkit::nsToolkit()
{
  /* member initializers and constructor code */
}

nsToolkit::~nsToolkit()
{
  /* destructor code */
}

/* void Init (in PRThread aThread); */
NS_IMETHODIMP nsToolkit::Init(PRThread * aThread)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void addObserver (in nsIToolkitObserver aObserver); */
NS_IMETHODIMP nsToolkit::AddObserver(nsIToolkitObserver *aObserver)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeObserver (in nsIToolkitObserver aObserver); */
NS_IMETHODIMP nsToolkit::RemoveObserver(nsIToolkitObserver *aObserver)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

extern NS_METHOD NS_GetCurrentToolkit(nsIToolkit* *aResult);

#endif /* __gen_nsIToolkit_h__ */
