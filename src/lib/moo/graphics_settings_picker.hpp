/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GRAPHICS_SETTINGS_PICKER_HPP
#define GRAPHICS_SETTINGS_PICKER_HPP

namespace Moo
{


class SettingsGroup;
typedef SmartPointer<SettingsGroup> SettingsGroupPtr;

typedef std::pair<std::string, uint32> Setting;
typedef std::vector<Setting> SettingsVector;

/**
 *	This class is a helper class that helps with picking 
 *	appropriate graphics settings for a specific device.
 *	The settings and matching criteria for the device are
 *	defined in xml files and are matched agains the 
 *	D3DADAPTER_IDENTIFIER9 struct which is queried from
 *	IDirect3D9 object, there are three tiers of
 *	matching criteria. Listed from most important to least
 *	important they are:
 *	The first one is GUID which is an exact match on a 
 *	chipset, manufacturer and driver version. 
 *	The second tier is the chipset and manufacturer using 
 *	the VendorID and DeviceID of the current graphics card.
 *	The third is a string based compare of the device
 *	description.
 */
class GraphicsSettingsPicker
{
public:
	GraphicsSettingsPicker();
	~GraphicsSettingsPicker();
	bool init();

	bool getSettings( const D3DADAPTER_IDENTIFIER9& AdapterID, SettingsVector& out ) const;

private:
	std::vector<SettingsGroupPtr> settingsGroups_;
	
	void addSettingGroup( DataSectionPtr pSection );
};

}

#endif // GRAPHICS_SETTINGS_REGISTRY_HPP
