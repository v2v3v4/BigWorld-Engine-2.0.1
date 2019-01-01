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

#include <maya/MStringArray.h>
#include <maya/MItDag.h>
#include <maya/MFnMesh.h>

#include "VisualFileTranslator.hpp"
#include "expsets.hpp"
#include "mesh.hpp"
#include "portal.hpp"
#include "blendshapes.hpp"
#include "visual_mesh.hpp"
#include "visual_envelope.hpp"
#include "visual_portal.hpp"
#include "hull_mesh.hpp"
#include "bsp_generator.hpp"
#include "hierarchy.hpp"
#include "utility.hpp"

#include <cstdmf/stdmf.hpp>
#include <cstdmf/dprintf.hpp>
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/binaryfile.hpp"
#include "resmgr/primitive_file.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/dataresource.hpp"
#include "resmgr/xml_section.hpp"
#include "gizmo/meta_data_creation_utility.hpp"
#include "math/blend_transform.hpp"
#include "../resourcechecker/visual_checker.hpp"
#include "exportiterator.h"
#include "node_catalogue_holder.hpp"
#include "data_section_cache_purger.hpp"
#include "moo/node.hpp"
#include "moo/visual_splitter.hpp"
#include "resource.h"
#include <commctrl.h>
#include "exporter_dialog.hpp"

// Defines
#define DEBUG_PRIMITIVE_FILE 0
#define DEBUG_ANIMATION_FILE 0
#define DEBUG_VISUAL_FILE 0

MDagPath findDagPath( std::string path );

void addAllHardpoints(Hierarchy& hierarchy)
{
	MStatus status;
	ExportIterator it( MFn::kTransform, &status );

	if( status == MStatus::kSuccess )
	{
		for( ; it.isDone() == false; it.next() )
		{
			MFnTransform transform( it.item() );

			std::string path = transform.fullPathName().asChar();
			std::string checkPath = path;
			if (checkPath.find_last_of("|") != std::string::npos)
			{
				checkPath = checkPath.substr(checkPath.find_last_of("|"));
			}
			if (checkPath.find("HP_") != std::string::npos)
				hierarchy.addNode( path, findDagPath( path ) );
		}
	}
}

void exportTree( DataSectionPtr pParentSection, Hierarchy& hierarchy, std::string parent = "", bool* hasInvalidTransforms = 0 )
{
// RA: Why not export hard point children??
//	if( parent.find( "HP_" ) != std::string::npos )
//		return;

	DataSectionPtr pThisSection = pParentSection;

	pThisSection = pParentSection->newSection( "node" );

	if (pThisSection )
	{
		if ( parent == "" )
			pThisSection->writeString( "identifier", "Scene Root" );
		else
			pThisSection->writeString( "identifier", hierarchy.name().substr( hierarchy.name().find_last_of( "|" ) + 1 ) );

		matrix4<float> m = hierarchy.transform( 0, true );
		Vector3 row0( m.v11, m.v21, m.v31 );
		Vector3 row1( m.v12, m.v22, m.v32 );
		Vector3 row2( m.v13, m.v23, m.v33 );
		Vector3 row3( m.v14, m.v24, m.v34 );
		row3 *= ExportSettings::instance().unitScale();
		pThisSection->writeVector3( "transform/row0", row0 );
		pThisSection->writeVector3( "transform/row1", row1 );
		pThisSection->writeVector3( "transform/row2", row2 );
		pThisSection->writeVector3( "transform/row3", row3 );

		// Check if the node transformation is valid
		Matrix testMatrix = pThisSection->readMatrix34( "transform", Matrix::identity );
		BlendTransform BTTest = BlendTransform( testMatrix );
		Quaternion quat = BTTest.rotation();
		if ( !almostEqual( quat.length(), 1.0f ) )
			*hasInvalidTransforms = true;
	}

	for( int i = 0; i < (int)hierarchy.children().size(); i++ )
	{
		exportTree( pThisSection, hierarchy.child( hierarchy.children()[i] ), parent + "|" + hierarchy.name(), hasInvalidTransforms );
	}
}

void exportAnimationXmlHeader(
	const std::string& animDebugXmlFileName, const std::string& animName, float numberOfFrames, int nChannels )
{
	DataSectionPtr spFileRoot = BWResource::openSection( animDebugXmlFileName );
	if ( !spFileRoot )
		spFileRoot = new XMLSection( "AnimationXml" );

	DataSectionPtr spFile = spFileRoot->newSection( animName );

	// Print the animation header
	DataSectionPtr pAH = spFile->newSection( "AnimHeader" );
	pAH->writeString( "AnimName", animName );
	pAH->writeFloat( "NumFrame", numberOfFrames );
	pAH->writeInt( "NumChannels", nChannels );

	spFileRoot->save( animDebugXmlFileName );
}

void exportAnimationXml(
	DataSectionPtr spParentNode, Hierarchy& hierarchy, bool stripRefPrefix = false )
{
	DataSectionPtr pNode = spParentNode->newSection( "Node" );

	if( hierarchy.name() != "" )
	{
		if (stripRefPrefix)
			hierarchy.name( stripReferencePrefix( hierarchy.name() ) );
		
		typedef std::pair<float, Vector3> ScaleKey;
		typedef std::pair<float, Vector3> PositionKey;
		typedef std::pair<float, Quaternion> RotationKey;

		std::vector< int > boundTable;
		std::vector< ScaleKey > scales;
		std::vector< PositionKey > positions;
		std::vector< RotationKey > rotations;

		int firstFrame = 0;
		int lastFrame = hierarchy.numberOfFrames() - 1;

		float time = 0;
		for (int i = firstFrame; i <= lastFrame; i++)
		{
			matrix4<float> theMatrix = hierarchy.transform( i, true );

			Matrix m;
			m.setIdentity();
			m[0][0] = theMatrix.v11;
			m[0][1] = theMatrix.v21;
			m[0][2] = theMatrix.v31;
			m[0][3] = theMatrix.v41;
			m[1][0] = theMatrix.v12;
			m[1][1] = theMatrix.v22;
			m[1][2] = theMatrix.v32;
			m[1][3] = theMatrix.v42;
			m[2][0] = theMatrix.v13;
			m[2][1] = theMatrix.v23;
			m[2][2] = theMatrix.v33;
			m[2][3] = theMatrix.v43;
			m[3][0] = theMatrix.v14;
			m[3][1] = theMatrix.v24;
			m[3][2] = theMatrix.v34;
			m[3][3] = theMatrix.v44;
			BlendTransform bt( m );

			// Normalise the rotation quaternion
			bt.normaliseRotation();

			boundTable.push_back( i - firstFrame + 1 );
			scales.push_back( ScaleKey( time, bt.scaling() ) );
			positions.push_back( PositionKey( time, bt.translation() * ExportSettings::instance().unitScale() ) );
			rotations.push_back( RotationKey( time, bt.rotation() ) );
			time = time + 1;
		}

		std::string nodeName =
			(stripRefPrefix) ?
				( stripReferencePrefix( hierarchy.path().substr( hierarchy.path().find_last_of( "|" ) + 1 ) ) )
				:
				( hierarchy.path().substr( hierarchy.path().find_last_of( "|" ) + 1 ) );

		pNode->writeString( "NodeName", nodeName );

		DataSectionPtr pScales = pNode->newSection( "Scales" );
		std::vector< ScaleKey >::iterator scaleIt;
		for (scaleIt = scales.begin(); scaleIt != scales.end(); ++scaleIt)
		{
			DataSectionPtr pScale = pScales->newSection( "Scale" );
			pScale->writeFloat( "Key", scaleIt->first );
			pScale->writeVector3( "Value", scaleIt->second ); 
		}

		DataSectionPtr pPositions = pNode->newSection( "Positions" );
		std::vector< PositionKey >::iterator posIt;
		for (posIt = positions.begin(); posIt != positions.end(); ++posIt)
		{
			DataSectionPtr pPosition = pPositions->newSection( "Position" );
			pPosition->writeFloat( "Key", posIt->first );
			pPosition->writeVector3( "Value", posIt->second );
		}

		DataSectionPtr pRotations = pNode->newSection( "Rotations" );
		std::vector< RotationKey >::iterator rotIt;
		for (rotIt = rotations.begin(); rotIt != rotations.end(); ++rotIt)
		{
			DataSectionPtr pRotation = pRotations->newSection( "Rotation" );
			pRotation->writeFloat( "Key", rotIt->first );
			pRotation->writeVector4( "Value", Vector4(rotIt->second.x, rotIt->second.y, rotIt->second.z, rotIt->second.w) );
		}

		DataSectionPtr pBTS = pNode->newSection( "BoundTableScales" );
		pBTS->writeInts( "Value", boundTable );

		DataSectionPtr pBTP = pNode->newSection( "BoundTablePositions" );
		pBTP->writeInts( "Value", boundTable );

		DataSectionPtr pBTR = pNode->newSection( "BoundTableRotations" );
		pBTR->writeInts( "Value", boundTable );
	}

	std::vector<std::string> children = hierarchy.children();
	for (uint i = 0; i < children.size(); i++)
	{
		exportAnimationXml( pNode, hierarchy.child( children[i] ), stripRefPrefix );
	}
}

void exportAnimation( BinaryFile& animFile, Hierarchy& hierarchy, bool stripRefPrefix = false )
{
	if( hierarchy.name() != "" )
	{
		if (stripRefPrefix)
			hierarchy.name( stripReferencePrefix( hierarchy.name() ) );
		
		typedef std::pair<float, Vector3> ScaleKey;
		typedef std::pair<float, Vector3> PositionKey;
		typedef std::pair<float, Quaternion> RotationKey;

		std::vector< int > boundTable;
		std::vector< ScaleKey > scales;
		std::vector< PositionKey > positions;
		std::vector< RotationKey > rotations;

			int firstFrame = 0;//~ExportSettings::instance().firstFrame();
			int lastFrame = hierarchy.numberOfFrames() - 1;//~ExportSettings::instance().lastFrame();

			float time = 0;
			for (int i = firstFrame; i <= lastFrame; i++)
			{
				matrix4<float> theMatrix = hierarchy.transform( i, true );

				Matrix m;
				m.setIdentity();
				m[0][0] = theMatrix.v11;
				m[0][1] = theMatrix.v21;
				m[0][2] = theMatrix.v31;
				m[0][3] = theMatrix.v41;
				m[1][0] = theMatrix.v12;
				m[1][1] = theMatrix.v22;
				m[1][2] = theMatrix.v32;
				m[1][3] = theMatrix.v42;
				m[2][0] = theMatrix.v13;
				m[2][1] = theMatrix.v23;
				m[2][2] = theMatrix.v33;
				m[2][3] = theMatrix.v43;
				m[3][0] = theMatrix.v14;
				m[3][1] = theMatrix.v24;
				m[3][2] = theMatrix.v34;
				m[3][3] = theMatrix.v44;
				BlendTransform bt( m );

				// Normalise the rotation quaternion
				bt.normaliseRotation();

				boundTable.push_back( i - firstFrame + 1 );
				scales.push_back( ScaleKey( time, bt.scaling() ) );
				positions.push_back( PositionKey( time, bt.translation() * ExportSettings::instance().unitScale() ) );
				rotations.push_back( RotationKey( time, bt.rotation() ) );
				time = time + 1;
			}

			{
				animFile << int(1);

				if (stripRefPrefix)
					animFile << stripReferencePrefix( hierarchy.path().substr( hierarchy.path().find_last_of( "|" ) + 1 ) );
				else
					animFile << hierarchy.path().substr( hierarchy.path().find_last_of( "|" ) + 1 );
			}

			animFile.writeSequence( scales );
			animFile.writeSequence( positions );
			animFile.writeSequence( rotations );
			animFile.writeSequence( boundTable );
			animFile.writeSequence( boundTable );
			animFile.writeSequence( boundTable );
	}

	std::vector<std::string> children = hierarchy.children( true );

	for (uint i = 0; i < children.size(); i++)
	{
		exportAnimation( animFile, hierarchy.child( children[i] ), stripRefPrefix );
	}
}

bool isShell( const std::string& resName )
{
	std::string s = resName;
	std::replace( s.begin(), s.end(), '\\', '/' );
	return ( s.size() > 7 && s.substr( 0, 7 ) == "shells/" ) || (s.find( "/shells/" ) != std::string::npos);
}


/**
 *	Export the user-defined hull for the shell.
 *	Convert all triangles in the hull model to planes,
 *	and detect if any planes are portals.
 */
void exportHull( DataSectionPtr pVisualSection, std::vector<VisualMeshPtr>& hullMeshes )
{
	//Accumulate triangles into an accumulator HullMesh.
	//The Hull Mesh stores plane equations, and works out
	//which of the planes are also portals.
	HullMeshPtr hullMesh = new HullMesh();
	while ( hullMeshes.size() )
	{
		hullMeshes.back()->add( *hullMesh );
		hullMeshes.pop_back();
	}

	//Save the boundary planes, and any attached portals.
	hullMesh->save( pVisualSection );
}


/**
 *	Generate a hull for a shell.
 *  Use the 6 planes of the bounding box of the visual,
 *	and make sure the portals are on a plane.
 */
void generateHull( DataSectionPtr pVisualSection, const BoundingBox& bb )
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
void planeFromBoundarySection(
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
bool portalOnBoundary( const PlaneEq& portal, const PlaneEq& boundary )
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
void exportPortalsToBoundaries( DataSectionPtr pVisualSection, std::vector<VisualPortalPtr>& portals )
{
	if ( !portals.size() )
		return;

	// store the boundaries as plane equations
	std::vector<DataSectionPtr> boundarySections;
	pVisualSection->openSections( "boundary", boundarySections );
	std::vector<PlaneEq> boundaries;
	for ( size_t b=0; b < boundarySections.size(); b++ )
	{
		PlaneEq boundary;
		planeFromBoundarySection( boundarySections[b], boundary );
		boundaries.push_back( boundary );
	}


	Vector3 bbMin = pVisualSection->readVector3( "boundingBox/min" );
	Vector3 bbMax = pVisualSection->readVector3( "boundingBox/max" );
	Vector3 size = bbMax - bbMin;
	BoundingBox bb(bbMin, bbMax);

	// now look at all our portals, and assign them to a boundary
	bool eachOnBoundary = true;
	for (size_t i=0; i<portals.size(); ++i )
	{
		PlaneEq portalPlane;
		portals[i]->planeEquation( portalPlane );

		if ( bb.centre().dotProduct(portalPlane.normal()) - portalPlane.d() > 0.f)
		{
			// portal is facing outwards ...reversing
			portals[i]->reverseWinding();
			portals[i]->planeEquation( portalPlane );
		}

		bool found = false;
		for ( size_t b=0; b < boundaries.size(); b++ )
		{
			if ( portalOnBoundary(portalPlane,boundaries[b]) )
			{
				portals[i]->save( boundarySections[b] );
				found = true;
				break;	//continue with next visual portal
			}
		}

		if (!found)
			eachOnBoundary = false;
	}

	// Warn the user that one or more portals is not being exported
	if (!eachOnBoundary)
		::MessageBox(	0,
						L"One or more portals will not be exported since they are not on a boundary!",
						L"Portal error!",
						MB_ICONERROR );
}

bool checkHierarchy( Hierarchy& hierarchy, std::set<std::string>& names )
{
	std::string name = hierarchy.name().substr( hierarchy.name().find_last_of( "|" ) + 1 );
	if( names.find( name ) != names.end() )
		return false;
	names.insert( name );
	for( int i = 0; i < (int)hierarchy.children().size(); i++ )
	{
		if( ! checkHierarchy( hierarchy.child( hierarchy.children()[i] ), names ) )
			return false;
	}
	return true;
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
void invalidExportDlg( const std::string& fileName )
{
	std::string errorString;
	errorString = std::string( "The exported file \"" ) + fileName + std::string( "\" is not in a valid game path.\n" );
	errorString += "Need to export to a *subfolder* of:";
    uint32 pathIndex = 0;
	while (BWResource::getPath(pathIndex) != std::string(""))
	{
		errorString += std::string("\n") + BWResource::getPath(pathIndex++);
	}
	MessageBox( NULL, bw_utf8tow( errorString ).c_str(), L"Visual/Animation Exporter Error", MB_OK | MB_ICONWARNING );
}


/**
 *	Display a dialog explaining the resources is invalid.
 */
void invalidResourceDlg( const std::vector<std::string>& fileNames )
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
	MessageBox( NULL, bw_utf8tow( errorString ).c_str(), L"Visual/Animation Exporter Error", MB_OK | MB_ICONWARNING );
}


/**
 *	Returns a map of visual names and bone counts.
 */
void VisualFileTranslator::getBoneCounts( BoneCountMap& boneCountMap )
{
	typedef std::vector<VisualMeshPtr>::iterator visualMeshVectorIt;
	
	// If there are no visual meshes return zero
	if (visualMeshes.empty())
	{
		return;
	}

	// For each visual envelope, add its name and bone count
	visualMeshVectorIt beginIt = visualMeshes.begin();
	visualMeshVectorIt endIt = visualMeshes.end();
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


bool VisualFileTranslator::exportMesh( std::string fileName )
{
	static int exportCount = 0;

	// remove extension from filename
	fileName = BWResource::removeExtension( fileName ) + ".visual";

	// Check if the filename is in the right res path.
	std::string resName;
	if (!validResource( fileName, resName ))
	{
		invalidExportDlg( fileName );
		return false;
	}

	bool shell = isShell(resName);	// boolean for checking if this file will be a shell

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
	std::string visualBspResName = BWResource::removeExtension( resName ) + ".bsp";

#if DEBUG_PRIMITIVE_FILE
	// Delete the previous primitive debug xml file is it exists
	std::string primDebugXmlFileName = BWResource::removeExtension( fileName ) + ".primitives.xml";
	::DeleteFile( primDebugXmlFileName.c_str() );
#endif

	//Check all resources used by all VisualMeshes.
	std::vector<std::string> allResources;
	for (uint32 i=0; i<visualMeshes.size(); i++)
	{
		visualMeshes[i]->resources(allResources);
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
		invalidResourceDlg( invalidResources );
		return false;
	}

	BWResource::instance().purgeAll();

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

	if (!ExportSettings::instance().keepExistingMaterials())
		pExistingVisual = NULL;

	DataResource visualFile( tempFileName, RESOURCE_TYPE_XML );
	DataSectionPtr pVisualSection = visualFile.getRootSection();


	if (pVisualSection)
	{
		//------------------------------------
		//Export the nodes
		//------------------------------------
		pVisualSection->delChildren();
		Hierarchy hierarchy( NULL );

		// export hierarchy only for STATIC_WITH_NODES and NORMAL
		Mesh mesh;
		if ( ExportSettings::instance().exportMode() != ExportSettings::STATIC && ExportSettings::instance().exportMode() != ExportSettings::MESH_PARTICLES )
		{
			addAllHardpoints(hierarchy);
			hierarchy.getMeshes( mesh );
		}
		Skin skin;

		skin.trim( mesh.meshes() );
		if( skin.count() > 0 && skin.initialise( 0 ) &&
			ExportSettings::instance().exportMode() != ExportSettings::STATIC && ExportSettings::instance().exportMode() != ExportSettings::MESH_PARTICLES )
		{

			hierarchy.getSkeleton( skin );
			for( uint32 i = 1; i < skin.count(); ++i )
				if( skin.initialise( i ) )
					hierarchy.getSkeleton( skin );

			for( uint32 j = 0; j < skin.count(); ++j )
			{
				skin.initialise( j );
				for( uint32 i = 0; i < skin.numberOfBones(); ++i )
				{
						hierarchy.find( std::string( skin.boneDAG( i ).fullPathName().asChar() ) ).addNode( std::string( skin.boneDAG( i ).fullPathName().asChar() ) + "_BlendBone", matrix4<float>()/*skin.transform( i, false ).inverse()*/ );
				}
			}
		}
//		hierarchy.dump();
//		fflush( stdout );
/*		{
			std::set<std::string> names;
			names.insert( "Scene Root" );
			if( !checkHierarchy( hierarchy, names ) )
			{
				::MessageBox( 0, "There are 2 or more bones with same names in the hierarchy!", "Bone hierarchy validity checking", MB_ICONERROR );
				return false;
			}
		}*/

		bool hasInvalidTransforms = false;
		exportTree( pVisualSection, hierarchy, "", &hasInvalidTransforms );
		if ( hasInvalidTransforms )
			::MessageBox(	0,
							L"The visual contains bones with invalid transforms (probably due to bone scaling)!\n"
							L"This may introduce animation artifacts inside BigWorld!",
							L"Invalid bone transform warning!",
							MB_ICONWARNING );


		if (!materialKind.empty())
			pVisualSection->writeString( "materialKind", materialKind );


		//~ //------------------------------------
		//~ //Export the primitives and vertices
		//~ //
		//~ //Calculate the bounding box while doing so
		//~ //------------------------------------


		BoundingBox bb = BoundingBox::s_insideOut_;
		VisualMeshPtr accMesh = new VisualMesh();

		// Remove the primitives file from disk to get rid of any unwanted sections in it
		::DeleteFile( bw_utf8tow( tempPrimFileName ).c_str() );

		std::vector<VisualMeshPtr> origVisualMeshes = visualMeshes;

		// save out each mesh
		uint32 meshCount = visualMeshes.size() + bspMeshes.size() + hullMeshes.size();
		int count = 0;
		while (visualMeshes.size())
		{
			if (ExportSettings::instance().exportMode() == ExportSettings::NORMAL)
			{

				visualMeshes.back()->save( pVisualSection, pExistingVisual, tempPrimFileName, true );

#if DEBUG_PRIMITIVE_FILE
				visualMeshes.back()->savePrimXml( primDebugXmlFileName );
#endif

				BoundingBox bb2 = visualMeshes.back()->bb();
				MDagPath meshDag = findDagPath( visualMeshes.back()->fullName() );

				MStatus stat;
				MFnTransform trans(meshDag.transform(), &stat );
				if (stat == MS::kSuccess)
				{
					Hierarchy* pNode = &hierarchy.find( trans.fullPathName().asChar() );
					if (&hierarchy != pNode)
					{
						matrix4<float> trans = hierarchy.transform(0, false).inverse() * pNode->transform( 0, false );
						bb2.transformBy( *(Matrix*)&trans );
					}
				}
				bb.addBounds( bb2 );
			}
			else
			{
				accMesh->dualUV( accMesh->dualUV() || visualMeshes.back()->dualUV() );
				accMesh->vertexColours( accMesh->vertexColours() || visualMeshes.back()->vertexColours() );

				bool worldSpaceAcc = ExportSettings::instance().exportMode() == ExportSettings::MESH_PARTICLES ? false : true;
				visualMeshes.back()->add( *accMesh, count++, worldSpaceAcc );
				bb.addBounds( visualMeshes.back()->bb() );
			}


			visualMeshes.pop_back();
		}

		// add bsps to the bb
		for (uint32 i = 0; i < bspMeshes.size(); i++)
		{
			bb.addBounds( bspMeshes[i]->bb() );
		}

		// add hulls to the bb
		for (uint32 i = 0; i < hullMeshes.size(); i++)
		{
			bb.addBounds( hullMeshes[i]->bb() );
		}

		// for mesh particles ensure there are 15 meshes
		if ( ExportSettings::instance().exportMode() == ExportSettings::MESH_PARTICLES )
		{
			int currentIndex = 0;
			while (count < 15 && !origVisualMeshes.empty())
			{
				// add mesh to acculated meshes
				origVisualMeshes[currentIndex++]->add( *accMesh, count, false );

				if (currentIndex == origVisualMeshes.size())
					currentIndex = 0;

				++count;
			}
		}

		bool saved=true;
		// save out accumulated mesh instead
		if ( ExportSettings::instance().exportMode() != ExportSettings::NORMAL )
		{
			saved=accMesh->save( pVisualSection, pExistingVisual, tempPrimFileName, false );

#if DEBUG_PRIMITIVE_FILE
			accMesh->savePrimXml( primDebugXmlFileName );
#endif
		}
		accMesh = NULL;

		if (meshCount==0 || !saved)
		{
			pVisualSection = NULL;
			::DeleteFile( bw_utf8tow( tempFileName ).c_str() );
			::DeleteFile( bw_utf8tow( tempPrimFileName ).c_str() );
			return false;
		}

		bb.setBounds(	Vector3(	bb.minBounds().x,
									bb.minBounds().y,
									bb.minBounds().z ) *
									ExportSettings::instance().unitScale(),
						Vector3(	bb.maxBounds().x,
									bb.maxBounds().y,
									bb.maxBounds().z ) *
									ExportSettings::instance().unitScale() );
		pVisualSection->writeVector3( "boundingBox/min", bb.minBounds() );
		pVisualSection->writeVector3( "boundingBox/max", bb.maxBounds() );

		// export hull and portals
		// First remove all existing boundary sections from the visual
		DataSectionPtr pBoundary = pVisualSection->openSection( "boundary" );
		while ( pBoundary )
		{
			pVisualSection->delChild( pBoundary );
			pBoundary = pVisualSection->openSection( "boundary" );
		}

		// export hull/boundaries for shells
		if ( shell )
		{
			// support custom hulls
			if ( hullMeshes.empty() == false )
			{
				exportHull( pVisualSection, hullMeshes );
				pVisualSection->writeBool( "customHull", true );
			}
			else
			{
				generateHull(pVisualSection, bb);
				pVisualSection->writeBool( "customHull", false );
			}

			// Now write any explicitly set portal nodes.
			// The following method finds boundaries for our portals
			// and writes them into the appropriate boundary sections.
			exportPortalsToBoundaries( pVisualSection, visualPortals );
		}

		if ( ExportSettings::instance().exportMode() != ExportSettings::NORMAL && !bspMeshes.empty() )
			pVisualSection->writeBool( "customBsp", true );
	}

	visualFile.save();

	pVisualSection = NULL;

	BWResource::instance().purge( tempResName, true );
	BWResource::instance().purge( tempPrimResName, true );

	Moo::VisualSplitter* pSplitter = new Moo::VisualSplitter( ExportSettings::instance().maxBones() );

	pSplitter->open( tempResName );
	if( pSplitter->split() )
	{
#if DEBUG_VISUAL_FILE
	// debug: keep copy of original file
	::CopyFile( bw_utf8tow( tempFileName ).c_str(), bw_utf8tow( (fileName + ".original") ).c_str(), false );
#endif
		pSplitter->save( tempFileName );
	}
	else
	{
		delete pSplitter;
		::DeleteFile( bw_utf8tow( tempFileName ).c_str() );
		::DeleteFile( bw_utf8tow( tempPrimFileName ).c_str() );
		::MessageBox( 0, L"Failed to split the meshes", L"Visual Exporter", MB_ICONERROR );
		return false;
	}

	delete pSplitter;

	BWResource::instance().purge( tempResName, true );
	BWResource::instance().purge( tempPrimResName, true );

	pVisualSection = BWResource::openSection( tempResName );

	if ( !ExportSettings::instance().disableVisualChecker() )
	{
		VisualChecker vc( tempFileName, false, ExportSettings::instance().snapVertices() );
		if( !vc.check( pVisualSection, tempPrimResName ) )
		{
			std::string errs = vc.errorText();
			::MessageBox( 0, bw_utf8tow( errs ).c_str(), L"Visual failed validity checking", MB_ICONERROR );

			printf( errs.c_str() );
			printf( "\n" );
			fflush( stdout );

			::DeleteFile( bw_utf8tow( tempFileName ).c_str() );
			::DeleteFile( bw_utf8tow( tempPrimFileName ).c_str() );

			return false;
		}
	}

	// Passed checking, rename to proper name
	::DeleteFile( bw_utf8tow( fileName ).c_str() );
	::DeleteFile( bw_utf8tow( primFileName ).c_str() );

	::MoveFile( bw_utf8tow( tempFileName ).c_str(), bw_utf8tow( fileName ).c_str() );
	::MoveFile( bw_utf8tow( tempPrimFileName ).c_str(), bw_utf8tow( primFileName ).c_str() );

	// Now process the BSP
	// Make sure there's no existing bsp file
	::DeleteFile( bw_utf8tow( visualBspFileName ).c_str() );

	//------------------------------------
	//Export the BSP
	//------------------------------------
	std::vector<std::string> bspMaterialIDs;
	// Export bsp tree (don't export bsps for MESH_PARTICLES).
	if ( ExportSettings::instance().exportMode() != ExportSettings::MESH_PARTICLES )
	{
		// Export bsp tree for all objects if there is a custom bsp tree defined.
		if ( !bspMeshes.empty() )
		{
			std::string bspFileName = BWResource::removeExtension( fileName ) + "_bsp.visual";
			std::string bspResName = BWResource::removeExtension( resName ) + "_bsp.visual";
			std::string bspPrimFileName = BWResource::removeExtension( fileName ) + "_bsp.primitives";
			std::string bspPrimResName = BWResource::removeExtension( resName ) + "_bsp.primitives";
			std::string bspBspFileName = BWResource::removeExtension( fileName ) + "_bsp.bsp";

			DataResource bspFile( bspFileName, RESOURCE_TYPE_XML );

			DataSectionPtr pBSPSection = bspFile.getRootSection();

			if ( pBSPSection )
			{
				pBSPSection->delChildren();

				// Use same material kind as the real visual, so we get the correct data in the bsp
				if (!materialKind.empty())
					pBSPSection->writeString( "materialKind", materialKind );

				// bsp has only one node
				Hierarchy h( NULL );
				exportTree( pBSPSection, h );

				BoundingBox bb = BoundingBox::s_insideOut_;
				VisualMeshPtr accMesh = new VisualMesh();

				while ( bspMeshes.size() )
				{
					bspMeshes.back()->add( *accMesh );

					bb.addBounds( bspMeshes.back()->bb() );

					bspMeshes.pop_back();
				}

				accMesh->save( pBSPSection, pExistingVisual, bspPrimFileName, false );
				accMesh = NULL;

				bb.setBounds(	Vector3(	bb.minBounds().x,
											bb.minBounds().z,
											bb.minBounds().y ),
								Vector3(	bb.maxBounds().x,
											bb.maxBounds().z,
											bb.maxBounds().y ) );
				pBSPSection->writeVector3( "boundingBox/min", bb.minBounds() * ExportSettings::instance().unitScale() );
				pBSPSection->writeVector3( "boundingBox/max", bb.maxBounds() * ExportSettings::instance().unitScale() );
			}

			bspFile.save();

			// Generate the bsp for the bsp visual
			generateBSP( bspResName, BWResource::removeExtension( bspResName ) + ".bsp", bspMaterialIDs );

			// Rename the .bsp file to the real visuals name
			::MoveFile( bw_utf8tow( bspBspFileName ).c_str(), bw_utf8tow( visualBspFileName ).c_str() );

			// Clean up the bsp visual
			::DeleteFile( bw_utf8tow( bspFileName ).c_str() );
			::DeleteFile( bw_utf8tow( bspPrimFileName ).c_str() );
		}
		// Only generate a bsp if the object is static,
		else if ( ExportSettings::instance().exportMode() != ExportSettings::NORMAL &&
			ExportSettings::instance().exportMode() != ExportSettings::NORMAL )
		{
			// Generate the bsp from the plain visual
			generateBSP( resName, visualBspResName, bspMaterialIDs );
		}

		// We may now have a .bsp file, move it into the .primitives file
		DataSectionPtr ds = BWResource::openSection( visualBspResName );
		BinaryPtr bp;

		if ( ds && (bp = ds->asBinary()) )
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
			// Clean up the .bsp file
			::DeleteFile( bw_utf8tow( visualBspFileName ).c_str() );
		}
	}

	return true;
}

// we overload this method to perform any
// file export operations needed
MStatus VisualFileTranslator::writer( const MFileObject& file, const MString& optionsString, FileAccessMode mode)
{
	// Purges the datasection cache after finishing each export
	DataSectionCachePurger dscp;

	// Initialise NodeCatalogue.
	NodeCatalogueHolder nch;

	ExportSettings& settings = ExportSettings::instance();

	// Get the output filename
	MString output_filename = bw_acptoutf8( file.fullName().asChar() ).c_str();
	if ( output_filename.rindex('.') > 1 )
		output_filename = output_filename.substring(0, output_filename.rindex('.')-1 );

	std::string resNameCamelCase;
	if (!validResource( ( output_filename + ".visual" ).asChar(), resNameCamelCase ))
	{
		invalidExportDlg( ( output_filename + ".visual" ).asChar() );
		return MS::kFailure;
	}
	
	VisualChecker vc( resNameCamelCase, false, settings.snapVertices() );
	settings.visualTypeIdentifier( vc.typeName() );

	// Choose a default export type based on the visual type
	if (vc.exportAs() == "normal")
	{
		settings.exportMode( ExportSettings::NORMAL );
	}
	else if (vc.exportAs() == "static")
	{
		settings.exportMode( ExportSettings::STATIC );
	}
	else if (vc.exportAs() == "static with nodes")
	{
		settings.exportMode( ExportSettings::STATIC_WITH_NODES );
	}

	VisualExporterDialog ved;
	if( ved.doModal( hInstance, GetForegroundWindow() ) == IDCANCEL )
		return MS::kSuccess;
	if( vc.typeName() == UNKNOWN_TYPE_NAME )
		settings.disableVisualChecker( true );

	settings.nodeFilter(ExportSettings::ALL);
	if( mode == kExportActiveAccessMode )
	{
		settings.nodeFilter(ExportSettings::SELECTED);

	}

	BlendShapes blendShapes;

	Mesh mesh;

	Skin skin;

	Portal portal;

	for( uint32 i = 0; i < mesh.count(); ++i )
	{
		// Disable the affect of BlendShapes
		BlendShapes::disable();

		bool bspNode = (toLower(mesh.meshes()[i].fullPathName().asChar()).find("_bsp") != std::string::npos);
		if( mesh.initialise( i, (settings.exportMode() == ExportSettings::NORMAL
			|| settings.exportMode() == ExportSettings::MESH_PARTICLES ) && !bspNode ) == false )
			continue;

		if( mesh.name().substr( mesh.name().find_last_of( "|" ) + 1 ).substr( 0, 3 ) == "HP_" )
			continue;

		// FIX very inefficient - but too slow?
		bool skipMesh = false;

		// loop through all portals here
		for( uint32 j = 0; skipMesh == false && j < portal.count(); ++j )
		{
			portal.initialise( j );

			// do we have a portal mesh
			if ( mesh.meshes()[i].fullPathName().asChar() == portal.name() )
			{
				VisualPortalPtr spVisualPortal = new VisualPortal;

				if( spVisualPortal )
				{
					if( spVisualPortal->init( portal ) )
						visualPortals.push_back( spVisualPortal );
				}

				skipMesh = true;
			}
		}

		// check for a hull mesh
		bool hullNode = (toLower(mesh.meshes()[i].fullPathName().asChar()).find("_hull") != std::string::npos);
		if ( hullNode && skipMesh == false )
		{
			// add the mesh to a hull node meshlist
			//~ printf( "Hull Mesh: %s\n", mesh.meshes()[i].fullPathName().asChar() );
			//~ fflush( stdout );
			VisualMeshPtr spVisualMesh = new VisualMesh;
			spVisualMesh->snapVertices(
				settings.exportMode() != ExportSettings::NORMAL &&
				ExportSettings::instance().snapVertices() );
			if( spVisualMesh )
			{
				if( spVisualMesh->init( mesh ) )
				{
					// add to hulls list
					hullMeshes.push_back( spVisualMesh );
				}
			}

			skipMesh = true;
		}

		// check for bsp node
		if ( bspNode && skipMesh == false )
		{
			// add the mesh to a hull node meshlist
			//~ printf( "Bsp Mesh: %s\n", mesh.meshes()[i].fullPathName().asChar() );
			//~ fflush( stdout );
			VisualMeshPtr spVisualMesh = new VisualMesh;
			spVisualMesh->snapVertices(
				settings.exportMode() != ExportSettings::NORMAL &&
				ExportSettings::instance().snapVertices() );
			if( spVisualMesh )
			{
				if( spVisualMesh->init( mesh ) )
				{
					// add to hulls list
					bspMeshes.push_back( spVisualMesh );
				}
			}

			skipMesh = true;
		}


		bool skinned = false;
		// loop through all skin objects, only for NORMAL models
		for( uint32 j = 0; settings.exportMode() == ExportSettings::NORMAL && skipMesh == false && j < skin.count(); ++j )
		{
			skin.initialise( j );

			for( uint32 k = 0; skipMesh == false && k < skin.meshes().length(); ++k )
			{
				if( mesh.meshes()[i] == skin.meshes()[k] )
				{
					mesh.initialise(i, false);
					//~ printf( "Skinned: %s\n", mesh.meshes()[i].fullPathName().asChar() );
					//~ fflush( stdout );
					VisualEnvelopePtr spVisualEnvelope = new VisualEnvelope;

					if( spVisualEnvelope )
					{
						if( spVisualEnvelope->init( skin, mesh ) )
							visualMeshes.push_back( spVisualEnvelope );
					}

					skipMesh = true;
					skinned = true;
				}
			}
		}

		if ( blendShapes.isBlendShapeTarget( mesh.meshes()[i].node() ) == false )
		{
			if ( skipMesh == true )
			{
				if  ( skinned == true )
				{
					//use the last created mesh
					VisualMeshPtr spVisualMesh = visualMeshes.back();
					if( spVisualMesh )
					{
						if( blendShapes.hasBlendShape( mesh.meshes()[i].node() ) )
							spVisualMesh->object = mesh.meshes()[i].node();
					}
				}
			}
			else
			{
				//~ printf( "Mesh: %s\n", mesh.meshes()[i].fullPathName().asChar() );
				//~ fflush( stdout );
				VisualMeshPtr spVisualMesh = new VisualMesh;
				spVisualMesh->snapVertices(
					settings.exportMode() != ExportSettings::NORMAL &&
					ExportSettings::instance().snapVertices() );

				if( blendShapes.hasBlendShape( mesh.meshes()[i].node() ) )
					spVisualMesh->object = mesh.meshes()[i].node();

				if( spVisualMesh )
				{
					if( spVisualMesh->init( mesh ) )
					{
						visualMeshes.push_back( spVisualMesh );
					}
				}
			}
		}
	}

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
			::MessageBox( 0, bw_utf8tow( msg ).c_str(), L"Visual exporter - Bone warning!", MB_ICONWARNING );
		}
	}

	// Re-enable the affect of BlendShapes
	BlendShapes::enable();

	bool hasPortal = !visualPortals.empty();
	if( !hasPortal )
	{
		for( std::vector<VisualMeshPtr>::iterator iter = hullMeshes.begin();
			iter != hullMeshes.end(); ++iter )
			if( ((HullMesh*)iter->getObject())->hasPortal() )
			{
				hasPortal = true;
				break;
			}
	}

	std::string resName = BWResolver::dissolveFilename( output_filename.asChar() );
	if( !isShell( resName ) && hasPortal )
	{
		if( MessageBox( NULL,
			L"You are exporting an object with portals into a non shell directory.\n"
			L"If you continue this object will not be recognised as a shell",
			L"Visual Exporter", MB_OKCANCEL | MB_ICONEXCLAMATION ) == IDCANCEL )
		{
			visualMeshes.clear();
			hullMeshes.clear();
			bspMeshes.clear();
			visualPortals.clear();

			return MS::kFailure;
		}
	}
	if( !settings.exportAnim() )
	{
		if( !exportMesh( output_filename.asChar() ) )
		{
			visualMeshes.clear();
			hullMeshes.clear();
			bspMeshes.clear();
			visualPortals.clear();
			return MS::kFailure;
		}
	}

	// Xiaoming Shi : save model file, for bug 4993{
	if( ExportSettings::instance().exportMode() != ExportSettings::MESH_PARTICLES &&
		//RA: changed to save the model file even if it exists...
		//! BWResource::fileExists( BWResource::removeExtension( output_filename.asChar() ) + ".model") &&
		!settings.exportAnim() )
	{
		std::string resName = BWResolver::dissolveFilename(  output_filename.asChar()  );
		DataResource modelFile( BWResource::removeExtension(  output_filename.asChar()  )
			+ ".model", RESOURCE_TYPE_XML );
		DataSectionPtr pModelSection = modelFile.getRootSection();
		if (pModelSection)
		{
			MetaData::updateCreationInfo( pModelSection );
			pModelSection->deleteSections( "nodefullVisual" );
			pModelSection->deleteSections( "nodelessVisual" );
			std::string filename = BWResource::removeExtension( resName );
			if (ExportSettings::instance().exportMode() == ExportSettings::NORMAL ||
				ExportSettings::instance().exportMode() == ExportSettings::STATIC_WITH_NODES)
				pModelSection->writeString( "nodefullVisual", filename );
			else if (ExportSettings::instance().exportMode() == ExportSettings::STATIC)
				pModelSection->writeString( "nodelessVisual", filename );
		}
		modelFile.save();
	}
	// Xiaoming Shi : save model file, for bug 4993}

	if (settings.exportAnim())
	{
		bool errors = false;

		// Check if the filename is in the right res path.
		std::string name = output_filename.asChar();
		std::string name2 = BWResolver::dissolveFilename( name );
		if ((name2.length() == name.length()) || (name2.find_last_of("/\\") == std::string::npos))
		{
			errors = true;
			std::string errorString;
			errorString = std::string( "The exported file \"" ) + name + std::string( "\" is not in a valid game path.\n" );
			errorString += "Need to export to a *subfolder* of:";
			uint32 pathIndex = 0;
			while (BWResource::getPath(pathIndex) != std::string(""))
			{
				errorString += std::string("\n") + BWResource::getPath(pathIndex++);
			}
			MessageBox( NULL, bw_utf8tow( errorString ).c_str(), L"Animation Exporter Error", MB_OK | MB_ICONWARNING );
			visualMeshes.clear();
			hullMeshes.clear();
			bspMeshes.clear();
			visualPortals.clear();
			return MStatus::kFailure;
		}

		std::string animName = name;
		int cropIndex = 0;
		if ((cropIndex = animName.find_last_of( "\\/" )) > 0)
		{
			cropIndex++;
			animName = animName.substr( cropIndex, animName.length() - cropIndex );
		}

		{
			// to make it save *.animations instead of *.visual.animations
			// when I give full file name in open file dialog
			// or overwrite existing file
			FILE* file2 = _wfopen( bw_utf8tow( ( BWResource::removeExtension( name ) + ".animation") ).c_str(), L"wb" );

#if DEBUG_ANIMATION_FILE
			// Delete the previous animation debug xml file if it exists
			std::string animDebugXmlFileName = BWResource::removeExtension( name ) + ".animation.xml";
			::DeleteFile( animDebugXmlFileName.c_str() );
#endif

			if (file2)
			{
				Hierarchy hierarchy( NULL );
				Skin skin;

				BinaryFile animation( file2 );

				int nChannels = 0;

				skin.trim( mesh.meshes() );
				bool skeletalAnimation = skin.count() > 0 && skin.initialise( 0 );

				// Need to get meshes hierarchies first, then skeletal
				Mesh mesh;
				bool bMeshSkeletalAnimation = false;
				if (mesh.meshes().length() > 0)
				{
					bMeshSkeletalAnimation = true;
					hierarchy.getMeshes( mesh );
				}
				if (skeletalAnimation)
				{
					hierarchy.getSkeleton( skin );
					for (uint32 i = 1; i < skin.count(); ++i)
					{
						if (skin.initialise( i ))
						{
							hierarchy.getSkeleton( skin );
						}
					}
				}
				if (bMeshSkeletalAnimation || skeletalAnimation)
				{
					nChannels += hierarchy.count();
					skeletalAnimation = true;
				}

				if (ExportSettings::instance().referenceNode())
				{
					wchar_t fileName[1025] = { 0 };
					OPENFILENAME openFileName = {
						sizeof( OPENFILENAME ),
						GetForegroundWindow(),
						0,
						L"Visual Files\0*.visual\0All Files\0*.*\0\0",
						NULL,
						0,
						0,
						fileName,
						1024,
						NULL,
						0,
						NULL,
						NULL,
						OFN_LONGNAMES
					};
					if (GetOpenFileName( &openFileName ))
					{
						this->loadReferenceNodes( bw_wtoutf8( fileName ) );
						StringHashMap< std::string >::iterator it = nodeParents_.begin();
						StringHashMap< std::string >::iterator end = nodeParents_.end();
						while (it != end)
						{
							std::string parent = it->second;
							std::string child = it->first;

							Hierarchy* p = hierarchy.recursiveFind( parent );
							Hierarchy* c = hierarchy.recursiveFind( child );
							if ((p && c) && (p != c->getParent()))
							{
								// 1. check if in the same branch
								{
									Hierarchy* parent = p;
									while (parent && parent->getParent() != c)
									{
										parent = parent->getParent();
									}
									if (parent)
									{
										std::string name = c->removeNode( parent );
										if (name.size())
										{
											c->getParent()->addNode( name, *parent );
										}
									}
								}

								// 2. move
								Hierarchy* op = c->getParent();
								if (op)
								{
									op->removeNode( child );
								}
								p->addNode( child, *c );
							}
							
							++it;
						}
					}
				}

				BlendShapes blendShapes;

				for( uint32 i = 0; i < blendShapes.count(); ++i )
				{
					blendShapes.initialise( i );

					// TODO : fix, only doing base 0 for now
					if (blendShapes.countTargets() > 0)
					{
						nChannels += blendShapes.numTargets( 0 );
					}
				}

				if (!ExportSettings::instance().sceneRootAdded())
				{
					++nChannels; // Plus one for the "Scene Root" (see below)
				}

				animation << float( hierarchy.numberOfFrames() - 1 ) << animName << animName;
				animation << nChannels;

#if DEBUG_ANIMATION_FILE
				exportAnimationXmlHeader( animDebugXmlFileName, animName, float( hierarchy.numberOfFrames() - 1 ), nChannels );
#endif

				if( skeletalAnimation )
				{
					{
						std::set<std::string> names;
						names.insert( "Scene Root" );
						if( !checkHierarchy( hierarchy, names ) )
						{
							::MessageBox( 0, L"There are 2 or more bones with same names in the hierarchy!", L"Bone hierarchy validity checking", MB_ICONERROR );
							visualMeshes.clear();
							hullMeshes.clear();
							bspMeshes.clear();
							visualPortals.clear();
							return MS::kFailure;
						}
					}

					// Special requirement for the root node.  The node must be named for
					// it to be exported.
					if (!ExportSettings::instance().sceneRootAdded())
					{
						hierarchy.name("Scene Root");
						hierarchy.customName("Scene Root");
					}
					exportAnimation( animation, hierarchy, ExportSettings::instance().stripRefPrefix() );

#if DEBUG_ANIMATION_FILE
					DataSectionPtr spFileRoot = BWResource::openSection( animDebugXmlFileName );
					if ( spFileRoot )
					{
						DataSectionPtr spFile = spFileRoot->openSection( animName );
						
						if ( spFile )
							exportAnimationXml( spFile, hierarchy, ExportSettings::instance().stripRefPrefix() );
					}
					spFileRoot->save( animDebugXmlFileName );
#endif
				}
				for (uint i = 0; i < visualMeshes.size(); i++)
				{
					visualMeshes[i]->exportMorphAnims( animation, ExportSettings::instance().stripRefPrefix() );

#if DEBUG_ANIMATION_FILE
					visualMeshes[i]->exportMorphAnimsXml( animDebugXmlFileName, animName, ExportSettings::instance().stripRefPrefix() );
#endif
				}

				fclose(file2);
			}
		}
	}

	// clean up
	visualMeshes.clear();
	hullMeshes.clear();
	bspMeshes.clear();
	visualPortals.clear();

	// writing of file was sucessful
	return MS::kSuccess;
}


// returns true if this class can export files
bool VisualFileTranslator::haveWriteMethod() const
{
	return true;
}


// returns the default extension of the file supported
// by this FileTranslator.
MString VisualFileTranslator::defaultExtension() const
{
	return "BigWorld";
}

// add a filter for open file dialog
MString VisualFileTranslator::filter() const
{
	// Maya 6.5 doesn't support extension name that is longer than 6
	// like "visual", otherwise it crashes randomly, need further investigation
	return "*.*";
}

// Used by Maya to create a new instance of our
// custom file translator
void* VisualFileTranslator::creator()
{
	return new VisualFileTranslator;
}

void VisualFileTranslator::loadReferenceNodes( const std::string & visualFile )
{
	std::string filename = BWResolver::dissolveFilename( visualFile );

	DataSectionPtr pVisualSection = BWResource::openSection( filename );

	if (pVisualSection)
	{
		readReferenceHierarchy( pVisualSection->openSection( "node" ) );
	}
}

void VisualFileTranslator::readReferenceHierarchy( DataSectionPtr hierarchy )
{
	std::string id = hierarchy->readString( "identifier" );
	std::vector< DataSectionPtr> nodeSections;
	hierarchy->openSections( "node", nodeSections );

	for (uint i = 0; i < nodeSections.size(); i++)
	{
		DataSectionPtr p = nodeSections[i];
		nodeParents_[ p->readString( "identifier" ) ] = id;
		readReferenceHierarchy( p );
	}
}
