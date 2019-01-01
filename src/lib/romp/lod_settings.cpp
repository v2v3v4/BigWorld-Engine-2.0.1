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
#include "lod_settings.hpp"

// BW Tech Hearders
#include "moo/graphics_settings.hpp"

DECLARE_DEBUG_COMPONENT2( "Moo", 0 );

// -----------------------------------------------------------------------------
// Section: LodSettings
// -----------------------------------------------------------------------------

/**
 *	Initialises the graphics settings registry.
 *
 */
void LodSettings::init(DataSectionPtr resXML)
{
	// register object lod settigns
	typedef Moo::GraphicsSetting::GraphicsSettingPtr GraphicsSettingPtr;
	this->lodSettings_ = Moo::makeCallbackGraphicsSetting(
			"OBJECT_LOD", "Object Level of Detail", *this, 
			&LodSettings::setLodOption,
			-1, false, false);
			
	if (resXML.exists())
	{
		DataSectionIterator sectIt  = resXML->begin();
		DataSectionIterator sectEnd = resXML->end();
		while (sectIt != sectEnd)
		{
			static const float undefined = -1.0f;
			float value = (*sectIt)->readFloat("value", undefined);
			std::string label = (*sectIt)->readString("label");
			if (!label.empty() && value != undefined)
			{
				this->lodSettings_->addOption(label, label, true);
				this->lodOptions_.push_back(value);
			}
			++sectIt;
		}
	}
	else
	{
		this->lodSettings_->addOption("HIGH", "High", true);
		this->lodOptions_.push_back(1.0f);
	}

	Moo::GraphicsSetting::add(this->lodSettings_);
}


float LodSettings::applyLodBias(float distance)
{
	if (this->lodSettings_.exists())
	{
		MF_ASSERT(this->lodSettings_->activeOption() < int(this->lodOptions_.size()));
		return distance / this->lodOptions_[this->lodSettings_->activeOption()];
	}
	else
	{
		return distance;
	}
}


void LodSettings::applyLodBias(float & lodNear, float & lodFar)
{
	if (this->lodSettings_.exists()) 
	{
		MF_ASSERT(this->lodSettings_->activeOption() < int(this->lodOptions_.size()));
		const float & lodFactor = this->lodOptions_[this->lodSettings_->activeOption()];
		lodNear = lodNear * lodFactor;
		lodFar  = lodFar  * lodFactor;
	}
}


/**
 *	Returns singleton FloraSettings instance.
 */
LodSettings & LodSettings::instance()
{
	static LodSettings instance;
	return instance;
}


void LodSettings::setLodOption(int optionIdx)
{}

// settings_registry.cpp
