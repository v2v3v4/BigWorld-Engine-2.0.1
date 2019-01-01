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
#include "material_draw_override.hpp"
#include "moo/effect_visual_context.hpp"
#include "moo/visual_channels.hpp"

MaterialDrawOverride::MaterialDrawOverride( const std::string& shaderPrefix, bool allowSorted, int doubleSided /* = -1 */ ):
allowSorted_( allowSorted )
{
	BW_GUARD;
	pSkinnedEffect_ = new Moo::EffectMaterial();
	pSkinnedEffect_->initFromEffect( shaderPrefix + "_skinned.fx", "", doubleSided );
	pRigidEffect_ = new Moo::EffectMaterial();
	pRigidEffect_->initFromEffect( shaderPrefix + ".fx", "", doubleSided );
}

/**
 *	This method implements the draw method for the visual override.
 */
HRESULT MaterialDrawOverride::draw( Moo::Visual* pVisual, bool ignoreBoundingBox )
{
	BW_GUARD;
	typedef Moo::Visual::RenderSetVector RSV;
	RSV& renderSets = pVisual->renderSets();
	RSV::iterator rsit = renderSets.begin();
	RSV::iterator rsend = renderSets.end();

	Moo::EffectVisualContext::instance().initConstants();
	Moo::EffectVisualContext::instance().staticLighting( false );

	// Do bounding box check
	Matrix viewMatrix;
	BoundingBox bb( pVisual->boundingBox() );	
	viewMatrix.multiply( Moo::rc().world(), Moo::rc().viewProjection() );	

	bool shouldDraw = ignoreBoundingBox;
	if (!shouldDraw)
	{
		bb.calculateOutcode( viewMatrix );
		shouldDraw = !bb.combinedOutcode();
	}
	if (!shouldDraw)
		return S_OK;

	// iterate over rendersets
	while (rsit != rsend)
	{
		Moo::Visual::RenderSet& rs = *(rsit++);
		Moo::EffectVisualContextSetter setter( &rs );
		Moo::Visual::GeometryVector::iterator git = rs.geometry_.begin();
		Moo::Visual::GeometryVector::iterator gend = rs.geometry_.end();
		// iterate over geometry
		while (git != gend)
		{
			Moo::Visual::Geometry& geom = *(git++);
		
			if (FAILED(geom.vertices_->setVertices(Moo::rc().mixedVertexProcessing())))
			{
				ERROR_MSG( "MaterialDrawOverride::draw -"
					" Unable to set vertices for %s\n",
					pVisual->resourceID().c_str() );
				continue;
			}
			
			if (FAILED(geom.primitives_->setPrimitives()))
			{
				ERROR_MSG( "MaterialDrawOverride::draw -"
							" Unable to set primitives for %s\n",
								pVisual->resourceID().c_str() );
				continue;
			}

			const std::string& vFormat	= geom.vertices_->format();
			Moo::EffectMaterialPtr pMat	= NULL;

			// Select material depending on the vertex format
			if (vFormat.length() >= 11 &&
				vFormat.substr(0, 11) == "xyznuviiiww" )
			{
				pMat = pSkinnedEffect_;
			}			
			else
			{
				pMat = pRigidEffect_;
			}

			// If the material has no channel render straight away
			if (!pMat->channel())
			{
				if (pMat->begin())
				{
					for (uint32 i = 0; i < pMat->nPasses(); i++)
					{
						if ( pMat->beginPass( i ) )
						{
							Moo::Visual::PrimitiveGroupVector::iterator pgit 
								= geom.primitiveGroups_.begin();
							Moo::Visual::PrimitiveGroupVector::iterator pgend 
								= geom.primitiveGroups_.end();					
							
							while (pgit != pgend)
							{						
								Moo::Visual::PrimitiveGroup& primGroup = *pgit;
								
								// If the original material is sorted, only 
								// render if the allowSorted_ flag is set
								if ( primGroup.material_->readyToUse() &&
									(allowSorted_ || 
										!primGroup.material_->channel()) 
										&& primGroup.material_->pEffect() )
								{
									geom.primitives_->drawPrimitiveGroup( 
										primGroup.groupIndex_ );
								}

								pgit++;						
							}
							pMat->endPass();
						}
					}
					pMat->end();
				}
			}
			else
			{
				// Render using the materials channel
				Moo::Visual::PrimitiveGroupVector::iterator pgit = 
												geom.primitiveGroups_.begin();
				Moo::Visual::PrimitiveGroupVector::iterator pgend = 
													geom.primitiveGroups_.end();					
				
				while (pgit != pgend)
				{						
					Moo::Visual::PrimitiveGroup& primGroup = *pgit;
					if ((allowSorted_ || !primGroup.material_->channel()) 
							&& primGroup.material_->pEffect() )
					{
						Moo::VertexSnapshotPtr pVSS = 
							geom.vertices_->getSnapshot( rs.transformNodes_, 
									pMat->skinned(), pMat->bumpMapped());
						
						float distance = 
							Moo::rc().view().applyPoint( bb.centre() ).z;
						
						pMat->channel()->addItem( pVSS, geom.primitives_, pMat,
							primGroup.groupIndex_, distance, NULL );
					}
					pgit++;						
				}				
			}
		}
	}

	return S_OK;
}
