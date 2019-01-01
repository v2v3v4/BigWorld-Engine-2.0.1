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
#include "gui_functor.hpp"
#include "gui_item.hpp"
#include "resmgr/string_provider.hpp"

BEGIN_GUI_NAMESPACE

Functor::~Functor()
{}

std::map< std::string, Functor* >& FunctorManager::functors()
{
	static std::map< std::string, Functor* > functors;
	return functors;
}

void FunctorManager::trim( std::string& str )
{
	BW_GUARD;

	while( !str.empty() && isspace( str[ 0 ] ) )
		str.erase( str.begin() );
	while( !str.empty() && isspace( *str.rbegin() ) )
		str.resize( str.size() - 1 );
}

FunctorManager::FunctorManager()
{
}

FunctorManager::~FunctorManager()
{
}

std::string FunctorManager::text( const std::string& textor, ItemPtr item )
{
	BW_GUARD;

	std::string functor = textor;
	std::string functorName;
	std::string result;
	if( functor.find( ':' ) != functor.npos )
	{
		functorName = functor.substr( 0, functor.find( ':' ) );
		functor = functor.substr( functor.find( ':' ) + 1 );
		trim( functorName );
		trim( functor );
	}
	if( functors().find( functorName ) != functors().end() )
	{
		if( !functors().find( functorName )->second->text( functor, item, result ) )
			0;// should give some error message here
		return result;
	}
	for( std::map< std::string, Functor* >::iterator iter = functors().begin();
		iter != functors().end(); ++iter )
	{
		if( iter->second->text( functor, item, result ) )
			return result;
	}
	if( functor.size() >=2 && functor[0] == '\"' && *functor.rbegin() == '\"' )
	{
		functor.erase( functor.begin() );
		functor.resize( functor.size() - 1 );
	}
	else
	{
		std::wstring wfunctor;
		bw_utf8tow( functor, wfunctor );
		if( const wchar_t* str = Localise( wfunctor.c_str(), StringProvider::RETURN_NULL_IF_NOT_EXISTING ) )
		{
			bw_wtoutf8( str, functor );
		}
	}
	return functor;
}

unsigned int FunctorManager::update( const std::string& updater, ItemPtr item )
{
	BW_GUARD;

	std::string functor = updater;
	std::string functorName;
	unsigned int result = 1;
	if( functor.find( ':' ) != functor.npos )
	{
		functorName = functor.substr( 0, functor.find( ':' ) );
		functor = functor.substr( functor.find( ':' ) + 1 );
		trim( functorName );
		trim( functor );
	}
	if( functors().find( functorName ) != functors().end() )
	{
		if( !functors().find( functorName )->second->update( functor, item, result ) )
			0;//// should give some error message here
		return result;
	}
	for( std::map< std::string, Functor* >::iterator iter = functors().begin();
		iter != functors().end(); ++iter )
	{
		if( iter->second->update( functor, item, result ) )
			return result;
	}
	return result;
}

DataSectionPtr FunctorManager::import( const std::string& importer, ItemPtr item )
{
	return NULL;
}

bool FunctorManager::act( const std::string& action, ItemPtr item )
{
	BW_GUARD;

	std::string functor = action;
	std::string functorName;
	bool result = false;
	if( functor.find( ':' ) != functor.npos )
	{
		functorName = functor.substr( 0, functor.find( ':' ) );
		functor = functor.substr( functor.find( ':' ) + 1 );
		trim( functorName );
		trim( functor );
	}
	if( functors().find( functorName ) != functors().end() )
	{
		if( !functors().find( functorName )->second->act( functor, item, result ) )
			0;//// should give some error message here
		return result;
	}
	for( std::map< std::string, Functor* >::iterator iter = functors().begin();
		iter != functors().end(); ++iter )
	{
		if( iter->second->act( functor, item, result ) )
			return result;
	}
	return result;
}

void FunctorManager::registerFunctor( Functor* functor )
{
	BW_GUARD;

	functors()[ functor->name() ] = functor;
}

void FunctorManager::unregisterFunctor( Functor* functor )
{
	BW_GUARD;

	if( functors().find( functor->name() ) != functors().end() &&
		functors().find( functor->name() )->second == functor )
		functors().erase( functors().find( functor->name() ) );
}

END_GUI_NAMESPACE
