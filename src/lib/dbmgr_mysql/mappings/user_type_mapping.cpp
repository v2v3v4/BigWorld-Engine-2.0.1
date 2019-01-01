/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "user_type_mapping.hpp"

#include "../py_user_type_binder.hpp"

#include "user_type_mapping.hpp"

#include "cstdmf/binary_stream.hpp"
#include "cstdmf/memory_stream.hpp"

#include "entitydef/data_types.hpp"

/**
 *	Constructor.
 */
UserTypeMapping::UserTypeMapping( const std::string & propName ) :
	CompositePropertyMapping( propName )
{
}


/*
 *	Override from PropertyMapping.
 */
void UserTypeMapping::fromStreamToDatabase( StreamToQueryHelper & helper,
			BinaryIStream & strm,
			QueryRunner & queryRunner ) const
{
	std::string encStr;
	strm >> encStr;

	if (strm.error())
	{
		ERROR_MSG( "UserTypeMapping::fromStreamToDatabase: "
					"Failed destreaming stage 1 for property '%s'.\n",
				this->propName().c_str() );
		return;
	}

	// TODO: Remove the size and just use stream
	// Only really doing this to remove the prefixed size
	MemoryIStream encStrm( const_cast<char *>( encStr.c_str() ),
		encStr.length() );
	this->CompositePropertyMapping::fromStreamToDatabase( helper,
			encStrm, queryRunner );

	if (encStrm.error())
	{
		ERROR_MSG( "UserTypeMapping::fromStreamToDatabase: "
					"Failed destreaming stage 2 for property '%s'.\n",
				this->propName().c_str() );
		strm.error( true );
	}
}


/*
 *	Override from PropertyMapping.
 */
void UserTypeMapping::fromDatabaseToStream( ResultToStreamHelper & helper,
			ResultStream & results,
			BinaryOStream & strm ) const
{
	MemoryOStream localStream;
	this->CompositePropertyMapping::fromDatabaseToStream( helper, results,
			localStream );

	strm.appendString( static_cast< const char * >( localStream.data() ),
			localStream.size() );
}


/**
 *	This static method creates a UserTypeMapping for a USER_TYPE property.
 *
 *	@param namer          The helper class to use for constructing DB names.
 *	@param propName       The property name of the USER_TYPE to create.
 *	@param type           The UserDataType class which contains information on
 *	                      how to construct and manipulate a new instance.
 *	@param pDefaultValue  The default value to use when creating a new instance.
 *
 *	@returns The new CompositePropertyMapping on success, an empty PropertyMapping on failure.
 */
PropertyMappingPtr UserTypeMapping::create( const Namer & namer,
		const std::string & propName, const UserDataType & type,
		DataSectionPtr pDefaultValue )
{
	const std::string & moduleName = type.moduleName();
	const std::string & instanceName = type.instanceName();

	// get the module
	PyObjectPtr pModule(
			PyImport_ImportModule(
				const_cast< char * >( moduleName.c_str() ) ),
			PyObjectPtr::STEAL_REFERENCE );

	if (!pModule)
	{
		ERROR_MSG( "UserDataType::create: unable to import %s from %s\n",
				instanceName.c_str(), moduleName.c_str() );
		PyErr_Print();
		return PropertyMappingPtr();
	}

	// get the object
	PyObjectPtr pObject(
			PyObject_GetAttrString( pModule.getObject(),
				const_cast<char *>( instanceName.c_str() ) ) );

	if (!pObject)
	{
		ERROR_MSG( "UserDataType::create: unable to import %s from %s\n",
				instanceName.c_str(), moduleName.c_str() );
		PyErr_Print();
		return PropertyMappingPtr();
	}
	else
	{
		Py_DECREF( pObject.getObject() );
	}

	// create our binder class
	PyUserTypeBinderPtr pBinder(
		new PyUserTypeBinder( namer, propName, pDefaultValue )
							  , true );

	// call the method
	PyObjectPtr pResult(
			PyObject_CallMethod( pObject.getObject(), "bindSectionToDB",
				"O", pBinder.getObject() ) );

	if (!pResult)
	{
		ERROR_MSG( "UserDataType::create: (%s.%s) bindSectionToDB failed\n",
				moduleName.c_str(), instanceName.c_str() );
		PyErr_Print();
		return PropertyMappingPtr();
	}

	// pull out the result
	PropertyMappingPtr pTypeMapping = pBinder->getResult();

	if (!pTypeMapping.exists())
	{
		ERROR_MSG( "UserDataType::create: (%s.%s) bindSectionToDB missing "
					"endTable\n", moduleName.c_str(), instanceName.c_str() );
	}

	return pTypeMapping;
}

// user_type_mapping.cpp
