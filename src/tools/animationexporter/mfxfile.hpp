/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MFXFILE_HPP
#define MFXFILE_HPP

#include "chunkids.hpp"

#include <iostream>

class MFXFile
{
public:
	MFXFile();
	~MFXFile();

	bool openForOutput( const std::string &filename );
	bool openForInput( const std::string & filename );
	bool close( void );

	void writeChunkHeader( ChunkID id, int totalSize, int size );
	void writePoint( const Point3 &p );
	void writeColour( const Color &c );
	void writeUV( const Point3 &uv );
	void writeMatrix( const Matrix3 &m );
	void writeInt( int i );
	void writeFloat( float f );
	void writeString( const std::string &s );
	void writeTriangle( unsigned int a, unsigned int b, unsigned int c );
	static unsigned int getStringSize( const std::string &s );

	bool readChunkHeader( ChunkID &id, int &totalSize, int &size );
	bool readInt( int &i );
	bool readString( std::string &s );
	void nextChunk();

private:

	FILE	*stream_;

	int		nextChunkStart_;

	MFXFile(const MFXFile&);
	MFXFile& operator=(const MFXFile&);

	friend std::ostream& operator<<(std::ostream&, const MFXFile&);
};

#ifdef CODE_INLINE
#include "mfxfile.ipp"
#endif




#endif
/*mfx.hpp*/
