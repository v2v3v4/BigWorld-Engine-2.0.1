/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifdef _MSC_VER 
#pragma once
#endif

#ifndef MODEL_STATIC_LIGHTING_HPP
#define MODEL_STATIC_LIGHTING_HPP

#include "forward_declarations.hpp"
#include "romp/static_light_values.hpp"


/**
 *	Inner class to represent static lighting
 */
class ModelStaticLighting : public ReferenceCount
{
public:
	virtual void set() = 0;
	virtual void unset() = 0;

	/** Get the values used for the static lighting */
	virtual StaticLightValuesPtr staticLightValues() = 0;
};




#endif // MODEL_STATIC_LIGHTING_HPP
