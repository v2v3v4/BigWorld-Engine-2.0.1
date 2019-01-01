/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_PROPERTY_PARSER_HPP
#define ENTITY_PROPERTY_PARSER_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"


/**
 *	This interface is used by EditorChunkEntity to parse entity properties
 *  and their widgets, such as ENUM, RADIUS, etc. Extend it in the cpp to
 *  add support for other widgets.
 */
class EntityPropertyParser : public ReferenceCount
{
public:
	// interface
	virtual GeneralProperty* createProperty(
		BasePropertiesHelper* props,
		int propIndex,
		const std::string& name,
		DataTypePtr dataType,
		DataSectionPtr widget,
		MatrixProxy* pMP ) = 0;


	// statics
	class Factory: public ReferenceCount
	{
	public:
		virtual EntityPropertyParserPtr create( PyObject* pyClass,
			const std::string& name, DataTypePtr dataType, DataSectionPtr widget ) = 0;
	};
	typedef SmartPointer<Factory> FactoryPtr;

	static void registerFactory( FactoryPtr factory );
	static EntityPropertyParserPtr create(
		PyObject* pyClass, const std::string& name, DataTypePtr dataType, DataSectionPtr widget );

private:
	static std::vector<FactoryPtr> s_factories_;
};


#endif // ENTITY_PROPERTY_PARSER_HPP
