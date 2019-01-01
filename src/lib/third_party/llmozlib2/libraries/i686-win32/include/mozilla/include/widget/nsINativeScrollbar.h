/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/widget/public/nsINativeScrollbar.idl
 */

#ifndef __gen_nsINativeScrollbar_h__
#define __gen_nsINativeScrollbar_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsPresContext; /* forward declaration */

class nsIFrame; /* forward declaration */

class nsIContent; /* forward declaration */

class nsIScrollbarMediator; /* forward declaration */


/* starting interface:    nsINativeScrollbar */
#define NS_INATIVESCROLLBAR_IID_STR "b77380bc-610b-49e3-8df7-18cc946285c5"

#define NS_INATIVESCROLLBAR_IID \
  {0xb77380bc, 0x610b, 0x49e3, \
    { 0x8d, 0xf7, 0x18, 0xcc, 0x94, 0x62, 0x85, 0xc5 }}

class NS_NO_VTABLE nsINativeScrollbar : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_INATIVESCROLLBAR_IID)

  /* void setContent (in nsIContent content, in nsISupports scrollbar, in nsIScrollbarMediator mediator); */
  NS_IMETHOD SetContent(nsIContent *content, nsISupports *scrollbar, nsIScrollbarMediator *mediator) = 0;

  /* readonly attribute long narrowSize; */
  NS_IMETHOD GetNarrowSize(PRInt32 *aNarrowSize) = 0;

  /* attribute unsigned long position; */
  NS_IMETHOD GetPosition(PRUint32 *aPosition) = 0;
  NS_IMETHOD SetPosition(PRUint32 aPosition) = 0;

  /* attribute unsigned long maxRange; */
  NS_IMETHOD GetMaxRange(PRUint32 *aMaxRange) = 0;
  NS_IMETHOD SetMaxRange(PRUint32 aMaxRange) = 0;

  /* attribute unsigned long lineIncrement; */
  NS_IMETHOD GetLineIncrement(PRUint32 *aLineIncrement) = 0;
  NS_IMETHOD SetLineIncrement(PRUint32 aLineIncrement) = 0;

  /* attribute unsigned long viewSize; */
  NS_IMETHOD GetViewSize(PRUint32 *aViewSize) = 0;
  NS_IMETHOD SetViewSize(PRUint32 aViewSize) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSINATIVESCROLLBAR \
  NS_IMETHOD SetContent(nsIContent *content, nsISupports *scrollbar, nsIScrollbarMediator *mediator); \
  NS_IMETHOD GetNarrowSize(PRInt32 *aNarrowSize); \
  NS_IMETHOD GetPosition(PRUint32 *aPosition); \
  NS_IMETHOD SetPosition(PRUint32 aPosition); \
  NS_IMETHOD GetMaxRange(PRUint32 *aMaxRange); \
  NS_IMETHOD SetMaxRange(PRUint32 aMaxRange); \
  NS_IMETHOD GetLineIncrement(PRUint32 *aLineIncrement); \
  NS_IMETHOD SetLineIncrement(PRUint32 aLineIncrement); \
  NS_IMETHOD GetViewSize(PRUint32 *aViewSize); \
  NS_IMETHOD SetViewSize(PRUint32 aViewSize); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSINATIVESCROLLBAR(_to) \
  NS_IMETHOD SetContent(nsIContent *content, nsISupports *scrollbar, nsIScrollbarMediator *mediator) { return _to SetContent(content, scrollbar, mediator); } \
  NS_IMETHOD GetNarrowSize(PRInt32 *aNarrowSize) { return _to GetNarrowSize(aNarrowSize); } \
  NS_IMETHOD GetPosition(PRUint32 *aPosition) { return _to GetPosition(aPosition); } \
  NS_IMETHOD SetPosition(PRUint32 aPosition) { return _to SetPosition(aPosition); } \
  NS_IMETHOD GetMaxRange(PRUint32 *aMaxRange) { return _to GetMaxRange(aMaxRange); } \
  NS_IMETHOD SetMaxRange(PRUint32 aMaxRange) { return _to SetMaxRange(aMaxRange); } \
  NS_IMETHOD GetLineIncrement(PRUint32 *aLineIncrement) { return _to GetLineIncrement(aLineIncrement); } \
  NS_IMETHOD SetLineIncrement(PRUint32 aLineIncrement) { return _to SetLineIncrement(aLineIncrement); } \
  NS_IMETHOD GetViewSize(PRUint32 *aViewSize) { return _to GetViewSize(aViewSize); } \
  NS_IMETHOD SetViewSize(PRUint32 aViewSize) { return _to SetViewSize(aViewSize); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSINATIVESCROLLBAR(_to) \
  NS_IMETHOD SetContent(nsIContent *content, nsISupports *scrollbar, nsIScrollbarMediator *mediator) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetContent(content, scrollbar, mediator); } \
  NS_IMETHOD GetNarrowSize(PRInt32 *aNarrowSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNarrowSize(aNarrowSize); } \
  NS_IMETHOD GetPosition(PRUint32 *aPosition) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPosition(aPosition); } \
  NS_IMETHOD SetPosition(PRUint32 aPosition) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPosition(aPosition); } \
  NS_IMETHOD GetMaxRange(PRUint32 *aMaxRange) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMaxRange(aMaxRange); } \
  NS_IMETHOD SetMaxRange(PRUint32 aMaxRange) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMaxRange(aMaxRange); } \
  NS_IMETHOD GetLineIncrement(PRUint32 *aLineIncrement) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLineIncrement(aLineIncrement); } \
  NS_IMETHOD SetLineIncrement(PRUint32 aLineIncrement) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetLineIncrement(aLineIncrement); } \
  NS_IMETHOD GetViewSize(PRUint32 *aViewSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetViewSize(aViewSize); } \
  NS_IMETHOD SetViewSize(PRUint32 aViewSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetViewSize(aViewSize); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsNativeScrollbar : public nsINativeScrollbar
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSINATIVESCROLLBAR

  nsNativeScrollbar();

private:
  ~nsNativeScrollbar();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsNativeScrollbar, nsINativeScrollbar)

nsNativeScrollbar::nsNativeScrollbar()
{
  /* member initializers and constructor code */
}

nsNativeScrollbar::~nsNativeScrollbar()
{
  /* destructor code */
}

/* void setContent (in nsIContent content, in nsISupports scrollbar, in nsIScrollbarMediator mediator); */
NS_IMETHODIMP nsNativeScrollbar::SetContent(nsIContent *content, nsISupports *scrollbar, nsIScrollbarMediator *mediator)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long narrowSize; */
NS_IMETHODIMP nsNativeScrollbar::GetNarrowSize(PRInt32 *aNarrowSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute unsigned long position; */
NS_IMETHODIMP nsNativeScrollbar::GetPosition(PRUint32 *aPosition)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsNativeScrollbar::SetPosition(PRUint32 aPosition)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute unsigned long maxRange; */
NS_IMETHODIMP nsNativeScrollbar::GetMaxRange(PRUint32 *aMaxRange)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsNativeScrollbar::SetMaxRange(PRUint32 aMaxRange)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute unsigned long lineIncrement; */
NS_IMETHODIMP nsNativeScrollbar::GetLineIncrement(PRUint32 *aLineIncrement)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsNativeScrollbar::SetLineIncrement(PRUint32 aLineIncrement)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute unsigned long viewSize; */
NS_IMETHODIMP nsNativeScrollbar::GetViewSize(PRUint32 *aViewSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsNativeScrollbar::SetViewSize(PRUint32 aViewSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsINativeScrollbar_h__ */
