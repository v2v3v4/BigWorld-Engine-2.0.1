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
#include "py_graphics_setting.hpp"
#pragma warning( disable:4786 )		//This used in member initialisation list.

DECLARE_DEBUG_COMPONENT2( "Romp", 0 )


// -----------------------------------------------------------------------------
// Section: PyCallbackGraphicsSetting
// -----------------------------------------------------------------------------
PyCallbackGraphicsSetting::PyCallbackGraphicsSetting(
	const std::string & label, 
	const std::string & desc,
	int                 activeOption,
	bool                delayed,
	bool                needsRestart,
	PyObject*			callback):
	GraphicsSetting( label, desc, activeOption, delayed, needsRestart ),
	pCallback_( callback, false )
{
}


/**
 *	This method is called by the Graphics Settings system when our
 *	setting is changed.  The newly selected option is passed in.
 *
 *	@param	optionIndex		The newly selected option in our graphics setting.
 */
void PyCallbackGraphicsSetting::onOptionSelected(int optionIndex)
{
	if (pCallback_.hasObject())
	{
		Py_INCREF( pCallback_.getObject() );
		Script::call( pCallback_.getObject(), Py_BuildValue( "(i)", optionIndex ),
			"PyCallbackGraphicsSetting::onOptionSelected" );
	}
}


void PyCallbackGraphicsSetting::pCallback( PyObjectPtr pc )
{
	pCallback_ = pc;
}


PyObjectPtr PyCallbackGraphicsSetting::pCallback() const
{
	return pCallback_;
}


// -----------------------------------------------------------------------------
// Section: PyGraphicsSetting
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyGraphicsSetting )
PY_BEGIN_METHODS( PyGraphicsSetting )
	/*~ function PyGraphicsSetting.addOption
	 *	@components{ client, tools }
	 * This method adds an option to the graphics setting.
	 *	@param	label		UI label to display for the option
	 *	@param	desc		More detailed help string for the option.
	 *	@param	isSupported Whether the current host system supports this option.
	 *	@return	int			Index to the newly added option.
	 */
	PY_METHOD( addOption )
	/*~ function PyGraphicsSetting.registerSetting
	 *	@components{ client, tools }
	 * This method registers the graphics setting with the Graphics Setting
	 * registry.  It should be called only after options have been added to
	 * the graphics setting.
	 */
	PY_METHOD( registerSetting )
PY_END_METHODS()
PY_BEGIN_ATTRIBUTES( PyGraphicsSetting )
	/*~ attribute PyGraphicsSetting.callback
	 *	@components{ client, tools }
	 * This attribute specifies the callback function.  The function is called
	 * when this graphics setting has a new option selected. The callback function
	 * takes a single Integer parameter, the newly selected option index.
	 */
	PY_ATTRIBUTE( callback )
PY_END_ATTRIBUTES()

PY_SCRIPT_CONVERTERS( PyGraphicsSetting )

/*~ function BigWorld.GraphicsSetting
 *	@components{ client, tools }
 *
 *	This function creates a PyGraphicsSetting.
 *
 *	For example:
 *	@{
 *	def onOptionSelected( idx ):
 *		print "graphics setting option %d selected" % (idx,)
 *
 *	gs = BigWorld.GraphicsSetting( "label", "description", -1, False, False, onOptionSelected )
 *	gs.addOption( "High", "High Quality", True )
 *	gs.addOption( "Low", "Low Quality", True )
 *	gs.registerSetting()	#always add after the options have been added.
 *
 *	@}
 *
 *	@param		String		Short label to display in the UI.
 *	@param		String		Longer description to display in the UI.
 *	@param		Int			Active option index.	When this setting is added
 *							to the registry, the active option will be either
 *							reset to the first supported option or to the one
 *							restored from the options file, if the provided
 *							active option index is -1.  If, instead, the
 *							provided activeOption is a non negative value, it
 *							will either be reset to the one restored from the
 *							options file (if this happens to be different from
 *							the value passed as parameter) or not be reset at
 *							all.
 *	@param		Boolean		Delayed - apply immediately on select, or batched up.
 *	@param		Boolean		Requires Restart - game needs a restart to take effect.
 *	@param		PyObject	Callback function.
 *
 *	@return		a new PyGraphicsSetting
 */
PY_FACTORY_NAMED( PyGraphicsSetting, "GraphicsSetting", BigWorld )


//constructor
PyGraphicsSetting::PyGraphicsSetting(
	PyCallbackGraphicsSettingPtr pSetting,
	PyTypePlus * pType ):
	PyObjectPlus( pType ),
	pSetting_( pSetting )
{
}


/**
 *	This method is callable from python, and adds an option to
 *	our graphics setting.
 *	@param	label		UI label to display for the option
 *	@param	desc		More detailed help string for the option.
 *	@param	isSupported Whether the current host system supports this option.
 *	@return	int			Index to the newly added option.
 */
int PyGraphicsSetting::addOption(
	const std::string & label,
	const std::string & desc,
	bool isSupported )
{
	return pSetting_->addOption( label, desc, isSupported );
}


/**
 *	This method is callable from python, and registers our graphics
 *	setting.  This should be called after the settings options have
 *	been added.  Once added, there is no need to ever remove.
 */
void PyGraphicsSetting::registerSetting()
{
	Moo::GraphicsSetting::add( pSetting_ );
}


/**
 *	Get an attribute for python
 */
PyObject * PyGraphicsSetting::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();
	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	Set an attribute for python
 */
int PyGraphicsSetting::pySetAttribute( const char * attr, PyObject * value )
{
	PY_SETATTR_STD();
	return PyObjectPlus::pySetAttribute( attr, value );
}


/**
 *	Factory method.
 */
PyObject * PyGraphicsSetting::pyNew( PyObject * args )
{
	char * label, * desc;
	int activeOption;
	bool delayed;
	bool needsRestart;
	PyObject* callback;

	if (!PyArg_ParseTuple( args, "ssibbO", &label, &desc, &activeOption, &delayed, &needsRestart, &callback ))
	{		
		PyErr_SetString( PyExc_TypeError,
			"BigWorld.PyGraphicsSetting() expects "
			"a label (string), description (string), default active option "
			"(int), delayed (bool) needsRetart (bool) and callback function "
			"(Python Object)." );
		return NULL;
	}

	PyCallbackGraphicsSettingPtr pSettings = new PyCallbackGraphicsSetting(
		std::string(label), std::string(desc), activeOption, delayed, needsRestart, callback );

	return new PyGraphicsSetting( pSettings );
}


//-----------------------------------------------------------------------------
// section - python access to static Moo::GraphicsSetting methods
//-----------------------------------------------------------------------------

/*~ function BigWorld.graphicsSettings
 *	@components{ client,  tools }
 *
 *  Returns list of registered graphics settings
 *	@return	list of 4-tuples in the form (label : string, index to active
 *			option in options list: int, options : list, desc : string). Each option entry is a
 *			3-tuple in the form (option label : string, support flag : boolean, desc : string).
 */
static PyObject * graphicsSettings()
{
	typedef Moo::GraphicsSetting::GraphicsSettingVector GraphicsSettingVector;
	const GraphicsSettingVector & settings = Moo::GraphicsSetting::settings();
	PyObject * settingsList = PyList_New(settings.size());

	GraphicsSettingVector::const_iterator setIt  = settings.begin();
	GraphicsSettingVector::const_iterator setEnd = settings.end();
	while (setIt != setEnd)
	{
		typedef Moo::GraphicsSetting::StringStringBoolVector StringStringBoolVector;
		const StringStringBoolVector & options = (*setIt)->options();
		PyObject * optionsList = PyList_New(options.size());
		StringStringBoolVector::const_iterator optIt  = options.begin();
		StringStringBoolVector::const_iterator optEnd = options.end();
		while (optIt != optEnd)
		{
			PyObject * optionItem = PyTuple_New(3);
			PyTuple_SetItem(optionItem, 0, Script::getData(optIt->first)); // Label
			PyTuple_SetItem(optionItem, 1, Script::getData(optIt->second.second)); // Enabled
			PyTuple_SetItem(optionItem, 2, Script::getData(optIt->second.first)); // Description

			int optionIndex = std::distance(options.begin(), optIt);
			PyList_SetItem(optionsList, optionIndex, optionItem);
			++optIt;
		}
		PyObject * settingItem = PyTuple_New(4);
		PyTuple_SetItem(settingItem, 0, Script::getData((*setIt)->label()));
		PyTuple_SetItem(settingItem, 3, Script::getData((*setIt)->desc()));

		// is setting is pending, use value stored in pending
		// list. Otherwise, use active option in setting
		int activeOption = 0;
		if (!Moo::GraphicsSetting::isPending(*setIt, activeOption))
		{
			activeOption = (*setIt)->activeOption();
		}
		PyTuple_SetItem(settingItem, 1, Script::getData(activeOption));
		PyTuple_SetItem(settingItem, 2, optionsList);

		int settingIndex = std::distance(settings.begin(), setIt);
		PyList_SetItem(settingsList, settingIndex, settingItem);
		++setIt;
	}

	return settingsList;
}
PY_AUTO_MODULE_FUNCTION( RETOWN, graphicsSettings, END, BigWorld )


/*~ function BigWorld.setGraphicsSetting
 *	@components{ client,  tools }
 *
 *  Sets graphics setting option.
 *
 *  Raises a ValueError if the given label does not name a graphics setting, if
 *  the option index is out of range, or if the option is not supported.
 *
 *	@param	label		    string - label of setting to be adjusted.
 *	@param	optionIndex		int - index of option to set.
 */
static bool setGraphicsSetting( const std::string label, int optionIndex )
{

	bool result = false;
	typedef Moo::GraphicsSetting::GraphicsSettingVector GraphicsSettingVector;
	GraphicsSettingVector settings = Moo::GraphicsSetting::settings();
	GraphicsSettingVector::const_iterator setIt  = settings.begin();
	GraphicsSettingVector::const_iterator setEnd = settings.end();
	while (setIt != setEnd)
	{
		if ((*setIt)->label() == label)
		{
			if (optionIndex < int( (*setIt)->options().size() ))
			{
				if ((*setIt)->options()[optionIndex].second.second)
				{
					(*setIt)->selectOption( optionIndex );
					result = true;
				}
				else
				{
					PyErr_SetString( PyExc_ValueError,
						"Option is not supported." );
				}
			}
			else
			{
				PyErr_SetString( PyExc_ValueError,
					"Option index out of range." );
			}
			break;
		}
		++setIt;
	}
	if (setIt == setEnd)
	{
		PyErr_SetString( PyExc_ValueError,
			"No setting found with given label." );
	}

	return result;
}
PY_AUTO_MODULE_FUNCTION( RETOK, setGraphicsSetting,
	ARG( std::string, ARG( int, END ) ), BigWorld )


/*~ function BigWorld.getGraphicsSetting
 *	@components{ client,  tools }
 *
 *  Gets graphics setting option.
 *
 *  Raises a ValueError if the given label does not name a graphics setting, if
 *  the option index is out of range, or if the option is not supported.
 *
 *	@param	label		    string - label of setting to be retrieved.
 *	@return optionIndex		int - index of option.
 */
static int getGraphicsSetting( const std::string label )
{
	bool result = false;
	typedef Moo::GraphicsSetting::GraphicsSettingVector GraphicsSettingVector;
	GraphicsSettingVector settings = Moo::GraphicsSetting::settings();
	GraphicsSettingVector::const_iterator setIt  = settings.begin();
	GraphicsSettingVector::const_iterator setEnd = settings.end();
	while (setIt != setEnd)
	{
		if ((*setIt)->label() == label)
		{
			return ((*setIt)->activeOption());
		}

		++setIt;
	}

	if (setIt == setEnd)
	{
		PyErr_SetString( PyExc_ValueError,
			"No setting found with given label." );
	}

	return 0;
}
PY_AUTO_MODULE_FUNCTION( RETDATA, getGraphicsSetting,
	ARG( std::string, END ), BigWorld )


/*~ function BigWorld.commitPendingGraphicsSettings
 *	@components{ client,  tools }
 *
 *  This function commits any pending graphics settings. Some graphics
 *  settings, because they may block the game for up to a few minutes when
 *  coming into effect, are not committed immediately. Instead, they are
 *  flagged as pending and require commitPendingGraphicsSettings to be called
 *  to actually apply them.
 */
static void commitPendingGraphicsSettings()
{
	Moo::GraphicsSetting::commitPending();
}
PY_AUTO_MODULE_FUNCTION( RETVOID, commitPendingGraphicsSettings, END, BigWorld )


/*~ function BigWorld.hasPendingGraphicsSettings
 *	@components{ client,  tools }
 *
 *  This function returns true if there are any pending graphics settings.
 *  Some graphics settings, because they may block the game for up to a few
 *  minutes when coming into effect, are not committed immediately. Instead,
 *  they are flagged as pending and require commitPendingGraphicsSettings to be
 *  called to actually apply them.
 */
static bool hasPendingGraphicsSettings()
{
	return Moo::GraphicsSetting::hasPending();
}
PY_AUTO_MODULE_FUNCTION( RETDATA, hasPendingGraphicsSettings, END, BigWorld )


/*~ function BigWorld.graphicsSettingsNeedRestart
 *	@components{ client,  tools }
 *
 *  This function returns true if any recent graphics setting change
 *	requires the client to be restarted to take effect. If that's the
 *	case, restartGame can be used to restart the client. The need restart
 *	flag is reset when this method is called.
 */
static bool graphicsSettingsNeedRestart()
{
	return Moo::GraphicsSetting::needsRestart();
}
PY_AUTO_MODULE_FUNCTION( RETDATA, graphicsSettingsNeedRestart, END, BigWorld )


/*~ function BigWorld.autoDetectGraphicsSettings
 *	@components{ client,  tools }
 *  
 *	Automatically detect the graphics settings
 *	based on the client's system properties.
 */
static void autoDetectGraphicsSettings()
{
	// Init GraphicsSettings with an empty DataSection to autoDetect settings.
	Moo::GraphicsSetting::init( NULL );
}
PY_AUTO_MODULE_FUNCTION( RETVOID, autoDetectGraphicsSettings, END, BigWorld )


/*~ function BigWorld.rollBackPendingGraphicsSettings
 *	@components{ client,  tools }
 *
 *  This function rolls back any pending graphics settings.
 */
static void rollBackPendingGraphicsSettings()
{
	return Moo::GraphicsSetting::rollbackPending();
}
PY_AUTO_MODULE_FUNCTION( RETVOID,
		rollBackPendingGraphicsSettings, END, BigWorld )



// py_graphics_setting.cpp
