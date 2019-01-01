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
#include "gui_functor_option.hpp"
#include "gui_item.hpp"

BEGIN_GUI_NAMESPACE

OptionFunctor::OptionFunctor()
	:option_( NULL )
{}

void OptionFunctor::setOption( OptionMap* option )
{
	BW_GUARD;

	option_ = option;
}

const std::string& OptionFunctor::name() const
{
	static std::string name = "Option";
	return name;
}

bool OptionFunctor::text( const std::string& textor, ItemPtr item, std::string& result )
{
	BW_GUARD;

	if( option_ && option_->exist( textor ) )
	{
		result = option_->get( textor );
		return true;
	}
	return false;
}

bool OptionFunctor::update( const std::string& updater, ItemPtr item, unsigned int& result )
{
	BW_GUARD;

	if( option_ && updater.find( '=' ) != updater.npos )
	{
		std::string name = updater.substr( 0, updater.find( '=' ) );
		std::string value = updater.substr( updater.find( '=' ) + 1 );
		if( !value.empty() && value[0] == '=' )
			value.erase( value.begin() );
		FunctorManager::trim( name );
		FunctorManager::trim( value );
		if( option_->exist( name ) )
		{
			if( option_->get( name ) == value )
				result = 1;
			else
				result = 0;
			return true;
		}
	}
	return false;
}

DataSectionPtr OptionFunctor::import( const std::string& importer, ItemPtr item )
{
	return NULL;
}

bool OptionFunctor::act( const std::string& action, ItemPtr item, bool& result )
{
	BW_GUARD;

	if( option_ && action.find( '=' ) != action.npos )
	{
		std::string name = action.substr( 0, action.find( '=' ) );
		std::string value = action.substr( action.find( '=' ) + 1 );
		FunctorManager::trim( name );
		FunctorManager::trim( value );
		if( option_->exist( name ) )
		{
			option_->set( name, value );
			result = true;
			return true;
		}
	}
	return false;
}


END_GUI_NAMESPACE
