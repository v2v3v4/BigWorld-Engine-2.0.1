/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROPERTY_CHANGE_READER_HPP
#define PROPERTY_CHANGE_READER_HPP

#include <Python.h>

#include "cstdmf/smartpointer.hpp"
#include "cstdmf/stdmf.hpp"

class BinaryIStream;
class BitReader;
class PropertyOwnerBase;

typedef uint8 PropertyChangeType;
typedef SmartPointer< PyObject > PyObjectPtr;


/**
 *	This base class is used to read in and apply a property change from a
 *	stream.
 */
class PropertyChangeReader
{
public:
	int readAndApply( BinaryIStream & stream, PropertyOwnerBase * pOwner,
			int clientServerID,
			PyObjectPtr * ppOldValue = NULL,
			PyObjectPtr * ppChangePath = NULL );

	virtual void setIndex( int index ) {}

protected:
	virtual PyObjectPtr apply( BinaryIStream & stream,
			PropertyOwnerBase * pOwner, PyObjectPtr pChangePath ) = 0;

	// Virtual methods to allow derived classes to read their specific
	// information
	virtual int readExtraBits( BinaryIStream & stream ) = 0;
	virtual int readExtraBits( BitReader & reader, int leafSize ) = 0;

	void updatePath( PyObjectPtr * ppChangePath,
		PyObjectPtr pIndex = NULL ) const;
};


/**
 *	This class is used to read in and apply a change to a single property. It
 *	may be a simple property of an entity or a single value in another
 *	PropertyOwner like an array.
 */
class SinglePropertyChangeReader : public PropertyChangeReader
{
private:
	virtual PyObjectPtr apply( BinaryIStream & stream,
			PropertyOwnerBase * pOwner, PyObjectPtr pChangePath );

	virtual int readExtraBits( BinaryIStream & stream );
	virtual int readExtraBits( BitReader & reader, int leafSize );

	virtual void setIndex( int index ) { leafIndex_ = index; }

	int leafIndex_;
};


/**
 *	This class is used to read in and apply a change to an array that is not a
 *	single property. This includes adding and deleting elements and replacing
 *	slices.
 */
class SlicePropertyChangeReader : public PropertyChangeReader
{
private:
	virtual PyObjectPtr apply( BinaryIStream & stream,
			PropertyOwnerBase * pOwner, PyObjectPtr pChangePath );

	virtual int readExtraBits( BinaryIStream & stream );
	virtual int readExtraBits( BitReader & reader, int leafSize );

	int32 startIndex_; //< The start index of the slice to replace
	int32 endIndex_; //< One after the end index of the slice to replace
};

#endif // PROPERTY_CHANGE_READER_HPP
