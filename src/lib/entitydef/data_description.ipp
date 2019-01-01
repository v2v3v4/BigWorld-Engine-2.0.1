/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifdef CODE_INLINE
#define INLINE	inline
#else
/// INLINE macro
#define INLINE
#endif


/**
 *	This method adds the value of the appropriate type onto the input bundle.
 */
INLINE
bool DataDescription::addToStream( PyObject * pNewValue,
			BinaryOStream & stream, bool isPersistentOnly ) const
{
	return pDataType_->addToStream( pNewValue, stream, isPersistentOnly );
}

INLINE
bool DataDescription::addToSection( PyObject * pNewValue,
			DataSectionPtr pSection )
{
	return pDataType_->addToSection( pNewValue, pSection );
}

/**
 *	This method creates a value of the appropriate type from the stream. It
 *	returns a new reference.
 */
INLINE
PyObjectPtr DataDescription::createFromStream( BinaryIStream & stream,
	bool isPersistentOnly ) const
{
	return pDataType_->createFromStream( stream, isPersistentOnly );
}


/**
 *	This method creates a value of the appropriate type from a datasection. It
 *	returns a new reference.
 */
INLINE
PyObjectPtr DataDescription::createFromSection( DataSectionPtr pSection ) const
{
	return pDataType_->createFromSection( pSection );
}


/**
 *	This method reads a value from the input stream and adds it to the input
 *	data section.
 */
INLINE
bool DataDescription::fromStreamToSection( BinaryIStream & stream,
		DataSectionPtr pSection, bool isPersistentOnly ) const
{
	return pDataType_->fromStreamToSection( stream, pSection, isPersistentOnly );
}


/**
 *	This method reads a value from the input data section and adds it to the
 *	input stream.
 */
INLINE
bool DataDescription::fromSectionToStream( DataSectionPtr pSection,
			BinaryOStream & stream, bool isPersistentOnly ) const
{
	if (!pSection ||
			!pDataType_->fromSectionToStream( pSection, stream, isPersistentOnly ))
	{
		PyObjectPtr pValue = this->pInitialValue();
		return pDataType_->addToStream( pValue.getObject(), stream, 
			isPersistentOnly );
	}
	return true;
}

// -----------------------------------------------------------------------------
// Section: Accessors
// -----------------------------------------------------------------------------

/**
 *	This method returns the name associated with the data described by this
 *	object.
 */
INLINE
const std::string& DataDescription::name() const
{
	return name_;
}


#if 0
/**
 * 	This method returns the priority of the data described by this object.
 * 	Data with higher priority will be sent more often when there is limited
 * 	bandwidth.
 */
INLINE
int DataDescription::priority() const
{
	return priority_;
}
#endif


/**
 * 	This method returns true if changes in this property should be
 * 	propogated to ghosts.
 */
INLINE
bool DataDescription::isGhostedData() const
{
	// Double not is to get rid of VC++ warnings. Kind of lame.
	return !!(dataFlags_ & DATA_GHOSTED);
}


/**
 * 	This method returns true if nearby clients other than the owner
 * 	should be informed when this property changes.
 */
INLINE
bool DataDescription::isOtherClientData() const
{
	return !!(dataFlags_ & DATA_OTHER_CLIENT);
}


/**
 * 	This method returns true if the client who owns this entity
 * 	should be informed when this property changes.
 */
INLINE
bool DataDescription::isOwnClientData() const
{
	return !!(dataFlags_ & DATA_OWN_CLIENT);
}


/**
 * 	This method returns true if this property is stored on a cell rather than
 * 	on a base.
 */
INLINE
bool DataDescription::isCellData() const
{
	// At the moment, data only lives on a base or a cell but not both. At some
	// stage, we may have data that lives on both a base and a cell. We probably
	// still want one to be authoritative.
	return !this->isBaseData();
}


/**
 * 	This method returns true if this property is stored on a base rather
 * 	than on a cell.
 */
INLINE
bool DataDescription::isBaseData() const
{
	return !!(dataFlags_ & DATA_BASE);
}


/**
 * 	This method returns true if changes in the property should be propogated
 * 	to all nearby clients, including the owner of the entity.
 *
 * 	@see isOtherClientData()
 * 	@see isOwnClientData()
 */
INLINE
bool DataDescription::isClientServerData() const
{
	return !!(dataFlags_ & (DATA_OTHER_CLIENT | DATA_OWN_CLIENT));
}


/**
 *	This method returns whether or not this property should be stored to the
 *	database.
 */
INLINE
bool DataDescription::isPersistent() const
{
	return !!(dataFlags_ & DATA_PERSISTENT);
}


/**
 *	This method returns whether or not this property is only handled by the
 *	editor.
 */
INLINE
bool DataDescription::isEditorOnly() const
{
	return !!(dataFlags_ & DATA_EDITOR_ONLY);
}

/**
 *	This method returns whether or not this property is an identifier for the
 * 	entity.
 */
INLINE
bool DataDescription::isIdentifier() const
{
	return !!(dataFlags_ & DATA_ID);
}

/**
 * 	This method returns true if all the given flags apply to this
 * 	property.
 */
INLINE
bool DataDescription::isOfType( EntityDataFlags flags )
{
	return (dataFlags_ & flags) == flags;
}


/**
 * 	This method returns the index of this property.
 * 	Indicies are ordered sequentially, based on the order of their definition
 * 	in the entities.xml file.
 */
INLINE
int DataDescription::index() const
{
	return index_;
}


/**
 * 	This method sets the index of this property.
 * 	This will normally be called when parsing this property from xml at startup.
 */
INLINE
void DataDescription::index( int index )
{
	index_ = index;
}


/**
 *	This method returns the index that is used to save the number of the event
 *	that last changed this property.
 */
INLINE
int DataDescription::eventStampIndex() const
{
	return eventStampIndex_;
}


/**
 *	This method sets the index that is used to save the number of the event that
 *	last changed this property.
 */
INLINE
void DataDescription::eventStampIndex( int index )
{
	eventStampIndex_ = index;
}


/**
 *	This method returns the detail level associated with this property.
 */
INLINE
int DataDescription::detailLevel() const
{
	return detailLevel_;
}


/**
 *	This method sets the detail level associated with this property.
 */
INLINE
void DataDescription::detailLevel( int level )
{
	detailLevel_ = level;
}


/**
 *  This method returns the length that this field should consume in the
 *  database
 */
INLINE
int DataDescription::databaseLength() const
{
	return databaseLength_;
}


#ifdef EDITOR_ENABLED
/**
 *	This method returns if we want the user to be able to see and edit the
 *	property in WorldEditor
 */
INLINE
bool DataDescription::editable() const
{
	return editable_;
}

/**
 *	This method sets the user's ability to be able to see and edit the
 *	property in WorldEditor
 */
INLINE
void DataDescription::editable( bool v )
{
	editable_ = v;
}
#endif	// EDITOR_ENABLED


// data_description.ipp
