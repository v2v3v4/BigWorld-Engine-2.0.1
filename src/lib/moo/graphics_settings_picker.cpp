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
#include "graphics_settings_picker.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/bwresource.hpp"
#include "cstdmf/unique_id.hpp"


DECLARE_DEBUG_COMPONENT2( "Moo", 0 );

static AutoConfigString s_graphicsSettingsConfigName( "system/graphicsSettingsPresets" );


using namespace Moo;


/*
 *	This class stores a VendorID and deviceID pair and allows you to 
 *	compare them it is a helper class for checking for a device 
 *	matches.
 */
class VendorDeviceIDPair
{
public:
	/*
	 *	Constructor that reads the id's from a datasection
	 *	@param pIDSection data section that contains the  VendorID 
	 *	and DeviceID pair
	 */
	VendorDeviceIDPair( DataSectionPtr pIDSection )
	{
		BW_GUARD;
		vendorID_ = pIDSection->readInt( "VendorID" );
		deviceID_ = pIDSection->readInt( "DeviceID" );
	}
	
	/*
	 *	Constructor that takes the id's as arguments
	 *	@param vendorID the vendor id
	 *	@param deviceID the device id
	 */
	VendorDeviceIDPair( uint32 vendorID, uint32 deviceID ) :
	vendorID_(vendorID),
	deviceID_(deviceID)
	{
	}
	
	/*
	 * Comparison operator
	 * @return true if the objects are identical
	 */
	bool operator == (const VendorDeviceIDPair& other) const
	{
		BW_GUARD;
		return other.vendorID_ == vendorID_ && 
			other.deviceID_ == deviceID_;
	}
private:
	uint32 vendorID_;
	uint32 deviceID_;
};

/*
 * This class is a helper class that matches a set of predefined strings
 * to a device description string.
 */
class DeviceDescriptionMatcher
{
public:
	/*
	 *	Constructor loads up a number of substrings from the provided datasection
	 */
	DeviceDescriptionMatcher( DataSectionPtr pDescriptionMatcherSection )
	{
		BW_GUARD;
		DataSectionIterator it = pDescriptionMatcherSection->begin();
		for (;it != pDescriptionMatcherSection->end();++it)
		{
			std::string substring = (*it)->asString();
			std::transform( substring.begin(), substring.end(), 
				substring.begin(), tolower );
			subStrings_.push_back( substring );
		}
	}

	/*
	 *	This method checks a device description against the substrings 
	 *	stored in this object
	 *	@param deviceDescription the device description string to check
	 *		this is assumed to be lower case
	 *	@return true if all the substrings defined in this object are
	 *		contained in the device description
	 */
	bool check( const std::string& deviceDescription ) const
	{
		BW_GUARD;
		for (uint32 i = 0; i < subStrings_.size(); ++i)
		{
			if (deviceDescription.find(subStrings_[i]) == std::string::npos)
				return false;
		}
		return subStrings_.size() != 0;
	}

private:
	std::vector<std::string> subStrings_;
};

/*
 *	This class handles a group of settings, it contains the actual graphics 
 *	settings for the group as well as any matching criteria.
 */
class Moo::SettingsGroup : public ReferenceCount
{
public:
	SettingsGroup() :
	  isDefault_( false )
	{
	}
	/*
	 *	This method inits the settings group from a data section
	 *	@param pSection the data section to init the group from
	 *	@return true if we have both matching criteria and settings
	 */
	bool init( DataSectionPtr pSection )
	{
		BW_GUARD;
		// Read our GUID matches
		std::vector<std::string> guidMatches;
		pSection->readStrings( "GUIDMatch", guidMatches );
		for (uint32 i = 0; i < guidMatches.size(); ++i)
		{
			guidMatches_.push_back( UniqueID( guidMatches[i] ) );
		}

		// Read our device and vendor id matches
		std::vector<DataSectionPtr> vendorDeviceIDMatchSections;
		pSection->openSections( "VendorDeviceIDMatch", vendorDeviceIDMatchSections );
		for (uint32 i = 0; i < vendorDeviceIDMatchSections.size(); ++i)
		{
			vendorDeviceIDMatches_.push_back( VendorDeviceIDPair( vendorDeviceIDMatchSections[i] ) );
		}

		// Read our device description matches
		std::vector<DataSectionPtr> deviceDescriptionMatchSections;
		pSection->openSections( "DeviceDescriptionMatch", deviceDescriptionMatchSections );

		for (uint32 i = 0; i < deviceDescriptionMatchSections.size(); ++i)
		{
			deviceDescriptionMatches_.push_back( DeviceDescriptionMatcher(deviceDescriptionMatchSections[i]) );
		}

		// Read the actual graphics settings
		std::vector<DataSectionPtr> settingsSections;
		pSection->openSections( "entry", settingsSections );

		for (uint32 i = 0; i < settingsSections.size(); ++i)
		{
			DataSectionPtr pSettingSection = settingsSections[i];
			std::string label = pSettingSection->readString( "label" );
			uint32 activeOption = pSettingSection->readInt( "activeOption" );
			settings_.push_back(std::make_pair( label, activeOption ) );
		}

		// Read the default setting
		isDefault_ = pSection->readBool( "defaultSetting", false );

		return (!guidMatches_.empty() || 
				!vendorDeviceIDMatches_.empty() || 
				!deviceDescriptionMatches_.empty() ||
				isDefault_) &&
			   !settings_.empty();
	}

	/*
	 *	This method checks if this SettingsGroup matches the adapter id
	 *	@param adapterID the adapter id of the device we want to use
	 *	@return the matching level, if the GUID matches, the return value
	 *		is 4 if the vendor and device id matches the return value is 3
	 *		if the device description matches the return value is 2, if there
	 *		is no match but this the default settings group, the return value 
	 *		is one, otherwise the return value is 0
	 */
	uint32 check( const D3DADAPTER_IDENTIFIER9& adapterID ) const
	{
		BW_GUARD;
		// First check if we have a GUID match, the GUID match is highest priority 
		// match
		const UniqueID& deviceId = (const UniqueID&)adapterID.DeviceIdentifier;
		for (uint32 i = 0; i < guidMatches_.size(); ++i)
		{
			// If the GUID matches, return 4
			if (guidMatches_[i] == deviceId)
				return 4;
		}

		// Check if the vendor and device id matches
		// this is the second highest match
		VendorDeviceIDPair currentVendorDeviceID( adapterID.VendorId, adapterID.DeviceId );
		for (uint32 i = 0; i < vendorDeviceIDMatches_.size(); i++)
		{
			// If the id's match, return 3
			if (vendorDeviceIDMatches_[i] == currentVendorDeviceID)
				return 3;
		}

		// Check if the device description matches, this is weakest actual match
		// We make sure the description is lower case before checking
		std::string deviceDescription = adapterID.Description;
		std::transform( deviceDescription.begin(), deviceDescription.end(), 
			deviceDescription.begin(), tolower );
		for (uint32 i = 0; i < deviceDescriptionMatches_.size(); ++i)
		{
			// If the description matches, return 2
			if (deviceDescriptionMatches_[i].check( deviceDescription ))
				return 2;
		}

		// If no matches were found, return 1 if we are the default
		// otherwise return 0, i.e. no matches found
		return isDefault_ ? 1 : 0;
	}

	const SettingsVector& settings() const { return settings_; }
	
private:
	std::vector<UniqueID> guidMatches_;
	std::vector<VendorDeviceIDPair> vendorDeviceIDMatches_;
	std::vector<DeviceDescriptionMatcher> deviceDescriptionMatches_;
	SettingsVector settings_;

	bool isDefault_;
};


/**
 *	This method inits the graphics settings picker, it loads the graphics settings
 *	config from the .xml file so that we can match it with the currently set device
 *	@return true if the settings load successfully
 */
bool GraphicsSettingsPicker::init()
{
	BW_GUARD;
	DataSectionPtr pConfig = BWResource::instance().openSection( s_graphicsSettingsConfigName.value() );
	if (pConfig.exists())
	{
		DataSectionIterator it = pConfig->begin();
		for (;it != pConfig->end(); ++it)
		{
			addSettingGroup( *it );
		}
	}
	return pConfig.exists();
}

/**
 *	This method gets the settings for a specific device
 *	@param adapterID the id of the current adapter
 *	@param out return the settings that have been selected for the adapter, 
		if no settings are found, this value remains unchanged
 *	@return true if settings for the current device were found, false 
		if not
 */
bool GraphicsSettingsPicker::getSettings( const D3DADAPTER_IDENTIFIER9& adapterID, 
														  SettingsVector& out ) const
{
	BW_GUARD;
	bool res = false;
	uint32 settingsWeight = 0;
	SettingsGroupPtr pSelected;

	// Iterate over all our setiings groups and see if they match our device
	for (uint32 i = 0; i < settingsGroups_.size(); i++)
	{
		// Get the group and check if the weight of the group
		// is higher than the currently selected weight,
		// we have found a better match
		SettingsGroupPtr pGroup = settingsGroups_[i];
		uint32 weight = pGroup->check( adapterID );
		if (weight > settingsWeight)
		{
			settingsWeight = weight;
			pSelected = pGroup;
		}
	}
	
	// If a settings group has been selected, return the settings from
	// this group.
	if (pSelected.exists())
	{
		out = pSelected->settings();
		res = true;
	}

	return res;
}

/*
 *	This method loads and adds a settings group to our list
 *	@param pSection the datasection
 */
void GraphicsSettingsPicker::addSettingGroup( DataSectionPtr pSection )
{
	BW_GUARD;
	// Load the settings group, if the load succeeded, add it to our 
	// list of settings groups
	SettingsGroupPtr pSetting = new SettingsGroup;
	if (pSetting->init(pSection))
	{
		settingsGroups_.push_back( pSetting );
	}
}

GraphicsSettingsPicker::GraphicsSettingsPicker()
{
}

GraphicsSettingsPicker::~GraphicsSettingsPicker()
{
}


// graphics_settings_picker.cpp
