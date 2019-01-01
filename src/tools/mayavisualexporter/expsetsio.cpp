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
#include "expsets.hpp"
#include "resmgr/dataresource.hpp"

bool ExportSettings::readSettings()
{
	DataResource file( settingFilename_, RESOURCE_TYPE_XML );

	DataSectionPtr pRoot = file.getRootSection( );

	if( pRoot )
	{
		exportAnim_ = pRoot->readBool( "exportAnimation", exportAnim_ );
		exportMode_ = (ExportMode)pRoot->readInt( "exportMode", exportMode_ );
		maxBones_ = pRoot->readInt( "boneCount", maxBones_ );
		allowScale_ = pRoot->readBool( "allowScale", allowScale_ );
		bumpMapped_ = pRoot->readBool( "bumpMapped", bumpMapped_ );
		keepExistingMaterials_ = pRoot->readBool( "keepExistingMaterials", keepExistingMaterials_ );
		snapVertices_ = pRoot->readBool( "snapVertices", snapVertices_ );
		stripRefPrefix_ = pRoot->readBool( "stripRefPrefix", stripRefPrefix_ );
		referenceNode_ = pRoot->readBool( "useReferenceNode", referenceNode_ );
		disableVisualChecker_ = pRoot->readBool( "disableVisualChecker", disableVisualChecker_ );
		useLegacyScaling_ = pRoot->readBool( "useLegacyScaling", useLegacyScaling_ );
		fixCylindrical_ = pRoot->readBool( "fixCylindrical", fixCylindrical_ );
		useLegacyOrientation_ = pRoot->readBool( "useLegacyOrientation", useLegacyOrientation_ );
		sceneRootAdded_ = pRoot->readBool( "sceneRootAdded", sceneRootAdded_ );

		includeMeshes_ = pRoot->readBool( "includeMeshes", includeMeshes_ );
		includeEnvelopesAndBones_ = pRoot->readBool( "includeEnvelopesAndBones", includeEnvelopesAndBones_ );
		includeNodes_ = pRoot->readBool( "includeNodes", includeNodes_ );
		includeMaterials_ = pRoot->readBool( "includeMaterials", includeMaterials_ );
		includeAnimations_ = pRoot->readBool( "includeAnimations", includeAnimations_ );
		useCharacterMode_ = pRoot->readBool( "useCharacterMode", useCharacterMode_ );
		animationName_ = pRoot->readString( "animationName", animationName_ );
		includePortals_ = pRoot->readBool( "includePortals", includePortals_ );
		worldSpaceOrigin_ = pRoot->readBool( "worldSpaceOrigin", worldSpaceOrigin_ );
		unitScale_ = pRoot->readFloat( "unitScale", unitScale_ );
		localHierarchy_ = pRoot->readBool( "localHierarchy", localHierarchy_ );
		nodeFilter_ = (NodeFilter)pRoot->readInt( "nodeFilter", nodeFilter_ );
		return true;
	}
	return false;
}

bool ExportSettings::writeSettings()
{
	DataResource file( settingFilename_, RESOURCE_TYPE_XML );

	DataSectionPtr pRoot = file.getRootSection( );


	if( pRoot )
	{
		pRoot->writeBool( "exportAnimation", exportAnim_ );
		pRoot->writeInt( "exportMode", exportMode_ );
		pRoot->writeInt( "boneCount", maxBones_ );
		pRoot->writeBool( "allowScale", allowScale_ );
		pRoot->writeBool( "bumpMapped", bumpMapped_ );
		pRoot->writeBool( "keepExistingMaterials", keepExistingMaterials_ );
		pRoot->writeBool( "snapVertices", snapVertices_ );
		pRoot->writeBool( "stripRefPrefix", stripRefPrefix_ );
		pRoot->writeBool( "useReferenceNode", referenceNode_ );
		pRoot->writeBool( "disableVisualChecker", disableVisualChecker_ );
		pRoot->writeBool( "useLegacyScaling", useLegacyScaling_ );
		pRoot->writeBool( "fixCylindrical", fixCylindrical_ );
		pRoot->writeBool( "useLegacyOrientation", useLegacyOrientation_ );
		pRoot->writeBool( "sceneRootAdded", sceneRootAdded_ );

		pRoot->writeBool( "includeMeshes", includeMeshes_ );
		pRoot->writeBool( "includeEnvelopesAndBones", includeEnvelopesAndBones_ );
		pRoot->writeBool( "includeNodes", includeNodes_ );
		pRoot->writeBool( "includeMaterials", includeMaterials_ );
		pRoot->writeBool( "includeAnimations", includeAnimations_ );
		pRoot->writeBool( "useCharacterMode", useCharacterMode_ );
		pRoot->writeString( "animationName", animationName_ );
		pRoot->writeBool( "includePortals", includePortals_ );
		pRoot->writeBool( "worldSpaceOrigin", worldSpaceOrigin_ );
		pRoot->writeFloat( "unitScale", unitScale_ );
		pRoot->writeBool( "localHierarchy", localHierarchy_ );
		pRoot->writeInt( "nodeFilter", nodeFilter_ );

		file.save();
		return true;
	}
	return false;
}