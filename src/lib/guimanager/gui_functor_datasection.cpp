/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "gui_functor_datasection.hpp"
#include "gui_item.hpp"

BEGIN_GUI_NAMESPACE

void DataSectionFunctor::setRoot( DataSectionPtr root )
{
	BW_GUARD;

	ds_ = root;
}

const std::string& DataSectionFunctor::name() const
{
	static std::string name = "DataSection";
	return name;
}

bool DataSectionFunctor::text( const std::string& textor, ItemPtr item, std::string& result )
{
	BW_GUARD;

	DataSectionPtr section;
	if( ds_ && ( section = ds_->openSection( textor, false ) ) )
	{
		result = section->asString();
		return true;
	}
	return false;
}

bool DataSectionFunctor::update( const std::string& updater, ItemPtr item, unsigned int& result )
{
	BW_GUARD;

	if( ds_ && updater.find( '=' ) != updater.npos )
	{
		std::string name = updater.substr( 0, updater.find( '=' ) );
		std::string value = updater.substr( updater.find( '=' ) + 1 );
		if( !value.empty() && value[0] == '=' )
			value.erase( value.begin() );
		FunctorManager::trim( name );
		FunctorManager::trim( value );
		DataSectionPtr section = ds_->openSection( name, false );
		if( section )
		{
			if( section->asString() == value )
				result = 1;
			else
				result = 0;
			return true;
		}
	}
	return false;
}

DataSectionPtr DataSectionFunctor::import( const std::string& importer, ItemPtr item )
{
	return NULL;
}

bool DataSectionFunctor::act( const std::string& action, ItemPtr item, bool& result )
{
	BW_GUARD;

	if( ds_ && action.find( '=' ) != action.npos )
	{
		std::string name = action.substr( 0, action.find( '=' ) );
		std::string value = action.substr( action.find( '=' ) + 1 );
		FunctorManager::trim( name );
		FunctorManager::trim( value );
		DataSectionPtr section = ds_->openSection( name, false );
		if( section )
		{
			section->be( value );
			result = true;
			return true;
		}
	}
	return false;
}


END_GUI_NAMESPACE
