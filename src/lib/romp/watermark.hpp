/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WATERMARK_HPP
#define WATERMARK_HPP

std::string decodeWaterMark (const std::string& encodedData, int width, int height);

std::string encodeWaterMark(const char* data, size_t size);

#endif // WATERMARK_HPP
