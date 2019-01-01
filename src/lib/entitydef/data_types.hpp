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
 * 	@file data_types.hpp
 *
 *	Concrete data types as referred to by DataDescriptions
 *
 *	@ingroup entity
 */

#ifndef DATA_TYPES_HPP
#define DATA_TYPES_HPP

#include "data_description.hpp"

/**
 *	This class is a simple meta data type that holds a single DataType
 *	object and does no parsing of the contents of its &lt;Type> sections.
 */
template <class DATATYPE>
class SimpleMetaDataType : public MetaDataType
{
public:
	SimpleMetaDataType( const char * name ) :
		name_( name )
	{
		MetaDataType::addMetaType( this );
	}
	virtual ~SimpleMetaDataType()
	{
		MetaDataType::delMetaType( this );
	}

	virtual const char * name() const
	{
		return name_.c_str();
	}

	virtual DataTypePtr getType( DataSectionPtr pSection )
	{
		return new DATATYPE( this );
	}

private:
	std::string name_;
};


/**
 *	A simple macro used to register simple data types. That is, types that do
 *	not have a meta data type.
 */
#define SIMPLE_DATA_TYPE( TYPE, NAME )					\
	SimpleMetaDataType< TYPE > s_##NAME##_metaDataType( #NAME );	\



/**
 *	This template class is used to represent the string data type.
 *
 *	@ingroup entity
 */
class StringDataType : public DataType
{
public:
	StringDataType( MetaDataType * pMeta ) : DataType( pMeta ) { }

protected:
	virtual bool isSameType( PyObject * pValue );
	virtual void setDefaultValue( DataSectionPtr pSection );
	virtual PyObjectPtr pDefaultValue() const;

	virtual bool addToStream( PyObject * pNewValue,
			BinaryOStream & stream, bool isPersistentOnly ) const;
	virtual PyObjectPtr createFromStream( BinaryIStream & stream,
			bool isPersistentOnly ) const;

	virtual bool addToSection( PyObject * pNewValue, DataSectionPtr pSection )
			const;
	virtual PyObjectPtr createFromSection( DataSectionPtr pSection ) const;

	virtual bool fromStreamToSection( BinaryIStream & stream,
			DataSectionPtr pSection, bool isPersistentOnly ) const;
	virtual bool fromSectionToStream( DataSectionPtr pSection,
			BinaryOStream & stream, bool isPersistentOnly ) const;

	virtual void addToMD5( MD5 & md5 ) const;

	virtual bool operator<( const DataType & other ) const;

private:
	PyObjectPtr pDefaultValue_;
};


/**
 *	This template class is used to represent the Unicode string data type.
 *
 *	@ingroup entity
 */
class UnicodeStringDataType : public DataType
{
public:
	UnicodeStringDataType( MetaDataType * pMeta );
	virtual ~UnicodeStringDataType();

protected:
	virtual bool isSameType( PyObject * pValue );
	virtual void setDefaultValue( DataSectionPtr pSection );
	virtual PyObjectPtr pDefaultValue() const;

	virtual bool addToStream( PyObject * pNewValue,
			BinaryOStream & stream, bool isPersistentOnly ) const;
	virtual PyObjectPtr createFromStream( BinaryIStream & stream,
			bool isPersistentOnly ) const;

	virtual bool addToSection( PyObject * pNewValue, DataSectionPtr pSection )
			const;
	virtual PyObjectPtr createFromSection( DataSectionPtr pSection ) const;

	virtual bool fromStreamToSection( BinaryIStream & stream,
			DataSectionPtr pSection, bool isPersistentOnly ) const;
	virtual bool fromSectionToStream( DataSectionPtr pSection,
			BinaryOStream & stream, bool isPersistentOnly ) const;

	virtual void addToMD5( MD5 & md5 ) const;

	virtual bool operator<( const DataType & other ) const;

private:
	PyObjectPtr pDefaultValue_;
};


/**
 *	This base class is used to represent sequence data type. This includes
 *	tuples and lists.
 */
class SequenceDataType : public DataType
{
	public:
		SequenceDataType( MetaDataType * pMeta,
				DataTypePtr elementTypePtr,
				int size,
				int dbLen,
				bool isConst ) :
			DataType( pMeta, isConst && elementTypePtr->isConst() ),
			elementTypePtr_( elementTypePtr ),
			elementType_( *elementTypePtr ),
			size_( size ),
			dbLen_( dbLen )
		{
			// note: our meta data type will not always be SequenceMetaDataType,
			// it could be e.g. SimpleMetaDataType used by PatrolPath.
		}

		int getSize() const
		{
			return size_;
		}

		int dbLen() const
		{
			return dbLen_;
		}

		DataType& getElemType() const
		{
			return elementType_;
		}

	protected:
		virtual PyObjectPtr newSequence( int size ) const = 0;
		// Note: The derived method should steal a reference to pElement.
		virtual void setItem( PyObject * pSeq,
			int i, PyObjectPtr pElement ) const = 0;
		virtual int compareDefaultValue( const DataType & other ) const = 0;

		PyObjectPtr createDefaultValue() const;

		// Overrides the DataType method.
		virtual bool isSameType( PyObject * pValue );
		virtual bool addToStream( PyObject * pNewValue,
			BinaryOStream & stream, bool isPersistentOnly ) const;
		virtual PyObjectPtr createFromStream( BinaryIStream & stream,
			bool isPersistentOnly ) const;
		virtual bool addToSection( PyObject * pNewValue,
			DataSectionPtr pSection ) const;
		virtual PyObjectPtr createFromSection( DataSectionPtr pSection ) const;
		virtual bool fromStreamToSection( BinaryIStream & stream,
				DataSectionPtr pSection, bool isPersistentOnly ) const;
		virtual bool fromSectionToStream( DataSectionPtr pSection,
				BinaryOStream & stream, bool isPersistentOnly ) const;
		virtual void addToMD5( MD5 & md5 ) const;
		virtual bool operator<( const DataType & other ) const;
		virtual std::string typeName() const;

	private:
		void addToStreamInternal( PyObject * pNewValue,
			BinaryOStream & stream, bool isPersistentOnly ) const;
		PyObjectPtr createFromStreamInternal( BinaryIStream & stream,
			bool isPersistentOnly ) const;
		bool fromStreamToSectionInternal( BinaryIStream & stream,
			DataSectionPtr pSection, bool isPersistentOnly ) const;
		bool fromSectionToStreamInternal( DataSectionPtr pSection,
			BinaryOStream & stream, bool isPersistentOnly ) const;

		DataTypePtr elementTypePtr_;
		DataType & elementType_;
		int size_;
		int dbLen_;
};

/**
 *	This data type contains named properties like an entity class does.
 */
class ClassDataType : public DataType
{
public:

	/**
	 *	This structure is used to represent a field in a class.
	 */
	struct Field
	{
		std::string	name_;
		DataTypePtr	type_;
		int			dbLen_;
		bool		isPersistent_;
	};
	typedef std::vector<Field>	Fields;
	typedef Fields::const_iterator Fields_iterator;


	ClassDataType( MetaDataType * pMeta, Fields & fields, bool allowNone ) :
		DataType( pMeta, /*isConst:*/false ),
		allowNone_( allowNone )
	{
		fields_.swap( fields );

		for (Fields::iterator it = fields_.begin(); it != fields_.end(); ++it)
			fieldMap_[it->name_] = it - fields_.begin();
	}

	// Overrides the DataType method.
	virtual bool isSameType( PyObject * pValue );
	virtual void setDefaultValue( DataSectionPtr pSection );
	virtual PyObjectPtr pDefaultValue() const;
	virtual DataSectionPtr pDefaultSection() const { return pDefaultSection_; }
	virtual bool addToStream( PyObject * pValue,
		BinaryOStream & stream, bool isPersistentOnly ) const;
	virtual PyObjectPtr createFromStream( BinaryIStream & stream,
		bool isPersistentOnly ) const;
	virtual bool addToSection( PyObject * pValue, DataSectionPtr pSection )
		const;
	virtual PyObjectPtr createFromSection( DataSectionPtr pSection ) const;
	virtual PyObjectPtr attach( PyObject * pObject,
		PropertyOwnerBase * pOwner, int ownerRef );
	virtual void detach( PyObject * pObject );
	virtual PropertyOwnerBase * asOwner( PyObject * pObject );
	virtual void addToMD5( MD5 & md5 ) const;
	virtual bool operator<( const DataType & other ) const;
	virtual std::string typeName() const;

public:

	const Fields & fields()	const	{ return fields_; }
	const Fields & getFields() const	{ return fields_; }	// same as above

	int fieldIndexFromName( const char * name ) const
	{
		FieldMap::const_iterator found = fieldMap_.find( name );
		if (found != fieldMap_.end()) return found->second;
		return -1;
	}

	bool allowNone() const 	{ return allowNone_; }

private:
	Fields		fields_;

	typedef std::map<std::string,int> FieldMap;
	FieldMap	fieldMap_;

	bool		allowNone_;

	DataSectionPtr	pDefaultSection_;
};

/**
 *	This class is used to represent the data type that is described and
 *	implemented in script.
 *
 *	An example:
 *	<code>
 *	import cPickle
 *
 *	class TestDataType:
 *		def addToStream( self, test ):
 *			return cPickle.dumps( test )
 *
 *		def addToSection( self, test, section ):
 *			for i in test.items():
 *				section.writeInt( i[0], i[1] )
 *
 *		def createFromStream( self, stream ):
 *			return cPickle.loads( stream )
 *
 *		def createFromSection( self, section ):
 *			result = {}
 *			for i in section.values():
 *				result[ i.name ] = i.asInt
 *			return result
 *
 *		def fromStreamToSection( self, stream, section ):	# optional
 *			o = self.createFromStream( stream )
 *			self.addToSection( o, section )
 *
 *		def fromSectionToStream( self, section):			# optional
 *			o = self.createFromSection( section )
 *			return self.addToStream( o )
 *
 *		def defaultValue( self ):
 *			return { "ONE" : 1, "TWO" : 2 }
 *
 *	instance = TestDataType()
 *	</code>
 *
 *	@ingroup entity
 */
class UserDataType : public DataType
{
	public:
		/**
		 *	Constructor.
		 *
		 *	@param pMeta			The meta data type.
		 *	@param moduleName		The name of the module to find the instance.
		 *	@param instanceName	The name of the instance that implements the
		 *							data type.
		 */
		UserDataType( MetaDataType * pMeta,
				const std::string & moduleName,
				const std::string & instanceName ) :
			DataType( pMeta, /*isConst:*/false ), // = do not reuse default
			moduleName_( moduleName ),
			instanceName_( instanceName ),
			isInited_( false )
		{
		}

		~UserDataType();

		const std::string& moduleName() const { return moduleName_; }
		const std::string& instanceName() const { return instanceName_; }

	protected:
		// Overrides the DataType method.
		virtual bool isSameType( PyObject * /*pValue*/ );
		virtual void reloadScript();
		virtual void clearScript();
		virtual void setDefaultValue( DataSectionPtr pSection );
		virtual PyObjectPtr pDefaultValue() const;
		virtual DataSectionPtr pDefaultSection() const { return pDefaultSection_; }
		virtual bool addToStream( PyObject * pNewValue,
				BinaryOStream & stream, bool isPersistentOnly ) const;
		virtual PyObjectPtr createFromStream( BinaryIStream & stream,
				bool isPersistentOnly ) const;
		virtual bool addToSection( PyObject * pNewValue,
				DataSectionPtr pSection ) const;
		virtual PyObjectPtr createFromSection( DataSectionPtr pSection ) const;
		virtual bool fromStreamToSection( BinaryIStream & stream,
				DataSectionPtr pSection, bool isPersistentOnly ) const;
		virtual bool fromSectionToStream( DataSectionPtr pSection,
				BinaryOStream & stream, bool isPersistentOnly ) const;
		virtual void addToMD5( MD5 & md5 ) const;
		virtual bool operator<( const DataType & other ) const;
		virtual std::string typeName() const;

	private:
		void init();

		PyObject * pObject() const
		{
			if (!isInited_)
			{
				const_cast< UserDataType * >( this )->init();
			}

			return pObject_.getObject();
		}

		PyObject * method( const char * name ) const;

		std::string moduleName_;
		std::string instanceName_;

		bool isInited_;
		PyObjectPtr pObject_;
		DataSectionPtr pDefaultSection_;
};

/**
 *	This class is a a combination of ClassDataType and UserDataType. It allows
 * 	the user to specify a dictionary-like property with fixed keys. It also
 * 	optionally allow the user to replace the dictionary with a custom Python
 * 	class, thereby providing functionality similar to UserDataType. If the user
 * 	chooses to replace the dictionary with their own class, they must provide
 * 	a class object which implements the following methods:
 * 		def getDictFromObj( self, obj )
 * 		def createObjFromDict( self, dict )
 * 		def	isSameType( self, obj )
 * 		def addToStream( self, test )			[Optional]
 * 		def createFromStream( self, stream )	[Optional]
 *
 * 	For example, code to implement a "real" dictionary (i.e. non-fixed keys):
 *	<code>
 *	import cPickle
 *
 *	class TestDataType:
 * 		def getDictFromObj( self, obj ):
 * 			list = []
 * 			for i in test.items():
 * 				item = { "key": i[0], "value": i[1] }
 * 				list.append( item )
 * 			return { "list": list }
 *
 * 		def createObjFromDict( self, dict ):
 * 			result = {}
 * 			list = dict["list"]
 * 			for i in list:
 * 				result[i["key"]] = i["value"]
 * 			return result
 *
 * 		def	isSameType( self, obj ):
 * 			return type(obj) is dict
 *
 *		def addToStream( self, test ):						# optional
 *			return cPickle.dumps( test )
 *
 *		def createFromStream( self, stream ):				# optional
 *			return cPickle.loads( stream )
 *
 *	instance = TestDataType()
 *	</code>
 *
 *	@ingroup entity
 */
class PyFixedDictDataInstance;

/**
 *	This class is used to describe FixedDict data types.
 */
class FixedDictDataType : public DataType
{
public:
	typedef ClassDataType::Field	Field;
	typedef ClassDataType::Fields 	Fields;

	FixedDictDataType( MetaDataType* pMeta, Fields& fields, bool allowNone ) :
		DataType( pMeta, /*isConst:*/false ), allowNone_( allowNone ),
		isCustomClassImplInited_( true )
	{
		fields_.swap( fields );

		for (Fields::const_iterator i = fields_.begin(); i != fields_.end(); ++i)
			fieldMap_[i->name_] = i - fields_.begin();
	}
	virtual ~FixedDictDataType();

	int getFieldIndex( const std::string& fieldName ) const
	{
		FieldMap::const_iterator found = fieldMap_.find( fieldName );
		return (found != fieldMap_.end()) ? found->second : -1;
	}

	DataType& getFieldDataType( int index )
	{
		return *(fields_[index].type_);
	}

	const std::string& getFieldName( int index ) const
	{
		return fields_[index].name_;
	}

	Fields::size_type getNumFields() const { return fields_.size(); }
	const Fields& getFields() const	{ return fields_; }

	bool allowNone() const 	{ return allowNone_; }

	void setCustomClassImplementor( const std::string& moduleName,
			const std::string& instanceName );

	// Overrides the DataType method.
	virtual bool isSameType( PyObject * pValue );
	virtual void reloadScript();
	virtual void clearScript();
	virtual void setDefaultValue( DataSectionPtr pSection );
	virtual PyObjectPtr pDefaultValue() const;
	virtual DataSectionPtr pDefaultSection() const { return pDefaultSection_; }
	virtual bool addToStream( PyObject * pNewValue,
			BinaryOStream & stream, bool isPersistentOnly ) const;
	virtual PyObjectPtr createFromStream( BinaryIStream & stream,
			bool isPersistentOnly ) const;
	virtual bool addToSection( PyObject * pNewValue,
			DataSectionPtr pSection ) const;
	virtual PyObjectPtr createFromSection( DataSectionPtr pSection ) const;
	virtual void addToMD5( MD5 & md5 ) const;
	virtual bool operator<( const DataType & other ) const;
	virtual std::string typeName() const;
	virtual PyObjectPtr attach( PyObject * pObject,
		PropertyOwnerBase * pOwner, int ownerRef );
	virtual void detach( PyObject * pObject );
	virtual PropertyOwnerBase * asOwner( PyObject * pObject );

private:

	typedef std::map<std::string,int> FieldMap;

	bool 		allowNone_;
	Fields		fields_;
	FieldMap	fieldMap_;

	std::string moduleName_;
	std::string instanceName_;

	DataSectionPtr	pDefaultSection_;

	bool			isCustomClassImplInited_;
	PyObjectPtr 	pImplementor_;
	PyObjectPtr 	pGetDictFromObjFn_;
	PyObjectPtr 	pCreateObjFromDictFn_;
	PyObjectPtr 	pIsSameTypeFn_;
	PyObjectPtr 	pAddToStreamFn_;
	PyObjectPtr 	pCreateFromStreamFn_;

	// Functions to handle PyFixedDictDataInstance
	PyObjectPtr createDefaultInstance() const;
	void addInstanceToStream( PyFixedDictDataInstance* pInst,
			BinaryOStream& stream, bool isPersistentOnly ) const;
	PyObjectPtr createInstanceFromStream( BinaryIStream& stream,
			bool isPersistentOnly ) const;
	bool addInstanceToSection( PyFixedDictDataInstance* pInst,
			DataSectionPtr pSection ) const;
	PyObjectPtr createInstanceFromSection( DataSectionPtr pSection ) const;
	PyObjectPtr createInstanceFromMappingObj(
			PyObject* pyMappingObj ) const;

	// Functions to handle custom class
	void initCustomClassImplOnDemand()
	{
		if (!isCustomClassImplInited_)
		{
			this->setCustomClassFunctions( false );
		}
	}
	void setCustomClassFunctions( bool ignoreFailure );
	bool hasCustomClass() const { return pImplementor_; }
	const std::string& moduleName() const { return moduleName_; }
	const std::string& instanceName() const { return instanceName_; }
	bool hasCustomIsSameType() const { return pIsSameTypeFn_; }
	bool hasCustomAddToStream() const { return pAddToStreamFn_; }
	bool hasCustomCreateFromStream() const { return pCreateFromStreamFn_; }
	PyObjectPtr createCustomClassFromInstance( PyFixedDictDataInstance* pInst )
			const;
	bool isSameTypeCustom( PyObject* pValue ) const;
	void addToStreamCustom( PyObject* pValue, BinaryOStream& stream,
		bool isPersistentOnly) const;
	PyObjectPtr createFromStreamCustom( BinaryIStream & stream,
			bool isPersistentOnly ) const;
	PyObject* getDictFromCustomClass( PyObject* pCustomClass ) const;
	PyObjectPtr createInstanceFromCustomClass( PyObject* pValue )
			const;
};

bool PythonDataType_IsExpression( const std::string& value );


#endif // DATA_TYPES_HPP

// data_types.hpp
