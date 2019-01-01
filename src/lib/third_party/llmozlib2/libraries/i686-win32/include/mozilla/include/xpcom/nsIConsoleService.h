/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/base/nsIConsoleService.idl
 */

#ifndef __gen_nsIConsoleService_h__
#define __gen_nsIConsoleService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsIConsoleListener_h__
#include "nsIConsoleListener.h"
#endif

#ifndef __gen_nsIConsoleMessage_h__
#include "nsIConsoleMessage.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIConsoleService */
#define NS_ICONSOLESERVICE_IID_STR "a647f184-1dd1-11b2-a9d1-8537b201161b"

#define NS_ICONSOLESERVICE_IID \
  {0xa647f184, 0x1dd1, 0x11b2, \
    { 0xa9, 0xd1, 0x85, 0x37, 0xb2, 0x01, 0x16, 0x1b }}

class NS_NO_VTABLE nsIConsoleService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICONSOLESERVICE_IID)

  /* void logMessage (in nsIConsoleMessage message); */
  NS_IMETHOD LogMessage(nsIConsoleMessage *message) = 0;

  /**
     * Convenience method for logging simple messages.
     */
  /* void logStringMessage (in wstring message); */
  NS_IMETHOD LogStringMessage(const PRUnichar *message) = 0;

  /**
     * Get an array of all the messages logged so far.  If no messages
     * are logged, this function will return a count of 0, but still
     * will allocate one word for messages, so as to show up as a
     * 0-length array when called from script.
     */
  /* void getMessageArray ([array, size_is (count)] out nsIConsoleMessage messages, out PRUint32 count); */
  NS_IMETHOD GetMessageArray(nsIConsoleMessage ***messages, PRUint32 *count) = 0;

  /**
     * To guard against stack overflows from listeners that could log
     * messages (it's easy to do this inadvertently from listeners
     * implemented in JavaScript), we don't call any listeners when
     * another error is already being logged.
     */
  /* void registerListener (in nsIConsoleListener listener); */
  NS_IMETHOD RegisterListener(nsIConsoleListener *listener) = 0;

  /**
     * Each registered listener should also be unregistered.
     */
  /* void unregisterListener (in nsIConsoleListener listener); */
  NS_IMETHOD UnregisterListener(nsIConsoleListener *listener) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICONSOLESERVICE \
  NS_IMETHOD LogMessage(nsIConsoleMessage *message); \
  NS_IMETHOD LogStringMessage(const PRUnichar *message); \
  NS_IMETHOD GetMessageArray(nsIConsoleMessage ***messages, PRUint32 *count); \
  NS_IMETHOD RegisterListener(nsIConsoleListener *listener); \
  NS_IMETHOD UnregisterListener(nsIConsoleListener *listener); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICONSOLESERVICE(_to) \
  NS_IMETHOD LogMessage(nsIConsoleMessage *message) { return _to LogMessage(message); } \
  NS_IMETHOD LogStringMessage(const PRUnichar *message) { return _to LogStringMessage(message); } \
  NS_IMETHOD GetMessageArray(nsIConsoleMessage ***messages, PRUint32 *count) { return _to GetMessageArray(messages, count); } \
  NS_IMETHOD RegisterListener(nsIConsoleListener *listener) { return _to RegisterListener(listener); } \
  NS_IMETHOD UnregisterListener(nsIConsoleListener *listener) { return _to UnregisterListener(listener); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICONSOLESERVICE(_to) \
  NS_IMETHOD LogMessage(nsIConsoleMessage *message) { return !_to ? NS_ERROR_NULL_POINTER : _to->LogMessage(message); } \
  NS_IMETHOD LogStringMessage(const PRUnichar *message) { return !_to ? NS_ERROR_NULL_POINTER : _to->LogStringMessage(message); } \
  NS_IMETHOD GetMessageArray(nsIConsoleMessage ***messages, PRUint32 *count) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMessageArray(messages, count); } \
  NS_IMETHOD RegisterListener(nsIConsoleListener *listener) { return !_to ? NS_ERROR_NULL_POINTER : _to->RegisterListener(listener); } \
  NS_IMETHOD UnregisterListener(nsIConsoleListener *listener) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnregisterListener(listener); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsConsoleService : public nsIConsoleService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICONSOLESERVICE

  nsConsoleService();

private:
  ~nsConsoleService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsConsoleService, nsIConsoleService)

nsConsoleService::nsConsoleService()
{
  /* member initializers and constructor code */
}

nsConsoleService::~nsConsoleService()
{
  /* destructor code */
}

/* void logMessage (in nsIConsoleMessage message); */
NS_IMETHODIMP nsConsoleService::LogMessage(nsIConsoleMessage *message)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void logStringMessage (in wstring message); */
NS_IMETHODIMP nsConsoleService::LogStringMessage(const PRUnichar *message)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getMessageArray ([array, size_is (count)] out nsIConsoleMessage messages, out PRUint32 count); */
NS_IMETHODIMP nsConsoleService::GetMessageArray(nsIConsoleMessage ***messages, PRUint32 *count)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void registerListener (in nsIConsoleListener listener); */
NS_IMETHODIMP nsConsoleService::RegisterListener(nsIConsoleListener *listener)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unregisterListener (in nsIConsoleListener listener); */
NS_IMETHODIMP nsConsoleService::UnregisterListener(nsIConsoleListener *listener)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIConsoleService_MOZILLA_1_8_BRANCH */
#define NS_ICONSOLESERVICE_MOZILLA_1_8_BRANCH_IID_STR "3c3f3e30-ebd5-11da-8ad9-0800200c9a66"

#define NS_ICONSOLESERVICE_MOZILLA_1_8_BRANCH_IID \
  {0x3c3f3e30, 0xebd5, 0x11da, \
    { 0x8a, 0xd9, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66 }}

/**
 * Temporary interface for Gecko 1.8.1.
 *
 * @status TEMPORARY
 */
class NS_NO_VTABLE nsIConsoleService_MOZILLA_1_8_BRANCH : public nsIConsoleService {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICONSOLESERVICE_MOZILLA_1_8_BRANCH_IID)

  /**
     * Clear the message buffer (e.g. for privacy reasons).
     */
  /* void reset (); */
  NS_IMETHOD Reset(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICONSOLESERVICE_MOZILLA_1_8_BRANCH \
  NS_IMETHOD Reset(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICONSOLESERVICE_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD Reset(void) { return _to Reset(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICONSOLESERVICE_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD Reset(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Reset(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsConsoleService_MOZILLA_1_8_BRANCH : public nsIConsoleService_MOZILLA_1_8_BRANCH
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICONSOLESERVICE_MOZILLA_1_8_BRANCH

  nsConsoleService_MOZILLA_1_8_BRANCH();

private:
  ~nsConsoleService_MOZILLA_1_8_BRANCH();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsConsoleService_MOZILLA_1_8_BRANCH, nsIConsoleService_MOZILLA_1_8_BRANCH)

nsConsoleService_MOZILLA_1_8_BRANCH::nsConsoleService_MOZILLA_1_8_BRANCH()
{
  /* member initializers and constructor code */
}

nsConsoleService_MOZILLA_1_8_BRANCH::~nsConsoleService_MOZILLA_1_8_BRANCH()
{
  /* destructor code */
}

/* void reset (); */
NS_IMETHODIMP nsConsoleService_MOZILLA_1_8_BRANCH::Reset()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

#define NS_CONSOLESERVICE_CLASSNAME "Console Service"
#define NS_CONSOLESERVICE_CID \
{ 0x7e3ff85c, 0x1dd2, 0x11b2, { 0x8d, 0x4b, 0xeb, 0x45, 0x2c, 0xb0, 0xff, 0x40 }}
#define NS_CONSOLESERVICE_CONTRACTID "@mozilla.org/consoleservice;1"

#endif /* __gen_nsIConsoleService_h__ */
