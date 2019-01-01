/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/xtf/public/nsIXTFElement.idl
 */

#ifndef __gen_nsIXTFElement_h__
#define __gen_nsIXTFElement_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIAtom; /* forward declaration */

class nsIDOMDocument; /* forward declaration */

class nsIDOMElement; /* forward declaration */

class nsIDOMNode; /* forward declaration */

class nsIDOMEvent; /* forward declaration */


/* starting interface:    nsIXTFElement */
#define NS_IXTFELEMENT_IID_STR "a8b607fd-24b6-4a8c-9a89-d9b24f8e2592"

#define NS_IXTFELEMENT_IID \
  {0xa8b607fd, 0x24b6, 0x4a8c, \
    { 0x9a, 0x89, 0xd9, 0xb2, 0x4f, 0x8e, 0x25, 0x92 }}

class NS_NO_VTABLE nsIXTFElement : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXTFELEMENT_IID)

  /* void onDestroyed (); */
  NS_IMETHOD OnDestroyed(void) = 0;

  enum { ELEMENT_TYPE_GENERIC_ELEMENT = 0U };

  enum { ELEMENT_TYPE_SVG_VISUAL = 1U };

  enum { ELEMENT_TYPE_XML_VISUAL = 2U };

  enum { ELEMENT_TYPE_XUL_VISUAL = 3U };

  enum { ELEMENT_TYPE_BINDABLE = 4U };

  /* readonly attribute unsigned long elementType; */
  NS_IMETHOD GetElementType(PRUint32 *aElementType) = 0;

  /* readonly attribute boolean isAttributeHandler; */
  NS_IMETHOD GetIsAttributeHandler(PRBool *aIsAttributeHandler) = 0;

  /* void getScriptingInterfaces (out unsigned long count, [array, size_is (count), retval] out nsIIDPtr array); */
  NS_IMETHOD GetScriptingInterfaces(PRUint32 *count, nsIID * **array) = 0;

  enum { NOTIFY_WILL_CHANGE_DOCUMENT = 1U };

  enum { NOTIFY_DOCUMENT_CHANGED = 2U };

  enum { NOTIFY_WILL_CHANGE_PARENT = 4U };

  enum { NOTIFY_PARENT_CHANGED = 8U };

  enum { NOTIFY_WILL_INSERT_CHILD = 16U };

  enum { NOTIFY_CHILD_INSERTED = 32U };

  enum { NOTIFY_WILL_APPEND_CHILD = 64U };

  enum { NOTIFY_CHILD_APPENDED = 128U };

  enum { NOTIFY_WILL_REMOVE_CHILD = 256U };

  enum { NOTIFY_CHILD_REMOVED = 512U };

  enum { NOTIFY_WILL_SET_ATTRIBUTE = 1024U };

  enum { NOTIFY_ATTRIBUTE_SET = 2048U };

  enum { NOTIFY_WILL_REMOVE_ATTRIBUTE = 4096U };

  enum { NOTIFY_ATTRIBUTE_REMOVED = 8192U };

  enum { NOTIFY_BEGIN_ADDING_CHILDREN = 16384U };

  enum { NOTIFY_DONE_ADDING_CHILDREN = 32768U };

  enum { NOTIFY_HANDLE_DEFAULT = 65536U };

  /* void willChangeDocument (in nsIDOMDocument newDoc); */
  NS_IMETHOD WillChangeDocument(nsIDOMDocument *newDoc) = 0;

  /* void documentChanged (in nsIDOMDocument newDoc); */
  NS_IMETHOD DocumentChanged(nsIDOMDocument *newDoc) = 0;

  /* void willChangeParent (in nsIDOMElement newParent); */
  NS_IMETHOD WillChangeParent(nsIDOMElement *newParent) = 0;

  /* void parentChanged (in nsIDOMElement newParent); */
  NS_IMETHOD ParentChanged(nsIDOMElement *newParent) = 0;

  /* void willInsertChild (in nsIDOMNode child, in unsigned long index); */
  NS_IMETHOD WillInsertChild(nsIDOMNode *child, PRUint32 index) = 0;

  /* void childInserted (in nsIDOMNode child, in unsigned long index); */
  NS_IMETHOD ChildInserted(nsIDOMNode *child, PRUint32 index) = 0;

  /* void willAppendChild (in nsIDOMNode child); */
  NS_IMETHOD WillAppendChild(nsIDOMNode *child) = 0;

  /* void childAppended (in nsIDOMNode child); */
  NS_IMETHOD ChildAppended(nsIDOMNode *child) = 0;

  /* void willRemoveChild (in unsigned long index); */
  NS_IMETHOD WillRemoveChild(PRUint32 index) = 0;

  /* void childRemoved (in unsigned long index); */
  NS_IMETHOD ChildRemoved(PRUint32 index) = 0;

  /* void willSetAttribute (in nsIAtom name, in AString newValue); */
  NS_IMETHOD WillSetAttribute(nsIAtom *name, const nsAString & newValue) = 0;

  /* void attributeSet (in nsIAtom name, in AString newValue); */
  NS_IMETHOD AttributeSet(nsIAtom *name, const nsAString & newValue) = 0;

  /* void willRemoveAttribute (in nsIAtom name); */
  NS_IMETHOD WillRemoveAttribute(nsIAtom *name) = 0;

  /* void attributeRemoved (in nsIAtom name); */
  NS_IMETHOD AttributeRemoved(nsIAtom *name) = 0;

  /* void beginAddingChildren (); */
  NS_IMETHOD BeginAddingChildren(void) = 0;

  /* void doneAddingChildren (); */
  NS_IMETHOD DoneAddingChildren(void) = 0;

  /* boolean handleDefault (in nsIDOMEvent aEvent); */
  NS_IMETHOD HandleDefault(nsIDOMEvent *aEvent, PRBool *_retval) = 0;

  /* void cloneState (in nsIDOMElement aElement); */
  NS_IMETHOD CloneState(nsIDOMElement *aElement) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXTFELEMENT \
  NS_IMETHOD OnDestroyed(void); \
  NS_IMETHOD GetElementType(PRUint32 *aElementType); \
  NS_IMETHOD GetIsAttributeHandler(PRBool *aIsAttributeHandler); \
  NS_IMETHOD GetScriptingInterfaces(PRUint32 *count, nsIID * **array); \
  NS_IMETHOD WillChangeDocument(nsIDOMDocument *newDoc); \
  NS_IMETHOD DocumentChanged(nsIDOMDocument *newDoc); \
  NS_IMETHOD WillChangeParent(nsIDOMElement *newParent); \
  NS_IMETHOD ParentChanged(nsIDOMElement *newParent); \
  NS_IMETHOD WillInsertChild(nsIDOMNode *child, PRUint32 index); \
  NS_IMETHOD ChildInserted(nsIDOMNode *child, PRUint32 index); \
  NS_IMETHOD WillAppendChild(nsIDOMNode *child); \
  NS_IMETHOD ChildAppended(nsIDOMNode *child); \
  NS_IMETHOD WillRemoveChild(PRUint32 index); \
  NS_IMETHOD ChildRemoved(PRUint32 index); \
  NS_IMETHOD WillSetAttribute(nsIAtom *name, const nsAString & newValue); \
  NS_IMETHOD AttributeSet(nsIAtom *name, const nsAString & newValue); \
  NS_IMETHOD WillRemoveAttribute(nsIAtom *name); \
  NS_IMETHOD AttributeRemoved(nsIAtom *name); \
  NS_IMETHOD BeginAddingChildren(void); \
  NS_IMETHOD DoneAddingChildren(void); \
  NS_IMETHOD HandleDefault(nsIDOMEvent *aEvent, PRBool *_retval); \
  NS_IMETHOD CloneState(nsIDOMElement *aElement); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXTFELEMENT(_to) \
  NS_IMETHOD OnDestroyed(void) { return _to OnDestroyed(); } \
  NS_IMETHOD GetElementType(PRUint32 *aElementType) { return _to GetElementType(aElementType); } \
  NS_IMETHOD GetIsAttributeHandler(PRBool *aIsAttributeHandler) { return _to GetIsAttributeHandler(aIsAttributeHandler); } \
  NS_IMETHOD GetScriptingInterfaces(PRUint32 *count, nsIID * **array) { return _to GetScriptingInterfaces(count, array); } \
  NS_IMETHOD WillChangeDocument(nsIDOMDocument *newDoc) { return _to WillChangeDocument(newDoc); } \
  NS_IMETHOD DocumentChanged(nsIDOMDocument *newDoc) { return _to DocumentChanged(newDoc); } \
  NS_IMETHOD WillChangeParent(nsIDOMElement *newParent) { return _to WillChangeParent(newParent); } \
  NS_IMETHOD ParentChanged(nsIDOMElement *newParent) { return _to ParentChanged(newParent); } \
  NS_IMETHOD WillInsertChild(nsIDOMNode *child, PRUint32 index) { return _to WillInsertChild(child, index); } \
  NS_IMETHOD ChildInserted(nsIDOMNode *child, PRUint32 index) { return _to ChildInserted(child, index); } \
  NS_IMETHOD WillAppendChild(nsIDOMNode *child) { return _to WillAppendChild(child); } \
  NS_IMETHOD ChildAppended(nsIDOMNode *child) { return _to ChildAppended(child); } \
  NS_IMETHOD WillRemoveChild(PRUint32 index) { return _to WillRemoveChild(index); } \
  NS_IMETHOD ChildRemoved(PRUint32 index) { return _to ChildRemoved(index); } \
  NS_IMETHOD WillSetAttribute(nsIAtom *name, const nsAString & newValue) { return _to WillSetAttribute(name, newValue); } \
  NS_IMETHOD AttributeSet(nsIAtom *name, const nsAString & newValue) { return _to AttributeSet(name, newValue); } \
  NS_IMETHOD WillRemoveAttribute(nsIAtom *name) { return _to WillRemoveAttribute(name); } \
  NS_IMETHOD AttributeRemoved(nsIAtom *name) { return _to AttributeRemoved(name); } \
  NS_IMETHOD BeginAddingChildren(void) { return _to BeginAddingChildren(); } \
  NS_IMETHOD DoneAddingChildren(void) { return _to DoneAddingChildren(); } \
  NS_IMETHOD HandleDefault(nsIDOMEvent *aEvent, PRBool *_retval) { return _to HandleDefault(aEvent, _retval); } \
  NS_IMETHOD CloneState(nsIDOMElement *aElement) { return _to CloneState(aElement); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXTFELEMENT(_to) \
  NS_IMETHOD OnDestroyed(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnDestroyed(); } \
  NS_IMETHOD GetElementType(PRUint32 *aElementType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetElementType(aElementType); } \
  NS_IMETHOD GetIsAttributeHandler(PRBool *aIsAttributeHandler) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIsAttributeHandler(aIsAttributeHandler); } \
  NS_IMETHOD GetScriptingInterfaces(PRUint32 *count, nsIID * **array) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScriptingInterfaces(count, array); } \
  NS_IMETHOD WillChangeDocument(nsIDOMDocument *newDoc) { return !_to ? NS_ERROR_NULL_POINTER : _to->WillChangeDocument(newDoc); } \
  NS_IMETHOD DocumentChanged(nsIDOMDocument *newDoc) { return !_to ? NS_ERROR_NULL_POINTER : _to->DocumentChanged(newDoc); } \
  NS_IMETHOD WillChangeParent(nsIDOMElement *newParent) { return !_to ? NS_ERROR_NULL_POINTER : _to->WillChangeParent(newParent); } \
  NS_IMETHOD ParentChanged(nsIDOMElement *newParent) { return !_to ? NS_ERROR_NULL_POINTER : _to->ParentChanged(newParent); } \
  NS_IMETHOD WillInsertChild(nsIDOMNode *child, PRUint32 index) { return !_to ? NS_ERROR_NULL_POINTER : _to->WillInsertChild(child, index); } \
  NS_IMETHOD ChildInserted(nsIDOMNode *child, PRUint32 index) { return !_to ? NS_ERROR_NULL_POINTER : _to->ChildInserted(child, index); } \
  NS_IMETHOD WillAppendChild(nsIDOMNode *child) { return !_to ? NS_ERROR_NULL_POINTER : _to->WillAppendChild(child); } \
  NS_IMETHOD ChildAppended(nsIDOMNode *child) { return !_to ? NS_ERROR_NULL_POINTER : _to->ChildAppended(child); } \
  NS_IMETHOD WillRemoveChild(PRUint32 index) { return !_to ? NS_ERROR_NULL_POINTER : _to->WillRemoveChild(index); } \
  NS_IMETHOD ChildRemoved(PRUint32 index) { return !_to ? NS_ERROR_NULL_POINTER : _to->ChildRemoved(index); } \
  NS_IMETHOD WillSetAttribute(nsIAtom *name, const nsAString & newValue) { return !_to ? NS_ERROR_NULL_POINTER : _to->WillSetAttribute(name, newValue); } \
  NS_IMETHOD AttributeSet(nsIAtom *name, const nsAString & newValue) { return !_to ? NS_ERROR_NULL_POINTER : _to->AttributeSet(name, newValue); } \
  NS_IMETHOD WillRemoveAttribute(nsIAtom *name) { return !_to ? NS_ERROR_NULL_POINTER : _to->WillRemoveAttribute(name); } \
  NS_IMETHOD AttributeRemoved(nsIAtom *name) { return !_to ? NS_ERROR_NULL_POINTER : _to->AttributeRemoved(name); } \
  NS_IMETHOD BeginAddingChildren(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->BeginAddingChildren(); } \
  NS_IMETHOD DoneAddingChildren(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->DoneAddingChildren(); } \
  NS_IMETHOD HandleDefault(nsIDOMEvent *aEvent, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->HandleDefault(aEvent, _retval); } \
  NS_IMETHOD CloneState(nsIDOMElement *aElement) { return !_to ? NS_ERROR_NULL_POINTER : _to->CloneState(aElement); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXTFElement : public nsIXTFElement
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFELEMENT

  nsXTFElement();

private:
  ~nsXTFElement();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXTFElement, nsIXTFElement)

nsXTFElement::nsXTFElement()
{
  /* member initializers and constructor code */
}

nsXTFElement::~nsXTFElement()
{
  /* destructor code */
}

/* void onDestroyed (); */
NS_IMETHODIMP nsXTFElement::OnDestroyed()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long elementType; */
NS_IMETHODIMP nsXTFElement::GetElementType(PRUint32 *aElementType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean isAttributeHandler; */
NS_IMETHODIMP nsXTFElement::GetIsAttributeHandler(PRBool *aIsAttributeHandler)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getScriptingInterfaces (out unsigned long count, [array, size_is (count), retval] out nsIIDPtr array); */
NS_IMETHODIMP nsXTFElement::GetScriptingInterfaces(PRUint32 *count, nsIID * **array)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void willChangeDocument (in nsIDOMDocument newDoc); */
NS_IMETHODIMP nsXTFElement::WillChangeDocument(nsIDOMDocument *newDoc)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void documentChanged (in nsIDOMDocument newDoc); */
NS_IMETHODIMP nsXTFElement::DocumentChanged(nsIDOMDocument *newDoc)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void willChangeParent (in nsIDOMElement newParent); */
NS_IMETHODIMP nsXTFElement::WillChangeParent(nsIDOMElement *newParent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void parentChanged (in nsIDOMElement newParent); */
NS_IMETHODIMP nsXTFElement::ParentChanged(nsIDOMElement *newParent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void willInsertChild (in nsIDOMNode child, in unsigned long index); */
NS_IMETHODIMP nsXTFElement::WillInsertChild(nsIDOMNode *child, PRUint32 index)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void childInserted (in nsIDOMNode child, in unsigned long index); */
NS_IMETHODIMP nsXTFElement::ChildInserted(nsIDOMNode *child, PRUint32 index)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void willAppendChild (in nsIDOMNode child); */
NS_IMETHODIMP nsXTFElement::WillAppendChild(nsIDOMNode *child)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void childAppended (in nsIDOMNode child); */
NS_IMETHODIMP nsXTFElement::ChildAppended(nsIDOMNode *child)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void willRemoveChild (in unsigned long index); */
NS_IMETHODIMP nsXTFElement::WillRemoveChild(PRUint32 index)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void childRemoved (in unsigned long index); */
NS_IMETHODIMP nsXTFElement::ChildRemoved(PRUint32 index)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void willSetAttribute (in nsIAtom name, in AString newValue); */
NS_IMETHODIMP nsXTFElement::WillSetAttribute(nsIAtom *name, const nsAString & newValue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void attributeSet (in nsIAtom name, in AString newValue); */
NS_IMETHODIMP nsXTFElement::AttributeSet(nsIAtom *name, const nsAString & newValue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void willRemoveAttribute (in nsIAtom name); */
NS_IMETHODIMP nsXTFElement::WillRemoveAttribute(nsIAtom *name)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void attributeRemoved (in nsIAtom name); */
NS_IMETHODIMP nsXTFElement::AttributeRemoved(nsIAtom *name)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void beginAddingChildren (); */
NS_IMETHODIMP nsXTFElement::BeginAddingChildren()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void doneAddingChildren (); */
NS_IMETHODIMP nsXTFElement::DoneAddingChildren()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean handleDefault (in nsIDOMEvent aEvent); */
NS_IMETHODIMP nsXTFElement::HandleDefault(nsIDOMEvent *aEvent, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void cloneState (in nsIDOMElement aElement); */
NS_IMETHODIMP nsXTFElement::CloneState(nsIDOMElement *aElement)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXTFElement_h__ */
