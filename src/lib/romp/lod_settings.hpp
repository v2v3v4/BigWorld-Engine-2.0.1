/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOD_SETTINGS_REGISTRY_HPP
#define LOD_SETTINGS_REGISTRY_HPP

#include "cstdmf/debug.hpp"
#include "moo/graphics_settings.hpp"
#include <string>
#include <vector>
#include <map>

// Forward declarations
typedef SmartPointer<class DataSection> DataSectionPtr;


class LodSettings
{
public:
	void init(DataSectionPtr resXML);
	
	float applyLodBias(float distance);
	void applyLodBias(float & lodNear, float & lodFar);

	static LodSettings & instance();

private:
	void setLodOption(int optionIdx);

	std::vector<float> lodOptions_;
	Moo::GraphicsSetting::GraphicsSettingPtr lodSettings_;

private:
	LodSettings() :
		lodOptions_(),
		lodSettings_(NULL)
	{}
};

#endif // LOD_SETTINGS_REGISTRY_HPP
