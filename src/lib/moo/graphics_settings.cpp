/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// Module Interface
#include "pch.hpp"
#include "graphics_settings.hpp"
#include "graphics_settings_picker.hpp"

// BW Tech Headers
#include "resmgr/dataresource.hpp"
#include "cstdmf/unique_id.hpp"

DECLARE_DEBUG_COMPONENT2( "Moo", 0 );

namespace Moo
{

// -----------------------------------------------------------------------------
// Section: GraphicsSetting
// -----------------------------------------------------------------------------

/**
 *	Constructor. When this setting is added to the registry, the active option 
 *	will be either reset to the first supported option or to the one restored 
 *	from the options file, if the provided activeOption is -1. If, instead, the
 *	provided activeOption is a non negative value, it will either be reset to 
 *	the one restored from the options file (if this happens to be different 
 *	from the value passed as parameter) or not be reset at all.
 * 
 *	@param	label			label for this setting.
 *	@param	activeOption	initial active option (see above).
 *	@param	delayed			delayed flag.
 *	@param	needsRestart	needsRestart flag.
 */
GraphicsSetting::GraphicsSetting(
	const std::string & label, 
	const std::string & desc, 
	              int   activeOption, 
	             bool   delayed,
	             bool   needsRestart) :
	label_(label),
	desc_(desc),
	options_(),
	activeOption_(activeOption),
	delayed_(delayed),
	needsRestart_(needsRestart)
{}

/**
 *	Destructor.
 */
GraphicsSetting::~GraphicsSetting()
{}

/**
 *	Retrieves the label for this setting. Applications must translate 
 *	this to the local human readable string representation.
 *
 *	@return		label for this setting.
 */
const std::string & GraphicsSetting::label() const
{
	return this->label_;
}

/**
 *	Retrieves the description for this setting.
 *
 *	@return		description for this setting.
 */
const std::string & GraphicsSetting::desc() const
{
	return this->desc_;
}

/**
 *	Adds new option for this setting.
 *
 *	@param	label		label for this option.
 *	@param	isSupported	true if this option is supported be the system.
 *
 *	@return	index to the newly added option.
 */
int GraphicsSetting::addOption(const std::string & label, const std::string & desc, bool isSupported)
{
	BW_GUARD;
	this->options_.push_back( std::make_pair(label, std::make_pair(desc, isSupported)));
	return this->options_.size() - 1;
}

/**
 *	Returns list of options for this setting.
 */
const GraphicsSetting::StringStringBoolVector & GraphicsSetting::options() const
{
	return this->options_;
}

/**
 *	Selects new option. Will trigger onOptionSelected. 
 *	If the chosen options is not supported, the call will be ignored.
 *
 *	@param	optionIndex	index to option being selected.
 *	@param	force		Boolean indicating whether to force the option to be
 *						set, even if it's not supported.
 */
bool GraphicsSetting::selectOption(int optionIndex,  bool force)
{
	BW_GUARD;
	// must be in this order, because needsRestart_ may change after
	// this->selectOptionPriv() gets called.
	if (!this->selectOptionPriv(optionIndex, force))
	{
		return false;
	}
	if ( this->needsRestart_)
	{
		GraphicsSetting::s_needsRestart = true;
	}
	return true;
}

/**
 *	Returns currently active option.
 */
int GraphicsSetting::activeOption() const
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV(this->activeOption_>=0)
	{
		MF_EXIT( "active option is < 0" );
	}

	return this->activeOption_;
}

/**
 * Do we need to restart now?
 */
void GraphicsSetting::needsRestart( bool a_needsRestart )
{
	this->needsRestart_ = a_needsRestart;
}

/**
 *	Registers another GraphicsSetting object as an slave to this one. Slave 
 *	settings are not listed with GraphicsSetting::settings() and they have
 *	their activeOption property updated and onOptionSelected method triggered 
 *	whenever their master's active option changes.
 *
 *	@param	slave	GraphicsSetting object to be added as slave.
 */
void GraphicsSetting::addSlave(GraphicsSettingPtr slave)
{
	BW_GUARD;
	// effects must have the exact same options
	if (slave->options() == this->options())
	{
		GraphicsSettingVector & slaves = this->slaves_;
		GraphicsSettingVector::iterator it = std::find(
			slaves.begin(), slaves.end(), slave);

		if (it == slaves.end())
		{
			slaves.push_back(slave);	
		}
		else
		{
			ERROR_MSG( "GraphicsSetting::addSlave: Failed, slave is already in the master's list.\n" );
		}
	}
	else
	{
		ERROR_MSG( "GraphicsSetting::addSlave: Failed, options differ between slave and master.\n" );
	}
}

/**
 *	This method inits the graphics settings.
 *	The settings are either read from the data section provided
 *	If the graphics device or driver has changed since the last time
 *	the settings were saved, the settings are auto-detected.
 *
 *	@param	section DataSection where to load the options from.
 */
void GraphicsSetting::init(DataSectionPtr section)
{
	BW_GUARD;

	SettingsVector tempSettings;

	// Load up the settings from the configuration file and fill in 
	// the tempSettings vector, these settings will be used if the device
	// matches the device last used.
	typedef std::vector<DataSectionPtr> EntrySections;
	EntrySections entrySections;

	if (section.exists())
		section->openSections( "entry", entrySections );

	EntrySections::iterator sectIt = entrySections.begin();
	EntrySections::iterator sectEnd = entrySections.end();

	for (;sectIt < sectEnd;++sectIt)
	{
		std::string label = (*sectIt)->readString("label");
		uint32 activeOption   = (*sectIt)->readInt("activeOption");
		if (!label.empty())
		{
			tempSettings.push_back( std::make_pair( label, activeOption ) );
		}
	}

	// Get the stored GUID from the configuration file and the GUID for
	// the current device
	const DeviceInfo& di = rc().deviceInfo( 0 );
	std::string deviceGUIDString;
	if (section.exists())
		deviceGUIDString = section->readString( "deviceGUID" );

	UniqueID oldGUID(deviceGUIDString);
	UniqueID newGUID = (UniqueID&)(di.identifier_.DeviceIdentifier);

	// If the GUID's are the same, we don't do anything
	// If they are different, we try to autodetect the settings
	// for the current device
	if (oldGUID == newGUID)
	{
		INFO_MSG("GraphicsSetting::init - the graphics card/driver matches so we don't autodetect settings\n");
	}
	else
	{
		// Use the graphics settings picker to try to autodetect the optimal settings
		// for the current device.
		GraphicsSettingsPicker picker;
		picker.init();
		bool foundSettings = picker.getSettings( di.identifier_, tempSettings );
		INFO_MSG("GraphicsSetting::init - autodetection %s\n", foundSettings ? "succeeded" : "failed" );
	}

	// Iterate over the temp settings and set them all up
	for (uint32 i = 0; i < tempSettings.size(); i++)
	{
		std::string label = tempSettings[i].first;
		int activeOption   = tempSettings[i].second;
		GraphicsSettingPtr setting = GraphicsSetting::getFromLabel(label);
		if (setting.exists())
		{	
			setting->selectOption(activeOption);
		}
		else
		{
			GraphicsSetting::s_latentSettings[label] = activeOption;
		}
	}

	GraphicsSetting::commitPending();
}

/**
 *	Writes graphics options into DataSection provided.
 *
 *	@param	section DataSection where to save the options into.
 */
void GraphicsSetting::write(DataSectionPtr section)
{
	BW_GUARD;
	section->delChildren();

	const DeviceInfo& di = rc().deviceInfo( 0 );
	UniqueID deviceGUID = (UniqueID&)(di.identifier_.DeviceIdentifier);

	section->writeString( "deviceGUID", deviceGUID.toString() );

	GraphicsSettingVector::const_iterator setIt  = GraphicsSetting::s_settings.begin();
	GraphicsSettingVector::const_iterator setEnd = GraphicsSetting::s_settings.end();
	while (setIt != setEnd)
	{
		int optionInSetting = -1;
		if (!GraphicsSetting::isPending((*setIt), optionInSetting))
		{
			optionInSetting = (*setIt)->activeOption();
		}

		DataSectionPtr settingsEntry = section->newSection( "entry" );
		settingsEntry->writeString("label", (*setIt)->label());
		settingsEntry->writeInt("activeOption", optionInSetting);
		++setIt;
	} 
}

/**
 *	Registers new GraphicsSetting into global registry. 
 *  
 *	More than one setting can be registered under the same entry. For this to 
 *	happen, they must share the same description, have the same number of 
 *	options, each with the same descripition text and appearing in the same 
 *	order. If two or more effects shared the same description, but the above 
 *	rules are not respected, an assertion will fail. 
 *
 *	@param	setting	The new GraphicsSetting to register.
 */
void GraphicsSetting::add(GraphicsSettingPtr setting)
{
	BW_GUARD;
	int optionInSetting = -1;
	if (!GraphicsSetting::isPending(setting, optionInSetting))
	{
		optionInSetting = setting->activeOption_;
	}

	const StringIntMap & latentSettings = GraphicsSetting::s_latentSettings;
	const std::string & label = setting->label();
	StringIntMap::const_iterator setIt = latentSettings.find(label);
	
	// if we have an existing setting with this name, use the existing selected option.
	GraphicsSettingPtr existingSettings = GraphicsSetting::getFromLabel( label );
	if (existingSettings)
	{
		int existingOption = existingSettings->activeOption();

		if (existingOption != optionInSetting)
		{
			// don't update needRestart
			setting->selectOptionPriv(existingOption);
		}
	}
	else if (setIt != latentSettings.end())
	{
		// look for option  loaded from file
		const int & optionInFile = setIt->second;

		// if option read from file is different
		// from the one currently set, reset it.
		if (optionInFile != optionInSetting)
		{
			// don't update needRestart
			setting->selectOptionPriv(optionInFile);
		}
	}
	// if the one currently set is negative,
	// reset it to the first supported value
	else if (optionInSetting < 0)
	{
		// don't update needRestart
		setting->selectOptionPriv(0);
	}
	// otherwise, assume engine is already running with the 
	// current activeOption setting. Don't call selectOption

	GraphicsSettingPtr master = GraphicsSetting::getFromLabel(label);
	if (master.exists())
	{
		master->addSlave(setting);
	}
	else 
	{
		GraphicsSetting::s_settings.push_back(setting);
	}
}

/**
 *	Returns list of registered settings.
 */
const GraphicsSetting::GraphicsSettingVector & GraphicsSetting::settings()
{
	return GraphicsSetting::s_settings;
}

/**
 *	Returns true if there are any pending settings that require commit. 
 *	Settings flagged with &lt;delayed> do not have their activeOption property 
 *	and onOptionSelected method triggered immediatly. Instead, they are added 
 *	to a pending list. Use this method to query whether there are any settings 
 *	in the pending list.
 */
bool GraphicsSetting::hasPending()
{
	return !GraphicsSetting::s_pending.empty();
}


/**
 *	Returns true if the given setting object is currently pending.
 *
 *	@param	setting				setting to check for pending status.
 *	@param	o_pendingOption		(out) value of active option pending.
 *
 *	@return						True if setting is pending.
 */
bool GraphicsSetting::isPending(GraphicsSettingPtr setting, int & o_pendingOption)
{
	BW_GUARD;
	bool result = false;
	SettingIndexVector::iterator pendingIt  = GraphicsSetting::s_pending.begin();
	SettingIndexVector::iterator pendingEnd = GraphicsSetting::s_pending.end();
	while (pendingIt != pendingEnd)
	{
		if (pendingIt->first == setting)
		{
			o_pendingOption = pendingIt->second;
			result = true;
			break;
		}
		++pendingIt;
	}
	return result;
}

/**
 *	Commits all pending settings. Causes the activeOption property on pending 
 *	settings to be updated and their onOptionSelected method to be triggered.
 */
void GraphicsSetting::commitPending()
{
	BW_GUARD;
	SettingIndexVector::iterator pendingIt  = GraphicsSetting::s_pending.begin();
	while (pendingIt != GraphicsSetting::s_pending.end())
	{
		int index = pendingIt->second;
		pendingIt->first->activeOption_ = index;
		pendingIt->first->onOptionSelected(index);

		// notify all slaves
		GraphicsSettingVector::iterator slvIt  = pendingIt->first->slaves_.begin();
		GraphicsSettingVector::iterator slvEnd = pendingIt->first->slaves_.end();
		while (slvIt != slvEnd)
		{
			(*slvIt)->activeOption_ = index;
			(*slvIt)->onOptionSelected(index);
			++slvIt;
		}
		pendingIt = GraphicsSetting::s_pending.erase(pendingIt);
	}
}

/**
 *	Rolls back all pending settings. This will cause the engine 
 *	to ignore all options changes that are currently pending and 
 *	clear the pending serrings list.
 */
void GraphicsSetting::rollbackPending()
{
	 GraphicsSetting::s_pending.clear();
}

/**
 *	Returns true if, since the last time this method was called, 
 *	there has been any settings change that requires the application 
 *	to be restarted to come into effect.
 */
bool GraphicsSetting::needsRestart()
{
	return GraphicsSetting::s_needsRestart;
}

/**
 *	Does the internal work related to setting the current active option.
 */
bool GraphicsSetting::selectOptionPriv(int optionIndex, bool force)
{
	BW_GUARD;
	// keep option in the range [0, size-1]
	int index = std::max(0, std::min(optionIndex, int(this->options_.size()-1)));

	// make sure it's supported. If it's not,
	// look for next supported option in list.
	if (!force)
	{
		for (; index<int(this->options_.size()); ++index)
		{
			if (this->options_[index].second.second) // supported?
			{
				break;
			}
		}
		if (index == this->options_.size())
		{
			ERROR_MSG("GraphicsSetting::selectOptionPriv - no supported option found for %s graphic setting\n", label_.c_str());	
			return false;
		}
	}

	bool applied = false;
	if (!this->delayed_ || force)
	{
		if (index != this->activeOption_ || force)
		{
			this->activeOption_ = index;

			// notify derived objects
			this->onOptionSelected(index);

			// notify all slaves
			GraphicsSettingVector::iterator slaveIt  = this->slaves_.begin();
			GraphicsSettingVector::iterator slaveEnd = this->slaves_.end();
			while (slaveIt != slaveEnd)
			{
				(*slaveIt)->activeOption_ = index;
				(*slaveIt)->onOptionSelected(index);
				++slaveIt;
			}
			applied = true;
		}
	}
	else
	{
		applied = GraphicsSetting::addPending(this, index);
	}
	return applied;
}

/**
 *	Adds a setting to the pending list (private).
 */
bool GraphicsSetting::addPending(GraphicsSettingPtr setting, int optionIndex)
{
	BW_GUARD;
	// if setting is already pending,
	// just reset the optionIndex
	bool applied = false;
	bool found = false;
	SettingIndexVector::iterator pendingIt  = GraphicsSetting::s_pending.begin();
	SettingIndexVector::iterator pendingEnd = GraphicsSetting::s_pending.end();
	while (pendingIt != pendingEnd)
	{
		if (pendingIt->first == setting)
		{
			if (optionIndex == setting->activeOption_)
			{
				// new setting is redundant (the same as the one 
				// currently in use). Remove setting from pending list
				GraphicsSetting::s_pending.erase(pendingIt);
			}
			else
			{
				pendingIt->second = optionIndex;
				applied = true;
			}
			found = true;
			break;
		}
		++pendingIt;
	}

	// otherwise, add new entry, but only if new option is 
	// different from the one currently set on settings 
	// (selectOption does not filter for delayed settings)
	if (!found && optionIndex != setting->activeOption_)
	{
		applied = false;
		GraphicsSetting::s_pending.push_back(
			std::make_pair(setting.getObject(), optionIndex));
	}
	return applied;
}

/**
 *	Returns registered GraphicsSetting object from given label. If
 *	a suitable GraphicsSetting object is not found, returns empty pointer.
 *
 *	@param	label	label of setting to retrieve.
 */
GraphicsSetting::GraphicsSettingPtr GraphicsSetting::getFromLabel(
	const std::string & label)
{
	BW_GUARD;
	GraphicsSettingVector::const_iterator setIt  = GraphicsSetting::s_settings.begin();
	GraphicsSettingVector::const_iterator setEnd = GraphicsSetting::s_settings.end();
	while (setIt != setEnd)
	{
		if ((*setIt)->label() == label)
		{
			break;
		}
		++setIt;
	}
	return setIt != setEnd ? *setIt : 0;
}

GraphicsSetting::SettingIndexVector GraphicsSetting::s_pending;
GraphicsSetting::GraphicsSettingVector GraphicsSetting::s_settings;
GraphicsSetting::StringIntMap GraphicsSetting::s_latentSettings;
bool GraphicsSetting::s_needsRestart = false;
std::string GraphicsSetting::s_filename = "";

} // namespace moo

// settings_registry.cpp
