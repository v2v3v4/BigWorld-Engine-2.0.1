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
#include "worldeditor/misc/placement_presets.hpp"
#include "worldeditor/gui/dialogs/placement_ctrls_dlg.hpp"
#include "worldeditor/gui/pages/panel_manager.hpp"
#include "appmgr/options.hpp"


static const char* PLACECTRL_EDIT_STR = "editPresets";
static const char* PLACECTRL_NORMAL_STR = "normalPlacement";
static const char* PLACECTRL_OPTIONS_STR = "placementPreset";
static const int PLACECTRL_EDIT_INT = -2;
static const int PLACECTRL_NORMAL_INT = -1;

PlacementPresets* PlacementPresets::instance_ = 0;

PlacementPresets::PlacementPresets() :
	GUI::ActionMaker<PlacementPresets>(
		"placementPresetChanged", &PlacementPresets::presetChanged ),
	GUI::UpdaterMaker<PlacementPresets>(
		"placementPresetUpdate", &PlacementPresets::presetUpdate ),
	GUI::ActionMaker<PlacementPresets, 1>(
		"showPlacementDialog", &PlacementPresets::showDialog ),
	dialog_( 0 ),
	currentPreset_( PLACECTRL_NORMAL_STR ),
	section_( 0 ),
	presetsSection_( 0 ),
	placementXML_( "resources/data/placement.xml" )
{
}

PlacementPresets* PlacementPresets::instance()
{
	if ( !instance_ )
	{
		BW_GUARD;

		instance_ = new PlacementPresets();
	}
	return instance_;
}

/*static*/ void PlacementPresets::fini()
{
	BW_GUARD;

	delete instance_;
	instance_ = NULL;
}

void PlacementPresets::init( const std::string& placementXML )
{
	BW_GUARD;

	if ( !!section_ && !!presetsSection_ )
		return;

	section_ = BWResource::openSection( placementXML_ );
	if ( !!section_ )
		presetsSection_ = section_->openSection( "Presets", true );

	currentPreset_ = Options::getOptionString( PLACECTRL_OPTIONS_STR, PLACECTRL_NORMAL_STR );
	readPresets();
}

void PlacementPresets::readPresets()
{
	BW_GUARD;

	if ( !presetsSection_ )
		return;

	std::vector<std::string> presetsComboNames;
	std::string dialogCombo = "/PlacementToolbar/Presets";
	presetsComboNames.push_back( dialogCombo ); // Internal toolbar presets combo name
	presetsComboNames.push_back( "/UalToolbar/PlacementPresets" ); // UAL's presets combo name

	for( std::vector<std::string>::iterator i = presetsComboNames.begin();
		i != presetsComboNames.end(); ++i )
	{
		GUI::ItemPtr presets = GUI::Manager::instance()( (*i) );
		if( !presets )
			continue;

		while( presets->num() > 0 )
			presets->remove( 0 );

		// add the 'Edit Presets' item
		GUI::ItemPtr item = 0;

		if ( (*i) != dialogCombo )
		{
			item = new GUI::Item( "CHILD",
				PLACECTRL_EDIT_STR,
				section_->readString( "editPresetsString",
					LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/TOOL/PLACEMENT_PRESETS/EDIT_PRESET") ),
				"",	"", "", "showPlacementDialog", "placementPresetUpdate", "" );
			presets->add( item );
		}

		// add default non-random placement item
		item = new GUI::Item( "CHILD",
			PLACECTRL_NORMAL_STR,
			section_->readString( "normalPlacementString",
				LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/TOOL/PLACEMENT_PRESETS/NORMAL_PLACEMENT") ),
			"",	"", "", "placementPresetChanged", "placementPresetUpdate", "" );
		presets->add( item );

		// add user presets
		for( int i = 0; i < presetsSection_->countChildren(); ++i )
		{
			DataSectionPtr itemData = presetsSection_->openChild( i );
			if ( itemData )
			{
				item = new GUI::Item( "CHILD",
					itemData->readString( "name" ), itemData->readString( "displayName" ),
					"",	"", "", "placementPresetChanged", "placementPresetUpdate", "" );
				presets->add( item );
			}
		}
	}
	GUI::Manager::instance().update();

	// standard windows combo boxes
	for( std::vector<CComboBox*>::iterator c = comboBoxes_.begin();
		c != comboBoxes_.end(); ++c )
	{
		if ( !(*c)->GetSafeHwnd() )
			continue;

		(*c)->ResetContent();

		// add default non-random placement item
		int idx = (*c)->AddString( section_->readWideString( "editPresetsString",
			Localise(L"WORLDEDITOR/WORLDEDITOR/TOOL/PLACEMENT_PRESETS/EDIT_PRESET") ).c_str() );
		(*c)->SetItemData( idx, PLACECTRL_EDIT_INT );
		idx = (*c)->AddString( section_->readWideString( "normalPlacementString",
			Localise(L"WORLDEDITOR/WORLDEDITOR/TOOL/PLACEMENT_PRESETS/NORMAL_PLACEMENT") ).c_str() );
		(*c)->SetItemData( idx, PLACECTRL_NORMAL_INT );

		// add user presets
		for( int i = 0; i < presetsSection_->countChildren(); ++i )
		{
			DataSectionPtr itemData = presetsSection_->openChild( i );
			if ( itemData )
			{
				idx = (*c)->AddString( itemData->readWideString( "displayName", L"" ).c_str() );
				(*c)->SetItemData( idx, i );
			}
		}
	}
	currentPreset( currentPreset_ );
}

DataSectionPtr PlacementPresets::getCurrentPresetData()
{
	BW_GUARD;

	return getPresetData( currentPreset_ );
}

float PlacementPresets::getCurrentPresetData( DataType type, Axis axis, Value value )
{
	BW_GUARD;

	if ( defaultPresetCurrent() )
		return type == ROTATION ? 0.0f : 1.0f;

	DataSectionPtr preset = getCurrentPresetData();
	if ( !preset )
		return 0;

	std::string secName;
	secName += ( value == MIN ? "min" : "max" );
	secName += ( type == ROTATION ? "Rot" : "Sca" );
	secName += ( axis == X_AXIS ? "X" : ( axis == Y_AXIS ? "Y" : "Z" ) );
	return preset->readFloat( secName );
}

bool PlacementPresets::isCurrentDataUniform( DataType type )
{
	BW_GUARD;

	if ( defaultPresetCurrent() )
		return true;

	DataSectionPtr preset = getCurrentPresetData();
	if ( !preset )
		return true;
	
	return type == SCALE && preset->readBool( "uniformScale" );
}


DataSectionPtr PlacementPresets::getPresetData( const std::string& presetName )
{
	BW_GUARD;

	if ( !presetsSection_ )
		return 0;

	for( int i = 0; i < presetsSection_->countChildren(); ++i )
	{
		DataSectionPtr preset = presetsSection_->openChild( i );
		if ( presetName == preset->readString( "name" ) )
			return preset;
	}
	return 0;
}

void PlacementPresets::addComboBox( CComboBox* cbox )
{
	BW_GUARD;

	if ( !cbox )
		return;
	comboBoxes_.push_back( cbox );
}

void PlacementPresets::removeComboBox( CComboBox* cbox )
{
	BW_GUARD;

	if ( !cbox )
		return;
	std::vector<CComboBox*>::iterator i = std::find( comboBoxes_.begin(), comboBoxes_.end(), cbox );
	if ( i != comboBoxes_.end() )
		comboBoxes_.erase( i );
}



std::string PlacementPresets::currentPreset()
{
	return currentPreset_;
}

void PlacementPresets::currentPreset( const std::string& preset )
{
	BW_GUARD;

	currentPreset_ = preset;
	Options::setOptionString( PLACECTRL_OPTIONS_STR, currentPreset_ );

	// a little bit of hardcoding here. Shouldn't be a problem
	for( std::vector<CComboBox*>::iterator c = comboBoxes_.begin();
		c != comboBoxes_.end(); ++c )
	{
		if ( !(*c)->GetSafeHwnd() )
			continue;

		if ( currentPreset_ == PLACECTRL_NORMAL_STR )
		{
			(*c)->SetCurSel( 1 );
			continue;
		}
		for( int i = 0; i < presetsSection_->countChildren(); ++i )
		{
			DataSectionPtr preset = presetsSection_->openChild( i );
			if ( preset->readString( "name" ) == currentPreset_ )
			{
				(*c)->SetCurSel( i + 2 );
				break;
			}
		}
	}
};

void PlacementPresets::currentPreset( CComboBox* cbox )
{
	BW_GUARD;

	if ( !cbox || cbox->GetCurSel() < 0 )
		return;

	int idx = cbox->GetItemData( cbox->GetCurSel() );

	if ( idx == PLACECTRL_EDIT_INT )
	{
		showDialog();
		currentPreset( currentPreset_ );
	}
	else if ( idx == PLACECTRL_NORMAL_INT )
		currentPreset_ = PLACECTRL_NORMAL_STR;
	else if ( idx >= 0 && idx < presetsSection_->countChildren() )
	{
		DataSectionPtr preset = presetsSection_->openChild( idx );
		currentPreset_ = preset->readString( "name" );
	}
	Options::setOptionString( PLACECTRL_OPTIONS_STR, currentPreset_ );
	if ( dialog_ )
		dialog_->currentPresetChanged( currentPreset_ );
	GUI::Manager::instance().update();
};

std::string PlacementPresets::defaultPreset()
{
	return PLACECTRL_NORMAL_STR;
}

bool PlacementPresets::defaultPresetCurrent()
{
	return PLACECTRL_NORMAL_STR == currentPreset_;
}

bool PlacementPresets::currentPresetStock()
{
	BW_GUARD;

	DataSectionPtr preset = getCurrentPresetData();

	if ( PLACECTRL_NORMAL_STR != currentPreset_ &&
		preset && preset->readBool( "stockPreset" ) )
		return true;
	else
		return false;
}


void PlacementPresets::presetNames(std::vector<std::wstring> &names) const
{
	BW_GUARD;

	for( int i = 0; i < presetsSection_->countChildren(); ++i )
	{
		DataSectionPtr preset = presetsSection_->openChild( i );
		std::wstring name = preset->readWideString( "displayName" );
		if (!name.empty())
		{
			names.push_back(name);
		}
	}
}


void PlacementPresets::deleteCurrentPreset()
{
	BW_GUARD;

	DataSectionPtr data = getCurrentPresetData();
	if ( !data )
		return;

	presetsSection_->delChild( data );
	section_->save();
}

void PlacementPresets::save()
{
	BW_GUARD;

	if ( !!section_ )
		section_->save();
}

std::string PlacementPresets::getNewPresetName()
{
	BW_GUARD;

	if ( !presetsSection_ )
		return "";

	int max = 0;

	for( int i = 0; i < presetsSection_->countChildren(); ++i )
	{
		DataSectionPtr child = presetsSection_->openChild( i );
		if ( max < child->readInt( "name" ) )
			max = child->readInt( "name" );
	}

	char newName[ 80 ];
	bw_snprintf( newName, sizeof(newName), "%d", max+1 );

	return newName;
}


bool PlacementPresets::presetChanged( GUI::ItemPtr item )
{
	BW_GUARD;

	currentPreset( item->name() );
	if ( dialog_ )
		dialog_->currentPresetChanged( currentPreset_ );
	return true;
}

unsigned int PlacementPresets::presetUpdate( GUI::ItemPtr item )
{
	BW_GUARD;

	if ( currentPreset_ == item->name() )
		return 1;
	else
		return 0;
}

void  PlacementPresets::showDialog()
{
	BW_GUARD;

	showDialog( 0 );
}

bool PlacementPresets::showDialog( GUI::ItemPtr item )
{
	BW_GUARD;

	/* without guitabs
	PlacementCtrlsDlg dlg( AfxGetMainWnd() );
	dlg.DoModal();
	*/
	GUITABS::PanelHandle panel = PanelManager::instance().panels().getContent( PlacementCtrlsDlg::contentID );
	if ( !panel )
		PanelManager::instance().panels().insertPanel( PlacementCtrlsDlg::contentID, GUITABS::FLOATING );

	PanelManager::instance().panels().showPanel( panel, true );

	return true;
}
