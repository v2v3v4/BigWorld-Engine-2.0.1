/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/widget/public/nsIAppShell.idl
 */

#ifndef __gen_nsIAppShell_h__
#define __gen_nsIAppShell_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIEventQueue;
/**
 * Flags for the getNativeData function.
 * See GetNativeData()
 */
#define NS_NATIVE_SHELL   0
class nsIWidget;

/* starting interface:    nsIAppShell */
#define NS_IAPPSHELL_IID_STR "a0757c31-eeac-11d1-9ec1-00aa002fb821"

#define NS_IAPPSHELL_IID \
  {0xa0757c31, 0xeeac, 0x11d1, \
    { 0x9e, 0xc1, 0x00, 0xaa, 0x00, 0x2f, 0xb8, 0x21 }}

class NS_NO_VTABLE nsIAppShell : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IAPPSHELL_IID)

  /**
  * Creates an application shell
  */
  /* void Create (inout int argc, inout string argv); */
  NS_IMETHOD Create(int *argc, char **argv) = 0;

  /**
  * Enter an event loop.
  * Don't leave until application exits.
  */
  /* void Run (); */
  NS_IMETHOD Run(void) = 0;

  /**
  * Prepare to process events. 
  */
  /* void Spinup (); */
  NS_IMETHOD Spinup(void) = 0;

  /**
  * Prepare to stop processing events.  
  */
  /* void Spindown (); */
  NS_IMETHOD Spindown(void) = 0;

  /**
  * An event queue has been created or destroyed. Hook or unhook it from
  * your system, as necessary.
  * @param aQueue the queue in question
  * @param aListen PR_TRUE for a new queue wanting hooking up. PR_FALSE
  *                for a queue wanting to be unhooked.
  */
  /* void ListenToEventQueue (in nsIEventQueue aQueue, in PRBool aListen); */
  NS_IMETHOD ListenToEventQueue(nsIEventQueue * aQueue, PRBool aListen) = 0;

  /**
  * After event dispatch execute app specific code
  */
  /* void GetNativeEvent (in PRBoolRef aRealEvent, in voidPtrRef aEvent); */
  NS_IMETHOD GetNativeEvent(PRBool & aRealEvent, void * & aEvent) = 0;

  /**
  * After event dispatch execute app specific code
  */
  /* void DispatchNativeEvent (in PRBool aRealEvent, in voidPtr aEvent); */
  NS_IMETHOD DispatchNativeEvent(PRBool aRealEvent, void * aEvent) = 0;

  /**
  * Exit the handle event loop
  */
  /* void Exit (); */
  NS_IMETHOD Exit(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIAPPSHELL \
  NS_IMETHOD Create(int *argc, char **argv); \
  NS_IMETHOD Run(void); \
  NS_IMETHOD Spinup(void); \
  NS_IMETHOD Spindown(void); \
  NS_IMETHOD ListenToEventQueue(nsIEventQueue * aQueue, PRBool aListen); \
  NS_IMETHOD GetNativeEvent(PRBool & aRealEvent, void * & aEvent); \
  NS_IMETHOD DispatchNativeEvent(PRBool aRealEvent, void * aEvent); \
  NS_IMETHOD Exit(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIAPPSHELL(_to) \
  NS_IMETHOD Create(int *argc, char **argv) { return _to Create(argc, argv); } \
  NS_IMETHOD Run(void) { return _to Run(); } \
  NS_IMETHOD Spinup(void) { return _to Spinup(); } \
  NS_IMETHOD Spindown(void) { return _to Spindown(); } \
  NS_IMETHOD ListenToEventQueue(nsIEventQueue * aQueue, PRBool aListen) { return _to ListenToEventQueue(aQueue, aListen); } \
  NS_IMETHOD GetNativeEvent(PRBool & aRealEvent, void * & aEvent) { return _to GetNativeEvent(aRealEvent, aEvent); } \
  NS_IMETHOD DispatchNativeEvent(PRBool aRealEvent, void * aEvent) { return _to DispatchNativeEvent(aRealEvent, aEvent); } \
  NS_IMETHOD Exit(void) { return _to Exit(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIAPPSHELL(_to) \
  NS_IMETHOD Create(int *argc, char **argv) { return !_to ? NS_ERROR_NULL_POINTER : _to->Create(argc, argv); } \
  NS_IMETHOD Run(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Run(); } \
  NS_IMETHOD Spinup(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Spinup(); } \
  NS_IMETHOD Spindown(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Spindown(); } \
  NS_IMETHOD ListenToEventQueue(nsIEventQueue * aQueue, PRBool aListen) { return !_to ? NS_ERROR_NULL_POINTER : _to->ListenToEventQueue(aQueue, aListen); } \
  NS_IMETHOD GetNativeEvent(PRBool & aRealEvent, void * & aEvent) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNativeEvent(aRealEvent, aEvent); } \
  NS_IMETHOD DispatchNativeEvent(PRBool aRealEvent, void * aEvent) { return !_to ? NS_ERROR_NULL_POINTER : _to->DispatchNativeEvent(aRealEvent, aEvent); } \
  NS_IMETHOD Exit(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Exit(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsAppShell : public nsIAppShell
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIAPPSHELL

  nsAppShell();

private:
  ~nsAppShell();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsAppShell, nsIAppShell)

nsAppShell::nsAppShell()
{
  /* member initializers and constructor code */
}

nsAppShell::~nsAppShell()
{
  /* destructor code */
}

/* void Create (inout int argc, inout string argv); */
NS_IMETHODIMP nsAppShell::Create(int *argc, char **argv)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void Run (); */
NS_IMETHODIMP nsAppShell::Run()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void Spinup (); */
NS_IMETHODIMP nsAppShell::Spinup()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void Spindown (); */
NS_IMETHODIMP nsAppShell::Spindown()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void ListenToEventQueue (in nsIEventQueue aQueue, in PRBool aListen); */
NS_IMETHODIMP nsAppShell::ListenToEventQueue(nsIEventQueue * aQueue, PRBool aListen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void GetNativeEvent (in PRBoolRef aRealEvent, in voidPtrRef aEvent); */
NS_IMETHODIMP nsAppShell::GetNativeEvent(PRBool & aRealEvent, void * & aEvent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void DispatchNativeEvent (in PRBool aRealEvent, in voidPtr aEvent); */
NS_IMETHODIMP nsAppShell::DispatchNativeEvent(PRBool aRealEvent, void * aEvent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void Exit (); */
NS_IMETHODIMP nsAppShell::Exit()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIAppShell_h__ */
