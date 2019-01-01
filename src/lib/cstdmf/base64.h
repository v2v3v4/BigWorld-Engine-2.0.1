
//*********************************************************************
//* C_Base64 - a simple base64 encoder and decoder.
//*
//*     Copyright (c) 1999, Bob Withers - bwit@pobox.com
//*
//* This code may be freely used for any purpose, either personal
//* or commercial, provided the authors copyright notice remains
//* intact.
//*********************************************************************

#ifndef Base64_H
#define Base64_H

#include <string>

/**
 *  This class provides encoding/decoding of Base64 streams.
 */
class Base64
{
public:
	static std::string encode(const char* data, size_t len);
	static std::string encode( const std::string & data )
	{
		return encode( data.data(), data.size() );
	}

	static int decode(const std::string& data, char* results, size_t bufSize);
	static bool decode( const std::string & inData, std::string & outData );

private:
	static std::string Base64Table;
	static std::string::size_type DecodeTable[];
};

#endif
