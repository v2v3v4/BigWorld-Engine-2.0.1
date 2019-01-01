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
#include "terrain_graphics_options.hpp"
#include "manager.hpp"
#include "cstdmf/debug.hpp"
#include "terrain_settings.hpp"

using namespace Terrain;


TerrainGraphicsOptions::TerrainGraphicsOptions() : 
	lodModifier_( 1.f )
{
}

/**
 *	This method inits the terrain graphics options
 */
void TerrainGraphicsOptions::init( DataSectionPtr pTerrain2Defaults )
{
	BW_GUARD;
	initOptions( pTerrain2Defaults->openSection( "lodOptions" ) );
}


/**
 *	Instance wrapper method for the TerrainGraphicsOptions
 */
TerrainGraphicsOptions* TerrainGraphicsOptions::instance()
{
	BW_GUARD;
	if (Manager::pInstance() == NULL)
		return NULL;
	
	return &Manager::instance().graphicsOptions();
}


/**
 *	This method initialises the lod options
 *	@param pOptions the options to use
 */
void TerrainGraphicsOptions::initOptions( DataSectionPtr pOptions )
{
	BW_GUARD;
	typedef Moo::GraphicsSetting::GraphicsSettingPtr GraphicsSettingPtr;

	// Create the lod distance setting
	pLodDistanceSettings_ = Moo::makeCallbackGraphicsSetting(
			"TERRAIN_LOD", "Terrain LOD Distance", *this, 
			&TerrainGraphicsOptions::setLodDistanceOption,
			-1, false, false);


	// Initialise the distance options from the data section
	if (pOptions)
	{
		DataSectionIterator sectIt  = pOptions->begin();
		DataSectionIterator sectEnd = pOptions->end();
		while (sectIt != sectEnd)
		{
			float value = (*sectIt)->readFloat("value");
			std::string label = (*sectIt)->readString("label");
			if (!label.empty() && value > 0.f)
			{
				pLodDistanceSettings_->addOption(label, label, true);
				lodDistanceOptions_.push_back(value);
			}
			++sectIt;
		}
	}

	// If no distance settings have been added 
	// initialise the default one
	if (!pLodDistanceSettings_->options().size())
	{
		pLodDistanceSettings_->addOption("FAR",  "Far Plane Distance", true);
		lodDistanceOptions_.push_back(1.0f);
	}

	// Add the distance setting
	Moo::GraphicsSetting::add(this->pLodDistanceSettings_);

	// Create the top lod setting
	pTopLodSettings_ = Moo::makeCallbackGraphicsSetting(
			"TERRAIN_MESH_RESOLUTION", "Terrain Mesh Resolution", *this, 
			&TerrainGraphicsOptions::setTopLodOption,
			-1, false, false);
	
	// Add three options
	pTopLodSettings_->addOption( "HIGH", "High", true );
	pTopLodSettings_->addOption( "MEDIUM", "Medium", true );
	pTopLodSettings_->addOption( "LOW", "Low", true );
	
	// Add the top lod setting
	Moo::GraphicsSetting::add(this->pTopLodSettings_);
}

/**
 *	This method is used by the graphics setting system.
 *	@param optionIdx the selected lod option
 */
void TerrainGraphicsOptions::setLodDistanceOption(int optionIdx)
{
	BW_GUARD;
	optionIdx = std::min( int(lodDistanceOptions_.size() - 1), 
		std::max( 0, optionIdx) );
	lodModifier_ = lodDistanceOptions_[optionIdx];

	SimpleMutexHolder sm(mutex_);
	for (uint32 i = 0; i < settings_.size(); i++)
	{
		settings_[i]->applyLodModifier( lodModifier_ );
	}

}

/**
 *	This method is used by the graphics setting system.
 *	@param optionIdx the selected top lod
 */
void TerrainGraphicsOptions::setTopLodOption(int optionIdx)
{
	BW_GUARD;
	TerrainSettings::topVertexLod( optionIdx );
}

/**
 *	This method adds a terrain settings object so that we can apply
 *	settings to it.
 */
void TerrainGraphicsOptions::addSettings( TerrainSettings* pSettings )
{
	BW_GUARD;
	SimpleMutexHolder sm(mutex_);
	settings_.push_back( pSettings );
}

/**
 *	This method removes a terrain settings object from the internal list.
 */
void TerrainGraphicsOptions::delSettings( TerrainSettings* pSettings )
{
	BW_GUARD;
	SimpleMutexHolder sm(mutex_);

	std::vector< TerrainSettings* >::iterator it = 
		std::find(settings_.begin(), settings_.end(), pSettings );

	if (it != settings_.end())
		settings_.erase( it );
}
