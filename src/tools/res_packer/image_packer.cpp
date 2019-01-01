/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "config.hpp"
#include "packers.hpp"
#include "packer_helper.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/file_system.hpp"
#include "resmgr/multi_file_system.hpp"
#include "image_packer.hpp"
#ifndef MF_SERVER
#include "moo/texture_manager.hpp"
#endif // MF_SERVER

#include "xml_packer.hpp"

IMPLEMENT_PACKER( ImagePacker )

int ImagePacker_token = 0;


bool ImagePacker::prepare( const std::string& src, const std::string& dst )
{
	// TODO: This is somewhat of a temporary workaround so that we can have
	// "encrypted" dds files for the 1.8.0 evaluation release. If running with
	// the --encrypt command line, we want dds files to be processed by
	// XmlPacker.
	if (XmlPacker::shouldEncrypt())
		return false;

	std::string ext = BWResource::getExtension( src );
	if (bw_stricmp( ext.c_str(), "bmp" ) == 0 ||
		bw_stricmp( ext.c_str(), "tga" ) == 0 ||
		bw_stricmp( ext.c_str(), "png" ) == 0 ||
		bw_stricmp( ext.c_str(), "jpg" ) == 0)
		type_ = IMAGE;
	else if (bw_stricmp( ext.c_str(), "dds" ) == 0)
		type_ = DDS;
	else
		return false;

	src_ = src;
	dst_ = dst;

	return true;
}

bool ImagePacker::print()
{
	if ( src_.empty() )
	{
		printf( "Error: ImagePacker Not initialised properly\n" );
		return false;
	}

	if ( type_ == DDS )
		printf( "DDSFile: %s\n", src_.c_str() );
	else
		printf( "ImageFile: %s\n", src_.c_str() );
	return true;
}

bool ImagePacker::pack()
{
#ifndef MF_SERVER
	if ( src_.empty() || dst_.empty() )
	{
		printf( "Error: ImagePacker Not initialised properly\n" );
		return false;
	}

	if ( type_ == DDS )
	{
		// if it's a DDS image file, find out if it has a matching source image
		std::string baseName = BWResource::removeExtension( src_ );
		if ( !PackerHelper::fileExists( baseName + ".bmp" ) &&
			 !PackerHelper::fileExists( baseName + ".tga" ) &&
			 !PackerHelper::fileExists( baseName + ".png" ) &&
			 !PackerHelper::fileExists( baseName + ".jpg" ) )
		{
			bool isFont = false;

#ifndef PACK_FONT_DDS
			// Make sure that it is not a font DDS
			std::string directory = 
				BWResource::getFilePath( BWResolver::dissolveFilename( baseName ) );
			std::string fontName = BWResource::getFilename( baseName );

			MultiFileSystemPtr mfs = BWResource::instance().fileSystem();
			IFileSystem::Directory pFolder = mfs->readDirectory( directory );
			
			if (pFolder.size())
			{
				IFileSystem::Directory::iterator it = pFolder.begin();
				IFileSystem::Directory::iterator end = pFolder.end();

				for (; !isFont && it != end; ++it)
				{
					std::string fileName = directory + (*it);
					if (BWResource::getExtension( fileName ) == "font")
					{
						DataSectionPtr fontFile = 
							BWResource::openSection( fileName );
						if (fontFile)
						{
							std::string sourceFont = 
								fontFile->readString( "creation/sourceFont", "" );
							int fontSize = abs( fontFile->readInt( 
												"creation/sourceFontSize", 0 ) );
	
							char buffer[256];
							bw_snprintf( buffer, sizeof(buffer), "%s_%d", 
													sourceFont.c_str(), fontSize );
	
							if (fontName == std::string( buffer ))
							{
								isFont = true;
							}
						}
					}
				}
			}
#endif // PACK_FONT_DDS

			// if the DDS doesn't have a matching image file, then copy it
			if (!isFont && !PackerHelper::copyFile( src_, dst_ ))
			{
				return false;
			}
		}
		// the DDS was copied, or we want to skip it, so return true
		return true;
	}
	else
	{
		// convert to DDS using the source image and the texformat file
		std::string dissolved = BWResolver::dissolveFilename( src_ );
		Moo::TextureDetailLevelPtr lod = Moo::TextureManager::detailLevel( dissolved );
		if (lod->noResize())
		{
			// Detail level says don't resize, so copy the file over untouched.
			if (!PackerHelper::fileExists( dst_ ))
			{
				if (!PackerHelper::copyFile( src_, dst_ ))
				{
					return false;
				}
			}
		}
		else
		{
			std::string ddsName = BWResource::removeExtension( dissolved ) + ".dds";
			if ( !Moo::TextureManager::convertToDDS( dissolved, ddsName, lod->format() ) )
			{
				printf( "Error generating uncompressed DDS\n" );
				return false;
			}
			else
			{
				//If the convert to dds worked then copy it to the destination
				src_ = BWResource::removeExtension( src_ ) + ".dds";
				dst_ = BWResource::removeExtension( dst_ ) + ".dds";
				if (!PackerHelper::fileExists( dst_ ))
				{
					if (!PackerHelper::copyFile( src_, dst_ ))
					{
						return false;
					}
				}
			}
			// the uncompressed DDS should be already created in the destination
			// folder because the first path for BWResource is set to the dest
			// folder when initialising
			if ( lod->formatCompressed() != D3DFMT_UNKNOWN )
			{
				// generate compressed DDS
				ddsName = BWResource::removeExtension( dissolved ) + ".c.dds";
				if ( !Moo::TextureManager::convertToDDS( dissolved, ddsName, lod->formatCompressed() ) )
				{
					printf( "Error generating compressed DDS\n" );
					return false;
				}
				else
				{
					//If the convert to dds worked then copy it to the destination
					src_ = BWResource::removeExtension( src_ ) + ".c.dds";
					dst_ = BWResource::removeExtension( dst_ ) + ".c.dds";
					if (!PackerHelper::fileExists( dst_ ))
					{
						if (!PackerHelper::copyFile( src_, dst_ ))
						{
							return false;
						}
					}
				}
				// the compressed DDS should be already created in the destination
				// folder because the first path for BWResource is set to the dest
				// folder when initialising
			}
		}
	}
#else // MF_SERVER

	// just skip image files for the server

#endif // MF_SERVER

	return true;
}
