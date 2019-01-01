/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVER_CHUNK_SPACE_HPP
#define SERVER_CHUNK_SPACE_HPP

#include "base_chunk_space.hpp"

/**
 *	This class is used to store chunk information about a space. It is used by
 *	the server.
 */
class ServerChunkSpace : public BaseChunkSpace
{
public:
	ServerChunkSpace( ChunkSpaceID id );

	void addHomelessItem( ChunkItem * /*pItem*/ ) {};
//	void delHomelessItem( ChunkItem * pItem );

	void focus();

	/**
	 *	This class is used to store the grid of columns.
	 */
	class ColumnGrid
	{
	public:
		ColumnGrid() :
			xOrigin_( 0 ),
			zOrigin_( 0 ),
			xSize_( 0 ),
			zSize_( 0 )
		{
		}

		void rect( int xOrigin, int zOrigin, int xSize, int zSize );

		Column *& operator()( int x, int z )
		{
			static Column * pNullCol = NULL;
			return this->inSpan( x, z ) ?
				grid_[ (x - xOrigin_) + xSize_ * (z - zOrigin_) ] :
				pNullCol;
		}

		const Column * operator()( int x, int z ) const
		{
			return this->inSpan( x, z ) ?
				grid_[ (x - xOrigin_) + xSize_ * (z - zOrigin_) ] :
				NULL;
		}

		bool inSpan( int x, int z ) const
		{
			return xOrigin_ <= x && x < xOrigin_ + xSize_ &&
					zOrigin_ <= z && z < zOrigin_ + zSize_;
		}

		const int xBegin() const	{	return xOrigin_; }
		const int xEnd() const		{	return xOrigin_ + xSize_; }
		const int zBegin() const	{	return zOrigin_; }
		const int zEnd() const		{	return zOrigin_ + zSize_; }

	private:
		std::vector< Column * > grid_;

		int		xOrigin_;
		int		zOrigin_;

		int		xSize_;
		int		zSize_;
	};

	void seeChunk( Chunk * );

protected:
	void recalcGridBounds();

	ColumnGrid					currentFocus_;

};

#endif // SERVER_CHUNK_SPACE_HPP
