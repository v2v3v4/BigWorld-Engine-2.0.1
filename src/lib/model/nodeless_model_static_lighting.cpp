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

#include "nodeless_model_static_lighting.hpp"

#include "moo/visual.hpp"
#include "romp/static_light_values.hpp"


DECLARE_DEBUG_COMPONENT2( "Model", 0 )


NodelessModelStaticLighting::NodelessModelStaticLighting(
										Moo::VisualPtr bulk,
										StaticLightValuesPtr pSLV ) :
	bulk_( bulk ),
	pSLV_( pSLV )
{
	BW_GUARD;
}


/**
 *	This method sets the static lighting for the main bulk of the nodeless
 *	model from which it was loaded.
 */
void NodelessModelStaticLighting::set()
{
	BW_GUARD;
	bulk_->staticVertexColours( pSLV_ );
}


void NodelessModelStaticLighting::unset()
{
	BW_GUARD;
	bulk_->staticVertexColours( NULL );
}


StaticLightValuesPtr NodelessModelStaticLighting::staticLightValues()
{
	BW_GUARD;
	return pSLV_;
}


// nodeless_model_static_lighting.cpp
