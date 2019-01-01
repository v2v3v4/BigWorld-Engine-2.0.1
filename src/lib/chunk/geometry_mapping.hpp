/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GEOMETRY_MAPPING_HPP
#define GEOMETRY_MAPPING_HPP

#include "cstdmf/aligned.hpp"
#include "cstdmf/smartpointer.hpp"

#include "math/matrix.hpp"

class BoundingBox;

class ChunkSpace;
typedef SmartPointer< ChunkSpace > ChunkSpacePtr;

class DataSection;
typedef SmartPointer< DataSection > DataSectionPtr;

class Vector3;

/**
 *	This class is a mapping of a resource directory containing chunks
 *	into a chunk space.
 *
 *	@note Only its chunk space and chunks queued to load retain references
 *	to this object.
 */
class GeometryMapping : public Aligned, public SafeReferenceCount
{
public:
	GeometryMapping( ChunkSpacePtr pSpace, const Matrix & m,
		const std::string & path, DataSectionPtr pSettings );
	~GeometryMapping();

	ChunkSpacePtr	pSpace() const			{ return pSpace_; }

	const Matrix &	mapper() const			{ return mapper_; }
	const Matrix &	invMapper() const		{ return invMapper_; }

	const std::string & path() const		{ return path_; }
	const std::string & name() const		{ return name_; }

	DataSectionPtr	pDirSection();
	static DataSectionPtr openSettings( const std::string & path );

	/// The following accessors return the world-space grid bounds of this
	/// mapping, after the transform is applied. These bounds are expanded
	/// to include even the slightest intersection of the mapping with a
	/// grid square in the space's coordinate system
	int minGridX() const					{ return minGridX_; }
	int maxGridX() const 					{ return maxGridX_; }
	int minGridY() const 					{ return minGridY_; }
	int maxGridY() const 					{ return maxGridY_; }

	/// The following accessors return the bounds of this mapping in its
	/// own local coordinate system.
	int minLGridX() const					{ return minLGridX_; }
	int maxLGridX() const 					{ return maxLGridX_; }
	int minLGridY() const 					{ return minLGridY_; }
	int maxLGridY() const 					{ return maxLGridY_; }

	/// Utility coordinate space mapping functions
	void gridToLocal( int x, int y, int& lx, int& ly ) const;
	void boundsToGrid( const BoundingBox& bb,
		int& minGridX, int& minGridZ,
		int& maxGridX, int& maxGridZ ) const;
	void gridToBounds( int minGridX, int minGridY,
		int maxGridX, int maxGridY,
		BoundingBox& retBB ) const;
	bool inLocalBounds( const int gridX, const int gridZ ) const;
	bool inWorldBounds( const int gridX, const int gridZ ) const;

	bool condemned()						{ return condemned_; }
	void condemn();

	std::string outsideChunkIdentifier( const Vector3 & localPoint,
		bool checkBounds = true ) const;
	std::string outsideChunkIdentifier( int x, int z, bool checkBounds = true ) const;
	static bool gridFromChunkName( const std::string& chunkName, int16& x, int16& z );

private:
	ChunkSpacePtr	pSpace_;

	Matrix			mapper_;
	Matrix			invMapper_;

	std::string		path_;
	std::string		name_;
	DataSectionPtr	pDirSection_;

	int				minGridX_;
	int				maxGridX_;
	int				minGridY_;
	int				maxGridY_;

	int				minLGridX_;
	int				maxLGridX_;
	int				minLGridY_;
	int				maxLGridY_;

	bool			condemned_;
	bool			singleDir_;
};


/**
 *	This class is an interface used to create GeometryMapping instances. It is
 *	used by the server to create a derived type.
 */
class GeometryMappingFactory
{
public:
	virtual ~GeometryMappingFactory() {}

	virtual GeometryMapping * createMapping( ChunkSpacePtr pSpace,
				const Matrix & m, const std::string & path,
				DataSectionPtr pSettings ) = 0;
};

#endif // GEOMETRY_MAPPING_HPP
