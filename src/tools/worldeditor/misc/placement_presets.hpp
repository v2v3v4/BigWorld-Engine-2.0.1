/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PLACEMENT_PRESETS_HPP
#define PLACEMENT_PRESETS_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "guimanager/gui_functor_cpp.hpp"


class PlacementPresetsCallback
{
public:
	virtual void currentPresetChanged( const std::string& presetName ) = 0;
};

class PlacementPresets : 
	GUI::ActionMaker<PlacementPresets>,
	GUI::UpdaterMaker<PlacementPresets>,
	GUI::ActionMaker<PlacementPresets, 1>
{
public:
	enum DataType {
		ROTATION,
		SCALE
	};
	enum Axis {
		X_AXIS,
		Y_AXIS,
		Z_AXIS
	};
	enum Value {
		MIN,
		MAX
	};
	static PlacementPresets* instance();
	static void fini();

	void init( const std::string& placementXML );

	void readPresets();

	std::string xmlFilePath() { return placementXML_; };

	DataSectionPtr rootSection() { return section_; };
	DataSectionPtr presetsSection() { return presetsSection_; };

	DataSectionPtr getCurrentPresetData();
	float getCurrentPresetData( DataType type, Axis axis, Value value );
	bool isCurrentDataUniform( DataType type );
	DataSectionPtr getPresetData( const std::string& presetName );
	
	void addComboBox( CComboBox* cbox );
	void removeComboBox( CComboBox* cbox );

	std::string currentPreset();
	void currentPreset( const std::string& preset );
	void currentPreset( CComboBox* cbox );
	std::string defaultPreset();
	bool defaultPresetCurrent();
	bool currentPresetStock();
	void presetNames(std::vector<std::wstring> &names) const;

	void dialog( PlacementPresetsCallback* dialog ) { dialog_ = dialog; };

	void deleteCurrentPreset();
	void save();
	std::string getNewPresetName();

	void showDialog();

private:
	PlacementPresets();
	static PlacementPresets* instance_;

	std::vector<CComboBox*> comboBoxes_;

	std::string currentPreset_;
	PlacementPresetsCallback* dialog_;
	DataSectionPtr section_;
	DataSectionPtr presetsSection_;
	std::string placementXML_;

	bool presetChanged( GUI::ItemPtr item );
	unsigned int presetUpdate( GUI::ItemPtr item );
	bool showDialog( GUI::ItemPtr item );
};


#endif // PLACEMENT_PRESETS_HPP
