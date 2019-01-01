/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PRIMITIVE_FILE_HPP
#define PRIMITIVE_FILE_HPP

#include "cstdmf/smartpointer.hpp"

#include <map>
#include <string>

class BinaryBlock;
typedef SmartPointer<BinaryBlock> BinaryPtr;

class DataSection;
typedef SmartPointer<DataSection> DataSectionPtr;

class PrimitiveFile;
typedef SmartPointer<PrimitiveFile> PrimitiveFilePtr;

/**
 *	This class provides access to a file that contains binary data
 *	for a number of primitive resources. At some stage this kind
 *	of functionality will be integrated into the resource manager
 *	(using either a file system or a data section or some strange
 *	hybrid - zip file reform will happen then too), but for now it
 *	serves only primitive data
 */
class PrimitiveFile : public ReferenceCount
{
public:
	~PrimitiveFile();

	static DataSectionPtr get( const std::string & resourceID );

#if 0	 // ifdef'd out since functionality moved to BinSection
	BinaryPtr readBinary( const std::string & name );

	BinaryPtr updateBinary( const std::string & name,
		void * data, int len );
	void updateBinary( const std::string & name, BinaryPtr bp );

	void deleteBinary( const std::string & name );


	void save( const std::string & name );

#ifdef _WIN32
private:
#else
// Avoiding warning under gcc.
public:
#endif
	PrimitiveFile( BinaryPtr pFile );
private:

	PrimitiveFile( const PrimitiveFile& );
	PrimitiveFile& operator=( const PrimitiveFile& );

	typedef std::map<std::string,BinaryPtr> Directory;
	Directory	dir_;
	// intentionally not using stringmap as these are not likely
	// to be the kind of strings that it handles well
	// (i.e. they'll probably have common prefixes)

	static void add( const std::string & id, PrimitiveFile * pPrimitiveFile );
	static void del( PrimitiveFile * pPrimitiveFile );

	BinaryPtr	file_;
#endif
};


// utility functions until primitive references have transitioned

void splitOldPrimitiveName( const std::string & resourceID,
	std::string & file, std::string & part);

BinaryPtr fetchOldPrimitivePart( std::string & file, std::string & part );


#ifdef CODE_INLINE
#include "primitive_file.ipp"
#endif

#endif // PRIMITIVE_FILE_HPP
