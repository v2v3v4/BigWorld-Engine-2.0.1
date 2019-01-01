/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsIURLParser.idl
 */

#ifndef __gen_nsIURLParser_h__
#define __gen_nsIURLParser_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIURLParser */
#define NS_IURLPARSER_IID_STR "7281076d-cf37-464a-815e-698235802604"

#define NS_IURLPARSER_IID \
  {0x7281076d, 0xcf37, 0x464a, \
    { 0x81, 0x5e, 0x69, 0x82, 0x35, 0x80, 0x26, 0x04 }}

/**
 * nsIURLParser specifies the interface to an URL parser that attempts to
 * follow the definitions of RFC 2396.
 */
class NS_NO_VTABLE nsIURLParser : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IURLPARSER_IID)

  /**
     * The string to parse in the following methods may be given as a null
     * terminated string, in which case the length argument should be -1.
     *
     * Out parameters of the following methods are all optional (ie. the caller
     * may pass-in a NULL value if the corresponding results are not needed).
     * Signed out parameters may hold a value of -1 if the corresponding result
     * is not part of the string being parsed.
     *
     * The parsing routines attempt to be as forgiving as possible.
     */
/**
     * ParseSpec breaks the URL string up into its 3 major components: a scheme,
     * an authority section (hostname, etc.), and a path.
     *
     * spec = <scheme>://<authority><path>
     */
  /* void parseURL (in string spec, in long specLen, out unsigned long schemePos, out long schemeLen, out unsigned long authorityPos, out long authorityLen, out unsigned long pathPos, out long pathLen); */
  NS_IMETHOD ParseURL(const char *spec, PRInt32 specLen, PRUint32 *schemePos, PRInt32 *schemeLen, PRUint32 *authorityPos, PRInt32 *authorityLen, PRUint32 *pathPos, PRInt32 *pathLen) = 0;

  /**
     * ParseAuthority breaks the authority string up into its 4 components:
     * username, password, hostname, and hostport.
     *
     * auth = <username>:<password>@<hostname>:<port>
     */
  /* void parseAuthority (in string authority, in long authorityLen, out unsigned long usernamePos, out long usernameLen, out unsigned long passwordPos, out long passwordLen, out unsigned long hostnamePos, out long hostnameLen, out long port); */
  NS_IMETHOD ParseAuthority(const char *authority, PRInt32 authorityLen, PRUint32 *usernamePos, PRInt32 *usernameLen, PRUint32 *passwordPos, PRInt32 *passwordLen, PRUint32 *hostnamePos, PRInt32 *hostnameLen, PRInt32 *port) = 0;

  /**
     * userinfo = <username>:<password>
     */
  /* void parseUserInfo (in string userinfo, in long userinfoLen, out unsigned long usernamePos, out long usernameLen, out unsigned long passwordPos, out long passwordLen); */
  NS_IMETHOD ParseUserInfo(const char *userinfo, PRInt32 userinfoLen, PRUint32 *usernamePos, PRInt32 *usernameLen, PRUint32 *passwordPos, PRInt32 *passwordLen) = 0;

  /**
     * serverinfo = <hostname>:<port>
     */
  /* void parseServerInfo (in string serverinfo, in long serverinfoLen, out unsigned long hostnamePos, out long hostnameLen, out long port); */
  NS_IMETHOD ParseServerInfo(const char *serverinfo, PRInt32 serverinfoLen, PRUint32 *hostnamePos, PRInt32 *hostnameLen, PRInt32 *port) = 0;

  /**
     * ParsePath breaks the path string up into its 4 major components: a file path,
     * a param string, a query string, and a reference string.
     *
     * path = <filepath>;<param>?<query>#<ref>
     */
  /* void parsePath (in string path, in long pathLen, out unsigned long filepathPos, out long filepathLen, out unsigned long paramPos, out long paramLen, out unsigned long queryPos, out long queryLen, out unsigned long refPos, out long refLen); */
  NS_IMETHOD ParsePath(const char *path, PRInt32 pathLen, PRUint32 *filepathPos, PRInt32 *filepathLen, PRUint32 *paramPos, PRInt32 *paramLen, PRUint32 *queryPos, PRInt32 *queryLen, PRUint32 *refPos, PRInt32 *refLen) = 0;

  /**
     * ParseFilePath breaks the file path string up into: the directory portion,
     * file base name, and file extension.
     *
     * filepath = <directory><basename>.<extension>
     */
  /* void parseFilePath (in string filepath, in long filepathLen, out unsigned long directoryPos, out long directoryLen, out unsigned long basenamePos, out long basenameLen, out unsigned long extensionPos, out long extensionLen); */
  NS_IMETHOD ParseFilePath(const char *filepath, PRInt32 filepathLen, PRUint32 *directoryPos, PRInt32 *directoryLen, PRUint32 *basenamePos, PRInt32 *basenameLen, PRUint32 *extensionPos, PRInt32 *extensionLen) = 0;

  /**
     * filename = <basename>.<extension>
     */
  /* void parseFileName (in string filename, in long filenameLen, out unsigned long basenamePos, out long basenameLen, out unsigned long extensionPos, out long extensionLen); */
  NS_IMETHOD ParseFileName(const char *filename, PRInt32 filenameLen, PRUint32 *basenamePos, PRInt32 *basenameLen, PRUint32 *extensionPos, PRInt32 *extensionLen) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIURLPARSER \
  NS_IMETHOD ParseURL(const char *spec, PRInt32 specLen, PRUint32 *schemePos, PRInt32 *schemeLen, PRUint32 *authorityPos, PRInt32 *authorityLen, PRUint32 *pathPos, PRInt32 *pathLen); \
  NS_IMETHOD ParseAuthority(const char *authority, PRInt32 authorityLen, PRUint32 *usernamePos, PRInt32 *usernameLen, PRUint32 *passwordPos, PRInt32 *passwordLen, PRUint32 *hostnamePos, PRInt32 *hostnameLen, PRInt32 *port); \
  NS_IMETHOD ParseUserInfo(const char *userinfo, PRInt32 userinfoLen, PRUint32 *usernamePos, PRInt32 *usernameLen, PRUint32 *passwordPos, PRInt32 *passwordLen); \
  NS_IMETHOD ParseServerInfo(const char *serverinfo, PRInt32 serverinfoLen, PRUint32 *hostnamePos, PRInt32 *hostnameLen, PRInt32 *port); \
  NS_IMETHOD ParsePath(const char *path, PRInt32 pathLen, PRUint32 *filepathPos, PRInt32 *filepathLen, PRUint32 *paramPos, PRInt32 *paramLen, PRUint32 *queryPos, PRInt32 *queryLen, PRUint32 *refPos, PRInt32 *refLen); \
  NS_IMETHOD ParseFilePath(const char *filepath, PRInt32 filepathLen, PRUint32 *directoryPos, PRInt32 *directoryLen, PRUint32 *basenamePos, PRInt32 *basenameLen, PRUint32 *extensionPos, PRInt32 *extensionLen); \
  NS_IMETHOD ParseFileName(const char *filename, PRInt32 filenameLen, PRUint32 *basenamePos, PRInt32 *basenameLen, PRUint32 *extensionPos, PRInt32 *extensionLen); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIURLPARSER(_to) \
  NS_IMETHOD ParseURL(const char *spec, PRInt32 specLen, PRUint32 *schemePos, PRInt32 *schemeLen, PRUint32 *authorityPos, PRInt32 *authorityLen, PRUint32 *pathPos, PRInt32 *pathLen) { return _to ParseURL(spec, specLen, schemePos, schemeLen, authorityPos, authorityLen, pathPos, pathLen); } \
  NS_IMETHOD ParseAuthority(const char *authority, PRInt32 authorityLen, PRUint32 *usernamePos, PRInt32 *usernameLen, PRUint32 *passwordPos, PRInt32 *passwordLen, PRUint32 *hostnamePos, PRInt32 *hostnameLen, PRInt32 *port) { return _to ParseAuthority(authority, authorityLen, usernamePos, usernameLen, passwordPos, passwordLen, hostnamePos, hostnameLen, port); } \
  NS_IMETHOD ParseUserInfo(const char *userinfo, PRInt32 userinfoLen, PRUint32 *usernamePos, PRInt32 *usernameLen, PRUint32 *passwordPos, PRInt32 *passwordLen) { return _to ParseUserInfo(userinfo, userinfoLen, usernamePos, usernameLen, passwordPos, passwordLen); } \
  NS_IMETHOD ParseServerInfo(const char *serverinfo, PRInt32 serverinfoLen, PRUint32 *hostnamePos, PRInt32 *hostnameLen, PRInt32 *port) { return _to ParseServerInfo(serverinfo, serverinfoLen, hostnamePos, hostnameLen, port); } \
  NS_IMETHOD ParsePath(const char *path, PRInt32 pathLen, PRUint32 *filepathPos, PRInt32 *filepathLen, PRUint32 *paramPos, PRInt32 *paramLen, PRUint32 *queryPos, PRInt32 *queryLen, PRUint32 *refPos, PRInt32 *refLen) { return _to ParsePath(path, pathLen, filepathPos, filepathLen, paramPos, paramLen, queryPos, queryLen, refPos, refLen); } \
  NS_IMETHOD ParseFilePath(const char *filepath, PRInt32 filepathLen, PRUint32 *directoryPos, PRInt32 *directoryLen, PRUint32 *basenamePos, PRInt32 *basenameLen, PRUint32 *extensionPos, PRInt32 *extensionLen) { return _to ParseFilePath(filepath, filepathLen, directoryPos, directoryLen, basenamePos, basenameLen, extensionPos, extensionLen); } \
  NS_IMETHOD ParseFileName(const char *filename, PRInt32 filenameLen, PRUint32 *basenamePos, PRInt32 *basenameLen, PRUint32 *extensionPos, PRInt32 *extensionLen) { return _to ParseFileName(filename, filenameLen, basenamePos, basenameLen, extensionPos, extensionLen); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIURLPARSER(_to) \
  NS_IMETHOD ParseURL(const char *spec, PRInt32 specLen, PRUint32 *schemePos, PRInt32 *schemeLen, PRUint32 *authorityPos, PRInt32 *authorityLen, PRUint32 *pathPos, PRInt32 *pathLen) { return !_to ? NS_ERROR_NULL_POINTER : _to->ParseURL(spec, specLen, schemePos, schemeLen, authorityPos, authorityLen, pathPos, pathLen); } \
  NS_IMETHOD ParseAuthority(const char *authority, PRInt32 authorityLen, PRUint32 *usernamePos, PRInt32 *usernameLen, PRUint32 *passwordPos, PRInt32 *passwordLen, PRUint32 *hostnamePos, PRInt32 *hostnameLen, PRInt32 *port) { return !_to ? NS_ERROR_NULL_POINTER : _to->ParseAuthority(authority, authorityLen, usernamePos, usernameLen, passwordPos, passwordLen, hostnamePos, hostnameLen, port); } \
  NS_IMETHOD ParseUserInfo(const char *userinfo, PRInt32 userinfoLen, PRUint32 *usernamePos, PRInt32 *usernameLen, PRUint32 *passwordPos, PRInt32 *passwordLen) { return !_to ? NS_ERROR_NULL_POINTER : _to->ParseUserInfo(userinfo, userinfoLen, usernamePos, usernameLen, passwordPos, passwordLen); } \
  NS_IMETHOD ParseServerInfo(const char *serverinfo, PRInt32 serverinfoLen, PRUint32 *hostnamePos, PRInt32 *hostnameLen, PRInt32 *port) { return !_to ? NS_ERROR_NULL_POINTER : _to->ParseServerInfo(serverinfo, serverinfoLen, hostnamePos, hostnameLen, port); } \
  NS_IMETHOD ParsePath(const char *path, PRInt32 pathLen, PRUint32 *filepathPos, PRInt32 *filepathLen, PRUint32 *paramPos, PRInt32 *paramLen, PRUint32 *queryPos, PRInt32 *queryLen, PRUint32 *refPos, PRInt32 *refLen) { return !_to ? NS_ERROR_NULL_POINTER : _to->ParsePath(path, pathLen, filepathPos, filepathLen, paramPos, paramLen, queryPos, queryLen, refPos, refLen); } \
  NS_IMETHOD ParseFilePath(const char *filepath, PRInt32 filepathLen, PRUint32 *directoryPos, PRInt32 *directoryLen, PRUint32 *basenamePos, PRInt32 *basenameLen, PRUint32 *extensionPos, PRInt32 *extensionLen) { return !_to ? NS_ERROR_NULL_POINTER : _to->ParseFilePath(filepath, filepathLen, directoryPos, directoryLen, basenamePos, basenameLen, extensionPos, extensionLen); } \
  NS_IMETHOD ParseFileName(const char *filename, PRInt32 filenameLen, PRUint32 *basenamePos, PRInt32 *basenameLen, PRUint32 *extensionPos, PRInt32 *extensionLen) { return !_to ? NS_ERROR_NULL_POINTER : _to->ParseFileName(filename, filenameLen, basenamePos, basenameLen, extensionPos, extensionLen); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsURLParser : public nsIURLParser
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIURLPARSER

  nsURLParser();

private:
  ~nsURLParser();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsURLParser, nsIURLParser)

nsURLParser::nsURLParser()
{
  /* member initializers and constructor code */
}

nsURLParser::~nsURLParser()
{
  /* destructor code */
}

/* void parseURL (in string spec, in long specLen, out unsigned long schemePos, out long schemeLen, out unsigned long authorityPos, out long authorityLen, out unsigned long pathPos, out long pathLen); */
NS_IMETHODIMP nsURLParser::ParseURL(const char *spec, PRInt32 specLen, PRUint32 *schemePos, PRInt32 *schemeLen, PRUint32 *authorityPos, PRInt32 *authorityLen, PRUint32 *pathPos, PRInt32 *pathLen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void parseAuthority (in string authority, in long authorityLen, out unsigned long usernamePos, out long usernameLen, out unsigned long passwordPos, out long passwordLen, out unsigned long hostnamePos, out long hostnameLen, out long port); */
NS_IMETHODIMP nsURLParser::ParseAuthority(const char *authority, PRInt32 authorityLen, PRUint32 *usernamePos, PRInt32 *usernameLen, PRUint32 *passwordPos, PRInt32 *passwordLen, PRUint32 *hostnamePos, PRInt32 *hostnameLen, PRInt32 *port)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void parseUserInfo (in string userinfo, in long userinfoLen, out unsigned long usernamePos, out long usernameLen, out unsigned long passwordPos, out long passwordLen); */
NS_IMETHODIMP nsURLParser::ParseUserInfo(const char *userinfo, PRInt32 userinfoLen, PRUint32 *usernamePos, PRInt32 *usernameLen, PRUint32 *passwordPos, PRInt32 *passwordLen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void parseServerInfo (in string serverinfo, in long serverinfoLen, out unsigned long hostnamePos, out long hostnameLen, out long port); */
NS_IMETHODIMP nsURLParser::ParseServerInfo(const char *serverinfo, PRInt32 serverinfoLen, PRUint32 *hostnamePos, PRInt32 *hostnameLen, PRInt32 *port)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void parsePath (in string path, in long pathLen, out unsigned long filepathPos, out long filepathLen, out unsigned long paramPos, out long paramLen, out unsigned long queryPos, out long queryLen, out unsigned long refPos, out long refLen); */
NS_IMETHODIMP nsURLParser::ParsePath(const char *path, PRInt32 pathLen, PRUint32 *filepathPos, PRInt32 *filepathLen, PRUint32 *paramPos, PRInt32 *paramLen, PRUint32 *queryPos, PRInt32 *queryLen, PRUint32 *refPos, PRInt32 *refLen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void parseFilePath (in string filepath, in long filepathLen, out unsigned long directoryPos, out long directoryLen, out unsigned long basenamePos, out long basenameLen, out unsigned long extensionPos, out long extensionLen); */
NS_IMETHODIMP nsURLParser::ParseFilePath(const char *filepath, PRInt32 filepathLen, PRUint32 *directoryPos, PRInt32 *directoryLen, PRUint32 *basenamePos, PRInt32 *basenameLen, PRUint32 *extensionPos, PRInt32 *extensionLen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void parseFileName (in string filename, in long filenameLen, out unsigned long basenamePos, out long basenameLen, out unsigned long extensionPos, out long extensionLen); */
NS_IMETHODIMP nsURLParser::ParseFileName(const char *filename, PRInt32 filenameLen, PRUint32 *basenamePos, PRInt32 *basenameLen, PRUint32 *extensionPos, PRInt32 *extensionLen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

// url parser key for use with the category manager
// mapping from scheme to url parser.
#define NS_IURLPARSER_KEY "@mozilla.org/urlparser;1"

#endif /* __gen_nsIURLParser_h__ */
