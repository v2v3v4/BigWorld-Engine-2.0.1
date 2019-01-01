/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef USER_DATA_OBJECT_LINK_DATA_TYPE_HPP
#define USER_DATA_OBJECT_LINK_DATA_TYPE_HPP


#include "entitydef/data_description.hpp"


class MetaDataType;


class UserDataObjectLinkDataType : public DataType
{
public:
	UserDataObjectLinkDataType( MetaDataType * pMeta );

#ifdef EDITOR_ENABLED
	static std::string asString( PyObject* pValue );

	// This method is implemented in a different C++ file in the editor, in the
	// bigbang folder, in editor_user_data_object_link_data_type.cpp.
	GeneralProperty * createEditorProperty( const std::string& name,
		ChunkItem* item, int editorPropertyId );
#endif //EDITOR_ENABLED

protected:
	virtual bool isSameType( PyObject* pValue );

	virtual void setDefaultValue( DataSectionPtr pSection );

	virtual PyObjectPtr pDefaultValue() const;

	virtual bool addToStream( PyObject * pValue,
			BinaryOStream & stream, bool /*isPersistentOnly*/ ) const;

	virtual PyObjectPtr createFromStream( BinaryIStream & stream,
			bool /*isPersistentOnly*/ ) const;

	virtual bool addToSection( PyObject * pValue,
			DataSectionPtr pSection ) const;

	virtual PyObjectPtr createFromSection( DataSectionPtr pSection ) const;

	virtual void addToMD5( MD5 & md5 ) const;

	virtual bool operator<( const DataType & other ) const;

private:

#ifdef EDITOR_ENABLED
	std::string defaultId_;
	std::string defaultChunkId_;
#else
	PyObjectPtr pDefaultValue_;
#endif
};


#endif // USER_DATA_OBJECT_LINK_DATA_TYPE_HPP
