/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "stdafx.h"

#include "test_harness.hpp"

#include "resmgr/file_system.hpp"
#include "resmgr/multi_file_system.hpp"

#include <string.h>

const std::string TEMP_DIR( "/tmp/" );

TEST_F( ResMgrUnitTestHarness, ResMgrTest_IFileSystem )
{
	CHECK( this->isOK() );

	const std::string NEW_DIR( "bigworld_file_system_test" );
	const std::string NEW_FILE( "test_file" );

	FileSystemPtr pFS = NativeFileSystem::create( TEMP_DIR );
	
	// Test makeDirectory and getFileType
	CHECK( pFS->makeDirectory( NEW_DIR ) );
	CHECK( !pFS->makeDirectory( NEW_DIR ) );
	CHECK( pFS->getFileType( NEW_DIR ) == IFileSystem::FT_DIRECTORY );
	
	// Test writeFile and getFileType
	const std::string relativeFilePath = NEW_DIR + '/' + NEW_FILE;
	const char * testFileData = ". . . ignorance gives one a large range of "
			"probabilities. -- Daniel Deronda by George Eliot";
	size_t testFileDataLen = strlen( testFileData );
	BinaryPtr	pTestFileDataBlock( 
			new BinaryBlock( testFileData, testFileDataLen, "std" ) );
	CHECK( pFS->writeFile( relativeFilePath, pTestFileDataBlock, true ) );
	CHECK( pFS->getFileType( relativeFilePath ) == IFileSystem::FT_FILE );
	IFileSystem::FileInfo fi;
	CHECK( pFS->getFileType( relativeFilePath, &fi ) == IFileSystem::FT_FILE );
	CHECK( fi.size == testFileDataLen && fi.created > 0 && 
			fi.modified > 0 && fi.accessed > 0 );
	
	// Test getAbsolutePath
	const std::string absoluteFilePath = TEMP_DIR + relativeFilePath;
	const std::string absoluteDirPath = TEMP_DIR + NEW_DIR;
	CHECK( pFS->getAbsolutePath( relativeFilePath ) == absoluteFilePath );
	CHECK( pFS->getAbsolutePath( NEW_DIR ) == absoluteDirPath );
	
	// Test getAbsoluteFileType
	const std::string absoluteNonExistPath = TEMP_DIR + "i.dont.really.exist";
	CHECK( NativeFileSystem::getAbsoluteFileType( absoluteFilePath ) == 
		IFileSystem::FT_FILE );
	CHECK( NativeFileSystem::getAbsoluteFileType( absoluteDirPath ) == 
		IFileSystem::FT_DIRECTORY );
	CHECK( NativeFileSystem::getAbsoluteFileType( absoluteNonExistPath ) == 
		IFileSystem::FT_NOT_FOUND );
	CHECK( NativeFileSystem::getAbsoluteFileType( absoluteFilePath, &fi ) == 
		IFileSystem::FT_FILE );
	CHECK( fi.size == testFileDataLen && fi.created > 0 && 
			fi.modified > 0 && fi.accessed > 0 );
	
	// Test eraseFileOrDirectory and getFileType
	CHECK( pFS->eraseFileOrDirectory( relativeFilePath ) );
	CHECK( !pFS->eraseFileOrDirectory( relativeFilePath ) );
	CHECK( pFS->getFileType( relativeFilePath ) == IFileSystem::FT_NOT_FOUND );
	CHECK( pFS->eraseFileOrDirectory( NEW_DIR ) );
	CHECK( !pFS->eraseFileOrDirectory( NEW_DIR ) );
	CHECK( pFS->getFileType( NEW_DIR ) == IFileSystem::FT_NOT_FOUND );
}

TEST_F( ResMgrUnitTestHarness, ResMgr_TestMultiFileSystem )
{
	CHECK( this->isOK() );

	const std::string FIRST_PATH = "ichy/";
	const std::string SECOND_PATH = "knee/";
	
	// Set-up MultiFileSystem
	FileSystemPtr pTempFS = NativeFileSystem::create( TEMP_DIR );
	CHECK( pTempFS->makeDirectory( FIRST_PATH ) );
	CHECK( pTempFS->makeDirectory( SECOND_PATH ) );
	
	FileSystemPtr pFS1 = NativeFileSystem::create( TEMP_DIR + FIRST_PATH );
	FileSystemPtr pFS2 = NativeFileSystem::create( TEMP_DIR + SECOND_PATH );
	
	MultiFileSystem multiFS;
	
	multiFS.addBaseFileSystem( pFS1 );
	multiFS.addBaseFileSystem( pFS2 );
	
	// Test resolveToAbsolutePath
	const char * testFileData = "Any sufficiently advanced technology is "
			"indistinguishable from magic. -- Arthur C. Clarke";
	size_t testFileDataLen = strlen( testFileData );
	BinaryPtr	pTestFileDataBlock( 
			new BinaryBlock( testFileData, testFileDataLen, "std" ) );
	std::string secondFullPath = "blah";
	CHECK( pFS2->writeFile( secondFullPath, pTestFileDataBlock, true ) );
	CHECK( multiFS.resolveToAbsolutePath( secondFullPath ) == 
			IFileSystem::FT_FILE );
	CHECK( secondFullPath == TEMP_DIR + SECOND_PATH + "blah" );
	std::string firstFullPath = "blah";
	CHECK( pFS1->makeDirectory( firstFullPath ) );
	CHECK( multiFS.resolveToAbsolutePath( firstFullPath ) == 
			IFileSystem::FT_DIRECTORY );
	CHECK( firstFullPath == TEMP_DIR + FIRST_PATH + "blah" );
	std::string nonExistPath = "i.dont.exist";
	CHECK( multiFS.resolveToAbsolutePath( nonExistPath ) == 
			IFileSystem::FT_NOT_FOUND );
	CHECK( nonExistPath == "i.dont.exist" );

	// Clean-up
	CHECK( pFS2->eraseFileOrDirectory( "blah" ) );
	CHECK( pFS1->eraseFileOrDirectory( "blah" ) );
	
	CHECK( pTempFS->eraseFileOrDirectory( FIRST_PATH ) );
	CHECK( pTempFS->eraseFileOrDirectory( SECOND_PATH ) );
}

// test_file_system.cpp
