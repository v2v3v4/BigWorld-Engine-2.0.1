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

#ifndef MATERIAL_OVERRIDE_HPP
#define MATERIAL_OVERRIDE_HPP


#include "moo/forward_declarations.hpp"
#include "moo/visual.hpp"


/**
 *	Inner class to represent material override
 */
class MaterialOverride
{
public:
	std::string identifier_;
	std::vector< Moo::Visual::PrimitiveGroup * >		effectiveMaterials_;
	std::vector< Moo::EffectMaterialPtr >				savedMaterials_;
	void update( Moo::EffectMaterialPtr newMat );
	void reverse();
};



#endif // MATERIAL_OVERRIDE_HPP
