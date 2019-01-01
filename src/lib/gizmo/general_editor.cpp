/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "general_editor.hpp"
#include "tool_manager.hpp"
#include "appmgr/options.hpp"
#include "current_general_properties.hpp"

#include <set>

// -----------------------------------------------------------------------------
// Section: GeneralEditor
// -----------------------------------------------------------------------------

// make the standard python stuff
PY_TYPEOBJECT( GeneralEditor )

PY_BEGIN_METHODS( GeneralEditor )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( GeneralEditor )
PY_END_ATTRIBUTES()

PY_MODULE_STATIC_METHOD( GeneralEditor, getCurrentEditors, WorldEditor )
PY_MODULE_STATIC_METHOD( GeneralEditor, setCurrentEditors, WorldEditor )

DECLARE_DEBUG_COMPONENT2( "WorldEditor", 0 )

/**
 *	Constructor.
 */
GeneralEditor::GeneralEditor( PyTypePlus * pType ) :
	PyObjectPlus( pType ),
	constructorOver_( false )
{
}


/**
 *	Destructor.
 */
GeneralEditor::~GeneralEditor()
{
	BW_GUARD;

	for (PropList::iterator it = properties_.begin();
		it != properties_.end();
		it++)
	{
		(*it)->deleteSelf();
	}
	properties_.clear();
}


/**
 *	This method adds the given property to this editor. The editor takes
 *	ownership of the pointer passed in. Properties may only be added in
 *	response to an edEdit call on an item ... they are not dynamic.
 *	This is more to enable property views to be written more easily and
 *	not worry about new properties appearing while elected than anything
 *	else. If this ruling proves constrictive then it could be lifted.
 */
void GeneralEditor::addProperty( GeneralProperty * pProp )
{
	BW_GUARD;

	MF_ASSERT( !constructorOver_ );
    MF_ASSERT( pProp );

	// add the item as the parent group property belongs to
	if ( lastItemName_.empty() )
		pProp->setGroup( pProp->getGroup() );
	else
		pProp->setGroup( lastItemName_ + L"/" + pProp->getGroup() );

	properties_.push_back( pProp );
}


/**
 *	This method takes whatever action is necessary when a chunk item editor
 *	is elected to the list of currently visible editors
 */
void GeneralEditor::elect()
{
	BW_GUARD;

	s_settingMultipleEditors_ = s_currentEditors_.size() > 1 &&
		std::find( s_currentEditors_.begin(), s_currentEditors_.end(), this )
			!= s_currentEditors_.end();

	// Find last property with a view, needed to finish off common properties.
	PropList::iterator itLastProp = properties_.end();
	for (PropList::iterator it = properties_.begin();
		it != properties_.end(); ++it)
	{
		if ((*it)->hasViews())
		{
			itLastProp = it;
		}
	}

	for (PropList::iterator it = properties_.begin();
		it != properties_.end(); ++it)
	{
		(*it)->elect();
		(*it)->elected();
	}
}

/**
 *	This method takes whatever action is necessary when a chunk item editor
 *	is expelled from the list of currently visible editors
 */
void GeneralEditor::expel()
{
	BW_GUARD;

	for (PropList::iterator it = properties_.begin();
		it != properties_.end();
		it++)
	{
		(*it)->expel();
	}
}

/**
 *	This static method returns the list of current editors
 */
const GeneralEditor::Editors & GeneralEditor::currentEditors()
{
	BW_GUARD;

	return s_currentEditors_;
}

/**
 *	This static method sets the list of current editors
 */
void GeneralEditor::currentEditors( const Editors & newEds )
{
	BW_GUARD;

	std::set<int>	notChanged;

	GeneralProperty::View::pLastElected( NULL );

	Editors oldEds = s_currentEditors_;
	s_currentEditors_.clear();

	// get rid of old editors
	for (Editors::iterator it = oldEds.begin(); it != oldEds.end(); it++)
	{
		Editors::const_iterator found =
			std::find( newEds.begin(), newEds.end(), *it );
		if (found != newEds.end())
		{
			notChanged.insert( found - newEds.begin() );
		}
		else
		{
			(*it)->expel();
		}
	}

	oldEds.clear();

	s_currentEditors_ = newEds;
	s_currentEditorIndex_ = notChanged.size();
	s_lastEditorIndex_ = newEds.size() - 1;

	// instate new editors
	for (uint i = 0; i < newEds.size(); i++)
	{
		if (notChanged.find( i ) == notChanged.end())
		{
			newEds[i]->elect();
			s_currentEditorIndex_++;
		}
	}

	
	if (GeneralProperty::View::pLastElected())
	{
		GeneralProperty::View::pLastElected()->lastElected();
	}
}


/**
 *	Get an attribute for python
 */
PyObject * GeneralEditor::pyGetAttribute( const char * attr )
{
	BW_GUARD;

	PY_GETATTR_STD();

	// see if the name matches any of our properties
	for (PropList::iterator it = properties_.begin();
		it != properties_.end();
		it++)
	{
		if (strcmp( (*it)->name(), attr ) == 0)
		{
			return (*it)->pyGet();
		}
	}

	return PyObjectPlus::pyGetAttribute( attr );
}

/**
 *	Set an attribute for python
 */
int GeneralEditor::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;

	// make sure a tool isn't applying itself
	if (ToolManager::instance().tool()->applying())
	{
		PyErr_Format( PyExc_EnvironmentError, "GeneralEditor.%s "
			"cannot set editor attributes while a tool is applying itself",
			attr );
		return NULL;
	}

	// do standard stuff
	PY_SETATTR_STD();

	// see if the name matches any of our properties
	for (PropList::iterator it = properties_.begin();
		it != properties_.end();
		it++)
	{
		if (strcmp( (*it)->name(), attr ) == 0)
		{
			return (*it)->pySet( value );
		}
	}

	return PyObjectPlus::pySetAttribute( attr, value );
}


/**
 *	This method adds the names of our properties as additional members
 *	of this object.
 */
PyObject * GeneralEditor::pyAdditionalMembers( PyObject * pBaseSeq )
{
	BW_GUARD;

	int baseSize = PySequence_Size( pBaseSeq );
	PyObject * pWholeSeq = PyTuple_New( baseSize + properties_.size() );
	for (int i = 0; i < baseSize; i++)
	{
		PyTuple_SetItem( pWholeSeq, i, PySequence_GetItem( pBaseSeq, i ) );
		// sequence returns new and tuple borrows, so this is fine
	}
	for (uint i = 0; i < properties_.size(); i++)
	{
		PyTuple_SetItem( pWholeSeq, baseSize+i,
			PyString_FromStringAndSize( properties_[i]->name(), strlen( properties_[i]->name() ) ) );
		// this one is fine with the ref counts too
	}

	Py_DECREF( pBaseSeq );
	return PyObjectPlus::pyAdditionalMembers( pWholeSeq );
}


/*~ function WorldEditor.getCurrentEditors
 *	@components{ tools }
 *
 *	This function gets the sequence of current editors.
 *
 *	@return Returns a sequence of the current editors in use.
 */
PyObject * GeneralEditor::py_getCurrentEditors( PyObject * args )
{
	BW_GUARD;

	if (PyTuple_Size(args))
	{
		PyErr_SetString( PyExc_TypeError, "WorldEditor.getCurrentEditors "
			"expects no arguments" );
		return NULL;
	}

	PyObject * pTuple = PyTuple_New( s_currentEditors_.size() );
	for (uint i = 0; i < s_currentEditors_.size(); i++)
	{
		PyObject * editor = s_currentEditors_[i].getObject();
		Py_INCREF( editor );
		PyTuple_SetItem( pTuple, i, editor );
	}
	return pTuple;
}

/*~ function WorldEditor.setCurrentEditors
 *	@components{ tools }
 *
 *	This function sets the sequence of current editors.
 *
 *	@param editors The sequence of editors to be set as the current editors.
 */
PyObject * GeneralEditor::py_setCurrentEditors( PyObject * args )
{
	BW_GUARD;

	// use the first argument as args if it's a sequence
	if (PyTuple_Size( args ) == 1 &&
		PySequence_Check( PyTuple_GetItem( args, 0 ) ))
	{
		args = PyTuple_GetItem( args, 0 );	// borrowed
	}

	Editors	newEds;

	int na = PySequence_Size( args );
	for (int i = 0; i < na; i++)
	{
		PyObject * pItem = PySequence_GetItem( args, i );	// new
		if (pItem == NULL || !GeneralEditor::Check( pItem ))
		{
			Py_XDECREF( pItem );

			PyErr_SetString( PyExc_TypeError, "WorldEditor.setCurrentEditors "
				"expects some (or one sequence of some) GeneralEditors" );
			return NULL;
		}

		newEds.push_back( GeneralEditorPtr(
			static_cast<GeneralEditor*>(pItem), true ) );
	}

	GeneralEditor::currentEditors( newEds );

	Py_Return;
}


/// static initialisers
GeneralEditor::Editors GeneralEditor::s_currentEditors_;
int GeneralEditor::s_currentEditorIndex_ = 0;
int GeneralEditor::s_lastEditorIndex_ = 0;
bool GeneralEditor::s_settingMultipleEditors_ = false;
bool GeneralEditor::s_createViews_ = true;

GeneralProperty::View* GeneralProperty::View::pLastElected_ = NULL;

// -----------------------------------------------------------------------------
// Section: PropManager
// -----------------------------------------------------------------------------


namespace
{
	std::vector<PropManager::PropFini> *s_finiCallback_ = NULL;
}


void PropManager::registerFini( PropFini fn )
{
	BW_GUARD;

	if (s_finiCallback_ == NULL)
		s_finiCallback_ = new std::vector<PropManager::PropFini>();
	s_finiCallback_->push_back(fn);
}


void PropManager::fini()
{
	BW_GUARD;

	if (s_finiCallback_ != NULL)
	{
		std::vector<PropFini>::iterator it = s_finiCallback_->begin();
		for (;it != s_finiCallback_->end(); it++)
		{
			PropFini fin = (*it);
			if (fin != NULL)
				(*fin)( );
		}
		delete s_finiCallback_;
		s_finiCallback_ = NULL;
	}
}


// -----------------------------------------------------------------------------
// Section: GeneralProperty
// -----------------------------------------------------------------------------

GENPROPERTY_VIEW_FACTORY( GeneralProperty )


/**
 *	Constructor.
 */
 GeneralProperty::GeneralProperty( const std::string & name, const std::wstring & uiname ) :
	name_( name.c_str() ),
	propManager_( NULL ),
        WBEditable_( false ),
		descName_( L"" ),
        UIName_( uiname ),
		UIDesc_( L"" ),
		exposedToScriptName_( L"" ),
		canExposeToScript_( false )
{
	GENPROPERTY_MAKE_VIEWS()
}

/**
 *	Destructor
 */
GeneralProperty::~GeneralProperty()
{
}

void GeneralProperty::WBEditable( bool editable )
{
        WBEditable_ = editable;
}

bool GeneralProperty::WBEditable() const
{
        return WBEditable_;
}

void GeneralProperty::descName( const std::wstring& descName )
{
	descName_ = descName;
}

const std::wstring& GeneralProperty::descName()
{
	return descName_;
}

void GeneralProperty::UIName( const std::wstring& name )
{
        UIName_ = name;
}

const std::wstring& GeneralProperty::UIName()
{
        if (UIName_ == L"") // If one hasn't been set
        {
			BW_GUARD;

			//Use the property name instead
			std::wstring wname;
			bw_ansitow( name_, wname );
			UIName( wname );
        }
        return UIName_;
}

void GeneralProperty::UIDesc( const std::wstring& name )
{
        UIDesc_ = name;
}

const std::wstring& GeneralProperty::UIDesc()
{
        return UIDesc_;
}

void GeneralProperty::exposedToScriptName( const std::wstring& exposedToScriptName )
{
	exposedToScriptName_ = exposedToScriptName;
}

const std::wstring& GeneralProperty::exposedToScriptName()
{
	return exposedToScriptName_;
}

void GeneralProperty::canExposeToScript( bool canExposeToScript )
{
	canExposeToScript_ = canExposeToScript;
}

bool GeneralProperty::canExposeToScript() const
{
	return canExposeToScript_;
}

/**
 *	This method notifies all the views that this property has been elected.
 *
 *	Most derived property classes will not need to override this method
 */
void GeneralProperty::elect()
{
	BW_GUARD;

	for (int i = 0; i < nextViewKindID_; i++)
	{
		if (views_[i] != NULL)
		{
			views_[i]->elect();
			if (GeneralEditor::settingMultipleEditors() &&
				views_[i]->isEditorView())
			{
				GeneralProperty::View::pLastElected( views_[i] );
			}
		}
	}
}

/**
 *	This method notifies all the views that this property has finished electing.
 *
 *	Most derived property classes will not need to override this method
 */
void GeneralProperty::elected()
{
	BW_GUARD;

	for (int i = 0; i < nextViewKindID_; i++)
	{
		if (views_[i] != NULL)
		{
			views_[i]->elected();
		}
	}
}

/**
 *	This method notifies all the views that this property has been selected as
 *  the current active property.
 *
 *	Most derived property classes will not need to override this method
 */
void GeneralProperty::select()
{
	BW_GUARD;

	for (int i = 0; i < nextViewKindID_; i++)
	{
		if (views_[i] != NULL)
		{
            //DEBUG_MSG( "GeneralProperty [%lx] : electing view %lx\n", (int)this, (int)views_[i] );
			views_[i]->select();
            //DEBUG_MSG( "...done %d\n", i );
		}
	}
}


/**
 *	This method notifies all the views that this property has been expelled.
 *
 *	Most derived property classes will not need to override this method
 */
void GeneralProperty::expel()
{
	BW_GUARD;

	for (int i = 0; i < nextViewKindID_; i++)
	{
		if (views_[i] != NULL)
		{
			views_[i]->expel();
		}
	}
}


/**
 *	Get the python object equivalent to the current value of this property
 *
 *	All classes derived from GeneralProperty should override this method
 */
PyObject * EDCALL GeneralProperty::pyGet()
{
	BW_GUARD;

	PyErr_Format( PyExc_SystemError,
		"GeneralEditor.%s does not have a get method", name_.c_str() );
	return NULL;
}

/**
 *	Set the current value of this property from an equivalent python object
 *
 *	All classes derived from GeneralProperty should override this method
 */
int EDCALL GeneralProperty::pySet( PyObject * value, bool transient,
													bool addBarrier )
{
	BW_GUARD;

	PyErr_Format( PyExc_SystemError,
		"GeneralEditor.%s does not have a set method", name_.c_str() );
	return -1;
}


/**
 *	Allocates a new view kind id and returns it. Each view kind should
 *	only call this function once (at static initialisation time) and
 *	store their own id somewhere.
 */
int GeneralProperty::nextViewKindID()
{
	return nextViewKindID_++;
}


/**
 *	Finds out if the property is visible, that is, has views. This is useful in
 *	setting up editing of common properties.
 */
bool GeneralProperty::hasViews() const
{
	BW_GUARD;

	for (int i = 0; i < nextViewKindID_; i++)
	{
		if (views_[i] != NULL)
		{
			return true;
		}
	}
	return false;
}


/**
 *	This function is a C-style version of GeneralProperty::nextViewKindID. It is
 *	exported by the DLL.
 */
int GeneralProperty_nextViewKindID()
{
	return GeneralProperty::nextViewKindID();
}

/// static initialisers
int GeneralProperty::nextViewKindID_ = 0;



// -----------------------------------------------------------------------------
// Section: GeneralProperty Views
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
GeneralProperty::Views::Views() :
	e_( new View*[ nextViewKindID_ ] )
{
	BW_GUARD;

	for (int i = 0; i < nextViewKindID_; i++)
	{
		e_[i] = NULL;
	}
}

/**
 *	Destructor.
 */
GeneralProperty::Views::~Views()
{
	BW_GUARD;

	for (int i = 0; i < nextViewKindID_; i++)
	{
		if (e_[i] != NULL)
		{
			e_[i]->deleteSelf();
		}
	}
	delete [] e_;
}


/**
 *	Set a view into the array
 */
void GeneralProperty::Views::set( int i, View * v )
{
	BW_GUARD;

	if (e_[i] != NULL)
	{
		e_[i]->deleteSelf();
	}

	e_[i] = v;
}

/**
 *	Retrieve a view from the array
 */
GeneralProperty::View * GeneralProperty::Views::operator[]( int i )
{
	BW_GUARD;

	return e_[i];
}


/**
 *	Retrieve a view from the array (const version)
 */
const GeneralProperty::View * GeneralProperty::Views::operator[]( int i ) const
{
	BW_GUARD;

	return e_[i];
}


// -----------------------------------------------------------------------------
// Section: GeneralROProperty
// -----------------------------------------------------------------------------


/**
 *	Constructor
 */
GeneralROProperty::GeneralROProperty( const std::string & name ) :
	GeneralProperty( name )
{
	GENPROPERTY_MAKE_VIEWS()
}


/**
 *	Python set method
 */
int EDCALL GeneralROProperty::pySet( PyObject * value, bool transient /*=false*/,
													bool addBarrier /*=true*/ )
{
	BW_GUARD;

	PyErr_Format( PyExc_TypeError, "Sorry, the attribute %s in "
		"GeneralEditor is read-only", name_.c_str() );
	return NULL;
}

/// static initialiser
GENPROPERTY_VIEW_FACTORY( GeneralROProperty )

// general_editor.cpp
