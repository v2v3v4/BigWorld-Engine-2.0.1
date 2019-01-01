/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// weather_settings_table.cpp : implementation file
//
#include "pch.hpp"
#include "weather_settings_table.hpp"
#include "user_messages.hpp"
#include "gizmo/general_editor.hpp"
#include "gizmo/general_properties.hpp"
#include "resmgr/xml_section.hpp"
#include "ual/ual_manager.hpp"

DECLARE_DEBUG_COMPONENT( 0 )


///////////////////////////////////////////////////////////////////////////////
// Section: WeatherSettingsTable
///////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor
 */
WeatherSettingsTable::WeatherSettingsTable( UINT dlgId )
	: PropertyTable( dlgId )
	, pSystem_( NULL )	
{
}


/**
 *	Destructor
 */
WeatherSettingsTable::~WeatherSettingsTable()
{
}


void WeatherSettingsTable::initDragDrop()
{
	BW_GUARD;

	UalManager::instance().dropManager().add(
		new UalDropFunctor< WeatherSettingsTable >(
			propertyList(),
			"xml",
			this,
			&WeatherSettingsTable::doDrop,
			false,
			&WeatherSettingsTable::dropTest ) );
}


/**
 *	Initialises the property list from the weather settings.
 *
 *	@param			DataSectionPtr	weather system information.
 *  @returns		true if successful, false otherwise.
 */
bool WeatherSettingsTable::init( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!pSection)
		return false;

	if (pSection == pSystem_)
		return false;

	pSystem_ = pSection;

	PropTableSetter setter( this );

	int oldSel = propertyList()->GetCurSel();
	propertyList()->deselectCurrentItem();

	if ( editor_ )
	{
		editor_->expel();
	}

	// Must tell the smartpointer that the reference is already incremented,
	// because the PyObjectPlus base class increments the refcnt (!)
	editor_ = GeneralEditorPtr( new GeneralEditor(), true );

	GeneralProperty * gp ;
	std::wstring sectionName = bw_utf8tow( pSystem_->sectionName() + "/" );	

	//fx	string
	TextProperty* tp = new TextProperty(
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/SPECIAL_EFFECT_FILE" ),
		new DataSectionStringProxy(
			pSystem_, "sfx", "onWeatherAdjust", "" ));	
	tp->fileFilter( L"Special Effect files(*.xml;*.sfx)|*.xml;*.sfx||" );
	tp->setGroup( sectionName );
	tp->UIDesc( Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/SPECIAL_EFFECT_FILE/TOOLTIP" ) );
	editor_->addProperty(tp);

	//rain	float	
	gp = new GenFloatProperty(
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/RAIN_AMOUNT" ),
		new ClampedDataSectionFloatProxy(
			pSystem_,
			"rain",
			"onWeatherAdjust",
			0.f, 0.f, 1.f, 2));
	gp->setGroup( sectionName );
	gp->UIDesc( Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/RAIN_AMOUNT/TOOLTIP" ) );
	editor_->addProperty(gp);

	//sun	Vector4
	gp = new ColourScalarProperty(
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/SUNLIGHT" ),
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/MULTIPLIER" ),
		new ClampedDataSectionColourScalarProxy(
			pSystem_,
			"sun",
			"onWeatherAdjust",
			Vector4(1.f,1.f,1.f,1.f),
			0.1f, 5.f, 1));
	gp->setGroup( sectionName );
	gp->UIDesc( Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/SUNLIGHT/TOOLTIP" ) );
	editor_->addProperty( gp );

	//ambient	Vector4
	gp = new ColourScalarProperty(
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/AMBIENT" ),
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/MULTIPLIER" ),
		new ClampedDataSectionColourScalarProxy(
			pSystem_,
			"ambient",
			"onWeatherAdjust",
			Vector4(1.f,1.f,1.f,1.f),
			0.1f, 5.f, 1));
	gp->setGroup( sectionName );
	gp->UIDesc( Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/AMBIENT/TOOLTIP" ) );
	editor_->addProperty(gp);

	//fog	Vector4
	gp = new ColourScalarProperty(
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/FOG" ),
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/DENSITY" ),
		new ClampedDataSectionColourScalarProxy(
			pSystem_,
			"fog",
			"onWeatherAdjust",
			Vector4(1.f,1.f,1.f,1.f),
			1.f, 5.f, 1));
	gp->setGroup( sectionName );
	gp->UIDesc( Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/FOG/TOOLTIP" ) );
	editor_->addProperty(gp);

	//fogFactor	float
	gp = new GenFloatProperty(
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/FOG_FACTOR" ),
		new ClampedDataSectionFloatProxy(
			pSystem_,
			"fogFactor",
			"onWeatherAdjust",
			0.15f, 0.f, 2.f, 2) );
	gp->setGroup( sectionName );
	gp->UIDesc( Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/FOG_FACTOR/TOOLTIP" ) );
	editor_->addProperty(gp);

	//windSpeed	Vector2
	gp = new Vector2Property(
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/WIND_SPEED" ),
		new ClampedDataSectionVector2Proxy(
			pSystem_,
			"windSpeed",
			"onWeatherAdjust",
			Vector2(0.f,0.f),
			Vector2(-15.f,-15.f),
			Vector2(15.f,15.f)));
	gp->setGroup( sectionName );
	gp->UIDesc(Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/WIND_SPEED/TOOLTIP" ));
	editor_->addProperty(gp);

	//gustiness	float
	gp = new GenFloatProperty(
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/WIND_GUSTINESS" ),
		new ClampedDataSectionFloatProxy(
			pSystem_,
			"windGustiness",
			"onWeatherAdjust",
			0.f, 0.f, 15.f, 1) );
	gp->setGroup( sectionName );
	gp->UIDesc(Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/WIND_GUSTINESS/TOOLTIP" ));
	editor_->addProperty(gp);


	//bloom
	
	//attenuation Vector4
	gp = new ColourScalarProperty(
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/ATTENUATION" ),
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/MULTIPLIER" ),
		new ClampedDataSectionColourScalarProxy(
			pSystem_,
			"bloom/attenuation",
			"onWeatherAdjust",
			Vector4(1.f,1.f,1.f,1.f),
			1.f, 2.f, 4));
	gp->setGroup( Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/BLOOM_SETTINGS" ) );
	gp->UIDesc(Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/ATTENUATION/TOOLTIP" ));
	editor_->addProperty(gp);

	//numPasses	int
	gp = new GenIntProperty(
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/NUM_PASSES" ),
		new ClampedDataSectionIntProxy(
			pSystem_,
			"bloom/numPasses",
			"onWeatherAdjust",
			2, 0, 15, 1) );
	gp->setGroup( Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/BLOOM_SETTINGS" ) );
	gp->UIDesc(Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/NUM_PASSES/TOOLTIP" ));
	editor_->addProperty(gp);

	//power	float
	gp = new GenFloatProperty(
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/SENSITIVITY" ),
		new ClampedDataSectionFloatProxy(
			pSystem_,
			"bloom/power",
			"onWeatherAdjust",
			8, 0.5f, 64.f, 1) );
	gp->setGroup( Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/BLOOM_SETTINGS" ) );
	gp->UIDesc(Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/SENSITIVITY/TOOLTIP" ));
	editor_->addProperty(gp);

	//width	int
	gp = new GenIntProperty(
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/FILTER_WIDTH" ),
		new ClampedDataSectionIntProxy(
			pSystem_,
			"bloom/width",
			"onWeatherAdjust",
			1, 1, 15, 1) );
	gp->setGroup( Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/BLOOM_SETTINGS" ) );
	gp->UIDesc(Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/WEATHER_EDITOR/FILTER_WIDTH/TOOLTIP" ));
	editor_->addProperty(gp);

	editor_->elect();

	return true;
}


/**
 *	DoDataExchange
 */
void WeatherSettingsTable::DoDataExchange(CDataExchange* pDX)
{
	PropertyTable::DoDataExchange(pDX);
}


/**
 *	MFC Message Map
 */
BEGIN_MESSAGE_MAP(WeatherSettingsTable, CFormView)
	ON_MESSAGE(WM_UPDATE_CONTROLS, OnUpdateControls)
	ON_MESSAGE(WM_SELECT_PROPERTYITEM, OnSelectPropertyItem)
	ON_MESSAGE(WM_CHANGE_PROPERTYITEM, OnChangePropertyItem)
	ON_MESSAGE(WM_DBLCLK_PROPERTYITEM, OnDblClkPropertyItem)
END_MESSAGE_MAP()


/**
 *	Called every frame by the tool. Inits the property list (if not already
 *	inited) and calls update on the base class.
 */
afx_msg LRESULT WeatherSettingsTable::OnUpdateControls(WPARAM wParam, LPARAM lParam)
{
	BW_GUARD;

	init();
	update();
	return 0;
}


/**
 *	A list item has been selected
 */
afx_msg LRESULT WeatherSettingsTable::OnSelectPropertyItem(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
	{
		BW_GUARD;

		BaseView* relevantView = (BaseView*)lParam;
		relevantView->onSelect();
	}

	return 0;
}


/**
 *	A list item's value has been changed
 */
afx_msg LRESULT WeatherSettingsTable::OnChangePropertyItem(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
	{
		BW_GUARD;

		BaseView* relevantView = (BaseView*)lParam;
		relevantView->onChange(wParam != 0);
	}

	return 0;
}


/**
 *	A list item has been right-clicked
 */
afx_msg LRESULT WeatherSettingsTable::OnDblClkPropertyItem(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
	{
		BW_GUARD;

		PropertyItem* relevantView = (PropertyItem*)lParam;
		relevantView->onBrowse();
	}
	
	return 0;
}


CRect WeatherSettingsTable::dropTest( UalItemInfo* ii )
{
	BW_GUARD;

	return propertyList()->dropTest( CPoint( ii->x(), ii->y() ),
		BWResource::dissolveFilenameW( ii->longText() ) );
}


bool WeatherSettingsTable::doDrop( UalItemInfo* ii )
{
	BW_GUARD;

	return propertyList()->doDrop( CPoint( ii->x(), ii->y() ),
		BWResource::dissolveFilenameW( ii->longText() ) );
}
