/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DATA_TYPE_HPP
#define DATA_TYPE_HPP

#include <Python.h>

#include "cstdmf/smartpointer.hpp"

#include <map>
#include <set>
#include <string>

class BinaryOStream;
class BinaryIStream;
class DataSection;
class DataType;
class MD5;
class MetaDataType;
class PropertyOwnerBase;

typedef SmartPointer< DataSection > DataSectionPtr;
typedef SmartPointer< DataType > DataTypePtr;
typedef SmartPointer< PyObject > PyObjectPtr;

#ifdef EDITOR_ENABLED
class ChunkItem;
class GeneralProperty;
#endif

/**
 *	Objects derived from this class are used to describe a type of data that can
 *	be used in a data description. Note: When implementing the abstract methods
 *	of this class, in general data from Python can be trusted as it has been
 *	through the 'isSameType' check, but data from sections and streams cannot
 *	and must always be checked for errors.
 *
 *	@ingroup entity
 */
class DataType : public ReferenceCount
{
public:
	/**
	 *	Constructor.
	 */
	DataType( MetaDataType * pMetaDataType, bool isConst = true ) :
		pMetaDataType_( pMetaDataType ),
		isConst_( isConst )
	{
	}

	/**
	 *	Destructor.
	 */
	virtual ~DataType();

	/**
	 *	This method causes any stored script objects derived from user script
	 *	to be reloaded.
	 */
	virtual void reloadScript()
	{
	}

	/**
	 *	This method causes any stored script objects derived from user script
	 *	to be reloaded.
	 */
	virtual void clearScript()
	{
	}

	/**
	 *	This method sets the default value associated with this type. This
	 * 	value will be subsequently returned by the pDefaultValue() meth
	 */
	virtual void setDefaultValue( DataSectionPtr pSection ) = 0;

	/**
	 *	This method returns a new reference to the default value associated
	 *	with this data type. That is, the caller is responsible for
	 *	dereferencing it.
	 */
	virtual PyObjectPtr pDefaultValue() const = 0;

	/**
	 *	This method returns the default section for this type as defined
	 *	in alias.xml or the entity definition files.
	 */
	virtual DataSectionPtr pDefaultSection() const;

	/**
	 *	This method returns whether the input object is of this type.
	 */
	virtual bool isSameType( PyObject * pValue ) = 0;

	/**
	 *	This method adds the value of the appropriate type onto the input
	 *	bundle. The value can be assumed to have been created by this
	 *	class or to have (previously) passed the isSameType check.
	 * 	If isPersistentOnly is true, then the object should streamed only the
	 *  persistent parts of itself.
	 *
	 *	@param pValue	The value to add.
	 *	@param stream	The stream to add the value to.
	 *	@param isPersistentOnly	Indicates if only persistent data is being added.
	 *	@return true if succeeded, false otherwise. In the failure case, the
	 *	stream is generally clobbered and should be discarded.
	 */
	virtual bool addToStream( PyObject * pValue, BinaryOStream & stream,
		bool isPersistentOnly )	const = 0;

	/**
	 *	This method returns a new object created from the input bundle.
	 *	The caller is responsible for decrementing the reference of the
	 *	object.
	 *
	 *	@param stream		The stream to read from.
	 *  @param isPersistentOnly	If true, then the stream only contains
	 * 						persistent properties of the object.
	 *
	 *	@return			A <b>new reference</b> to an object created from the
	 *					stream.
	 */
	virtual PyObjectPtr createFromStream( BinaryIStream & stream,
		bool isPersistentOnly ) const = 0;


	/**
	 *	This method adds the value of the appropriate type into the input
	 *	data section. The value can be assumed to have been created by this
	 *	class or to have (previously) passed the isSameType check.
	 *
	 *	@param pValue 	The value to add.
	 *	@param pSection	The data section to add the value to.
	 *	@return true if succeeded, false otherwise. In the failure case, the
	 *	section is generally clobbered, and should be discarded.
	 */
	virtual bool addToSection( PyObject * pValue, DataSectionPtr pSection )
		const = 0;

	/**
	 *	This method returns a new object created from the given
	 *	DataSection.
	 *
	 *	@param pSection	The datasection to use
	 *
	 *	@return			A <b>new reference</b> to an object created from the
	 *					section.
	 */
	virtual PyObjectPtr createFromSection( DataSectionPtr pSection ) const = 0;

	// DEPRECATED
	virtual bool fromStreamToSection( BinaryIStream & stream,
			DataSectionPtr pSection, bool isPersistentOnly ) const;
	// DEPRECATED
	virtual bool fromSectionToStream( DataSectionPtr pSection,
			BinaryOStream & stream, bool isPersistentOnly ) const;


	virtual PyObjectPtr attach( PyObject * pObject,
		PropertyOwnerBase * pOwner, int ownerRef );
	virtual void detach( PyObject * pObject );

	virtual PropertyOwnerBase * asOwner( PyObject * pObject );


	/**
	 *	This method adds this object to the input MD5 object.
	 */
	virtual void addToMD5( MD5 & md5 ) const = 0;


	static DataTypePtr buildDataType( DataSectionPtr pSection );
	static DataTypePtr buildDataType( const std::string& typeName );

#ifdef EDITOR_ENABLED
	virtual GeneralProperty * createEditorProperty( const std::string& name,
		ChunkItem* chunkItem, int editorEntityPropertyId )
		{ return NULL; }

	static DataSectionPtr findAliasWidget( const std::string& name )
	{
		AliasWidgets::iterator i = s_aliasWidgets_.find( name );
		if ( i != s_aliasWidgets_.end() )
			return (*i).second;
		else
			return NULL;
	}

	static void fini()
	{
		s_aliasWidgets_.clear();
	}
#endif

	MetaDataType * pMetaDataType()	{ return pMetaDataType_; }
	const MetaDataType * pMetaDataType() const	{ return pMetaDataType_; }

	bool isConst() const			{ return isConst_; }

	bool canIgnoreAssignment( PyObject * pOldValue,
			PyObject * pNewValue ) const;
	bool hasChanged( PyObject * pOldValue, PyObject * pNewValue ) const;

	// derived class should call this first then do own checks
	virtual bool operator<( const DataType & other ) const
		{ return pMetaDataType_ < other.pMetaDataType_ ; }

	virtual std::string typeName() const;

	static void clearStaticsForReload();

	static void reloadAllScript()
	{
		DataType::callOnEach( &DataType::reloadScript );
	}

	static void clearAllScript()
	{
		DataType::callOnEach( &DataType::clearScript );
	}
	static void callOnEach( void (DataType::*f)() );

protected:
	MetaDataType * pMetaDataType_;
	bool isConst_;

private:
	// Knows to propagate internal change
	virtual bool isSmart() const	{ return false; }

	bool canIgnoreSelfAssignment() const
		{ return this->isConst() || this->isSmart(); }

	struct SingletonPtr
	{
		explicit SingletonPtr( DataType * pInst ) : pInst_( pInst ) { }

		DataType * pInst_;

		bool operator<( const SingletonPtr & me ) const
			{ return (*pInst_) < (*me.pInst_); }
	};
	typedef std::set< SingletonPtr > SingletonMap;
	static SingletonMap * s_singletonMap_;

	static DataTypePtr findOrAddType( DataTypePtr pDT );


	static bool initAliases();

	typedef std::map< std::string, DataTypePtr >	Aliases;
	static Aliases s_aliases_;
#ifdef EDITOR_ENABLED
	typedef std::map< std::string, DataSectionPtr >	AliasWidgets;
	static AliasWidgets s_aliasWidgets_;
#endif // EDITOR_ENABLED
};

#endif // DATA_TYPE_HPP
