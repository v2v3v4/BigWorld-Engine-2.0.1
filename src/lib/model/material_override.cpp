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

#include "material_override.hpp"


DECLARE_DEBUG_COMPONENT2( "Model", 0 )


void MaterialOverride::update( Moo::EffectMaterialPtr newMat )
{
	BW_GUARD;
	savedMaterials_.resize( effectiveMaterials_.size() );
	std::vector< Moo::Visual::PrimitiveGroup * >::iterator iter = effectiveMaterials_.begin();
	std::vector< Moo::EffectMaterialPtr >::iterator saveIter = savedMaterials_.begin();
	while( iter != effectiveMaterials_.end() )
	{
		*saveIter = (*iter)->material_;
		(*iter)->material_ = newMat;
		++saveIter;
		++iter;
	}
}


void MaterialOverride::reverse()
{
	BW_GUARD;
	if( savedMaterials_.size() )
	{
		std::vector< Moo::Visual::PrimitiveGroup * >::iterator iter = effectiveMaterials_.begin();
		std::vector< Moo::EffectMaterialPtr >::iterator saveIter = savedMaterials_.begin();
		while( iter != effectiveMaterials_.end() )
		{
			(*iter)->material_ = *saveIter;
			++saveIter;
			++iter;
		}
	}
}



// material_override.cpp
