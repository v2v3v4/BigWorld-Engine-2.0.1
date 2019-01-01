/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/css/nsIDOMCSSMozDocumentRule.idl
 */

#ifndef __gen_nsIDOMCSSMozDocumentRule_h__
#define __gen_nsIDOMCSSMozDocumentRule_h__


#ifndef __gen_nsIDOMCSSRule_h__
#include "nsIDOMCSSRule.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMCSSMozDocumentRule */
#define NS_IDOMCSSMOZDOCUMENTRULE_IID_STR "4eb9adac-afaf-4b8a-8640-7340863c1587"

#define NS_IDOMCSSMOZDOCUMENTRULE_IID \
  {0x4eb9adac, 0xafaf, 0x4b8a, \
    { 0x86, 0x40, 0x73, 0x40, 0x86, 0x3c, 0x15, 0x87 }}

/**
 * Modified version of nsIDOMCSSMediaRule for @-moz-document rules.
 */
class NS_NO_VTABLE nsIDOMCSSMozDocumentRule : public nsIDOMCSSRule {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMCSSMOZDOCUMENTRULE_IID)

  /* readonly attribute nsIDOMCSSRuleList cssRules; */
  NS_IMETHOD GetCssRules(nsIDOMCSSRuleList * *aCssRules) = 0;

  /* unsigned long insertRule (in DOMString rule, in unsigned long index)  raises (DOMException); */
  NS_IMETHOD InsertRule(const nsAString & rule, PRUint32 index, PRUint32 *_retval) = 0;

  /* void deleteRule (in unsigned long index)  raises (DOMException); */
  NS_IMETHOD DeleteRule(PRUint32 index) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMCSSMOZDOCUMENTRULE \
  NS_IMETHOD GetCssRules(nsIDOMCSSRuleList * *aCssRules); \
  NS_IMETHOD InsertRule(const nsAString & rule, PRUint32 index, PRUint32 *_retval); \
  NS_IMETHOD DeleteRule(PRUint32 index); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMCSSMOZDOCUMENTRULE(_to) \
  NS_IMETHOD GetCssRules(nsIDOMCSSRuleList * *aCssRules) { return _to GetCssRules(aCssRules); } \
  NS_IMETHOD InsertRule(const nsAString & rule, PRUint32 index, PRUint32 *_retval) { return _to InsertRule(rule, index, _retval); } \
  NS_IMETHOD DeleteRule(PRUint32 index) { return _to DeleteRule(index); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMCSSMOZDOCUMENTRULE(_to) \
  NS_IMETHOD GetCssRules(nsIDOMCSSRuleList * *aCssRules) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCssRules(aCssRules); } \
  NS_IMETHOD InsertRule(const nsAString & rule, PRUint32 index, PRUint32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->InsertRule(rule, index, _retval); } \
  NS_IMETHOD DeleteRule(PRUint32 index) { return !_to ? NS_ERROR_NULL_POINTER : _to->DeleteRule(index); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMCSSMozDocumentRule : public nsIDOMCSSMozDocumentRule
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMCSSMOZDOCUMENTRULE

  nsDOMCSSMozDocumentRule();

private:
  ~nsDOMCSSMozDocumentRule();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMCSSMozDocumentRule, nsIDOMCSSMozDocumentRule)

nsDOMCSSMozDocumentRule::nsDOMCSSMozDocumentRule()
{
  /* member initializers and constructor code */
}

nsDOMCSSMozDocumentRule::~nsDOMCSSMozDocumentRule()
{
  /* destructor code */
}

/* readonly attribute nsIDOMCSSRuleList cssRules; */
NS_IMETHODIMP nsDOMCSSMozDocumentRule::GetCssRules(nsIDOMCSSRuleList * *aCssRules)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* unsigned long insertRule (in DOMString rule, in unsigned long index)  raises (DOMException); */
NS_IMETHODIMP nsDOMCSSMozDocumentRule::InsertRule(const nsAString & rule, PRUint32 index, PRUint32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void deleteRule (in unsigned long index)  raises (DOMException); */
NS_IMETHODIMP nsDOMCSSMozDocumentRule::DeleteRule(PRUint32 index)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMCSSMozDocumentRule_h__ */
