/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROPERTY_OWNER_HPP
#define PROPERTY_OWNER_HPP

#include "pyscript/pyobject_plus.hpp"

#include "cstdmf/smartpointer.hpp"

class BinaryIStream;
class BinaryOStream;
class DataType;
class PropertyChange;

typedef uint8 PropertyChangeType;
typedef SmartPointer< PyObject > PyObjectPtr;


/**
 *	This base class is an object that can own properties.
 */
class PropertyOwnerBase
{
public:
	virtual ~PropertyOwnerBase() { }

	/**
	 *	This method is called by a child PropertyOwnerBase to inform us that
	 *	a property has changed. Each PropertyOwner should pass this to their
	 *	parent, adding their index to the path, until the Entity is reached.
	 */
	virtual void onOwnedPropertyChanged( PropertyChange & change ) {};

	virtual bool getTopLevelOwner( PropertyChange & change,
			PropertyOwnerBase *& rpTopLevelOwner ) { return true; }

	/**
	 *	This method returns the number of child properties this PropertyOwner
	 *	currently has.
	 */
	virtual int getNumOwnedProperties() const = 0;

	/**
	 *	This method returns the child PropertyOwner of this PropertyOwner. If
	 *	the child property is not a PropertyOwner, NULL is returned.
	 *
	 *	@param childIndex The index of the child to get.
	 */
	virtual PropertyOwnerBase *
		getChildPropertyOwner( int childIndex ) const = 0;

	/**
	 *	This method sets a child property to a new value.
	 *
	 *	@param childIndex	The index of the child property to change.
	 *	@param data			A stream containing the new value.
	 *
	 *	@return The previous value of the property.
	 */
	virtual PyObjectPtr setOwnedProperty( int childIndex,
			BinaryIStream & data ) = 0;


	/**
	 *	This method modifies a slice of an array property.
	 *
	 *	@param startIndex The start index of the slice to replace.
	 *	@param endIndex One greater than the end index of the slice to replace.
	 *	@param data A stream containing the new values.
	 */
	virtual PyObjectPtr setOwnedSlice( int startIndex, int endIndex,
			BinaryIStream & data ) { return NULL; }


	/**
	 *	This method returns a Python object representing the property with the
	 *	given index.
	 */
	virtual PyObjectPtr getPyIndex( int index ) const = 0;


	/**
	 *	This method modifies a property owned by this object.
	 *
	 *	@param rpOldValue A reference that will be populated with the old value.
	 *	@param pNewValue The new value to set the property to.
	 *	@param dataType The type of the property being changed.
	 *	@param index The index of the property being changed.
	 */
	bool changeOwnedProperty( PyObjectPtr & rpOldValue, PyObject * pNewValue,
								DataType & dataType, int index );
};


/**
 *	This is the normal property owner for classes that can have a virtual
 *	function table.
 */
class PropertyOwner : public PyObjectPlus, public PropertyOwnerBase
{
protected:
	PropertyOwner( PyTypePlus * pType ) : PyObjectPlus( pType ) { }
};


/**
 *	This class specialises PropertyOwnerBase to add functionality for top-level
 *	Property Owners. That is Entity.
 */
class TopLevelPropertyOwner : public PropertyOwnerBase
{
public:
	int setPropertyFromInternalStream( BinaryIStream & stream,
			PropertyChangeType type );

	int setPropertyFromExternalStream( BinaryIStream & stream,
			int clientServerIndex,
			PyObjectPtr * ppOldValue, PyObjectPtr * ppChangePath );

private:
	int setPropertyFromStream( BinaryIStream & stream,
			int clientServerID = -1,
			PyObjectPtr * ppOldValue = NULL,
			PyObjectPtr * ppChangePath = NULL );

	int setSliceFromStream( BinaryIStream & stream,
			int clientServerID = -1,
			PyObjectPtr * ppOldValue = NULL,
			PyObjectPtr * ppChangePath = NULL );

	virtual PyObjectPtr getPyIndex( int index ) const
	{
		MF_ASSERT( 0 );
		return NULL;
	}
};


/**
 *	This is a handy linking class for objects that dislike virtual functions.
 */
template <class C>
class PropertyOwnerLink : public TopLevelPropertyOwner
{
public:
	PropertyOwnerLink( C & self ) : self_( self ) { }

	virtual void onOwnedPropertyChanged( PropertyChange & change )
	{
		self_.onOwnedPropertyChanged( change );
	}

	virtual bool getTopLevelOwner( PropertyChange & change,
			PropertyOwnerBase *& rpTopLevelOwner )
	{
		return self_.getTopLevelOwner( change, rpTopLevelOwner );
	}

	virtual int getNumOwnedProperties() const
	{
		return self_.getNumOwnedProperties();
	}

	virtual PropertyOwnerBase * getChildPropertyOwner( int ref ) const
	{
		return self_.getChildPropertyOwner( ref ) ;
	}

	virtual PyObjectPtr setOwnedProperty( int ref, BinaryIStream & data )
	{
		return self_.setOwnedProperty( ref, data );
	}

private:
	C & self_;
};

#endif // PROPERTY_OWNER_HPP
