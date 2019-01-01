/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _BASE_PROPERTIES_HELPER_HPP_
#define _BASE_PROPERTIES_HELPER_HPP_


// forward declarations
class EditorChunkItem;


/**
 *  This class stores indices for the item and sub-items of a property. If the
 *  corresponding property is not an array, the "count" should be 1, but if a
 *  property is an array, a PropertyItem corresponding to an element in the array
 *  will contain 2 indices: one is the index of the property and the other is the
 *  sub-index of the array item in the array.
 */
class PropertyIndex
{
public:
	PropertyIndex();
	PropertyIndex( int propIdx );

	bool empty() const;
	int count() const;

	PropertyIndex tail() const;
	void append( const PropertyIndex& i );

	int valueAt( int i );
	bool valueAt( int i, int propIdx );

private:
	std::vector<int> indices_;
};


/**
 *	This is the base properties helper class.
 */
class BasePropertiesHelper
{
public:
	BasePropertiesHelper();

	virtual EditorChunkItem*	pItem() const;

	virtual void			resetSelUpdate( bool keepSelection = false );
	virtual void			refreshItem();

	virtual std::string		propName( PropertyIndex index ) = 0;
    virtual int				propCount() const = 0;
	virtual PropertyIndex	propGetIdx( const std::string& name ) const = 0;

	virtual bool			propUsingDefault( int index );
	virtual void			propUsingDefault( int index, bool usingDefault );
	virtual void			propUsingDefaults( std::vector<bool> usingDefault );

	virtual PyObject*		propGetPy( PropertyIndex index ) = 0;
	virtual bool			propSetPy( PropertyIndex index, PyObject * pObj ) = 0;
	virtual DataSectionPtr	propGet( PropertyIndex index ) = 0;
	virtual bool			propSet( PropertyIndex index, DataSectionPtr pTemp ) = 0;
	virtual int				propGetInt( PropertyIndex index );
	virtual bool			propSetInt( PropertyIndex index, int i );
	virtual uint32			propGetUInt( PropertyIndex index );
	virtual bool			propSetUInt( PropertyIndex index, uint32 i );
	virtual float			propGetFloat( PropertyIndex index );
	virtual bool			propSetFloat( PropertyIndex index, float f );
	virtual std::string		propGetString( PropertyIndex index ) const;
	virtual bool			propSetString( PropertyIndex index, const std::string & s );
	virtual Vector2			propGetVector2( PropertyIndex index );
	virtual bool			propSetVector2( PropertyIndex index, Vector2 v );
	virtual Vector4			propGetVector4( PropertyIndex index );
	virtual bool			propSetVector4( PropertyIndex index, Vector4 v );

	virtual bool			isUserDataObjectLink( int index ) = 0;
	virtual bool			isUserDataObjectLinkArray( int index ) = 0;

protected:
	EditorChunkItem*		pItem_;
	std::vector<bool>		usingDefault_;
};


#endif //_BASE_PROPERTIES_HELPER_HPP_
