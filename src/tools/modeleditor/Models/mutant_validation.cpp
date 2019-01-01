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

#include "resmgr/string_provider.hpp"
#include "resmgr/primitive_file.hpp"
#include "moo/primitive_file_structs.hpp"
#include "common/file_dialog.hpp"
DECLARE_DEBUG_COMPONENT( 0 )
#include "me_error_macros.hpp"

#include "mutant.hpp"


/**
 * This is a helper function which strip two strings down to their differing roots, e.g.
 *   strA = "sets/test/images"  strB = "sets/new_test/images"
 * becomes
 *   strA = "sets/test"         strB = "sets/new_test"

 *  @param	strA			The first string
 *	@param	strB			The second string
 **/
void Mutant::clipToDiffRoot( std::string& strA, std::string& strB )
{
	BW_GUARD;

	std::string::size_type posA = strA.find_last_of("/");
	std::string::size_type posB = strB.find_last_of("/");

	while ((posA != std::string::npos) &&
		(posB != std::string::npos) &&
		( strA.substr( posA ) == strB.substr( posB )))
	{
		strA = strA.substr( 0, posA );
		strB = strB.substr( 0, posB );

		posA = strA.find_last_of("/");
		posB = strB.find_last_of("/");
	}
}

/**
 * This method checks whether a texture could be an animated texture and fixes the animtex if needed.
 *
 *  @param	texRoot			The texture to test
 *	@param	oldRoot			The old root to replace
 *	@param	newRoot			The new root to replace with
 */
void Mutant::fixTexAnim( const std::string& texRoot, const std::string& oldRoot, const std::string& newRoot )
{
	BW_GUARD;

	std::string texAnimName = texRoot.substr( 0, texRoot.find_last_of(".") ) + ".texanim";

	DataSectionPtr texAnimFile =  BWResource::openSection( texAnimName );
	if (texAnimFile != NULL)
	{
		bool changed = false;

		std::vector< DataSectionPtr > textures;
		texAnimFile->openSections( "texture", textures );
		for (unsigned i=0; i<textures.size(); i++)
		{
			std::string textureName = textures[i]->asString("");
			if (textureName != "")
			{
				if (BWResource::fileExists( textureName )) continue;
				std::string::size_type pos = textureName.find( oldRoot );
				if (pos != std::string::npos)
				{
					textureName = textureName.replace( pos, oldRoot.length(), newRoot );
					textures[i]->setString( textureName );
					changed = true;
				}
			}
		}
		if (changed)
		{
			texAnimFile->save();
		}
	}
}

/**
 * This method fixes the texture paths of a <material> data section if needed.
 *
 *  @param	mat				The <material> data section to fix
 *	@param	oldRoot			The old root to replace
 *	@param	newRoot			The new root to replace with
 *
 *	@return Whether any texture paths were changed.
 */
bool Mutant::fixTextures( DataSectionPtr mat, const std::string& oldRoot, const std::string& newRoot )
{
	BW_GUARD;

	bool changed = false;
	
	std::string newMfm = mat->readString("mfm","");
	if (newMfm != "")
	{
		if (!BWResource::fileExists( newMfm ))
		{
			std::string::size_type pos = newMfm.find( oldRoot );
			if (pos != std::string::npos)
			{
				newMfm = newMfm.replace( pos, oldRoot.length(), newRoot );
				mat->writeString( "mfm", newMfm );
				changed = true;
			}
		}
	}
	
	instantiateMFM( mat );
						
	std::vector< DataSectionPtr > effects;
	mat->openSections( "fx", effects );
	for (unsigned i=0; i<effects.size(); i++)
	{
		std::string newFx = effects[i]->asString("");
		if (newFx != "")
		{
			if (BWResource::fileExists( newFx )) continue;
			std::string::size_type pos = newFx.find( oldRoot );
			if (pos != std::string::npos)
			{
				newFx = newFx.replace( pos, oldRoot.length(), newRoot );
				effects[i]->setString( newFx );
				changed = true;
			}
		}
	}

	std::vector< DataSectionPtr > props;
	mat->openSections( "property", props );
	for (unsigned i=0; i<props.size(); i++)
	{
		std::string newTexture = props[i]->readString( "Texture", "" );
		if (newTexture != "")
		{
			if (BWResource::fileExists( newTexture )) continue;
			std::string::size_type pos = newTexture.find( oldRoot );
			if (pos != std::string::npos)
			{
				newTexture = newTexture.replace( pos, oldRoot.length(), newRoot );
				props[i]->writeString( "Texture", newTexture );
				fixTexAnim( newTexture, oldRoot, newRoot );
				changed = true;
			}
		}
		else // Make sure to handle texture feeds as well
		{
			newTexture = props[i]->readString( "TextureFeed/default", "" );
			if (newTexture != "")
			{
				if (BWResource::fileExists( newTexture )) continue;
				std::string::size_type pos = newTexture.find( oldRoot );
				if (pos != std::string::npos)
				{
					newTexture = newTexture.replace( pos, oldRoot.length(), newRoot );
					props[i]->writeString( "TextureFeed/default", newTexture );
					fixTexAnim( newTexture, oldRoot, newRoot );
					changed = true;
				}
			}
		}
	}

	return changed;
}

/*static*/ std::vector< std::string > Mutant::s_missingFiles_;
/**
 * Static method to clear the list of files which couldn't be found
 */
/*static*/ void Mutant::clearFilesMissingList()
{
	BW_GUARD;

	s_missingFiles_.clear();
}

/**
 * This method tries to locate the file requested.
 *
 *  @param	fileName		Path of the missing file.
 *  @param	modelName		Path of the model trying to be opened.
 *  @param	ext				The extension of the file type to find (e.g. "model", "visual", etc)
 *  @param	what			What operation is being exectuted (e.g. "load", "add", etc)
 *  @param	criticalMsg		A string for whether the model can load without locating the missing file

 *	@param	oldRoot			The old root to replace
 *	@param	newRoot			The new root to replace with
 *
 *	@return Whether the file was located.
 */
bool Mutant::locateFile( std::string& fileName, std::string modelName, const std::string& ext, const std::string& what, const std::string& criticalMsg /* = "" */ )
{
	BW_GUARD;

	std::vector< std::string >::iterator entry = std::find( s_missingFiles_.begin(), s_missingFiles_.end(), fileName );

	if (entry != s_missingFiles_.end()) return false;
	
	s_missingFiles_.push_back( fileName );
		
	int response = ::MessageBox( AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
		Localise(L"MODELEDITOR/MODELS/MUTANT_VALIDATION/CANT_LOCATE_Q", ext, fileName, ext, criticalMsg ), 
		Localise(L"MODELEDITOR/MODELS/MUTANT_VALIDATION/UNABLE_LOCATE", ext), MB_ICONERROR | MB_YESNO );
	
	if (response == IDYES)
	{
		std::string oldFileName = fileName.substr( fileName.find_last_of( "/" ) + 1 ) + "." + ext;

		std::string fileFilter = ext +" (*."+ext+")|*."+ext+"||";
		BWFileDialog fileDlg ( TRUE, L"", bw_utf8tow( oldFileName ).c_str(), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, bw_utf8tow( fileFilter ).c_str() );

		modelName = BWResource::resolveFilename( modelName );
		std::replace( modelName.begin(), modelName.end(), '/', '\\' );
		std::wstring wmodelName = bw_utf8tow( modelName );
		fileDlg.m_ofn.lpstrInitialDir = wmodelName.c_str();

		if ( fileDlg.DoModal() == IDOK )
		{
			fileName = BWResource::dissolveFilename( bw_wtoutf8( fileDlg.GetPathName().GetString() ));
			fileName = fileName.substr( 0, fileName.rfind(".") );
			return true;
		}
	}
	
	return false;
}

/**
 * This method tests to see if the given file is read only
 *
 *  @param	file			Path of the file to test for read only.
 *
 *	@return Whether the file is read only.
 */
bool Mutant::isFileReadOnly( const std::string& file )
{
	BW_GUARD;

	std::wstring fullPath = BWResource::resolveFilenameW( file );
	DWORD att = GetFileAttributes( fullPath.c_str() );
	if (att & FILE_ATTRIBUTE_READONLY)
	{
		::MessageBox( AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
			Localise(L"MODELEDITOR/MODELS/MUTANT_VALIDATION/READ_ONLY", file ), 
			Localise(L"MODELEDITOR/MODELS/MUTANT_VALIDATION/READ_ONLY_TITLE" ),
			MB_ICONEXCLAMATION | MB_OK );
		return true;
	}
	return false;
}

/**
 * This method tests to see if any of the files associated with the model are read only
 *
 *  @param	modelName		Path of the model to test for read only.
 *	@param	visualName		The path of the visual file to test for read only
 *	@param	primitivesName	The path of the primitives file to test for read only
 *
 *	@return Whether any of the files associated with the model are read only.
 */
bool Mutant::testReadOnly( const std::string& modelName, const std::string& visualName, const std::string& primitivesName )
{
	BW_GUARD;

	// Test the main files
	if ( isFileReadOnly( modelName ) )
		return true;
		
	if ( isFileReadOnly( visualName ) )
		return true;
		
	if ( isFileReadOnly( primitivesName ) )
		return true;
		
	// Test the animation files
	DataSectionPtr model = BWResource::openSection( modelName );
	std::vector< DataSectionPtr > pAnimations;
	model->openSections( "animation", pAnimations );
	std::vector< DataSectionPtr >::iterator animIt = pAnimations.begin();
	std::vector< DataSectionPtr >::iterator animEnd = pAnimations.end();
	while (animIt != animEnd)
	{
		DataSectionPtr pData = *animIt++;
		std::string animFile = pData->readString( "nodes", "" );
		if ( !animFile.empty() )
		{
			animFile += ".animation";
			if (BWResource::fileExists( animFile ))
			{
				if (isFileReadOnly( animFile ))
				{
					return true;
				}
			}
		}
	}

	return false;
}

/**
 * This method tests the validity of the model requested.
 *
 *  @param	name			Path of the model to test for validity.
 *  @param	what			What operation is being exectuted (e.g. "load", "add", etc)

 *  @param	model			The datasection of the model to return
 *  @param	visual			The datasection of the visual to return
 *	@param	visualName		The path of the visual file to return
 *	@param	primitivesName	The path of the primitives file to return
 *
 *	@return Whether the file was valid.
 */
bool Mutant::ensureModelValid( const std::string& name, const std::string& what,
	DataSectionPtr* model /* = NULL */, DataSectionPtr* visual /* = NULL */,
	std::string* visualName /* = NULL */, std::string* primitivesName /* = NULL */,
	bool* readOnly /* = NULL */)
{
	BW_GUARD;

	//This will get the data section for the visual file,
	// this is used for the node heirarchy
	DataSectionPtr testModel = BWResource::openSection( name, false );
	DataSectionPtr testVisual = NULL;
	std::string testRoot = "";

	if (testModel)
	{
		testRoot = testModel->readString("nodefullVisual","");
		std::string oldRoot;
		std::string newRoot;

		if (testRoot != "")
		{
			testVisual = BWResource::openSection( testRoot + ".visual", false );

			if (!testVisual)
			{
				oldRoot = testRoot;

				if (!locateFile( testRoot, name, "visual", what, LocaliseUTF8(L"MODELEDITOR/MODELS/MUTANT_VALIDATION/UNABLE_LOAD_OTHERWISE") ))
					return false;

				newRoot = testRoot;
				testVisual = BWResource::openSection( testRoot + ".visual", false );
				testModel->writeString( "nodefullVisual", testRoot );

				newRoot = newRoot.substr( 0, newRoot.find_last_of( "/" ));
				oldRoot = oldRoot.substr( 0, oldRoot.find_last_of( "/" ));
				clipToDiffRoot( newRoot, oldRoot );

				//Fix all the animation references
				std::vector< DataSectionPtr > anims;
				testModel->openSections( "animation", anims );

				for (unsigned i=0; i<anims.size(); i++)
				{
					std::string animRoot = anims[i]->readString( "nodes", "" );
					if (BWResource::fileExists( animRoot + ".animation" )) continue;
					std::string::size_type pos = animRoot.find( oldRoot );
					if (pos != std::string::npos)
					{
						animRoot = animRoot.replace( pos, oldRoot.length(), newRoot );
						anims[i]->writeString( "nodes", animRoot );
					}
				}

				//Fix all the tint references
				std::vector< DataSectionPtr > dyes;
				testModel->openSections( "dye", dyes );
				for (unsigned i=0; i<dyes.size(); i++)
				{
					std::vector< DataSectionPtr > tints;
					dyes[i]->openSections( "tint", tints );
					for (unsigned j=0; j<tints.size(); j++)
					{
						fixTextures( tints[j]->openSection("material"), oldRoot, newRoot );
					}
				}

				testModel->save();
			}
		}
		else
		{
			testRoot = testModel->readString("nodelessVisual","");

			if (testRoot != "")
			{
				testVisual = BWResource::openSection( testRoot + ".visual", false );

				if (!testVisual)
				{
					oldRoot = testRoot;

					if (!locateFile( testRoot, name, "visual", what, LocaliseUTF8(L"MODELEDITOR/MODELS/MUTANT_VALIDATION/UNABLE_LOAD_OTHERWISE") ))
						return false;
					
					newRoot = testRoot;
					testVisual = BWResource::openSection( testRoot + ".visual", false );
					testModel->writeString( "nodelessVisual", testRoot );
					
					newRoot = newRoot.substr( 0, newRoot.find_last_of( "/" ));
					oldRoot = oldRoot.substr( 0, oldRoot.find_last_of( "/" ));
					clipToDiffRoot( newRoot, oldRoot );

					testModel->save();
				}
			}
			else
			{
				testRoot = testModel->readString("billboardVisual","");

				if (testRoot != "")
				{
					testVisual = BWResource::openSection( testRoot + ".visual", false );

					if (!testVisual)
					{
						oldRoot = testRoot;

						if (!locateFile( testRoot, name, "visual", what, LocaliseUTF8(L"MODELEDITOR/MODELS/MUTANT_VALIDATION/UNABLE_LOAD_OTHERWISE") ))
							return false;

						newRoot = testRoot;
						testVisual = BWResource::openSection( testRoot + ".visual", false );
						testModel->writeString( "billboardVisual", testRoot );

						newRoot = newRoot.substr( 0, newRoot.find_last_of( "/" ));
						oldRoot = oldRoot.substr( 0, oldRoot.find_last_of( "/" ));
						clipToDiffRoot( newRoot, oldRoot );

						testModel->save();
					}
				}
				else
				{
					ERROR_MSG( "Cannot determine the visual type of the model\"%s\".\n"
						"Unable to %s model.", name.c_str(), what.c_str() );
					return false;
				}
			}
		}

		// Update any references in the visual file if needed
		if (newRoot != oldRoot)
		{
			bool visualChange = false;

			std::vector< DataSectionPtr > renderSets;
			testVisual->openSections( "renderSet", renderSets );
			for (unsigned i=0; i<renderSets.size(); i++)
			{
				std::vector< DataSectionPtr > geometries;
				renderSets[i]->openSections( "geometry", geometries );
				for (unsigned j=0; j<geometries.size(); j++)
				{
					std::string::size_type pos;
					std::string newVerts = geometries[j]->readString("vertices","");
					pos = newVerts.find( oldRoot );
					if (pos != std::string::npos)
					{
						newVerts = newVerts.replace( pos, oldRoot.length(), newRoot );
						geometries[j]->writeString( "vertices", newVerts );
						visualChange = true;
					}

					std::string newStream = geometries[j]->readString("stream","");
					pos = newStream.find( oldRoot );
					if (pos != std::string::npos)
					{
						newStream = newStream.replace( pos, oldRoot.length(), newRoot );
						geometries[j]->writeString( "stream", newStream );
						visualChange = true;
					}

					std::string newPrims = geometries[j]->readString("primitive","");
					pos = newPrims.find( oldRoot );
					if (pos != std::string::npos)
					{
						newPrims = newPrims.replace( pos, oldRoot.length(), newRoot );
						geometries[j]->writeString( "primitive", newPrims );
						visualChange = true;
					}

					std::vector< DataSectionPtr > primGroups;
					geometries[j]->openSections( "primitiveGroup", primGroups );
					for (unsigned k=0; k<primGroups.size(); k++)
					{
						std::vector< DataSectionPtr > materials;
						primGroups[k]->openSections( "material", materials );
						for (unsigned l=0; l<materials.size(); l++)
						{
							visualChange = fixTextures( materials[l], oldRoot, newRoot ) || visualChange;
						}
					}
				}
			}
		
			if (visualChange)
			{
				testVisual->save();
			}
		}

		if ( ! BWResource::openSection( testRoot + ".primitives" ) )
        {
			ERROR_MSG( "Unable to find the primitives file for \"%s\".\n\n"
				"Make sure that \"%s.primitives\" exists and is in the same directory as the *.visual file.\n\n"
				"Unable to %s model.\n", name.c_str(), testRoot.c_str(), what.c_str() );

			return false;
        }

		// Make sure the parent model exists and can be found
		std::string oldParentRoot = testModel->readString( "parent", "" );
		std::string parentRoot = oldParentRoot;
		if (oldParentRoot != "")
		{
			DataSectionPtr testParentModel = BWResource::openSection( oldParentRoot + ".model", false );
			if (testParentModel == NULL)
			{
				parentRoot = oldParentRoot;
				if (locateFile( parentRoot, name, "model", what ))
				{
					testModel->writeString( "parent", parentRoot );
					testModel->save();
				}
			}

			if (!ensureModelValid( parentRoot + ".model", what, NULL, NULL, NULL, NULL, readOnly )) return false;
		}
	}
	else
	{
		// this means that no model file could be opened.
		return false;
	}
	
	if ( isFormatDepreciated( testVisual, testRoot+".primitives" ) )
	{
		ERROR_MSG( "The model: \"%s\" is using a depreciated vertex format.\n"
				   "Please re-export this model and try again.\n", name.c_str() );
		return false;
	}
	// Update any parameters requested
	
	if (model)
		*model = testModel;

	if (visual)
		*visual = testVisual;

	if (visualName)
		*visualName = testRoot + ".visual";

	if (primitivesName)
		*primitivesName = testRoot + ".primitives";

	if (readOnly)
		*readOnly |= testReadOnly( name, testRoot + ".visual", testRoot + ".primitives" );
	
	return true;
}

/**
 *	This method checks to see whether the model being load is using a depreciated vertex format.
 *
 *	@param visual A DataSectionPtr to the visual of this model.
 *	@param primitivesName The name of the primitives file.
 *
 *	@return Returns true if the vertex format used by the model is depreciated, false otherwise.
 */
bool Mutant::isFormatDepreciated( DataSectionPtr visual, const std::string& primitivesName )
{
	BW_GUARD;

	DataSectionPtr primFile = PrimitiveFile::get( primitivesName );
	if (!primFile)
	{
		primFile = new BinSection( 
			primitivesName, new BinaryBlock( 0, 0, "BinaryBlock/FormatTest" ) );
	}

	// iterate through our rendersets
	std::vector< DataSectionPtr >	pRenderSets;
	visual->openSections( "renderSet", pRenderSets );
	std::vector< DataSectionPtr >::iterator renderSetsIt = pRenderSets.begin();
	std::vector< DataSectionPtr >::iterator renderSetsEnd = pRenderSets.end();
	while (renderSetsIt != renderSetsEnd)
	{
		DataSectionPtr renderSet = *renderSetsIt++;

		// iterate through our geometries
		std::vector< DataSectionPtr >	pGeometries;
		renderSet->openSections( "geometry", pGeometries );
		std::vector< DataSectionPtr >::iterator geometriesIt = pGeometries.begin();
		std::vector< DataSectionPtr >::iterator geometriesEnd = pGeometries.end();
		while (geometriesIt != geometriesEnd)
		{
			DataSectionPtr geometries = *geometriesIt++;

			std::string vertGroup = geometries->readString( "vertices", "" );
			if ( vertGroup.find_first_of( '/' ) < vertGroup.size() )
			{
				std::string fileName, partName;
				splitOldPrimitiveName( vertGroup, fileName, partName );
				vertGroup = partName;
			}
			// get the vertices information
			BinaryPtr pVertices = primFile->readBinary( vertGroup );
			if (pVertices)
			{
				const Moo::VertexHeader * vh = 
					reinterpret_cast< const Moo::VertexHeader* >( pVertices->data() );
				std::string format = std::string( vh->vertexFormat_ );

				if ((format == "xyznuvi") || (format == "xyznuvitb"))
				{
					return true;
				}
			}
		}
	}
	return false;
}
