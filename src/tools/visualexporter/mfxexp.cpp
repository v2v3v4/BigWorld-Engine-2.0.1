/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#pragma warning ( disable : 4530 )
#pragma warning ( disable : 4786 )

#include <cstdmf/stdmf.hpp>
#include <cstdmf/dprintf.hpp>

#include "resmgr/primitive_file.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/xml_section.hpp"
#include "gizmo/meta_data_creation_utility.hpp"
#include "mfxexp.hpp"
#include "aboutbox.hpp"
#include "vertcont.hpp"
#include "utility.hpp"
#include "../resourcechecker/visual_checker.hpp"
#include "wm3.h"
#include "hull_mesh.hpp"

#include "bsp_generator.hpp"

#include "ieditnormals.h"
#include "iparamm2.h"
#include "iskin.h"

#include "data_section_cache_purger.hpp"
#include "node_catalogue_holder.hpp"

#include "moo/node.hpp"
#include "moo/visual_splitter.hpp"

#ifndef CODE_INLINE
#include "mfxexp.ipp"
#endif

#include "resmgr/file_system.hpp"
#include "resmgr/multi_file_system.hpp"

DECLARE_DEBUG_COMPONENT2( "Exporter", 0 )


// Defines
#define DEBUG_PRIMITIVE_FILE 0


/**
 *	Constructor
 */
MFXExport::MFXExport()
: nodeCount_ ( 0 ),
  animNodeCount_( 0 ),
  settings_( ExportSettings::instance() ),
  mfxRoot_(NULL)
{
}

/**
 *	Destructor
 */
MFXExport::~MFXExport()
{
}


/**
 * Number of extensions supported, one (mfx)
 */
int MFXExport::ExtCount()
{
	return 1;
}

/**
 * Extension #n (only supports one, visual )
 */
const TCHAR* MFXExport::Ext(int n)
{
	switch( n )
	{
	case 0:
		return _T( "visual" );
	}
	return _T("");
}

/**
 * Long ASCII description
 */
const TCHAR* MFXExport::LongDesc()
{
	return GetString(IDS_LONGDESC);
}

/**
 * Short ASCII description (i.e. "3D Studio")
 */
const TCHAR* MFXExport::ShortDesc()
{
#if defined BW_EXPORTER_DEBUG
	static char shortDesc[] = "Visual Exporter Debug";
	return shortDesc;
#else
	return GetString(IDS_SHORTDESC);
#endif
}

/**
 * ASCII Author name
 */
const TCHAR* MFXExport::AuthorName()
{
	return GetString( IDS_AUTHORNAME );
}

/**
 * ASCII Copyright message
 */
const TCHAR* MFXExport::CopyrightMessage()
{
	return GetString(IDS_COPYRIGHT);
}

/**
 * Other message #1
 */
const TCHAR* MFXExport::OtherMessage1()
{
	return _T("");
}

/**
 * Other message #2
 */
const TCHAR* MFXExport::OtherMessage2()
{
	return _T("");
}

/**
 * Version number * 100 (i.e. v3.01 = 301)
 */
unsigned int MFXExport::Version()
{
	return MFX_EXPORT_VERSION;
}

/**
 * Show DLL's "About..." box
 */
void MFXExport::ShowAbout(HWND hWnd)
{
	AboutBox ab;
	ab.display( hWnd );
}


/**
 *	This method converts the filename into a resource name.  If the
 *	resource is not in the BW_RESOURCE_TREE returns false.
 *	Otherwise the method returns true and the proper resource name is
 *	returned in the resName.
 */
bool validResource( const std::string& fileName, std::string& retResourceName )
{
	std::string resName = BWResolver::dissolveFilename( fileName );

	if (resName == "") return false;
	bool hasDriveName = (resName.c_str()[1] == ':');
	bool hasNetworkName = (resName.c_str()[0] == '/' && resName.c_str()[1] == '/');
	hasNetworkName |= (resName.c_str()[0] == '\\' && resName.c_str()[1] == '\\');
	if (hasDriveName || hasNetworkName)
		return false;

	retResourceName = resName;
	return true;
}


/**
 *	Display a dialog explaining the resource is invalid.
 */
void invalidExportDlg( const std::string& fileName, Interface *i )
{
	std::string errorString;
	errorString = std::string( "The exported file \"" ) + fileName + std::string( "\" is not in a valid game path.\n" );
	errorString += "Need to export to a *subfolder* of:";
    uint32 pathIndex = 0;
	while (BWResource::getPath(pathIndex) != std::string(""))
	{
		errorString += std::string("\n") + BWResource::getPath(pathIndex++);
	}
	MessageBox( i->GetMAXHWnd(), errorString.c_str(), "Visual Exporter Error", MB_OK | MB_ICONWARNING );
}


/**
 *	Display a dialog explaining the resources is invalid.
 */
void invalidResourceDlg( const std::vector<std::string>& fileNames, Interface *i )
{
	std::string errorString( "The following resources are not in a valid game path:   \n\n" );
	
	for (uint32 i=0; i<fileNames.size(); i++)
	{
		errorString += fileNames[i];
		errorString += "\n";
	}		

	errorString += "\nAll resources must be from a *subfolder* of:    \n";
    uint32 pathIndex = 0;
	while (BWResource::getPath(pathIndex) != std::string(""))
	{
		errorString += std::string("\n") + BWResource::getPath(pathIndex++);
	}
	MessageBox( i->GetMAXHWnd(), errorString.c_str(), "Visual Exporter Error", MB_OK | MB_ICONWARNING );
}





/**
 * Main entrypoint to exporter.
 * Do export.
 */
int MFXExport::DoExport(const TCHAR *nameFromMax,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options)
{
	// Purges the datasection cache after finishing each export
	DataSectionCachePurger dscp;
	
	// Initialise NodeCatalogue.
	NodeCatalogueHolder nch;

	ip_ = i;

	BWResource::instance().fileSystem()->addBaseFileSystem( NativeFileSystem::create("") );
	if( !AutoConfig::configureAllFrom("resources.xml") )
	{
		MessageBox( ip_->GetMAXHWnd(),
					"Cannot find resources.xml!\n"
					"Is the bigworld/tools/exporter/3dsmaxX/paths.xml set correctly?!!",
					"Visual Exporter Error",
			MB_OK | MB_ICONERROR );
		return 1;
	}
	if (mfxRoot_)
		delete mfxRoot_;
	portalNodes_.clear();
	envelopeNodes_.clear();
	meshNodes_.clear();
	visualMeshes_.clear();
	visualPortals_.clear();
	bspMeshes_.clear();
	hullMeshes_.clear();


	// This has to be done for edit normals modifier to work properly, don't know quite why ;)
	i->SetCommandPanelTaskMode( TASK_MODE_MODIFY );

	// Used for generating unique temp export names
	static int exportCount = 0;

	// Force the extension to lowercase
	std::string fileName = bw_acptoutf8( nameFromMax );
	fileName = fileName.substr(0, fileName.size() - 7) + ".visual";

	errors_ = false;

	Interval inter = ip_->GetAnimRange();

	settings_.readSettings( getCfgFilename() );
	settings_.setStaticFrame( ip_->GetTime() / GetTicksPerFrame() );


	// Check if the filename is in the right res path.
	std::string resName;
	if (!validResource( fileName, resName ))
	{
		invalidExportDlg( fileName, ip_ );
		errors_ = true;
		return 1;
	}
	std::string resNameCamelCase;
	if (!validResource( fileName, resNameCamelCase ))
	{
		invalidExportDlg( fileName, ip_ );
		errors_ = true;
		return 1;
	}

	std::string filename = BWResource::getFilename( resName );
	if (filename.length() >= 12)
	{
		if (filename.substr( 0, 9 ) == "exporter_"
			&& filename.substr( filename.size() - 12, 12 ) == "_temp.visual")
		{
			errors_ = true;
			std::string msg = filename + " is a reserved name, not exporting.";
			MessageBox( ip_->GetMAXHWnd(), msg.c_str(), "Visual Exporter Error",
				MB_OK | MB_ICONERROR );
			return 1;
		}
	}

	VisualChecker vc = VisualChecker( resNameCamelCase, false, settings_.snapVertices() );

	settings_.visualTypeIdentifier( vc.typeName() );

	// Choose a default export type based on the visual type
	if (vc.exportAs() == "normal")
		settings_.exportMode( ExportSettings::NORMAL );
	else if (vc.exportAs() == "static")
		settings_.exportMode( ExportSettings::STATIC );
	else if (vc.exportAs() == "static with nodes")
		settings_.exportMode( ExportSettings::STATIC_WITH_NODES );

	if( !settings_.displayDialog( ip_->GetMAXHWnd() ) )	
	{
		OutputDebugString( "Canceled\n" );
		return 1;
	}

	// Update the state of the snapping flag inside visual checker
	// since it may have changed
	vc.snapVertices( settings_.snapVertices() );

	if (options == SCENE_EXPORT_SELECTED)
	{
		settings_.nodeFilter( ExportSettings::SELECTED );
	}
	else
	{
		settings_.nodeFilter( ExportSettings::VISIBLE );
	}

	settings_.writeSettings( getCfgFilename() );

	preProcess( ip_->GetRootNode() );

	// Snapping is now a flagable option for static objects
	this->exportMeshes(	settings_.exportMode() != ExportSettings::NORMAL &&
					settings_.snapVertices() );
	this->exportEnvelopes();
	this->exportPortals();

	// There is a 256 bone limit for each skinned object.  For each object that
	// has more than 256 bones, display a warning stating the objects name and
	// the number of bones it is using.
	BoneCountMap boneCountMap;
	this->getBoneCounts( boneCountMap );
	BoneCountMapIt bcmBeginIt = boneCountMap.begin();
	BoneCountMapIt bcmEndIt = boneCountMap.end();
	for ( BoneCountMapIt it = bcmBeginIt; it != bcmEndIt; ++it )
	{
		if (it->second > 256)
		{
			char msg[256];
			bw_snprintf(
				msg,
				sizeof(msg),
				"There are %d bones in %s.  "
					"This exceeds the 256 bone limit!\n"
					"This object may not appear correctly inside BigWorld!",
					it->second, it->first.c_str() );
			::MessageBox( 0, msg, "Visual exporter - Bone warning!", MB_ICONWARNING );
		}
	}

	bool hasPortal = !visualPortals_.empty();
	if( !hasPortal )
	{
		for( std::vector<VisualMeshPtr>::iterator iter = hullMeshes_.begin();
			iter != hullMeshes_.end(); ++iter )
			if( ((HullMesh*)iter->getObject())->hasPortal() )
			{
				hasPortal = true;
				break;
			}
	}

	if( !isShell( resNameCamelCase ) && hasPortal )
	{
		if( MessageBox( ip_->GetMAXHWnd(),
			"You are exporting an object with portals into a non shell directory.\n"
			"If you continue this object will not be recognised as a shell",
			"Visual Exporter", MB_OKCANCEL | MB_ICONEXCLAMATION ) == IDCANCEL )
			return 1;
	}

	// Calculate a few useful file names
	char buf[256];
	bw_snprintf( buf, sizeof(buf), "exporter_%03d_temp.visual", exportCount++ );

	std::string tempFileName = BWResource::getFilePath( fileName ) + buf;
	std::string tempResName = BWResolver::dissolveFilename( tempFileName );
	std::string tempPrimFileName = BWResource::removeExtension( tempFileName ) + ".primitives";
	std::string tempPrimResName = BWResource::removeExtension( tempResName ) + ".primitives";
	std::string primFileName = BWResource::removeExtension( fileName ) + ".primitives";
	std::string primResName = BWResource::removeExtension( resName ) + ".primitives";
	std::string visualBspFileName = BWResource::removeExtension( fileName ) + ".bsp";

#if DEBUG_PRIMITIVE_FILE
	// Delete the previous primitive debug xml file is it exists
	std::string primDebugXmlFileName = BWResource::removeExtension( fileName ) + ".primitives.xml";
	::DeleteFile( primDebugXmlFileName.c_str() );
#endif

	// Make sure we don't have any of the file hanging around in the cache
	BWResource::instance().purgeAll();
	BWResource::instance().purge( resName, true );
	BWResource::instance().purge( tempResName, true );
	BWResource::instance().purge( primResName, true );
	BWResource::instance().purge( tempPrimResName, true );

	if (visualMeshes_.size() == 1 && 
		meshNodes_.size() > 0 &&
		settings_.exportMode() == ExportSettings::NORMAL &&
		!visualMeshes_[0]->hasMorphTargets() &&
		envelopeNodes_.size() == 0 &&
		settings_.visualCheckerEnable() == true)
	{
		if (IDCANCEL == MessageBox( ip_->GetMAXHWnd(),
			"The visual appears to be static.  If it is static press <Cancel> to stop " \
			"exporting and then re-export the visual as a STATIC or STATIC WITH NODES visual.\n" \
			"If the visual is animated press <OK> to continue." ,
			"VisualExporter Warning", MB_OKCANCEL | MB_ICONWARNING ))
		{
			return 1;
		}
	}

	//Check all resources used by all VisualMeshes.
	std::vector<std::string> allResources;
	for (uint32 i=0; i<visualMeshes_.size(); i++)
	{
		visualMeshes_[i]->resources(allResources);		
	}

	std::vector<std::string> invalidResources;
	for (uint32 i=0; i<allResources.size(); i++)
	{
		std::string temp;
		if (!validResource( allResources[i], temp ))
		{
			invalidResources.push_back(allResources[i]);
		}
	}

	if (!invalidResources.empty())
	{
		invalidResourceDlg( invalidResources, ip_ );
		errors_ = true;
		return 1;
	}

	std::string visualBspResName = BWResource::removeExtension( resName ) + ".bsp";

	// For keeping the current material kind setting.
	// Also, load up the existing visual file, if any.  We will try and use
	// the materials from the existing visual file if the ids match.  If there
	// is no existing file, no worries.
	DataSectionPtr pExistingVisual = BWResource::openSection( resName, false );

	std::string materialKind;
	{
		if (pExistingVisual)
			materialKind = pExistingVisual->readString( "materialKind" );
	}

	if (!settings_.keepExistingMaterials())
		pExistingVisual = NULL;

	DataResource visualFile( tempFileName, RESOURCE_TYPE_XML );

	DataSectionPtr pVisualSection = visualFile.getRootSection();

	bool hasMeshes = false;

	if (pVisualSection)
	{
		//------------------------------------
		//Export the nodes
		//------------------------------------
		pVisualSection->delChildren();
		if (mfxRoot_)
		{
			mfxRoot_->setMaxNode( NULL );
			mfxRoot_->setIdentifier( "Scene Root" );
			if (settings_.exportMode() == ExportSettings::STATIC
				|| settings_.exportMode() == ExportSettings::MESH_PARTICLES)
			{
				mfxRoot_->delChildren();
			}

			bool hasInvalidTransforms = false;
			mfxRoot_->exportTree( pVisualSection, 0, &hasInvalidTransforms );
			if ( hasInvalidTransforms )
				::MessageBox(	0,
								"The visual contains bones with invalid transforms (probably due to bone scaling)!\n"
								"This may introduce animation artifacts inside BigWorld!",
								"Invalid bone transform warning!",
								MB_ICONWARNING );
		}

		if (!materialKind.empty())
			pVisualSection->writeString( "materialKind", materialKind );


		//------------------------------------
		//Export the primitives and vertices
		//
		//Calculate the bounding box while doing so
		//------------------------------------

		BoundingBox bb = BoundingBox::s_insideOut_;
		VisualMeshPtr accMesh = new VisualMesh();

		// Remove the primitives file from disk to get rid of any unwanted sections in it
		::DeleteFile( tempPrimFileName.c_str() );

		std::vector<VisualMeshPtr> origVisualMeshes = visualMeshes_;

		bool bLargeIndices = false;
		uint32 meshCount = visualMeshes_.size() + bspMeshes_.size() + hullMeshes_.size();
		int count = 0;
		while (visualMeshes_.size())
		{
			if (settings_.exportMode() == ExportSettings::NORMAL)
			{
				visualMeshes_.back()->save( pVisualSection, pExistingVisual, tempPrimFileName, true );
				bLargeIndices = (bLargeIndices || visualMeshes_.back()->largeIndices());

#if DEBUG_PRIMITIVE_FILE
				visualMeshes_.back()->savePrimXml( primDebugXmlFileName );
#endif
			}
			else
			{
				accMesh->dualUV( accMesh->dualUV() || visualMeshes_.back()->dualUV() );
				accMesh->vertexColours( accMesh->vertexColours() || visualMeshes_.back()->vertexColours() );

				bool worldSpaceAcc = 
					settings_.exportMode() == ExportSettings::MESH_PARTICLES ? 
					false : true;
				visualMeshes_.back()->add( *accMesh, count++, worldSpaceAcc );
			}

			bb.addBounds( visualMeshes_.back()->bb() );

			visualMeshes_.pop_back();
		}

		for (uint32 i = 0; i < bspMeshes_.size(); i++)
		{
			bb.addBounds( bspMeshes_[i]->bb() );
		}

		for (uint32 i = 0; i < hullMeshes_.size(); i++)
		{
			bb.addBounds( hullMeshes_[i]->bb() );
		}

		// Make sure we have the 15 meshes if exporting as mesh particles
		if (settings_.exportMode() == ExportSettings::MESH_PARTICLES)
		{
			int currentIndex = 0;
			while (count < 15 && !origVisualMeshes.empty())
			{
				dprintf( "adding mesh %d as index %d\n", currentIndex, count );
				origVisualMeshes[currentIndex++]->add( *accMesh, count, false );
				if (currentIndex == origVisualMeshes.size())
					currentIndex = 0;

				++count;
			}
		}

		bool saved=true;
		if (settings_.exportMode() != ExportSettings::NORMAL)
		{
			if (count)
			{
				saved=accMesh->save( pVisualSection, pExistingVisual, tempPrimFileName, false );
				bLargeIndices = (bLargeIndices || accMesh->largeIndices());

#if DEBUG_PRIMITIVE_FILE
				accMesh->savePrimXml( primDebugXmlFileName );
#endif
			}
		}
		if ( !saved )
		{
			return IMPEXP_FAIL;
		}
		accMesh = NULL;

		hasMeshes = meshCount > 0;

		if (ExportSettings::instance().useLegacyOrientation())
		{
			bb.setBounds(	Vector3(	-bb.maxBounds().x,
										bb.minBounds().z,
										-bb.maxBounds().y ) *
										ExportSettings::instance().unitScale(),
							Vector3(	-bb.minBounds().x,
										bb.maxBounds().z,
										-bb.minBounds().y ) *
										ExportSettings::instance().unitScale());
		}
		else
		{
			bb.setBounds(	Vector3(	bb.minBounds().x,
										bb.minBounds().z,
										bb.minBounds().y ) *
										ExportSettings::instance().unitScale(),
							Vector3(	bb.maxBounds().x,
										bb.maxBounds().z,
										bb.maxBounds().y ) *
										ExportSettings::instance().unitScale());
		}

		if(hasMeshes)
		{
			pVisualSection->writeVector3( "boundingBox/min", bb.minBounds() );
			pVisualSection->writeVector3( "boundingBox/max", bb.maxBounds() );
		}

		//------------------------------------
		// Export the boundaries and portals
		//------------------------------------
		// First remove all existing boundary sections from the visual
		DataSectionPtr pBoundary = pVisualSection->openSection( "boundary" );
		while ( pBoundary )
		{
			pVisualSection->delChild( pBoundary );
			pBoundary = pVisualSection->openSection( "boundary" );
		}

		if ( isShell(resNameCamelCase) && settings_.exportMode() != ExportSettings::NORMAL )
		{
			// Export hull tree for all objects if there is a custom hull defined.
			if (!hullMeshes_.empty())
			{
				exportHull( pVisualSection );
				pVisualSection->writeBool( "customHull", true );
			}
			else
			{
				generateHull( pVisualSection, bb );
				pVisualSection->writeBool( "customHull", false );
			}

			// Now write any explicitly set portal nodes.
			// The following method finds boundaries for our portals
			// and writes them into the appropriate boundary sections.
			exportPortalsToBoundaries( pVisualSection, bb );
		}

		if (settings_.exportMode() != ExportSettings::NORMAL && !bspMeshes_.empty())
			pVisualSection->writeBool( "customBsp", true );
		else if (bLargeIndices)
		{
			MessageBox( ip_->GetMAXHWnd(), "The mesh being exported is very large. It is advisable to have a custom BSP defined.",
			"VisualExporter Warning", MB_OK | MB_ICONWARNING );
		}
	}

	visualFile.save();

	pVisualSection = NULL;
	BWResource::instance().purge( tempResName, true );
	BWResource::instance().purge( tempPrimResName, true );

	Moo::VisualSplitter* pSplitter = new Moo::VisualSplitter( settings_.boneCount() );

	pSplitter->open( tempResName );
	if( pSplitter->split() )
		pSplitter->save( tempFileName );
	else
	{
		delete pSplitter;
		::DeleteFile( tempFileName.c_str() );
		::DeleteFile( tempPrimFileName.c_str() );
		::MessageBox( 0, "Failed to split the meshes", "Visual Exporter", MB_ICONERROR );
		return 0;
	}
	delete pSplitter;

	BWResource::instance().purge( tempResName, true );
	BWResource::instance().purge( tempPrimResName, true );

	pVisualSection = BWResource::openSection( tempResName );

	if (settings_.visualCheckerEnable())
	{
		// Check the visual is valid
		if ( hasMeshes && !vc.check( pVisualSection, tempPrimResName ) )
		{
			std::string errs = vc.errorText();
			::MessageBox( 0, errs.c_str(), "Visual failed validity checking", MB_ICONERROR );

			::DeleteFile( tempFileName.c_str() );
			::DeleteFile( tempPrimFileName.c_str() );
			return 0;
		}
	}


	// Passed checking, rename to proper name
	::DeleteFile( fileName.c_str() );
	::DeleteFile( primFileName.c_str() );

	::MoveFile( tempFileName.c_str(), fileName.c_str() );
	
	// we do not need a primitive file if there are no meshes
	if (hasMeshes)
		::MoveFile( tempPrimFileName.c_str(), primFileName.c_str() );
	else
		::DeleteFile( tempPrimFileName.c_str() );


	// Changed to update the model file even if it already exists..
	if (settings_.exportMode() != ExportSettings::MESH_PARTICLES)
	{
		DataResource modelFile( BWResource::removeExtension( fileName ) + ".model", RESOURCE_TYPE_XML );
		DataSectionPtr pModelSection = modelFile.getRootSection();
		if (pModelSection)
		{
			MetaData::updateCreationInfo( pModelSection );
			pModelSection->deleteSections( "nodefullVisual" );
			pModelSection->deleteSections( "nodelessVisual" );
			std::string filename = BWResource::removeExtension( resName );
			if (settings_.exportMode() == ExportSettings::NORMAL || 
				settings_.exportMode() == ExportSettings::STATIC_WITH_NODES)
				pModelSection->writeString( "nodefullVisual", filename );
			else
				pModelSection->writeString( "nodelessVisual", filename );
		}
		modelFile.save();
	}

	// purge primitivefile minicache
	PrimitiveFile::get( "" );
	// Make sure there's no existing bsp file
	::DeleteFile( visualBspFileName.c_str() );

	//------------------------------------
	//Export the BSP 
	//------------------------------------
	std::vector<std::string> bspMaterialIDs;
	// Export bsp tree (don't export bsps for MESH_PARTICLES).
	if (settings_.exportMode() != ExportSettings::MESH_PARTICLES)
	{
		// Export bsp tree for all objects if there is a custom bsp tree defined.
		if (!bspMeshes_.empty())
		{
			std::string bspFileName = BWResource::removeExtension( fileName ) + "_bsp.visual";
			std::string bspResName = BWResource::removeExtension( resName ) + "_bsp.visual";
			std::string bspPrimFileName = BWResource::removeExtension( fileName ) + "_bsp.primitives";
			std::string bspPrimResName = BWResource::removeExtension( resName ) + "_bsp.primitives";
			std::string bspBspFileName = BWResource::removeExtension( fileName ) + "_bsp.bsp";

			DataResource bspFile( bspFileName, RESOURCE_TYPE_XML );

			DataSectionPtr pBSPSection = bspFile.getRootSection();

			if (pBSPSection)
			{
				pBSPSection->delChildren();

				// Use same material kind as the real visual, so we get the correct data in the bsp
				if (!materialKind.empty())
					pBSPSection->writeString( "materialKind", materialKind );

				if (mfxRoot_)
				{
					mfxRoot_->delChildren();
					mfxRoot_->exportTree( pBSPSection );
				}

				BoundingBox bb = BoundingBox::s_insideOut_;
				VisualMeshPtr accMesh = new VisualMesh();

				while (bspMeshes_.size())
				{
					bspMeshes_.back()->add( *accMesh );

					bb.addBounds( bspMeshes_.back()->bb() );

					bspMeshes_.pop_back();
				}

				accMesh->save( pBSPSection, pExistingVisual, bspPrimFileName, false );
				accMesh = NULL;

				bb.setBounds(	Vector3(	bb.minBounds().x,
											bb.minBounds().z,
											bb.minBounds().y ),
								Vector3(	bb.maxBounds().x,
											bb.maxBounds().z,
											bb.maxBounds().y ));
				pBSPSection->writeVector3( "boundingBox/min", bb.minBounds() * ExportSettings::instance().unitScale() );
				pBSPSection->writeVector3( "boundingBox/max", bb.maxBounds() * ExportSettings::instance().unitScale() );
			}

			bspFile.save();

			// Generate the bsp for the bsp visual
			generateBSP( bspResName, BWResource::removeExtension( bspResName ) + ".bsp", bspMaterialIDs );

			// Rename the .bsp file to the real visuals name
			::MoveFile( bspBspFileName.c_str(), visualBspFileName.c_str() );

			// Clean up the bsp visual
			::DeleteFile( bspFileName.c_str() );
			::DeleteFile( bspPrimFileName.c_str() );
		}
		// Only generate a bsp if the object is static, 
		else if (settings_.exportMode() != ExportSettings::NORMAL)
		{
			// Generate the bsp from the plain visual
			generateBSP( resName, visualBspResName, bspMaterialIDs );
		}

		
		// We may now have a .bsp file, move it into the .primitives file
		DataSectionPtr ds = BWResource::openSection( visualBspResName );
		BinaryPtr bp;

		if (ds && (bp = ds->asBinary()))
		{			
			DataSectionPtr p = BWResource::openSection( primResName );
			if (p)
			{
				p->writeBinary( "bsp2", bp );

				DataSectionPtr materialIDsSection = 
					BWResource::openSection( "temp_bsp_materials.xml", true, XMLSection::creator() );
				materialIDsSection->writeStrings( "id", bspMaterialIDs );
				p->writeBinary( "bsp2_materials", materialIDsSection->asBinary() );

				p->save( primResName );
			}
			else if(hasMeshes)
			{
				MessageBox( ip_->GetMAXHWnd(), "Warning: Cannot find primitive file", "MFExporter Error", MB_OK | MB_ICONERROR );
				return 1;
			}

			// Clean up the .bsp file
			::DeleteFile( visualBspFileName.c_str() );
		}
	}

	return 1;
}


bool MFXExport::isShell( const std::string& resName )
{
	std::string s = resName;
	std::replace( s.begin(), s.end(), '\\', '/' );
	return ( s.size() > 7 && s.substr( 0, 7 ) == "shells/" ) ||
		(s.find( "/shells/" ) != std::string::npos);
}


/**
 *	Export the user-defined hull for the shell.
 *	Convert all triangles in the hull model to planes,
 *	and detect if any planes are portals.
 */
void MFXExport::exportHull( DataSectionPtr pVisualSection )
{
	//Accumulate triangles into an accumulator HullMesh.
	//The Hull Mesh stores plane equations, and works out
	//which of the planes are also portals.
	HullMeshPtr hullMesh = new HullMesh();
	while (hullMeshes_.size())
	{
		hullMeshes_.back()->add( *hullMesh );
		hullMeshes_.pop_back();
	}

	//Save the boundary planes, and any attached portals.
	hullMesh->save( pVisualSection );
}


/**
 *	Generate a hull for a shell.
 *  Use the 6 planes of the bounding box of the visual,
 *	and make sure the portals are on a plane.
 */
void MFXExport::generateHull( DataSectionPtr pVisualSection, const BoundingBox& bb )
{
	Vector3 bbMin( bb.minBounds() );
	Vector3 bbMax( bb.maxBounds() );

	// now write out the boundary
	for (int b = 0; b < 6; b++)
	{
		DataSectionPtr pBoundary = pVisualSection->newSection( "boundary" );

		// figure out the boundary plane in world coordinates
		int sign = 1 - (b&1)*2;
		int axis = b>>1;
		Vector3 normal(
			sign * ((axis==0)?1.f:0.f),
			sign * ((axis==1)?1.f:0.f),
			sign * ((axis==2)?1.f:0.f) );
		float d = sign > 0 ? bbMin[axis] : -bbMax[axis];

		Vector3 localCentre = normal * d;
		Vector3 localNormal = normal;
		localNormal.normalise();
		PlaneEq localPlane( localNormal, localNormal.dotProduct( localCentre ) );

		pBoundary->writeVector3( "normal", localPlane.normal() );
		pBoundary->writeFloat( "d", localPlane.d() );
	}
}


/**
 *	This method reads a boundary section, and interprets it as
 *	a plane equation.  The plane equation is returned via the
 *	ret parameter.
 */
void MFXExport::planeFromBoundarySection(
	const DataSectionPtr pBoundarySection,
	PlaneEq& ret )
{
	Vector3 normal = pBoundarySection->readVector3( "normal", ret.normal() );
	float d = pBoundarySection->readFloat( "d", ret.d() );
	ret = PlaneEq( normal,d );
}


/**
 *	This method checks whether the two planes coincide, within a
 *	certain tolerance.
 */
bool MFXExport::portalOnBoundary( const PlaneEq& portal, const PlaneEq& boundary )
{
	//remember, portal normals face the opposite direction to boundary normals.
	const float normalTolerance = -0.999f;
	const float distTolerance = 0.0001f;	//distance squared

	if ( portal.normal().dotProduct( boundary.normal() ) < normalTolerance )
	{
		Vector3 p1( portal.normal() * portal.d() );
		Vector3 p2( boundary.normal() * boundary.d() );

		if ( (p1-p2).lengthSquared() < distTolerance )
			return true;
	}

	return false;
}


/**
 *	Go through our portals, and find the boundary they are on.
 *	Write the portals into the appropriate boundary sections.
 */
void MFXExport::exportPortalsToBoundaries( DataSectionPtr pVisualSection, const BoundingBox& bb )
{
	if (!visualPortals_.size())
		return;

	// store the boundaries as plane equations
	std::vector<DataSectionPtr> boundarySections;
	pVisualSection->openSections( "boundary", boundarySections );
	std::vector<PlaneEq> boundaries;
	for ( int b=0; b < boundarySections.size(); b++ )
	{
		PlaneEq boundary;
		planeFromBoundarySection( boundarySections[b], boundary );
		boundaries.push_back( boundary );
	}

	// now look at all our portals, and assign them to a boundary
	bool eachOnBoundary = true;
	bool portalCulled = false;
	while (visualPortals_.size())
	{
		bool portalCulledTemp = false;
		if( !visualPortals_.back()->cullHull( boundaries, portalCulledTemp ) )
		{
			visualPortals_.pop_back();
			eachOnBoundary = false;
			continue;
		}
		PlaneEq portalPlane;
		visualPortals_.back()->planeEquation( portalPlane );

		bool found = false;
		for ( int b=0; b < boundaries.size(); b++ )
		{
			if ( portalOnBoundary(portalPlane,boundaries[b]) )
			{
				visualPortals_.back()->save( boundarySections[b] );
				found = true;
				break;	//continue with next visual portal
			}
		}

		if( !found )
		{
			visualPortals_.back()->reverse();
			visualPortals_.back()->planeEquation( portalPlane );
			for ( int b=0; b < boundaries.size(); b++ )
			{
				if ( portalOnBoundary(portalPlane,boundaries[b]) )
				{
					visualPortals_.back()->save( boundarySections[b] );
					found = true;
					break;	//continue with next visual portal
				}
			}
		}

		if (!found)
			eachOnBoundary = false;
		else
			portalCulled |= portalCulledTemp;

		visualPortals_.pop_back();
	}

	// Warn the user that one or more of the exported portals were culled in some way
	if (portalCulled)
		::MessageBox(	0,
						"One or more portals were clipped because they extend past the hull/bounding box!\n"
						"This may lead to snapping issues inside WorldEditor.\n"
						"Hint: A custom hull can be used to resolve this issue.",
						"Portal clipping warning!",
						MB_ICONWARNING );

	// Warn the user that one or more portals is not being exported
	if (!eachOnBoundary)
		::MessageBox(	0,
						"One or more portals will not be exported since they are not on a boundary!",
						"Portal error!",
						MB_ICONERROR );
}

/*
 *	List of superclass id's that are ignored when exporting
 */
static uint32 s_nonExportableSuperClass[] =
{
	CAMERA_CLASS_ID,
	LIGHT_CLASS_ID,
	SHAPE_CLASS_ID
};
const uint32 NUM_NONEXPORTABLE_SUPER = sizeof( s_nonExportableSuperClass ) / sizeof( uint32 );

/*
 *	List of class id's that are ignored when exporting
 */
static Class_ID s_nonExportableClass[] =
{
	Class_ID(TARGET_CLASS_ID, 0),
	Class_ID(0x74f93b07, 0x1eb34300) // Particle view class id
};
const uint32 NUM_NONEXPORTABLE_CLASS = sizeof( s_nonExportableClass ) / sizeof( Class_ID );


/**
 * Preprocess all the nodes, counts the total number of nodes.
 * Adds all used materials to the material list etc.
 */
void MFXExport::preProcess( INode * node, MFXNode *mfxParent )
{
	nodeCount_++;
	
	bool includeNode = false;

	if (settings_.nodeFilter() == ExportSettings::SELECTED )
	{
		if (node->Selected() && !node->IsHidden())
			includeNode = true;
	}
	else if (settings_.nodeFilter() == ExportSettings::VISIBLE)
	{
		if (!node->IsHidden())
			includeNode = true;
	}

	if (toLower(node->GetName()).find("_bsp") != std::string::npos &&
		!node->IsHidden())
	{
		if (settings_.nodeFilter() == ExportSettings::SELECTED )
		{
			if (node->Selected())
			{
				meshNodes_.push_back( node );
				return;
			}
		}
		else
		{
			meshNodes_.push_back( node );
			return;
		}
	}

	ObjectState os = node->EvalWorldState( staticFrame() );

	if( os.obj )
	{
		Class_ID* foundClass = std::find( s_nonExportableClass, s_nonExportableClass +
			NUM_NONEXPORTABLE_CLASS, os.obj->ClassID() );

		if (foundClass != s_nonExportableClass + NUM_NONEXPORTABLE_CLASS)
			includeNode = false;

		uint32* found =  std::find( s_nonExportableSuperClass, s_nonExportableSuperClass + 
			NUM_NONEXPORTABLE_SUPER, os.obj->SuperClassID() );
		if (found != s_nonExportableSuperClass + NUM_NONEXPORTABLE_SUPER)
		{
			includeNode = false;
		}


/*		std::string nodeName = node->GetName();
		INFO_MSG( "--- Object: %s\n", nodeName.c_str() );
		INFO_MSG( "******* Superclass 0x%08x\n", os.obj->SuperClassID() );
		Class_ID cid = os.obj->ClassID();
		INFO_MSG( "******* Class ID 0x%08x 0x%08x\n", cid.PartA(), cid.PartB() );*/
	}


	if (includeNode && node->Renderable())
	{
		BOOL isPortal = false;
		node->GetUserPropBool( "portal", isPortal );

		Modifier *pPhyMod = findPhysiqueModifier( node );
		Modifier* pSkinMod = findSkinMod( node );

		std::string nodeName = node->GetName();
		std::string nodePrefix;
		if (nodeName.length() > 3)
		{
			nodePrefix = toLower(nodeName.substr( 0, 3 ));
		}

		if (isPortal)
		{
			portalNodes_.push_back( node );
			includeNode = false;
		}
		else if( (pPhyMod || pSkinMod) && settings_.exportMode() == ExportSettings::NORMAL )
		{
			envelopeNodes_.push_back( node );
			includeNode = false;
		}
		else if (nodePrefix != "hp_")
		{
			meshNodes_.push_back( node );
		}
	}

	MFXNode* thisNode = mfxParent;

	thisNode = new MFXNode( node );
	thisNode->include( includeNode );
	if (mfxParent)
		mfxParent->addChild( thisNode );
	else
		mfxRoot_ = thisNode;

	if (includeNode)
		thisNode->includeAncestors();

	if (includeNode && trailingLeadingWhitespaces(thisNode->getIdentifier()))
	{
		char s[1024];
		bw_snprintf( s, sizeof(s), "Node \"%s\" has an illegal name. Remove any spaces at the beginning and end of the name.", thisNode->getIdentifier().c_str() );
		MessageBox( ip_->GetMAXHWnd(), s, "MFExporter Error", MB_OK | MB_ICONWARNING );
	}

	for( int index = 0; index < node->NumberOfChildren(); index++ )
		preProcess( node->GetChildNode( index ), thisNode );

}

/*
 * Create the config filename and path from the predefined config filename
 * and the global plugin config directory.
 */
std::string MFXExport::getCfgFilename( void )
{
	std::string filename = ip_->GetDir(APP_PLUGCFG_DIR);
	filename += "\\";
	filename += CFGFilename;
	
	return filename;
}

void MFXExport::exportMeshes( bool snapVertices )
{
	for( int i = 0; i < meshNodes_.size(); i++ )
	{
		exportMesh( meshNodes_[ i ], snapVertices );
	}
}

void MFXExport::exportMesh( INode *node, bool snapVertices )
{

	bool deleteIt = false;

	bool bspNode = (toLower(node->GetName()).find("_bsp") != std::string::npos);
	bool hullNode = (toLower(node->GetName()).find("_hull") != std::string::npos);

	VisualMeshPtr spVisualMesh = new VisualMesh;
	spVisualMesh->snapVertices( snapVertices );

	if (spVisualMesh)
	{
		if (spVisualMesh->init( node ) )
		{
			if (bspNode)
			{
				bspMeshes_.push_back( spVisualMesh );
			}
			else if (hullNode)
			{
				hullMeshes_.push_back( spVisualMesh );
			}
			else
			{
				visualMeshes_.push_back( spVisualMesh );
			}
		}
	}
}

//
//	- exportEnvelopes
//  Goes through the list of envelopenodes and exports them through exportenvelope 
//
void MFXExport::exportEnvelopes( void )
{
	for( uint32 i = 0; i < envelopeNodes_.size(); i++ )
	{
		exportEnvelope( envelopeNodes_[ i ] );
	}
}

/*
 * 
 * 
 */
void MFXExport::exportEnvelope( INode *node )
{

	VisualEnvelopePtr spVisualEnvelope = new VisualEnvelope;
	if (spVisualEnvelope)
	{
		if(spVisualEnvelope->init( node, mfxRoot_ ))
			visualMeshes_.push_back( spVisualEnvelope );
	}
}

void MFXExport::exportPortals()
{
	for( INodeVector::iterator it = portalNodes_.begin(); it != portalNodes_.end() ; it++ )
	{
		exportPortal( *it );
	}
}

void MFXExport::exportPortal( INode* node )
{
	if( !node->IsHidden() )
	{
		bool deleteIt = false;

		//Get triangulated object.
		TriObject* tri = getTriObject( node, staticFrame(), deleteIt );
		if( tri )
		{
			Mesh *mesh = &tri->mesh;

			if( mesh->getNumFaces() && mesh->getNumVerts() )
			{
				std::string identifier = node->GetName();
				Matrix3 portalMatrix = node->GetObjectTM( staticFrame() );

				bool inverted = isMirrored( portalMatrix );

				UniqueVertices verts;

				for( int i = 0; i < mesh->getNumFaces(); i++ )
				{
					if( inverted )
					{
						verts.addVertex( VertexContainer( mesh->faces[ i ].v[ 2 ], 0 ) );
						verts.addVertex( VertexContainer( mesh->faces[ i ].v[ 1 ], 0 ) );
						verts.addVertex( VertexContainer( mesh->faces[ i ].v[ 0 ], 0 ) );
					}
					else
					{
						verts.addVertex( VertexContainer( mesh->faces[ i ].v[ 0 ], 0 ) );
						verts.addVertex( VertexContainer( mesh->faces[ i ].v[ 1 ], 0 ) );
						verts.addVertex( VertexContainer( mesh->faces[ i ].v[ 2 ], 0 ) );
					}
				}
				
				VisualPortalPtr vpp = new VisualPortal();
				
				vpp->nameFromProps( node );

				for( int i = 0; i < verts.v().size(); i++ )
				{
					Point3 ppp = applyUnitScale(
						mesh->verts[ verts.v()[ i ].getV() ] * portalMatrix );
					
					if ( settings_.snapVertices() )
						ppp = snapPoint3( ppp );
					vpp->addPoint( *(Vector3*)&ppp );
				}

				vpp->createConvexHull();

				visualPortals_.push_back( vpp );

				OutputDebugString( "Exported portal " );
				OutputDebugString( identifier.c_str() );
				OutputDebugString( "\n" );
			}
		}
	}
}

/**
 * Returns the bone count for each visual envelope object in a map, using the 
 * object name as the key and the bone count as the value.
 */
void MFXExport::getBoneCounts( BoneCountMap& boneCountMap )
{
	typedef std::vector<VisualMeshPtr>::iterator visualMeshVectorIt;

	// If there are no visual meshes return zero
	if (visualMeshes_.empty())
	{
		return;
	}

	// For each visual envelope, add its name and bone count
	visualMeshVectorIt beginIt = visualMeshes_.begin();
	visualMeshVectorIt endIt = visualMeshes_.end();
	for ( visualMeshVectorIt it = beginIt; it != endIt; ++it )
	{
		VisualEnvelope* visEnv =
			((*it)->isVisualEnvelope()) ?
				static_cast<VisualEnvelope*>(it->getObject()) :
				NULL;

		if (visEnv)
		{
			boneCountMap.insert(
				std::pair< std::string, int >( 
					visEnv->getIdentifier(), visEnv->boneNodesCount() ) );
		}
	}
}

TriObject* MFXExport::getTriObject( INode *node, TimeValue t, bool &needDelete )
{
	needDelete = false;
	ObjectState os = node->EvalWorldState( t );

	if( os.obj && ( os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID ) )
	{
		Object *obj = os.obj;
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
		{ 
			TriObject *tri = (TriObject *) obj->ConvertToType( t, Class_ID(TRIOBJ_CLASS_ID, 0) );
			if ( obj != tri ) 
				needDelete = true;

			return tri;
		}
	}
	return NULL;
}

Modifier* MFXExport::findMorphModifier( INode* node )
{
	// Get object from node. Abort if no object.
	Object* object = node->GetObjectRef();

	// Is derived object ?
	while( object && object->SuperClassID() == GEN_DERIVOB_CLASS_ID )
	{
		// Yes -> Cast.
		IDerivedObject* derivedObject = static_cast< IDerivedObject* >( object );

		// Iterate over all entries of the modifier stack.
		int modStackIndex = 0;
		while( modStackIndex < derivedObject->NumModifiers() )
		{
			// Get current modifier.
			Modifier* modifier = derivedObject->GetModifier( modStackIndex );

			// Is this the morpher modifier?
			if( modifier->ClassID() == MR3_CLASS_ID )
			{
				// Yes -> Exit.
				return modifier;
			}

			// Next modifier stack entry.
			modStackIndex++;
		}
		object = derivedObject->GetObjRef();
	}

	// Not found.
	return NULL;
}

Modifier* MFXExport::findPhysiqueModifier( INode* node )
{
	// Get object from node. Abort if no object.
	Object* object = node->GetObjectRef();

	if( !object ) 
		return NULL;

	// Is derived object ?
	if( object->SuperClassID() == GEN_DERIVOB_CLASS_ID )
	{
		// Yes -> Cast.
		IDerivedObject* derivedObject = static_cast< IDerivedObject* >( object );

		// Iterate over all entries of the modifier stack.
		int modStackIndex = 0;
		while( modStackIndex < derivedObject->NumModifiers() )
		{
			// Get current modifier.
			Modifier* modifier = derivedObject->GetModifier( modStackIndex );

			// Is this Physique ?
			if( modifier->ClassID() == Class_ID( PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B ) )
			{
				// Yes -> Exit.
				return modifier;
			}

			// Next modifier stack entry.
			modStackIndex++;
		}
	}

	// Not found.
	return NULL;
}

IEditNormalsMod* MFXExport::findEditNormalsMod( INode* node )
{
	// Get object from node. Abort if no object.
	Object* object = node->GetObjectRef();

	if( !object ) 
		return NULL;

	// Is derived object ?
	if( object->SuperClassID() == GEN_DERIVOB_CLASS_ID )
	{
		// Yes -> Cast.
		IDerivedObject* derivedObject = static_cast< IDerivedObject* >( object );

		// Iterate over all entries of the modifier stack.
		int modStackIndex = 0;
		while( modStackIndex < derivedObject->NumModifiers() )
		{
			// Get current modifier.
			Modifier* modifier = derivedObject->GetModifier( modStackIndex );

			// Does this modifier have the editnormals interface?
			IEditNormalsMod* mod = (IEditNormalsMod*)modifier->GetInterface( EDIT_NORMALS_MOD_INTERFACE );
			if (mod)
				return mod;

			// Next modifier stack entry.
			modStackIndex++;
		}
	}

	// Not found.
	return NULL;
}

Modifier* MFXExport::findSkinMod( INode* node )
{
	// Get object from node. Abort if no object.
	Object* object = node->GetObjectRef();

	if( !object ) 
		return NULL;

	// Is derived object ?
	if( object->SuperClassID() == GEN_DERIVOB_CLASS_ID )
	{
		// Yes -> Cast.
		IDerivedObject* derivedObject = static_cast< IDerivedObject* >( object );

		// Iterate over all entries of the modifier stack.
		int modStackIndex = 0;
		while( modStackIndex < derivedObject->NumModifiers() )
		{
			// Get current modifier.
			Modifier* modifier = derivedObject->GetModifier( modStackIndex );

			// Is this Physique ?
			if( modifier->ClassID() == SKIN_CLASSID )
			{
				// Yes -> Exit.
				return modifier;
			}

			// Next modifier stack entry.
			modStackIndex++;
		}
	}

	// Not found.
	return NULL;
}


std::ostream& operator<<(std::ostream& o, const MFXExport& t)
{
	o << "MFXExport\n";
	return o;
}
