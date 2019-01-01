/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef USER_DATA_OBJECT_TYPE_HPP
#define USER_DATA_OBJECT_TYPE_HPP

#include <Python.h>
#include <string>
#include <map>
#include "entitydef/user_data_object_description.hpp"

#include "cstdmf/unique_id.hpp"


class UserDataObject;
class BinaryIStream;
class BinaryOStream;

class UserDataObjectType;
typedef SmartPointer<UserDataObjectType> UserDataObjectTypePtr;

typedef std::map<const std::string, UserDataObjectTypePtr> UserDataObjectTypes;
struct UserDataObjectInitData;

#define USER_DATA_OBJECT_ATTR_STR "userDataObjects"

/**
 *	This class is used to represent an user data object type.
 */
class UserDataObjectType : public SafeReferenceCount
{
public:
	UserDataObjectType( const UserDataObjectDescription& description,
			PyTypeObject * pType );

	~UserDataObjectType();

	UserDataObject * newUserDataObject( const UniqueID & guid ) const;

	bool hasProperty( const char * attr ) const;

	//DataDescription * propIndex( int index ) const	{ return propDescs_[index];}
	//int propCount( ) const	{ return propDescs_.size();}
	PyTypeObject * pPyType() const 	{ return pPyType_; }
	void setPyType( PyTypeObject * pPyType );

	const char * name() const;

	/// @name static methods
	//@{
	static bool init();
	static bool load( UserDataObjectTypes& types );	
	static void migrate( UserDataObjectTypes& types );

	static void clearStatics();

	static UserDataObjectTypePtr getType( const char * className );

	static UserDataObjectTypes& getTypes();
	//@}
	const UserDataObjectDescription& description()  const { return description_; };


private:
	UserDataObjectDescription description_;
	PyTypeObject * pPyType_;

	std::vector<DataDescription*>	propDescs_;

	static UserDataObjectTypes s_curTypes_;
};

typedef SmartPointer<UserDataObjectType> UserDataObjectTypePtr;


#ifdef CODE_INLINE
#include "user_data_object_type.ipp"
#endif

#endif // USER_DATA_OBJECT_TYPE_HPP

//user_data_object_type.hpp
