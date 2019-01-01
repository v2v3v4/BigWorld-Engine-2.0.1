/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/widget/public/nsISound.idl
 */

#ifndef __gen_nsISound_h__
#define __gen_nsISound_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIURL; /* forward declaration */


/* starting interface:    nsISound */
#define NS_ISOUND_IID_STR "b148eed1-236d-11d3-b35c-00a0cc3c1cde"

#define NS_ISOUND_IID \
  {0xb148eed1, 0x236d, 0x11d3, \
    { 0xb3, 0x5c, 0x00, 0xa0, 0xcc, 0x3c, 0x1c, 0xde }}

class NS_NO_VTABLE nsISound : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISOUND_IID)

  /* void play (in nsIURL aURL); */
  NS_IMETHOD Play(nsIURL *aURL) = 0;

  /**
   * for playing system sounds
   */
  /* void playSystemSound (in string soundAlias); */
  NS_IMETHOD PlaySystemSound(const char *soundAlias) = 0;

  /* void beep (); */
  NS_IMETHOD Beep(void) = 0;

  /**
    * Not strictly necessary, but avoids delay before first sound.
    * The various methods on nsISound call Init() if they need to.
	*/
  /* void init (); */
  NS_IMETHOD Init(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISOUND \
  NS_IMETHOD Play(nsIURL *aURL); \
  NS_IMETHOD PlaySystemSound(const char *soundAlias); \
  NS_IMETHOD Beep(void); \
  NS_IMETHOD Init(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISOUND(_to) \
  NS_IMETHOD Play(nsIURL *aURL) { return _to Play(aURL); } \
  NS_IMETHOD PlaySystemSound(const char *soundAlias) { return _to PlaySystemSound(soundAlias); } \
  NS_IMETHOD Beep(void) { return _to Beep(); } \
  NS_IMETHOD Init(void) { return _to Init(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISOUND(_to) \
  NS_IMETHOD Play(nsIURL *aURL) { return !_to ? NS_ERROR_NULL_POINTER : _to->Play(aURL); } \
  NS_IMETHOD PlaySystemSound(const char *soundAlias) { return !_to ? NS_ERROR_NULL_POINTER : _to->PlaySystemSound(soundAlias); } \
  NS_IMETHOD Beep(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Beep(); } \
  NS_IMETHOD Init(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSound : public nsISound
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISOUND

  nsSound();

private:
  ~nsSound();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSound, nsISound)

nsSound::nsSound()
{
  /* member initializers and constructor code */
}

nsSound::~nsSound()
{
  /* destructor code */
}

/* void play (in nsIURL aURL); */
NS_IMETHODIMP nsSound::Play(nsIURL *aURL)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void playSystemSound (in string soundAlias); */
NS_IMETHODIMP nsSound::PlaySystemSound(const char *soundAlias)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void beep (); */
NS_IMETHODIMP nsSound::Beep()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void init (); */
NS_IMETHODIMP nsSound::Init()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISound_h__ */
