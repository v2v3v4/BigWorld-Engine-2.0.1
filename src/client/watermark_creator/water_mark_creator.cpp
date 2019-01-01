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

#include <fstream>
#include <iostream>
#include "zip/zlib.h"
#include "cstdmf/base64.h"
#include "romp/watermark.hpp"

/**
 * This utility is used to create a base64 out of a ppm (see the example.ppm for sizs)
 * 1. It creates an encoded base64 string for it.
 * 2. it decodes this base64 into ppm again
 * 3. It writes the decoded ppm to disk so you can compare the original to the new decoded
 * ppm to see no changes have happened.
*/
int main(int argc, char* argv[])
{
	if (argc != 4) 
	{
		std::cout << "usage <file name to encrypt (ppm)> <width> <height>" << std::endl;
		//return 0 to not hurt the unit tests
		exit(0);
	}
	std::fstream readFile;
	readFile.open(argv[1], std::fstream::in | std::fstream::binary);
	
	if (! readFile)
	{
		std::cout << "can't read " << argv[1] << std::endl;
		exit(1);
	}

	//get the length of the file 
	readFile.seekg (0, std::ios::end);
	std::streamoff length = readFile.tellg();
	readFile.seekg (0, std::ios::beg);

	std::string readString((size_t)length, 'a');
	readFile.read((char*)readString.data(), length);
	std::cout << "read " << length << std::endl;
	const char* startLocation = 0;
	if (strstr((const char*)readString.data(), "255") == 0)
	{
		std::cout << "Corrupt file " << argv[1] << std::endl;
		exit(1);
	}
	std::string encodedData = encodeWaterMark(readString.data(), readString.size());
	std::cout << "EncodedData:\n" << encodedData << std::endl;
	std::string txstr = decodeWaterMark (encodedData, atoi(argv[2]), atoi(argv[3]));

	unlink("c:\\temp\\out.ppm");
	FILE* outputFile = fopen("c:\\temp\\out.ppm", "wb");
	//fwrite(txstr.c_str() + 4, 1, txstr.size(), outputFile);
	fwrite(txstr.c_str() + 4, 1, txstr.size(), outputFile);
	fclose(outputFile);

	exit(0);
}
