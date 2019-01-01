/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VISUAL_COMMON_HPP
#define VISUAL_COMMON_HPP

#include "physics2/bsp.hpp"
#include "physics2/worldtri.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/xml_section.hpp"
#include "resmgr/primitive_file.hpp"

#include "cstdmf/guard.hpp"

namespace Moo
{

typedef std::vector< std::string > 	BSPMaterialIDs;

/**
 * 	This class' reason for existence is to hold the common code used to
 * 	load a visual file.
 *
 * 	The VISUAL template parameter is supposed to be a class very similar to
 * 	Moo::Visual. See individual functions for requirements for VISUAL class.
 */
template <class VISUAL>
class VisualLoader
{
	std::string		resourceBaseName_;
	std::string		visualResID_;
	std::string		primitivesResID_;
	DataSectionPtr	pVisualSection_;

public:
	VisualLoader( const std::string& resourceBaseName );

	DataSectionPtr& getRootDataSection() { return pVisualSection_; }

	const std::string& getVisualResID() const { return visualResID_; }
	const std::string& getPrimitivesResID() const { return primitivesResID_; }

	bool loadBSPTree( typename VISUAL::BSPTreePtr& pBSPTree,
			BSPMaterialIDs& materialIDs );
	bool loadBSPTreeNoCache( typename VISUAL::BSPTreePtr& pBSPTree,
			BSPMaterialIDs& materialIDs );
	void setBoundingBox( BoundingBox& boundingBox );

	void collectMaterials( typename VISUAL::Materials& materials );
	void remapBSPMaterialFlags( BSPTree& bspTree,
			const typename VISUAL::Materials& materials,
			const BSPMaterialIDs& materialIDs );
};

/**
 *	Constructor.
 */
template <class VISUAL>
VisualLoader< VISUAL >::VisualLoader( const std::string& resourceBaseName ) :
	resourceBaseName_( resourceBaseName ),
	visualResID_(), primitivesResID_(), pVisualSection_()
{
	BW_GUARD;
	std::replace( resourceBaseName_.begin(), resourceBaseName_.end(), '\\', '/' );

	// Try .visual file
	visualResID_ = resourceBaseName_ + ".visual";
	pVisualSection_ = BWResource::openSection( visualResID_ );
	if (!pVisualSection_)
	{
		// Try .static.visual file
		visualResID_ = resourceBaseName_ + ".static.visual";
		pVisualSection_ = BWResource::openSection( visualResID_ );
		if (!pVisualSection_)
		{
			ERROR_MSG( "VisualLoader::VisualLoader: Failed to load visual file "
						"for %s\n", resourceBaseName_.c_str() );
		}
	}

	primitivesResID_ = resourceBaseName_ + ".primitives";
}

/**
 *	This method returns the BSP tree for the visual. It gets the BSP using
 * 	the following methods:
 * 		1. Look in BSP cache.
 * 		2. Load pre-generated BSP from file (see loadBSPTreeNoCache() method)
 *	The returned BSP is inserted into the BSP cache if it wasn't there before.
 *
 * 	The BSPTree is returned via the bspTreeProxy output parameter.
 *
 * 	Ruturns a boolean indicating whether the BSP tree uses primitive groups
 * 	for its collision and material flags (i.e. it's a bsp2 BSPTree).
 * 	A bsp2 BSPTree will need the primitive groups information to complete its
 * 	loading. This is done by calling the remapFlags() method of the BSPTree.
 * 	Non-bsp2 BSPTrees are fully loaded by this function. A NULL BSPTree is
 * 	returned if it is not found in the cache or on disk i.e. needs to be
 * 	generated.
 *
 * 	Requires:
 * 		- VISUAL::BSPCache& VISUAL::BSPCache::instance()
 * 		- VISUAL::BSPCache::find( std::string ) returns VISUAL::BSPTreePtr
 * 		- VISUAL::BSPCache::add( std::string, VISUAL::BSPTreePtr )
 * 		- VISUAL::loadBSPVisual( std::string ) returns VISUAL::BSPTreePtr
 */
template <class VISUAL>
bool VisualLoader< VISUAL >::loadBSPTree( typename VISUAL::BSPTreePtr& pBSPTree,
		BSPMaterialIDs& materialIDs )
{
	BW_GUARD;
	bool shouldRemapFlags = false;
	pBSPTree = VISUAL::BSPCache::instance().find( visualResID_ );
	if (!pBSPTree)
	{
		shouldRemapFlags = this->loadBSPTreeNoCache( pBSPTree, materialIDs );
		if (pBSPTree)
		{
			VISUAL::BSPCache::instance().add( visualResID_, pBSPTree );
		}
	}

	return shouldRemapFlags;
}

/**
 *	This function finds and loads a bsp for a visual.
 *	At the moment (1.8), it goes like this :
 *	- <basename>_bsp.visual file
 *	- "bsp2" subfile in <basename>.primitives
 *	- "bsp" subfile in <basename>.primitives
 *	- "<basename>.bsp" file
 * 	Returns the BSPTree if found and whether it was loaded from the "bsp2"
 * 	sub-file.
 */
template <class VISUAL>
bool VisualLoader< VISUAL >::loadBSPTreeNoCache(
		typename VISUAL::BSPTreePtr& pBSPTree, BSPMaterialIDs& materialIDs )
{
	BW_GUARD;
	bool 		shouldRemapFlags = false;

	// Try loading _bsp.visual
	std::string bspVisualResID = resourceBaseName_ + "_bsp.visual";
	// Potential infinite recursion. VISUAL::loadBSPVisual() should test
	// whether file exists and exit early if it doesn't.
	pBSPTree = VISUAL::loadBSPVisual( bspVisualResID );
	if (pBSPTree)
	{
		WARNING_MSG( "Separate BSP files (_bsp.visual) are deprecated "
				"in BigWorld 1.8. Please re-export %s\n",
				resourceBaseName_.c_str() );
	}
	else
	{
		DataSectionPtr pBSPSection;

		// Try loading from .primitives file
		DataSectionPtr pPrimitivesSection =
				PrimitiveFile::get( primitivesResID_ );
		if (pPrimitivesSection)
		{
			pBSPSection = pPrimitivesSection->findChild( "bsp2" );
			if (pBSPSection)
			{
				//TODO : this is the only codepath that will be valid for
				//BigWorld 1.9 (we are deprecating everything else.)
				shouldRemapFlags = true;

				BinaryPtr pBin =
						pPrimitivesSection->readBinary( "bsp2_materials" );
				if (pBin)
				{
					//Make a copy of pBin, as createFromBinary modifies the
					//data (see fn comment in XMLSection::createFromBinary)
					BinaryPtr pCopy = new BinaryBlock(pBin->data(), pBin->len(),
						"VisualLoader::loadBSPTreeNoCache");
					XMLSectionPtr pMaterialIDs = 
						XMLSection::createFromBinary( "root", pCopy );

					if (pMaterialIDs)
					{
						pMaterialIDs->readStrings( "id", materialIDs );
#ifdef EDITOR_ENABLED
						if (materialIDs.empty())
						{
							WARNING_MSG( "%s was exported using the old bsp2 "
									"exporter, and must be re-exported manually\n",
									resourceBaseName_.c_str() );
						}
#endif
					}
				}
			}
			else
			{
				pBSPSection = pPrimitivesSection->findChild( "bsp" );
				if (pBSPSection)
				{
					WARNING_MSG( "Primitives file contains \"bsp\" section, "
						"which is deprecated in BigWorld 1.8 and replaced"
						" with \"bsp2\" section.  Please re-export %s\n",
						resourceBaseName_.c_str() );
				}
			}
		}

		if (!pBSPSection)
		{
			// Try loading from .bsp file
			std::string bspResID = resourceBaseName_ + ".bsp";
			pBSPSection = BWResource::openSection( bspResID );
			if (pBSPSection)
			{
				if (BWResource::isFileOlder( bspResID, visualResID_ ))
				{
					WARNING_MSG( "%s is older than %s\n", bspResID.c_str(),
							visualResID_.c_str() );
				}

				WARNING_MSG( "Separate BSP files (.bsp) are deprecated in "
					"BigWorld 1.8\nPlease re-export %s\n",
					resourceBaseName_.c_str() );
			}
		}

		BinaryPtr pBSPData;
		if (pBSPSection)
			pBSPData = pBSPSection->asBinary();

		if (pBSPData)
		{
			BSPTree* pNewBSPTree = new BSPTree();

			if (pNewBSPTree->load( pBSPData ))
			{
				pBSPTree = pNewBSPTree;
			}
			else
			{
				// This will always fail for visuals that do not have a BSP.
				delete pNewBSPTree;
			}
		}
		else
		{
			//Hopefully this is a skinned object and doesn't require a BSP.
//			WARNING_MSG( "No BSP data found. Please re-export %s\n",
//				resourceBaseName_.c_str() );
		}
	}

	return shouldRemapFlags;
}

/**
 *	This method remaps the flags in the visual's BSP, from bsp material
 *	index to use the visual's material's collision flags / material kinds.
 *
 *	@param	bspMaterialIDs	ordered list of material names, one for each
 *							index currently in the world triangles of the bsp.
 */
template <class VISUAL>
void VisualLoader< VISUAL >::remapBSPMaterialFlags( BSPTree& bspTree,
		const typename VISUAL::Materials& materials,
		const BSPMaterialIDs& materialIDs )
{
	BW_GUARD;
	int objectMaterialKind = pVisualSection_->readInt( "materialKind" );

	//construct bspMap given the bsp's bspMaterialIDs and
	//the visual's materials.
	//
	//go through the BSP's materials list, and for each one
	//add an entry to the bspMap that says what materialKind
	//to give triangles that have the material.
	BSPFlagsMap bspMap;
	for ( BSPMaterialIDs::const_iterator iterMatID = materialIDs.begin();
			iterMatID != materialIDs.end(); ++iterMatID )
	{
		const typename VISUAL::Material * pMaterial =
			&*materials.find( *iterMatID );

		// If material is not found this means the custom BSP was not
		// assigned materials from the object, so we didn't find the material
		// identifier in the object.  So that means we use the object's material
		// kind.
		WorldTriangle::Flags flags = (pMaterial) ? pMaterial->getFlags( objectMaterialKind ) :
									WorldTriangle::packFlags( 0, objectMaterialKind );
		bspMap.push_back( flags );
	}

	if (bspMap.empty())
	{
		bspMap.push_back( WorldTriangle::packFlags( 0, objectMaterialKind ) );
	}

	bspTree.remapFlags( bspMap );
}

/**
 *	This function loops visits all the materials in the primitive groups
 * 	in the visual and adds them to materials.
 *
 * 	NOTE: This method is only used by the server because it usually does not
 * 	need to process all the information in the visual file.
 *
 * 	Requires:
 * 		- VISUAL::Materials::add( VISUAL::Material* )
 * 		- VISUAL::Material::Material( DataSectionPtr )
 */
template <class VISUAL>
void VisualLoader< VISUAL >::collectMaterials(
		typename VISUAL::Materials& materials )
{
	BW_GUARD;
	// Loop through all renderSet/geometry/primitiveGroup sections
	for ( DataSection::SearchIterator iterRenderSet =
			pVisualSection_->beginSearch( "renderSet" );
			iterRenderSet != pVisualSection_->endOfSearch(); ++iterRenderSet )
	{
		DataSectionPtr pRenderSetSection = *iterRenderSet;
		for ( DataSection::SearchIterator iterGeometry =
				pRenderSetSection->beginSearch( "geometry" );
				iterGeometry !=  pRenderSetSection->endOfSearch();
				++iterGeometry )
		{
			DataSectionPtr pGeometrySection = *iterGeometry;
			for ( DataSection::SearchIterator iterPrimGrp =
					pGeometrySection->beginSearch( "primitiveGroup" );
					iterPrimGrp !=  pGeometrySection->endOfSearch();
					++iterPrimGrp )
			{
				DataSectionPtr pPrimitiveGrpSection = *iterPrimGrp;
				DataSectionPtr pMaterialSection =
						pPrimitiveGrpSection->openSection( "material" );

				typename VISUAL::Material* pMaterial =
						new typename VISUAL::Material( pMaterialSection );
				materials.add( pMaterialSection->readString( "identifier" ),
						pMaterial );
			}
		}
	}
}

/**
 *	Sets the bounding box from the visual file.
 */
template <class VISUAL>
void VisualLoader< VISUAL >::setBoundingBox( BoundingBox& boundingBox )
{
	BW_GUARD;
	boundingBox.setBounds( pVisualSection_->readVector3( "boundingBox/min" ),
			pVisualSection_->readVector3( "boundingBox/max" ) );
}

}	// namespace Moo

#endif // VISUAL_COMMON_HPP
