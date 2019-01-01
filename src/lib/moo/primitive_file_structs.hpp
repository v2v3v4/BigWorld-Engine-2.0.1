/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PRIMITIVE_FILE_STRUCTS_HPP
#define PRIMITIVE_FILE_STRUCTS_HPP

namespace Moo 
{

/**
 *	Header structure for the vertex values stored in a file.
 */
struct VertexHeader
{
	char	vertexFormat_[ 64 ];
	int		nVertices_;
};


/**
 *	Header structure for the morph values stored in a file.
 */
struct MorphHeader
{
	int version_;
	int nMorphTargets_;
};


/**
 *	Header structure for the morph target information stored in a file.
 *	Appears directly after the MorphHeader.
 */
struct MorphTargetHeader
{
	char	identifier_[ 64 ];
	int		channelIndex_;
	int		nVertices_;
};


/**
 *	Header structure for the index values stored in a file.
 */
struct IndexHeader
{
	char	indexFormat_[ 64 ];
	int		nIndices_;
	int		nTriangleGroups_;
};


/**
 *	The primitive group structure maps to a set of indices within the index
 *	buffer.
 */
struct PrimitiveGroup
{
	int		startIndex_;
	int		nPrimitives_;
	int		startVertex_;
	int		nVertices_;
};

}

#endif // PRIMITIVE_FILE_STRUCTS_HPP
