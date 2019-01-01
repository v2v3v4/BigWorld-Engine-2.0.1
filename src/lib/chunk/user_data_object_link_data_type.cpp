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

#include "user_data_object_link_data_type.hpp"

#include "cstdmf/binary_stream.hpp"
#include "cstdmf/guard.hpp"
#include "cstdmf/md5.hpp"

#include "entitydef/data_types.hpp"



int UserDataObjectLinkDataType_token = 1;


#ifndef EDITOR_ENABLED

///////////////////////////////////////////////////////////////////////////////
// Server and Client side UserDataObjectLinkDataType Class
///////////////////////////////////////////////////////////////////////////////

#include "user_data_object.hpp"


/**
 *	Constructor
 */
UserDataObjectLinkDataType::UserDataObjectLinkDataType( MetaDataType * pMeta ):
	DataType( pMeta, /*isConst:*/false )
{
}


/**
 *	Overrides the DataType method.
 *
 *	@see DataType::isSameType
 */
bool UserDataObjectLinkDataType::isSameType( PyObject* pValue )
{
	return UserDataObject::Check( pValue ) || pValue == Py_None;
}


/**
 *	This method sets the default value for this type.
 *
 *	@see DataType::setDefaultValue
 */
void UserDataObjectLinkDataType::setDefaultValue( DataSectionPtr pSection )
{
	pDefaultValue_ = NULL;
}


/**
 *	Overrides the DataType method.
 *
 *	@see DataType::pDefaultValue
 */
PyObjectPtr UserDataObjectLinkDataType::pDefaultValue() const
{
	Py_Return;
}


/**
 *	Overrides the DataType method.
 *
 *	@see DataType::addToStream
 */
bool UserDataObjectLinkDataType::addToStream( PyObject * pValue,
		BinaryOStream & stream, bool /*isPersistentOnly*/ ) const
{
	BW_GUARD;
	if ( !UserDataObject::Check( pValue ) )
	{
		if (pValue != Py_None)
		{
			ERROR_MSG(
				"UserDataObjectLinkDataType::addToStream: type is not"
				" a UserDataObject.\n" );
			return false;
		}
		stream << UniqueID::zero();
		return true;
	}

	UserDataObject* udo = static_cast<UserDataObject*>( pValue );
	stream << udo->guid();

	return true;
}


/**
 *	Overrides the DataType method.
 *
 *	@see DataType::createFromStream
 */
PyObjectPtr UserDataObjectLinkDataType::createFromStream(
	BinaryIStream & stream, bool /*isPersistentOnly*/ ) const
{
	BW_GUARD;
	UniqueID guid;
	stream >> guid;

	if (stream.error())
	{
		ERROR_MSG( "UserDataObjectLinkDataType::createFromStream: "
				   "Not enough data on stream to read value\n" );
		Py_Return;
	}

	if (guid == UniqueID::zero())
	{
		Py_Return;
	}

	PyObjectPtr udo( UserDataObject::createRef( guid ), PyObjectPtr::STEAL_REFERENCE );

	if ( udo == NULL )
		Py_Return;

	return udo;
}


/**
 *	Overrides the DataType method.
 *
 *	@see DataType::addToSection
 */
bool UserDataObjectLinkDataType::addToSection( PyObject * pValue,
		DataSectionPtr pSection ) const
{
	BW_GUARD;
	MF_ASSERT_DEV( UserDataObject::Check( pValue ) || pValue == Py_None )

	if (UserDataObject::Check( pValue ))
	{
		UserDataObject* udo = static_cast<UserDataObject*>( pValue );
		pSection->writeString( "guid", udo->guid() );
	}

	return true;
}


/**
 *	Overrides the DataType method.
 *
 *	@see DataType::createFromSection
 */
PyObjectPtr UserDataObjectLinkDataType::createFromSection( DataSectionPtr pSection ) const
{
	BW_GUARD;
	std::string guid = pSection->readString( "guid", "" );

	PyObjectPtr udo( UserDataObject::createRef( guid ), PyObjectPtr::STEAL_REFERENCE );

	if ( udo == NULL )
		Py_Return;

	return udo;
}


void UserDataObjectLinkDataType::addToMD5( MD5 & md5 ) const
{
	const char* md5String = "UserDataObjectLinkDataType";
	// TODO: Fix this when the login version is next bumped to include the
	// entire string, not just the first 4 bytes.
	md5.append( md5String, 4 );
}


bool UserDataObjectLinkDataType::operator<( const DataType & other ) const
{
	if (this->DataType::operator<( other )) return true;
	if (other.DataType::operator<( *this )) return false;

	const UserDataObjectLinkDataType& otherStr =
		static_cast< const UserDataObjectLinkDataType& >( other );
	return (Script::compare( pDefaultValue_.getObject(),
		otherStr.pDefaultValue_.getObject() ) < 0);
}


#else // EDITOR_ENABLED


///////////////////////////////////////////////////////////////////////////////
// Editor side UserDataObjectLinkDataType Class
///////////////////////////////////////////////////////////////////////////////

UserDataObjectLinkDataType::UserDataObjectLinkDataType( MetaDataType * pMeta ) :
	DataType( pMeta, /*isConst:*/false ),
	defaultId_( "" ),
	defaultChunkId_( "" )
{
}


/**
 *	This method returns a string representation of the data type.
 *
 *	@param pValue	Python object containing a UserDataObjectLinkDataType.
 *	@return			String representation of the data type.
 */
/*static*/ std::string UserDataObjectLinkDataType::asString( PyObject* pValue )
{
	BW_GUARD;
	if ( !PyTuple_Check( pValue ) || PyTuple_Size( pValue ) != 2 )
		return "( , )";

	return std::string( "( " ) +
		PyString_AsString( PyTuple_GetItem( pValue, 0 ) ) + ", " +
		PyString_AsString( PyTuple_GetItem( pValue, 1 ) ) + " )";
}


/**
 *	Overrides the DataType method.
 *
 *	@see DataType::isSameType
 */
bool UserDataObjectLinkDataType::isSameType( PyObject* pValue )
{
	BW_GUARD;
	if (!PyTuple_Check( pValue ) || PyTuple_Size( pValue ) != 2)
		return false;

	std::string id;
	std::string chunkId;

	if (Script::setData( PyTuple_GetItem( pValue, 0 ), id, "user data object ID" ) == -1 ||
		Script::setData( PyTuple_GetItem( pValue, 1 ), chunkId, "chunk ID" ) == -1)
	{
		PyErr_Clear();
		return false;
	}

	return true;
}


/**
 *	This method sets the default value for this type.
 *
 *	@see DataType::setDefaultValue
 */
void UserDataObjectLinkDataType::setDefaultValue( DataSectionPtr pSection )
{
	BW_GUARD;
	if (pSection)
	{
		defaultId_ = pSection->readString( "guid", "" );
		defaultChunkId_ = pSection->readString( "chunkId", "" );
	}
	else
	{
		defaultId_ = "";
		defaultChunkId_ = "";
	}
}


/**
 *	Overrides the DataType method.
 *
 *	@see DataType::pDefaultValue
 */
PyObjectPtr UserDataObjectLinkDataType::pDefaultValue() const
{
	BW_GUARD;
	PyObject * pTuple = PyTuple_New( 2 );
	PyTuple_SET_ITEM( pTuple, 0, Script::getData( defaultId_ ) );
	PyTuple_SET_ITEM( pTuple, 1, Script::getData( defaultChunkId_ ) );

	return PyObjectPtr( pTuple, PyObjectPtr::STEAL_REFERENCE );
}


/**
 *	Overrides the DataType method.
 *
 *	@see DataType::addToStream
 */
bool UserDataObjectLinkDataType::addToStream( PyObject * pValue,
		BinaryOStream & stream, bool /*isPersistentOnly*/ ) const
{
	BW_GUARD;
	if (!PyTuple_Check( pValue ) || PyTuple_Size( pValue ) != 2)
	{
		CRITICAL_MSG(
			"UserDataObjectLinkDataType::addToStream: type is not"
			" a tuple of two (id, chunkId)\n" );
		return false;
	}

	std::string id;
	std::string chunkID;

	if (Script::setData( PyTuple_GetItem( pValue, 0 ), 
				id, "user data object ID" ) == -1 ||
		Script::setData( PyTuple_GetItem( pValue, 1 ), 
				chunkID, "chunk ID" ) == -1)
	{
		CRITICAL_MSG(
			"UserDataObjectLinkDataType::addToStream: elements were not"
			" strings after passing isSameType\n" );
		PyErr_Clear();
		return false;
	}
	stream << id;
	stream << chunkID;

	return true;
}


/**
 *	Overrides the DataType method.
 *
 *	@see DataType::createFromStream
 */
PyObjectPtr UserDataObjectLinkDataType::createFromStream( BinaryIStream & stream,
		bool /*isPersistentOnly*/ ) const
{
	BW_GUARD;
	std::string id;
	std::string chunkId;
	stream >> id;
	stream >> chunkId;

	if (stream.error())
	{
		ERROR_MSG( "UserDataObjectLinkDataType::createFromStream: "
				   "Not enough data on stream to read value\n" );
		return NULL;
	}

	PyObject * pTuple = PyTuple_New( 2 );
	PyTuple_SET_ITEM( pTuple, 0, Script::getData( id ) );
	PyTuple_SET_ITEM( pTuple, 1, Script::getData( chunkId ) );

	return PyObjectPtr( pTuple, PyObjectPtr::STEAL_REFERENCE );
}


/**
 *	Overrides the DataType method.
 *
 *	@see DataType::addToSection
 */
bool UserDataObjectLinkDataType::addToSection( PyObject * pValue,
		DataSectionPtr pSection ) const
{
	BW_GUARD;
	if (!PyTuple_Check( pValue ) || PyTuple_Size( pValue ) != 2)
	{
		CRITICAL_MSG(
			"UserDataObjectLinkDataType::addToSection: type was not"
			" a tuple of two (id, chunkId)\n" );
		return false;
	}

	std::string id;
	std::string chunkId;

	if ((Script::setData( PyTuple_GetItem( pValue, 0 ), 
				id, "user data object ID" ) != 0) ||
		(Script::setData( PyTuple_GetItem( pValue, 1 ), 
				chunkId, "chunk ID" ) != 0))
	{
		CRITICAL_MSG(
			"UserDataObjectLinkDataType::addToSection: tuple was not"
			" strings after passing isSameType\n" );
		PyErr_Clear();
		return false;
	}
	pSection->writeString( "guid", id );
	pSection->writeString( "chunkId", chunkId );
	return true;
}


/**
 *	Overrides the DataType method.
 *
 *	@see DataType::createFromSection
 */
PyObjectPtr UserDataObjectLinkDataType::createFromSection( DataSectionPtr pSection ) const
{
	BW_GUARD;
	std::string id = pSection->readString( "guid", "" );
	std::string chunkId = pSection->readString( "chunkId", "" );

	PyObject * pTuple = PyTuple_New( 2 );
	PyTuple_SET_ITEM( pTuple, 0, Script::getData( id ) );
	PyTuple_SET_ITEM( pTuple, 1, Script::getData( chunkId ) );

	return PyObjectPtr( pTuple, PyObjectPtr::STEAL_REFERENCE );
}


void UserDataObjectLinkDataType::addToMD5( MD5 & md5 ) const
{
	BW_GUARD;
	md5.append( "UserDataObjectLinkDataType", sizeof( "UserDataObjectLinkDataType" ) );
}


bool UserDataObjectLinkDataType::operator<( const DataType & other ) const
{
	BW_GUARD;
	if (this->DataType::operator<( other )) return true;
	if (other.DataType::operator<( *this )) return false;

	const UserDataObjectLinkDataType& otherVec =
		static_cast< const UserDataObjectLinkDataType& >( other );
	return
		defaultId_ < otherVec.defaultId_ &&
		defaultChunkId_ < otherVec.defaultChunkId_;
}

#endif // EDITOR_ENABLED


// static class to implement the data type
class UserDataObjectDataType : public UserDataObjectLinkDataType
{
public:
	UserDataObjectDataType(MetaDataType * pMetaType ) : UserDataObjectLinkDataType( pMetaType ) {}
};
static SimpleMetaDataType< UserDataObjectDataType > userDataObjectDataType("UDO_REF");
