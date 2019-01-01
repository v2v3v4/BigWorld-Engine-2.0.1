/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 * 	@file
 *
 * 	This file defines the MethodDescription class.
 *
 *	@ingroup entity
 */
#ifndef METHOD_DESCRIPTION_HPP
#define METHOD_DESCRIPTION_HPP

#include "Python.h"

#include "cstdmf/smartpointer.hpp"

#include "network/basictypes.hpp"

#include "resmgr/datasection.hpp"

#include "entitydef/entity_member_stats.hpp"

#include <set>
#include <string>

class BinaryIStream;
class BinaryOStream;
class DataType;
class MD5;
class Watcher;

namespace Mercury
{
	class NetworkInterface;
}

typedef SmartPointer<Watcher> WatcherPtr;

typedef SmartPointer<PyObject> PyObjectPtr;
typedef SmartPointer<DataType> DataTypePtr;

typedef uint16 MethodIndex;

/**
 *	This class is used to describe a method associated with an entity class.
 *
 *	@ingroup entity
 */
class MethodDescription
{
public:
	MethodDescription();
	MethodDescription( const MethodDescription& description );
	~MethodDescription();

	MethodDescription & operator=( const MethodDescription & description );

	enum Component
	{
		CLIENT,
		CELL,
		BASE
	};

	bool parse( DataSectionPtr pSection, Component component );

	bool parseReturnValues( DataSectionPtr pSection );

	bool areValidArgs( bool exposedExplicit, PyObject * args,
		bool generateException ) const;

	bool addToStream( bool isFromServer, PyObject * args,
		BinaryOStream & stream ) const;

	bool callMethod( PyObject * self,
		BinaryIStream & data,
		EntityID sourceID = 0,
		int replyID = -1,
		const Mercury::Address* pReplyAddr = NULL,
		Mercury::NetworkInterface * pInterface = NULL ) const;

	PyObjectPtr getArgsAsTuple( BinaryIStream & data,
		EntityID sourceID = 0 ,
		int replyID = -1,
		const Mercury::Address* pReplyAddr = NULL,
		Mercury::NetworkInterface * pInterface = NULL ) const;

	uint returnValues() const;

	const std::string& returnValueName( uint index ) const;

	DataTypePtr returnValueType( uint index ) const;

	/// This method returns the name of the method.
	const std::string&	name() const				{ return name_; }

	/// This method returns true if the method is exposed.
	bool isExposed() const					{ return !!(flags_ & IS_EXPOSED); }
	void setExposed()						{ flags_ |= IS_EXPOSED; }

	Component component() const				{ return Component(flags_ & 0x3); }
	void component( Component c )			{ flags_ |= c; }

	MethodIndex internalIndex() const
	{ return MethodIndex( internalIndex_ ); }

	void internalIndex( int index )			{ internalIndex_ = index; }

	MethodIndex exposedIndex() const
	{ return MethodIndex( exposedIndex_ ); }

	void exposedIndex( MethodIndex index, int subIndex = -1 )
	{
		exposedIndex_ = index;
		exposedSubIndex_ = subIndex;
	}

	void addToMD5( MD5 & md5, int legacyExposedIndex ) const;

	float priority() const;

	bool hasPythonArg() const;

	EntityMemberStats & stats() const
	{ return stats_; }

#if ENABLE_WATCHERS
	static WatcherPtr pWatcher();
#endif
private:
	enum
	{
		IS_EXPOSED = 0x4
	};

	std::string		name_;					///< The name of the method.

	// Lowest two bits for which component. Next for whether it can be called by
	// the client.
	uint8			flags_;

	typedef			std::vector< DataTypePtr > Args;
	Args			args_;

	typedef			std::pair< std::string, DataTypePtr > ReturnValue;
	typedef			std::vector< ReturnValue > ReturnValues;
	ReturnValues	returnValues_;

	int				internalIndex_;			///< Used within the server
	int				exposedIndex_;			///< Used between client and server
	int				exposedSubIndex_;		///< Used to extend address space

	float priority_;

#if ENABLE_WATCHERS
	mutable TimeStamp timeSpent_;
	mutable uint64 timesCalled_;
#endif
	mutable EntityMemberStats stats_;
	// NOTE: If adding data, check the assignment operator.
};

#ifdef CODE_INLINE
#include "method_description.ipp"
#endif

#endif // METHOD_DESCRIPTION_HPP
