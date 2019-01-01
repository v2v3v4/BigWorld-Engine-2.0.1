/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BASIC_TYPES_HPP
#define BASIC_TYPES_HPP

#ifdef _XBOX360
#include "net_360.hpp"
#endif

/// @todo It would be better not to have any include in the common header file.
#include "cstdmf/stdmf.hpp"
#include "cstdmf/md5.hpp"

#include "math/vector3.hpp"

#include <string>
#include <vector>

class Watcher;

// Fwd decl for streaming operators
class BinaryIStream;
class BinaryOStream;

/**
 *	The number of bytes in the header of a UDP packet.
 *
 * 	@ingroup common
 */
const int UDP_OVERHEAD = 28;

/**
 *	The number of bits per byte.
 *
 * 	@ingroup common
 */
const int NETWORK_BITS_PER_BYTE = 8;

/**
 *  Standard localhost address.  Assumes little endian host byte order.
 *
 * 	@ingroup common
 */
const uint32 LOCALHOST = 0x0100007F;

/**
 *  Standard broadcast address.  Assumes little endian host byte order.
 *
 * 	@ingroup common
 */
const uint32 BROADCAST = 0xFFFFFFFF;


/**
 * Watcher Nub packet size.
 *
 * @ingroup common
 */
static const int WN_PACKET_SIZE = 0x10000;

static const int WN_PACKET_SIZE_TCP = 0x1000000;


/**
 *	A game timestamp. This is synchronised between all clients and servers,
 *	and is incremented by one every game tick.
 *
 * 	@ingroup common
 */
typedef uint32 GameTime;


/**
 *	A unique identifier representing an entity. An entity's ID is constant
 *	between all its instances on the baseapp, cellapp and client.
 *	EntityIDs are however not persistent and should be assumed to be different
 *	upon each run of the server.
 *
 *	A value of zero is considered a null value as defined by NULL_ENTITY.
 *	The range of IDs greater than 1<<30 is reserved for client only entities.
 *
 *	This type replaces the legacy ObjectID type.
 *
 * 	@ingroup common
 */
typedef int32 EntityID;

/**
 *	The null value for an EntityID meaning that no entity is referenced.
 *
 * 	@ingroup common
 */
const EntityID NULL_ENTITY = ((EntityID)0);

/**
 *	A unique ID representing a space.
 *
 *	A value of zero is considered a null value as defined by NULL_SPACE.
 *
 * 	@ingroup common
 */
typedef int32 SpaceID;

/**
 *	The null value for an SpaceID meaning that no space is referenced.
 *
 * 	@ingroup common
 */
const SpaceID NULL_SPACE = ((SpaceID)0);

/**
 *	A unique ID representing a cell application.
 *
 * 	@ingroup common
 */
typedef int32 CellAppID;

/**
 *	A unique ID representing a base application.
 *
 * 	@ingroup common
 */
typedef int32 BaseAppID;

/**
 *	The authentication key used between ClientApp and BaseApp.
 */
typedef uint32 SessionKey;

/**
 *	A 3D vector used for storing object positions.
 *
 * 	@ingroup common
 */
typedef Vector3 Position3D;

/**
 *	A unique ID representing a class of entity.
 *
 * 	@ingroup common
 */
typedef uint16 EntityTypeID;
const EntityTypeID INVALID_ENTITY_TYPE_ID = EntityTypeID(-1);

/**
 *  An ID representing an entities position in the database
 */
typedef int64 DatabaseID;
const DatabaseID PENDING_DATABASE_ID = -1;

/**
 *  printf format string for DatabaseID e.g.
 *		DatabaseID dbID = &lt;something>;
 *		printf( "Entity %"FMT_DBID" is not valid!", dbID );
 */
#ifndef _WIN32
#ifndef _LP64
	#define FMT_DBID "lld"
#else
	#define FMT_DBID "ld"
#endif
#else
	#define FMT_DBID "I64d"
#endif

/**
 *	This is a 32 bit ID that represents a grid square. The most significant
 *	16 bits is the signed X position. The least significant 16 bits is the
 *	signed Y position.
 */
typedef uint32 GridID;

/**
 *	The type used as a sequence number for the events of an entity.
 */
typedef int32 EventNumber;
const EventNumber INITIAL_EVENT_NUMBER = 1;

typedef std::vector< EventNumber > CacheStamps;


/**
 *	The type used as a sequence number for volatile updates.
 */
typedef uint16 VolatileNumber;

typedef uint8 DetailLevel;

const int MAX_DATA_LOD_LEVELS = 6;

/**
 * 	This structure stores a 3D direction.
 *
 * 	@ingroup common
 */
struct Direction3D
{
	Direction3D() {};
	Direction3D( const Vector3 & v ) :
		roll ( v[0] ),
		pitch( v[1] ),
		yaw  ( v[2] ) {}

	Vector3 asVector3() const { return Vector3( roll, pitch, yaw ); }

	float roll;		///< The roll component of the direction
	float pitch;	///< The pitch component of the direction
	float yaw;		///< The yaw component of the direction
};

BinaryIStream& operator>>( BinaryIStream &is, Direction3D &d );
BinaryOStream& operator<<( BinaryOStream &is, const Direction3D &d );

std::istream & operator>>( std::istream &is, Direction3D &d );
std::ostream & operator<<( std::ostream &os, const Direction3D &d );


namespace Mercury
{
	/**
	 *	This class encapsulates an IP address and TCP port.
	 *
	 *	@ingroup mercury
	 */
	class Address
	{
	public:
		/// @name Construction/Destruction
		// @{
		Address();
		Address( uint32 ipArg, uint16 portArg );
		// @}

		uint32	ip;		///< IP address.
		uint16	port;	///< The port.
		uint16	salt;	///< Different each time.

		int writeToString( char * str, int length ) const;

		// TODO: Remove this operator
		operator char*() const	{ return this->c_str(); }
		char * c_str() const;
		const char * ipAsString() const;

		bool isNone() const			{ return this->ip == 0; }

		static Watcher & watcher();

		static const Address NONE;

	private:
		/// Temporary storage used for converting the address to a string.  At
		/// present we support having two string representations at once.
		static const int MAX_STRLEN = 32;
		static char s_stringBuf[ 2 ][ MAX_STRLEN ];
		static int s_currStringBuf;
		static char * nextStringBuf();
	};

	inline bool operator==(const Address & a, const Address & b);
	inline bool operator!=(const Address & a, const Address & b);
	inline bool operator<(const Address & a, const Address & b);

	BinaryIStream& operator>>( BinaryIStream &in, Address &a );
	BinaryOStream& operator<<( BinaryOStream &out, const Address &a );

	bool watcherStringToValue( const char *, Address & );
	std::string watcherValueToString( const Address & );
} // namespace Mercury

/**
 *	This structure is a packed version of a mailbox for an entity
 */
struct EntityMailBoxRef
{
	EntityID			id;
	Mercury::Address	addr;

	enum Component
	{
		CELL = 0,
		BASE = 1,
		CLIENT = 2,
		BASE_VIA_CELL = 3,
		CLIENT_VIA_CELL = 4,
		CELL_VIA_BASE = 5,
		CLIENT_VIA_BASE = 6
	};

	Component component() const		{ return (Component)(addr.salt >> 13); }
	void component( Component c )	{ addr.salt = type() | (uint16(c) << 13); }

	EntityTypeID type() const		{ return addr.salt & 0x1FFF; }
	void type( EntityTypeID t )		{ addr.salt = (addr.salt & 0xE000) | t; }

	void init() { id = 0; addr.ip = 0; addr.port = 0; addr.salt = 0; }
	void init( EntityID i, const Mercury::Address & a,
		Component c, EntityTypeID t )
	{ id = i; addr = a; addr.salt = (uint16(c) << 13) | t; }

	static const char * componentAsStr( Component component );

	const char * componentName() const
	{
		return componentAsStr( this->component() );
	}
};


/**
 *	This class maintains a set of 32 boolean capabilities.
 *
 *	@ingroup common
 */
class Capabilities
{
	typedef uint32 CapsType;
public:
	Capabilities();
	void add( uint cap );
	bool has( uint cap ) const;
	bool empty() const;
	bool match( const Capabilities& on, const Capabilities& off ) const;
	bool matchAny( const Capabilities& on, const Capabilities& off ) const;

	/// This is the maximum cap that a Capability object can store.
	static const uint s_maxCap_ = std::numeric_limits<CapsType>::digits - 1;
private:

	/// This member stores the capabilities bitmask.
	CapsType	caps_;
};


/// This type identifies an entry in the data for a space
class SpaceEntryID : public Mercury::Address { };
inline bool operator==(const SpaceEntryID & a, const SpaceEntryID & b);
inline bool operator!=(const SpaceEntryID & a, const SpaceEntryID & b);
inline bool operator<(const SpaceEntryID & a, const SpaceEntryID & b);

#include "basictypes.ipp"
#endif // BASIC_TYPES_HPP
