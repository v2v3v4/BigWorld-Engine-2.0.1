/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _ARRAY_PROPERTIES_HELPER_HPP_
#define _ARRAY_PROPERTIES_HELPER_HPP_


#include "base_properties_helper.hpp"
#include "cstdmf/bw_functor.hpp"


// forward declarations
class EditorChunkItem;
class DataType;
typedef SmartPointer<DataType> DataTypePtr;


/**
 *	This is a properties helper class to manage arrays of properties.
 */
class ArrayPropertiesHelper : public BasePropertiesHelper
{
public:
	ArrayPropertiesHelper();
	virtual ~ArrayPropertiesHelper();

	virtual void init(
		EditorChunkItem* pItem,
		DataTypePtr dataType,
		PyObject* pSeq,
		BWBaseFunctor1<int>* changedCallback = NULL );

    virtual PyObject*		pSeq() const;

    virtual bool			addItem();
    virtual bool			delItem( int index );

	virtual std::string		propName( PropertyIndex index );
    virtual int				propCount() const;
	virtual PropertyIndex	propGetIdx( const std::string& name ) const;

	virtual PyObject*		propGetPy( PropertyIndex index );
	virtual bool			propSetPy( PropertyIndex index, PyObject * pObj );
	virtual DataSectionPtr	propGet( PropertyIndex index );
	virtual bool			propSet( PropertyIndex index, DataSectionPtr pTemp );

	void					propSetToDefault( int index );

	virtual bool			isUserDataObjectLink( int index );
	virtual bool			isUserDataObjectLinkArray( int index );

private:
	DataTypePtr			dataType_;
	PyObject*			pSeq_;
	SmartPointer< BWBaseFunctor1<int> >	changedCallback_;
};


#endif //_ARRAY_PROPERTIES_HELPER_HPP_
