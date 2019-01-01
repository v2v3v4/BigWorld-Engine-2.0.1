/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STATIC_LIGHT_FASHION_HPP
#define STATIC_LIGHT_FASHION_HPP

#include "model/super_model.hpp"
#include "romp/static_light_values.hpp"


/**
 *	This class is a SuperModel fashion that sets static lighting
 */
class StaticLightFashion : public Fashion
{
public:
	static SmartPointer<StaticLightFashion> get( StaticLightValueCachePtr cache,
		SuperModel & sm, const DataSectionPtr modelLightingSection );

	std::vector<StaticLightValuesPtr> staticLightValues();

#ifdef EDITOR_ENABLED
	bool isDataValid() const;
#endif// EDITOR_ENABLED

	/** Get the resource section name for the given models static lighting info */
	static std::string StaticLightFashion::lightingTag( int index, int count );

private:
	StaticLightFashion( StaticLightValueCachePtr cache, SuperModel & sm,
		const DataSectionPtr modelLightingSection );
	virtual ~StaticLightFashion();

	StaticLightFashion( const StaticLightFashion& );
	StaticLightFashion& operator=( const StaticLightFashion& );

	int						nModels_;
	ModelStaticLighting *	lighting_[1];

	virtual void dress( SuperModel & superModel );
	virtual void undress( SuperModel & superModel );
};

typedef SmartPointer<StaticLightFashion> StaticLightFashionPtr;



#endif // STATIC_LIGHT_FASHION_HPP
