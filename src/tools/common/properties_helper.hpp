/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _PROPERTIES_HELPER_HPP_
#define _PROPERTIES_HELPER_HPP_


#include "base_properties_helper.hpp"
#include "cstdmf/bw_functor.hpp"


// forward declarations
class EditorChunkItem;
class BaseUserDataObjectDescription;


/**
 *	This is a properties helper class. It must be initialised using a
 *	editor chunk, a type description and a python dictionary.
 */
class PropertiesHelper : public BasePropertiesHelper
{
public:
	typedef std::map< int, PropertyIndex > PropertyMap;

	PropertiesHelper();

	virtual void init(
		EditorChunkItem* pItem,
		BaseUserDataObjectDescription* pType,
		PyObject* pDict,
		BWBaseFunctor1<int>* changedCallback = NULL );

	virtual BaseUserDataObjectDescription*	pType() const;

	virtual std::string		propName( PropertyIndex index );
    virtual int				propCount() const;
	virtual PropertyIndex	propGetIdx( const std::string& name ) const;

	virtual PyObject*		propGetPy( PropertyIndex index );
	virtual bool			propSetPy( PropertyIndex index, PyObject * pObj );
	virtual DataSectionPtr	propGet( PropertyIndex index );
	virtual bool			propSet( PropertyIndex index, DataSectionPtr pTemp );

	PyObjectPtr				propGetDefault( int index );
	void					propSetToDefault( int index );

	virtual void			setEditProps(
								const std::list<std::string>& names, std::vector<bool>& allowEdit );
	virtual void			clearEditProps( std::vector<bool>& allowEdit );
	virtual void			clearProperties();
	virtual void			clearPropertySection( DataSectionPtr pOwnSect );

	virtual bool			isUserDataObjectLink( int index );
	virtual bool			isUserDataObjectLinkArray( int index );

	std::vector<std::string>	command();
	PropertyIndex				commandIndex( int index );

private:
	BaseUserDataObjectDescription*			pType_;
	PyObject*								pDict_;
	SmartPointer< BWBaseFunctor1<int> >	changedCallback_;
	PropertyMap								propMap_;
};


#endif //_PROPERTIES_HELPER_HPP_
