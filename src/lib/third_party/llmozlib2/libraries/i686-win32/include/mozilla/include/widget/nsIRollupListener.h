/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/widget/public/nsIRollupListener.idl
 */

#ifndef __gen_nsIRollupListener_h__
#define __gen_nsIRollupListener_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIWidget; /* forward declaration */


/* starting interface:    nsIRollupListener */
#define NS_IROLLUPLISTENER_IID_STR "23c2ba03-6c76-11d3-96ed-0060b0fb9956"

#define NS_IROLLUPLISTENER_IID \
  {0x23c2ba03, 0x6c76, 0x11d3, \
    { 0x96, 0xed, 0x00, 0x60, 0xb0, 0xfb, 0x99, 0x56 }}

class NS_NO_VTABLE nsIRollupListener : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IROLLUPLISTENER_IID)

  /**
   * Notifies the object to rollup
   * @result NS_Ok if no errors
   */
  /* void Rollup (); */
  NS_IMETHOD Rollup(void) = 0;

  /**
   * Asks the RollupListener if it should rollup on mousevents
   * @result NS_Ok if no errors
   */
  /* void ShouldRollupOnMouseWheelEvent (out PRBool aShould); */
  NS_IMETHOD ShouldRollupOnMouseWheelEvent(PRBool *aShould) = 0;

  /**
   * Asks the RollupListener if it should rollup on mouse activate, eg. X-Mouse
   * @result NS_Ok if no errors
   */
  /* void ShouldRollupOnMouseActivate (out PRBool aShould); */
  NS_IMETHOD ShouldRollupOnMouseActivate(PRBool *aShould) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIROLLUPLISTENER \
  NS_IMETHOD Rollup(void); \
  NS_IMETHOD ShouldRollupOnMouseWheelEvent(PRBool *aShould); \
  NS_IMETHOD ShouldRollupOnMouseActivate(PRBool *aShould); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIROLLUPLISTENER(_to) \
  NS_IMETHOD Rollup(void) { return _to Rollup(); } \
  NS_IMETHOD ShouldRollupOnMouseWheelEvent(PRBool *aShould) { return _to ShouldRollupOnMouseWheelEvent(aShould); } \
  NS_IMETHOD ShouldRollupOnMouseActivate(PRBool *aShould) { return _to ShouldRollupOnMouseActivate(aShould); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIROLLUPLISTENER(_to) \
  NS_IMETHOD Rollup(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Rollup(); } \
  NS_IMETHOD ShouldRollupOnMouseWheelEvent(PRBool *aShould) { return !_to ? NS_ERROR_NULL_POINTER : _to->ShouldRollupOnMouseWheelEvent(aShould); } \
  NS_IMETHOD ShouldRollupOnMouseActivate(PRBool *aShould) { return !_to ? NS_ERROR_NULL_POINTER : _to->ShouldRollupOnMouseActivate(aShould); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsRollupListener : public nsIRollupListener
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIROLLUPLISTENER

  nsRollupListener();

private:
  ~nsRollupListener();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsRollupListener, nsIRollupListener)

nsRollupListener::nsRollupListener()
{
  /* member initializers and constructor code */
}

nsRollupListener::~nsRollupListener()
{
  /* destructor code */
}

/* void Rollup (); */
NS_IMETHODIMP nsRollupListener::Rollup()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void ShouldRollupOnMouseWheelEvent (out PRBool aShould); */
NS_IMETHODIMP nsRollupListener::ShouldRollupOnMouseWheelEvent(PRBool *aShould)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void ShouldRollupOnMouseActivate (out PRBool aShould); */
NS_IMETHODIMP nsRollupListener::ShouldRollupOnMouseActivate(PRBool *aShould)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIRollupListener_h__ */
