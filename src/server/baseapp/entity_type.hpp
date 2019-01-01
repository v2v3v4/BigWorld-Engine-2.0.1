/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_TYPE_HPP
#define ENTITY_TYPE_HPP

#include "Python.h"

#include "network/basictypes.hpp"
#include "entitydef/entity_description.hpp"
#include "entitydef/method_description.hpp"

class Base;


class EntityType;
typedef SmartPointer< EntityType > EntityTypePtr;
typedef SmartPointer<Base> BasePtr;

/**
 *	This class is the entity type of a base
 */
class EntityType : public ReferenceCount
{
public:
	EntityType( const EntityDescription & desc, PyTypeObject * pType,
		   bool isProxy );
	~EntityType();

	BasePtr create( EntityID id, DatabaseID dbID, BinaryIStream & data,
		bool hasPersistentDataOnly );

	Base * newEntityBase( EntityID id, DatabaseID dbID );

	PyObject * createScript( BinaryIStream & data );
	PyObjectPtr createCellDict( BinaryIStream & data,
								bool strmHasPersistentDataOnly );

	const EntityDescription & description() const
											{ return entityDescription_; }

	static EntityTypePtr getType( EntityTypeID typeID );
	static EntityTypePtr getType( const char * className );
	static EntityTypeID nameToIndex( const char * name );

	static bool init( bool isReload = false );
	static bool reloadScript( bool isRecover = false );
	static void migrate( bool isFullReload = true );
	static void cleanupAfterReload( bool isFullReload = true );

	static void clearStatics();

#if ENABLE_WATCHERS
	static WatcherPtr pWatcher();
	// Functions to collect statistical information
	void updateBackupSize(const Base & instance, uint32 newSize);
	void updateDbSize(const Base & instance, uint32 newSize);
	void countNewInstance(const Base & instance);
	void forgetOldInstance(const Base & instance);

	uint32 averageBackupSize() const;
	uint32 averageDbSize() const;
	uint32 totalBackupSize() const;
	uint32 totalDbSize() const;
#endif
	const char * name() const	{ return entityDescription_.name().c_str(); }

	bool hasBaseScript() const	{ return entityDescription_.hasBaseScript(); }
	bool hasCellScript() const	{ return entityDescription_.hasCellScript(); }

	bool isProxy() const		{ return isProxy_; }

	PyObject * pClass() const		{ return (PyObject*)pClass_; }
	void setClass( PyTypeObject * pClass );

	EntityTypeID id() const	{ return entityDescription_.index(); }

	EntityTypePtr old() const	{ return pOldSelf_; }
	void old( EntityTypePtr pOldType );

	static MD5 s_persistentPropertiesMD5_;

private:
	EntityDescription entityDescription_;
	PyTypeObject *	pClass_;

	EntityTypePtr	pOldSelf_;

	bool			isProxy_;

#if ENABLE_WATCHERS
	// statistics
	uint32 backupSize_;
	uint32 dbSize_;
	uint32 totalDerivedWithKnownSize_;
	uint32 totalDerived_;
#endif

	// static stuff
	static PyObject * s_pInitModules_, * s_pNewModules_;

	typedef std::vector< EntityTypePtr > EntityTypes;
	static EntityTypes s_curTypes_, s_newTypes_;

	typedef StringHashMap< EntityTypeID > NameToIndexMap;
	static NameToIndexMap s_nameToIndexMap_;
};

#endif // ENTITY_TYPE_HPP
