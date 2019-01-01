/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _INCLUDE_VISUALFILETRANSLATOR_HPP
#define _INCLUDE_VISUALFILETRANSLATOR_HPP

#include <maya/MPxFileTranslator.h>

#include "visual_mesh.hpp"
#include "visual_portal.hpp"

#include "cstdmf/stringmap.hpp"

// our plugin file translator class. This must inherit
// from the base class MPxFileTranslator
class VisualFileTranslator : public MPxFileTranslator
{
public:
	typedef std::map<std::string, int>	BoneCountMap;
	typedef BoneCountMap::iterator		BoneCountMapIt;

	VisualFileTranslator() : MPxFileTranslator() {};
	~VisualFileTranslator() {};

	// we overload this method to perform any 
	// file export operations needed
	MStatus writer( const MFileObject& file, const MString& optionsString, FileAccessMode mode); 

	// returns true if this class can export files
	bool haveWriteMethod() const;

	// returns the default extension of the file supported
	// by this FileTranslator.
	MString defaultExtension() const;

	// add a filter for open file dialog
	MString filter() const;

	// Used by Maya to create a new instance of our
	// custom file translator
	static void* creator();

	// some option flags set by the mel script
	bool	exportMesh( std::string fileName );

	// Returns the number of unique bones
	void	getBoneCounts( BoneCountMap& boneCountMap );

private:
	void loadReferenceNodes( const std::string & mfxFile );
	void readReferenceHierarchy( DataSectionPtr hierarchy );
	StringHashMap<std::string> nodeParents_;

	std::vector<VisualMeshPtr>		visualMeshes;
	std::vector<VisualMeshPtr>		hullMeshes;
	std::vector<VisualMeshPtr>		bspMeshes;
	std::vector<VisualPortalPtr>	visualPortals;
};


#endif
