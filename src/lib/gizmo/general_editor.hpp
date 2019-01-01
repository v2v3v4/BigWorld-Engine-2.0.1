/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GENERAL_EDITOR_HPP
#define GENERAL_EDITOR_HPP


#include "cstdmf/smartpointer.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

#include "value_type.hpp"


class GeneralEditor;
typedef SmartPointer<GeneralEditor> GeneralEditorPtr;

class GeneralProperty;


/**
 *	This class controls and defines the editing operations which
 *	can be performed on a general object.
 */
class GeneralEditor : public PyObjectPlus
{
	Py_Header( GeneralEditor, PyObjectPlus )
public:
	GeneralEditor( PyTypePlus * pType = &s_type_ );
	virtual ~GeneralEditor();

	virtual void addProperty( GeneralProperty * pProp );
	// desired views:
	//  gizmos (can be left to item itself 'tho... maybe all should be?)
	//  text (I guess)
	//  Borland controls
	//  python (combine with text...?)

	virtual void addingAssetMetadata( bool assetMetadata ) {}
	virtual bool useFullDateFormat() { return false; }

	void elect();
	void expel();

	typedef std::vector< GeneralEditorPtr > Editors;

	static const Editors & currentEditors( void );
	static bool settingMultipleEditors() { return s_settingMultipleEditors_; }
	static void currentEditors( const Editors & editors );

	static void createViews( bool doCreate ) { s_createViews_ = doCreate; }
	static bool createViews() { return s_createViews_; }

	PyObject *		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject * value );

	PyObject *		pyAdditionalMembers( PyObject * pBaseSeq );

	PY_MODULE_STATIC_METHOD_DECLARE( py_getCurrentEditors )
	PY_MODULE_STATIC_METHOD_DECLARE( py_setCurrentEditors )

protected:
	typedef std::vector< GeneralProperty * > PropList;
	PropList		properties_;

	bool			constructorOver_;

	// this keeps track of the last item edited
	std::wstring lastItemName_;

private:
	GeneralEditor( const GeneralEditor& );
	GeneralEditor& operator=( const GeneralEditor& );

	static Editors	s_currentEditors_;
	static int s_currentEditorIndex_;
	static int s_lastEditorIndex_;
	static bool s_settingMultipleEditors_;
	static bool s_createViews_;
};

// move this down.... theres already a propertymanager...
namespace PropManager
{
	typedef void (*PropFini)( void );

	void registerFini(PropFini fn);
	void fini();
};

/**
 *	This macro declares the view factory stuff for a property class
 */
#define GENPROPERTY_VIEW_FACTORY_DECLARE( PROPCLASS )						\
	public:																	\
		typedef View * (*ViewFactory)( PROPCLASS & prop );					\
		static void registerViewFactory( int vkid, ViewFactory fn );		\
		static void fini();													\
																			\
	private:																\
		static std::vector<ViewFactory>	* viewFactories_;					\
};																			\
extern "C" WORLDEDITORDLL_API void PROPCLASS##_registerViewFactory(			\
				int vkid, PROPCLASS::ViewFactory fn );						\
namespace DodgyNamespace {													\


/**
 *	This macro implements the view factory stuff for a property class
 */
#define GENPROPERTY_VIEW_FACTORY( PROPCLASS )								\
	std::vector<PROPCLASS::ViewFactory> * PROPCLASS::viewFactories_ = NULL;	\
																			\
	void PROPCLASS::registerViewFactory( int vkid, ViewFactory fn )			\
	{																		\
		if (viewFactories_ == NULL)											\
			viewFactories_ = new std::vector<ViewFactory>;					\
		while (int(viewFactories_->size()) <= vkid)							\
			viewFactories_->push_back( NULL );								\
		(*viewFactories_)[ vkid ] = fn;										\
		PropManager::registerFini( &fini );									\
	}																		\
	void PROPCLASS::fini( )													\
	{																		\
		delete viewFactories_;												\
		viewFactories_ = NULL;												\
	}																		\
																			\
	void PROPCLASS##_registerViewFactory(									\
							int vkid, PROPCLASS::ViewFactory fn )			\
	{																		\
		PROPCLASS::registerViewFactory( vkid, fn );							\
	}																		\


class PropertyManager : public ReferenceCount
{
public:
	virtual bool canRemoveItem() { return false; }
	virtual void removeItem() = 0;

	virtual bool canAddItem() { return false; }
	virtual void addItem() = 0;
};
typedef SmartPointer<PropertyManager> PropertyManagerPtr;


#define RETURN_VALUETYPE( TYPE )						\
	static ValueType s_##TYPE( ValueTypeDesc::TYPE );	\
	return s_##TYPE;


//TODO: just revert back and add the vectors to a global list to be destructed at some stage....

class GeneralProperty
{
public:

	GeneralProperty( const std::string & name, const std::wstring & uiname = L"" );

    void WBEditable( bool editable );
    bool WBEditable() const;

    void descName( const std::wstring& descName );
	const std::wstring& descName();
	
	void UIName( const std::wstring& name );
    const std::wstring& UIName();

	void UIDesc( const std::wstring& name );
    const std::wstring& UIDesc();

	void exposedToScriptName( const std::wstring& exposedToScriptName );
	const std::wstring& exposedToScriptName();

	void canExposeToScript( bool canExposeToScript );
	bool canExposeToScript() const;

	virtual void fileFilter( const std::wstring & fileFilter ) {}
	virtual const std::wstring & fileFilter() const
	{
		static std::wstring s_emptyFilter( L"" );
		return s_emptyFilter;
	}

	virtual const ValueType & valueType() const { RETURN_VALUETYPE( UNKNOWN ); }

	virtual void deleteSelf() { delete this; }

	virtual void elect();
	virtual void elected();
	virtual void expel();
	virtual void select();

	virtual PyObject * EDCALL pyGet();
	virtual int EDCALL pySet( PyObject * value, bool transient = false,
													bool addBarrier = true );

	const char * EDCALL name() const		{ return name_.c_str(); }

	void setGroup( const std::wstring & groupName ) { group_ = groupName; }
	const std::wstring & getGroup() { return group_; }

	class View
	{
	public:
		// Make the object responsible for deleting itself so that it is deleted
		// from the correct heap.
		virtual void EDCALL deleteSelf()	{ delete this; }

		virtual void EDCALL elect() = 0;
		virtual void EDCALL elected() {};
		virtual void EDCALL lastElected() { ERROR_MSG( "This view does not support 'lastElected'\n" ); }
		virtual bool isEditorView() const { return false; }
		virtual void EDCALL expel() = 0;

		virtual void EDCALL select() = 0;

		static void pLastElected( View* v ) { pLastElected_ = v; }
		static View* pLastElected() { return pLastElected_; }
	protected:
		// Always use deleteSelf. This is because the object may not have been
		// created in the DLL.
		virtual ~View() {}
		static View* pLastElected_;
	};

	static int nextViewKindID();

	void setPropertyManager( PropertyManagerPtr management ) { propManager_ = management; }
	PropertyManagerPtr getPropertyManager() const { return propManager_; }

	bool hasViews() const;

protected:
	virtual ~GeneralProperty();
	static int nextViewKindID_;

	class Views
	{
	public:
		Views();
		~Views();

		void set( int i, View * v );
		View * operator[]( int i );
		const View * operator[]( int i ) const;

	private:
		View **		e_;

		Views( const Views & oth );
		Views & operator=( const Views & oth );
	};

	Views				views_;
	std::string			name_;
	std::wstring		group_;

private:
	GeneralProperty( const GeneralProperty & oth );
	GeneralProperty & operator=( const GeneralProperty & oth );

	friend class Views;

	PropertyManagerPtr propManager_;
	uint32 flags_;

    bool            WBEditable_;
    std::wstring    descName_;
	std::wstring    UIName_;
    std::wstring    UIDesc_;
	std::wstring    exposedToScriptName_;
	bool			canExposeToScript_;

	GENPROPERTY_VIEW_FACTORY_DECLARE( GeneralProperty )
};


// Exported C style function for Borland to call.
extern "C" WORLDEDITORDLL_API int GeneralProperty_nextViewKindID();


/**
 *	This macro sets up the views for a general property type.
 *
 *	It should be used in the constructor of all classes that derive from
 *	GeneralProperty.
 *
 *	Note that if a view kind has implementations for both derived and base
 *	classes of a property (which would not be not unusual), then the base
 *	class view will get created for a short time before it is deleted and
 *	replaced by the derived class view. If this turns out to be a problem
 *	it could be avoided, but I will leave it for now.
 */
#define GENPROPERTY_MAKE_VIEWS()											\
	if (GeneralEditor::createViews())										\
	{																		\
		if (viewFactories_ != NULL)											\
		{																	\
			MF_ASSERT( int(viewFactories_->size()) <= nextViewKindID_ );	\
																			\
			for (uint i = 0; i < viewFactories_->size(); i++)				\
			{																\
				ViewFactory vf = (*viewFactories_)[i];						\
				if (vf != NULL)												\
				{															\
					View * v = (*vf)( *this );								\
					if (v != NULL)											\
					{														\
						views_.set( i, v );									\
					}														\
				}															\
			}																\
		}																	\
		else																\
		{																	\
			MF_ASSERT( "View Factory NULL.  Try linking in the Enroller" )	\
		}																	\
	}



/**
 *	This is the base class for all read only properties
 */
class GeneralROProperty : public GeneralProperty
{
public:
	GeneralROProperty( const std::string & name );

	virtual int EDCALL pySet( PyObject * value, bool transient = false,
													bool addBarrier = true );

private:

	GENPROPERTY_VIEW_FACTORY_DECLARE( GeneralROProperty )
};



#endif // GENERAL_EDITOR_HPP
