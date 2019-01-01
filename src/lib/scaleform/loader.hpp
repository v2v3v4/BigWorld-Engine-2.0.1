/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCALEFORM_LOADER_HPP
#define SCALEFORM_LOADER_HPP
#if SCALEFORM_SUPPORT

#include <GFXLoader.h>
#include "pyscript/script.hpp"
#include "resmgr/binary_block.hpp"

namespace Scaleform
{
	class FileOpener : public GFxFileOpener
	{
	public:
		virtual GFile* OpenFile(const char *pfilename, SInt flags = GFileConstants::Open_Read|GFileConstants::Open_Buffered, SInt mode = GFileConstants::Mode_ReadWrite);
	};


	class BWToGFxImageLoader : public GFxImageLoader
	{
	public:
		BWToGFxImageLoader(){}
		GImageInfoBase* LoadImage(const char *purl);
	};


	// We're deriving from GMemoryFile so that we can store the BinaryPtr on the file instance
	// rather than on the FileOpener singleton. This will avoid any problems that may arise if
	// scaleform decides it wants to read from two files at the same time (when stored on the singleton
	// and if you load a second file, the BinaryPtr will be replaced - potentially invalidating the 
	// original file's memory buffer).
	class BWGMemoryFile : public GMemoryFile
	{
	public:
		BWGMemoryFile (const char *pFileName, BinaryPtr ptr)
			:	GMemoryFile( pFileName, (const UByte*)ptr->cdata(), ptr->len() ),
				swfFile_(ptr)
		{
			BW_GUARD;
			//INFO_MSG("BWGMemoryFile::BWGMemoryFile - %s (BinaryPtr: 0x%p, len: %d, compressed: %d)\n", pFileName, ptr.get(), ptr->len(), ptr->isCompressed() );
		}

		virtual ~BWGMemoryFile()
		{
			BW_GUARD;
			//INFO_MSG("BWGMemoryFile::~BWGMemoryFile\n");
		}

	private:
		BinaryPtr swfFile_;
	};


	class Loader : public GFxLoader
	{
	public:
		GPtr<GFxFileOpener>			pFileOpener_;

		Loader();
		~Loader();
	};


}	//namespace Scaleform

#endif  //#if SCALEFORM_SUPPORT
#endif	//SCALEFORM_LOADER_HPP