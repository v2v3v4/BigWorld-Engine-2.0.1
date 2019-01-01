/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/docshell/base/nsIDocShellTreeOwner.idl
 */

#ifndef __gen_nsIDocShellTreeOwner_h__
#define __gen_nsIDocShellTreeOwner_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDocShellTreeItem; /* forward declaration */


/* starting interface:    nsIDocShellTreeOwner */
#define NS_IDOCSHELLTREEOWNER_IID_STR "9e508466-5ebb-4618-abfa-9ad47bed0b2e"

#define NS_IDOCSHELLTREEOWNER_IID \
  {0x9e508466, 0x5ebb, 0x4618, \
    { 0xab, 0xfa, 0x9a, 0xd4, 0x7b, 0xed, 0x0b, 0x2e }}

class NS_NO_VTABLE nsIDocShellTreeOwner : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOCSHELLTREEOWNER_IID)

  /* nsIDocShellTreeItem findItemWithName (in wstring name, in nsIDocShellTreeItem aRequestor, in nsIDocShellTreeItem aOriginalRequestor); */
  NS_IMETHOD FindItemWithName(const PRUnichar *name, nsIDocShellTreeItem *aRequestor, nsIDocShellTreeItem *aOriginalRequestor, nsIDocShellTreeItem **_retval) = 0;

  /* void contentShellAdded (in nsIDocShellTreeItem aContentShell, in boolean aPrimary, in wstring aID); */
  NS_IMETHOD ContentShellAdded(nsIDocShellTreeItem *aContentShell, PRBool aPrimary, const PRUnichar *aID) = 0;

  /* readonly attribute nsIDocShellTreeItem primaryContentShell; */
  NS_IMETHOD GetPrimaryContentShell(nsIDocShellTreeItem * *aPrimaryContentShell) = 0;

  /* void sizeShellTo (in nsIDocShellTreeItem shell, in long cx, in long cy); */
  NS_IMETHOD SizeShellTo(nsIDocShellTreeItem *shell, PRInt32 cx, PRInt32 cy) = 0;

  /* void setPersistence (in boolean aPersistPosition, in boolean aPersistSize, in boolean aPersistSizeMode); */
  NS_IMETHOD SetPersistence(PRBool aPersistPosition, PRBool aPersistSize, PRBool aPersistSizeMode) = 0;

  /* void getPersistence (out boolean aPersistPosition, out boolean aPersistSize, out boolean aPersistSizeMode); */
  NS_IMETHOD GetPersistence(PRBool *aPersistPosition, PRBool *aPersistSize, PRBool *aPersistSizeMode) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOCSHELLTREEOWNER \
  NS_IMETHOD FindItemWithName(const PRUnichar *name, nsIDocShellTreeItem *aRequestor, nsIDocShellTreeItem *aOriginalRequestor, nsIDocShellTreeItem **_retval); \
  NS_IMETHOD ContentShellAdded(nsIDocShellTreeItem *aContentShell, PRBool aPrimary, const PRUnichar *aID); \
  NS_IMETHOD GetPrimaryContentShell(nsIDocShellTreeItem * *aPrimaryContentShell); \
  NS_IMETHOD SizeShellTo(nsIDocShellTreeItem *shell, PRInt32 cx, PRInt32 cy); \
  NS_IMETHOD SetPersistence(PRBool aPersistPosition, PRBool aPersistSize, PRBool aPersistSizeMode); \
  NS_IMETHOD GetPersistence(PRBool *aPersistPosition, PRBool *aPersistSize, PRBool *aPersistSizeMode); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOCSHELLTREEOWNER(_to) \
  NS_IMETHOD FindItemWithName(const PRUnichar *name, nsIDocShellTreeItem *aRequestor, nsIDocShellTreeItem *aOriginalRequestor, nsIDocShellTreeItem **_retval) { return _to FindItemWithName(name, aRequestor, aOriginalRequestor, _retval); } \
  NS_IMETHOD ContentShellAdded(nsIDocShellTreeItem *aContentShell, PRBool aPrimary, const PRUnichar *aID) { return _to ContentShellAdded(aContentShell, aPrimary, aID); } \
  NS_IMETHOD GetPrimaryContentShell(nsIDocShellTreeItem * *aPrimaryContentShell) { return _to GetPrimaryContentShell(aPrimaryContentShell); } \
  NS_IMETHOD SizeShellTo(nsIDocShellTreeItem *shell, PRInt32 cx, PRInt32 cy) { return _to SizeShellTo(shell, cx, cy); } \
  NS_IMETHOD SetPersistence(PRBool aPersistPosition, PRBool aPersistSize, PRBool aPersistSizeMode) { return _to SetPersistence(aPersistPosition, aPersistSize, aPersistSizeMode); } \
  NS_IMETHOD GetPersistence(PRBool *aPersistPosition, PRBool *aPersistSize, PRBool *aPersistSizeMode) { return _to GetPersistence(aPersistPosition, aPersistSize, aPersistSizeMode); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOCSHELLTREEOWNER(_to) \
  NS_IMETHOD FindItemWithName(const PRUnichar *name, nsIDocShellTreeItem *aRequestor, nsIDocShellTreeItem *aOriginalRequestor, nsIDocShellTreeItem **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->FindItemWithName(name, aRequestor, aOriginalRequestor, _retval); } \
  NS_IMETHOD ContentShellAdded(nsIDocShellTreeItem *aContentShell, PRBool aPrimary, const PRUnichar *aID) { return !_to ? NS_ERROR_NULL_POINTER : _to->ContentShellAdded(aContentShell, aPrimary, aID); } \
  NS_IMETHOD GetPrimaryContentShell(nsIDocShellTreeItem * *aPrimaryContentShell) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPrimaryContentShell(aPrimaryContentShell); } \
  NS_IMETHOD SizeShellTo(nsIDocShellTreeItem *shell, PRInt32 cx, PRInt32 cy) { return !_to ? NS_ERROR_NULL_POINTER : _to->SizeShellTo(shell, cx, cy); } \
  NS_IMETHOD SetPersistence(PRBool aPersistPosition, PRBool aPersistSize, PRBool aPersistSizeMode) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPersistence(aPersistPosition, aPersistSize, aPersistSizeMode); } \
  NS_IMETHOD GetPersistence(PRBool *aPersistPosition, PRBool *aPersistSize, PRBool *aPersistSizeMode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPersistence(aPersistPosition, aPersistSize, aPersistSizeMode); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDocShellTreeOwner : public nsIDocShellTreeOwner
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOCSHELLTREEOWNER

  nsDocShellTreeOwner();

private:
  ~nsDocShellTreeOwner();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDocShellTreeOwner, nsIDocShellTreeOwner)

nsDocShellTreeOwner::nsDocShellTreeOwner()
{
  /* member initializers and constructor code */
}

nsDocShellTreeOwner::~nsDocShellTreeOwner()
{
  /* destructor code */
}

/* nsIDocShellTreeItem findItemWithName (in wstring name, in nsIDocShellTreeItem aRequestor, in nsIDocShellTreeItem aOriginalRequestor); */
NS_IMETHODIMP nsDocShellTreeOwner::FindItemWithName(const PRUnichar *name, nsIDocShellTreeItem *aRequestor, nsIDocShellTreeItem *aOriginalRequestor, nsIDocShellTreeItem **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void contentShellAdded (in nsIDocShellTreeItem aContentShell, in boolean aPrimary, in wstring aID); */
NS_IMETHODIMP nsDocShellTreeOwner::ContentShellAdded(nsIDocShellTreeItem *aContentShell, PRBool aPrimary, const PRUnichar *aID)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDocShellTreeItem primaryContentShell; */
NS_IMETHODIMP nsDocShellTreeOwner::GetPrimaryContentShell(nsIDocShellTreeItem * *aPrimaryContentShell)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void sizeShellTo (in nsIDocShellTreeItem shell, in long cx, in long cy); */
NS_IMETHODIMP nsDocShellTreeOwner::SizeShellTo(nsIDocShellTreeItem *shell, PRInt32 cx, PRInt32 cy)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setPersistence (in boolean aPersistPosition, in boolean aPersistSize, in boolean aPersistSizeMode); */
NS_IMETHODIMP nsDocShellTreeOwner::SetPersistence(PRBool aPersistPosition, PRBool aPersistSize, PRBool aPersistSizeMode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getPersistence (out boolean aPersistPosition, out boolean aPersistSize, out boolean aPersistSizeMode); */
NS_IMETHODIMP nsDocShellTreeOwner::GetPersistence(PRBool *aPersistPosition, PRBool *aPersistSize, PRBool *aPersistSizeMode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDocShellTreeOwner_MOZILLA_1_8_BRANCH */
#define NS_IDOCSHELLTREEOWNER_MOZILLA_1_8_BRANCH_IID_STR "3c2a6927-e923-4ea8-bbda-a335c768ce4e"

#define NS_IDOCSHELLTREEOWNER_MOZILLA_1_8_BRANCH_IID \
  {0x3c2a6927, 0xe923, 0x4ea8, \
    { 0xbb, 0xda, 0xa3, 0x35, 0xc7, 0x68, 0xce, 0x4e }}

/**
 * Interface added to handle window targeting in tabbrowser.  This is a total
 * hack that's only needed to work around the fact that the tree owner api is
 * really pretty useless for dealing with multiple "real" browsers in the same
 * "docshell tree" and that there's no way to set up multiple treeowners in
 * XUL-land right now.  Gecko 1.9 will NOT be shipping this interface, and
 * nsIDocShellTreeOwner will hopefully be improved significantly.
 *
 * @status TEMPORARY
 */
class NS_NO_VTABLE nsIDocShellTreeOwner_MOZILLA_1_8_BRANCH : public nsIDocShellTreeOwner {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOCSHELLTREEOWNER_MOZILLA_1_8_BRANCH_IID)

  /**
   * Called when a content shell is added to the docshell tree.  This is _only_
   * called for "root" content shells (that is, ones whose parent is a chrome
   * shell).
   *
   * @param aContentShell the shell being added.
   * @param aPrimary whether the shell is primary.
   * @param aTargetable whether the shell can be a target for named window
   *                    targeting.
   * @param aID the "id" of the shell.  What this actually means is undefined.
   *            Don't rely on this for anything.
   */
  /* void contentShellAdded2 (in nsIDocShellTreeItem aContentShell, in boolean aPrimary, in boolean aTargetable, in AString aID); */
  NS_IMETHOD ContentShellAdded2(nsIDocShellTreeItem *aContentShell, PRBool aPrimary, PRBool aTargetable, const nsAString & aID) = 0;

  /**
   * Called when a content shell is removed from the docshell tree.  This is
   * _only_ called for "root" content shells (that is, ones whose parent is a
   * chrome shell).  Note that if aContentShell was never added,
   * contentShellRemoved should just do nothing.
   *
   * @param aContentShell the shell being removed.
   */
  /* void contentShellRemoved (in nsIDocShellTreeItem aContentShell); */
  NS_IMETHOD ContentShellRemoved(nsIDocShellTreeItem *aContentShell) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOCSHELLTREEOWNER_MOZILLA_1_8_BRANCH \
  NS_IMETHOD ContentShellAdded2(nsIDocShellTreeItem *aContentShell, PRBool aPrimary, PRBool aTargetable, const nsAString & aID); \
  NS_IMETHOD ContentShellRemoved(nsIDocShellTreeItem *aContentShell); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOCSHELLTREEOWNER_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD ContentShellAdded2(nsIDocShellTreeItem *aContentShell, PRBool aPrimary, PRBool aTargetable, const nsAString & aID) { return _to ContentShellAdded2(aContentShell, aPrimary, aTargetable, aID); } \
  NS_IMETHOD ContentShellRemoved(nsIDocShellTreeItem *aContentShell) { return _to ContentShellRemoved(aContentShell); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOCSHELLTREEOWNER_MOZILLA_1_8_BRANCH(_to) \
  NS_IMETHOD ContentShellAdded2(nsIDocShellTreeItem *aContentShell, PRBool aPrimary, PRBool aTargetable, const nsAString & aID) { return !_to ? NS_ERROR_NULL_POINTER : _to->ContentShellAdded2(aContentShell, aPrimary, aTargetable, aID); } \
  NS_IMETHOD ContentShellRemoved(nsIDocShellTreeItem *aContentShell) { return !_to ? NS_ERROR_NULL_POINTER : _to->ContentShellRemoved(aContentShell); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDocShellTreeOwner_MOZILLA_1_8_BRANCH : public nsIDocShellTreeOwner_MOZILLA_1_8_BRANCH
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOCSHELLTREEOWNER_MOZILLA_1_8_BRANCH

  nsDocShellTreeOwner_MOZILLA_1_8_BRANCH();

private:
  ~nsDocShellTreeOwner_MOZILLA_1_8_BRANCH();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDocShellTreeOwner_MOZILLA_1_8_BRANCH, nsIDocShellTreeOwner_MOZILLA_1_8_BRANCH)

nsDocShellTreeOwner_MOZILLA_1_8_BRANCH::nsDocShellTreeOwner_MOZILLA_1_8_BRANCH()
{
  /* member initializers and constructor code */
}

nsDocShellTreeOwner_MOZILLA_1_8_BRANCH::~nsDocShellTreeOwner_MOZILLA_1_8_BRANCH()
{
  /* destructor code */
}

/* void contentShellAdded2 (in nsIDocShellTreeItem aContentShell, in boolean aPrimary, in boolean aTargetable, in AString aID); */
NS_IMETHODIMP nsDocShellTreeOwner_MOZILLA_1_8_BRANCH::ContentShellAdded2(nsIDocShellTreeItem *aContentShell, PRBool aPrimary, PRBool aTargetable, const nsAString & aID)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void contentShellRemoved (in nsIDocShellTreeItem aContentShell); */
NS_IMETHODIMP nsDocShellTreeOwner_MOZILLA_1_8_BRANCH::ContentShellRemoved(nsIDocShellTreeItem *aContentShell)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDocShellTreeOwner_h__ */
