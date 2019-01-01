/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STREAM_HELPER_HPP
#define STREAM_HELPER_HPP

#include "cstdmf/binary_stream.hpp"

#include "math/vector3.hpp"

#include "network/basictypes.hpp"
#include "network/misc.hpp"

namespace StreamHelper
{
	/**
	 *	This helper function is used to stream on data that will be streamed off
	 *	in the constructor of RealEntity.
	 */
	inline void addRealEntity( BinaryOStream & /*stream*/ )
	{
	}

	/**
	 *	This helper function is used to stream on data that will be streamed off
	 *	in the constructor of RealEntityWithWitnesses.
	 */
	inline void addRealEntityWithWitnesses( BinaryOStream & stream,
			int maxPacketSize = 2000,
			float aoiRadius = 0.f,
			float aoiHyst = 5.f )
	{
		stream << maxPacketSize;				// Max packet size
		stream << uint32(0);					// SpaceData entry count
		stream << uint32(0);					// Entity queue size
		stream << aoiRadius;					// AoI radius
		stream << aoiHyst;						// AoI Hyst
		stream << 0.f;							// Stealth factor
	}

#pragma pack( push, 1 )
	/** @internal */
	struct AddEntityData
	{
		AddEntityData() :
			id( 0 ),
			isOnGround( false ),
			lastEventNumber( 1 ),
			baseAddr( Mercury::Address( 0, 0 ) )
		{}
		AddEntityData( EntityID _id, const Vector3 & _position,
				bool _isOnGround, EntityTypeID _typeID,
				const Direction3D & _direction,
				Mercury::Address _baseAddr = Mercury::Address( 0, 0 ) ) :
			id( _id ),
			typeID( _typeID ),
			position( _position ),
			isOnGround( _isOnGround ),
			direction( _direction ),
			lastEventNumber( 1 ),
			baseAddr( _baseAddr )
		{}

		EntityID id;
		EntityTypeID typeID;
		Vector3 position;
		bool isOnGround;
		Direction3D direction;
		EventNumber lastEventNumber;
		Mercury::Address baseAddr;
	};
#pragma pack( pop )


	/**
	 *	This helper function is used to stream on data that will be streamed off
	 *	in the initialisation of the entity. See Entity::initReal.
	 */
	inline void addEntity( BinaryOStream & stream, const AddEntityData & data )
	{
		stream << data;
	}


	/**
	 *	This helper function is used to remove data from a stream that was
	 *	streamed on by StreamHelper::addEntity.
	 *
	 *	@return The amount of data that was read off.
	 */
	inline int removeEntity( BinaryIStream & stream, AddEntityData & data )
	{
		stream >> data;

		return sizeof( data );
	}


	/**
	 *	This helper function is used to add the cell entity's log-off data.
	 */
	inline void addEntityLogOffData( BinaryOStream & stream,
			EventNumber number )
	{
		stream << number;
	}


	/**
	 *	This helper function is used to remove the cell entity's log-off data.
	 */
	inline void removeEntityLogOffData( BinaryIStream & stream,
			EventNumber & number )
	{
		stream >> number;
	}


	/**
	 *	This contains the data that is at the start of the backup data sent from
	 *	the cell entity to the base entity.
	 */
	class CellEntityBackupFooter
	{
	public:
		CellEntityBackupFooter( int32 cellClientSize, EntityID vehicleID ) :
			cellClientSize_( cellClientSize ),
			vehicleID_( vehicleID )
		{
		}

		int32		cellClientSize() const	{ return cellClientSize_; }
		EntityID	vehicleID() const		{ return vehicleID_; }

	private:
		int32		cellClientSize_;
		EntityID	vehicleID_;
	};
}

#endif // STREAM_HELPER_HPP
