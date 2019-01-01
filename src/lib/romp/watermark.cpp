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

#include "cstdmf/base64.h"
#include "zip/zlib.h"

#include <iostream>
#include <sstream>
#include <set>
#include <vector>

/**
* Decode a base 64 into a ppm to be used to display a watermark
* 
* @param encodedData - the base64 string
*
*/
std::string decodeWaterMark (const std::string& encodedData, int width, int height)
{
	std::string compData( 512 * 1024, '\0' );
	//decode it
	int ub64Size = Base64::decode( encodedData, (char*)compData.data(), compData.size());

	//now inflate it
	const int unzipSize = 512 * 1024;
	uint8* ltl = new uint8[unzipSize];
	z_stream zs;
	zs.next_in = (unsigned char *)compData.data();
	zs.avail_in = ub64Size;
	zs.next_out = (unsigned char *)ltl;
	zs.avail_out = unzipSize;
	zs.zalloc = NULL;
	zs.zfree = NULL;
	inflateInit(&zs);
	inflate(&zs, Z_FINISH); 
	inflateEnd(&zs);
	
	int w = width;
	int h = height;
	std::string txstr( w*h*3, '\0' );
	std::stringstream out;
	//Add the ppm header 
	out << "////P6\n" << w << " " << h << "\n255\n";
	std::string txbeg = out.str();
	txstr = txbeg + txstr;
	uint8 *inbeg = (uint8*) ltl;
	//as red green and blue are equal we use just one of them (grayscale)
	uint8 *outbeg = (uint8*)txstr.data()+strlen(txbeg.c_str());
	for (int i = w*h-1; i >= 0; i--)
		outbeg[i*3] = outbeg[i*3+1] = outbeg[i*3+2] = inbeg[i];
	
	delete[] ltl;
	return txstr;
}

/**
* Encode a ppm (specific sizes) into a base 64 
* 
* @param data A char buffer containing the ppm data
* @param size A size_t for the buffer size 
*
*/
std::string encodeWaterMark(const char* data, size_t size)
{
	const char* startLocation = 0;
	if (strstr((const char*)data, "255") != 0)
	{
		startLocation = strstr((const char*)data, "255") + 3;
		while (*startLocation == ' ' || *startLocation == '\n' || *startLocation == '\r') 
		{
			startLocation++;
		}
	}
	else 
	{
		std::cout << "Corrupt watermark" << std::endl;
		exit(1);
	}
	//as red green and blue are equal we use just one of them
	int nElems = (size - (startLocation  - data)) / 3;
	std::string strippedData(nElems, 'a');
	char* strippedDataChar = (char*)strippedData.data();
	for (unsigned int i = 0; i < strippedData.size(); i++)
		strippedDataChar[i] = startLocation[i*3];


	//now zip it
	std::string deflatedData(nElems * 10, 'a');

	z_stream zs;
	zs.next_in = (unsigned char *)strippedData.data();
	zs.avail_in = strippedData.size();
	zs.next_out = (unsigned char *)deflatedData.data();
	zs.avail_out = deflatedData.size();
	zs.zalloc = NULL;
	zs.zfree = NULL;
	deflateInit(&zs, Z_DEFAULT_COMPRESSION);
	deflate(&zs, Z_FINISH);
	deflateEnd(&zs);
	//now encode it
	std::string encodedData = Base64::encode( (const char*)deflatedData.data(), zs.total_out);
	return encodedData;
}
