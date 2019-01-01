/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROPERTY_CHANGE_HPP
#define PROPERTY_CHANGE_HPP

#include <Python.h>

#include "cstdmf/smartpointer.hpp"
#include "cstdmf/stdmf.hpp"

#include <vector>

class BinaryOStream;
class BitWriter;
class DataType;
class PropertyOwnerBase;

typedef uint8 PropertyChangeType;
typedef SmartPointer< PyObject > PyObjectPtr;

const PropertyChangeType PROPERTY_CHANGE_TYPE_SINGLE = 0;
const PropertyChangeType PROPERTY_CHANGE_TYPE_SLICE = 1;

const int MAX_SIMPLE_PROPERTY_CHANGE_ID = 60;
const int PROPERTY_CHANGE_ID_SINGLE = 61;
const int PROPERTY_CHANGE_ID_SLICE = 62;


// -----------------------------------------------------------------------------
// Section: PropertyChange
// -----------------------------------------------------------------------------

/**
 *	This class represents a change to a property of an entity.
 */
class PropertyChange
{
public:
	PropertyChange( const DataType & type );

	virtual uint8 addToStream( BinaryOStream & stream,
			const PropertyOwnerBase * pOwner, int messageID ) const = 0;

	virtual PropertyChangeType type() const = 0;

	int rootIndex() const
		{ return path_.empty() ? this->getRootIndex() : path_.back(); }

	void addToPath( int index )
	{
		path_.push_back( index );
	}

protected:
	// A sequence of child indexes ordered from the leaf to the root
	// (i.e. entity). For example, 3,4,6 would be the 6th property of the
	// entity, the 4th "child" of that property and then the 3rd "child".
	// E.g. If the 6th property is a list of lists called myList, this refers
	// to entity.myList[4][3]
	typedef std::vector< int32 > ChangePath;

	uint8 addPathToStream( BinaryOStream & stream,
			const PropertyOwnerBase * pOwner, int messageID ) const;

	void writePathSimple( BinaryOStream & stream ) const;

	virtual void addExtraBits( BitWriter & writer, int leafSize ) const = 0;
	virtual void addExtraBits( BinaryOStream & stream ) const = 0;

	virtual int getRootIndex() const	{ return 0; }

	const DataType & type_; //< Type of the new value(s).
	ChangePath path_; //< Path to the owner being changed.
};


/**
 *	This class is a specialised PropertyChange. It represents a single value of
 *	an entity changing.
 */
class SinglePropertyChange : public PropertyChange
{
public:
	SinglePropertyChange( int leafIndex, const DataType & type );

	virtual uint8 addToStream( BinaryOStream & stream,
			const PropertyOwnerBase * pOwner, int messageID ) const;

	virtual PropertyChangeType type() const
	{
		return PROPERTY_CHANGE_TYPE_SINGLE;
	}

	void setValue( PyObjectPtr pValue )	{ pValue_ = pValue; }

private:
	virtual void addExtraBits( BitWriter & writer, int leafSize ) const;
	virtual void addExtraBits( BinaryOStream & stream ) const;

	virtual int getRootIndex() const	{ return leafIndex_; }

	int leafIndex_;
	PyObjectPtr pValue_;
};


/**
 *	This class is a specialised PropertyChange. It represents a change to an
 *	array replacing a slice of values.
 */
class SlicePropertyChange : public PropertyChange
{
public:
	SlicePropertyChange( Py_ssize_t startIndex, Py_ssize_t endIndex,
			const std::vector< PyObjectPtr > & newValues,
			const DataType & type );

	virtual uint8 addToStream( BinaryOStream & stream,
			const PropertyOwnerBase * pOwner, int messageID ) const;

	virtual PropertyChangeType type() const
	{
		return PROPERTY_CHANGE_TYPE_SLICE;
	}

private:
	virtual void addExtraBits( BitWriter & writer, int leafSize ) const;
	virtual void addExtraBits( BinaryOStream & stream ) const;

	int32 startIndex_;
	int32 endIndex_;
	const std::vector< PyObjectPtr > & newValues_;
};

#endif // PROPERTY_CHANGE_HPP
