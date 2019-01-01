/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "Python.h"

#include "cstdmf/debug.hpp"
#include "cstdmf/guard.hpp"
#include "cstdmf/watcher.hpp"

#include "entitydef/property_change.hpp"
#include "entitydef/property_owner.hpp"

#include "simple_client_entity.hpp"

DECLARE_DEBUG_COMPONENT2( "Connect", 0 )


namespace SimpleClientEntity
{
bool g_verbose = false;

class StaticWatcherInitialiser
{
public:
	StaticWatcherInitialiser()
	{
		BW_GUARD;
		MF_WATCH( "Debug/dumpEntityMessages", g_verbose,
			Watcher::WT_READ_WRITE,
			"If true, all entity property and method messages from the server "
				"are printed to the Debug console." );
	}
};
StaticWatcherInitialiser s_initialiser;


/**
 *	Helper class to cast entity as a property owner
 */
class EntityPropertyOwner : public TopLevelPropertyOwner
{
public:
	EntityPropertyOwner( PyObjectPtr e, const EntityDescription & edesc ) :
		e_( e ), edesc_( edesc ) { }

	// called going to the root of the tree
	virtual void onOwnedPropertyChanged( PropertyChange & change )
	{
		BW_GUARD;
		// unimplemented
	}

	virtual bool getTopLevelOwner( PropertyChange & change,
			PropertyOwnerBase *& rpTopLevelOwner )
	{
		BW_GUARD;
		return true;
	}

	// called going to the leaves of the tree
	virtual int getNumOwnedProperties() const
	{
		BW_GUARD;
		return edesc_.clientServerPropertyCount();
	}

	virtual PropertyOwnerBase * getChildPropertyOwner( int ref ) const
	{
		BW_GUARD;
		DataDescription * pDD = edesc_.clientServerProperty( ref );
		PyObjectPtr pPyObj(
			PyObject_GetAttrString( e_.get(), (char*)pDD->name().c_str() ),
			PyObjectPtr::STEAL_REFERENCE );
		if (!pPyObj)
		{
			PyErr_Clear();
			return NULL;
		}

		return pDD->dataType()->asOwner( pPyObj.get() );
	}

	virtual PyObjectPtr setOwnedProperty( int ref, BinaryIStream & data )
	{
		BW_GUARD;
		DataDescription * pDD = edesc_.clientServerProperty( ref );
		if (pDD == NULL) return NULL;

		PyObjectPtr pNewObj = pDD->createFromStream( data, false );
		if (!pNewObj)
		{
			ERROR_MSG( "Entity::handleProperty: "
				"Error streaming off new property value\n" );
			return NULL;
		}

		PyObjectPtr pOldObj(
			PyObject_GetAttrString( e_.get(), (char*)pDD->name().c_str() ),
			PyObjectPtr::STEAL_REFERENCE );
		if (!pOldObj)
		{
			PyErr_Clear();
			pOldObj = Py_None;
		}

		int err = PyObject_SetAttrString(
			e_.get(), (char*)pDD->name().c_str(), pNewObj.get() );
		if (err == -1)
		{
			ERROR_MSG( "Entity::handleProperty: "
				"Failed to set new property into Entity\n" );
			PyErr_PrintEx(0);
		}

		return pOldObj;
	}

	virtual bool SetOwnedSlice( int startIndex, int endIndex,
			BinaryIStream & data )
	{
		// TODO: Implement this
		MF_ASSERT( !"Not implemented" );
		return true;
	}

private:
	PyObjectPtr e_;
	const EntityDescription & edesc_;
};


/**
 *	Update the identified property on the given entity. Returns true if
 *	the property was found to update.
 */
bool propertyEvent( PyObjectPtr pEntity, const EntityDescription & edesc,
	int messageID, BinaryIStream & data, bool callSetForTopLevel )
{
	BW_GUARD;
	EntityPropertyOwner king( pEntity, edesc );

	PyObjectPtr * ppOldValue = NULL;
	PyObjectPtr * ppChangePath = NULL;

	PyObjectPtr pOldValue = Py_None;
	PyObjectPtr pChangePath = NULL;

	if (callSetForTopLevel)
	{
		ppOldValue = &pOldValue;
		ppChangePath = &pChangePath;
	}

	int topLevelIndex = king.setPropertyFromExternalStream( data, messageID,
			ppOldValue, ppChangePath );

	// if this was a top-level property then call the set handler for it
	// TODO: Restore functionality prior to hierarchical property.
    // need to improve to inform script which element in a complex
	// property has been updated.
	if (callSetForTopLevel)
	{
		DataDescription * pDataDescription =
			edesc.clientServerProperty( topLevelIndex );
		MF_ASSERT_DEV( pDataDescription != NULL );

		bool isDone = false;

		if (pChangePath)
		{
			bool isSlice = (messageID == PROPERTY_CHANGE_ID_SLICE);

			std::string methodName = (isSlice ? "setSlice_" : "setNested_") +
				pDataDescription->name();

			PyObject * pFunc = PyObject_GetAttrString( pEntity.get(),
					(char*)methodName.c_str() );

			isDone = (pFunc != NULL);

			Script::call( pFunc,
				PyTuple_Pack( 2, pChangePath.get(), pOldValue.get() ),
				"Entity::propertyEvent: ",
				/*okIfFunctionNull:*/true );
		}

		if (!isDone)
		{
			std::string methodName = "set_" + pDataDescription->name();
			Script::call(
				PyObject_GetAttrString( pEntity.get(),
					(char*)methodName.c_str() ),
				PyTuple_Pack( 1, pOldValue.get() ),
				"Entity::propertyEvent: ",
				/*okIfFunctionNull:*/true );
		}
	}

	return true;
}

/**
 *	Call the identified method on the given entity. Returns true if the
 *	method description was found.
 */
bool methodEvent( PyObjectPtr pEntity, const EntityDescription & edesc,
	int messageID, BinaryIStream & data )
{
	BW_GUARD;
	MethodDescription * pMethodDescription =
		edesc.clientMethod( messageID, data );

	if (pMethodDescription == NULL)
	{
		ERROR_MSG( "SimpleClientEntity::methodEvent: "
			"No method starting with message id %d\n", messageID );
		return false;
	}

	if (g_verbose)
	{
		DEBUG_MSG( "SimpleClientEntity::methodEvent: %s.%s - %d bytes\n",
			edesc.name().c_str(),
			pMethodDescription->name().c_str(),
			data.remainingLength() );
	}

	pMethodDescription->callMethod( pEntity.get(), data );
	return true;
}

} // namespace SimpleClientEntity

// simple_client_entity.cpp
